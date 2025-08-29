#include "../../include/nuklear_gdip.h"

#include "../stdinc.hpp"
#include "fonts.hpp"

#include "../resources.hpp"

namespace ui
{
    namespace fonts
    {
        GdipFont *bold_font_15;

        GdipFont *medium_font_11;
        GdipFont *medium_font_12;
        GdipFont *medium_font_13;
        GdipFont *medium_font_14;
        GdipFont *medium_font_15;
        GdipFont *medium_font_18;

        void init(struct nk_context *context)
        {
            auto bold_font_data = resources::get_file("resources/fonts/WorkSans-Bold.ttf");
            bold_font_15 = nk_gdipfont_create_from_memory(bold_font_data.data(), bold_font_data.size(), 15);

            auto medium_font_data = resources::get_file("resources/fonts/WorkSans-Medium.ttf");
            medium_font_11 = nk_gdipfont_create_from_memory(medium_font_data.data(), medium_font_data.size(), 11);
            medium_font_12 = nk_gdipfont_create_from_memory(medium_font_data.data(), medium_font_data.size(), 12);
            medium_font_13 = nk_gdipfont_create_from_memory(medium_font_data.data(), medium_font_data.size(), 13);
            medium_font_14 = nk_gdipfont_create_from_memory(medium_font_data.data(), medium_font_data.size(), 14);
            medium_font_15 = nk_gdipfont_create_from_memory(medium_font_data.data(), medium_font_data.size(), 15);
            medium_font_18 = nk_gdipfont_create_from_memory(medium_font_data.data(), medium_font_data.size(), 18);

            use_font(medium_font_11);
        }

        void use_font(GdipFont *font)
        {
            nk_gdip_set_font(font);
        }
    }
}