#include "application.h"

#include <map>
#include <utility>

#include <fmt/core.h>
#include <ns3/application.h>
#include <ns3/string.h>

#include "model_build_error.h"
#include "parser/parser.h"
#include <ns3/object-factory.h>
#include <ns3/type-id.h>

namespace model {

Application::Application(const ns3::Ptr<ns3::Application> &app,
                         std::string name)
    : _name{std::move(name)}, _application{app} {}

auto Application::create(const parser::ApplicationDescription &description)
    -> Application {
  ns3::TypeId type_id;
  if (!ns3::TypeId::LookupByNameFailSafe(description.type, &type_id)) {
    throw ModelBuildError(
        fmt::format(R"(Unknown type "{}" of entity named "{}")",
                    description.type, description.name));
  }

  ns3::ObjectFactory factory;
  factory.SetTypeId(type_id);

  for (const auto &[key, value] : description.attributes) {
    ns3::TypeId::AttributeInformation info;
    if (!type_id.LookupAttributeByName(key, &info)) {
      throw ModelBuildError(fmt::format(
          R"(Unknown attribute "{}" of type "{}")", key, description.type));
    }

    // TODO: handle attributes error
    // auto val = info.initialValue->Copy();
    // if (val->DeserializeFromString(value, info.checker) == false) {
    //   throw ModelBuildError(
    //       fmt::format(R"(Bad attribute value "{}" of attribute "{}" of
    //       "{}")",
    //                   value, key, description.type));
    // }

    // FIX: May occure deserialization errors
    factory.Set(key, ns3::StringValue(value));
  }

  auto app = factory.Create<ns3::Application>();
  return {app, description.name};
}
}  // namespace model