#pragma once

#include <exception>
#include <string>

namespace dev::system {

class NullPtrException : public std::exception {
public:
    explicit NullPtrException(const std::string& msg) : message_(msg) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

} // dev::system
