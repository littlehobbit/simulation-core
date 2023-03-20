#include "device.h"

#include <map>

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/ip/network_v4.hpp>
#include <boost/asio/ip/network_v6.hpp>

#include <fmt/core.h>
#include <ns3/address.h>
#include <ns3/object.h>
#include <ns3/string.h>

#include "model_build_error.h"
#include "parser/parser.h"
#include <ns3/csma-net-device.h>
#include <ns3/ipv4-address.h>
#include <ns3/ipv4-interface-address.h>
#include <ns3/ipv6-address.h>
#include <ns3/mac48-address.h>
#include <ns3/net-device.h>
#include <ns3/point-to-point-net-device.h>

namespace model {

Device Device::create(const parser::DeviceDescription &description) {
  auto type = device_type_from_string(description.type);
  if (!type.has_value()) {
    throw ModelBuildError(fmt::format(R"(Invalid type "{}" of device "{}")",
                                      description.type, description.name));
  }

  // TODO: extract creation to factory
  ns3::Ptr<ns3::NetDevice> device{};
  if (type == device_type::CSMA) {
    device = ns3::CreateObject<ns3::CsmaNetDevice>();
  } else if (type == device_type::PPP) {
    device = ns3::CreateObject<ns3::PointToPointNetDevice>();
  }

  device->SetAddress(ns3::Mac48Address::Allocate());
  for (const auto &[key, value] : description.attributes) {
    device->SetAttribute(key, ns3::StringValue(value));
  }

  std::vector<ns3::Ipv4InterfaceAddress> ipv4;
  for (const auto &ip : description.ipv4_addresses) {
    ipv4.emplace_back(ns3::Ipv4Address(ip.address().to_uint()),
                      ns3::Ipv4Mask(ip.netmask().to_uint()));
  }

  std::vector<ns3::Ipv6InterfaceAddress> ipv6;
  for (const auto &ip : description.ipv6_addresses) {
    ipv6.emplace_back(ns3::Ipv6Address(ip.address().to_string().c_str()),
                      ip.prefix_length());
  }

  // TODO: move to Node::attach
  // ns3::Names::Add(node, device_desc.name, device);

  return {device, description.name, std::move(ipv4), std::move(ipv6)};
}

};  // namespace model