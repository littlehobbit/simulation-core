#include "channel.h"

#include <ns3/channel.h>
#include <ns3/csma-channel.h>
#include <ns3/object.h>
#include <ns3/point-to-point-channel.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/string.h>

#include "model/name_service.h"
#include "parser/parser.h"

namespace model {

auto Channel::create(const parser::ConnectionDescription &description)
    -> std::shared_ptr<Channel> {
  ns3::Ptr<ns3::Channel> channel;
  if (description.type == channel_type::CSMA) {
    channel = ns3::CreateObject<ns3::CsmaChannel>();
  } else if (description.type == channel_type::PPP) {
    channel = ns3::CreateObject<ns3::PointToPointChannel>();
  }

  for (const auto &[key, value] : description.attributes) {
    channel->SetAttributeFailSafe(key, ns3::StringValue(value));
  }

  names::add(channel, description.name);
  return std::shared_ptr<Channel>(
      new Channel{channel, description.name, description.type});
}

}  // namespace model