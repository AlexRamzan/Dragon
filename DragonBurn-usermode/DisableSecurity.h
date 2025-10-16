#include <Windows.h>
#include <iostream>
#include <string>
#include "Helpers/Logger.h"

namespace DisableSecurity
{
    bool DisableVulnerableDriverBlocklist()
    {
        Log::Info("Disabling vulnerable driver blocklist...");

        // Disable Virtualization-Based Security
        Log::Info("Disabling Virtualization-Based Security...");
        HKEY hKey;
        DWORD dwValue = 0;

        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\DeviceGuard", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
        {
            if (RegSetValueExW(hKey, L"EnableVirtualizationBasedSecurity", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue)) == ERROR_SUCCESS)
            {
                Log::Fine("Virtualization-Based Security disabled");
            }
            else
            {
                Log::Warning("Failed to disable Virtualization-Based Security");
            }
            RegCloseKey(hKey);
        }
        else
        {
            Log::Warning("Failed to open DeviceGuard registry key");
        }

        // Disable Hypervisor Launch
        Log::Info("Disabling hypervisor launch...");
        DWORD exitCode;
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        std::wstring command = L"bcdedit /set hypervisorlaunchtype off";

        if (CreateProcessW(NULL, (LPWSTR)command.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        {
            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            if (exitCode == 0)
            {
                Log::Fine("Hypervisor launch disabled");
            }
            else
            {
                Log::Warning("Failed to disable hypervisor launch (exit code: " + std::to_string(exitCode) + ")");
            }
        }
        else
        {
            Log::Warning("Failed to execute bcdedit command");
        }

        // Disable Vulnerable Driver Blocklist (main issue)
        Log::Info("Disabling vulnerable driver blocklist...");
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\CI\\Config", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
        {
            if (RegSetValueExW(hKey, L"VulnerableDriverBlocklistEnable", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue)) == ERROR_SUCCESS)
            {
                Log::Fine("Vulnerable driver blocklist disabled successfully");
                RegCloseKey(hKey);
                return true;
            }
            else
            {
                Log::Warning("Failed to disable vulnerable driver blocklist");
            }
            RegCloseKey(hKey);
        }
        else
        {
            Log::Warning("Failed to open CI\\Config registry key");
        }

        return false;
    }

    bool RequiresRestart()
    {
        Log::Info("Checking if system restart is required...");

        HKEY hKey;
        DWORD dwValue, dwSize = sizeof(dwValue);

        // Check if Virtualization-Based Security is enabled
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\DeviceGuard", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
            if (RegQueryValueExW(hKey, L"EnableVirtualizationBasedSecurity", 0, NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
            {
                if (dwValue != 0)
                {
                    RegCloseKey(hKey);
                    Log::Warning("Virtualization-Based Security is still enabled, restart required");
                    return true;
                }
            }
            RegCloseKey(hKey);
        }

        // Check if Vulnerable Driver Blocklist is enabled
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\CI\\Config", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
            if (RegQueryValueExW(hKey, L"VulnerableDriverBlocklistEnable", 0, NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
            {
                if (dwValue != 0)
                {
                    RegCloseKey(hKey);
                    Log::Warning("Vulnerable driver blocklist is still enabled, restart required");
                    return true;
                }
            }
            RegCloseKey(hKey);
        }

        Log::Fine("No restart required");
        return false;
    }
}
