#ifndef __DEVICE_H_D9MGWIE9T1CE__
#define __DEVICE_H_D9MGWIE9T1CE__

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/algorithm/string/case_conv.hpp>

#include <ns3/ipv4-interface-address.h>
#include <ns3/ipv6-interface-address.h>
#include <ns3/net-device.h>
#include <ns3/ptr.h>

// IWYU pragma: no_include <boost/iterator/iterator_facade.hpp>

namespace parser {
struct DeviceDescription;
}

namespace model {

class Channel;

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

  void attach(const std::shared_ptr<Channel> &channel);

  auto channel() const -> std::shared_ptr<Channel> { return _attached_channel; }

 private:
  Device(const ns3::Ptr<ns3::NetDevice>& device, std::string name,
         device_type type, std::vector<ns3::Ipv4InterfaceAddress> ipv4,
         std::vector<ns3::Ipv6InterfaceAddress> ipv6);

  std::string _name;
  device_type _type;
  ns3::Ptr<ns3::NetDevice> _device;

  std::shared_ptr<Channel> _attached_channel;

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
