/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2013 Intel Corporation
 * Copyright (C) 2013 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "config.h"

#include <errno.h>
#include <ibus.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "input-method-client-protocol.h"

struct _IBusWaylandIM
{
    struct wl_input_method *input_method;
    struct wl_input_method_context *context;
    struct wl_keyboard *keyboard;

    IBusInputContext *ibuscontext;
    IBusText *preedit_text;
    guint preedit_cursor_pos;
    IBusModifierType modifiers;

    struct xkb_context *xkb_context;

    struct xkb_keymap *keymap;
    struct xkb_state *state;

    xkb_mod_mask_t shift_mask;
    xkb_mod_mask_t lock_mask;
    xkb_mod_mask_t control_mask;
    xkb_mod_mask_t mod1_mask;
    xkb_mod_mask_t mod2_mask;
    xkb_mod_mask_t mod3_mask;
    xkb_mod_mask_t mod4_mask;
    xkb_mod_mask_t mod5_mask;
    xkb_mod_mask_t super_mask;
    xkb_mod_mask_t hyper_mask;
    xkb_mod_mask_t meta_mask;

    uint32_t serial;

    GCancellable *cancellable;
};
typedef struct _IBusWaylandIM IBusWaylandIM;

struct _IBusWaylandKeyEvent
{
    struct wl_input_method_context *context;
    uint32_t serial;
    uint32_t time;
    uint32_t key;
    enum wl_keyboard_key_state state;
};
typedef struct _IBusWaylandKeyEvent IBusWaylandKeyEvent;

struct _IBusWaylandSource
{
    GSource source;
    GPollFD pfd;
    uint32_t mask;
    struct wl_display *display;
};
typedef struct _IBusWaylandSource IBusWaylandSource;

struct wl_display *_display = NULL;
struct wl_registry *_registry = NULL;
static IBusBus *_bus = NULL;

static gboolean _use_sync_mode = FALSE;

static gboolean
_get_boolean_env (const gchar *name,
                  gboolean     defval)
{
    const gchar *value = g_getenv (name);

    if (value == NULL)
      return defval;

    if (g_strcmp0 (value, "") == 0 ||
        g_strcmp0 (value, "0") == 0 ||
        g_strcmp0 (value, "false") == 0 ||
        g_strcmp0 (value, "False") == 0 ||
        g_strcmp0 (value, "FALSE") == 0)
      return FALSE;

    return TRUE;
}

static gboolean
ibus_wayland_source_prepare (GSource *base,
                             gint    *timeout)
{
    IBusWaylandSource *source = (IBusWaylandSource *) base;

    *timeout = -1;

    wl_display_flush (source->display);

    return FALSE;
}

static gboolean
ibus_wayland_source_check (GSource *base)
{
    IBusWaylandSource *source = (IBusWaylandSource *) base;

    if (source->pfd.revents & (G_IO_ERR | G_IO_HUP))
        g_error ("Lost connection to wayland compositor");

    return source->pfd.revents;
}

static gboolean
ibus_wayland_source_dispatch (GSource    *base,
                              GSourceFunc callback,
                              gpointer    data)
{
    IBusWaylandSource *source = (IBusWaylandSource *) base;

    if (source->pfd.revents) {
        wl_display_dispatch (source->display);
        source->pfd.revents = 0;
    }

    return TRUE;
}

static void
ibus_wayland_source_finalize (GSource *source)
{
}

static GSourceFuncs ibus_wayland_source_funcs = {
    ibus_wayland_source_prepare,
    ibus_wayland_source_check,
    ibus_wayland_source_dispatch,
    ibus_wayland_source_finalize
};

GSource *
ibus_wayland_source_new (struct wl_display *display)
{
    GSource *source;
    IBusWaylandSource *wlsource;

    source = g_source_new (&ibus_wayland_source_funcs,
                           sizeof (IBusWaylandSource));
    wlsource = (IBusWaylandSource *) source;

    wlsource->display = display;
    wlsource->pfd.fd = wl_display_get_fd (display);
    wlsource->pfd.events = G_IO_IN | G_IO_ERR | G_IO_HUP;
    g_source_add_poll (source, &wlsource->pfd);

    return source;
}

static void
_context_commit_text_cb (IBusInputContext *context,
                         IBusText         *text,
                         IBusWaylandIM    *wlim)
{
    wl_input_method_context_commit_string (wlim->context,
                                           wlim->serial,
                                           text->text);
}

static void
_context_forward_key_event_cb (IBusInputContext *context,
                               guint             keyval,
                               guint             keycode,
                               guint             modifiers,
                               IBusWaylandIM    *wlim)
{
    uint32_t state;

    if (modifiers & IBUS_RELEASE_MASK)
        state = WL_KEYBOARD_KEY_STATE_RELEASED;
    else
        state = WL_KEYBOARD_KEY_STATE_PRESSED;

    wl_input_method_context_keysym (wlim->context,
                                    wlim->serial,
                                    0,
                                    keyval,
                                    state,
                                    modifiers);
}

static void
_context_show_preedit_text_cb (IBusInputContext *context,
                               IBusWaylandIM    *wlim)
{
    /* CURSOR is byte offset.  */
    uint32_t cursor =
        g_utf8_offset_to_pointer (wlim->preedit_text->text,
                                  wlim->preedit_cursor_pos) -
        wlim->preedit_text->text;

    wl_input_method_context_preedit_cursor (wlim->context,
                                            cursor);
    wl_input_method_context_preedit_string (wlim->context,
                                            wlim->serial,
                                            wlim->preedit_text->text,
                                            wlim->preedit_text->text);
}

static void
_context_hide_preedit_text_cb (IBusInputContext *context,
                               IBusWaylandIM    *wlim)
{
    wl_input_method_context_preedit_string (wlim->context,
                                            wlim->serial,
                                            "",
                                            "");
}

static void
_context_update_preedit_text_cb (IBusInputContext *context,
                                 IBusText         *text,
                                 gint              cursor_pos,
                                 gboolean          visible,
                                 IBusWaylandIM    *wlim)
{
    if (wlim->preedit_text)
        g_object_unref (wlim->preedit_text);
    wlim->preedit_text = g_object_ref_sink (text);
    wlim->preedit_cursor_pos = cursor_pos;

    if (visible)
        _context_show_preedit_text_cb (context, wlim);
    else
        _context_hide_preedit_text_cb (context, wlim);
}

static void
handle_surrounding_text (void                           *data,
                         struct wl_input_method_context *context,
                         const char                     *text,
                         uint32_t                        cursor,
                         uint32_t                        anchor)
{
#if ENABLE_SURROUNDING
    IBusWaylandIM *wlim = data;

    if (wlim->ibuscontext != NULL &&
        ibus_input_context_needs_surrounding_text (wlim->ibuscontext)) {
        /* CURSOR_POS and ANCHOR_POS are character offset.  */
        guint cursor_pos = g_utf8_pointer_to_offset (text, text + cursor);
        guint anchor_pos = g_utf8_pointer_to_offset (text, text + anchor);
        IBusText *ibustext = ibus_text_new_from_string (text);

        ibus_input_context_set_surrounding_text (wlim->ibuscontext,
                                                 ibustext,
                                                 cursor_pos,
                                                 anchor_pos);
    }
#endif
}

static void
handle_reset (void                           *data,
              struct wl_input_method_context *context)
{
}

static void
handle_content_type (void                           *data,
                     struct wl_input_method_context *context,
                     uint32_t                        hint,
                     uint32_t                        purpose)
{
}

static void
handle_invoke_action (void                           *data,
                      struct wl_input_method_context *context,
                      uint32_t                        button,
                      uint32_t                        index)
{
}

static void
handle_commit_state (void                           *data,
                     struct wl_input_method_context *context,
                     uint32_t                        serial)
{
    IBusWaylandIM *wlim = data;

    wlim->serial = serial;
}

static void
handle_preferred_language (void                           *data,
                           struct wl_input_method_context *context,
                           const char                     *language)
{
}

static const struct wl_input_method_context_listener context_listener = {
    handle_surrounding_text,
    handle_reset,
    handle_content_type,
    handle_invoke_action,
    handle_commit_state,
    handle_preferred_language
};

static void
input_method_keyboard_keymap (void               *data,
                              struct wl_keyboard *wl_keyboard,
                              uint32_t            format,
                              int32_t             fd,
                              uint32_t            size)
{
    IBusWaylandIM *wlim = data;
    GMappedFile *map;
    GError *error;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        close(fd);
        return;
    }

    error = NULL;
    map = g_mapped_file_new_from_fd (fd, FALSE, &error);
    if (map == NULL) {
        close (fd);
        return;
    }

    wlim->keymap =
        xkb_map_new_from_string (wlim->xkb_context,
                                 g_mapped_file_get_contents (map),
                                 XKB_KEYMAP_FORMAT_TEXT_V1,
                                 0);

    g_mapped_file_unref (map);
    close(fd);

    if (!wlim->keymap) {
        return;
    }

    wlim->state = xkb_state_new (wlim->keymap);
    if (!wlim->state) {
        xkb_map_unref (wlim->keymap);
        return;
    }

    wlim->shift_mask =
        1 << xkb_map_mod_get_index (wlim->keymap, "Shift");
    wlim->lock_mask =
        1 << xkb_map_mod_get_index (wlim->keymap, "Lock");
    wlim->control_mask =
        1 << xkb_map_mod_get_index (wlim->keymap, "Control");
    wlim->mod1_mask =
        1 << xkb_map_mod_get_index (wlim->keymap, "Mod1");
    wlim->mod2_mask =
        1 << xkb_map_mod_get_index (wlim->keymap, "Mod2");
    wlim->mod3_mask =
        1 << xkb_map_mod_get_index (wlim->keymap, "Mod3");
    wlim->mod4_mask =
        1 << xkb_map_mod_get_index (wlim->keymap, "Mod4");
    wlim->mod5_mask =
        1 << xkb_map_mod_get_index (wlim->keymap, "Mod5");
    wlim->super_mask =
        1 << xkb_map_mod_get_index (wlim->keymap, "Super");
    wlim->hyper_mask =
        1 << xkb_map_mod_get_index (wlim->keymap, "Hyper");
    wlim->meta_mask =
        1 << xkb_map_mod_get_index (wlim->keymap, "Meta");
}

static void
_process_key_event_done (GObject      *object,
                         GAsyncResult *res,
                         gpointer      user_data)
{
    IBusInputContext *context = (IBusInputContext *)object;
    IBusWaylandKeyEvent *event = (IBusWaylandKeyEvent *) user_data;

    GError *error = NULL;
    gboolean retval = ibus_input_context_process_key_event_async_finish (
            context,
            res,
            &error);

    if (error != NULL) {
        g_warning ("Process Key Event failed: %s.", error->message);
        g_error_free (error);
    }

    if (!retval) {
        wl_input_method_context_key (event->context,
                                     event->serial,
                                     event->time,
                                     event->key,
                                     event->state);
    }

    g_free (event);
}

static void
input_method_keyboard_key (void               *data,
                           struct wl_keyboard *wl_keyboard,
                           uint32_t            serial,
                           uint32_t            time,
                           uint32_t            key,
                           uint32_t            state)
{
    IBusWaylandIM *wlim = data;
    uint32_t code;
    uint32_t num_syms;
    const xkb_keysym_t *syms;
    xkb_keysym_t sym;
    guint32 modifiers;

    if (!wlim->state)
        return;

    if (!wlim->ibuscontext) {
        wl_input_method_context_key (wlim->context,
                                     serial,
                                     time,
                                     key,
                                     state);
        return;
    }
        
    code = key + 8;
    num_syms = xkb_key_get_syms (wlim->state, code, &syms);

    sym = XKB_KEY_NoSymbol;
    if (num_syms == 1)
        sym = syms[0];

    modifiers = wlim->modifiers;
    if (state == WL_KEYBOARD_KEY_STATE_RELEASED)
        modifiers |= IBUS_RELEASE_MASK;

    if (_use_sync_mode) {
        gboolean retval =
            ibus_input_context_process_key_event (wlim->ibuscontext,
                                                  sym,
                                                  code,
                                                  modifiers);
        if (!retval) {
            wl_input_method_context_key (wlim->context,
                                         serial,
                                         time,
                                         key,
                                         state);
        }
    } else {
        IBusWaylandKeyEvent *event = g_new (IBusWaylandKeyEvent, 1);
        event->context = wlim->context;
        event->serial = serial;
        event->time = time;
        event->key = key;
        event->state = state;
        ibus_input_context_process_key_event_async (wlim->ibuscontext,
                                                    sym,
                                                    code,
                                                    modifiers,
                                                    -1,
                                                    NULL,
                                                    _process_key_event_done,
                                                    event);
    }
}

static void
input_method_keyboard_modifiers (void               *data,
                                 struct wl_keyboard *wl_keyboard,
                                 uint32_t            serial,
                                 uint32_t            mods_depressed,
                                 uint32_t            mods_latched,
                                 uint32_t            mods_locked,
                                 uint32_t            group)
{
    IBusWaylandIM *wlim = data;
    struct wl_input_method_context *context = wlim->context;
    xkb_mod_mask_t mask;

    xkb_state_update_mask (wlim->state, mods_depressed,
                           mods_latched, mods_locked, 0, 0, group);
    mask = xkb_state_serialize_mods (wlim->state,
                                     XKB_STATE_DEPRESSED |
                                     XKB_STATE_LATCHED);

    wlim->modifiers = 0;
    if (mask & wlim->shift_mask)
        wlim->modifiers |= IBUS_SHIFT_MASK;
    if (mask & wlim->lock_mask)
        wlim->modifiers |= IBUS_LOCK_MASK;
    if (mask & wlim->control_mask)
        wlim->modifiers |= IBUS_CONTROL_MASK;
    if (mask & wlim->mod1_mask)
        wlim->modifiers |= IBUS_MOD1_MASK;
    if (mask & wlim->mod2_mask)
        wlim->modifiers |= IBUS_MOD2_MASK;
    if (mask & wlim->mod3_mask)
        wlim->modifiers |= IBUS_MOD3_MASK;
    if (mask & wlim->mod4_mask)
        wlim->modifiers |= IBUS_MOD4_MASK;
    if (mask & wlim->mod5_mask)
        wlim->modifiers |= IBUS_MOD5_MASK;
    if (mask & wlim->super_mask)
        wlim->modifiers |= IBUS_SUPER_MASK;
    if (mask & wlim->hyper_mask)
        wlim->modifiers |= IBUS_HYPER_MASK;
    if (mask & wlim->meta_mask)
        wlim->modifiers |= IBUS_META_MASK;

    wl_input_method_context_modifiers (context, serial,
                                       mods_depressed, mods_latched,
                                       mods_locked, group);
}

static const struct wl_keyboard_listener keyboard_listener = {
    input_method_keyboard_keymap,
    NULL, /* enter */
    NULL, /* leave */
    input_method_keyboard_key,
    input_method_keyboard_modifiers
};

static void
_create_input_context_done (GObject      *object,
                            GAsyncResult *res,
                            gpointer      user_data)
{
    IBusWaylandIM *wlim = (IBusWaylandIM *) user_data;

    GError *error = NULL;
    IBusInputContext *context = ibus_bus_create_input_context_async_finish (
            _bus, res, &error);

    if (wlim->cancellable != NULL) {
        g_object_unref (wlim->cancellable);
        wlim->cancellable = NULL;
    }

    if (context == NULL) {
        g_warning ("Create input context failed: %s.", error->message);
        g_error_free (error);
    }
    else {
        wlim->ibuscontext = context;

        g_signal_connect (wlim->ibuscontext, "commit-text",
                          G_CALLBACK (_context_commit_text_cb),
                          wlim);
        g_signal_connect (wlim->ibuscontext, "forward-key-event",
                          G_CALLBACK (_context_forward_key_event_cb),
                          wlim);

        g_signal_connect (wlim->ibuscontext, "update-preedit-text",
                          G_CALLBACK (_context_update_preedit_text_cb),
                          wlim);
        g_signal_connect (wlim->ibuscontext, "show-preedit-text",
                          G_CALLBACK (_context_show_preedit_text_cb),
                          wlim);
        g_signal_connect (wlim->ibuscontext, "hide-preedit-text",
                          G_CALLBACK (_context_hide_preedit_text_cb),
                          wlim);
    
#ifdef ENABLE_SURROUNDING
        ibus_input_context_set_capabilities (wlim->ibuscontext,
                                             IBUS_CAP_FOCUS |
                                             IBUS_CAP_PREEDIT_TEXT |
                                             IBUS_CAP_SURROUNDING_TEXT);
#else
        ibus_input_context_set_capabilities (wlim->ibuscontext,
                                             IBUS_CAP_FOCUS |
                                             IBUS_CAP_PREEDIT_TEXT);
#endif

        ibus_input_context_focus_in (wlim->ibuscontext);
    }
}

static void
input_method_activate (void                           *data,
                       struct wl_input_method         *input_method,
                       struct wl_input_method_context *context)
{
    IBusWaylandIM *wlim = data;

    if (wlim->context) {
        wl_input_method_context_destroy (wlim->context);
        wlim->context = NULL;
    }

    wlim->serial = 0;
    wlim->context = context;

    wl_input_method_context_add_listener (context, &context_listener, wlim);
    wlim->keyboard = wl_input_method_context_grab_keyboard (context);
    wl_keyboard_add_listener(wlim->keyboard,
                             &keyboard_listener,
                             wlim);

    if (wlim->ibuscontext) {
        g_object_unref (wlim->ibuscontext);
        wlim->ibuscontext = NULL;
    }

    wlim->cancellable = g_cancellable_new ();
    ibus_bus_create_input_context_async (_bus,
                                         "wayland",
                                         -1,
                                         wlim->cancellable,
                                         _create_input_context_done,
                                         wlim);
}

static void
input_method_deactivate (void                           *data,
                         struct wl_input_method         *input_method,
                         struct wl_input_method_context *context)
{
    IBusWaylandIM *wlim = data;

    if (wlim->cancellable) {
        /* Cancel any ongoing create input context request.  */
        g_cancellable_cancel (wlim->cancellable);
        g_object_unref (wlim->cancellable);
        wlim->cancellable = NULL;
    }

    if (wlim->ibuscontext) {
        ibus_input_context_focus_out (wlim->ibuscontext);
        g_object_unref (wlim->ibuscontext);
        wlim->ibuscontext = NULL;
    }

    if (wlim->preedit_text) {
        g_object_unref (wlim->preedit_text);
        wlim->preedit_text = NULL;
    }

    if (wlim->context) {
        wl_input_method_context_destroy (wlim->context);
        wlim->context = NULL;
    }
}

static const struct wl_input_method_listener input_method_listener = {
    input_method_activate,
    input_method_deactivate
};

static void
registry_handle_global (void               *data,
                        struct wl_registry *registry,
                        uint32_t            name,
                        const char         *interface,
                        uint32_t            version)
{
    IBusWaylandIM *wlim = data;

    if (!g_strcmp0 (interface, "wl_input_method")) {
        wlim->input_method =
            wl_registry_bind (registry, name, &wl_input_method_interface, 1);
        wl_input_method_add_listener (wlim->input_method,
                                      &input_method_listener, wlim);
    }
}

static void
registry_handle_global_remove (void               *data,
                               struct wl_registry *registry,
                               uint32_t            name)
{
}

static const struct wl_registry_listener registry_listener = {
    registry_handle_global,
    registry_handle_global_remove
};

gint
main (gint    argc,
      gchar **argv)
{
    IBusWaylandIM wlim;
    GSource *source;

    ibus_init ();

    _bus = ibus_bus_new ();
    if (!ibus_bus_is_connected (_bus)) {
        g_printerr ("Cannot connect to ibus-daemon\n");
        return EXIT_FAILURE;
    }

    _display = wl_display_connect (NULL);
    if (_display == NULL) {
        g_printerr ("Failed to connect to Wayland server: %s\n",
                    g_strerror (errno));
        return EXIT_FAILURE;
    }

    _registry = wl_display_get_registry (_display);
    memset (&wlim, 0, sizeof (wlim));
    wl_registry_add_listener (_registry, &registry_listener, &wlim);
    wl_display_roundtrip (_display);
    if (wlim.input_method == NULL) {
        g_printerr ("No input_method global\n");
        return EXIT_FAILURE;
    }

    wlim.xkb_context = xkb_context_new (0);
    if (wlim.xkb_context == NULL) {
        g_printerr ("Failed to create XKB context\n");
        return EXIT_FAILURE;
    }

    _use_sync_mode = _get_boolean_env ("IBUS_ENABLE_SYNC_MODE", FALSE);

    source = ibus_wayland_source_new (_display);
    g_source_set_priority (source, G_PRIORITY_DEFAULT);
    g_source_set_can_recurse (source, TRUE);
    g_source_attach (source, NULL);

    ibus_main ();

    return EXIT_SUCCESS;
}
