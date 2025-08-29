#include "../../include/nuklear_gdip.h"

#include "../stdinc.hpp"
#include "modpage.hpp"

#include "../ui.hpp"
#include "../mod.hpp"
#include "colors.hpp"
#include "styles.hpp"
#include "fonts.hpp"

namespace modpage
{

    void draw(struct nk_context *context)
    {
        auto y_start = ui::WINDOW_HEIGHT * 0.1;
        auto height = ui::WINDOW_HEIGHT * 0.65;

        auto background_image = ui::load_image(util::format("resources/images/%s_background.jpg", mod::get_current_mod().c_str()).c_str());
        nk_draw_image(nk_window_get_canvas(context), nk_rect(0, y_start, background_image.w, background_image.h), &background_image, ui::colors::pink);

        auto logo_image = ui::load_image(util::format("resources/images/%s_logo.png", mod::get_current_mod().c_str()).c_str());

        float img_h = 84;
        float img_w = logo_image.w * (img_h / logo_image.h);

        auto logo_y = y_start + height * 0.5 - img_h * 0.55;
        auto logo_x = ui::WINDOW_WIDTH * 0.5 - img_w * 0.5;
        nk_draw_image(nk_window_get_canvas(context), nk_rect(logo_x, logo_y, img_w, img_h), &logo_image, ui::colors::pink);
    }
}