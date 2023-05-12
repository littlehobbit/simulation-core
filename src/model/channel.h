#ifndef __CHANNEL_H_5R0UZOSTZ1NM__
#define __CHANNEL_H_5R0UZOSTZ1NM__

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <boost/algorithm/string/case_conv.hpp>

#include <ns3/channel.h>
#include <ns3/ptr.h>

namespace parser {
struct ConnectionDescription;
}

namespace model {

enum class channel_type { Undefined, CSMA, PPP };

class Channel {
 public:
  Channel(const ns3::Ptr<ns3::Channel> &channel, std::string name,
          channel_type type)
      : _name{std::move(name)}, _channel{channel}, _type{type} {}

  static auto create(const parser::ConnectionDescription &description)
      -> std::shared_ptr<Channel>;

  auto get() const -> ns3::Ptr<ns3::Channel> { return _channel; }

  auto type() const -> channel_type { return _type; }

  auto name() const -> const std::string & { return _name; }

 private:
  std::string _name;
  ns3::Ptr<ns3::Channel> _channel;
  channel_type _type;
};

inline auto channel_from_string(const std::string& str) noexcept
    -> std::optional<channel_type> {
  auto type = boost::algorithm::to_lower_copy(str);
  if (type == "csma") {
    return channel_type::CSMA;
  }

  if (type == "ppp") {
    return channel_type::PPP;
  }

  return {};
}

}  // namespace model

#endif  // __CHANNEL_H_5R0UZOSTZ1NM__
