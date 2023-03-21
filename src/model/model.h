#ifndef __MODEL_H_VUYG9FKANX5V__
#define __MODEL_H_VUYG9FKANX5V__

#include <optional>
#include <vector>

#include <ns3/channel-list.h>
#include <ns3/net-device.h>
#include <ns3/node-container.h>
#include <ns3/node.h>
#include <ns3/ptr.h>

#include "node.h"

namespace parser {
struct ModelDescription;
struct NodeDescription;
};  // namespace parser

namespace model {

class Model {
 public:
  void build_from_description(const parser::ModelDescription &description);

  Node *find_node(const std::string &name) const;

 private:
  std::vector<std::unique_ptr<Node>> _nodes;
  std::map<std::string, Node *> _node_per_name;
};

}  // namespace model

#endif  // __MODEL_H_VUYG9FKANX5V__
