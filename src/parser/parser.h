#ifndef __PARSER_H_D9TYRJ9QIR76__
#define __PARSER_H_D9TYRJ9QIR76__

#include <cstdint>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "utils/address.h"
#include "model/channel.h"

namespace tinyxml2 {
class XMLElement;
}

namespace parser {

using Attributes = std::map<std::string, std::string>;

struct DeviceDescription {
  std::uint64_t id;
  std::string name;
  std::string type;

  std::vector<address::network_v4> ipv4_addresses;
  std::vector<address::network_v6> ipv6_addresses;

  Attributes attributes;
};

struct ApplicationDescription {
  std::string name;
  std::string type;
  Attributes attributes;
};

struct Ipv4Route {
  address::network_v4 network;
  std::string interface;
  std::uint8_t metric;
};

struct Ipv6Route {
  address::network_v6 network;
  std::string interface;
  std::uint8_t metric;
};

struct RoutingDescription {
  std::vector<Ipv4Route> ipv4;
  std::vector<Ipv6Route> ipv6;
};

struct NodeDescription {
  std::uint64_t id;
  std::string name;
  std::vector<DeviceDescription> devices;
  std::vector<ApplicationDescription> applications;
  RoutingDescription routing;
};

struct ConnectionDescription {
  std::uint64_t id;
  std::string name;
  model::channel_type type;
  std::vector<std::string> interfaces;
  Attributes attributes;
};

struct RegistratorDescription {
  std::string source;
  std::string type;
  std::string sink = "Output";
  std::string value_name = "value";
  std::string file;
  std::string start_time;
  std::optional<std::string> end_time;
};

struct ModelDescription {
  std::string model_name;
  bool polulate_tables = false;
  // TODO: add simulation end time

  std::vector<NodeDescription> nodes;
  std::vector<ConnectionDescription> connections;
  std::vector<RegistratorDescription> registrators;
};

class ParseError : public std::runtime_error {
 public:
  explicit ParseError(const std::string &what) : std::runtime_error(what) {}
};

class AttributeError final : public ParseError {
 public:
  AttributeError(std::string_view what, std::string_view tag,
                 std::string_view attribute, std::uint64_t line_num);
  AttributeError(std::string_view what, std::string_view attribute,
                 const tinyxml2::XMLElement *element);
};

class XmlParser {
 public:
  XmlParser() = default;

  ModelDescription parse(const std::string &xml);

 private:
  void parse_model_settings(const tinyxml2::XMLElement *root,
                            ModelDescription &description);

  auto parse_nodes(const tinyxml2::XMLElement *root)
      -> std::vector<NodeDescription>;

  auto parse_devices(const tinyxml2::XMLElement *node)
      -> std::vector<DeviceDescription>;

  auto parse_addresses(const tinyxml2::XMLElement *device)
      -> std::pair<std::vector<address::network_v4>,
                   std::vector<address::network_v6>>;

  auto parse_attributes(const tinyxml2::XMLElement *element) -> Attributes;

  auto parse_applications(const tinyxml2::XMLElement *node)
      -> std::vector<ApplicationDescription>;

  auto parse_routing(const tinyxml2::XMLElement *node) -> RoutingDescription;

  auto parse_connections(const tinyxml2::XMLElement *model)
      -> std::vector<ConnectionDescription>;

  auto parse_interfaces(const tinyxml2::XMLElement *connection)
      -> std::vector<std::string>;

  auto parse_statistics(const tinyxml2::XMLElement *root)
      -> std::vector<RegistratorDescription>;
};

};  // namespace parser

#endif  // __PARSER_H_D9TYRJ9QIR76__
