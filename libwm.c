#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "wm.h"

int
wm_init_xcb()
{
	conn = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(conn))
		return -1;
	return 0;
}

int
wm_kill_xcb()
{
	if (!conn)
		return -1;
	xcb_disconnect(conn);
	return 0;
}

int
wm_is_alive(xcb_window_t wid)
{
	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t  *r;

	c = xcb_get_window_attributes(conn, wid);
	r = xcb_get_window_attributes_reply(conn, c, NULL);

	if (r == NULL)
		return 0;

	free(r);
	return 1;
}

int
wm_is_mapped(xcb_window_t wid)
{
	int ms;
	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t  *r;

	c = xcb_get_window_attributes(conn, wid);
	r = xcb_get_window_attributes_reply(conn, c, NULL);

	if (r == NULL)
		return 0;

	ms = r->map_state;

	free(r);
	return ms == XCB_MAP_STATE_VIEWABLE;
}

int
wm_is_ignored(xcb_window_t wid)
{
	int or;
	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t  *r;

	c = xcb_get_window_attributes(conn, wid);
	r = xcb_get_window_attributes_reply(conn, c, NULL);

	if (r == NULL)
		return 0;

	or = r->override_redirect;

	free(r);
	return or;
}

int
wm_is_listable(xcb_window_t wid, int mask)
{
	if (!mask && wm_is_mapped (wid) && !wm_is_ignored(wid))
		return 1;
	if ((mask & LIST_ALL))
		return 1;
	if (!wm_is_mapped (wid) && mask & LIST_HIDDEN)
		return 1;
	if (wm_is_ignored(wid) && mask & LIST_IGNORE)
		return 1;

	return 0;
}

int
wm_get_screen()
{
	scrn = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
	if (scrn == NULL)
		return -1;
	return 0;
}

int
wm_get_windows(xcb_window_t wid, xcb_window_t **l)
{
	uint32_t childnum = 0;
	xcb_query_tree_cookie_t c;
	xcb_query_tree_reply_t *r;

	c = xcb_query_tree(conn, wid);
	r = xcb_query_tree_reply(conn, c, NULL);
	if (r == NULL)
		return -1;

	*l = malloc(sizeof(xcb_window_t) * r->children_len);
	memcpy(*l, xcb_query_tree_children(r),
			sizeof(xcb_window_t) * r->children_len);

	childnum = r->children_len;

	free(r);
	return childnum;
}

xcb_window_t
wm_get_focus(void)
{
	xcb_window_t wid = 0;
	xcb_get_input_focus_cookie_t c;
	xcb_get_input_focus_reply_t *r;

	c = xcb_get_input_focus(conn);
	r = xcb_get_input_focus_reply(conn, c, NULL);
	if (r == NULL)
		return scrn->root;

	wid = r->focus;
	free(r);
	return wid;
}


int
wm_get_attribute(xcb_window_t wid, int attr)
{
	xcb_get_geometry_cookie_t c;
	xcb_get_geometry_reply_t *r;

	c = xcb_get_geometry(conn, wid);
	r = xcb_get_geometry_reply(conn, c, NULL);

	if (r == NULL)
		return -1;

	switch (attr) {
	case ATTR_X:
		attr = r->x;
		break;
	case ATTR_Y:
		attr = r->y;
		break;
	case ATTR_W:
		attr = r->width;
		break;
	case ATTR_H:
		attr = r->height;
		break;
	case ATTR_B:
		attr = r->border_width;
		break;
	}

	free(r);
	return attr;
}

xcb_atom_t
wm_add_atom(xcb_atom_t type, char *name, size_t len)
{
	xcb_atom_t atom;
	xcb_intern_atom_cookie_t c;
	xcb_intern_atom_reply_t *r;

	c = xcb_intern_atom(conn, 0, len, name);
	r = xcb_intern_atom_reply(conn, c, NULL);
	if (!r)
		return 0;

	atom = r->atom;
	free(r);

	return atom;
}

int
wm_set_atom(xcb_window_t wid, xcb_atom_t atom, xcb_atom_t type, size_t len, void *data)
{
	int errcode;
	xcb_void_cookie_t c;
	xcb_generic_error_t *e;

	c = xcb_change_property_checked(conn, XCB_PROP_MODE_REPLACE,
		wid, atom, type, 32, len, data);
	e = xcb_request_check(conn, c);
	if (!e)
		return 0;

	errcode = e->error_code;
	free(e);

	return errcode;
}

void *
wm_get_atom(xcb_window_t wid, xcb_atom_t atom, xcb_atom_t type, size_t *len)
{
	void *d;
	size_t n;
	xcb_get_property_cookie_t c;
	xcb_get_property_reply_t *r;

	c = xcb_get_property(conn, 0, wid, atom, type, 0, 1);
	r = xcb_get_property_reply(conn, c, NULL);
	if (!r)
		return NULL;

	if (!(n = xcb_get_property_value_length(r))) {
		free(r);
		return NULL;
	}

	if (len)
		*len = n;

	d = xcb_get_property_value(r);

	return d;
}

int
wm_get_cursor(int mode, uint32_t wid, int *x, int *y)
{
	xcb_query_pointer_reply_t *r;
	xcb_query_pointer_cookie_t c;

	c = xcb_query_pointer(conn, wid);
	r = xcb_query_pointer_reply(conn, c, NULL);

	if (r == NULL)
		return -1;

	if (r->child != XCB_NONE) {
		*x = r->win_x;
		*y = r->win_y;
	} else {
		*x = r->root_x;
		*y = r->root_y;
	}

	return 0;
}

int
wm_set_border(int width, int color, xcb_window_t wid)
{
	uint32_t values[1];
	int mask;

	/* change width if >= 0 */
	if (width > -1) {
		values[0] = width;
		mask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
		xcb_configure_window(conn, wid, mask, values);
	}

	/*
	 * color is an ARGB representation (eg. 0x80ff0000) for
	 * translucent red.
	 * Absolutely all values are valid color representations, so we
	 * will set it no matter what.
	 */
	values[0] = color;
	mask = XCB_CW_BORDER_PIXEL;
	xcb_change_window_attributes(conn, wid, mask, values);

	return 0;
}

int
wm_set_cursor(int x, int y, int mode)
{
	xcb_warp_pointer(conn, XCB_NONE, mode ? XCB_NONE : scrn->root,
			0, 0, 0, 0, x, y);
	return 0;
}

int
wm_teleport(xcb_window_t wid, int x, int y, int w, int h)
{
	uint32_t values[4];
	uint32_t mask =   XCB_CONFIG_WINDOW_X
	                | XCB_CONFIG_WINDOW_Y
	                | XCB_CONFIG_WINDOW_WIDTH
	                | XCB_CONFIG_WINDOW_HEIGHT;
	values[0] = x;
	values[1] = y;
	values[2] = w;
	values[3] = h;
	xcb_configure_window(conn, wid, mask, values);

	return 0;
}

int
wm_move(xcb_window_t wid, int mode, int x, int y)
{
	int curx, cury, curw, curh, curb;

	if (!wm_is_mapped(wid) || wid == scrn->root)
		return -1;

	curb = wm_get_attribute(wid, ATTR_B);
	curx = wm_get_attribute(wid, ATTR_X);
	cury = wm_get_attribute(wid, ATTR_Y);
	curw = wm_get_attribute(wid, ATTR_W);
	curh = wm_get_attribute(wid, ATTR_H);

	if (mode == RELATIVE) {
		x += curx;
		y += cury;
	}

	/* the following prevent windows from moving off the screen */
	if (x < 0)
		x = 0;
	else if (x > scrn->width_in_pixels - curw - 2*curb)
		x = scrn->width_in_pixels - curw - 2*curb;

	if (y < 0)
		y = 0;
	else if (y > scrn->height_in_pixels - curh - 2*curb)
		y = scrn->height_in_pixels - curh - 2*curb;

	wm_teleport(wid, x, y, curw, curh);
	return 0;
}

int
wm_set_override(xcb_window_t wid, int or)
{
	uint32_t mask = XCB_CW_OVERRIDE_REDIRECT;
	uint32_t val[] = { or };

	xcb_change_window_attributes(conn, wid, mask, val);

	return 0;
}


int
wm_remap(xcb_window_t wid, int mode)
{
	switch (mode) {
	case MAP:
		xcb_map_window(conn, wid);
		break;
	case UNMAP:
		xcb_unmap_window(conn, wid);
		break;
	case TOGGLE:
		if (wm_is_mapped(wid))
			xcb_unmap_window(conn, wid);
		else
			xcb_map_window(conn, wid);
		break;
	}

	return 0;
}

int
wm_resize(xcb_window_t wid, int mode, int w, int h)
{
	int curx, cury, curw, curh, curb;

	if (!wm_is_mapped(wid) || wid == scrn->root)
		return -1;

	curb = wm_get_attribute(wid, ATTR_B);
	curx = wm_get_attribute(wid, ATTR_X);
	cury = wm_get_attribute(wid, ATTR_Y);
	curw = wm_get_attribute(wid, ATTR_W);
	curh = wm_get_attribute(wid, ATTR_H);

	if (mode == RELATIVE) {
		w += curw;
		h += curh;
	} else {
		w -= curx;
		h -= cury;
	}

	/*
	 * The following prevent windows from growing out of the screen, or
	 * having a negative size
	 */
	if (w < 0)
		w = curw;
	if (curx + w >  scrn->width_in_pixels)
		w = scrn->width_in_pixels - curx - 2*curb;

	if (h < 0)
		h = curh;
	if (cury + h > scrn->height_in_pixels)
		h = scrn->height_in_pixels - cury - 2*curb;

	wm_teleport(wid, curx, cury, w, h);
	return 0;
}

int
wm_restack(xcb_window_t wid, uint32_t mode)
{
	uint32_t values[1];
	xcb_configure_window(conn, wid, XCB_CONFIG_WINDOW_STACK_MODE, values);
	return 0;
}

int
wm_set_focus(xcb_window_t wid)
{
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, wid,
	                    XCB_CURRENT_TIME);
	return 0;
}

int
wm_reg_window_event(xcb_window_t wid, uint32_t mask)
{
	uint32_t val[] = { mask };
	xcb_void_cookie_t c;
	xcb_generic_error_t *e;

	c = xcb_change_window_attributes_checked(conn, wid, XCB_CW_EVENT_MASK, val);
	e = xcb_request_check(conn, c);
	if (!e)
		return -1;

	free(e);
	return 0;
}


int
wm_reg_cursor_event(xcb_window_t wid, uint32_t mask, char *cursor)
{
	xcb_cursor_t p;
	xcb_cursor_context_t *cx;
	xcb_grab_pointer_cookie_t c;
	xcb_grab_pointer_reply_t *r;

	p = XCB_NONE;
	if (cursor) {
		if (xcb_cursor_context_new(conn, scrn, &cx) < 0)
			return -1;

		p = xcb_cursor_load_cursor(cx, cursor);
	}

	c = xcb_grab_pointer(conn, 1, scrn->root, mask,
		XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
		XCB_NONE, p, XCB_CURRENT_TIME);

	r = xcb_grab_pointer_reply(conn, c, NULL);
	if (!r || r->status != XCB_GRAB_STATUS_SUCCESS)
		return -1;

	xcb_cursor_context_free(cx);
	return 0;
}
