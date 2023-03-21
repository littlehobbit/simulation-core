#include <exception>

#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/ip/network_v6.hpp>

#include <gtest/gtest.h>
#include <tinyxml2.h>

#include "model/channel.h"
#include "parser/parse_util.h"
#include "parser/parser.h"

TEST(TinyXmlTest, ReadingBoolValue) {  // NOLINT
  const auto* xml =
      R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <valt value="true"/>
        <valf value="false"/>
        <valtt>True</valtt>
        <valft>False</valft>
      )";

  tinyxml2::XMLDocument doc;
  ASSERT_EQ(doc.Parse(xml), tinyxml2::XML_SUCCESS);

  EXPECT_TRUE(doc.FirstChildElement("valt")->BoolAttribute("value"));
  EXPECT_TRUE(doc.FirstChildElement("valtt")->BoolText());

  EXPECT_FALSE(doc.FirstChildElement("valf")->BoolAttribute("value"));
  EXPECT_FALSE(doc.FirstChildElement("valft")->BoolText());
}

TEST(XmlParse, ThrowOnIncorrectXml) {  // NOLINT
  parser::XmlParser parser;

  const auto* xml =
      R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <model
        </model>
      )";

  ASSERT_THROW(parser.parse(xml), parser::ParseError);
}

// NOTE: maybe divide this test-case to
//  - test case of node
//  - test case of devices
//  - test case of addresses
//  - test case of attributes
TEST(XmlParse, ReadingNode) {  // NOLINT
  parser::XmlParser parser;

  const auto* xml =
      R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <model name="CsmaNetworkModel">
          <populate-routing-tables>true</populate-routing-tables>
          
          <node id="0" name="test">
            <device-list>
              <device id="0" name="eth0" type="Csma" mac="AB:CD:EF:01:02:03">
                <address value="10.1.22.222" netmask="255.255.255.0"/>
                <address value="2022:dead:beef:2023::1" prefix="64"/>

                <attributes>
                  <attribute key="Mtu" value="1200"/>
                </attributes>
              </device>
            </device-list>
            
            <applications>
              <application name="EchoClient" type="ns3::UdpEchoClientApplication">
                <attributes>
                  <attribute key="Port" value="666"/>
                  <attribute key="Remote" value="10.1.22.2"/>
                </attributes>
              </application>
            </applications>

            <routing>
              <route network="10.101.0.0" netmask="255.255.0.0" dst="eth0" metric="10"/>
              <route network="2001:dead:beef:1002::0" prefix="64" dst="eth1" metric="30"/>
            </routing>

          </node>
        
        </model>
    )";

  // TODO: extract check for model settings
  auto res = parser.parse(xml);
  EXPECT_EQ(res.model_name, "CsmaNetworkModel");
  EXPECT_TRUE(res.polulate_tables);

  ASSERT_EQ(res.nodes.size(), 1);
  auto& node = res.nodes.front();
  EXPECT_EQ(node.id, 0);
  EXPECT_EQ(node.name, "test");

  // devices
  ASSERT_EQ(node.devices.size(), 1);
  auto& device = node.devices.front();
  EXPECT_EQ(device.id, 0);
  EXPECT_EQ(device.name, "eth0");
  EXPECT_EQ(device.type, "Csma");

  ASSERT_EQ(device.ipv4_addresses.size(), 1);
  EXPECT_EQ(device.ipv4_addresses.front().address(),
            asio::ip::make_address_v4("10.1.22.222"));
  EXPECT_EQ(device.ipv4_addresses.front().prefix_length(), 24);

  ASSERT_EQ(device.ipv6_addresses.size(), 1);
  EXPECT_EQ(device.ipv6_addresses.front().address(),
            asio::ip::make_address_v6("2022:dead:beef:2023::1"));
  EXPECT_EQ(device.ipv6_addresses.front().prefix_length(), 64);

  ASSERT_FALSE(device.attributes.empty());
  EXPECT_EQ(device.attributes["Mtu"], "1200");

  // applications
  ASSERT_FALSE(node.applications.empty());
  auto& applications = node.applications;
  EXPECT_EQ(applications.front().name, "EchoClient");
  EXPECT_EQ(applications.front().type, "ns3::UdpEchoClientApplication");

  ASSERT_FALSE(applications.front().attributes.empty());
  EXPECT_EQ(applications.front().attributes["Port"], "666");

  // routing
  ASSERT_FALSE(node.routing.ipv4.empty());
  EXPECT_EQ(node.routing.ipv4.front().network.to_string(), "10.101.0.0/16");

  ASSERT_FALSE(node.routing.ipv6.empty());
  EXPECT_EQ(node.routing.ipv6.front().network.to_string(),
            "2001:dead:beef:1002::/64");
}

TEST(XmlParse, ReadsConnections) {  // NOLINT
  parser::XmlParser parser;

  const auto* xml =
      R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <model name="test_model">
          <connections>
            <connection id="0" name="test" type="Csma">
              <interfaces>
                <interface>eth0</interface>
                <interface>eth1</interface>
              </interfaces>

              <attributes>
                <attribute key="Key" value="Value"/>
              </attributes>
            </connection>
          </connections>
        
        </model>
    )";

  auto result = parser.parse(xml);
  ASSERT_FALSE(result.connections.empty());

  const auto& connection = result.connections.front();
  EXPECT_EQ(connection.name, "test");
  EXPECT_EQ(connection.type, model::channel_type::CSMA);
  EXPECT_EQ(connection.interfaces, (std::vector<std::string>{"eth0", "eth1"}));

  ASSERT_EQ(connection.attributes.at("Key"), "Value");
}

TEST(XmlParse, ReadsRegistrators) {  // NOLINT
  parser::XmlParser parser;

  const auto* xml =
      R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <model name="CsmaNetworkModel">
          <statistics>
            <registrator source="test-source"
               type="TestType"
               file="test-1"
               value_name="CWND"
               start="0s"/>

              <registrator source="test-source"
               type="TestType"
               file="test-2"
               start="0s"
               end="2s"/>
          </statistics>
        </model>
    )";

  auto result = parser.parse(xml);
  ASSERT_EQ(result.registrators.size(), 2);

  auto& first = result.registrators[0];
  EXPECT_EQ(first.source, "test-source");
  EXPECT_EQ(first.type, "TestType");
  EXPECT_EQ(first.file, "test-1");
  EXPECT_EQ(first.start_time, "0s");
  EXPECT_FALSE(first.end_time.has_value());
  EXPECT_EQ(first.value_name, "CWND");

  auto& second = result.registrators[1];
  EXPECT_EQ(second.source, "test-source");
  EXPECT_EQ(second.type, "TestType");
  EXPECT_EQ(second.file, "test-2");
  EXPECT_EQ(second.start_time, "0s");
  EXPECT_EQ(second.end_time, "2s");
  EXPECT_EQ(second.value_name, "value");  // by default
}

TEST(XmlParse, IncorrectNodeReading) {  // NOLINT
  parser::XmlParser parser;

  {
    // none id
    const auto* xml =
        R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <model name="CsmaNetworkModel">
          <node name="test">
          </node>
        </model>
    )";

    EXPECT_THROW(parser.parse(xml), parser::ParseError);
  }

  {
    // none name
    const auto* xml =
        R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <model name="CsmaNetworkModel">
          <node id="0">
          </node>
        </model>
    )";
    EXPECT_THROW(parser.parse(xml), parser::ParseError);
  }
}

TEST(XmlParse, ErrorOnMultiplePopulate) {  // NOLINT
  parser::XmlParser parser;

  const auto* xml =
      R"(
      <?xml version="1.0" encoding="UTF-8"?>
      <model name="CsmaNetworkModel">
      <populate-routing-tables>true</populate-routing-tables>
      <populate-routing-tables>true</populate-routing-tables>
      <populate-routing-tables>true</populate-routing-tables>
      <populate-routing-tables>true</populate-routing-tables>
      <populate-routing-tables>true</populate-routing-tables>
      </model>
    )";

  ASSERT_THROW(parser.parse(xml), parser::ParseError);  // NOLINT
}

TEST(XmlParse, BadAttributes) {  // NOLINT
  parser::XmlParser parser;

  {
    const auto* no_value =
        R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <model name="CsmaNetworkModel">
          <populate-routing-tables>true</populate-routing-tables>
          
          <node id="0" name="test">
            <device-list>
              <device id="0" name="eth0" type="Csma">
                <attributes>
                  <attribute key="Mtu"/>
                </attributes>
              </device>
            </device-list>
          </node>
        </model>
    )";
    EXPECT_THROW(parser.parse(no_value), parser::ParseError);
  }

  {
    const auto* no_key =
        R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <model name="CsmaNetworkModel">
          <populate-routing-tables>true</populate-routing-tables>
          
          <node id="0" name="test">
            <device-list>
              <device id="0" name="eth0" type="Csma">
                <attributes>
                  <attribute value="2"/>
                </attributes>
              </device>
            </device-list>
          </node>
        </model>
    )";
    EXPECT_THROW(parser.parse(no_key), parser::ParseError);
  }
}

TEST(XmlParse, ModelRequiresName) {  // NOLINT
  parser::XmlParser parser;

  const auto* xml =
      R"(
      <?xml version="1.0" encoding="UTF-8"?>
      <model>
      </model>
    )";

  ASSERT_THROW(parser.parse(xml), parser::ParseError);
}

TEST(XmlParse, ErrorOnNoModelTag) {  // NOLINT
  parser::XmlParser parser;

  const auto* xml =
      R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <node-list>
        </node-list>
      )";

  ASSERT_THROW(parser.parse(xml), parser::ParseError);
}

TEST(XmlParseUtil, ExtractAttribute) {  // NOLINT
  constexpr auto xml = R"(
    <item value_int="42" value_str="string"/>
  )";

  tinyxml2::XMLDocument doc;
  doc.Parse(xml);

  ASSERT_EQ(parser::util::get_attribute<int>(doc.RootElement(), "value_int"),
            42);
  ASSERT_STREQ(
      parser::util::get_attribute<const char*>(doc.RootElement(), "value_str"),
      "string");
}

TEST(XmlParseUtil, ThrowOnRequiredMissing) {  // NOLINT
  constexpr auto xml = R"(
    <item value=""/>
  )";

  tinyxml2::XMLDocument doc;
  doc.Parse(xml);

  EXPECT_THROW(parser::util::get_attribute<int>(doc.RootElement(), "missed"),
               parser::ParseError);
}

TEST(XmlParseUtil, DefaultOnNonRequiredMissing) {  // NOLINT
  constexpr auto xml = R"(
    <item value=""/>
  )";

  tinyxml2::XMLDocument doc;
  doc.Parse(xml);

  ASSERT_EQ(
      parser::util::get_attribute<int>(doc.RootElement(), "missed", false),
      int{});
  ASSERT_EQ(
      parser::util::get_attribute<int>(doc.RootElement(), "missed", false, 42),
      42);
}

TEST(XmlParseUtil, ThrowOnMissedType) {  // NOLINT
  constexpr auto xml = R"(
    <item value="string"/>
  )";

  tinyxml2::XMLDocument doc;
  doc.Parse(xml);

  EXPECT_THROW(
      parser::util::get_attribute<int>(doc.RootElement(), "value", true),
      parser::ParseError);
  EXPECT_THROW(
      parser::util::get_attribute<int>(doc.RootElement(), "value", false),
      parser::ParseError);
}