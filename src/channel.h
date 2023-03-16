#ifndef CHANNEL_H
#define CHANNEL_H

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

#endif  // CHANNEL_H