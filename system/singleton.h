#pragma once

namespace dev::system {

template <typename Type>
class Singleton {
public:
    static Type& GetInstance() {
        static Type instance;
        return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

protected:
    Singleton() = default;
    ~Singleton() = default;
};

} // namespace dev::system
