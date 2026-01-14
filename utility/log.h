#pragma once

#include <spdlog/spdlog.h>

namespace dev::utility {

class Log {
public:
    struct LogLevelInfo {
        spdlog::level::level_enum console_level;
        spdlog::level::level_enum file_level;
    };
    static void Initialize(const std::string& path, LogLevelInfo level_info, int32_t max_file_count);
    static void Logging(spdlog::level::level_enum log_level, const std::string& msg);
};

} // namespace dev::utility

template <typename... Args>
static void LOG_TRACE(const char* fmt, Args&&... args) {
    dev::utility::Log::Logging(spdlog::level::trace, std::vformat(fmt, std::make_format_args(args...)));
}

template <typename... Args>
static void LOG_DEBUG(const char* fmt, Args&&... args) {
    dev::utility::Log::Logging(spdlog::level::debug, std::vformat(fmt, std::make_format_args(args...)));
}

template <typename... Args>
static void LOG_INFO(const char* fmt, Args&&... args) {
    dev::utility::Log::Logging(spdlog::level::info, std::vformat(fmt, std::make_format_args(args...)));
}

template <typename... Args>
static void LOG_WARNING(const char* fmt, Args&&... args) {
    dev::utility::Log::Logging(spdlog::level::warn, std::vformat(fmt, std::make_format_args(args...)));
}

template <typename... Args>
static void LOG_ERROR(const char* fmt, Args&&... args) {
    dev::utility::Log::Logging(spdlog::level::err, std::vformat(fmt, std::make_format_args(args...)));
}

template <typename... Args>
static void LOG_CRITICAL(const char* fmt, Args&&... args) {
    dev::utility::Log::Logging(spdlog::level::critical, std::vformat(fmt, std::make_format_args(args...)));
}