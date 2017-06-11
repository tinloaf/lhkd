LockscreenHotKeyDaemon
======================

This software is a (very rudimentary) hotkey daemon for X11, i.e. a tool to execute a command whenever a certain key combination is pressed. The difference to various other hotkey daemons (and probably your window manager) is that it is designed to work even when your screen is locked.

Warnings
--------

Having active hotkeys when your screen is locked has obvious security implications. Some of them are:

* If the command executing at a keypress allows an attacker to do bad things, he can do bad things. D'Uh. Things to specifically keep in mind:
  * If a command starts a program that opens a window, that window *might* open in front of your screen locker. This is highly dependent on the way your screen locker locks the screen and might be non-deterministic.
* This is alpha software. I have only tested it with i3lock. It might behave badly with other screen lockers (tested it with another screen locker? Let me know!)
* The way lhkd grabs keys *may* conflict with the way your screen locker grabs the keys. Especially if both try to listen for the same key combination, race conditions will occur, and bad things could happen.

**The takeaway**: Please don't use this software on security-sensitive workstations. You have been warned.

Features
--------

LockscreenHotKeyDaemon …

* runs commands at the press of a hotkey (combination) even while your screen locker is active
* is able to distinguish between different keyboards. Personally, I have a second numblock keyboard next to my main keyboard, which I use solely for hotkeys.

Building
--------

You will need the headers for libX11, libXi and libxkb as well as a compiler supporting C++11 and cmake. Having this, building should be as easy as:

    mkdir build
    cd build
    cmake ..
    make

Usage
-----

You need to start lhkd with a JSON config file. Currently, the only required configuration is a mapping of key combinations to commands. Look at example.json for an example. The key modifiers currently available are:

* meta
* alt
* ctrl
* shift
* super (The "windows" key on non-Mac systems)

Optionally, you can assign keyboard numbers to your keyboards, and have certain hotkeys only listen to certain keyboards. Again, see example.json for an example.

License
-------

lhkd is released under MIT license. See License.txt for details. The JSON parser is taken from <https://github.com/nlohmann/json> and also available under MIT license.
