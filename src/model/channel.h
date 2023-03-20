#ifndef __CHANNEL_H_5R0UZOSTZ1NM__
#define __CHANNEL_H_5R0UZOSTZ1NM__

#include <optional>
#include <string_view>

#include <ns3/channel.h>

namespace parser {
struct ConnectionDescription;
}

namespace model {

enum class channel_type { Undefined, CSMA, PPP };

class Channel {
 public:
  static auto create(const parser::ConnectionDescription &description)
      -> std::shared_ptr<Channel>;

  auto get() const -> ns3::Ptr<ns3::Channel> { return _channel; }

  auto type() const -> channel_type { return _type; }

 private:
  Channel(const ns3::Ptr<ns3::Channel> &channel, std::string name,
          channel_type type)
      : _name{std::move(name)}, _channel{channel}, _type{type} {}

  std::string _name;
  ns3::Ptr<ns3::Channel> _channel;
  channel_type _type;
};

constexpr auto channel_from_string(std::string_view str) noexcept
    -> std::optional<channel_type> {
  if (str == "Csma") {
    return channel_type::CSMA;
  }

  if (str == "Ppp") {
    return channel_type::PPP;
  }

  return {};
}

}  // namespace model

#endif  // __CHANNEL_H_5R0UZOSTZ1NM__
