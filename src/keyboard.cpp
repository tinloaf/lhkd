#include "keyboard.hpp"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cassert>

Keyboard::Keyboard()
{
  XIEventMask eventmask;
  unsigned char mask[4] =  { 0, 0, 0, 0 };

  this->display = XOpenDisplay(NULL);

  eventmask.deviceid = XIAllDevices;
  eventmask.mask_len = sizeof(mask); /* always in bytes */
  eventmask.mask = mask;

  /* Enable all keyboard events */
  XISetMask(mask, XI_RawKeyPress);
  XISetMask(mask, XI_RawKeyRelease);

  XISelectEvents(display, DefaultRootWindow(display), &eventmask, 1);

  if(XQueryExtension(display, "XInputExtension", &this->opcode, &this->event, &this->error) == 0)
  {
    printf("X Input extension not available.\n");
    exit(-1);
  }

  for (int i = 0 ; i < 8 ; ++i) {
    this->mod_keys[i] = false;
  }
  for (int i = 0 ; i < 4 ; ++i) {
    this->mod_state[i] = false;
  }

}

void
Keyboard::add_hotkey(std::string keyname, bool meta, bool alt, bool ctrl, bool shift, std::string action)
{
  size_t profile = (meta ? 1 << MOD_META : 0) |
        (alt ? 1 << MOD_ALT : 0) |
        (ctrl ? 1 << MOD_CTRL : 0) |
        (shift ? 1 << MOD_SHIFT : 0);

  std::cout << "Resolving key name " << keyname << "\n";
  xkb_keysym_t ksym = xkb_keysym_from_name(keyname.c_str(), XKB_KEYSYM_CASE_INSENSITIVE);
  std::cout << "Adding hotkey: KeySym " << ksym << " action: " << action << "\n";
  assert(ksym != XKB_KEY_NoSymbol);
  this->actions[profile].insert(std::make_pair(ksym, action));
}

void
Keyboard::handle_keypress(KeySym ksym)
{
  std::cout << "handling..." << ksym << " Flags: " << this->mod_profile() << "\n";
  if (actions[this->mod_profile()].find(ksym) != actions[this->mod_profile()].end()) {
    std::cout << "Action: " << actions[this->mod_profile()][ksym] << "\n";
    auto p = popen(actions[this->mod_profile()][ksym].c_str(), "r");
    pclose(p);
  }
}

size_t
Keyboard::mod_profile()
{
  return (this->mod_state[MOD_META] ? 1 << MOD_META : 0) |
        (this->mod_state[MOD_ALT] ? 1 << MOD_ALT : 0) |
        (this->mod_state[MOD_CTRL] ? 1 << MOD_CTRL : 0) |
        (this->mod_state[MOD_SHIFT] ? 1 << MOD_SHIFT : 0);
}

void
Keyboard::handle(Display* display, XIRawEvent *ev, bool press) {
  KeySym ksym = XKeycodeToKeysym(display, ev->detail, 0);

  switch (ksym) {
    case XKB_KEY_Control_L:
      this->mod_keys[KC_CTRL_L] = press;
      this->mod_state[MOD_CTRL] = this->mod_keys[KC_CTRL_L] || this->mod_keys[KC_CTRL_R];
      break;
    case XKB_KEY_Control_R:
      this->mod_keys[KC_CTRL_R] = press;
      this->mod_state[MOD_CTRL] = this->mod_keys[KC_CTRL_L] || this->mod_keys[KC_CTRL_R];
      break;

    case XKB_KEY_Shift_L:
      this->mod_keys[KC_SHIFT_L] = press;
      this->mod_state[MOD_SHIFT] = this->mod_keys[KC_SHIFT_L] || this->mod_keys[KC_SHIFT_R];
      break;
    case XKB_KEY_Shift_R:
      this->mod_keys[KC_SHIFT_R] = press;
      this->mod_state[MOD_SHIFT] = this->mod_keys[KC_SHIFT_L] || this->mod_keys[KC_SHIFT_R];
      break;

    case XKB_KEY_Alt_L:
      this->mod_keys[KC_ALT_L] = press;
      this->mod_state[MOD_ALT] = this->mod_keys[KC_ALT_L] || this->mod_keys[KC_ALT_R];
      break;
    case XKB_KEY_Alt_R:
      this->mod_keys[KC_ALT_R] = press;
      this->mod_state[MOD_ALT] = this->mod_keys[KC_ALT_L] || this->mod_keys[KC_ALT_R];
      break;

    case XKB_KEY_Meta_L:
      this->mod_keys[KC_META_L] = press;
      this->mod_state[MOD_META] = this->mod_keys[KC_META_L] || this->mod_keys[KC_META_R];
      break;
    case XKB_KEY_Meta_R:
      this->mod_keys[KC_META_R] = press;
      this->mod_state[MOD_META] = this->mod_keys[KC_META_L] || this->mod_keys[KC_META_R];
      break;


    default:
      if (press)
        handle_keypress(ksym);
      break;
  }
}

void
Keyboard::listen()
{
  while(true) {
    XEvent ev;
    XNextEvent(display, &ev);

    if (ev.xcookie.type == GenericEvent &&
        ev.xcookie.extension == this->opcode &&
        XGetEventData(display, &ev.xcookie))
    {
      const char* s = NULL;
        switch(ev.xcookie.evtype)
        {
            case XI_RawKeyPress: handle(display, (XIRawEvent *)ev.xcookie.data, true); break;
            case XI_RawKeyRelease: handle(display, (XIRawEvent *)ev.xcookie.data, false); break;
            default: break;
        }
    }
    XFreeEventData(display, &ev.xcookie);
  }
}
