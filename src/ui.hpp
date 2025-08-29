
#define WM_MOD_DOWNLOAD_SUCCESS (WM_USER + 0x0001)
#define WM_MOD_DOWNLOAD_FAILED (WM_USER + 0x0002)
#define WM_MOD_UPDATE_STATUS (WM_USER + 0x0003)
#define WM_LAUNCHER_UPDATE_STATUS (WM_USER + 0x0004)
#define WM_LAUNCHER_DOWNLOAD_SUCCESS (WM_USER + 0x0005)
#define WM_LAUNCHER_DOWNLOAD_FAILED (WM_USER + 0x0006)
#define WM_LAUNCHER_RESTARTING_IN (WM_USER + 0x0007)
#define WM_KEY_SUBMIT_STATUS (WM_USER + 0x0008)

#define MP_MOD_INITIAL_DOWNLOAD 0
#define MP_MOD_UPDATE_DOWNLOAD 1

namespace ui
{
    constexpr std::uint32_t WINDOW_WIDTH = 766;
    constexpr std::uint32_t WINDOW_HEIGHT = 339;

    extern HWND window_handle;

    void init();
    bool handle_events();
    void draw();
    void shutdown();

    struct nk_image load_image(std::string path);

    void set_show_home(bool value);
    bool get_show_home();
}