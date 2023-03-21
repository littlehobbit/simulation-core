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

#include <ns3/ipv4-address.h>
#include <ns3/ipv4-interface-address.h>
#include <ns3/ipv6-address.h>
#include <ns3/ipv6-interface-address.h>
#include <ns3/ipv6-interface.h>

namespace asio = boost::asio;

namespace address {

using address_v4 = asio::ip::address_v4;
using network_v4 = asio::ip::network_v4;

using address_v6 = asio::ip::address_v6;
using network_v6 = asio::ip::network_v6;

inline auto to_ns3_v4(const address_v4 &address) noexcept -> ns3::Ipv4Address {
  return ns3::Ipv4Address{address.to_uint()};
}

inline auto to_ns3_v4(const network_v4 &network) noexcept
    -> ns3::Ipv4InterfaceAddress {
  return {to_ns3_v4(network.address()),
          ns3::Ipv4Mask{network.netmask().to_uint()}};
}

inline auto to_ns3_v6(const address_v6 &address) noexcept -> ns3::Ipv6Address {
  return ns3::Ipv6Address{address.to_bytes().data()};
}

inline auto to_ns3_v6(const network_v6 &network) noexcept
    -> ns3::Ipv6InterfaceAddress {
  return ns3::Ipv6InterfaceAddress{
      to_ns3_v6(network.address()),
      static_cast<uint8_t>(network.prefix_length())};
}

inline auto from_string_v4(std::string_view address,
                           std::string_view mask) noexcept
    -> std::optional<network_v4> {
  try {
    return asio::ip::make_network_v4(asio::ip::make_address_v4(address),
                                     asio::ip::make_address_v4(mask));
  } catch (...) {
    return {};
  }
}

inline auto from_string_v6(std::string_view address,
                           std::uint8_t prefix) noexcept
    -> std::optional<network_v6> {
  try {
    return asio::ip::make_network_v6(asio::ip::make_address_v6(address),
                                     prefix);
  } catch (...) {
    return {};
  }
}

}  // namespace address

#endif  // __ADDRESS_H_4CFIY4L3N4JV__
