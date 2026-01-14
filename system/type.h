#pragma once

#include <memory>
#include <type_traits>

namespace dev::system {

template<class T>
struct is_shared_ptr : std::false_type {};

template<class T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

template<class T>
struct is_unique_ptr : std::false_type {};

template<class T>
struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};

template<class T>
struct is_weak_ptr : std::false_type {};

template<class T>
struct is_weak_ptr<std::weak_ptr<T>> : std::true_type {};

template <typename T>
struct is_smart_ptr {
    static constexpr bool value = is_shared_ptr<T>::value || is_unique_ptr<T>::value || is_weak_ptr<T>::value;
};

template <typename... Args>
constexpr auto ToArray(Args&&... args) {
    using CommonType = typename std::common_type_t<Args...>;
    constexpr auto kArraySize = sizeof...(Args);
    std::array<CommonType, kArraySize> arr = { {std::forward<Args>(args)...} };
    return arr;
}

} // namespace dev::system
