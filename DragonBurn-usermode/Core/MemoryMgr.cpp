#include "MemoryMgr.h"
#include <TlHelp32.h>
#include <random>
#include <thread>
#include <chrono>

ObfuscatedMemoryManager::ObfuscatedMemoryManager()
{
	m_process_id = 0;
	m_kernel_driver_handle = nullptr;
}

void ObfuscatedMemoryManager::chaotic_sleep(int min_ms, int max_ms)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min_ms, max_ms);
    std::this_thread::sleep_for(std::chrono::milliseconds(distrib(gen)));
}

DWORD ObfuscatedMemoryManager::func_get_pid_um(const wchar_t* processName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(PROCESSENTRY32);

    DWORD pid = 0;
    if (Process32First(snapshot, &entry))
    {
        do
        {
            if (_wcsicmp(entry.szExeFile, processName) == 0)
            {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return pid;
}

ObfuscatedMemoryManager::~ObfuscatedMemoryManager()
{
	func_end_comm();
	if (m_user_process_handle != nullptr)
	{
		CloseHandle(m_user_process_handle);
		m_user_process_handle = nullptr;
	}
	m_process_id = 0;
	m_kernel_driver_handle = nullptr;
}

bool ObfuscatedMemoryManager::func_init_comm(const LPCWSTR name)
{
    for (int i = 0; i < 5; ++i) {
        m_kernel_driver_handle = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (m_kernel_driver_handle != INVALID_HANDLE_VALUE) {
            if (i > 0) {
                YieldProcessor(); // equivalent lightweight pause
            }
            return true;
        }
        chaotic_sleep(50, 200);
    }

    m_kernel_driver_handle = nullptr;
    return false;
}

bool ObfuscatedMemoryManager::func_end_comm()
{
	if (m_kernel_driver_handle != nullptr)
	{
		BOOL result = CloseHandle(m_kernel_driver_handle);
		m_kernel_driver_handle = nullptr;
		return result == TRUE;
	}
	return false;
}

bool ObfuscatedMemoryManager::func_attach_target(const DWORD pid)
{
    if (pid == 0) return false;

    for (int i = 0; i < 3; ++i) {
        if (m_kernel_driver_handle == nullptr) {
            // Attempt to reconnect to the driver if handle is lost
            if (!func_init_comm(L"\\\\.\\DB")) {
                chaotic_sleep(100, 300);
                continue;
            }
        }

        ORequest attachRequest;
        attachRequest.process_id = ULongToHandle(pid);
        attachRequest.target = nullptr;
        attachRequest.buffer = nullptr;
        attachRequest.size = 0;

        if (DeviceIoControl(m_kernel_driver_handle, OBF_IOCTL_ATTACH, &attachRequest, sizeof(attachRequest), &attachRequest, sizeof(attachRequest), nullptr, nullptr))
        {
            // Verify process handle is valid after attach
            HANDLE pHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
            if (pHandle != NULL) {
                DWORD exitCode;
                if (GetExitCodeProcess(pHandle, &exitCode) && exitCode == STILL_ACTIVE) {
                    CloseHandle(pHandle);
                    m_process_id = pid;
                    return true;
                }
                CloseHandle(pHandle);
            }
        }

        // If attach fails, nullify handle to trigger reconnect
        m_kernel_driver_handle = nullptr; 
        chaotic_sleep(100, 500);
    }
    return false;
}

static DWORD64 align_down(DWORD64 addr, DWORD64 align) { return addr & ~(align - 1); }
static DWORD64 align_up(DWORD64 addr, DWORD64 align) { return (addr + align - 1) & ~(align - 1); }

SIZE_T ObfuscatedMemoryManager::page_size() const {
    SYSTEM_INFO si{}; GetSystemInfo(&si); return static_cast<SIZE_T>(si.dwPageSize);
}

bool ObfuscatedMemoryManager::query_region_user(DWORD64 address, VmRegion& out) const {
    if (m_user_process_handle == nullptr) return false;
    MEMORY_BASIC_INFORMATION mbi{};
    SIZE_T r = VirtualQueryEx(m_user_process_handle, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi));
    if (r == 0) return false;
    out.base = reinterpret_cast<DWORD64>(mbi.BaseAddress);
    out.size = static_cast<SIZE_T>(mbi.RegionSize);
    out.protect = mbi.Protect;
    out.readable = (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)) != 0;
    out.writable = (mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE)) != 0;
    return mbi.State == MEM_COMMIT;
}

SIZE_T ObfuscatedMemoryManager::clamp_copy_safe(DWORD64 address, SIZE_T requested) const {
    VmRegion reg{};
    if (!query_region_user(address, reg)) return 0;
    DWORD64 end = address + requested;
    DWORD64 regEnd = reg.base + reg.size;
    if (end <= reg.base) return 0;
    DWORD64 maxEnd = end < regEnd ? end : regEnd;
    return static_cast<SIZE_T>(maxEnd - address);
}

bool ObfuscatedMemoryManager::func_read_buffer_retry(DWORD64 address, void* buffer, SIZE_T size, int maxRetries, DWORD initialSleepMs)
{
    if (buffer == nullptr || size == 0)
        return false;

    const SIZE_T sysPage = page_size();

    for (int attempt = 0; attempt < maxRetries; ++attempt)
    {
        BYTE* out = static_cast<BYTE*>(buffer);
        SIZE_T totalRead = 0;

        while (totalRead < size)
        {
            DWORD64 curAddr = address + totalRead;
            SIZE_T remaining = size - totalRead;

            SIZE_T safeSpan = clamp_copy_safe(curAddr, remaining);
            if (safeSpan == 0) { break; }

            // split into smaller chunks to reduce IO failures
            const SIZE_T maxChunk = (sysPage < 0x800 ? sysPage : 0x800);
            SIZE_T toRead = (safeSpan > maxChunk) ? maxChunk : safeSpan;

            if (m_kernel_driver_handle != nullptr && m_process_id != 0)
            {
                if (!func_read(curAddr, *(reinterpret_cast<BYTE(*)>(out + totalRead)), toRead))
                {
                    totalRead = 0; // reset progress for retry
                    break;
                }
            }
            else if (m_user_process_handle != nullptr && m_process_id != 0)
            {
                SIZE_T bytesRead = 0;
                if (!ReadProcessMemory(m_user_process_handle, reinterpret_cast<LPCVOID>(curAddr), out + totalRead, toRead, &bytesRead) || bytesRead != toRead)
                {
                    totalRead = 0;
                    break;
                }
            }
            else {
                totalRead = 0; break;
            }

            totalRead += toRead;
        }

        if (totalRead == size)
            return true;

        chaotic_sleep(initialSleepMs, initialSleepMs * (attempt + 1) * 2);
    }
    return false;
}

bool ObfuscatedMemoryManager::func_write_um(DWORD64 address, const void* buffer, SIZE_T size, int maxRetries, DWORD initialSleepMs)
{
    if (m_user_process_handle == nullptr || m_process_id == 0 || buffer == nullptr || size == 0)
        return false;

    const SIZE_T sysPage = page_size();
    const BYTE* in = static_cast<const BYTE*>(buffer);

    for (int attempt = 0; attempt < maxRetries; ++attempt)
    {
        SIZE_T totalWritten = 0;

        while (totalWritten < size)
        {
            DWORD64 curAddr = address + totalWritten;
            SIZE_T remaining = size - totalWritten;
            SIZE_T safeSpan = clamp_copy_safe(curAddr, remaining);
            if (safeSpan == 0) { totalWritten = 0; break; }

            const SIZE_T maxChunk = (sysPage < 0x800 ? sysPage : 0x800);
            SIZE_T toWrite = (safeSpan > maxChunk) ? maxChunk : safeSpan;

            DWORD oldProtect = 0;
            bool changed = false;
            VmRegion reg{};
            if (query_region_user(curAddr, reg) && !reg.writable)
            {
                if (!VirtualProtectEx(m_user_process_handle, reinterpret_cast<LPVOID>(align_down(curAddr, sysPage)), sysPage, PAGE_EXECUTE_READWRITE, &oldProtect))
                {
                    totalWritten = 0; break;
                }
                changed = true;
            }

            SIZE_T bytesWritten = 0;
            bool ok = WriteProcessMemory(m_user_process_handle, reinterpret_cast<LPVOID>(curAddr), in + totalWritten, toWrite, &bytesWritten) && bytesWritten == toWrite;

            if (changed)
            {
                DWORD tmp = 0; VirtualProtectEx(m_user_process_handle, reinterpret_cast<LPVOID>(align_down(curAddr, sysPage)), sysPage, oldProtect, &tmp);
            }

            if (!ok)
            {
                totalWritten = 0; break;
            }

            totalWritten += toWrite;
        }

        if (totalWritten == size)
            return true;

        chaotic_sleep(initialSleepMs, initialSleepMs * (attempt + 1) * 2);
    }
    return false;
}

std::string ObfuscatedMemoryManager::func_read_str_um(DWORD64 address, SIZE_T maxLength, int maxRetries)
{
	if (maxLength == 0 || maxLength > 0x10000)
		return {};

	std::string buffer;
	buffer.resize(maxLength);

	if (func_read_buffer_retry(address, buffer.data(), maxLength, maxRetries))
	{
		size_t end = buffer.find('\0');
		if (end != std::string::npos)
			buffer.resize(end);
		return buffer;
	}
	return {};
}

std::wstring ObfuscatedMemoryManager::func_read_wstr_um(DWORD64 address, SIZE_T maxLength, int maxRetries)
{
	if (maxLength == 0 || maxLength > 0x10000)
		return {};

	std::wstring buffer;
	buffer.resize(maxLength);

	if (func_read_buffer_retry(address, buffer.data(), maxLength * sizeof(wchar_t), maxRetries))
	{
		size_t end = buffer.find(L'\0');
		if (end != std::wstring::npos)
			buffer.resize(end);
		return buffer;
	}
	return {};
}

DWORD ObfuscatedMemoryManager::func_get_pid(const wchar_t* processName)
{
	if (m_kernel_driver_handle != nullptr)
	{
		OPidPack PidPack;
		RtlZeroMemory(PidPack.name, 1024);
		wcsncpy(PidPack.name, processName, 1024);

		BOOL result = DeviceIoControl(m_kernel_driver_handle,
			OBF_IOCTL_GET_PID,
			&PidPack,
			sizeof(PidPack),
			&PidPack,
			sizeof(PidPack),
			nullptr,
			nullptr);

		if (result == TRUE)
			return PidPack.pid;
		else
			return 0;
	}
	else
		return 0;
}

DWORD64 ObfuscatedMemoryManager::func_get_mod_base_um(const wchar_t* moduleName)
{
	if (m_user_process_handle == nullptr || m_process_id == 0)
		return 0;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_process_id);
	if (snapshot == INVALID_HANDLE_VALUE)
		return 0;

	MODULEENTRY32 module{};
	module.dwSize = sizeof(MODULEENTRY32);

	DWORD64 baseAddress = 0;
	if (Module32First(snapshot, &module))
	{
		do
		{
			if (_wcsicmp(module.szModule, moduleName) == 0)
			{
				baseAddress = reinterpret_cast<DWORD64>(module.modBaseAddr);
				break;
			}
		} while (Module32Next(snapshot, &module));
	}

	CloseHandle(snapshot);
	return baseAddress;
}

bool ObfuscatedMemoryManager::func_attach_target_um(const DWORD pid)
{
	if (pid == 0)
		return false;

	if (m_user_process_handle != nullptr)
	{
		CloseHandle(m_user_process_handle);
		m_user_process_handle = nullptr;
	}

	m_user_process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (m_user_process_handle == nullptr)
		return false;

	m_process_id = pid;
	return true;
}

DWORD64 ObfuscatedMemoryManager::func_get_mod_base(const wchar_t* moduleName)
{
	if (m_kernel_driver_handle != nullptr && m_process_id != 0)
	{
		OModulePack ModulePack;
		DWORD64 address = 0;
		ModulePack.pid = m_process_id;
		ModulePack.baseAddress = address;
		RtlZeroMemory(ModulePack.moduleName, 1024);
		wcsncpy(ModulePack.moduleName, moduleName, 1024);
		BOOL result = DeviceIoControl(m_kernel_driver_handle,
			OBF_IOCTL_GET_MODULE_BASE,
			&ModulePack,
			sizeof(ModulePack),
			&ModulePack,
			sizeof(ModulePack),
			nullptr,
			nullptr);
		if (result == TRUE)
			return ModulePack.baseAddress;
		else
			return 0;
	}
	else
		return 0;
}

/*
DWORD64 ObfuscatedMemoryManager::TraceAddress(DWORD64 baseAddress, std::vector<DWORD> offsets)
{
    if (m_kernel_driver_handle == nullptr || m_process_id == 0)
        return 0;

    if (baseAddress == 0 || baseAddress >= 0x7FFFFFFFFFFF)
        return 0;

    uint64_t address = baseAddress;
    if (offsets.empty())
        return baseAddress;

    uint64_t buffer = 0;
    if (!func_read(address, buffer))
        return 0;

    for (size_t i = 0; i < offsets.size() - 1; i++)
    {
        if (buffer == 0 || buffer >= 0x7FFFFFFFFFFF)
            return 0;

        address = buffer + offsets[i];

        if (address < buffer)
            return 0;

        if (!func_read(address, buffer))
            return 0;
    }

    if (buffer == 0 || buffer >= 0x7FFFFFFFFFFF)
        return 0;

    uint64_t finalAddress = buffer + offsets.back();
    return (finalAddress < buffer) ? 0 : finalAddress; // Check overflow
}
*/

bool ObfuscatedMemoryManager::func_batch_read(const std::vector<std::pair<DWORD64, SIZE_T>>& requests, void* output_buffer) {
	if (requests.empty() || m_process_id == 0) {
		return false;
	}

	if (m_kernel_driver_handle != nullptr) {
		// Calculate buffer size for output data only
		SIZE_T output_data_size = 0;
		for (const auto& req : requests) {
			output_data_size += req.second;
		}

		// Calculate total request structure size
		SIZE_T request_struct_size = sizeof(OBatchReadHeader) +
			(requests.size() * sizeof(OBatchReadRequest));

		// Total size includes both request structure and output buffer space
		SIZE_T total_buffer_size = request_struct_size + output_data_size;

		// Allocate buffer for the entire operation
		std::vector<BYTE> operation_buffer(total_buffer_size);

		OBatchReadHeader* header = reinterpret_cast<OBatchReadHeader*>(operation_buffer.data());
		OBatchReadRequest* batch_requests = reinterpret_cast<OBatchReadRequest*>(header + 1);

		// Fill header
		header->process_id = ULongToHandle(m_process_id);
		header->num_requests = static_cast<UINT32>(requests.size());
		header->total_buffer_size = output_data_size; // Size of output data only

		// Fill requests with correct offsets
		SIZE_T buffer_offset = 0;
		for (size_t i = 0; i < requests.size(); ++i) {
			batch_requests[i].address = requests[i].first;
			batch_requests[i].size = requests[i].second;
			batch_requests[i].offset_in_buffer = buffer_offset;
			buffer_offset += requests[i].second;
		}

		BOOL result = DeviceIoControl(
			m_kernel_driver_handle,
			OBF_IOCTL_BATCH_READ,
			operation_buffer.data(),
			static_cast<DWORD>(total_buffer_size),
			operation_buffer.data(),
			static_cast<DWORD>(total_buffer_size),
			nullptr,
			nullptr
		);

		if (result) {
			// Copy output data (starts after the request structures)
			BYTE* output_start = operation_buffer.data() + request_struct_size;
			memcpy(output_buffer, output_start, output_data_size);
		}

		return result == TRUE;
	}

	if (m_user_process_handle != nullptr) {
		BYTE* output_bytes = static_cast<BYTE*>(output_buffer);
		SIZE_T buffer_offset = 0;
		for (const auto& req : requests) {
			SIZE_T bytesRead = 0;
			if (!ReadProcessMemory(m_user_process_handle, reinterpret_cast<LPCVOID>(req.first), output_bytes + buffer_offset, req.second, &bytesRead) || bytesRead != req.second) {
				return false;
			}
			buffer_offset += req.second;
		}
		return true;
	}

	return false;
}