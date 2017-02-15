#ifndef CONFIG_H
#define CONFIG_H

#include "json.hpp"
#include "keyboard.hpp"

using json = nlohmann::json;

class Config {
public:
  Config(std::string filename);

  void process_keys(Keyboard &kbd);

private:
  json data;
};

#endif
