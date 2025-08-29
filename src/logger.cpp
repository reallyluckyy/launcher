#include "stdinc.hpp"
#include "logger.hpp"

namespace logger
{
    std::string get_timestamp()
    {
        const uint32_t MAX_BUFFER_SIZE = 256;
        char buffer[MAX_BUFFER_SIZE];

        time_t t = time(NULL);
        struct tm *lt = localtime(&t);

        snprintf(buffer, MAX_BUFFER_SIZE, "%02d/%02d/%02d %02d:%02d:%02d", lt->tm_mon + 1, lt->tm_mday, lt->tm_year % 100, lt->tm_hour, lt->tm_min, lt->tm_sec);

        return std::string(buffer);
    }

    std::string get_log_path()
    {
        return std::string(getenv("APPDATA")) + "/codmvm_launcher/launcher.log";
    }

    void write(std::string level, std::string message)
    {
        std::filesystem::create_directories(std::string(getenv("APPDATA")) + "/codmvm_launcher");

        std::ofstream file(get_log_path(), std::ios::app);
        if (!file.good())
        {
            std::cerr << "Failed to open log file" << strerror(errno) << std::endl;
            return;
        }

        for (std::string line : util::split(message, "\n"))
            file << get_timestamp() << " " << GetCurrentProcessId() << " [" << level << "]  "
                 << "     " << line << std::endl;

        file.flush();
        file.close();
    }

    void error_internal(std::string message)
    {
        write("ERROR", message);
    }

    void debug(std::string message)
    {
        write("INFO", message);
    }

    void upload_log()
    {
        // get log and filter out the last 250 lines
        std::string log_file_contents = util::read_file(get_log_path());
        auto lines = util::split(log_file_contents, "\n");
        while (lines.size() > 250)
            lines.erase(lines.begin());

        std::stringstream stream;
        for (auto l : lines)
            stream << l << std::endl;
        log_file_contents = stream.str();

        try
        {
            httplib::Client client("https://codmvm.com");
            httplib::Params params;
            params.emplace("log", log_file_contents.c_str());
            auto response = client.Post("/data/log_upload.php", params);

            if (!response)
                throw std::logic_error(util::format("httplib error %s", httplib::to_string(response.error()).c_str()));

            if (response->status != 200)
                throw std::logic_error(util::format("http status %d", response->status));

            if (util::starts_with(response->body, "ERR_"))
                throw std::logic_error(util::format("upload error %s", response->body.c_str()));

            logger::debug(util::format("uploaded log with id %s", response->body.c_str()));
            MessageBoxA(NULL, util::format("Uploaded log!\nYour ID is %s", response->body.c_str()).c_str(), "SUCCESS", MB_OK);
        }
        catch (std::exception &e)
        {
            logger_error(util::format("failed to upload log: %s", e.what()));
            MessageBoxA(NULL, "Failed to upload log file", "ERROR", MB_OK);
            return;
        }
    }
}