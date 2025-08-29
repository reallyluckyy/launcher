
namespace process
{
    bool is_running(std::string process_name);
    bool is_elevated(HANDLE process_handle);
    bool inject(std::string process_name, std::string mod_path);
}