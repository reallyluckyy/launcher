
namespace status
{
    enum class ContinueOption
    {
        None,
        Retry,
        ContinueInOfflineMode
    };

    void show(std::string message, bool loading = true);
    void hide();
    void exception(std::string message, ContinueOption option);

    void draw(struct nk_context *context);
}