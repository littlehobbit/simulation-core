#ifndef __DEVICE_H_D9MGWIE9T1CE__
#define __DEVICE_H_D9MGWIE9T1CE__

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string/case_conv.hpp>

#include <ns3/ptr.h>

#include <ns3/ipv4-interface-address.h>
#include <ns3/ipv6-interface-address.h>
#include <ns3/net-device.h>

// IWYU pragma: no_include <boost/iterator/iterator_facade.hpp>

namespace parser {
struct DeviceDescription;
}

namespace model {

enum class device_type { Undedined, CSMA, PPP };

class Device {
 public:
  static Device create(const parser::DeviceDescription& description);

  auto get() const -> ns3::Ptr<ns3::NetDevice> { return _device; }

  auto name() const -> const std::string& { return _name; }

  auto ipv4_addresses() const -> std::vector<ns3::Ipv4InterfaceAddress> {
    return _ipv4_addresses;
  }

  auto ipv6_addresses() const -> std::vector<ns3::Ipv6InterfaceAddress> {
    return _ipv6_addresses;
  }

 private:
  // TODO: remove
  Device() = default;

  Device(ns3::Ptr<ns3::NetDevice> device, std::string name,
         std::vector<ns3::Ipv4InterfaceAddress> ipv4,
         std::vector<ns3::Ipv6InterfaceAddress> ipv6)
      : _name{std::move(name)},
        _device{std::move(device)},
        _ipv4_addresses{std::move(ipv4)},
        _ipv6_addresses{std::move(ipv6)} {}

  std::string _name;
  ns3::Ptr<ns3::NetDevice> _device;
  std::vector<ns3::Ipv4InterfaceAddress> _ipv4_addresses;
  std::vector<ns3::Ipv6InterfaceAddress> _ipv6_addresses;
};

inline auto device_type_from_string(const std::string& str) noexcept
    -> std::optional<device_type> {
  auto type = boost::algorithm::to_lower_copy(str);
  if (type == "csma") {
    return device_type::CSMA;
  }

  if (type == "ppp") {
    return device_type::PPP;
  }

  return {};
}

}  // namespace model

#endif  // __DEVICE_H_D9MGWIE9T1CE__
