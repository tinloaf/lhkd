#include "config.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

Config::Config(std::string filename)
{
  std::ifstream i(filename);

  // Magically parse the JSON
  try {
    i >> this->data;
  } catch (const std::invalid_argument & e) {
    std::cerr << "Error: Could not parse JSON\n";
    exit(-1);
  }
}

void
Config::process_keys(Keyboard &kbd)
{
  for (auto entry : this->data["keys"]) {
    int keyboard_id = 0; // ALL_KEYBOARDS
    if (entry.find("keyboard") != entry.end()) {
      keyboard_id = entry["keyboard"];
    }
    kbd.add_hotkey(entry["key"], entry["meta"], entry["alt"], entry["ctrl"], entry["shift"], entry["super"], entry["action"], keyboard_id);
  }
}

void
Config::process_keyboards(Keyboard &kbd)
{
  if (this->data.find("keyboards") == this->data.end()) {
    return;
  }
  
  for (auto entry : this->data["keyboards"]) {
    kbd.add_keyboard_selector(entry["id"], entry["regex"]);
  }
}
