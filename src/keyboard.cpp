#include "keyboard.hpp"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cassert>
#include <xcb/xproto.h>
#include <X11/extensions/XInput2.h>
#include <X11/XKBlib.h>

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

  for (int i = 0 ; i < 10 ; ++i) {
    this->mod_keys[i] = false;
  }
  for (int i = 0 ; i < 5 ; ++i) {
    this->mod_state[i] = false;
  }

}

void
Keyboard::establish_grab(xkb_keysym_t ksym, bool meta, bool alt, bool ctrl, bool shift, bool super)
{
  auto kc = XKeysymToKeycode(this->display, ksym);
  uint16_t modifiers = this->get_xlib_modifiers(meta,alt,ctrl,shift,super);

  XGrabKey(this->display, kc, modifiers, DefaultRootWindow(display), false, GrabModeAsync, GrabModeAsync);
}

void
Keyboard::add_keyboard_selector(int keyboard_id, std::string keyboard_re)
{
  this->keyboard_selectors.push_back({std::regex(keyboard_re), keyboard_id});
}


void
Keyboard::add_hotkey(std::string keyname, bool meta, bool alt, bool ctrl, bool shift, bool super, std::string action, int keyboard_id)
{
  size_t profile = (meta ? 1 << MOD_META : 0) |
        (alt ? 1 << MOD_ALT : 0) |
        (ctrl ? 1 << MOD_CTRL : 0) |
        (shift ? 1 << MOD_SHIFT : 0) |
        (super ? 1 << MOD_SUPER : 0);

  xkb_keysym_t ksym = xkb_keysym_from_name(keyname.c_str(), XKB_KEYSYM_CASE_INSENSITIVE);

  assert(ksym != XKB_KEY_NoSymbol);
  if ((int)this->actions.size() < keyboard_id + 1) {
    this->actions.resize(keyboard_id + 1);
  }
  this->actions[keyboard_id][profile].insert(std::make_pair(ksym, action));

  this->establish_grab(ksym, meta, alt, ctrl, shift, super);
}

void
Keyboard::handle_keypress(KeySym ksym, int keyboard_id)
{
  // Handle "all keyboards"
  if (actions[0][this->mod_profile()].find(ksym) != actions[0][this->mod_profile()].end()) {
    // Re-grab
    assert(ksym <= std::numeric_limits<xkb_keysym_t>::max());
    this->establish_grab((xkb_keysym_t)ksym, this->mod_state[MOD_META], this->mod_state[MOD_ALT], this->mod_state[MOD_CTRL], this->mod_state[MOD_SHIFT], this->mod_state[MOD_SUPER]);

    auto p = popen(actions[0][this->mod_profile()][ksym].c_str(), "r");
    pclose(p);
  }

  if (keyboard_id > 0) {
    // Handle this keyboard
    if (actions[keyboard_id][this->mod_profile()].find(ksym) != actions[keyboard_id][this->mod_profile()].end()) {
      // Re-grab
      assert(ksym <= std::numeric_limits<xkb_keysym_t>::max());
      this->establish_grab((xkb_keysym_t)ksym, this->mod_state[MOD_META], this->mod_state[MOD_ALT], this->mod_state[MOD_CTRL], this->mod_state[MOD_SHIFT], this->mod_state[MOD_SUPER]);

      auto p = popen(actions[keyboard_id][this->mod_profile()][ksym].c_str(), "r");
      pclose(p);
    }
  }
}

uint16_t
Keyboard::get_xlib_modifiers(bool meta, bool alt, bool ctrl, bool shift, bool super)
{
  uint16_t modifiers = 0;
  if (alt) {
    modifiers |= Mod1Mask;
  }
  if (meta) {
    modifiers |= Mod3Mask;
  }
  if (super) {
    modifiers |= Mod4Mask;
  }
  if (ctrl) {
    modifiers |= ControlMask;
  }
  if (shift) {
    modifiers |= ShiftMask;
  }

  return modifiers;
}

size_t
Keyboard::mod_profile()
{
  return (this->mod_state[MOD_META] ? 1 << MOD_META : 0) |
        (this->mod_state[MOD_ALT] ? 1 << MOD_ALT : 0) |
        (this->mod_state[MOD_CTRL] ? 1 << MOD_CTRL : 0) |
        (this->mod_state[MOD_SHIFT] ? 1 << MOD_SHIFT : 0) |
        (this->mod_state[MOD_SUPER] ? 1 << MOD_SUPER : 0);
}

void
Keyboard::resolve_keyboard(int device_id, XIDeviceInfo * dinfo)
{
  for (const auto & keyboard_selector : this->keyboard_selectors) {
    if (std::regex_search(std::string(dinfo->name), std::get<0>(keyboard_selector))) {
      int keyboard_id = std::get<1>(keyboard_selector);
      this->keyboard_ids[device_id] = keyboard_id;

      // make sure we have an action store for it
      if ((int)this->actions.size() < keyboard_id + 1) {
        this->actions.resize(keyboard_id + 1);
      }
      return;
    }
  }

  // Not configured
  this->keyboard_ids[device_id] = KEYBOARD_ANY;
}

void
Keyboard::handle(Display* display_arg, XIRawEvent *ev, bool press) {
  KeySym ksym = XkbKeycodeToKeysym(display_arg, (KeyCode)ev->detail,
                                0, this->mod_state[MOD_SHIFT] ? 1 : 0);

  // For some reason we get every event twice. Deduplication happens here.
  if (this->last_key == ksym) {
    this->last_key = XKB_KEY_NoSymbol;
    return;
  }
  this->last_key = ksym;

  int deviceid = ev->deviceid;
  if (this->keyboard_ids.find(deviceid) == this->keyboard_ids.end()) {
    int device_count;
    XIDeviceInfo * dinfo = XIQueryDevice(display_arg, deviceid, &device_count);

    this->resolve_keyboard(deviceid, dinfo);

    for (int i = 0 ; i < device_count ; ++i) {
      XIFreeDeviceInfo(dinfo + i);
    }
  }

  int keyboard_id = this->keyboard_ids[deviceid];

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

    case XKB_KEY_Super_L:
      this->mod_keys[KC_SUPER_L] = press;
      this->mod_state[MOD_SUPER] = this->mod_keys[KC_SUPER_L] || this->mod_keys[KC_SUPER_R];
      break;
    case XKB_KEY_Super_R:
      this->mod_keys[KC_SUPER_R] = press;
      this->mod_state[MOD_SUPER] = this->mod_keys[KC_SUPER_L] || this->mod_keys[KC_SUPER_R];
      break;


    default:
      if (press)
        handle_keypress(ksym, keyboard_id);
      break;
  }
}

void
Keyboard::listen()
{
  while(true) {
    XEvent ev;
    XNextEvent(this->display, &ev);

    if (ev.xcookie.type == GenericEvent &&
        ev.xcookie.extension == this->opcode &&
        XGetEventData(display, &ev.xcookie))
    {
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
