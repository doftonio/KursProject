#ifndef ENUMUTILS_H
#define ENUMUTILS_H

#include <type_traits>
#include <utility>

template <typename Enum>
constexpr auto toUnderlying(Enum value) noexcept {
#if defined(__cpp_lib_to_underlying) && __cpp_lib_to_underlying >= 202102L
    return std::to_underlying(value);
#else
    return static_cast<std::underlying_type_t<Enum>>(value);
#endif
}

#endif
