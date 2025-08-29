#include "stdinc.hpp"

#include "mod.hpp"
#include "resources.hpp"
#include "updater.hpp"
#include "process.hpp"
#include "ui.hpp"
#include "ui/status.hpp"
#include "ui/modselect.hpp"
#include "ui/footer.hpp"

void update_launcher()
{
    modselect::disable();

    logger::debug("checking for updates...");
    status::show("CHECKING FOR UPDATES");

    std::thread([]()
                {
                    updater::UpdateStatus status = updater::check_update();
                    PostMessageA(ui::window_handle, WM_LAUNCHER_UPDATE_STATUS, (WPARAM)status, 0);
                })
        .detach();
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   PSTR lpCmdLine, INT nCmdShow)
{
    logger::debug(util::format("\n\nstarting launcher version %u", updater::CURRENT_VERSION));
    logger::debug(util::format("windows version: %s", util::get_windows_version_string().c_str()));
    bool running_elevated = process::is_elevated(GetCurrentProcess());
    logger::debug(util::format("running as elevated process: %u", running_elevated));

    updater::delete_old_launcher();
    updater::update_shortcut();

    try
    {
        logger::debug("Setting DPI awareness context");
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        ui::init();

        update_launcher();

        while (ui::handle_events())
        {
            ui::draw();
            mod::update_game_process_state();

            if (GetAsyncKeyState(VK_F8))
            {
                logger::upload_log();
            }
        }

        ui::shutdown();
    }
    catch (std::exception &e)
    {
        logger_error(util::format("exception caught while running application: %s", e.what()));
    }
    catch (const std::string &e)
    {
        logger_error(util::format("string exception caught while running application: %s", e.c_str()));
    }
    catch (...)
    {
        logger_error("unhandled exception caught while running application");
    }

    return 0;
}