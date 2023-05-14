#ifndef __OBJECT_H_1UBSBJIJJS62__
#define __OBJECT_H_1UBSBJIJJS62__

#include <stdexcept>

#include <ns3/object-factory.h>
#include <ns3/object.h>
#include <ns3/string.h>
#include <ns3/type-id.h>

#include <fmt/core.h>

namespace utils {

using Attributes = std::map<std::string, std::string>;

class BadTypeId : public std::logic_error {
 public:
  explicit BadTypeId(const std::string &type)
      : std::logic_error{fmt::format("Can't find typeid \"{}\"", type)},
        type{type} {}

  std::string type;
};

class BadAttribute : public std::logic_error {
 public:
  explicit BadAttribute(const std::string &attribute, const std::string &value)
      : std::logic_error{fmt::format(
            R"(Bad access to attribute "{}" with value "{}")", attribute,
            value)},
        attribute{attribute},
        value{value} {}

  std::string attribute;
  std::string value;
};

/**
 * @brief Set the attributes object
 *
 * Helper function to set attributes of ns-3 object by using
 *
 * @param object
 * @param attributes
 * @throws BadAttribute if can't set attribute
 */
inline void set_attributes(ns3::Ptr<ns3::Object> object,
                           const Attributes &attributes) {
  for (const auto &[key, value] : attributes) {
    if (!object->SetAttributeFailSafe(key, ns3::StringValue(value))) {
      throw BadAttribute(key, value);
    }
  }
}

/**
 * @brief Create ns3::Object with specifies type_id and attributes
 *
 * @param type_id type indentifier
 * @param attributes map of attributes
 * @return ns3::Ptr<ns3::Object>
 * @throws BadTypeId if there no such `type_id`
 */
inline auto create(const std::string &type_id,
                   const Attributes &attributes = {}) -> ns3::Ptr<ns3::Object> {
  ns3::TypeId id{};
  if (!ns3::TypeId::LookupByNameFailSafe(type_id, &id)) {
    throw BadTypeId(type_id);
  }

  ns3::ObjectFactory factory;
  factory.SetTypeId(type_id);
  auto object = factory.Create();
  set_attributes(object, attributes);

  return object;
}

/**
 * @brief Create ns3::Object and cast it to type T
 *
 * @tparam T
 * @param type_id
 * @param attributes
 * @return ns3::Ptr<T>
 */
template <typename T>
inline auto create(const std::string &type_id,
                   const Attributes &attributes = {}) -> ns3::Ptr<T> {
  return create(type_id, attributes)->GetObject<T>();
}

}  // namespace utils

#endif  // __OBJECT_H_1UBSBJIJJS62__
