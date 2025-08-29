#include "stdinc.hpp"
#include "process.hpp"

#include "mod.hpp"
#include "ui/status.hpp"

#include <TlHelp32.h>

namespace process
{
    std::uint32_t get_pid(std::string process_name)
    {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        HANDLE snapshot;
        int counter = 0;
        do
        {
            counter++;
            snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

            if (counter > 128)
            {
                logger_error("get_pid() failed since we exceeded 128 CreateToolhelp32Snapshot calls");
                return -1;
            }
        } while (snapshot == (HANDLE)ERROR_BAD_LENGTH);

        if (Process32First(snapshot, &entry) == TRUE)
        {
            while (Process32Next(snapshot, &entry) == TRUE)
            {
                if (std::string(entry.szExeFile).compare(process_name) == 0)
                {
                    CloseHandle(snapshot);
                    return entry.th32ProcessID;
                }
            }
        }

        CloseHandle(snapshot);
        return -1;
    }

    bool is_elevated(HANDLE process_handle)
    {
        bool result = false;
        HANDLE token = NULL;
        if (OpenProcessToken(process_handle, TOKEN_QUERY, &token))
        {
            TOKEN_ELEVATION elevation;
            DWORD size = sizeof(TOKEN_ELEVATION);
            if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size))
            {
                result = elevation.TokenIsElevated;
            }
            else
            {
                logger_error(util::format("failed to get token information: %u", GetLastError()));
                return false;
            }

            CloseHandle(token);
            return result;
        }
        else
        {
            logger_error(util::format("failed to open token: %u", GetLastError()));
            return false;
        }
    }

    bool inject(std::string process_name, std::string mod_path)
    {
        auto pid = get_pid(process_name);
        logger::debug(util::format("injecting into process %u (%s)", pid, process_name.c_str()));

        HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (process_handle == NULL)
        {
            DWORD error_code = GetLastError();

            logger_error(util::format("OpenProcess returned NULL: %u", error_code));
            status::exception("FAILED TO INJECT", status::ContinueOption::None);
            return false;
        }

        logger::debug(util::format("allocating memory, process handle is %X", process_handle));
        LPVOID dll_path_address = VirtualAllocEx(process_handle, NULL, mod_path.size() + 1, MEM_COMMIT, PAGE_READWRITE);
        if (dll_path_address == NULL)
        {
            logger_error(util::format("VirtualAllocEx returned NULL: %u", GetLastError()));
            status::exception("FAILED TO INJECT", status::ContinueOption::None);
            return false;
        }
        logger::debug(util::format("allocated memory at %X", dll_path_address));

        auto return_value = WriteProcessMemory(process_handle, dll_path_address, mod_path.c_str(), mod_path.size() + 1, NULL);
        if (return_value == NULL)
        {
            logger_error(util::format("WriteProcessMemory returned NULL: %u", GetLastError()));
            status::exception("FAILED TO INJECT", status::ContinueOption::None);
            return false;
        }
        logger::debug("wrote dll path");

        auto load_library_address = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
        if (load_library_address == NULL)
        {
            logger_error(util::format("GetProcAddress returned NULL: %u", GetLastError()));
            status::exception("FAILED TO INJECT", status::ContinueOption::None);
            return false;
        }
        logger::debug(util::format("LoadLibraryA is at %X", load_library_address));

        HANDLE thread_handle = CreateRemoteThread(process_handle, 0, 0, (LPTHREAD_START_ROUTINE)load_library_address, dll_path_address, 0, 0);
        if (thread_handle == NULL)
        {
            logger_error(util::format("CreateRemoteThread returned NULL: %u", GetLastError()));
            status::exception("FAILED TO INJECT", status::ContinueOption::None);
            return false;
        }
        logger::debug(util::format("created thread with handle %X", thread_handle));

        WaitForSingleObject(thread_handle, INFINITE);

        logger::debug("freeing memory for mod path");
        VirtualFreeEx(process_handle, dll_path_address, mod_path.size() + 1, MEM_RELEASE);

        return true;
    }

    bool is_running(std::string process_name)
    {
        auto pid = get_pid(process_name);
        return pid != -1;
    }
}