/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/**
 * SECTION: ibusinputcontext
 * @short_description: IBus input context proxy object.
 * @stability: Stable
 *
 * An IBusInputContext is a proxy object of BusInputContext,
 * which manages the context for input methods that supports
 * text input in various natural languages.
 *
 * Clients call the IBusInputContext to invoke BusInputContext,
 * through which invokes IBusEngine.
 */
#ifndef __IBUS_INPUT_CONTEXT_H_
#define __IBUS_INPUT_CONTEXT_H_

#include "ibusproxy.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_INPUT_CONTEXT             \
    (ibus_input_context_get_type ())
#define IBUS_INPUT_CONTEXT(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_INPUT_CONTEXT, IBusInputContext))
#define IBUS_INPUT_CONTEXT_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_INPUT_CONTEXT, IBusInputContextClass))
#define IBUS_IS_INPUT_CONTEXT(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_INPUT_CONTEXT))
#define IBUS_IS_INPUT_CONTEXT_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_INPUT_CONTEXT))
#define IBUS_INPUT_CONTEXT_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_INPUT_CONTEXT, IBusInputContextClass))

G_BEGIN_DECLS

typedef struct _IBusInputContext IBusInputContext;
typedef struct _IBusInputContextClass IBusInputContextClass;

/**
 * IBusInputContext:
 *
 * An opaque data type representing an IBusInputContext.
 */
struct _IBusInputContext {
  IBusProxy parent;
  /* instance members */
};

struct _IBusInputContextClass {
    IBusProxyClass parent;
    /* signals */

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType        ibus_input_context_get_type    (void);

/**
 * ibus_input_context_new:
 * @path: The path to the object that emitting the signal.
 * @connection: An IBusConnection.
 * @returns: A newly allocated IBusInputContext.
 *
 * New an IBusInputContext.
 */
IBusInputContext
            *ibus_input_context_new         (const gchar        *path,
                                             IBusConnection     *connection);
/**
 * ibus_input_context_process_key_event:
 * @context: An IBusInputContext.
 * @keyval: Key symbol of a key event.
 * @keycode: Keycode of a key event.
 * @state: Key modifier flags.
 * @returns: TRUE for successfully process the key; FALSE otherwise.
 *
 * Pass the key event to input method engine.
 *
 * Key symbols are characters/symbols produced by key press, for example,
 * pressing "s" generates key symbol "s"; pressing shift-"s" generates key symbol "S".
 * Same key on keyboard may produce different key symbols on different keyboard layout.
 * e.g., "s" key on QWERTY keyboard produces "o" in DVORAK layout.
 *
 * Unlike key symbol, keycode is only determined by the location of the key, and
 * irrelevant of the keyboard layout.
 *
 * Briefly speaking, input methods that expect certain keyboard layout should use
 * keycode; otherwise keyval is sufficient.
 * For example, Chewing, Cangjie, Wubi expect an en-US QWERTY keyboard, these should
 * use keycode; while pinyin can rely on keyval only, as it is less sensitive to
 * the keyboard layout change, DVORAK users can still use DVORAK layout to input pinyin.
 *
 * Use ibus_keymap_lookup_keysym() to convert keycode to keysym in given keyboard layout.
 *
 * @see_also: #IBusEngine::process-key-event
 */
gboolean     ibus_input_context_process_key_event
                                            (IBusInputContext   *context,
                                             guint32             keyval,
                                             guint32             keycode,
                                             guint32             state);

/**
 * ibus_input_context_set_cursor_location:
 * @context: An IBusInputContext.
 * @x: X coordinate of the cursor.
 * @y: Y coordinate of the cursor.
 * @w: Width of the cursor.
 * @h: Height of the cursor.
 *
 * Set the cursor location of IBus input context.
 *
 * @see_also: #IBusEngine::set-cursor-location
 */
void         ibus_input_context_set_cursor_location
                                            (IBusInputContext   *context,
                                             gint32              x,
                                             gint32              y,
                                             gint32              w,
                                             gint32              h);
/**
 * ibus_input_context_set_capabilities:
 * @context: An IBusInputContext.
 * @capabilities: Capabilities flags of IBusEngine, see #IBusCapabilite
 *
 * Set the capabilities flags of client application.
 *
 * @see_also: #IBusEngine::set-capabilities
 */
void         ibus_input_context_set_capabilities
                                            (IBusInputContext   *context,
                                             guint32             capabilites);

/**
 * ibus_input_context_focus_in:
 * @context: An IBusInputContext.
 *
 * Invoked when the client application get focus.
 *
 * @see_also: #IBusEngine::focus_in.
 */
void         ibus_input_context_focus_in    (IBusInputContext   *context);

/**
 * ibus_input_context_focus_out:
 * @context: An IBusInputContext.
 *
 * Invoked when the client application get focus.
 *
 * @see_also: #IBusEngine::focus_out.
 */
void         ibus_input_context_focus_out   (IBusInputContext   *context);


/**
 * ibus_input_context_reset:
 * @context: An IBusInputContext.
 *
 * Invoked when the IME is reset.
 *
 * @see_also: #IBusEngine::reset
 */
void         ibus_input_context_reset       (IBusInputContext   *context);

/**
 * ibus_input_context_enable:
 * @context: An IBusInputContext.
 *
 * Invoked when the IME is enabled, either by IME switch hotkey or select from the menu.
 *
 * @see_also: #IBusEngine::enable
 */
void         ibus_input_context_enable      (IBusInputContext   *context);

/**
 * ibus_input_context_disable:
 * @context: An IBusInputContext.
 *
 * Invoked when the IME is disabled, either by IME switch hotkey or select from the menu.
 *
 * @see_also: #IBusEngine::disable
 */
void         ibus_input_context_disable     (IBusInputContext   *context);

G_END_DECLS
#endif

