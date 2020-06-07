libwm
=====
A small library for X window manipulation

Features
-----
* Depends on [libxcb][0] only
* Wrappers for all window management operations
* Built-in checks for off-screen operations

Usage
-----
In order to use the library, you must first include its header file in
your source file:

	#include <wm.h>

To link your program against it, compile it as follows:

    	cc pgm.c -lwm -lxcb -o pgm

Here is the list of all functions provided by `libwm`:

	wm_init_xcb();
	wm_kill_xcb();
	wm_is_alive(wid);
	wm_is_ignored(wid);
	wm_is_listable(wid, mask);
	wm_is_mapped(wid);
	wm_add_atom(name, len);
	wm_set_atom(wid, atom, type, len, data);
	wm_get_atom_string(wid, atom, &value);
	wm_get_screen();
	wm_get_windows(wid, &list);
	wm_get_focus();
	wm_get_attribute(wid, attr);
	wm_get_cursor(mode, wid, &x, &y);
	wm_set_border(width, color, wid);
	wm_set_focus(wid);
	wm_set_cursor(x, y, mode);
	wm_set_override(wid, mode);
	wm_teleport(wid, w, h, x, y);
	wm_move(wid, mode, x, y);
	wm_remap(wid, mode);
	wm_resize(wid, mode, w, h);
	wm_restack(wid, mode);
	wm_reg_event(wid, mask);

Their usage is specified in the `wm.h` header file, as it is quite small for
now.

Installation
-----
Edit config.mk as needed, then build/install with the following commands:

    make
    make install

Require [libxcb][0].

[0]: https://xcb.freedesktop.org
