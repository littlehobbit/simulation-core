#include <cstdint>
#include <string_view>

#include <fmt/core.h>
#include <tinyxml2.h>

#include "parser.h"

namespace parser {

AttributeError::AttributeError(std::string_view what, std::string_view tag,
                               std::string_view attribute,
                               std::uint64_t line_num)
    : ParseError(
          fmt::format("Error with attribute \"{}\" in <{}> (line {}): {}",
                      attribute, tag, line_num, what)) {}

AttributeError::AttributeError(std::string_view what,
                               std::string_view attribute,
                               const tinyxml2::XMLElement *element)
    : AttributeError(what, element->Name(), attribute, element->GetLineNum()) {}

}  // namespace parser