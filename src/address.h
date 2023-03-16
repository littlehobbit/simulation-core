#ifndef __ADDRESS_H_4CFIY4L3N4JV__
#define __ADDRESS_H_4CFIY4L3N4JV__

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/ip/network_v4.hpp>
#include <boost/asio/ip/network_v6.hpp>

namespace address {

struct MacAddress {
  static constexpr auto buffer_size = 6;
  using buffer_t = std::array<uint8_t, buffer_size>;

  MacAddress() = default;
  MacAddress(const buffer_t &buffer) : buffer(buffer) {}

  static std::optional<MacAddress> from_string(std::string_view str) noexcept;

  bool operator==(const MacAddress &rhs) const {
    return this->buffer == rhs.buffer;
  }

  bool operator==(const buffer_t &buffer) const {
    return this->buffer == buffer;
  }

  buffer_t buffer = {0};
};

using Ipv4Network = boost::asio::ip::network_v4;
using Ipv6Network = boost::asio::ip::network_v6;

inline auto from_string_v4(std::string_view address,
                           std::string_view mask) noexcept
    -> std::optional<Ipv4Network> {
  try {
    return boost::asio::ip::make_network_v4(
        boost::asio::ip::make_address_v4(address),
        boost::asio::ip::make_address_v4(mask));
  } catch (...) {
    return {};
  }
}

inline auto from_string_v6(std::string_view address,
                           std::uint8_t prefix) noexcept
    -> std::optional<Ipv6Network> {
  try {
    return boost::asio::ip::make_network_v6(
        boost::asio::ip::make_address_v6(address), prefix);
  } catch (...) {
    return {};
  }
}

}  // namespace address

#endif  // __ADDRESS_H_4CFIY4L3N4JV__
