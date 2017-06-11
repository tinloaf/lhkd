#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <unordered_map>
#include <vector>
#include <regex>
#include <string>

#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

//#define XK_MISCELLANY 1
//#include <X11/keysymdef.h>

class Keyboard {
public:
  Keyboard();

  void add_hotkey(std::string keyname, bool meta, bool alt, bool ctrl, bool shift, bool super, std::string action, int keyboard_id);
  void listen();

  void add_keyboard_selector(int keyboard_id, std::string keyboard_re);

private:
  // wtf? why do i see every event twice?
  KeySym last_key = XKB_KEY_NoSymbol;

  void handle_keypress(KeySym ksym, int keyboard_id);
  void handle(Display* display, XIRawEvent *ev, bool press);

  void establish_grab(xkb_keysym_t ksym, bool meta, bool alt, bool ctrl, bool shift, bool super);

  int opcode, event, error;

  Display* display;

  static const size_t KC_CTRL_L =  0;
  static const size_t KC_CTRL_R =  1;
  static const size_t KC_ALT_L =  2;
  static const size_t KC_ALT_R =  3;
  static const size_t KC_META_L =  4;
  static const size_t KC_META_R =  5;
  static const size_t KC_SHIFT_L =  6;
  static const size_t KC_SHIFT_R =  7;
  static const size_t KC_SUPER_L =  6;
  static const size_t KC_SUPER_R =  7;

  static const size_t MOD_CTRL = 0;
  static const size_t MOD_ALT = 1;
  static const size_t MOD_META = 2;
  static const size_t MOD_SHIFT = 3;
  static const size_t MOD_SUPER = 4;

  static const int KEYBOARD_ANY = 0;

  // TODO store them per-keyboard?
  std::array<bool, 10> mod_keys;
  std::array<bool, 5> mod_state;

  std::vector<std::array<std::unordered_map<KeySym, std::string>, 32>> actions;

  size_t mod_profile();
  uint16_t get_xlib_modifiers(bool meta, bool alt, bool ctrl, bool shift, bool super);

  void resolve_keyboard(int, XIDeviceInfo*);
  std::unordered_map<int, int> keyboard_ids;
  std::vector<std::pair<std::regex, int>> keyboard_selectors;
};

#endif
