
namespace mod
{
    enum class LauncherState
    {
        Initial,
        Loading,
        WaitingForGame,
        GameFound,
        Injected
    };
    
    LauncherState get_launcher_state();

    enum class UpdateStatus : std::uint32_t
    {
        UpdateAvailable,
        UpToDate,
        CheckFailed
    };

    enum class KeyCheckStatus : std::uint32_t
    {
        Success,
        InvalidKey,
        DownloadFailed
    };

    void load(std::string name);

    bool is_iwxmvm();
    void toggle_iwxmvm();

    void update_game_process_state();
    void download_or_update_mod(bool force_check_for_update);
    bool download();
    void wait_for_game();
    void inject();

    KeyCheckStatus submit_key(std::string key);

    std::array<std::string, 8> get_supported_mods();
    std::string get_current_mod();
    std::vector<std::string> get_supported_process_names();
}