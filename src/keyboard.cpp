#include "keyboard.hpp"

#include <stdio.h>
#include <string.h>
#include <iostream>

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
}

void
Keyboard::handle(XEvent & ev)
{
  std::cout << "Event. KC: " <<  ((XKeyEvent*)&ev)->keycode << " M: " << ((XKeyEvent*)&ev)->state << "\n";
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
            case XI_RawKeyPress: handle(ev); break;
            default: break;
        }
    }
    XFreeEventData(display, &ev.xcookie);
  }
}
