#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <intrin.h>

#define OBF_DEVICE 0x8000
#define OBF_IOCTL_ATTACH CTL_CODE(OBF_DEVICE, 0x4452, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define OBF_IOCTL_READ CTL_CODE(OBF_DEVICE, 0x4453, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define OBF_IOCTL_GET_MODULE_BASE CTL_CODE(OBF_DEVICE, 0x4454, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define OBF_IOCTL_GET_PID CTL_CODE(OBF_DEVICE, 0x4455, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define OBF_IOCTL_BATCH_READ CTL_CODE(OBF_DEVICE, 0x4456, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
//#define IOCTL_WRITE CTL_CODE(DRAGON_DEVICE, 0x4457, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

class ObfuscatedMemoryManager
{
public:
    ObfuscatedMemoryManager();
    ~ObfuscatedMemoryManager();
    bool func_init_comm(const LPCWSTR name);
    bool func_end_comm();
    bool func_attach_target(const DWORD pid);
    bool func_attach_target_um(const DWORD pid);
    DWORD64 func_get_mod_base(const wchar_t* moduleName);
    DWORD64 func_get_mod_base_um(const wchar_t* moduleName);
    DWORD func_get_pid(const wchar_t* processName);
    DWORD func_get_pid_um(const wchar_t* processName);
    //DWORD64 TraceAddress(DWORD64 baseAddress, std::vector<DWORD> offsets);
    bool func_batch_read(const std::vector<std::pair<DWORD64, SIZE_T>>& requests, void* output_buffer);

    template <typename ReadType>
    bool func_read_um(DWORD64 address, ReadType& value, SIZE_T size = sizeof(ReadType))
    {
        if (m_user_process_handle == nullptr || m_process_id == 0)
            return false;

        SIZE_T bytesRead = 0;
        BOOL result = ReadProcessMemory(m_user_process_handle, reinterpret_cast<LPCVOID>(address), &value, size, &bytesRead);
        return result == TRUE && bytesRead == size;
    }

    template <typename ReadType>
    bool func_read(DWORD64 address, ReadType& value, SIZE_T size = sizeof(ReadType))
    {
        if (m_kernel_driver_handle != nullptr && m_process_id != 0)
        {
            if (address == 0 || address >= 0x7FFFFFFFFFFF || size == 0 || size > 0x1000) {
                return false;
            }

            if (address + size < address) {
                return false;
            }

            ORequest readRequest;
            readRequest.process_id = ULongToHandle(m_process_id);
            readRequest.target = reinterpret_cast<PVOID>(address);
            readRequest.buffer = &value;
            readRequest.size = size;

            BOOL result = DeviceIoControl(m_kernel_driver_handle,
                OBF_IOCTL_READ,
                &readRequest,
                sizeof(readRequest),
                &readRequest,
                sizeof(readRequest),
                nullptr,
                nullptr);
            return result == TRUE;
        }
        if (m_user_process_handle != nullptr && m_process_id != 0)
        {
            return func_read_um(address, value, size);
        }
        return false;
    }

    template<typename T>
    bool func_batch_read_struct(const std::vector<DWORD64>& addresses, std::vector<T>& results) {
        if (addresses.empty()) return false;

        std::vector<std::pair<DWORD64, SIZE_T>> requests;
        requests.reserve(addresses.size());

        for (DWORD64 addr : addresses) {
            requests.emplace_back(addr, sizeof(T));
        }

        results.resize(addresses.size());
        return func_batch_read(requests, results.data());
    }

    // Ultra-robust helpers
    template<typename T>
    bool func_read_retry(DWORD64 address, T& value, int maxRetries = 5, DWORD initialSleepMs = 1)
    {
        for (int attempt = 0; attempt < maxRetries; ++attempt)
        {
            if (func_read(address, value))
                return true;
            Sleep(initialSleepMs << attempt);
        }
        return false;
    }

    bool func_read_buffer_retry(DWORD64 address, void* buffer, SIZE_T size, int maxRetries = 5, DWORD initialSleepMs = 1);

    // User-mode write with protection + retries (driver write not available)
    bool func_write_um(DWORD64 address, const void* buffer, SIZE_T size, int maxRetries = 5, DWORD initialSleepMs = 1);

    // Convenience string readers (user-mode path)
    std::string func_read_str_um(DWORD64 address, SIZE_T maxLength = 256, int maxRetries = 3);
    std::wstring func_read_wstr_um(DWORD64 address, SIZE_T maxLength = 256, int maxRetries = 3);

    // Legacy API compatibility (preserve original names)
    inline bool ConnectDriver(const LPCWSTR name) { return func_init_comm(name); }
    inline bool DisconnectDriver() { return func_end_comm(); }
    inline bool Attach(const DWORD pid) { return func_attach_target(pid); }
    inline bool Attach_UserMode(const DWORD pid) { return func_attach_target_um(pid); }
    inline DWORD64 GetModuleBase(const wchar_t* moduleName) { return func_get_mod_base(moduleName); }
    inline DWORD64 GetModuleBase_UserMode(const wchar_t* moduleName) { return func_get_mod_base_um(moduleName); }
    inline DWORD GetProcessID(const wchar_t* processName) { return func_get_pid(processName); }
    inline DWORD GetProcessID_UserMode(const wchar_t* processName) { return func_get_pid_um(processName); }
    inline bool BatchReadMemory(const std::vector<std::pair<DWORD64, SIZE_T>>& requests, void* output_buffer) { return func_batch_read(requests, output_buffer); }

    template <typename ReadType>
    inline bool ReadMemory_UserMode(DWORD64 address, ReadType& value, SIZE_T size = sizeof(ReadType)) { return func_read_um(address, value, size); }

    template <typename ReadType>
    inline bool ReadMemory(DWORD64 address, ReadType& value, SIZE_T size = sizeof(ReadType)) { return func_read(address, value, size); }

    template<typename T>
    inline bool BatchReadStructured(const std::vector<DWORD64>& addresses, std::vector<T>& results) { return func_batch_read_struct(addresses, results); }

    template<typename T>
    inline bool ReadMemoryWithRetry(DWORD64 address, T& value, int maxRetries = 5, DWORD initialSleepMs = 1) { return func_read_retry(address, value, maxRetries, initialSleepMs); }

    inline bool ReadBufferWithRetry(DWORD64 address, void* buffer, SIZE_T size, int maxRetries = 5, DWORD initialSleepMs = 1) { return func_read_buffer_retry(address, buffer, size, maxRetries, initialSleepMs); }

    inline bool WriteMemory_UserMode(DWORD64 address, const void* buffer, SIZE_T size, int maxRetries = 5, DWORD initialSleepMs = 1) { return func_write_um(address, buffer, size, maxRetries, initialSleepMs); }

    inline std::string ReadString_UserMode(DWORD64 address, SIZE_T maxLength = 256, int maxRetries = 3) { return func_read_str_um(address, maxLength, maxRetries); }
    inline std::wstring ReadWString_UserMode(DWORD64 address, SIZE_T maxLength = 256, int maxRetries = 3) { return func_read_wstr_um(address, maxLength, maxRetries); }

private:
    void junk_func_a() { volatile int x = 0; }
    void chaotic_sleep(int min_ms, int max_ms);
    void junk_func_b(char* p) { if (p) p[0] = '\0'; }

    // Page-aware helpers
    struct VmRegion { DWORD64 base; SIZE_T size; DWORD protect; bool readable; bool writable; };
    bool query_region_user(DWORD64 address, VmRegion& out) const;
    SIZE_T clamp_copy_safe(DWORD64 address, SIZE_T requested) const;
    SIZE_T page_size() const;

    volatile DWORD m_process_id = 0;
    volatile HANDLE m_kernel_driver_handle = nullptr;
    volatile HANDLE m_user_process_handle = nullptr;

    int junk_data_1;
    void* junk_data_2;

    typedef struct _ORequest
    {
        HANDLE process_id;
        PVOID target;
        PVOID buffer;
        SIZE_T size;
    } ORequest, * PORequest;

    typedef struct _OPidPack
    {
        UINT32 pid;
        WCHAR name[1024];
    } OPidPack, * POPidPack;

    typedef struct _OModulePack {
        UINT32 pid;
        UINT64 baseAddress;
        SIZE_T size;
        WCHAR moduleName[1024];
    } OModulePack, * POModulePack;


    // Batch read structures
    struct OBatchReadRequest {
        DWORD64 address;
        SIZE_T size;
        SIZE_T offset_in_buffer; // Offset where this read's data starts in the output buffer
    };

    struct OBatchReadHeader {
        HANDLE process_id;
        UINT32 num_requests;
        SIZE_T total_buffer_size;
        // Followed by BatchReadRequest array, then output buffer
    };
};

inline ObfuscatedMemoryManager g_mem_access_point;
inline ObfuscatedMemoryManager& memoryManager = g_mem_access_point;