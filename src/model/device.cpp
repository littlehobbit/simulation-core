#include "device.h"

#include <utility>

#include <ns3/address.h>
#include <ns3/channel.h>
#include <ns3/csma-channel.h>
#include <ns3/csma-net-device.h>
#include <ns3/ipv4-interface-address.h>
#include <ns3/mac48-address.h>
#include <ns3/net-device.h>
#include <ns3/object.h>
#include <ns3/point-to-point-channel.h>
#include <ns3/point-to-point-net-device.h>

#include <fmt/core.h>

#include "model/channel.h"
#include "model_build_error.h"
#include "parser/parser.h"
#include "utils/address.h"
#include "utils/object.h"

namespace model {

Device::Device(const ns3::Ptr<ns3::NetDevice> &device, std::string name,
               device_type type, std::vector<ns3::Ipv4InterfaceAddress> ipv4,
               std::vector<ns3::Ipv6InterfaceAddress> ipv6)
    : _name{std::move(name)},
      _device{device},
      _type{type},
      _ipv4_addresses{std::move(ipv4)},
      _ipv6_addresses{std::move(ipv6)} {}

Device Device::create(const parser::DeviceDescription &description) {
  auto type = device_type_from_string(description.type);
  if (!type.has_value()) {
    throw ModelBuildError(fmt::format(R"(Invalid type "{}" of device "{}")",
                                      description.type, description.name));
  }

  // TODO: extract creation to factory
  ns3::Ptr<ns3::NetDevice> device{};
  if (type == device_type::CSMA) {
    device = utils::create<ns3::NetDevice>("ns3::CsmaNetDevice");
  } else if (type == device_type::PPP) {
    device = utils::create<ns3::NetDevice>("ns3::PointToPointNetDevice");
  }
  device->SetAddress(ns3::Mac48Address::Allocate());

  try {
    utils::set_attributes(device, description.attributes);
  } catch (utils::BadAttribute &attr) {
    throw ModelBuildError(fmt::format(R"(Bad attribute "{}" of device "{}")",
                                      attr.attribute, description.name));
  }

  std::vector<ns3::Ipv4InterfaceAddress> ipv4;
  for (const auto &ip : description.ipv4_addresses) {
    ipv4.emplace_back(address::to_ns3_v4(ip));
  }

  std::vector<ns3::Ipv6InterfaceAddress> ipv6;
  for (const auto &ip : description.ipv6_addresses) {
    ipv6.emplace_back(address::to_ns3_v6(ip));
  }

  return {device, description.name, *type, std::move(ipv4), std::move(ipv6)};
}

void Device::attach(const std::shared_ptr<Channel> &channel) {
  if (has_channel()) {
    throw ModelBuildError(fmt::format(R"(Device "{}" already has channel "{}")",
                                      _name, _attached_channel->name()));
  }

  if (_type == device_type::CSMA && channel->type() == channel_type::CSMA) {
    auto csma_device = _device->GetObject<ns3::CsmaNetDevice>();
    auto csma_channel = channel->get()->GetObject<ns3::CsmaChannel>();
    csma_device->Attach(csma_channel);
  } else if (_type == device_type::PPP &&
             channel->type() == channel_type::PPP) {
    // TODO: check PPP only 2 device
    auto ppp_device = _device->GetObject<ns3::PointToPointNetDevice>();
    auto ppp_channel = channel->get()->GetObject<ns3::PointToPointChannel>();
    ppp_device->Attach(ppp_channel);
  } else {
    throw ModelBuildError(fmt::format(
        R"(Can't attach channel "{}" to device "{}")", channel->name(), _name));
  }

  _attached_channel = channel;
}

bool Device::has_channel() const {
  return _attached_channel != nullptr;
}

};  // namespace model