
namespace updater
{
    constexpr std::uint32_t CURRENT_VERSION = 160;

    enum class UpdateStatus
    {
        UpdateAvailable,
        UpToDate,
        CheckFailed
    };

    updater::UpdateStatus check_update();
    bool update_launcher();
    void schedule_restart();
    void delete_old_launcher();
    void update_shortcut();
}