#include "config.hpp"

#include <fstream>

Config::Config(std::string filename)
{
  std::ifstream i(filename);

  i >> this->data;
}

void
Config::process_keys(Keyboard &kbd)
{
  for (auto entry : this->data["keys"]) {
    kbd.add_hotkey(entry["key"], entry["meta"], entry["alt"], entry["ctrl"], entry["shift"], entry["super"], entry["action"]);
  }
}
