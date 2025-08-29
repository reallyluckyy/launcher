#include "stdinc.hpp"
#include "mod.hpp"

#include "resources.hpp"
#include "ui.hpp"
#include "ui/status.hpp"
#include "ui/footer.hpp"
#include "process.hpp"

namespace mod
{
    std::string current_mod_name;
    LauncherState launcher_state = LauncherState::Initial;

    bool use_iwxmvm = true;

    std::array<std::string, 8> supported_mods = {
        "IW1MVM",
        "IW3MVM",
        "IW4MVM",
        "IW5MVM",
        "WAWMVM",
        "BO1MVM",
        "IW3D",
        "IW3MAP"};

    LauncherState get_launcher_state() 
    {
        return launcher_state;
    }

    bool is_mod_supported(std::string mod)
    {
        for (std::string s : supported_mods)
        {
            if (s.compare(mod) == 0)
                return true;
        }

        return false;
    }

    bool mod_requires_key()
    {
        switch (util::string_hash(current_mod_name.c_str()))
        {
        case util::string_hash("IW1MVM"):
            return true;
        case util::string_hash("IW3D"):
            return true;
        case util::string_hash("IW3MAP"):
            return true;
        }

        return false;
    }

    std::array<std::string, 8> get_supported_mods()
    {
        return supported_mods;
    }

    std::string get_current_mod()
    {
        return current_mod_name;
    }

    std::string get_mod_path()
    {
        return util::format("%s/codmvm_launcher/%s/%s_CLIENT.dll", getenv("APPDATA"), current_mod_name.c_str(), current_mod_name.c_str());
    }

    void toggle_iwxmvm() 
    {
        logger::debug(util::format("switching iwxmvm mode to %u", !use_iwxmvm));

        use_iwxmvm = !use_iwxmvm;
    }

    bool is_iwxmvm()
    {
        return use_iwxmvm;
    }

    std::vector<std::string> get_supported_process_names()
    {
        switch (util::string_hash(current_mod_name.c_str()))
        {
            case util::string_hash("IW1MVM"):
                return {"CoDMP.exe", "CoDUOMP.exe"};
            case util::string_hash("IW3MVM"):
                return {"iw3mp.exe", "iw3xo.exe"};
            case util::string_hash("IW4MVM"):
                return {"iw4x.exe", "iw4m.exe"};
            case util::string_hash("IW5MVM"):
                return {"iw5mp.exe", "plutonium-bootstrapper-win32.exe"};
            case util::string_hash("BO1MVM"):
                return {"BlackOpsMP.exe", "plutonium-bootstrapper-win32.exe"};
            case util::string_hash("WAWMVM"):
                return {"CoDWaWmp.exe", "plutonium-bootstrapper-win32.exe"};
            case util::string_hash("IW3D"):
                return {"iw3mp.exe"};
            case util::string_hash("IW3MAP"):
                return {"iw3mp.exe"};
        }

        return {};
    }

    std::string get_local_hash()
    {
        return util::compute_md5(get_mod_path());
    }

    std::string cached_server_hash;
    std::string get_server_hash()
    {
        if (!cached_server_hash.empty())
            return cached_server_hash;

        std::string mod_name_lowercase = current_mod_name;
        std::transform(mod_name_lowercase.begin(), mod_name_lowercase.end(), mod_name_lowercase.begin(), ::tolower);

        try
        {
            httplib::Client client("https://codmvm.com");
            auto path = util::format("/data/mods/%s%s_hash.php",
                mod::is_iwxmvm() ? "iwxmvm/" : "",
                mod_name_lowercase.c_str()
            );
            logger::debug(util::format("sending request to %s", path.c_str()));
            auto response = client.Get(path.c_str());

            if (!response)
                throw std::logic_error(util::format("httplib error %s", httplib::to_string(response.error()).c_str()));

            if (response->status != 200)
                throw std::logic_error(util::format("http status %d", response->status));

            cached_server_hash = response->body;
            return response->body;
        }
        catch (std::exception &e)
        {
            logger_error(util::format("failed to get server hash for %s: %s", current_mod_name.c_str(), e.what()));
            return "";
        }
    }

    UpdateStatus check_update()
    {
        std::string server_hash = get_server_hash();
        if (server_hash.empty())
            return UpdateStatus::CheckFailed;
        std::string local_hash = get_local_hash();

        logger::debug(util::format("server hash: %s", server_hash.c_str()));
        logger::debug(util::format("local hash: %s", local_hash.c_str()));

        return server_hash.compare(local_hash) == 0 ? UpdateStatus::UpToDate : UpdateStatus::UpdateAvailable;
    }

    bool download_ffmpeg() 
    {
        const auto& ffmpegPath = util::format("%s/codmvm_launcher/ffmpeg.exe", getenv("APPDATA"));
        if (util::file_exists(ffmpegPath))
            return true;

        logger::debug("downloading ffmpeg");

        try
        {
            httplib::Client client("https://codmvm.com");
            auto path = util::format("/data/iwxmvm/ffmpeg.exe");
            logger::debug(util::format("sending request to %s", path.c_str()));
            auto response = client.Get(path.c_str());

            if (!response)
                throw std::logic_error(util::format("httplib error %s", httplib::to_string(response.error()).c_str()));

            if (response->status != 200)
                throw std::logic_error(util::format("http status %d", response->status));

            logger::debug("writing ffmpeg exe");

            if (!util::write_file_binary(ffmpegPath, response->body.c_str(), response->body.size()))
            {
                throw std::logic_error(util::format("failed to write file to %s (%s)", ffmpegPath.c_str(), strerror(errno)));
                return false;
            }

            logger::debug("successfully downloaded mod");

            return true;
        }
        catch (std::exception &e)
        {
            logger_error(util::format("failed to download ffmpeg: %s", e.what()));

            // small fake-load since connection immediately fails if offline or blocked etc
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            return false;
        }
    }

    bool download()
    {
        if (mod::is_iwxmvm())
        {
            if (!download_ffmpeg()) 
            {
                return false;
            }
        }

        logger::debug("requesting mod download");

        try
        {
            httplib::Client client("https://codmvm.com");
            auto path = util::format("/data/mods/%s%s.dll", 
                mod::is_iwxmvm() ? "iwxmvm/" : "",
                current_mod_name.c_str()
            );
            logger::debug(util::format("sending request to %s", path.c_str()));
            auto response = client.Get(path.c_str());

            if (!response)
                throw std::logic_error(util::format("httplib error %s", httplib::to_string(response.error()).c_str()));

            if (response->status != 200)
                throw std::logic_error(util::format("http status %d", response->status));

            logger::debug("writing mod dll");

            if (!util::write_file_binary(get_mod_path(), response->body.c_str(), response->body.size()))
            {
                throw std::logic_error(util::format("failed to write file to %s (%s)", get_mod_path().c_str(), strerror(errno)));
                return false;
            }

            logger::debug("checking hash...");
            std::string local_hash = get_local_hash();
            std::string server_hash = get_server_hash();
            if (local_hash.compare(server_hash) != 0)
            {
                throw std::logic_error(util::format("hash after download did not match server hash (%s/%s)", local_hash.c_str(), server_hash.c_str()));
                return false;
            }

            logger::debug("successfully downloaded mod");

            return true;
        }
        catch (std::exception &e)
        {
            logger_error(util::format("failed to download mod %s: %s", current_mod_name.c_str(), e.what()));

            // small fake-load since connection immediately fails if offline or blocked etc
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            return false;
        }
    }

    KeyCheckStatus submit_key(std::string key)
    {
        logger::debug("submitting key " + key);

        std::string mod_name_lowercase = current_mod_name;
        std::transform(mod_name_lowercase.begin(), mod_name_lowercase.end(), mod_name_lowercase.begin(), ::tolower);

        try
        {
            httplib::Client client("https://codmvm.com");

            // NOTE: No mods are locked behind key checks as of august 2025. Eventually, this should just all be cleaned up
            auto response = client.Get(util::format("/data/mods/%s_login.php", mod_name_lowercase.c_str()).c_str());

            if (!response)
                throw std::logic_error(util::format("httplib error %s", httplib::to_string(response.error()).c_str()));

            if (response->status != 200)
                throw std::logic_error(util::format("http status %d", response->status));

            if (util::starts_with(response->body, "ERR_"))
            {
                logger_error(util::format("invalid key entered %s", key.c_str()));
                logger_error(util::format("response = %s", response->body.c_str()));
                return KeyCheckStatus::InvalidKey;
            }

            logger::debug("writing mod dll");

            if (!util::write_file_binary(get_mod_path(), response->body.c_str(), response->body.size()))
            {
                throw std::logic_error(util::format("failed to write file: %s", strerror(errno)));
            }

            logger::debug("successfully downloaded mod");

            return KeyCheckStatus::Success;
        }
        catch (std::exception &e)
        {
            logger_error(util::format("failed to download mod %s: %s", current_mod_name.c_str(), e.what()));

            return KeyCheckStatus::DownloadFailed;
        }
    }

    void download_or_update_mod(bool force_check_for_update) 
    {
        if (!util::file_exists(get_mod_path()))
        {
            logger::debug("downloading mod since it doesnt exist");
            status::show("DOWNLOADING MOD");

            std::thread([]()
                        {
                            auto result = download();
                            if (result)
                                PostMessageA(ui::window_handle, WM_MOD_DOWNLOAD_SUCCESS, MP_MOD_INITIAL_DOWNLOAD, 0);
                            else
                                PostMessageA(ui::window_handle, WM_MOD_DOWNLOAD_FAILED, MP_MOD_INITIAL_DOWNLOAD, 0);
                        })
                .detach();
        }
        else
        {
            if (force_check_for_update) {
                cached_server_hash = "";
            }

            logger::debug("checking for updates");
            status::show("CHECKING FOR UPDATES");

            std::thread([]()
                        {
                            UpdateStatus updateStatus = check_update();
                            PostMessageA(ui::window_handle, WM_MOD_UPDATE_STATUS, (WPARAM)updateStatus, 0);
                        })
                .detach();
        }
    }

    void load(std::string name)
    {
        launcher_state = LauncherState::Loading;

        if (name.compare(current_mod_name) != 0)
        {
            logger::debug("switching to mod " + name);
            if (!is_mod_supported(name))
            {
                logger_error("tried to load unknown or unsupported mod " + name);
                return;
            }

            current_mod_name = name;
            cached_server_hash.clear();
        }
        else
        {
            logger::debug("reloading mod " + name);
        }

        if (mod_requires_key())
        {
            footer::set_show_key_input(true);
            return;
        }

        download_or_update_mod(false);
    }

    int lastProcessIndex = 0;
    std::chrono::time_point<std::chrono::steady_clock> lastStatusChange;
    void set_process_status_display()
    {
        try {
            std::string process_name = get_supported_process_names().at(lastProcessIndex);
            std::transform(process_name.begin(), process_name.end(), process_name.begin(), ::toupper);

            if (process_name.size() > 12)
            {
                process_name.resize(12);
                process_name = process_name + "...";
            }

            status::show("WAITING FOR GAME (" + process_name + ")");
            logger::debug("waiting for " + process_name);
        }
        catch (std::exception& ex) 
        {
            logger_error(util::format("failed to update process status display %s", ex.what()));
        }
    }

    std::optional<std::string> found_process = std::nullopt;
    void update_game_process_state()
    {
        found_process = std::nullopt;
        for (const auto& process_name : mod::get_supported_process_names()) 
        {
            if (process::is_running(process_name)) 
            {
                found_process = process_name;
            }
        }

        if (launcher_state == LauncherState::WaitingForGame && found_process.has_value())
        {
            launcher_state = LauncherState::GameFound;
            logger::debug("game found!");
            status::hide();

            footer::set_show_load_button(true);
        }
        else if (launcher_state == LauncherState::Injected && !found_process.has_value())
        {
            // wait for game again after game was closed
            wait_for_game();
        }
        else if (launcher_state == LauncherState::WaitingForGame && !found_process.has_value())
        {
            if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastStatusChange) > std::chrono::seconds(1))
            {
                const auto& process_names = mod::get_supported_process_names();
                lastStatusChange = std::chrono::steady_clock::now();
                lastProcessIndex = (lastProcessIndex + 1) % process_names.size();

                set_process_status_display();
            }
        }
    }

    void wait_for_game()
    {
        lastProcessIndex = 0;
        set_process_status_display();

        launcher_state = LauncherState::WaitingForGame;
    }

    void inject()
    {
        status::show("LOADING MOD");

        if (!util::file_exists(get_mod_path()))
        {
            logger_error("mod dll not found when trying to inject");
            status::exception("MOD NOT FOUND", status::ContinueOption::Retry);
            return;
        }

        if (!found_process.has_value() || !process::is_running(found_process.value()))
        {
            logger_error("game process was not found when trying to inject");
            status::exception("GAME NOT FOUND", status::ContinueOption::Retry);
            return;
        }

        if (found_process.value().find("plutonium") != std::string::npos)
        {
            auto result = MessageBoxA(GetActiveWindow(), "You're about to load a CODMVM mod into Plutonium.\nPlease make sure your game is in LAN Mode with disabled anticheat, otherwise you might get banned.\n\nPlutonium is also not officially supported, which is why you might experience crashes or other issues. Good luck!\n\nPress OK if you're sure you want to continue.", "WARNING", MB_OKCANCEL);
            if (result != IDOK)
            {
                mod::load(get_current_mod());
                return;
            }
        }

        if (found_process.value().find("iw4x") != std::string::npos)
        {
            auto result = MessageBoxA(GetActiveWindow(), "You're about to load a CODMVM mod into IW4X.\nPlease make sure you do not join any servers with the mod loaded, otherwise you might be banned.\n\nIW4X is also not officially supported, which is why you might experience crashes or other issues. Good luck!\n\nPress OK if you're sure you want to continue.", "WARNING", MB_OKCANCEL);
            if (result != IDOK)
            {
                mod::load(get_current_mod());
                return;
            }
        }

        if (process::inject(found_process.value(), get_mod_path()))
        {
            launcher_state = LauncherState::Injected;
            logger::debug("injected mod!");
            status::show("SUCCESSFULLY LOADED MOD", false);
        }
    }
}