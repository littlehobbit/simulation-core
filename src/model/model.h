#ifndef __MODEL_H_VUYG9FKANX5V__
#define __MODEL_H_VUYG9FKANX5V__

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <ns3/nstime.h>

#include "node.h"

namespace parser {
struct ModelDescription;
}

namespace model {

class Registrator;

class Model {
 public:
  void build_from_description(const parser::ModelDescription &description);

  Node *find_node(const std::string &name) const;

  void start();

  void stop();

  auto get_registrators() const
      -> const std::vector<std::shared_ptr<Registrator>> & {
    return _registrators;
  }

  void set_resulution(ns3::Time::Unit resulution);

 private:
  std::vector<std::unique_ptr<Node>> _nodes;
  std::map<std::string, Node *> _node_per_name;
  std::vector<std::shared_ptr<Registrator>> _registrators;

  ns3::Time _end_time{};
  ns3::Time::Unit time_resolution = ns3::Time::NS;
};

}  // namespace model

#endif  // __MODEL_H_VUYG9FKANX5V__
