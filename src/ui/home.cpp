#include "../../include/nuklear_gdip.h"

#include "../stdinc.hpp"
#include "home.hpp"

#include "../ui.hpp"
#include "../updater.hpp"
#include "colors.hpp"
#include "styles.hpp"
#include "fonts.hpp"

namespace home
{
    void draw(struct nk_context *context)
    {
        auto y_start = ui::WINDOW_HEIGHT * 0.1;
        auto height = ui::WINDOW_HEIGHT * 0.65;

        nk_fill_rect(nk_window_get_canvas(context), nk_rect(-10, y_start, ui::WINDOW_WIDTH + 20, height + 2), 0.0f, ui::colors::darker);

        auto x = ui::WINDOW_WIDTH * 0.06;

        nk_layout_space_push(context, nk_rect(x, ui::WINDOW_HEIGHT * 0.2, INT_MAX, 20));
        ui::fonts::use_font(ui::fonts::medium_font_18);
        nk_label_colored(context, "NEWS", NK_TEXT_ALIGN_LEFT, ui::colors::title_text_color);

        nk_layout_space_push(context, nk_rect(x, (ui::WINDOW_HEIGHT * 0.2) + 35, INT_MAX, 20));
        ui::fonts::use_font(ui::fonts::medium_font_14);
        nk_label_colored(context, "IWXMVM 1.0", NK_TEXT_ALIGN_LEFT, ui::colors::title_text_color);

        nk_layout_space_push(context, nk_rect(x, (ui::WINDOW_HEIGHT * 0.2) + 62, INT_MAX, INT_MAX));
        ui::fonts::use_font(ui::fonts::medium_font_13);
        nk_label_colored(context, "It's been a long time, but IWXMVM 1.0 is finally out for COD4!\nWith this, IWXMVM has now become opt-out for IW3, instead of opt-in like before.", NK_TEXT_ALIGN_LEFT, ui::colors::title_text_color);

        nk_layout_space_push(context, nk_rect(x, (ui::WINDOW_HEIGHT * 0.2) + 120, 132, 26));
        if (nk_button_label_styled(context, &ui::styles::slider_button, "What is IWXMVM?"))
        {
            util::open_link("https://twitter.com/reallyluckyy/status/1767304140646977883");
        }

        nk_layout_space_push(context, nk_rect(ui::WINDOW_WIDTH * 0.9, y_start + height * 0.89, INT_MAX, INT_MAX));
        ui::fonts::use_font(ui::fonts::medium_font_11);
        nk_label_colored(context, util::format("VERSION %u", updater::CURRENT_VERSION).c_str(), NK_TEXT_ALIGN_LEFT, ui::colors::version_text_color);
    }
}