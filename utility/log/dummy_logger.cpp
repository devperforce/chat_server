#include "pch.h"
#include "utility/log/dummy_logger.h"

namespace dev::utility {

bool DummyLogger::Initialize() {
    return true;
}

void DummyLogger::ChangeLogLevel(LogLevel log_level) {
    blocked_level_ = log_level;
}

void DummyLogger::OnLogTrace(const std::string& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[TRACE] " << msg << std::endl;
    }
}

void DummyLogger::OnLogDebug(const std::string& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[DEBUG] " << msg << std::endl;
    }
}

void DummyLogger::OnLogInfo(const std::string& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[INFO] " << msg << std::endl;
    }
}

void DummyLogger::OnLogWarning(const std::string& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[WARNING] " << msg << std::endl;
    }
}

void DummyLogger::OnLogError(const std::string& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[ERROR] " << msg << std::endl;
    }
}

void DummyLogger::OnLogCritical(const std::string& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[CRITICAL] " << msg << std::endl;
    }
}

void DummyLogger::OnLogTrace(std::string&& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[TRACE] " << msg << std::endl;
    }
}

void DummyLogger::OnLogDebug(std::string&& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[DEBUG] " << msg << std::endl;
    }
}

void DummyLogger::OnLogInfo(std::string&& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[INFO] " << msg << std::endl;
    }
}

void DummyLogger::OnLogWarning(std::string&& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[WARNING] " << msg << std::endl;
    }
}

void DummyLogger::OnLogError(std::string&& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[ERROR] " << msg << std::endl;
    }
}

void DummyLogger::OnLogCritical(std::string&& msg) const {
    if (blocked_level_ <= LogLevel::kTrace) {
        std::cout << "[CRITICAL] " << msg << std::endl;
    }
}
} // namespace dev::utility
