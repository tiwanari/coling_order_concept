#pragma once

namespace order_concepts {
namespace util {
template <typename Enumeration>
inline auto as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<
        typename std::underlying_type<Enumeration>::type>(value);
}
}   // namespace util
} // namespace order_concepts
