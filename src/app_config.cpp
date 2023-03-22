#include "app_config.h"

#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Error.hpp>
#include <CLI/Option.hpp>
#include <CLI/Validators.hpp>

// NOLINTNEXTLINE
bool AppConfig::parse(int argc, char *argv[]) noexcept {
  CLI::App app{"NS3 simululation core", "simulation"};

  app.add_option("-i,--xml", xml_model_path,
                 "Path to network model in XML format")
      ->check(CLI::ExistingFile);

  try {
    app.parse(argc, argv);
  } catch (CLI::Error &e) {
    app.exit(e);
    return false;
  }

  return true;
}