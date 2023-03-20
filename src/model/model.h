#ifndef __MODEL_H_VUYG9FKANX5V__
#define __MODEL_H_VUYG9FKANX5V__

#include <vector>

#include <ns3/channel-list.h>
#include <ns3/net-device.h>
#include <ns3/node-container.h>
#include <ns3/node.h>
#include <ns3/ptr.h>

namespace parser {
struct ModelDescription;
struct NodeDescription;
};  // namespace parser

namespace model {

class Model {
 public:
  void build_from_description(const parser::ModelDescription &description);

 private:
};

}  // namespace model

#endif  // __MODEL_H_VUYG9FKANX5V__
