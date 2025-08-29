#pragma once

#define timer_start() std::chrono::steady_clock::time_point timer_begin_timestamp = std::chrono::steady_clock::now()
#define timer_end() std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timer_begin_timestamp).count()

namespace util
{
    std::string compute_md5(std::string file_path);

    std::vector<std::string> split(std::string s, std::string delimiter);

    // https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    template <typename... Args>
    std::string format(const std::string &f, Args... args)
    {
        int size_s = std::snprintf(nullptr, 0, f.c_str(), args...) + 1; // Extra space for '\0'
        if (size_s <= 0)
        {
            throw std::runtime_error("Error during formatting.");
        }
        auto size = static_cast<size_t>(size_s);
        auto buf = std::make_unique<char[]>(size);
        std::snprintf(buf.get(), size, f.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
    }

    constexpr unsigned int string_hash(const char *str, int h = 0)
    {
        return !str[h] ? 5381 : (string_hash(str, h + 1) * 33) ^ str[h];
    }

    bool ends_with(const std::string &str, const std::string &suffix);
    bool starts_with(const std::string &str, const std::string &prefix);

    bool file_exists(const std::string &file_name);
    std::string read_file(const std::string &file_name);
    std::vector<std::uint8_t> read_file_binary(const std::string &file_name);
    bool write_file_binary(const std::string &file_name, const char *bytes, std::uint32_t length);

    std::string get_current_exe_path();

    bool is_process_elevated();
    bool try_relaunch_unelevated();

    std::uint32_t get_windows_build_number();
    std::string get_windows_version_string();

    void open_link(std::string url);
}