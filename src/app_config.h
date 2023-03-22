#ifndef __APP_CONFIG_H_A5SZBOTDX6W8__
#define __APP_CONFIG_H_A5SZBOTDX6W8__

#include <string>

enum class log_type { plain, json };

struct AppConfig {
  bool parse(int argc, char *argv[]) noexcept;  // NOLINT

  std::string xml_model_path;
  // TODO: log type - plain or json
};

#endif  // __APP_CONFIG_H_A5SZBOTDX6W8__
