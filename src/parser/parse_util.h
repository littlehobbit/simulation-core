#ifndef __PARSE_UTIL_H_5T1B4GE8TUJ5__
#define __PARSE_UTIL_H_5T1B4GE8TUJ5__

#include <string_view>

#include <tinyxml2.h>

#include "parser.h"

namespace parser::util {

template <typename T>
T get_attribute(const tinyxml2::XMLElement* element, std::string_view attribute,
                bool required = true, const T& default_val = {}) {
  T value{};
  auto ret = element->QueryAttribute(attribute.data(), &value);

  if (ret == tinyxml2::XML_WRONG_ATTRIBUTE_TYPE) {
    throw AttributeError("Wrong type", attribute, element);
  }

  if (ret != tinyxml2::XML_SUCCESS) {
    if (!required) {
      return default_val;
    }

    // TODO: add attribute name and element tag with line number
    switch (ret) {
      case tinyxml2::XML_NO_ATTRIBUTE:
        throw AttributeError("No such attribute", attribute, element);

      case tinyxml2::XML_ERROR_PARSING_ATTRIBUTE:
        throw AttributeError("Error parsing", attribute, element);

      default:
        throw AttributeError("Unexpected error", attribute, element);
    }
  }

  return value;
}

template <>
inline std::string get_attribute(const tinyxml2::XMLElement* element,
                                 std::string_view attribute, bool required,
                                 const std::string& default_val) {
  const char* ret = get_attribute<const char*>(element, attribute, required,
                                               default_val.c_str());
  return {ret};
}

struct xml_element_iterator {
  xml_element_iterator() = default;
  xml_element_iterator(const tinyxml2::XMLElement* element,
                       std::string_view tag)
      : tag{tag}, element{element} {}

  xml_element_iterator& operator++() {
    element = element->NextSiblingElement(tag.data());
    return *this;
  }

  bool operator!=(const xml_element_iterator& rhs) const {
    return this->element != nullptr && (this->element != rhs.element);
  }

  template <typename T>
  T get_attribute(std::string_view attribute, bool required = true,
                  const T& default_val = {}) const {
    return util::get_attribute<T>(element, attribute, required, default_val);
  }

  xml_element_iterator operator*() const { return *this; }

  const tinyxml2::XMLElement* operator->() const { return element; }

  std::string_view tag;
  const tinyxml2::XMLElement* element = nullptr;
};

struct xml_element_range {
  xml_element_range(const tinyxml2::XMLElement* root, std::string_view tag)
      : root{root}, tag{tag} {}

  xml_element_iterator begin() const {
    return xml_element_iterator{root->FirstChildElement(tag.data()), tag};
  }

  xml_element_iterator end() const { return {}; }

  const tinyxml2::XMLElement* root;
  std::string_view tag;
};

};  // namespace parser::util

#endif  // __PARSE_UTIL_H_5T1B4GE8TUJ5__
