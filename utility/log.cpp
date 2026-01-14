#include "pch.h"
#include "utility/log.h"

#include <spdlog/sinks/hourly_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include "system/singleton.h"

namespace dev::utility {

class LogRepository : public system::Singleton<LogRepository> {
    std::unique_ptr<spdlog::logger> logger_;

    friend class Log;
};

void Log::Initialize(const std::string& path, LogLevelInfo level_info, int32_t max_file_count) {
    static const char* console_pattern  = "%^[%H:%M:%S:%e %z][%l][%t]%$ %v";
    const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    //console_sink->set_color(spdlog::level::info, console_sink->WHITE);
    console_sink->set_level(level_info.console_level);
    console_sink->set_pattern(console_pattern);

    static const char* file_pattern  = "[%H:%M:%S:%e %z][%l][%t] %v";
    const auto file_sink = std::make_shared<spdlog::sinks::hourly_file_sink_mt>(path, false, max_file_count);
    file_sink->set_level(level_info.file_level);
    file_sink->set_pattern(file_pattern);

    const spdlog::sinks_init_list sinks = { file_sink, console_sink };

    LogRepository::GetInstance().logger_ = std::make_unique<spdlog::logger>("dev", sinks.begin(), sinks.end());

    LogRepository::GetInstance().logger_->set_level(spdlog::level::trace);

    // 임시
    LogRepository::GetInstance().logger_->flush_on(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(1));
}

/*
auto& Log::Logger() {
    static boost::log::sources::severity_logger< boost::log::trivial::severity_level> logger;
    return logger;
}
*/
void Log::Logging(spdlog::level::level_enum log_level, const std::string& msg) {
    if (log_level == spdlog::level::level_enum::trace) {
        LogRepository::GetInstance().logger_->trace(msg);
    } else if (log_level == spdlog::level::level_enum::debug) {
        LogRepository::GetInstance().logger_->debug(msg);
    } else if (log_level == spdlog::level::level_enum::info) {
        LogRepository::GetInstance().logger_->info(msg);
    } else if (log_level == spdlog::level::level_enum::warn) {
        LogRepository::GetInstance().logger_->warn(msg);
    } else if (log_level == spdlog::level::level_enum::err) {
        LogRepository::GetInstance().logger_->error(msg);
    } else if (log_level == spdlog::level::level_enum::critical) {
        LogRepository::GetInstance().logger_->critical(msg);
    } 
}

} // namespace dev::utility

// https://github.com/gabime/spdlog/wiki/3.-Custom-formatting