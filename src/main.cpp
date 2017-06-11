#include "keyboard.hpp"
#include "config.hpp"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Error. Specify config.\n";
    exit(-1);
  }

  Config cfg(argv[1]);
  Keyboard kbd;

  cfg.process_keys(kbd);
  cfg.process_keyboards(kbd);
  
  kbd.listen();
}
