#include "stdinc.hpp"
#include "util.hpp"

#include <openssl/md5.h>

namespace util
{
    std::string compute_md5(std::string file_path)
    {
        auto bytes = read_file_binary(file_path);
        if (bytes.empty())
            return "file not found";

        unsigned char result[MD5_DIGEST_LENGTH];
        MD5((unsigned char *)bytes.data(), bytes.size(), result);

        std::stringstream hash;
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            char buf[8];
            sprintf_s(buf, "%02x", result[i]);
            hash << buf;
        }

        return hash.str();
    }

    std::vector<std::string> split(std::string s, std::string delimiter)
    {
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        std::string token;
        std::vector<std::string> res;

        while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
        {
            token = s.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back(token);
        }

        res.push_back(s.substr(pos_start));
        return res;
    }

    // https://stackoverflow.com/questions/27220/how-to-convert-stdstring-to-lpcwstr-in-c-unicode
    std::wstring s2ws(const std::string &s)
    {
        int len;
        int slength = (int)s.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
        wchar_t *buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
        std::wstring r(buf);
        delete[] buf;
        return r;
    }

    bool ends_with(const std::string &str, const std::string &suffix)
    {
        return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
    }

    bool starts_with(const std::string &str, const std::string &prefix)
    {
        return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
    }

    std::string read_file(const std::string &file_name)
    {
        std::ifstream stream(file_name);
        if (!stream.good())
            return "Failed to read file";
        std::stringstream buffer;
        buffer << stream.rdbuf();
        return buffer.str();
    }

    std::vector<std::uint8_t> read_file_binary(const std::string &file_name)
    {
        std::ifstream input(file_name, std::ios::binary);
        if (!input.good())
            return std::vector<std::uint8_t>();

        return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(input), {});
    }

    bool write_file_binary(const std::string &file_name, const char *bytes, std::uint32_t length)
    {
        auto index = file_name.find_last_of('/');
        if (index == std::string::npos)
            index = file_name.find_last_of('\\');
        if (index != std::string::npos)
        {
            std::filesystem::create_directories(file_name.substr(0, index));
        }

        // remove existing file
        if (file_exists(file_name))
        {
            std::remove(file_name.c_str());
        }

        std::ofstream file(file_name, std::ios::binary);
        if (!file.good())
            return false;

        file.write(bytes, length);
        file.close();
        return true;
    }

    bool file_exists(const std::string &file_name)
    {
        std::ifstream file(file_name);
        return file.good();
    }

    std::string get_current_exe_path()
    {
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        return std::string(path);
    }

    bool try_relaunch_unelevated()
    {
        HANDLE token = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
        {
            logger_error(util::format("failed to open process token: %u", GetLastError()));
            return false;
        }

        LUID luid;
        if (!LookupPrivilegeValue(NULL, "SeIncreaseQuotaPrivilege", &luid))
        {
            logger_error(util::format("failed to lookup privilege value: %u", GetLastError()));
            CloseHandle(token);
            return false;
        }

        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if (!AdjustTokenPrivileges(
                token,
                FALSE,
                &tp,
                sizeof(TOKEN_PRIVILEGES),
                (PTOKEN_PRIVILEGES)NULL,
                (PDWORD)NULL))
        {
            logger_error(util::format("failed to adjust privileges: %u", GetLastError()));
            CloseHandle(token);
            return false;
        }

        CloseHandle(token);

        HWND shell_window = GetShellWindow();
        if (!shell_window)
        {
            logger_error(util::format("failed to get shell window: %u", GetLastError()));
            return false;
        }

        DWORD shell_pid;
        if (!GetWindowThreadProcessId(shell_window, &shell_pid))
        {
            logger_error(util::format("failed to get process id: %u", GetLastError()));
            return false;
        }

        HANDLE shell_process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, shell_pid);
        if (!shell_process)
        {
            logger_error(util::format("failed to open process: %u", GetLastError()));
            return false;
        }

        HANDLE shell_token;
        if (!OpenProcessToken(shell_process, TOKEN_DUPLICATE, &shell_token))
        {
            logger_error(util::format("failed to open token: %u", GetLastError()));
            return false;
        }

        HANDLE new_token;
        if (!DuplicateTokenEx(shell_token, 395U, NULL, SecurityImpersonation, TokenPrimary, &new_token))
        {
            logger_error(util::format("failed to duplicate token: %u", GetLastError()));
            return false;
        }

        TOKEN_ELEVATION elevation;
        DWORD size = sizeof(TOKEN_ELEVATION);
        if (!GetTokenInformation(new_token, TokenElevation, &elevation, sizeof(elevation), &size))
        {
            logger_error(util::format("failed to get token information: %u", GetLastError()));
            return false;
        }

        if (elevation.TokenIsElevated)
        {
            logger_error("duplicated token is also elevated");
            MessageBoxA(NULL, "It seems like you have probably disabled UAC.\nThe launcher needs to be run with normal user permissions, so please make sure your UAC setting is set to default and restart your PC.\n\nIf you need more help, contact me on discord.", "ERROR", 1);

            ExitProcess(0);
            return false;
        }

        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcessWithTokenW(new_token, 0, s2ws(get_current_exe_path()).c_str(), L"", 0, NULL, NULL, &si, &pi))
        {
            logger_error(util::format("failed to create process with token: %u", GetLastError()));
            return false;
        }

        CloseHandle(shell_token);
        CloseHandle(new_token);
        CloseHandle(shell_process);

        logger::debug("restarted with user token!");

        return true;
    }

    std::uint32_t get_windows_build_number()
    {
        HKEY key;
        LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &key);
        if (result != ERROR_SUCCESS)
            return 0;

        char buildNumber[512];
        DWORD buildNumberSize = sizeof(buildNumber);
        result = RegQueryValueEx(key, "CurrentBuildNumber", 0, NULL, (LPBYTE)buildNumber, &buildNumberSize);
        if (result != ERROR_SUCCESS)
            strcpy_s(buildNumber, "N/A");

        RegCloseKey(key);

        return std::strtoul(buildNumber, NULL, 10);
    }

    std::string get_windows_version_string()
    {
        HKEY key;
        LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &key);
        if (result != ERROR_SUCCESS)
            return "failed to open registry key";

        char productName[512];
        DWORD productNameSize = sizeof(productName);

        char displayVersion[512];
        DWORD displayVersionSize = sizeof(displayVersion);

        char releaseId[512];
        DWORD releaseIdSize = sizeof(releaseId);

        result = RegQueryValueEx(key, "ProductName", 0, NULL, (LPBYTE)productName, &productNameSize);
        if (result != ERROR_SUCCESS)
            strcpy_s(productName, "N/A");

        result = RegQueryValueEx(key, "DisplayVersion", 0, NULL, (LPBYTE)displayVersion, &displayVersionSize);
        if (result != ERROR_SUCCESS)
            strcpy_s(displayVersion, "N/A");

        result = RegQueryValueEx(key, "ReleaseId", 0, NULL, (LPBYTE)releaseId, &releaseIdSize);
        if (result != ERROR_SUCCESS)
            strcpy_s(releaseId, "N/A");

        RegCloseKey(key);

        return util::format("%s %s (release %s, build %u)", productName, displayVersion, releaseId, get_windows_build_number());
    }

    void open_link(std::string url)
    {
        logger::debug(util::format("opening url %s", url.c_str()));
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}