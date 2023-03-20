#include "model.h"

#include <string>

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

#include "device.h"
#include "parser/parser.h"

namespace model {

void Model::build_from_description(
    const parser::ModelDescription &description) {
  // TODO: create nodes

  

  // TODO: create connections
}

}  // namespace model