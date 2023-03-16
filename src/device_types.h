#ifndef DEVICE_TYPES_H
#define DEVICE_TYPES_H

#include <array>
#include <optional>
#include <string_view>

namespace device {

enum class type { Undedined, CSMA, PPP };

constexpr auto from_string(std::string_view str) noexcept
    -> std::optional<type> {
  if (str == "Csma") {
    return type::CSMA;
  }

  if (str == "Ppp") {
    return type::PPP;
  }

  return {};
}

}  // namespace device

#endif  // DEVICE_TYPES_H