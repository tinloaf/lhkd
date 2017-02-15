#include <X11/Xlib.h> // Every Xlib program must include this
#include <assert.h>   // I include this to test return values the lazy way
#include <unistd.h>   // So we got the profile for 10 seconds
#include <iostream>
#include <X11/extensions/XInput2.h>
#include <unordered_map>

#define XK_MISCELLANY 1
#include <X11/keysymdef.h>

#define NIL (0)       // A name for the void pointer

bool mod_ctrl_l = false;
bool mod_ctrl_r = false;

bool mod_alt_l = false;
bool mod_alt_r = false;

bool mod_meta_l = false;
bool mod_meta_r = false;

std::array<std::unordered_map<KeySym, std::string>, 8> actions;

bool mod_ctrl() {
  return mod_ctrl_r || mod_ctrl_l;
}

bool mod_alt() {
  return mod_alt_r || mod_alt_l;
}

bool mod_meta() {
  return mod_meta_r || mod_meta_l;
}

size_t actions_index()
{
  return (mod_ctrl() ? 1 << 2 : 0) + (mod_alt() ? 1 << 1 : 0) + (mod_meta() ? 1 << 0 : 0);
}

void handle_keypress(KeySym ksym) {
  std::cout << "handling..." << ksym << " Flags: " << actions_index() << "\n";
  if (actions[actions_index()].find(ksym) != actions[actions_index()].end()) {
    std::cout << "Action: " << actions[actions_index()][ksym] << "\n";
    auto p = popen(actions[actions_index()][ksym].c_str(), "r");
    pclose(p);
  }
}

void handle_raw_event(Display* display, XIRawEvent *ev, bool press) {
  KeySym ksym = XKeycodeToKeysym(display, ev->detail, 0);

  std::cout << "Detail: " << ev->detail << " Flags: " << ev->flags << "\n";
  switch (ksym) {
    case XK_Control_L:
      mod_ctrl_l = press;
      break;
    case XK_Control_R:
      mod_ctrl_r = press;
      break;

    case XK_Meta_L:
      mod_meta_l = press;
      break;
    case XK_Meta_R:
      mod_meta_r = press;
      break;

    case XK_Alt_L:
      mod_alt_l = press;
      break;
    case XK_Alt_R:
      mod_alt_r = press;
      break;

    default:
      if (press)
        handle_keypress(ksym);
      break;
  }
  std::cout << "Keysym: " << ksym << " vs " << XK_Control_L << "\n";
}


int main(int argc, char **argv)
{
  Display* display;
	XIEventMask eventmask;
	unsigned char mask[4] =  { 0, 0, 0, 0 };
	int opcode, event, error;

  actions[1 << 2] = {
    {97, "echo test"}
  };

	display = XOpenDisplay(NULL);

	eventmask.deviceid = XIAllDevices;
	eventmask.mask_len = sizeof(mask); /* always in bytes */
	eventmask.mask = mask;

	/* Enable all raw events */
	XISetMask(mask, XI_RawButtonPress);
	XISetMask(mask, XI_RawButtonRelease);
	XISetMask(mask, XI_RawMotion);
	XISetMask(mask, XI_RawKeyPress);
	XISetMask(mask, XI_RawKeyRelease);


	if(XQueryExtension(display, "XInputExtension", &opcode, &event, &error) == 0)
	{
		printf("X Input extension not available.\n");
		return -1;
	}

	/* Select the events now */
	XISelectEvents(display, DefaultRootWindow(display), &eventmask, 1);

	while(1)
	{
		XEvent ev;
		XNextEvent(display, &ev);

		if (ev.xcookie.type == GenericEvent &&
    		ev.xcookie.extension == opcode &&
		    XGetEventData(display, &ev.xcookie))
		{
			const char* s = NULL;
		    switch(ev.xcookie.evtype)
		    {
		        case XI_RawButtonPress: break;

		        case XI_RawButtonRelease: break;
		        case XI_RawMotion:  break;
		        case XI_RawKeyPress: handle_raw_event(display, (XIRawEvent *)ev.xcookie.data, true);
						break;
		        case XI_RawKeyRelease: handle_raw_event(display, (XIRawEvent *)ev.xcookie.data, false);
						break;
		        default: break;
    		}
		}
		XFreeEventData(display, &ev.xcookie);
	}


}
