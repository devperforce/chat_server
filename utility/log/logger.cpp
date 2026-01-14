#include "pch.h"
#include "utility/log/logger.h"

namespace dev::utility {

void ILogger::LogTrace(const std::string& log_msg) const {
    OnLogTrace(log_msg);
}

void ILogger::LogTrace(std::string&& log_msg) const {
    OnLogTrace(std::move(log_msg));
}

void ILogger::LogDebug(const std::string& log_msg) const {
    OnLogDebug(log_msg);
}

void ILogger::LogDebug(std::string&& log_msg) const {
    OnLogDebug(std::move(log_msg));
}

void ILogger::LogInfo(const std::string& log_msg) const {
    OnLogInfo(log_msg);
}

void ILogger::LogInfo(std::string&& log_msg) const {
    OnLogInfo(std::move(log_msg));
}

void ILogger::LogWarning(const std::string& log_msg) const {
    OnLogWarning(log_msg);
}

void ILogger::LogWarning(std::string&& log_msg) const {
    OnLogWarning(std::move(log_msg));
}

void ILogger::LogError(const std::string& log_msg) const {
    OnLogError(log_msg);
}

void ILogger::LogError(std::string&& log_msg) const {
    OnLogError(std::move(log_msg));
}

void ILogger::LogCritical(const std::string& log_msg) const {
    OnLogCritical(log_msg);
}

void ILogger::LogCritical(std::string&& log_msg) const {
    OnLogCritical(std::move(log_msg));
}

} // namespace dev::utility
