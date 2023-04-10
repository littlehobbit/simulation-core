#include <csignal>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>

#include "app_config.h"
#include "model/model.h"
#include "parser/parser.h"

namespace {
std::function<void()> on_sigterm;  // NOLINT

void signal_handler(int /*sig*/) {
  if (on_sigterm) {
    on_sigterm();
  }
}
}  // namespace

inline std::string read_xml(const std::string &path) noexcept {
  std::stringstream in;
  std::ifstream in_file{path};
  in << in_file.rdbuf();
  return in.str();
}

int main(int argc, char *argv[]) {
  AppConfig config;
  if (!config.parse(argc, argv)) {
    return 1;
  }

  std::signal(SIGTERM, signal_handler); // NOLINT

  try {
    auto model_description =
        parser::XmlParser().parse(read_xml(config.xml_model_path));

    model::Model model;
    model.build_from_description(model_description);

    on_sigterm = [&model] { model.stop(); };

    model.start();

    // TODO: extract exceptions (use fmt only in exception handler)
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return 0;
}