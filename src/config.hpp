#ifndef CONFIG_H
#define CONFIG_H

#include "keyboard.hpp"

/* *****************************************************************
 * The json.hpp file is not under our control. Thus, we disable warnings.
 * *****************************************************************
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wswitch-default"

#include "json.hpp"
// Re-enable warnings
#pragma GCC diagnostic pop


using json = nlohmann::json;

class Config {
public:
  Config(std::string filename);

  void process_keys(Keyboard &kbd);

private:
  json data;
};

#endif
