#include "model.h"

#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/split.hpp>

#include <ns3/csma-helper.h>
#include <ns3/csma-net-device.h>
#include <ns3/drop-tail-queue.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/mac48-address.h>
#include <ns3/names.h>
#include <ns3/net-device.h>
#include <ns3/object-factory.h>
#include <ns3/packet.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/ptr.h>
#include <ns3/string.h>

#include <fmt/core.h>

#include "device.h"
#include "model/model_build_error.h"
#include "parser/parser.h"

namespace model {

void Model::build_from_description(
    const parser::ModelDescription &description) {
  // TODO: extract

  // Create nodes
  for (const auto &node_desc : description.nodes) {
    auto node = Node::create(node_desc);
    _node_per_name[node->name()] = node.get();
    _nodes.push_back(std::move(node));
  }

  // Create connections
  for (const auto &connection : description.connections) {
    auto channel = Channel::create(connection);

    for (const auto &interface : connection.interfaces) {
      // interface format is {node_name}/{interface_name}
      auto separator = interface.find_first_of('/');
      auto node_name = interface.substr(0, separator);
      auto interface_name = interface.substr(separator + 1);

      auto *node = find_node(node_name);
      if (node != nullptr) {
        auto *device = node->get_device_by_name(interface_name);
        if (device != nullptr) {
          device->attach(channel);
        } else {
          throw ModelBuildError(fmt::format(
              R"(Failed create connection: Unknown interface of "{}" with name "{}")",
              node_name, interface_name));
        }
      } else {
        throw ModelBuildError(fmt::format(
            "Failed create connection: Unknown node with name \"{}\"",
            node_name));
      }
    }
  }

  // TODO: create registrators

}

Node *Model::find_node(const std::string &name) const {
  if (auto it = _node_per_name.find(name); it != _node_per_name.end()) {
    return it->second;
  }
  return nullptr;
}

}  // namespace model