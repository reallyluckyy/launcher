#define logger_error(msg) logger::error_internal(util::format("%s:%i  %s", __FILE__, __LINE__, std::string(msg).c_str()))

namespace logger
{
    void debug(std::string message);
    void error_internal(std::string message);

    void upload_log();
}