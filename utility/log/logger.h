#pragma once

#include <boost/core/noncopyable.hpp>
#include <fmt/format.h>

namespace dev::utility {

enum class LogLevel : int32_t {
    kTrace,
    kDebug,
    kInfo,
    kWarning,
    kError,
    kCritical,
    kOff
};

class ILogger : boost::noncopyable {
public:
    virtual ~ILogger() = default;

    virtual bool Initialize() = 0;
    virtual void ChangeLogLevel(LogLevel log_level) = 0;

    void LogTrace(const std::string& log_msg) const;
    void LogTrace(std::string&& log_msg) const;

    void LogDebug(const std::string& log_msg) const;
    void LogDebug(std::string&& log_msg) const;

    void LogInfo(const std::string& log_msg) const;
    void LogInfo(std::string&& log_msg) const;

    void LogWarning(const std::string& log_msg) const;
    void LogWarning(std::string&& log_msg) const;

    void LogError(const std::string& log_msg) const;
    void LogError(std::string&& log_msg) const;

    void LogCritical(const std::string& log_msg) const;
    void LogCritical(std::string&& log_msg) const;

    template <typename... Args>
    void LogTrace(std::format_string<Args...> fmt, Args&&... args) const {
        OnLogTrace(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    constexpr void LogDebug(std::format_string<Args...> fmt, Args&&... args) const {
        OnLogDebug(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void LogInfo(std::format_string<Args...> fmt, Args&&... args) const {
        OnLogInfo(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void LogWarning(std::format_string<Args...> fmt, Args&&... args) const {
        OnLogWarning(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void LogError(std::format_string<Args...> fmt, Args&&... args) const {
        OnLogError(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void LogCritical(std::format_string<Args...> fmt, Args&&... args) const {
        OnLogCritical(std::format(fmt, std::forward<Args>(args)...));
    }

protected:
    virtual void OnLogTrace(const std::string& msg) const = 0;
    virtual void OnLogDebug(const std::string& msg) const = 0;
    virtual void OnLogInfo(const std::string& msg) const = 0;
    virtual void OnLogWarning(const std::string& msg) const = 0;
    virtual void OnLogError(const std::string& msg) const = 0;
    virtual void OnLogCritical(const std::string& msg) const = 0;

    virtual void OnLogTrace(std::string&& msg) const = 0;
    virtual void OnLogDebug(std::string&& msg) const = 0;
    virtual void OnLogInfo(std::string&& msg) const = 0;
    virtual void OnLogWarning(std::string&& msg) const = 0;
    virtual void OnLogError(std::string&& msg) const = 0;
    virtual void OnLogCritical(std::string&& msg) const = 0;
};

} // namespace dev::utility
