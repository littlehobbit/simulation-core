#include <string_view>

#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/ip/network_v4.hpp>

#include <ns3/attribute.h>
#include <ns3/channel-list.h>
#include <ns3/config.h>
#include <ns3/csma-net-device.h>
#include <ns3/event-id.h>
#include <ns3/ipv4-address.h>
#include <ns3/ipv4-interface-address.h>
#include <ns3/ipv4.h>
#include <ns3/ipv6.h>
#include <ns3/mac48-address.h>
#include <ns3/names.h>
#include <ns3/net-device.h>
#include <ns3/node-list.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/pointer.h>
#include <ns3/string.h>
#include <ns3/uinteger.h>

#include <gtest/gtest.h>

#include "address.h"
#include "model/application.h"
#include "model/channel.h"
#include "model/device.h"
#include "model/model.h"
#include "model/model_build_error.h"
#include "model/name_service.h"
#include "model/node.h"
#include "model/registrator.h"
#include "parser/parser.h"

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

  parser::ApplicationDescription application_desc{
      .name = "Client",
      .type = "ns3::UdpEchoClient",
      .attributes = {{"RemotePort", "6666"}}};

  parser::DeviceDescription device_desc{
      .name = "eth0",
      .type = "Csma",
      .ipv4_addresses = {asio::ip::make_network_v4("10.10.10.1/16"),
                         asio::ip::make_network_v4("10.20.20.1/24")},
      .ipv6_addresses = {asio::ip::make_network_v6("dead:beef::1/16")},
      .attributes = {{"Mtu", "442"},
                     {"Address", "ab:cd:ef:01:02:03"},
                     {"TxQueue", "ns3::DropTailQueue<Packet>"}}};

  parser::NodeDescription node_desc = {.name = "node",
                                       .devices = {device_desc},
                                       .applications = {application_desc}};

  auto node = model::Node::create(node_desc);

  EXPECT_EQ(ns3::Names::FindName(node->get()), "node");
  EXPECT_TRUE(node->get()->GetObject<ns3::Ipv4>() != nullptr);
  EXPECT_TRUE(node->get()->GetObject<ns3::Ipv6>() != nullptr);

  ASSERT_EQ(node->devices_count(), 1);
  const auto& device = node->get_device(0);

  // Node by default has loopback net-device at index 0
  EXPECT_EQ(node->devices_count(), node->get()->GetNDevices() - 1);
  EXPECT_EQ(device.get(), node->get()->GetDevice(1));

  EXPECT_EQ(ns3::Names::FindPath(device.get()), "/Names/node/eth0");
  EXPECT_EQ(ns3::Names::FindName(device.get()), "eth0");

  // IPv4 addresses
  ASSERT_EQ(node->devices_count(), node->ipv4()->GetNInterfaces() - 1);
  ASSERT_EQ(device.ipv4_addresses().size(), node->ipv4()->GetNAddresses(1));

  ASSERT_EQ(node->ipv4()->GetNAddresses(1), 2);
  EXPECT_EQ(device.ipv4_addresses().at(0), node->ipv4()->GetAddress(1, 0));
  EXPECT_EQ(device.ipv4_addresses().at(1), node->ipv4()->GetAddress(1, 1));

  // IPv6 addresses
  ASSERT_EQ(node->devices_count(), node->ipv6()->GetNInterfaces() - 1);
  ASSERT_EQ(device.ipv6_addresses().size(), node->ipv6()->GetNAddresses(1));

  ASSERT_EQ(node->ipv6()->GetNAddresses(1), 1);
  EXPECT_EQ(device.ipv6_addresses().at(0), node->ipv6()->GetAddress(1, 0));

  // Applications
  ASSERT_EQ(node->applications().size(), 1);
  ASSERT_EQ(node->get()->GetNApplications(), 1);

  const auto& app = node->applications().at(0);
  EXPECT_EQ(app.get(), node->get()->GetApplication(0));

  EXPECT_EQ(ns3::Names::FindName(app.get()), "Client");
  EXPECT_EQ(ns3::Names::FindPath(app.get()), "/Names/node/Client");

  EXPECT_ATTRIBUTE_EQ<ns3::UintegerValue>(app.get(), "RemotePort", 6666);

  model::names::cleanup();
}

TEST(Device, CreateCsmaDevice) {  // NOLINT
  parser::DeviceDescription desc{
      .name = "eth0",
      .type = "Csma",
      .ipv4_addresses = {asio::ip::make_network_v4("10.10.10.1/16"),
                         asio::ip::make_network_v4("10.20.20.1/24")},
      .ipv6_addresses = {asio::ip::make_network_v6("dead:beef::1/16")},
      .attributes = {{"Mtu", "442"},
                     {"Address", "ab:cd:ef:01:02:03"},
                     {"TxQueue", "ns3::DropTailQueue<Packet>[MaxSize=100p]"}}};

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

TEST(Application, CreateNonApplication) {  // NOLINT
  parser::ApplicationDescription descr{.type = "ns3::CsmaNetDevice"};
  EXPECT_THROW(model::Application::create(descr), model::ModelBuildError);
}

TEST(Channel, Create) {  // NOLINT
  parser::ConnectionDescription description = {
      .name = "test-connection",
      .type = model::channel_type::CSMA,
      .attributes = {{"Delay", "100ms"}}};

  auto channel = model::Channel::create(description);

  ASSERT_TRUE(channel.get() != nullptr);
  EXPECT_EQ(ns3::Names::FindName(channel->get()), "test-connection");
  EXPECT_EQ(ns3::Names::FindPath(channel->get()), "/Names/test-connection");

  EXPECT_ATTRIBUTE_EQ<ns3::TimeValue>(channel->get(), "Delay",
                                      ns3::Time("100ms"));

  auto channels_count = ns3::ChannelList::GetNChannels();
  ASSERT_TRUE(channels_count > 0);
  EXPECT_EQ(ns3::ChannelList::GetChannel(channels_count - 1), channel->get());

  model::names::cleanup();
}

TEST(Channel, BadAttribute) {  // NOLINT
  parser::ConnectionDescription description = {
      .name = "test-connection",
      .type = model::channel_type::CSMA,
      .attributes = {{"Del_ay", "100__ms"}}};

  EXPECT_THROW(model::Channel::create(description), model::ModelBuildError);
}

TEST(Device, ConnectToChannel) {  // NOLINT
  parser::DeviceDescription device_desc{
      .name = "eth0",
      .type = "Csma",
      .ipv4_addresses = {asio::ip::make_network_v4("10.10.10.1/16"),
                         asio::ip::make_network_v4("10.20.20.1/24")},
      .ipv6_addresses = {asio::ip::make_network_v6("dead:beef::1/16")},
      .attributes = {{"Mtu", "442"},
                     {"Address", "ab:cd:ef:01:02:03"},
                     {"TxQueue", "ns3::DropTailQueue<Packet>"}}};

  parser::ConnectionDescription channel_desc{.name = "from_eth0",
                                             .type = model::channel_type::CSMA};

  auto device = model::Device::create(device_desc);
  auto channel = model::Channel::create(channel_desc);

  device.attach(channel);

  EXPECT_TRUE(device.get()->GetChannel() != nullptr);
  EXPECT_TRUE(device.channel() != nullptr);

  // NEED TO FIND ALL DEVICES WRAPPERS FOR /{node}/{interface_name}
  // Maybe add DeviceStorage::find() for this

  model::names::cleanup();
}

TEST(Device, ErrorOnBadTypes) {  // NOLINT
  parser::DeviceDescription device_desc{
      .name = "eth0",
      .type = "PPP",
      .ipv4_addresses = {asio::ip::make_network_v4("10.10.10.1/16"),
                         asio::ip::make_network_v4("10.20.20.1/24")},
      .ipv6_addresses = {asio::ip::make_network_v6("dead:beef::1/16")},
      .attributes = {{"Mtu", "442"},
                     {"Address", "ab:cd:ef:01:02:03"},
                     {"TxQueue", "ns3::DropTailQueue<Packet>"}}};

  parser::ConnectionDescription channel_desc{.name = "from_eth0",
                                             .type = model::channel_type::CSMA};

  auto device = model::Device::create(device_desc);
  auto channel = model::Channel::create(channel_desc);

  EXPECT_THROW(device.attach(channel), model::ModelBuildError);

  model::names::cleanup();
}

TEST(Model, Build) {  // NOLINT
  parser::NodeDescription node_a_desc = {
      .name = "node_a",
      .devices = {parser::DeviceDescription{
          .name = "eth0",
          .type = "PPP",
          .ipv4_addresses = {asio::ip::make_network_v4("10.10.10.2/24")}}}};

  parser::NodeDescription node_b_desc = {
      .name = "node_b",
      .devices = {parser::DeviceDescription{
          .name = "eth0",
          .type = "PPP",
          .ipv4_addresses = {asio::ip::make_network_v4("10.10.10.4/24")}}}};

  parser::ConnectionDescription connection = {
      .name = "a_to_b",
      .type = model::channel_type::PPP,
      .interfaces = {"node_a/eth0", "node_b/eth0"}};

  parser::RegistratorDescription registrator_desc{
      .source = "/NodeList/*/$ns3::Ipv4L3Protocol/Tx",
      .type = "ns3::Uinteger32Probe",
      .sink = "OutputBytes",
      .start_time = "1s",
  };

  parser::ModelDescription model_desc = {
      .model_name = "model",
      .nodes = {node_a_desc, node_b_desc},
      .connections = {connection},
      .registrators = {registrator_desc}  //
  };

  model::Model model;
  model.build_from_description(model_desc);

  // Create nodes ans connections
  auto* node_a = model.find_node(node_a_desc.name);
  ASSERT_TRUE(node_a != nullptr);
  EXPECT_TRUE(node_a->get_device(0).channel() != nullptr);
  EXPECT_TRUE(node_a->get_device(0).channel() != nullptr);
  EXPECT_TRUE(node_a->get_device(0).get()->GetChannel() != nullptr);

  auto* node_b = model.find_node(node_b_desc.name);
  ASSERT_TRUE(node_b != nullptr);
  EXPECT_TRUE(node_b->get_device(0).channel() != nullptr);
  EXPECT_TRUE(node_b->get_device(0).channel() != nullptr);
  EXPECT_TRUE(node_b->get_device(0).get()->GetChannel() != nullptr);

  // Create registrators
  const auto& registrators = model.get_registrators();
  ASSERT_EQ(registrators.size(), 1);
  EXPECT_TRUE(registrators.at(0)->get_event_id().IsRunning());
  ASSERT_FALSE(registrators.at(0)->get_event_id().IsExpired());
  ASSERT_TRUE(registrators.at(0)->get_event_id().PeekEventImpl() != nullptr);
  EXPECT_FALSE(
      registrators.at(0)->get_event_id().PeekEventImpl()->IsCancelled());

  model::names::cleanup();
}

TEST(Model, BadInterfaces) {  // NOLINT
  parser::NodeDescription node_a_desc = {
      .name = "node_a",
      .devices = {parser::DeviceDescription{
          .name = "eth0",
          .type = "PPP",
          .ipv4_addresses = {asio::ip::make_network_v4("10.10.10.2/24")}}}};

  parser::NodeDescription node_b_desc = {
      .name = "node_b",
      .devices = {parser::DeviceDescription{
          .name = "eth0",
          .type = "PPP",
          .ipv4_addresses = {asio::ip::make_network_v4("10.10.10.4/24")}}}};

  parser::ModelDescription model_desc = {
      .model_name = "model",                //
      .nodes = {node_a_desc, node_b_desc},  //
  };

  {
    model::Model model;
    model_desc.connections = {
        {.name = "a_to_b",
         .type = model::channel_type::PPP,
         .interfaces = {"UNKNOWN_NAME/eth0", "node_b/eth0"}}};
    EXPECT_THROW(model.build_from_description(model_desc),
                 model::ModelBuildError);
    model::names::cleanup();
  }

  {
    model::Model model;
    model_desc.connections = {
        {.name = "a_to_b",
         .type = model::channel_type::PPP,
         .interfaces = {"node_a/eth0", "node_b/INTERFACE"}}};
    EXPECT_THROW(model.build_from_description(model_desc),
                 model::ModelBuildError);
    model::names::cleanup();
  }
}

TEST(Registrator, CreateAndSchedule) {  // NOLINT
  parser::RegistratorDescription desc = {
      .source = "/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/CongestionWindow",
      .type = "ns3::Uinteger32Probe",
      .sink = "Output",
      .file = "file",
      .start_time = "2s",
      .end_time = "3s"};

  auto registrator = model::Registrator::create(desc);
  registrator->shedule_init();

  ASSERT_TRUE(registrator->get_event_id().PeekEventImpl() != nullptr);
  ASSERT_FALSE(registrator->get_event_id().IsExpired());
  ASSERT_FALSE(registrator->get_event_id().PeekEventImpl()->IsCancelled());
}