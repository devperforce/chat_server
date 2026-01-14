#pragma once

#include "utility/log/logger.h"

namespace dev::utility {

class DummyLogger final : public ILogger {
public:
    virtual ~DummyLogger() = default;

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

    LogLevel blocked_level_ = LogLevel::kTrace;
};

} // namespace dev::utility