#ifndef __CHANNEL_H_5R0UZOSTZ1NM__
#define __CHANNEL_H_5R0UZOSTZ1NM__

#include <optional>
#include <string_view>

namespace channel {

enum class type { Undefined, Csma, Ppp };

constexpr auto from_string(std::string_view str) noexcept
    -> std::optional<type> {
  if (str == "Csma") {
    return type::Csma;
  }

  if (str == "Ppp") {
    return type::Ppp;
  }

  return {};
}

}  // namespace channel

#endif  // __CHANNEL_H_5R0UZOSTZ1NM__
