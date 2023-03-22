#include "application.h"

#include <utility>

#include <ns3/application.h>
#include <ns3/type-id.h>

#include <fmt/core.h>

#include "model_build_error.h"
#include "parser/parser.h"
#include "utils/object.h"

namespace model {

Application::Application(const ns3::Ptr<ns3::Application> &app,
                         std::string name)
    : _name{std::move(name)}, _application{app} {}

auto Application::create(const parser::ApplicationDescription &description)
    -> Application {
  try {
    if (!is_application(description.type)) {
      throw ModelBuildError(
          fmt::format("Can't create application of non-application type \"{}\"",
                      description.type));
    }

    auto app = utils::create<ns3::Application>(description.type,
                                               description.attributes);

    return {app, description.name};
  } catch (utils::BadTypeId &bad_type) {
    throw ModelBuildError(
        fmt::format(R"(Unknown type "{}" of entity named "{}")",
                    description.type, description.name));
  } catch (utils::BadAttribute &bad_attribute) {
    // TODO: refactor model build error
    throw ModelBuildError(fmt::format(R"(Unknown attribute "{}" of type "{}")",
                                      bad_attribute.attribute,
                                      description.type));
  }
}

bool Application::is_application(const std::string &type) noexcept {
  ns3::TypeId app_type;

  if (!ns3::TypeId::LookupByNameFailSafe(type, &app_type)) {
    return false;
  }

  return app_type.IsChildOf(ns3::Application::GetTypeId());
}

}  // namespace model