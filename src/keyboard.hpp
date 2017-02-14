#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

class Keyboard {
public:
  Keyboard();

  void listen();

private:
  void handle(XEvent &ev);
  int opcode, event, error;

  Display* display;
  XIEventMask eventmask;

};

#endif
