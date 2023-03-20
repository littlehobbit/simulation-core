#ifndef __NODE_H_PWWEHSK528G5__
#define __NODE_H_PWWEHSK528G5__

#include <cstddef>
#include <utility>
#include <vector>

#include <ns3/ipv4.h>
#include <ns3/ipv6.h>
#include <ns3/node.h>
#include <ns3/ptr.h>

#include "application.h"
#include "device.h"

namespace parser {
struct NodeDescription;
}

namespace model {

class Node {
 public:
  auto get() const -> ns3::Ptr<ns3::Node> { return _node; }

  auto get_device(std::size_t index) const -> const Device & {
    return _devices.at(index);
  }

  auto devices_count() const -> std::size_t { return _devices.size(); }

  auto applications() const -> const std::vector<Application> & {
    return _applications;
  }

  auto ipv4() const -> ns3::Ptr<ns3::Ipv4> { return _ipv4; }

  auto ipv6() const -> ns3::Ptr<ns3::Ipv6> { return _ipv6; }

  static Node create(const parser::NodeDescription &description);

 private:
  Node(ns3::Ptr<ns3::Node> node) : _node{std::move(node)} {}

  void attach(Device &&device);

  void attach(Application &&app);

  ns3::Ptr<ns3::Node> _node;
  std::vector<Device> _devices;
  std::vector<Application> _applications;

  ns3::Ptr<ns3::Ipv4> _ipv4;
  ns3::Ptr<ns3::Ipv6> _ipv6;
};

};  // namespace model

#endif  // __NODE_H_PWWEHSK528G5__
