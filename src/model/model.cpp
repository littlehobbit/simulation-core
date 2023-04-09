#include "model.h"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include <ns3/ipv4-global-routing-helper.h>
#include <ns3/nstime.h>
#include <ns3/show-progress.h>
#include <ns3/simulator.h>

#include <fmt/core.h>

#include "device.h"
#include "model/channel.h"
#include "model/model_build_error.h"
#include "model/node.h"
#include "model/registrator.h"
#include "parser/parser.h"

namespace model {

void Model::build_from_description(
    const parser::ModelDescription &description) {
  _end_time = ns3::Time{description.end_time};

  // TODO: extract methods

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

  // Create registrators
  for (const auto &desc : description.registrators) {
    auto registrator = Registrator::create(desc);
    registrator->shedule_init();
    _registrators.push_back(std::move(registrator));
  }

  if (description.polulate_tables) {
    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  }

  set_resulution(description.time_precision);
}

Node *Model::find_node(const std::string &name) const {
  if (auto it = _node_per_name.find(name); it != _node_per_name.end()) {
    return it->second;
  }
  return nullptr;
}

void Model::start() {
  std::unique_ptr<ns3::ShowProgress> progress_shower;
  if (_end_time != ns3::Time{}) {
    progress_shower = std::make_unique<ns3::ShowProgress>();
    progress_shower->SetVerbose(true);
    progress_shower->SetStream(std::cout);
    ns3::Simulator::Stop(_end_time);
  }

  ns3::Simulator::Run();
}

void Model::stop() { ns3::Simulator::Stop(); }

void Model::set_resulution(ns3::Time::Unit resulution) {
  ns3::Time::SetResolution(resulution);
}

}  // namespace model