#ifndef __NAME_SERVICE_H_N9DFCK7WBE99__
#define __NAME_SERVICE_H_N9DFCK7WBE99__

#include <stdexcept>

#include <fmt/core.h>
#include <ns3/names.h>
#include <ns3/object.h>
#include <ns3/ptr.h>

namespace model::names {

class DuplicateError final : public std::runtime_error {
 public:
  DuplicateError(const std::string &name)
      : std::runtime_error(fmt::format("Duplication of name \"{}\"", name)) {}

  DuplicateError(const std::string &name, const std::string &context)
      : std::runtime_error(fmt::format(R"(Duplication of name "{}" in "{}")",
                                       name, context)) {}
};

namespace internal {
inline void check_for_duplicate(const std::string &name);
inline void check_for_duplicate(const ns3::Ptr<ns3::Object> &context,
                                const std::string &name);
}  // namespace internal

inline void add(const ns3::Ptr<ns3::Object> &object, const std::string &name) {
  // ns3::Names::FindName(object) - already has
  internal::check_for_duplicate(name);
  ns3::Names::Add(name, object);
}

inline void add(const ns3::Ptr<ns3::Object> &context,
                const ns3::Ptr<ns3::Object> &object, const std::string &name) {
  internal::check_for_duplicate(context, name);
  ns3::Names::Add(context, name, object);
}

inline void cleanup() noexcept { ns3::Names::Clear(); }

namespace internal {
inline void check_for_duplicate(const std::string &name) {
  if (ns3::Names::Find<ns3::Object>(fmt::format("/Names/{}", name)) !=
      nullptr) {
    throw DuplicateError(name);
  }
}

inline void check_for_duplicate(const ns3::Ptr<ns3::Object> &context,
                                const std::string &name) {
  if (ns3::Names::Find<ns3::Object>(context, name) != nullptr) {
    throw DuplicateError(name);
  }
}
}  // namespace internal

}  // namespace model::names

#endif  // __NAME_SERVICE_H_N9DFCK7WBE99__
