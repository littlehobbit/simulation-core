#ifndef __APP_CONFIG_H_A5SZBOTDX6W8__
#define __APP_CONFIG_H_A5SZBOTDX6W8__

#include <string>

enum class log_type { plain, json };

/**
 * @brief CLI arguments parser
 * 
 */
struct AppConfig {

  /**
   * @brief Parse CLI args
   * 
   * @param argc number of args
   * @param argv array of CLI args
   * @return true if arguments were parsed correctly
   * @return false overwise
   */
  bool parse(int argc, char *argv[]) noexcept;  // NOLINT

  std::string xml_model_path;
};

#endif  // __APP_CONFIG_H_A5SZBOTDX6W8__
