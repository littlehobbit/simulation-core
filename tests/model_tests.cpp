#include <string_view>

#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/ip/network_v4.hpp>

#include <gtest/gtest.h>
#include <ns3/attribute.h>
#include <ns3/config.h>
#include <ns3/ipv4.h>
#include <ns3/ipv6.h>
#include <ns3/names.h>
#include <ns3/object.h>
#include <ns3/pointer.h>
#include <ns3/string.h>
#include <ns3/uinteger.h>

#include "address.h"
#include "model/application.h"
#include "model/device.h"
#include "model/model.h"
#include "model/model_build_error.h"
#include "model/node.h"
#include "parser/parser.h"
#include <ns3/channel-list.h>
#include <ns3/csma-net-device.h>
#include <ns3/ipv4-address.h>
#include <ns3/ipv4-interface-address.h>
#include <ns3/mac48-address.h>
#include <ns3/net-device.h>
#include <ns3/node-list.h>

template <typename AttributeType, typename ExpectedValue>
void EXPECT_ATTRIBUTE_NE(ns3::Ptr<ns3::Object> object,
                         const std::string& attribute,
                         const ExpectedValue& value) {
  AttributeType attribute_value;
  auto checker = ns3::MakeStringChecker();
  if (object->GetAttributeFailSafe(attribute, attribute_value)) {
    EXPECT_NE(attribute_value.Get(), value);
  } else {
    FAIL() << "Can't get attribute " << attribute;
  }
}

template <typename AttributeType, typename ExpectedValue>
void EXPECT_ATTRIBUTE_EQ(ns3::Ptr<ns3::Object> object,
                         const std::string& attribute,
                         const ExpectedValue& value) {
  AttributeType attribute_value;
  auto checker = ns3::MakeStringChecker();
  if (object->GetAttributeFailSafe(attribute, attribute_value)) {
    EXPECT_EQ(attribute_value.Get(), value);
  } else {
    FAIL() << "Can't get attribute " << attribute;
  }
}

TEST(Node, CreateNode) {  // NOLINT

  parser::ApplicationDescription application_desc{.name = "Client",
                                                  .type = "ns3::UdpEchoClient"};

  parser::DeviceDescription device_desc{
      .name = "eth0",
      .type = "Csma",
      .ipv4_addresses = {boost::asio::ip::make_network_v4("10.10.10.1/16"),
                         boost::asio::ip::make_network_v4("10.20.20.1/24")},
      .ipv6_addresses = {boost::asio::ip::make_network_v6("dead:beef::1/16")},
      .attributes = {{"Mtu", "442"},
                     {"Address", "ab:cd:ef:01:02:03"},
                     {"TxQueue", "ns3::DropTailQueue<Packet>"}}};

  parser::NodeDescription node_desc = {.name = "node",
                                       .devices = {device_desc},
                                       .applications = {application_desc}};

  auto node = model::Node::create(node_desc);

  EXPECT_EQ(ns3::Names::FindName(node.get()), "node");
  EXPECT_TRUE(node.get()->GetObject<ns3::Ipv4>() != nullptr);
  EXPECT_TRUE(node.get()->GetObject<ns3::Ipv6>() != nullptr);

  ASSERT_EQ(node.devices_count(), 1);
  const auto& device = node.get_device(0);

  // Node by default has loopback net-device at index 0
  EXPECT_EQ(node.devices_count(), node.get()->GetNDevices() - 1);
  EXPECT_EQ(device.get(), node.get()->GetDevice(1));

  EXPECT_EQ(ns3::Names::FindPath(device.get()), "/Names/node/eth0");
  EXPECT_EQ(ns3::Names::FindName(device.get()), "eth0");

  // IPv4 addresses
  ASSERT_EQ(node.devices_count(), node.ipv4()->GetNInterfaces() - 1);
  ASSERT_EQ(device.ipv4_addresses().size(), node.ipv4()->GetNAddresses(1));

  ASSERT_EQ(node.ipv4()->GetNAddresses(1), 2);
  EXPECT_EQ(device.ipv4_addresses().at(0), node.ipv4()->GetAddress(1, 0));
  EXPECT_EQ(device.ipv4_addresses().at(1), node.ipv4()->GetAddress(1, 1));

  // IPv6 addresses
  ASSERT_EQ(node.devices_count(), node.ipv6()->GetNInterfaces() - 1);
  ASSERT_EQ(device.ipv6_addresses().size(), node.ipv6()->GetNAddresses(1));

  ASSERT_EQ(node.ipv6()->GetNAddresses(1), 1);
  EXPECT_EQ(device.ipv6_addresses().at(0), node.ipv6()->GetAddress(1, 0));

  // Applications
  ASSERT_EQ(node.applications().size(), 1);
  ASSERT_EQ(node.get()->GetNApplications(), 1);

  const auto& app = node.applications().at(0);
  EXPECT_EQ(app.get(), node.get()->GetApplication(0));

  EXPECT_EQ(ns3::Names::FindName(app.get()), "Client");
  EXPECT_EQ(ns3::Names::FindPath(app.get()), "/Names/node/Client");
}

TEST(Device, CreateCsmaDevice) {  // NOLINT
  parser::DeviceDescription desc{
      .name = "eth0",
      .type = "Csma",
      .ipv4_addresses = {boost::asio::ip::make_network_v4("10.10.10.1/16"),
                         boost::asio::ip::make_network_v4("10.20.20.1/24")},
      .ipv6_addresses = {boost::asio::ip::make_network_v6("dead:beef::1/16")},
      .attributes = {{"Mtu", "442"},
                     {"Address", "ab:cd:ef:01:02:03"},
                     {"TxQueue", "ns3::DropTailQueue<Packet>"}}};

  auto device = model::Device::create(desc);

  ASSERT_TRUE(device.get() != nullptr);
  ASSERT_TRUE(device.get()->GetObject<ns3::CsmaNetDevice>() != nullptr);

  EXPECT_ATTRIBUTE_EQ<ns3::StringValue>(device.get(), "Mtu", "442");
  EXPECT_ATTRIBUTE_EQ<ns3::Mac48AddressValue>(
      device.get(), "Address", ns3::Mac48Address("ab:cd:ef:01:02:03"));
  EXPECT_ATTRIBUTE_NE<ns3::StringValue>(device.get(), "TxQueue", "0");

  EXPECT_EQ(device.name(), "eth0");

  // TODO: check converted addresses
  auto expexted_ipv4 = std::vector<ns3::Ipv4InterfaceAddress>{
      ns3::Ipv4InterfaceAddress("10.10.10.1", "255.255.0.0"),
      ns3::Ipv4InterfaceAddress("10.20.20.1", "255.255.255.0")};
  EXPECT_EQ(device.ipv4_addresses(), expexted_ipv4);

  auto expexted_ipv6 = std::vector<ns3::Ipv6InterfaceAddress>{
      ns3::Ipv6InterfaceAddress("dead:beef::1", 16)};
  EXPECT_EQ(device.ipv6_addresses(), expexted_ipv6);
}

TEST(Device, BadDeviceType) {  // NOLINT
  parser::DeviceDescription desc{.name = "eth0", .type = ""};
  EXPECT_ANY_THROW(model::Device::create(desc));
}

TEST(Application, Create) {  // NOLINT
  parser::ApplicationDescription description{
      .name = "app",
      .type = "ns3::UdpEchoClient",
      .attributes = {{"MaxPackets", "2"}}};

  auto application = model::Application::create(description);

  ASSERT_TRUE(application.get() != nullptr);

  EXPECT_EQ(application.name(), "app");

  EXPECT_ATTRIBUTE_EQ<ns3::UintegerValue>(application.get(), "MaxPackets", 2);

  // BUG: incorrect "RemoteAddress" deserialization from string
  // EXPECT_ATTRIBUTE_EQ<ns3::StringValue>(application.get(), "RemoteAddress",
  //                                       "01:01:01:01");
}

TEST(Application, ThrowOnBadType) {  // NOLINT
  parser::ApplicationDescription descr{.type = "bad"};
  EXPECT_THROW(model::Application::create(descr), model::ModelBuildError);
}

TEST(Application, ThrowOnBadAttribute) {  // NOLINT
  parser::ApplicationDescription descr{.type = "ns3::UdpEchoClient",
                                       .attributes = {{"bad", "attribute"}}};
  EXPECT_THROW(model::Application::create(descr), model::ModelBuildError);
}

// BUG: bad value makes asserts
// TEST(Applcation, BadAttributeValue) { // NOLINT
//   parser::ApplicationDescription description{
//       .name = "app",
//       .type = "ns3::UdpEchoClient",
//       .attributes = {{"MaxPackets", "BAD VALUE"}}};
//   EXPECT_THROW(model::Application::create(description),
//   model::ModelBuildError);
// }

// TEST(Model, ErrorOnDeviceWithoutQueue) {}

// TEST(Model, ErrorOnNameDuplication) {}