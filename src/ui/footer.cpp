#include "../../include/nuklear_gdip.h"

#include "../stdinc.hpp"
#include "footer.hpp"

#include "../ui.hpp"
#include "../mod.hpp"
#include "colors.hpp"
#include "styles.hpp"
#include "fonts.hpp"
#include "status.hpp"
#include "modselect.hpp"
#include "keyinput.hpp"

namespace footer
{
    bool show_load_button = false;
    bool show_status = false;
    bool show_key_input = false;

    void set_show_load_button(bool value)
    {
        show_load_button = value;
        if (show_load_button)
        {
            show_status = false;
            show_key_input = false;
        }
    }

    void set_show_status(bool value)
    {
        show_status = value;
        if (show_status)
        {
            show_load_button = false;
            show_key_input = false;
        }
    }

    void set_show_key_input(bool value)
    {
        show_key_input = value;
        if (show_key_input)
        {
            show_load_button = false;
            show_status = false;
        }
    }

    void draw_toggle(struct nk_context *context, int x, int y, const char* text, bool isSelected, std::function<void()> func)
    {
        nk_layout_space_push(context, nk_rect(x, y, INT_MAX, 20));
        ui::fonts::use_font(ui::fonts::medium_font_12);
        nk_label_colored(context, text, NK_TEXT_ALIGN_LEFT, isSelected ? ui::colors::text_color : ui::colors::text_color_faded);

        auto slider_x = x + 8;
        auto slider_y = y + 18;

        nk_layout_space_push(context, nk_rect(slider_x, slider_y, 50, 15));
        if (nk_button_label_styled(context, &ui::styles::slider_button, ""))
        {
            func();
        }

        nk_fill_rect(nk_window_get_canvas(context), nk_rect(slider_x + (isSelected ? 30 : -3), slider_y + 5, 11, 11), 0.0f, ui::colors::dark_hover);
    }

    void draw_mod_info(struct nk_context *context)
    {
        auto start_x = 225;
        auto start_y = (ui::WINDOW_HEIGHT * 0.825);

        if (mod::get_current_mod().compare("IW1MVM") == 0)
        {
            nk_layout_space_push(context, nk_rect(start_x, start_y, INT_MAX, 20));
            ui::fonts::use_font(ui::fonts::medium_font_12);
            nk_label_colored(context, "ENTER LITERALLY ANYTHING!", NK_TEXT_ALIGN_LEFT, ui::colors::text_color);
        }
        else if (mod::get_current_mod().compare("IW3D") == 0 || mod::get_current_mod().compare("IW3MAP") == 0)
        {
            nk_layout_space_push(context, nk_rect(start_x, start_y, INT_MAX, 20));
            ui::fonts::use_font(ui::fonts::medium_font_12);
            nk_label_colored(context, "ENTER LITERALLY ANYTHING!\nATTENTION: REQUIRES COD4 1.7", NK_TEXT_ALIGN_LEFT, ui::colors::text_color);
        }
        else if (mod::get_current_mod().compare("IW3MVM") == 0)
        {
            draw_toggle(context, start_x, start_y, "USE LEGACY", !mod::is_iwxmvm(), []() { mod::toggle_iwxmvm(); mod::download_or_update_mod(true); });
        }
    }

    void draw_social_buttons(struct nk_context* context)
    {
        if (show_status || show_key_input || show_load_button)
            return;

        auto button_size = 42;
        auto button_spacing = 10;
        auto x = ui::WINDOW_WIDTH * 0.70;

        nk_layout_space_push(context, nk_rect(x, ui::WINDOW_HEIGHT * 0.8, button_size, button_size));
        if (nk_button_image_styled(context, &ui::styles::social_button, ui::load_image("resources/images/website.png")))
        {
            util::open_link("https://codmvm.com/");
        }

        x += button_size + button_spacing;

        nk_layout_space_push(context, nk_rect(x, ui::WINDOW_HEIGHT * 0.8, button_size, button_size));
        if (nk_button_image_styled(context, &ui::styles::social_button, ui::load_image("resources/images/discord.png")))
        {
            util::open_link("https://discord.com/invite/NYey3vH");
        }

        x += button_size + button_spacing;

        nk_layout_space_push(context, nk_rect(x, ui::WINDOW_HEIGHT * 0.8, button_size, button_size));
        if (nk_button_image_styled(context, &ui::styles::social_button, ui::load_image("resources/images/twitter.png")))
        {
            util::open_link("https://twitter.com/reallyluckyy");
        }

        x += button_size + button_spacing;

        nk_layout_space_push(context, nk_rect(x, ui::WINDOW_HEIGHT * 0.8, button_size, button_size));
        if (nk_button_image_styled(context, &ui::styles::social_button, ui::load_image("resources/images/kofi.png")))
        {
            util::open_link("https://ko-fi.com/reallyluckyy");
        }
    }

    void draw(struct nk_context *context)
    {
        nk_fill_rect(nk_window_get_canvas(context), nk_rect(-10, ui::WINDOW_HEIGHT * 0.75, ui::WINDOW_WIDTH + 20, (ui::WINDOW_HEIGHT * 0.25) + 2), 0.0f, ui::colors::darkest);

        modselect::draw(context);

        if (!ui::get_show_home())
        {
            draw_mod_info(context);
        } 
        else
        {
            draw_social_buttons(context);
        }

        if (show_load_button)
        {
            auto button_width = ui::WINDOW_WIDTH * 0.2;
            auto button_height = ui::WINDOW_HEIGHT * 0.25 * 0.65;
            auto button_x = ui::WINDOW_WIDTH * 0.88 - button_width * 0.5;
            auto button_y = ui::WINDOW_HEIGHT * 0.87 - button_height * 0.5;

            nk_layout_space_push(context, nk_rect(button_x, button_y, button_width, button_height));
            ui::fonts::use_font(ui::fonts::medium_font_15);
            if (nk_button_label_styled(context, &ui::styles::load_button, "LOAD MOD"))
            {
                mod::inject();
            }
        }
        else if (show_status)
        {
            status::draw(context);
        }
        else if (show_key_input)
        {
            keyinput::draw(context);
        }
    }
}