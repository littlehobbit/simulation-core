#ifndef __DEVICE_TYPES_H_BWKCLPP7U6UU__
#define __DEVICE_TYPES_H_BWKCLPP7U6UU__

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

#endif  // __DEVICE_TYPES_H_BWKCLPP7U6UU__
