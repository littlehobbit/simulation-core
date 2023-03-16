#include "address.h"

#include <cstdio>
#include <optional>

namespace address {

// TODO: fix or remove
std::optional<MacAddress> MacAddress::from_string(
    std::string_view str) noexcept {
  MacAddress address;
  auto ret =
      std::sscanf(str.data(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                  &address.buffer[0], &address.buffer[1], &address.buffer[2],
                  &address.buffer[3], &address.buffer[4], &address.buffer[5]);
  return (ret == 6) ? std::optional{address} : std::nullopt;
}

}  // namespace address