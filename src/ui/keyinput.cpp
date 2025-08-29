#include "../../include/nuklear_gdip.h"

#include "../stdinc.hpp"
#include "keyinput.hpp"

#include "../ui.hpp"
#include "../mod.hpp"
#include "colors.hpp"
#include "styles.hpp"
#include "fonts.hpp"
#include "status.hpp"

namespace keyinput
{
    constexpr std::uint32_t MAX_KEY_LENGTH = 128;
    char key[MAX_KEY_LENGTH];

    void draw(struct nk_context *context)
    {
        auto x = ui::WINDOW_WIDTH * 0.732;
        auto y = ui::WINDOW_HEIGHT * 0.87;
        auto width = 130;
        auto height = 21;

        std::string text = "PLEASE INPUT YOUR KEY:";

        nk_handle handle = nk_handle();
        handle.ptr = ui::fonts::medium_font_11;
        auto text_width = nk_gdipfont_get_text_width(handle, 0, text.c_str(), text.size());

        nk_layout_space_push(context, nk_rect((x + 91) - text_width * 0.5, y - 18, INT_MAX, 50));
        ui::fonts::use_font(ui::fonts::medium_font_11);
        nk_label_colored(context, text.c_str(), NK_TEXT_ALIGN_LEFT, ui::colors::text_color);

        nk_layout_space_push(context, nk_rect(x, y, width, height));
        nk_edit_string_zero_terminated(context, NK_EDIT_FIELD, key, MAX_KEY_LENGTH, nk_filter_ascii);

        nk_layout_space_push(context, nk_rect(x + width + 10, y, 43, height));
        ui::fonts::use_font(ui::fonts::medium_font_13);
        if (nk_button_label_styled(context, &ui::styles::load_button, "GO"))
        {
            status::show("CHECKING KEY");

            std::thread([]()
                        {
                            auto result = mod::submit_key(get_key());
                            PostMessageA(ui::window_handle, WM_KEY_SUBMIT_STATUS, (WPARAM)result, 0);
                        })
                .detach();
        }
    }

    std::string get_key()
    {
        return std::string(key);
    }
}