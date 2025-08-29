#include "stdinc.hpp"
#include "updater.hpp"

#include "ui/status.hpp"
#include "ui.hpp"

namespace updater
{

    updater::UpdateStatus check_update()
    {
        logger::debug("checking for launcher updates...");

        std::uint32_t newest_version = 0;
        try
        {
            httplib::Client client("https://codmvm.com");
            auto response = client.Get("/data/launcher_version.php");

            if (!response)
                throw std::logic_error(util::format("httplib error %s", httplib::to_string(response.error()).c_str()));

            if (response->status != 200)
                throw std::logic_error(util::format("http status %d", response->status));

            newest_version = std::stoul(response->body);

            logger::debug(util::format("server version: %u", newest_version));
            logger::debug(util::format("local version: %u", CURRENT_VERSION));

            return newest_version == CURRENT_VERSION ? updater::UpdateStatus::UpToDate : updater::UpdateStatus::UpdateAvailable;
        }
        catch (std::exception &e)
        {
            logger_error(util::format("failed to get server launcher version %s", e.what()));
        }

        return updater::UpdateStatus::CheckFailed;
    }

    bool update_launcher()
    {
        try
        {
            httplib::Client client("https://codmvm.com");
            auto response = client.Get("/data/CODMVM_LAUNCHER.exe");

            if (!response)
                throw std::logic_error(util::format("httplib error %s", httplib::to_string(response.error()).c_str()));

            if (response->status != 200)
                throw std::logic_error(util::format("http status %d", response->status));

            // rename current exe
            std::string current_path = util::get_current_exe_path();
            if (std::rename(current_path.c_str(), (current_path + "_old").c_str()) != 0)
            {
                throw std::logic_error(util::format("failed to rename exe"));
            }

            // write new exe
            if (!util::write_file_binary(current_path, response->body.c_str(), response->body.size()))
            {
                throw std::logic_error(util::format("failed to write new exe"));
            }

            return true;
        }
        catch (std::exception &e)
        {
            logger_error(util::format("failed to download new launcher: %s", e.what()));
        }

        return false;
    }

    void restart()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        PostMessageA(ui::window_handle, WM_LAUNCHER_RESTARTING_IN, 3, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        PostMessageA(ui::window_handle, WM_LAUNCHER_RESTARTING_IN, 2, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        PostMessageA(ui::window_handle, WM_LAUNCHER_RESTARTING_IN, 1, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        PostMessageA(ui::window_handle, WM_LAUNCHER_RESTARTING_IN, 0, 0);

        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        CreateProcess(util::get_current_exe_path().c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        ExitProcess(0);
    }

    void schedule_restart()
    {
        logger::debug("scheduling restart...");
        std::thread(restart).detach();
    }

    void delete_old_launcher()
    {
        std::thread([]()
                    {
                        while (util::file_exists(util::get_current_exe_path() + "_old"))
                        {
                            logger::debug("attempting to delete old launcher");

                            int return_code = std::remove((util::get_current_exe_path() + "_old").c_str());
                            if (return_code == 0)
                                logger::debug("deleted old launcher");
                            else
                                logger_error(util::format("failed to delete old launcher: %i (%i)", return_code, errno));

                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        }
                    })
            .detach();
    }

    void update_shortcut()
    {
        try
        {
            std::string shortcut_path = getenv("APPDATA");
            shortcut_path += "/Microsoft/Windows/Start Menu/Programs/CODMVM LAUNCHER.lnk";

            char exe_path[MAX_PATH];
            GetModuleFileName(NULL, exe_path, MAX_PATH);

            HRESULT hres;
            hres = CoInitialize(NULL);
            if (FAILED(hres))
            {
                throw std::logic_error("CoInitialize failed");
            }

            IShellLink *psl;
            hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
            if (FAILED(hres))
            {
                throw std::logic_error("CoCreateInstance failed");
            }

            IPersistFile *ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
            if (FAILED(hres))
            {
                throw std::logic_error("QueryInterface failed");
            }

            WCHAR wsz[MAX_PATH];
            MultiByteToWideChar(CP_ACP, 0, shortcut_path.c_str(), -1, wsz, MAX_PATH);

            hres = ppf->Load(wsz, STGM_CREATE | STGM_READ);
            if (FAILED(hres))
            {
                throw std::logic_error("Load failed");
            }

            hres = psl->Resolve(NULL, 0);
            if (FAILED(hres))
            {
                throw std::logic_error("Resolve failed");
            }

            hres = psl->SetPath(exe_path);
            if (FAILED(hres))
            {
                throw std::logic_error("SetPath failed");
            }

            ppf->Save(wsz, NULL);
            if (FAILED(hres))
            {
                throw std::logic_error("Save failed");
            }

            ppf->Release();
            psl->Release();
        }
        catch (std::exception &e)
        {
            logger_error(util::format("failed to update shortcut: %s", e.what()));
        }
    }
}