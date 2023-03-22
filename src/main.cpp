#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include "app_config.h"
#include "model/model.h"
#include "parser/parser.h"

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

  try {
    auto model_description =
        parser::XmlParser().parse(read_xml(config.xml_model_path));

    model::Model model;
    model.build_from_description(model_description);

    model.start();

    // TODO: extract exceptions (use fmt only in exception handler)
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return 0;
}