#include "../../include/nuklear.h"

#include "../stdinc.hpp"
#include "styles.hpp"

#include "colors.hpp"

namespace ui
{
    namespace styles
    {
        struct nk_style_button titlebar_button;
        struct nk_style_button mod_info_button;
        struct nk_style_button slider_button;
        struct nk_style_button mod_select_button;
        struct nk_style_button mod_select_button_faded;
        struct nk_style_button mod_select_entry_button;
        struct nk_style_button load_button;
        struct nk_style_button social_button;

        void init(struct nk_context *context)
        {
            titlebar_button = context->style.button;
            titlebar_button.border = 0;
            titlebar_button.rounding = 0;
            titlebar_button.image_padding = nk_vec2(7, 7);
            titlebar_button.normal = nk_style_item_color(colors::dark);
            titlebar_button.hover = nk_style_item_color(colors::dark_hover);
            titlebar_button.active = nk_style_item_color(colors::dark_hover);

            mod_select_button = context->style.button;
            mod_select_button.border = 0;
            mod_select_button.rounding = 0;
            mod_select_button.text_alignment = nk_text_alignment::NK_TEXT_LEFT;
            mod_select_button.text_normal = colors::text_color;
            mod_select_button.padding = nk_vec2(20, 0);
            mod_select_button.normal = nk_style_item_color(colors::darker);
            mod_select_button.hover = nk_style_item_color(colors::dark);
            mod_select_button.active = nk_style_item_color(colors::darker);
            mod_select_button.text_normal = colors::text_color;
            mod_select_button.text_hover = colors::text_color;
            mod_select_button.text_active = colors::text_color;

            mod_select_button_faded = mod_select_button;
            mod_select_button_faded.normal = nk_style_item_color(colors::darker_faded);
            mod_select_button_faded.hover = nk_style_item_color(colors::darker_faded);
            mod_select_button_faded.active = nk_style_item_color(colors::darker_faded);
            mod_select_button_faded.text_normal = colors::text_color_faded;
            mod_select_button_faded.text_hover = colors::text_color_faded;
            mod_select_button_faded.text_active = colors::text_color_faded;

            mod_select_entry_button = context->style.button;
            mod_select_entry_button.border = 0;
            mod_select_entry_button.rounding = 0;
            mod_select_entry_button.text_alignment = nk_text_alignment::NK_TEXT_LEFT;
            mod_select_entry_button.text_normal = colors::text_color;
            mod_select_entry_button.padding = nk_vec2(20, 0);
            mod_select_entry_button.normal = nk_style_item_color(colors::darkest);
            mod_select_entry_button.hover = nk_style_item_color(colors::dark);
            mod_select_entry_button.active = nk_style_item_color(colors::darker);

            mod_info_button = context->style.button;
            mod_info_button.border = 0;
            mod_info_button.rounding = 0;
            mod_info_button.image_padding = nk_vec2(3, 3);
            mod_info_button.normal = nk_style_item_color(colors::darkest);
            mod_info_button.hover = nk_style_item_color(colors::dark);
            mod_info_button.active = nk_style_item_color(colors::darker);

            slider_button = context->style.button;
            slider_button.border = 0;
            slider_button.rounding = 0;
            slider_button.image_padding = nk_vec2(0, 0);
            slider_button.normal = nk_style_item_color(colors::darker);
            slider_button.hover = nk_style_item_color(colors::dark);
            slider_button.active = nk_style_item_color(colors::dark_slightly_darker);

            social_button = context->style.button;
            social_button.border = 0;
            social_button.rounding = 0;
            social_button.image_padding = nk_vec2(8, 8);
            social_button.normal = nk_style_item_color(colors::darker_faded);
            social_button.hover = nk_style_item_color(colors::dark);
            social_button.active = nk_style_item_color(colors::dark_slightly_darker);

            load_button = context->style.button;
            load_button.border = 0;
            load_button.rounding = 0;
            load_button.image_padding = nk_vec2(0, 0);
            load_button.normal = nk_style_item_color(colors::darker);
            load_button.hover = nk_style_item_color(colors::dark);
            load_button.active = nk_style_item_color(colors::darker);
        }
    }
}