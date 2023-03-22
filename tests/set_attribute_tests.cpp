#include <ns3/channel.h>
#include <ns3/data-rate.h>
#include <ns3/nstime.h>
#include <ns3/uinteger.h>

#include <gtest/gtest.h>

#include "utils/object.h"

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

TEST(ObjectUtils, Create) {  // NOLINT
  auto object = utils::create("ns3::CsmaChannel");
  ASSERT_TRUE(object != nullptr);
}

TEST(ObjectUtils, CreateWithAttributes) {  // NOLINT
  utils::Attributes attributes = {{"Delay", "10ms"}};
  auto object = utils::create("ns3::CsmaChannel", attributes);

  ASSERT_TRUE(object != nullptr);
  EXPECT_ATTRIBUTE_EQ<ns3::TimeValue>(object, "Delay", ns3::Time{"10ms"});
}

TEST(ObjectUtils, ThrowOnBadTypeId) {  // NOLINT
  EXPECT_THROW(utils::create("BadTypeId"), utils::BadTypeId);
}

TEST(ObjectUtils, ThrowOnUnknownAttribute) {  // NOLINT
  utils::Attributes attributes = {{"ATTRIBUTE", "XXX"}};
  EXPECT_THROW(utils::create("ns3::CsmaChannel", attributes),
               utils::BadAttribute);
}

// TODO: test on only get options

TEST(ObjectUtils, DynamicCast) {  // NOLINT
  utils::Attributes attributes = {{"Delay", "10ms"}};
  auto channel = utils::create<ns3::Channel>("ns3::CsmaChannel", attributes);
  ASSERT_TRUE(channel != nullptr);
}

TEST(ObjectUtils, SetAttributes) { // NOLINT
  utils::Attributes attributes = {{"Delay", "10ms"}};
  auto object = utils::create("ns3::CsmaChannel");

  ASSERT_TRUE(object != nullptr);
  EXPECT_ATTRIBUTE_NE<ns3::TimeValue>(object, "Delay", ns3::Time{"10ms"});

  utils::set_attributes(object, attributes);
  EXPECT_ATTRIBUTE_EQ<ns3::TimeValue>(object, "Delay", ns3::Time{"10ms"});
}

TEST(ObjectUtils, SetAttributesThrow) { // NOLINT
  utils::Attributes attributes = {{"XXX", "10ms"}};
  auto object = utils::create("ns3::CsmaChannel");

  ASSERT_TRUE(object != nullptr);
  EXPECT_ATTRIBUTE_NE<ns3::TimeValue>(object, "Delay", ns3::Time{"10ms"});

  EXPECT_THROW(utils::set_attributes(object, attributes), utils::BadAttribute);
}