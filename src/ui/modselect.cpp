#include "../../include/nuklear_gdip.h"

#include "../stdinc.hpp"
#include "modselect.hpp"

#include "../ui.hpp"
#include "../mod.hpp"
#include "../updater.hpp"
#include "colors.hpp"
#include "styles.hpp"
#include "fonts.hpp"
#include "status.hpp"
#include "footer.hpp"

namespace modselect
{
    bool is_enabled = true;

    std::string current_selection = "SELECT A MOD";
    bool is_unfolded = false;

    auto select_width = 192;
    auto select_height = 50;
    auto select_x = 20;
    auto select_y = (ui::WINDOW_HEIGHT * 0.75) + ((ui::WINDOW_HEIGHT * 0.25) - select_height) * 0.48;

    auto entry_height = 25;

    void toggle()
    {
        is_unfolded = !is_unfolded;
    }

    void draw_entries(struct nk_context *context)
    {
        auto mods = mod::get_supported_mods();

        auto y = select_y - ((mods.size() + 1) * entry_height) - 10;

        nk_fill_rect(nk_window_get_canvas(context), nk_rect(select_x - 6, y, select_width, ((mods.size() + 1) * entry_height)), 0.0f, ui::colors::darkest);

        nk_layout_space_push(context, nk_rect(select_x, y, select_width, entry_height));
        ui::fonts::use_font(ui::fonts::medium_font_15);
        if (nk_button_label_styled(context, &ui::styles::mod_select_entry_button, "HOME"))
        {
            if (current_selection.compare("HOME") != 0)
            {
                current_selection = "HOME";

                ui::set_show_home(true);
                status::hide();
                footer::set_show_key_input(false);
            }

            toggle();
        }

        y += entry_height + 5;

        for (int i = 0; i < mods.size(); i++)
        {
            auto mod_name = mods[i];

            nk_layout_space_push(context, nk_rect(select_x, y, select_width, entry_height));
            ui::fonts::use_font(ui::fonts::medium_font_15);
            if (nk_button_label_styled(context, &ui::styles::mod_select_entry_button, mod_name.c_str()))
            {
                if (current_selection.compare(mod_name) != 0)
                {
                    current_selection = mod_name;
                    status::hide();

                    mod::load(mod_name);
                    ui::set_show_home(false);
                }

                toggle();
            }

            y += entry_height;
        }
    }

    void draw(struct nk_context *context)
    {
        nk_layout_space_push(context, nk_rect(select_x, select_y, select_width, select_height));
        ui::fonts::use_font(ui::fonts::medium_font_15);
        if (nk_button_label_styled(context, is_enabled ? &ui::styles::mod_select_button : &ui::styles::mod_select_button_faded, current_selection.c_str()))
        {
            if (is_enabled)
                toggle();
        }

        if (!is_enabled)
            return;

        if (is_unfolded)
        {
            draw_entries(context);
        }
        else
        {
            nk_draw_image(nk_window_get_canvas(context), nk_rect(select_x + select_width * 0.8, select_y + select_height * 0.42, 16, 16), &ui::load_image("resources/images/dropdown_arrow.png"), ui::colors::pink);
        }
    }

    void disable()
    {
        is_enabled = false;
    }

    void enable()
    {
        is_enabled = true;
    }
}