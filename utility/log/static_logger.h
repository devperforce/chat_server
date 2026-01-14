#pragma once

#include <spdlog/spdlog.h>
#include "utility/log/logger.h"
#include <fmt/format.h>

namespace dev::utility {

class StaticLogger final : public ILogger {
public:
    struct LogLevelInfo {
        spdlog::level::level_enum console_level;
        spdlog::level::level_enum file_level;
    };

    StaticLogger(const std::string& path, LogLevelInfo level_info, int32_t max_file_count);
    virtual ~StaticLogger() = default;

    static spdlog::logger* GetLogger();

private:
    bool Initialize() override;
    void ChangeLogLevel(LogLevel log_level) override;

    void OnLogTrace(const std::string& msg) const override;
    void OnLogDebug(const std::string& msg) const override;
    void OnLogInfo(const std::string& msg) const override;
    void OnLogWarning(const std::string& msg) const override;
    void OnLogError(const std::string& msg) const override;
    void OnLogCritical(const std::string& msg) const override;

    void OnLogTrace(std::string&& msg) const override;
    void OnLogDebug(std::string&& msg) const override;
    void OnLogInfo(std::string&& msg) const override;
    void OnLogWarning(std::string&& msg) const override;
    void OnLogError(std::string&& msg) const override;
    void OnLogCritical(std::string&& msg) const override;

    const std::string path_;
    LogLevelInfo level_info_;
    int32_t max_file_count_;
};

template <typename... Args>
static void LOG_TRACE(const char* fmt, Args&&... args) {
    auto spdlog_logger = StaticLogger::GetLogger();
    if (spdlog_logger == nullptr) {
        return;
    }
    spdlog_logger->trace(fmt::runtime(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
static void LOG_DEBUG(const char* fmt, Args&&... args) {
    auto spdlog_logger = StaticLogger::GetLogger();
    if (spdlog_logger == nullptr) {
        return;
    }
    spdlog_logger->debug(fmt::runtime(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
static void LOG_INFO(const char* fmt, Args&&... args) {
    auto spdlog_logger = StaticLogger::GetLogger();
    if (spdlog_logger == nullptr) {
        return;
    }
    spdlog_logger->info(fmt::runtime(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
static void LOG_WARNING(const char* fmt, Args&&... args) {
    auto spdlog_logger = StaticLogger::GetLogger();
    if (spdlog_logger == nullptr) {
        return;
    }
    spdlog_logger->warn(fmt::runtime(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
static void LOG_ERROR(const char* fmt, Args&&... args) {
    auto spdlog_logger = StaticLogger::GetLogger();
    if (spdlog_logger == nullptr) {
        return;
    }
    spdlog_logger->error(fmt::runtime(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
static void LOG_CRITICAL(const char* fmt, Args&&... args) {
    auto spdlog_logger = StaticLogger::GetLogger();
    if (spdlog_logger == nullptr) {
        return;
    }
    spdlog_logger->critical(fmt::runtime(fmt), std::forward<Args>(args)...);
}


} // namespace dev::utility
