#include "parser.h"

#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

#include <tinyxml2.h>

#include "address.h"
#include "parser/parse_util.h"

using namespace std::literals;

namespace parser {

constexpr auto model_tag = "model";
constexpr auto populate_tag = "populate-routing-tables";
constexpr auto node_tag = "node";
constexpr auto device_list_tag = "device-list";
constexpr auto device_tag = "device";
constexpr auto address_tag = "address";
constexpr auto attributes_tag = "attributes";
constexpr auto attribute_tag = "attribute";
constexpr auto applications_tag = "applications";
constexpr auto application_tag = "application";
constexpr auto routing_tag = "routing";
constexpr auto route_tag = "route";
constexpr auto connections_tag = "connections";
constexpr auto connection_tag = "connection";
constexpr auto interfaces_tag = "interfaces";
constexpr auto interface_tag = "interface";
constexpr auto statistics_tag = "statistics";
constexpr auto registrator_tag = "registrator";

constexpr auto id_attr = "id";
constexpr auto name_attr = "name";
constexpr auto type_attr = "type";
constexpr auto mac_attr = "mac";
constexpr auto value_attr = "value";
constexpr auto prefix_attr = "prefix";
constexpr auto netmask_attr = "netmask";
constexpr auto key_attr = "key";
constexpr auto network_attr = "network";
constexpr auto metric_attr = "metric";
constexpr auto dst_attr = "dst";
constexpr auto file_attr = "file";
constexpr auto source_attr = "source";
constexpr auto start_attr = "start";
constexpr auto end_attr = "end";

using util::get_attribute;
using util::xml_element_range;

ModelDescription XmlParser::parse(const std::string &xml) {
  ModelDescription description;

  tinyxml2::XMLDocument doc;

  if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS) {
    throw ParseError(doc.ErrorStr());
  }

  const auto *root = doc.RootElement();
  if (root->Name() != std::string_view{model_tag}) {
    throw ParseError("No root tag <module>");
  }

  // Parsing model settings
  parse_model_settings(root, description);
  description.nodes = parse_nodes(root);
  description.connections = parse_connections(root);
  description.registrators = parse_statistics(root);

  return description;
}

void XmlParser::parse_model_settings(const tinyxml2::XMLElement *root,
                                     ModelDescription &description) {
  description.model_name = get_attribute<std::string>(root, name_attr);

  const auto *populate = root->FirstChildElement(populate_tag);
  if (populate != nullptr) {
    populate->QueryBoolText(&description.polulate_tables);

    // TODO: maybe delete? ombigious check
    const auto *next = populate->NextSiblingElement(populate_tag);
    if (next != nullptr) {
      throw ParseError("Redefinition of <populate-routing-tables>");
    }
  }

  // TODO: parse simulation end time
}

auto XmlParser::parse_nodes(const tinyxml2::XMLElement *root)
    -> std::vector<NodeDescription> {
  std::vector<NodeDescription> nodes;
  for (const auto &node : xml_element_range(root, node_tag)) {
    // NOTE: doesn't validate uint64 value
    auto id = node.get_attribute<std::uint64_t>(id_attr);
    auto node_name = node.get_attribute<std::string>(name_attr);

    auto devices = parse_devices(node.element);
    auto applications = parse_applications(node.element);
    auto routing = parse_routing(node.element);

    nodes.emplace_back(NodeDescription{.id = id,
                                       .name = std::move(node_name),
                                       .devices = std::move(devices),
                                       .applications = std::move(applications),
                                       .routing = std::move(routing)});
  }
  return nodes;
}

auto XmlParser::parse_devices(const tinyxml2::XMLElement *node)
    -> std::vector<DeviceDescription> {
  std::vector<DeviceDescription> devices;

  const auto *device_list = node->FirstChildElement(device_list_tag);

  if (device_list != nullptr) {
    for (const auto &device : xml_element_range(device_list, device_tag)) {
      auto id = device.get_attribute<std::uint64_t>(id_attr);

      // TODO: how does string allocated?
      auto name = device.get_attribute<std::string>(name_attr);

      auto device_type = device.get_attribute<std::string>(type_attr);
      auto type = device::from_string(device_type);
      if (!type.has_value()) {
        throw ParseError("Bad device type");
      }

      auto [ipv4, ipv6] = parse_addresses(device.element);
      auto attributes = parse_attributes(device.element);

      devices.push_back(DeviceDescription{.id = id,
                                          .name = std::move(name),
                                          .type = *type,
                                          .ipv4_addresses = std::move(ipv4),
                                          .ipv6_addresses = std::move(ipv6),
                                          .attributes = std::move(attributes)});
    }
  }

  return devices;
}

auto XmlParser::parse_addresses(const tinyxml2::XMLElement *device)
    -> std::pair<std::vector<address::Ipv4Network>,
                 std::vector<address::Ipv6Network>> {
  std::vector<address::Ipv4Network> ipv4;
  std::vector<address::Ipv6Network> ipv6;

  for (const auto &address_iter : xml_element_range(device, address_tag)) {
    auto value = address_iter.get_attribute<std::string>(value_attr);
    auto netmask = address_iter.get_attribute<std::string>(netmask_attr, false);

    constexpr auto undefined = 256;
    const auto prefix =
        address_iter.get_attribute<uint32_t>(prefix_attr, false, undefined);

    if (!netmask.empty() && prefix == undefined) {
      // Treat as IPv4
      auto address = address::from_string_v4(value, netmask);
      if (!address.has_value()) {
        throw ParseError("Bad Ipv4");
      }
      ipv4.push_back(*address);
    } else if (netmask.empty() && prefix != undefined) {
      // Treat as IPv6
      auto address = address::from_string_v6(value, prefix);
      if (!address.has_value()) {
        throw ParseError("Bad Ipv6");
      }
      ipv6.push_back(*address);
    } else {
      throw ParseError("Unexpected address");
    }
  }

  return {std::move(ipv4), std::move(ipv6)};
}

auto XmlParser::parse_applications(const tinyxml2::XMLElement *node)
    -> std::vector<ApplicationDescription> {
  std::vector<ApplicationDescription> applications;
  if (const auto *tag = node->FirstChildElement(applications_tag);
      tag != nullptr) {
    for (const auto &app : xml_element_range(tag, application_tag)) {
      auto name = app.get_attribute<std::string>(name_attr);
      auto type = app.get_attribute<std::string>(type_attr);
      auto attributes = parse_attributes(app.element);

      applications.push_back(
          ApplicationDescription{.name = std::string{name},
                                 .type = std::string{type},
                                 .attributes = std::move(attributes)});
    }
  }

  return applications;
}

auto XmlParser::parse_attributes(const tinyxml2::XMLElement *element)
    -> Attributes {
  Attributes attributes;

  const auto *tag = element->FirstChildElement(attributes_tag);
  if (tag == nullptr) {
    return attributes;
  }

  for (const auto &attr_it : xml_element_range(tag, attribute_tag)) {
    auto key = attr_it.get_attribute<std::string>(key_attr);
    auto value = attr_it.get_attribute<std::string>(value_attr);
    attributes.emplace(std::move(key), std::move(value));
  }

  return attributes;
}

auto XmlParser::parse_routing(const tinyxml2::XMLElement *node)
    -> RoutingDescription {
  RoutingDescription routing;
  if (const auto *tag = node->FirstChildElement(routing_tag); tag != nullptr) {
    for (const auto &route : xml_element_range(tag, route_tag)) {
      auto interface = route.get_attribute<std::string>(dst_attr);
      auto network = route.get_attribute<std::string>(network_attr);

      // Default metric in linux is 0
      const auto DEFAULT_METRIC = 0;
      std::uint8_t metric =
          route.get_attribute(metric_attr, false, DEFAULT_METRIC);

      auto netmask = route.get_attribute<std::string>(netmask_attr, false);
      const auto undefined = 256;
      auto prefix = route->UnsignedAttribute(prefix_attr, undefined);

      if (!netmask.empty() && prefix == undefined) {
        auto ipv4 = address::from_string_v4(network, netmask);
        if (ipv4.has_value()) {
          routing.ipv4.push_back(Ipv4Route{.network = *ipv4,
                                           .interface = std::string{interface},
                                           .metric = metric});
        } else {
          throw ParseError("Bad Ipv4 address");
        }
      } else if (netmask.empty() && prefix != undefined) {
        auto ipv6 = address::from_string_v6(network, prefix);
        if (ipv6.has_value()) {
          routing.ipv6.push_back(Ipv6Route{.network = *ipv6,
                                           .interface = std::string{interface},
                                           .metric = metric});
        }
      } else {
        throw ParseError("Bad route network");
      }
    }
  }

  return routing;
}

auto XmlParser::parse_connections(const tinyxml2::XMLElement *model)
    -> std::vector<ConnectionDescription> {
  std::vector<ConnectionDescription> connections;

  const auto *tag = model->FirstChildElement(connections_tag);
  if (tag != nullptr) {
    for (const auto &connection : xml_element_range(tag, connection_tag)) {
      auto name = connection.get_attribute<std::string>(name_attr);
      auto id = connection.get_attribute<std::uint64_t>(id_attr);

      auto type = connection.get_attribute<std::string>(type_attr);
      auto transformed_type = channel::from_string(type);

      // TODO: does it necessary to make checks here?
      if (!transformed_type.has_value()) {
        throw ParseError("Bad type of connection");
      }

      auto interfaces = parse_interfaces(connection.element);
      auto attributes = parse_attributes(connection.element);

      connections.push_back(
          ConnectionDescription{.id = id,
                                .name = std::string{name},
                                .type = *transformed_type,
                                .interfaces = std::move(interfaces),
                                .attributes = std::move(attributes)});
    }
  }

  return connections;
}

auto XmlParser::parse_interfaces(const tinyxml2::XMLElement *connection)
    -> std::vector<std::string> {
  std::vector<std::string> ifs;

  const auto *interfaces = connection->FirstChildElement(interfaces_tag);
  if (interfaces == nullptr) {
    return ifs;
  }

  for (const auto &interface : xml_element_range(interfaces, interface_tag)) {
    ifs.emplace_back(interface->GetText());
  }

  return ifs;
}

auto XmlParser::parse_statistics(const tinyxml2::XMLElement *root)
    -> std::vector<RegistratorDescription> {
  std::vector<RegistratorDescription> registrators;

  const auto *statistics = root->FirstChildElement(statistics_tag);
  if (statistics == nullptr) {
    return registrators;
  }

  for (const auto &registrator :
       xml_element_range(statistics, registrator_tag)) {
    auto type = registrator.get_attribute<std::string>(type_attr);
    auto file = registrator.get_attribute<std::string>(file_attr);
    auto source = registrator.get_attribute<std::string>(source_attr);
    auto start_time = registrator.get_attribute<std::string>(start_attr);

    auto end_time_val = registrator.get_attribute<std::string>(end_attr, false);
    std::optional<std::string> end_time =
        end_time_val.empty() ? std::nullopt : std::optional{end_time_val};

    registrators.push_back(
        RegistratorDescription{.source = std::move(source),
                               .type = std::move(type),
                               .file = std::move(file),
                               .start_time = std::move(start_time),
                               .end_time = end_time});
  }

  return registrators;
}

}  // namespace parser