#include <fmt/core.h>
#include <gtest/gtest.h>
#include <ns3/names.h>
#include <ns3/node.h>
#include <ns3/ptr.h>

#include "model/name_service.h"

using namespace model::names;

TEST(NameService, AddName) {
  auto node = ns3::CreateObject<ns3::Node>();
  const std::string name = "node_name";
  add(node, name);
  EXPECT_EQ(ns3::Names::FindName(node), name);

  cleanup();
}

TEST(NameService, NameDuplication) {
  auto node_1 = ns3::CreateObject<ns3::Node>();
  auto node_2 = ns3::Create<ns3::Node>();

  const std::string name = "node_name";
  add(node_1, name);
  EXPECT_THROW(add(node_2, name), DuplicateError);

  cleanup();
}

TEST(NameService, ContextAddName) {
  auto node = ns3::CreateObject<ns3::Node>();
  const std::string name = "node_name";
  const std::string sub_name = "sub_name";

  add(node, name);

  auto sub_node = ns3::CreateObject<ns3::Node>();
  add(node, sub_node, sub_name);

  EXPECT_EQ(ns3::Names::FindName(sub_node), sub_name);
  EXPECT_EQ(ns3::Names::FindPath(sub_node),
            fmt::format("/Names/{}/{}", name, sub_name));

  cleanup();
}

TEST(NameService, ContextNameDuplication) {
  auto node = ns3::CreateObject<ns3::Node>();
  const std::string name = "node_name";
  const std::string sub_name = "sub_name";

  add(node, name);

  auto sub_node_1 = ns3::CreateObject<ns3::Node>();
  auto sub_node_2 = ns3::CreateObject<ns3::Node>();

  add(node, sub_node_1, sub_name);
  EXPECT_THROW(add(node, sub_node_2, sub_name), DuplicateError);

  cleanup();
}