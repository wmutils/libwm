#ifndef __LIBWM_H__
#define __LIBWM_H__

/*
 * Variables used to hold the connection to the X server, and the first screen
 * of this connection. Both have to be defined as `extern`.
 */
extern xcb_connection_t *conn;
extern xcb_screen_t     *scrn;

/*
 * Mask attributes used to select which windows have to be listed by the
 * function `wm_is_listable(wid, mask)`.
 */
enum {
	LIST_HIDDEN = 1 << 0, /* windows that are not on-screen */
	LIST_IGNORE = 1 << 1, /* windows with override_redirect set to 1 */
	LIST_ALL    = 1 << 2  /* All existing windows */
};

/*
 * Actions used by the `wm_remap(wid, mode)` function to select what needs to be
 * done.
 */
enum {
	MAP     = 1 << 0,
	UNMAP   = 1 << 1,
	TOGGLE  = 1 << 2
};

/*
 * Attributes used internally by different functions to refer to windows
 * attributes, and select them.
 */
enum {
	ATTR_W = 1 << 0,
	ATTR_H = 1 << 1,
	ATTR_X = 1 << 2,
	ATTR_Y = 1 << 3,
	ATTR_B = 1 << 4,
	ATTR_M = 1 << 5,
	ATTR_I = 1 << 6,
	ATTR_D = 1 << 7,
	ATTR_MAX
};

/*
 * Selector used  by both `wm_move(wid, mode, x, y)` and `wm_resize(wid, mode, w, h)`
 * to choose between relative or absolute coordinates
 */
enum {
	ABSOLUTE = 0,
	RELATIVE = 1
};

/*
 * Initialize the connection to the X server. The connection could then be
 * accessed by other functions through the "conn" variable.
 */
int wm_init_xcb();

/*
 * Close connection to the X server.
 */
int wm_kill_xcb();

/*
 * Check existence of a window.
 * + 1 - window exists
 * + 0 - window doesn't exist
 */
int wm_is_alive(xcb_window_t wid);

/*
 * Returns the value of the "override_redirect" attribute of a window.
 * When this attribute is set to 1, it means the window manager should NOT
 * handle this window.
 */
int wm_is_ignored(xcb_window_t wid);

/*
 * Returns 1 if a window match the mask, 0 otherwise.
 * Possible value for the masks are:
 * 	LIST_HIDDEN
 * 	LIST_IGNORE
 * 	LIST_ALL
 */
int wm_is_listable(xcb_window_t wid, int mask);

/*
 * Returns 1 if the window is mapped on screen, 0 otherwise
 */
int wm_is_mapped(xcb_window_t wid);

/*
 * Request the X server to add a new atom, and return this new atom ID
 */
xcb_atom_t wm_add_atom(char *name, size_t len);

/*
 * Change the value of the specified atom
 */
int wm_set_atom(xcb_window_t wid, xcb_atom_t atom, xcb_atom_t type, size_t len, void *data);

/*
 * Retrieve the value of the given atom. The value length is set in the
 * `len` pointer if specified
 */
void *wm_get_atom(xcb_window_t wid, xcb_atom_t atom, xcb_atom_t type, size_t *len);

/*
 * Retrieve the name of the given atom. The name length is set in the
 * `len` pointer if specified
 */
char *wm_get_atom_name(xcb_atom_t atom, size_t *len);

/*
 * Get the first screen, and set the `scrn` global variable accordingly.
 */
int wm_get_screen();

/*
 * Ask the list of all existing windows to the X server, and fills the `*list`
 * argument with them.
 * The windows are listed in stacking order, from lower to upper window.
 * Returns the number of windows in the *list array
 */
int wm_get_windows(xcb_window_t wid, xcb_window_t **list);

/*
 * Returns the window that has keyboard focus
 * Will return the root window ID if no window has focus
 */
xcb_window_t wm_get_focus(void);

/*
 * Retrieve the value of an attribute for a specific windows.
 * The possible values for the attributes are:
 * 	ATTR_W - width
 * 	ATTR_H - height
 * 	ATTR_X - X offset
 * 	ATTR_Y - Y offset
 * 	ATTR_B - border width
 * 	ATTR_M - map state
 * 	ATTR_I - ignore state (override_redirect)
 */
int wm_get_attribute(xcb_window_t wid, int attr);

/*
 * Get the cursor position, and store its coordinates in the `x` and `y`
 * pointers.
 * The `mode` attribute isn't used yet, but is reserved to ask for either
 * absolute or relative coordinates
 */
int wm_get_cursor(int mode, uint32_t wid, int *x, int *y);

/*
 * Set a window's border.
 * The color should be an hexadecimal number, eg: 0xdeadca7
 */
int wm_set_border(int width, int color, xcb_window_t wid);

/*
 * Give the input focus to the specified window
 */
int wm_set_focus(xcb_window_t wid);

/*
 * Change the cursor position, either relatively or absolutely, eg:
 * 	wm_set_cursor(10, 10, ABSOLUTE);
 * 	wm_set_cursor(-10, 20, RELATIVE);
 */
int wm_set_cursor(int x, int y, int mode);

/*
 * Set override_redirect value for given window
 */
int wm_set_override(xcb_window_t wid, int override);

/*
 * Teleport a window to the given position.
 */
int wm_teleport(xcb_window_t wid, int w, int h, int x, int y);

/*
 * Move a window to the given position, either relatively or absolutely.
 * If the wm_move is supposed to wm_move the window outside the screen, then the
 * windows will only be wm_moved to the edge of the screen.
 *
 * You cannot wm_move windows outside the screen with this method. Use
 * `wm_teleport()` instead.
 */
int wm_move(xcb_window_t wid, int mode, int x, int y);

/*
 * Resize a window to the given size, either relatively or absolutely.
 * If the wm_resize is supposed to put an area of the window outside the screen,
 * then the windows will only be wm_resized to the edge of the screen.
 *
 * You cannot wm_resize windows farther than the screen edge with this method. Use
 * `wm_teleport()` instead.
 */
int wm_resize(xcb_window_t wid, int mode, int w, int h);

/*
 * Change the mapping state of a window. The `mode` attribute can be as follow:
 * 	MAP
 * 	UNMAP
 * 	TOGGLE
 */
int wm_remap(xcb_window_t wid, int mode);

/*
 * Change the position of the given window in the stack order.
 * You can either put it at the top, or at the bottom.
 * The possible values for the mode are:
 * 	XCB_STACK_MODE_ABOVE
 * 	XCB_STACK_MODE_BELOW
 * 	XCB_STACK_MODE_OPPOSITE
 */
int wm_restack(xcb_window_t wid, uint32_t mode);

/*
 * Register the given event(s) on the window.
 * Multiple events can be registered by ORing them together
 */
int wm_reg_window_event(xcb_window_t wid, uint32_t mask);

/*
 * Register the given cursor event(s) on the window.
 * Multiple events can be registered by ORing them together.
 * The cursor will be changed to the `cursor` while the pointer is
 * grabbed, if not NULL.
 */
int wm_reg_cursor_event(xcb_window_t wid, uint32_t mask, char *cursor);

/*
 * Return the number of active monitors connected to the display where
 * window `wid` is sitting.
 * The `list` argument, if not NULL, will be filled with the monitor's
 * index numbers. Note that `list` must be big enough to hold all indexes,
 * as it is not reallocated.
 */
int wm_get_monitors(xcb_window_t wid, int *list);

/*
 * Return the info scructure defining monitor with index number `index`.
 */
xcb_randr_monitor_info_t *wm_get_monitor(int index);

/*
 * Return the index of the (first) monitor associated with the
 * coordinates.
 */
int wm_find_monitor(int x, int y);

#endif /* __LIBWM_H__ */
