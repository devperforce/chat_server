#pragma once

#include <bitset>

namespace dev::utility {

class BitsetConverter {
public:
    template <int32_t N>
    static std::array<bool, N> ToArray(int32_t value) {
        std::bitset<N> flags(value);
        std::array<bool, N> flag_array{};
        for (int32_t i = 0; i < N; ++i) {
            flag_array[i] = flags[i];
        }
        return flag_array;
    }

    template <int32_t N>
    static int32_t Value(const std::array<bool, N>& flag_array) {
        std::bitset<N> flags;
        for (int32_t i = 0; i < N; ++i) {
            flags[i] = flag_array[i];
        }
        return static_cast<int32_t>(flags.to_ulong());
    }
};

} // namespace dev::utility
