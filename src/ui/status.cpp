#include "../../include/nuklear_gdip.h"

#include "../stdinc.hpp"
#include "status.hpp"

#include <ctime>

#include "../ui.hpp"
#include "../mod.hpp"
#include "colors.hpp"
#include "styles.hpp"
#include "fonts.hpp"
#include "footer.hpp"

namespace status
{
    std::string current_message = "";

    bool show_loading_animation = false;
    std::string loading_string = "...";
    std::time_t last_update_time = 0;

    bool show_continue_button = false;
    ContinueOption continue_option;

    void show(std::string message, bool loading)
    {
        current_message = message;
        show_loading_animation = loading;

        footer::set_show_status(true);
        show_continue_button = false;
    }

    void hide()
    {
        footer::set_show_status(false);
    }

    void exception(std::string message, ContinueOption option)
    {
        show(message, false);
        if (option != ContinueOption::None)
        {
            show_continue_button = true;
            continue_option = option;
        }
    }

    void draw(struct nk_context *context)
    {
        nk_handle handle = nk_handle();
        handle.ptr = ui::fonts::medium_font_11;
        auto text_width = nk_gdipfont_get_text_width(handle, 0, current_message.c_str(), current_message.size());

        nk_layout_space_push(context, nk_rect(ui::WINDOW_WIDTH * 0.85 - text_width * 0.5, ui::WINDOW_HEIGHT * 0.83, INT_MAX, 50));
        ui::fonts::use_font(ui::fonts::medium_font_11);
        nk_label_colored(context, current_message.c_str(), NK_TEXT_ALIGN_LEFT, ui::colors::text_color);

        if (show_loading_animation)
        {
            auto current_time = std::time(0);
            if (current_time - last_update_time > 0)
            {
                if (loading_string.size() >= 3)
                    loading_string = ".";
                else
                    loading_string += ".";

                last_update_time = current_time;
            }

            nk_layout_space_push(context, nk_rect(ui::WINDOW_WIDTH * 0.84, ui::WINDOW_HEIGHT * 0.86, INT_MAX, 50));
            ui::fonts::use_font(ui::fonts::medium_font_18);
            nk_label_colored(context, loading_string.c_str(), NK_TEXT_ALIGN_LEFT, ui::colors::text_color);
        }
        else if (show_continue_button)
        {
            auto width = 80;

            std::string text;
            if (continue_option == status::ContinueOption::Retry)
            {
                text = "RETRY";
                width = 80;
            }
            else if (continue_option == status::ContinueOption::ContinueInOfflineMode)
            {
                text = "CONTINUE OFFLINE";
                width = 120;
            }

            nk_layout_space_push(context, nk_rect(ui::WINDOW_WIDTH * 0.85 - width * 0.5, ui::WINDOW_HEIGHT * 0.88, width, 20));
            ui::fonts::use_font(ui::fonts::medium_font_11);
            if (nk_button_label_styled(context, &ui::styles::load_button, text.c_str()))
            {
                if (continue_option == status::ContinueOption::Retry)
                {
                    status::hide();

                    mod::load(mod::get_current_mod());
                }
                else if (continue_option == status::ContinueOption::ContinueInOfflineMode)
                {
                    mod::wait_for_game();
                }
            }
        }
    }
}