#define NK_GDIP_IMPLEMENTATION
#define NK_IMPLEMENTATION
#include "../include/nuklear_gdip.h"

#include "stdinc.hpp"
#include "ui.hpp"

#include "resources.hpp"
#include "mod.hpp"
#include "updater.hpp"

#include "ui/colors.hpp"
#include "ui/styles.hpp"
#include "ui/fonts.hpp"
#include "ui/titlebar.hpp"
#include "ui/home.hpp"
#include "ui/modpage.hpp"
#include "ui/footer.hpp"
#include "ui/status.hpp"
#include "ui/modselect.hpp"

namespace ui
{
    bool is_showing_home = true;

    GdipFont *font;
    struct nk_context *context;
    HWND window_handle;
    WNDCLASSA window_class;

    static LRESULT CALLBACK WindowProc(HWND window_handle, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_MOD_DOWNLOAD_SUCCESS:
            mod::wait_for_game();
            return 0;

        case WM_MOD_DOWNLOAD_FAILED:
            if (wparam == MP_MOD_INITIAL_DOWNLOAD)
            {
                // theres no way to continue from here since the mod isnt
                // already downloaded and cant be downloaded either
                status::exception("DOWNLOAD FAILED", status::ContinueOption::Retry);
            }
            else if (wparam == MP_MOD_UPDATE_DOWNLOAD)
            {
                status::exception("UPDATE FAILED", status::ContinueOption::ContinueInOfflineMode);
            }
            return 0;

        case WM_MOD_UPDATE_STATUS:
        {
            mod::UpdateStatus modUpdateStatus = (mod::UpdateStatus)wparam;
            if (modUpdateStatus == mod::UpdateStatus::CheckFailed)
            {
                // we have a dll either way, whether the update failed or not
                // so we'll let the user "skip" the exception and wait_for_game()
                status::exception("NO CONNECTION", status::ContinueOption::ContinueInOfflineMode);
            }
            else if (modUpdateStatus == mod::UpdateStatus::UpdateAvailable)
            {
                logger::debug("downloading update");
                status::show("DOWNLOADING UPDATE");

                std::thread([]()
                            {
                                auto result = mod::download();
                                if (result)
                                    PostMessageA(ui::window_handle, WM_MOD_DOWNLOAD_SUCCESS, MP_MOD_UPDATE_DOWNLOAD, 0);
                                else
                                    PostMessageA(ui::window_handle, WM_MOD_DOWNLOAD_FAILED, MP_MOD_UPDATE_DOWNLOAD, 0);
                            })
                    .detach();
            }
            else if (modUpdateStatus == mod::UpdateStatus::UpToDate)
            {
                mod::wait_for_game();
            }
            return 0;
        }

        case WM_LAUNCHER_UPDATE_STATUS:
        {
            updater::UpdateStatus launcherUpdateStatus = (updater::UpdateStatus)wparam;
            if (launcherUpdateStatus == updater::UpdateStatus::CheckFailed)
            {
                status::exception("NO CONNECTION:\nFAILED TO CHECK FOR UPDATES", status::ContinueOption::None);
                modselect::enable();
            }
            else if (launcherUpdateStatus == updater::UpdateStatus::UpdateAvailable)
            {
                logger::debug("downloading update");
                status::show("DOWNLOADING UPDATE");

                std::thread([]()
                            {
                                if (updater::update_launcher())
                                    PostMessageA(ui::window_handle, WM_LAUNCHER_DOWNLOAD_SUCCESS, 0, 0);
                                else
                                    PostMessageA(ui::window_handle, WM_LAUNCHER_DOWNLOAD_FAILED, 0, 0);
                            })
                    .detach();
            }
            else
            {
                logger::debug("up to date");
                status::hide();
                modselect::enable();
            }
            return 0;
        }

        case WM_LAUNCHER_DOWNLOAD_SUCCESS:
            status::exception("SUCCESSFULLY UPDATED", status::ContinueOption::None);
            updater::schedule_restart();
            return 0;

        case WM_LAUNCHER_DOWNLOAD_FAILED:
            status::exception("LAUNCHER UPDATE FAILED", status::ContinueOption::None);
            modselect::enable();

            return 0;

        case WM_LAUNCHER_RESTARTING_IN:
            status::exception(util::format("SUCCESSFULLY UPDATED\nRESTARTING IN %i", (int)wparam).c_str(), status::ContinueOption::None);
            return 0;

        case WM_KEY_SUBMIT_STATUS:
        {
            mod::KeyCheckStatus keyCheckStatus = (mod::KeyCheckStatus)wparam;
            if (keyCheckStatus == mod::KeyCheckStatus::InvalidKey)
            {
                status::exception("INVALID KEY", status::ContinueOption::Retry);
            }
            else if (keyCheckStatus == mod::KeyCheckStatus::Success)
            {
                mod::wait_for_game();
            }
            else if (keyCheckStatus == mod::KeyCheckStatus::DownloadFailed)
            {
                status::exception("DOWNLOAD FAILED", status::ContinueOption::Retry);
            }

            return 0;
        }
        }

        if (nk_gdip_handle_event(window_handle, msg, wparam, lparam))
            return 0;

        return DefWindowProcA(window_handle, msg, wparam, lparam);
    }

    HWND create_window()
    {
        RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        DWORD exstyle = WS_EX_APPWINDOW;

        memset(&window_class, 0, sizeof(window_class));
        window_class.style = CS_DBLCLKS;
        window_class.lpfnWndProc = WindowProc;
        window_class.hInstance = GetModuleHandle(0);
        window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
        window_class.lpszClassName = "CodmvmLauncher";
        RegisterClass(&window_class);

        return CreateWindowExA(exstyle, window_class.lpszClassName, "CODMVM LAUNCHER",
                               WS_POPUP | WS_VISIBLE, 400, 200,
                               rect.right - rect.left, rect.bottom - rect.top,
                               NULL, NULL, window_class.hInstance, NULL);
    }

    bool handle_events()
    {
        MSG msg;
        nk_input_begin(context);

        if (GetMessage(&msg, nullptr, 0, 0) == 0)
            return 0;

        if (msg.message == WM_QUIT)
            return 0;

        TranslateMessage(&msg);
        DispatchMessage(&msg);

        nk_input_end(context);

        return 1;
    }

    void init()
    {
        logger::debug("Initializing UI");

        window_handle = create_window();

        context = nk_gdip_init(window_handle, WINDOW_WIDTH, WINDOW_HEIGHT);

        styles::init(context);
        fonts::init(context);
    }

    std::map<std::string, struct nk_image> image_cache;
    struct nk_image load_image(std::string path)
    {
        if (image_cache.find(path) != image_cache.end()) 
        {
            return image_cache[path];
        } 
        else 
        {
            auto image_data = resources::get_file(path);
            auto image = nk_gdip_load_image_from_memory(image_data.data(), image_data.size());
            image_cache.insert(std::pair<std::string, struct nk_image>(path, image));
            return image;
        }
    }

    void draw()
    {
        if (nk_begin(context, "CODMVM LAUNCHER", nk_rect(-10, -1, WINDOW_WIDTH + 20, WINDOW_HEIGHT + 1), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND))
        {
            nk_layout_space_begin(context, NK_STATIC, WINDOW_HEIGHT, INT_MAX);

            titlebar::draw(context);

            if (is_showing_home)
                home::draw(context);
            else
                modpage::draw(context);

            footer::draw(context);

            nk_layout_space_end(context);
        }
        nk_end(context);

        nk_gdip_render(NK_ANTI_ALIASING_ON, colors::pink);
    }

    void shutdown()
    {
        nk_gdipfont_del(font);
        nk_gdip_shutdown();
        UnregisterClass(window_class.lpszClassName, window_class.hInstance);
    }

    void set_show_home(bool value)
    {
        is_showing_home = value;
    }

    bool get_show_home()
    {
        return is_showing_home;
    }
}