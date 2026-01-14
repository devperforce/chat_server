#pragma once

#include <concepts>
#include <cmath>
#include <limits>
#include <algorithm>

namespace dev::utility::math {

template<typename T>
concept FloatingPoint = std::floating_point<T>;

template<FloatingPoint T>
constexpr bool IsNearlyEqual(
    T a,
    T b,
    T absEpsilon = std::numeric_limits<T>::epsilon(),
    T relEpsilon = static_cast<T>(1e-8)
) noexcept
{
    if (std::isnan(a) || std::isnan(b)) {
        return false;
    }

    if (std::isinf(a) || std::isinf(b)) {
        return a == b;
    }

    const T diff = std::abs(a - b);
    const T maxAbs = std::max(std::abs(a), std::abs(b));

    return diff <= std::max(absEpsilon, relEpsilon * maxAbs);
}

template<FloatingPoint T>
constexpr bool IsNearlyEqualWithEpsilon(
    T a,
    T b,
    T epsilon
) noexcept
{
    if (std::isnan(a) || std::isnan(b)) {
        return false;
    }

    return std::abs(a - b) <= epsilon;
}

} // namespace dev::utility::math