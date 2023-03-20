#include "node.h"

#include <stdexcept>

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>

#include <fmt/core.h>
#include <ns3/node.h>
#include <ns3/object.h>

#include "address.h"
#include "model/application.h"
#include "model/device.h"
#include "model/model_build_error.h"
#include "name_service.h"
#include "parser/parser.h"
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/ipv4-static-routing.h>
#include <ns3/ipv6-address.h>
#include <ns3/ipv6-static-routing-helper.h>
#include <ns3/ipv6-static-routing.h>

namespace model {

void Node::attach(Device &&device) {
  _node->AddDevice(device.get());

  {
    auto interface = _ipv4->AddInterface(device.get());
    _ipv4->SetUp(interface);
    _ipv4->SetMetric(interface, 1);

    for (const auto &address : device.ipv4_addresses()) {
      if (!_ipv4->AddAddress(interface, address)) {
        // TODO: extract error
        throw std::runtime_error("Can't assign address");
      }
    }
  }

  {
    auto interface = _ipv6->AddInterface(device.get());
    _ipv6->SetUp(interface);
    _ipv6->SetMetric(interface, 1);

    for (const auto &address : device.ipv6_addresses()) {
      if (!_ipv6->AddAddress(interface, address)) {
        throw std::runtime_error("Can't assign address");
      }
    }
  }

  names::add(_node, device.get(), device.name());
  _device_per_name.emplace(device.name(), device.get());
  _devices.push_back(std::move(device));
}

void Node::attach(Application &&app) {
  _node->AddApplication(app.get());
  names::add(_node, app.get(), app.name());
  _applications.push_back(std::move(app));
}

auto Node::create(const parser::NodeDescription &description) -> Node {
  auto node = ns3::CreateObject<ns3::Node>();
  ns3::InternetStackHelper stack;
  stack.Install(node);
  names::add(node, description.name);

  // TODO: set internet protocols settings

  Node ret{node};
  ret._ipv4 = node->GetObject<ns3::Ipv4>();
  ret._ipv6 = node->GetObject<ns3::Ipv6>();

  for (const auto &device_desc : description.devices) {
    ret.attach(Device::create(device_desc));
  }

  for (const auto &app_desc : description.applications) {
    ret.attach(Application::create(app_desc));
  }

  // IPv4 routing
  auto ipv4_static_routing =
      ns3::Ipv4StaticRoutingHelper().GetStaticRouting(ret._ipv4);

  for (const auto &ipv4_route : description.routing.ipv4) {
    auto device = ret.get_device_by_name(ipv4_route.interface);
    if (device == nullptr) {
      throw ModelBuildError(fmt::format("Can't find interface \"{}\" for route",
                                        ipv4_route.interface));
    }

    auto interface = ret._ipv4->GetInterfaceForDevice(device);
    ipv4_static_routing->AddNetworkRouteTo(
        ns3::Ipv4Address{ipv4_route.network.network().to_uint()},
        ns3::Ipv4Mask{ipv4_route.network.netmask().to_uint()}, interface,
        ipv4_route.metric);
  }

  // IPv6 routing
  ns3::Ipv6StaticRoutingHelper ipv6_helper;
  auto ipv6_static_routing = ipv6_helper.GetStaticRouting(ret._ipv6);

  for (const auto &ipv6_route : description.routing.ipv6) {
    auto device = ret.get_device_by_name(ipv6_route.interface);
    if (device == nullptr) {
      throw ModelBuildError(fmt::format("Can't find interface \"{}\" for route",
                                        ipv6_route.interface));
    }

    auto interface = ret._ipv6->GetInterfaceForDevice(device);
    ipv6_static_routing->AddNetworkRouteTo(
        ns3::Ipv6Address{ipv6_route.network.network().to_string().c_str()},
        ipv6_route.network.prefix_length(), interface, ipv6_route.metric);
  }

  return ret;
}

}  // namespace model