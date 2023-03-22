#ifndef __REGISTRATOR_H_RNBREI3Y1PHL__
#define __REGISTRATOR_H_RNBREI3Y1PHL__

#include <memory>

#include <ns3/config.h>
#include <ns3/event-id.h>
#include <ns3/file-aggregator.h>
#include <ns3/file-helper.h>
#include <ns3/nstime.h>
#include <ns3/simulator.h>

#include <fmt/core.h>

#include "parser/parser.h"

namespace model {

class Registrator : public std::enable_shared_from_this<Registrator> {
 public:
  explicit Registrator(const parser::RegistratorDescription &descr)
      : _probe_type{descr.type},
        _file_name{descr.file},
        _trace{descr.source},
        _sink{descr.sink},
        _value_name{descr.value_name},
        _init_time{descr.start_time},
        _end_time{descr.end_time.has_value() ? descr.end_time.value() : "0s"} {}

  static std::shared_ptr<Registrator> create(
      const parser::RegistratorDescription &descr) {
    return std::make_shared<Registrator>(descr);
  }

  void shedule_init() {
    // NOLINTNEXTLINE
    _init_event =
        ns3::Simulator::Schedule(_init_time, [self = this->weak_from_this()]() {
          if (!self.expired()) {
            self.lock()->initialize();
          }
        });
  }

  auto get_event_id() const -> ns3::EventId { return _init_event; }

 private:
  void initialize() {
    _file_helper.ConfigureFile(_file_name,
                               ns3::FileAggregator::COMMA_SEPARATED);
    _file_helper.SetHeading(fmt::format("Time,{}", _value_name));

    // TODO: check probe type, _trace ans sink
    _file_helper.WriteProbe(_probe_type, _trace, _sink);

    auto probe = _file_helper.GetProbe("FileProbe-1");
    probe->SetAttribute("Stop", ns3::TimeValue(_end_time));
  }

  std::string _probe_type;
  std::string _file_name;
  std::string _trace;
  std::string _sink;
  std::string _value_name;

  ns3::Time _init_time;
  ns3::Time _end_time;

  ns3::FileHelper _file_helper;
  ns3::EventId _init_event;
};

}  // namespace model

#endif  // __REGISTRATOR_H_RNBREI3Y1PHL__
