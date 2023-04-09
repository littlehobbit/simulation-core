#include "node.h"

#include <algorithm>
#include <sstream>

#include <boost/asio/ip/address_v4.hpp>

#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address.h>
#include <ns3/ipv4-interface-address.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/ipv4-static-routing.h>
#include <ns3/ipv6-address.h>
#include <ns3/ipv6-interface-address.h>
#include <ns3/ipv6-static-routing-helper.h>
#include <ns3/ipv6-static-routing.h>
#include <ns3/net-device.h>
#include <ns3/node.h>
#include <ns3/object.h>
#include <ns3/ptr.h>

#include <fmt/core.h>

#include "model/application.h"
#include "model/device.h"
#include "model/model_build_error.h"
#include "name_service.h"
#include "parser/parser.h"
#include "utils/address.h"

namespace model {

Node::Node(const ns3::Ptr<ns3::Node> &node, std::string name)
    : _name{std::move(name)},
      _node{node},
      _ipv4{node->GetObject<ns3::Ipv4>()},
      _ipv6{node->GetObject<ns3::Ipv6>()} {}

void Node::attach(Device &&device) {
  _node->AddDevice(device.get());

  {
    auto interface = _ipv4->AddInterface(device.get());
    _ipv4->SetUp(interface);
    _ipv4->SetMetric(interface, 1);

    for (const auto &address : device.ipv4_addresses()) {
      if (!_ipv4->AddAddress(interface, address)) {
        std::stringstream address_stream;
        address.GetAddress().Print(address_stream);
        throw ModelBuildError(
            fmt::format("Can't assign address {} to interface \"{}\"",
                        address_stream.str(), device.name()));
      }
    }
  }

  {
    auto interface = _ipv6->AddInterface(device.get());
    _ipv6->SetUp(interface);
    _ipv6->SetMetric(interface, 1);

    for (const auto &address : device.ipv6_addresses()) {
      if (!_ipv6->AddAddress(interface, address)) {
        // TODO: reduce code duplication
        std::stringstream address_stream;
        address.GetAddress().Print(address_stream);
        throw ModelBuildError(
            fmt::format("Can't assign address {} to interface \"{}\"",
                        address_stream.str(), device.name()));
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

auto Node::create(const parser::NodeDescription &description)
    -> std::unique_ptr<Node> {
  auto node = create_ns3_node();
  names::add(node, description.name);

  auto ret = std::make_unique<Node>(node, description.name);

  ret->create_devices(description.devices);

  ret->create_applications(description.applications);

  ret->add_ipv4_routes(description.routing.ipv4);
  ret->add_ipv6_routes(description.routing.ipv6);

  return ret;
}

auto Node::create_ns3_node() -> ns3::Ptr<ns3::Node> {
  auto node = ns3::CreateObject<ns3::Node>();
  ns3::InternetStackHelper stack;
  stack.Install(node);
  return node;
}

void Node::create_devices(
    const std::vector<parser::DeviceDescription> &devices) {
  for (const auto &device_desc : devices) {
    this->attach(Device::create(device_desc));
  }
}

void Node::create_applications(
    const std::vector<parser::ApplicationDescription> &applications) {
  for (const auto &app : applications) {
    this->attach(Application::create(app));
  }
}

void Node::add_ipv4_routes(const std::vector<parser::Ipv4Route> &routes) {
  auto ipv4_static_routing =
      ns3::Ipv4StaticRoutingHelper().GetStaticRouting(_ipv4);

  for (const auto &ipv4_route : routes) {
    auto *device = get_device_by_name(ipv4_route.interface);

    if (device == nullptr) {
      throw ModelBuildError(fmt::format("Can't find interface \"{}\" for route",
                                        ipv4_route.interface));
    }

    auto interface = _ipv4->GetInterfaceForDevice(device->get());
    ipv4_static_routing->AddNetworkRouteTo(
        address::to_ns3_v4(ipv4_route.network.network()),
        ns3::Ipv4Mask{ipv4_route.network.netmask().to_uint()}, interface,
        ipv4_route.metric);
  }
}

void Node::add_ipv6_routes(const std::vector<parser::Ipv6Route> &routes) {
  ns3::Ipv6StaticRoutingHelper ipv6_helper;
  auto ipv6_static_routing = ipv6_helper.GetStaticRouting(_ipv6);

  for (const auto &ipv6_route : routes) {
    auto *device = get_device_by_name(ipv6_route.interface);
    if (device == nullptr) {
      throw ModelBuildError(fmt::format("Can't find interface \"{}\" for route",
                                        ipv6_route.interface));
    }

    auto interface = _ipv6->GetInterfaceForDevice(device->get());
    ipv6_static_routing->AddNetworkRouteTo(
        address::to_ns3_v6(ipv6_route.network.address()),
        ipv6_route.network.prefix_length(), interface, ipv6_route.metric);
  }
}

auto Node::get_device_by_name(const std::string &name) -> Device * {
  auto res = std::find_if(
      _devices.begin(), _devices.end(),
      [&name](const auto &device) { return device.name() == name; });

  if (res != _devices.end()) {
    return res.base();
  }

  return nullptr;
}

}  // namespace model