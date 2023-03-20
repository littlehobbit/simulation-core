#include "node.h"

#include <stdexcept>

#include <ns3/node.h>
#include <ns3/object.h>

#include "model/application.h"
#include "model/device.h"
#include "name_service.h"
#include "parser/parser.h"
#include <ns3/internet-stack-helper.h>

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

  // TODO: setup routing

  return ret;
}

}  // namespace model