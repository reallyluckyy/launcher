#include "../../include/nuklear_gdip.h"

#include "../stdinc.hpp"
#include "titlebar.hpp"

#include "../ui.hpp"
#include "colors.hpp"

#include "styles.hpp"
#include "fonts.hpp"

namespace titlebar
{
    void drag_window()
    {
        static tagPOINT pts, diffpts;
        static tagRECT windowrect;
        static int width, height;

        GetWindowRect(ui::window_handle, &windowrect);
        GetCursorPos(&diffpts);

        if (diffpts.x < windowrect.left || diffpts.y < windowrect.top || diffpts.x > windowrect.right || diffpts.y > (windowrect.top + ui::WINDOW_HEIGHT * 0.1))
            return;

        if (GetForegroundWindow() != ui::window_handle)
            return;

        diffpts.x = windowrect.left - diffpts.x;
        diffpts.y = windowrect.top - diffpts.y;
        width = windowrect.right - windowrect.left;
        height = windowrect.bottom - windowrect.top;

        while (MK_LBUTTON)
        {
            GetCursorPos(&pts);
            if ((pts.x, pts.y) != (diffpts.x, diffpts.y))
                MoveWindow(
                    ui::window_handle,
                    pts.x + diffpts.x, pts.y + diffpts.y,
                    width, height,
                    true);

            if (GetAsyncKeyState(VK_LBUTTON) >= 0)
                break;
        }
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }

    void draw(struct nk_context *context)
    {
        nk_fill_rect(nk_window_get_canvas(context), nk_rect(-10, 0, ui::WINDOW_WIDTH + 20, ui::WINDOW_HEIGHT * 0.1), 0.0f, ui::colors::dark);

        auto button_size = (ui::WINDOW_HEIGHT * 0.1) - 2;
        nk_layout_space_push(context, nk_rect(ui::WINDOW_WIDTH + 5 - (2 * button_size + 5), -2, button_size, button_size));
        if (nk_button_image_styled(context, &ui::styles::titlebar_button, ui::load_image("resources/images/minimize_button.png")))
        {
            ShowWindow(ui::window_handle, SW_MINIMIZE);
        }

        nk_layout_space_push(context, nk_rect(ui::WINDOW_WIDTH + 5 - (1 * button_size), -2, button_size, button_size));
        if (nk_button_image_styled(context, &ui::styles::titlebar_button, ui::load_image("resources/images/quit_button.png")))
        {
            PostQuitMessage(0);
        }

        nk_layout_space_push(context, nk_rect(18, 4, ui::WINDOW_WIDTH * 0.5, 20));
        ui::fonts::use_font(ui::fonts::bold_font_15);
        nk_label_colored(context, "CODMVM LAUNCHER", NK_TEXT_ALIGN_LEFT, ui::colors::title_text_color);

        if (GetAsyncKeyState(VK_LBUTTON))
        {
            drag_window();
        }
    }
}