#include "channel.h"

#include <ns3/channel.h>
#include <ns3/object.h>

#include <fmt/core.h>

#include "model/model_build_error.h"
#include "model/name_service.h"
#include "parser/parser.h"
#include "utils/object.h"

namespace model {

auto Channel::create(const parser::ConnectionDescription &description)
    -> std::shared_ptr<Channel> {
  try {
    ns3::Ptr<ns3::Channel> channel;
    if (description.type == channel_type::CSMA) {
      channel = utils::create<ns3::Channel>("ns3::CsmaChannel",
                                            description.attributes);
    } else if (description.type == channel_type::PPP) {
      channel = utils::create<ns3::Channel>("ns3::PointToPointChannel",
                                            description.attributes);
    }

    names::add(channel, description.name);
    return std::shared_ptr<Channel>(
        new Channel{channel, description.name, description.type});
  } catch (utils::BadTypeId &bad_type) {
    throw ModelBuildError(
        fmt::format("Can't create channel of type \"{}\"", bad_type.type));
  } catch (utils::BadAttribute &bad_attribute) {
    throw ModelBuildError(
        fmt::format(R"(Failed set attribute "{}" with value "{}")",
                    bad_attribute.attribute, bad_attribute.value));
  }
}

}  // namespace model