#include "pch.h"
#include "utility/log/static_logger.h"

#include <spdlog/sinks/hourly_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include "system/singleton.h"

namespace dev::utility {

class LogRepository : public system::Singleton<LogRepository> {
public:
    std::unique_ptr<spdlog::logger> logger_;
};

StaticLogger::StaticLogger(
    const std::string& path,
    LogLevelInfo level_info,
    int32_t max_file_count
) : path_(path), level_info_(level_info), max_file_count_{ max_file_count } {
}

spdlog::logger* StaticLogger::GetLogger() {
    return LogRepository::GetInstance().logger_.get();
}

bool StaticLogger::Initialize() {
    static const char* console_pattern = "%^[%H:%M:%S:%e %z][%l][%t]%$ %v";
    const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    //console_sink->set_color(spdlog::level::info, console_sink->WHITE);
    console_sink->set_level(level_info_.console_level);
    console_sink->set_pattern(console_pattern);

    static const char* file_pattern = "[%H:%M:%S:%e %z][%l][%t] %v";
    const auto file_sink = std::make_shared<spdlog::sinks::hourly_file_sink_mt>(path_, false, max_file_count_);
    file_sink->set_level(level_info_.file_level);
    file_sink->set_pattern(file_pattern);

    const spdlog::sinks_init_list sinks = { file_sink, console_sink };

    LogRepository::GetInstance().logger_ = std::make_unique<spdlog::logger>("dev", sinks.begin(), sinks.end());

    LogRepository::GetInstance().logger_->set_level(spdlog::level::trace);

    // 임시
    LogRepository::GetInstance().logger_->flush_on(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(1));

    return true;
}

static spdlog::level::level_enum ToSpdlogLevel(LogLevel log_level) {
    return static_cast<spdlog::level::level_enum>(log_level);
}

void StaticLogger::ChangeLogLevel(LogLevel log_level) {
    const auto spdlog_level = ToSpdlogLevel(log_level);
    LogRepository::GetInstance().logger_->set_level(spdlog_level);
}

template <typename String>
static void Logging(spdlog::level::level_enum log_level, String&& msg) {
    if (log_level == spdlog::level::level_enum::trace) {
        LogRepository::GetInstance().logger_->trace(std::forward<String>(msg));
    }
    else if (log_level == spdlog::level::level_enum::debug) {
        LogRepository::GetInstance().logger_->debug(std::forward<String>(msg));
    }
    else if (log_level == spdlog::level::level_enum::info) {
        LogRepository::GetInstance().logger_->info(std::forward<String>(msg));
    }
    else if (log_level == spdlog::level::level_enum::warn) {
        LogRepository::GetInstance().logger_->warn(std::forward<String>(msg));
    }
    else if (log_level == spdlog::level::level_enum::err) {
        LogRepository::GetInstance().logger_->error(std::forward<String>(msg));
    }
    else if (log_level == spdlog::level::level_enum::critical) {
        LogRepository::GetInstance().logger_->critical(std::forward<String>(msg));
    }
}

void StaticLogger::OnLogTrace(const std::string& msg) const {
    Logging(spdlog::level::level_enum::trace, msg);
}

void StaticLogger::OnLogDebug(const std::string& msg) const {
    Logging(spdlog::level::level_enum::debug, msg);
}

void StaticLogger::OnLogInfo(const std::string& msg) const {
    Logging(spdlog::level::level_enum::info, msg);
}

void StaticLogger::OnLogWarning(const std::string& msg) const {
    Logging(spdlog::level::level_enum::warn, msg);
}

void StaticLogger::OnLogError(const std::string& msg) const {
    Logging(spdlog::level::level_enum::err, msg);
}

void StaticLogger::OnLogCritical(const std::string& msg) const {
    Logging(spdlog::level::level_enum::critical, msg);
}

void StaticLogger::OnLogTrace(std::string&& msg) const {
    Logging(spdlog::level::level_enum::trace, std::move(msg));
}

void StaticLogger::OnLogDebug(std::string&& msg) const {
    Logging(spdlog::level::level_enum::debug, std::move(msg));
}

void StaticLogger::OnLogInfo(std::string&& msg) const {
    Logging(spdlog::level::level_enum::info, std::move(msg));
}

void StaticLogger::OnLogWarning(std::string&& msg) const {
    Logging(spdlog::level::level_enum::warn, std::move(msg));
}

void StaticLogger::OnLogError(std::string&& msg) const {
    Logging(spdlog::level::level_enum::err, std::move(msg));
}

void StaticLogger::OnLogCritical(std::string&& msg) const {
    Logging(spdlog::level::level_enum::critical, std::move(msg));
}

} // namespace dev::utility
