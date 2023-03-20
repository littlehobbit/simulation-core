#ifndef __APPLICATION_H_CSS6NU5OMIVH__
#define __APPLICATION_H_CSS6NU5OMIVH__

#include <string>

#include <ns3/application.h>
#include <ns3/ptr.h>

namespace parser {
struct ApplicationDescription;
}

namespace model {

class Application {
 public:
  auto get() const -> ns3::Ptr<ns3::Application> { return _application; }

  auto name() const -> const std::string& { return _name; }

  static Application create(const parser::ApplicationDescription& description);

 private:
  Application(const ns3::Ptr<ns3::Application>& app, std::string name);

  static bool is_application(const ns3::TypeId &app_type) noexcept;

  std::string _name;
  ns3::Ptr<ns3::Application> _application;
};

}  // namespace model

#endif  // __APPLICATION_H_CSS6NU5OMIVH__
