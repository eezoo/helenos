/*
 * Copyright (c) 2019 Jiri Svoboda
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <pcut/pcut.h>
#include <stdio.h>
#include <str.h>

#include "../client.h"
#include "../display.h"
#include "../window.h"

PCUT_INIT;

PCUT_TEST_SUITE(client);

static void test_ds_ev_pending(void *);

static ds_client_cb_t test_ds_client_cb = {
	.ev_pending = test_ds_ev_pending
};

static void test_ds_ev_pending(void *arg)
{
	bool *called_cb = (bool *) arg;
	printf("test_ds_ev_pending\n");
	*called_cb = true;

}

/** Client creation and destruction. */
PCUT_TEST(client_create_destroy)
{
	ds_display_t *disp;
	ds_client_t *client;
	errno_t rc;

	rc = ds_display_create(NULL, &disp);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_client_create(disp, &test_ds_client_cb, NULL, &client);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	ds_client_destroy(client);
	ds_display_destroy(disp);
}

/** Test ds_client_find_window().
 *
 * ds_client_add_window() and ds_client_remove_window() are indirectly
 * tested too as part of creating and destroying the window
 */
PCUT_TEST(client_find_window)
{
	ds_display_t *disp;
	ds_client_t *client;
	ds_window_t *w0;
	ds_window_t *w1;
	ds_window_t *wnd;
	errno_t rc;

	rc = ds_display_create(NULL, &disp);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_client_create(disp, &test_ds_client_cb, NULL, &client);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_window_create(client, &w0);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_window_create(client, &w1);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	wnd = ds_client_find_window(client, w0->id);
	PCUT_ASSERT_EQUALS(w0, wnd);

	wnd = ds_client_find_window(client, w1->id);
	PCUT_ASSERT_EQUALS(w1, wnd);

	wnd = ds_client_find_window(client, 0);
	PCUT_ASSERT_NULL(wnd);

	wnd = ds_client_find_window(client, w1->id + 1);
	PCUT_ASSERT_NULL(wnd);

	ds_window_delete(w0);
	ds_window_delete(w1);
	ds_client_destroy(client);
	ds_display_destroy(disp);
}

/** Test ds_client_first_window() / ds_client_next_window. */
PCUT_TEST(client_first_next_window)
{
	ds_display_t *disp;
	ds_client_t *client;
	ds_window_t *w0;
	ds_window_t *w1;
	ds_window_t *wnd;
	errno_t rc;

	rc = ds_display_create(NULL, &disp);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_client_create(disp, &test_ds_client_cb, NULL, &client);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_window_create(client, &w0);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_window_create(client, &w1);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	wnd = ds_client_first_window(client);
	PCUT_ASSERT_EQUALS(w0, wnd);

	wnd = ds_client_next_window(w0);
	PCUT_ASSERT_EQUALS(w1, wnd);

	wnd = ds_client_next_window(w1);
	PCUT_ASSERT_NULL(wnd);

	ds_window_delete(w0);
	ds_window_delete(w1);
	ds_client_destroy(client);
	ds_display_destroy(disp);
}

/** Test ds_client_get_event(), ds_client_post_kbd_event(). */
PCUT_TEST(display_get_post_kbd_event)
{
	ds_display_t *disp;
	ds_client_t *client;
	ds_window_t *wnd;
	kbd_event_t event;
	ds_window_t *rwindow;
	display_wnd_ev_t revent;
	bool called_cb = NULL;
	errno_t rc;

	rc = ds_display_create(NULL, &disp);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_client_create(disp, &test_ds_client_cb, &called_cb, &client);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_window_create(client, &wnd);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	event.type = KEY_PRESS;
	event.key = KC_ENTER;
	event.mods = 0;
	event.c = L'\0';

	PCUT_ASSERT_FALSE(called_cb);

#if 0
	// XXX Forgot to change ds_client_get_event not to block
	rc = ds_client_get_event(client, &rwindow, &revent);
	PCUT_ASSERT_ERRNO_VAL(ENOENT, rc);
#endif

	rc = ds_client_post_kbd_event(client, wnd, &event);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
	PCUT_ASSERT_TRUE(called_cb);

	rc = ds_client_get_event(client, &rwindow, &revent);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
	PCUT_ASSERT_EQUALS(wnd, rwindow);
	PCUT_ASSERT_EQUALS(event.type, revent.kbd_event.type);
	PCUT_ASSERT_EQUALS(event.key, revent.kbd_event.key);
	PCUT_ASSERT_EQUALS(event.mods, revent.kbd_event.mods);
	PCUT_ASSERT_EQUALS(event.c, revent.kbd_event.c);

	ds_window_delete(wnd);
	ds_client_destroy(client);
	ds_display_destroy(disp);
}

PCUT_EXPORT(client);
