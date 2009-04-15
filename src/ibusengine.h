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
 * SECTION: ibusengine
 * @short_description: Input method engine abstract.
 * @stability: Stable
 * @see_also: #IBusComponent, #IBusEngineDesc
 *
 * An IBusEngine provides infrastructure for input method engine.
 * Developers can "extend" this class for input method engine development.
 */
#ifndef __IBUS_ENGINE_H_
#define __IBUS_ENGINE_H_

#include <dbus/dbus.h>
#include "ibusservice.h"
#include "ibusattribute.h"
#include "ibuslookuptable.h"
#include "ibusproperty.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_ENGINE             \
    (ibus_engine_get_type ())
#define IBUS_ENGINE(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_ENGINE, IBusEngine))
#define IBUS_ENGINE_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_ENGINE, IBusEngineClass))
#define IBUS_IS_ENGINE(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_ENGINE))
#define IBUS_IS_ENGINE_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_ENGINE))
#define IBUS_ENGINE_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_ENGINE, IBusEngineClass))

G_BEGIN_DECLS

typedef struct _IBusEngine IBusEngine;
typedef struct _IBusEngineClass IBusEngineClass;

/**
 * IBusEngine:
 * @enabled: Whether the engine is enabled.
 * @has_focus: Whether the engine has focus.
 * @cursor_area: Area of cursor.
 * @client_capabilities: IBusCapabilite (client capabilities) flags.
 *
 * IBusEngine properties.
 */
struct _IBusEngine {
    IBusService parent;
    /* instance members */
    /*< public >*/
    gboolean enabled;
    gboolean has_focus;

    /* cursor location */
    IBusRectangle cursor_area;
    guint client_capabilities;
};

struct _IBusEngineClass {
    IBusServiceClass parent;

    /* class members */
    gboolean    (* process_key_event)
                                    (IBusEngine     *engine,
                                     guint           keyval,
                                     guint           state);
    void        (* focus_in)        (IBusEngine     *engine);
    void        (* focus_out)       (IBusEngine     *engine);
    void        (* reset)           (IBusEngine     *engine);
    void        (* enable)          (IBusEngine     *engine);
    void        (* disable)         (IBusEngine     *engine);
    void        (* set_cursor_location)
                                    (IBusEngine     *engine,
                                    gint             x,
                                    gint             y,
                                    gint             w,
                                    gint             h);
    void        (* set_capabilities)
                                    (IBusEngine     *engine,
                                     guint           caps);

    void        (* page_up)         (IBusEngine     *engine);
    void        (* page_down)       (IBusEngine     *engine);
    void        (* cursor_up)       (IBusEngine     *engine);
    void        (* cursor_down)     (IBusEngine     *engine);

    void        (* property_activate)
                                    (IBusEngine     *engine,
                                     const gchar    *prop_name,
                                     guint           prop_state);
    void        (* property_show)   (IBusEngine     *engine,
                                     const gchar    *prop_name);
    void        (* property_hide)   (IBusEngine     *engine,
                                     const gchar    *prop_name);

    /*< private >*/
    /* padding */
    gpointer pdummy[9];
};

GType        ibus_engine_get_type       (void);

/**
 * ibus_engine_new:
 * @name: Name of the IBusObject.
 * @path: Path for IBusService.
 * @connection: An opened IBusConnection.
 * @returns: A newly allocated IBusEngine.
 *
 * New an IBusEngine.
 */
IBusEngine  *ibus_engine_new            (const gchar        *name,
        const gchar        *path,
        IBusConnection     *connection);

/**
 * ibus_engine_commit_text:
 * @engine: An IBusEngine.
 * @text: String commit to IBusEngine.
 *
 * Commit output of input method to IBus client.
 */
void         ibus_engine_commit_text    (IBusEngine         *engine,
        IBusText           *text);

/**
 * ibus_engine_update_preedit_text:
 * @engine: An IBusEngine.
 * @text: Update content.
 * @cursor_pos: Current position of cursor
 * @visible: Whether the pre-edit buffer is visible.
 *
 * Update the pre-edit buffer.
 */
void         ibus_engine_update_preedit_text
(IBusEngine         *engine,
 IBusText           *text,
 guint               cursor_pos,
 gboolean            visible);

/**
 * ibus_engine_show_preedit_text:
 * @engine: An IBusEngine.
 *
 * Show the pre-edit buffer.
 */
void         ibus_engine_show_preedit_text
(IBusEngine         *engine);

/**
 * ibus_engine_hide_preedit_text:
 * @engine: An IBusEngine.
 *
 * Hide the pre-edit buffer.
 */
void         ibus_engine_hide_preedit_text
(IBusEngine         *engine);

/**
 * ibus_engine_update_auxiliary_text:
 * @engine: An IBusEngine.
 * @text: Update content.
 * @visible: Whether the auxiliary text bar is visible.
 *
 * Update the auxiliary bar.
 */
void         ibus_engine_update_auxiliary_text
(IBusEngine        *engine,
 IBusText          *text,
 gboolean           visible);

/**
 * ibus_engine_show_auxiliary_text:
 * @engine: An IBusEngine.
 *
 * Show the auxiliary bar.
 */
void         ibus_engine_show_auxiliary_text
(IBusEngine         *engine);

/**
 * ibus_engine_hide_auxiliary_text:
 * @engine: An IBusEngine.
 *
 * Hide the auxiliary bar.
 */
void         ibus_engine_hide_auxiliary_text
(IBusEngine         *engine);

/**
 * ibus_engine_update_lookup_table:
 * @engine: An IBusEngine.
 * @lookup_table: An lookup_table.
 * @visible: Whether the lookup_table is visible.
 *
 * Update the lookup table.
 */
void         ibus_engine_update_lookup_table
(IBusEngine         *engine,
 IBusLookupTable    *lookup_table,
 gboolean            visible);

/**
 * ibus_engine_update_lookup_table_fast:
 * @engine: An IBusEngine.
 * @lookup_table: An lookup_table.
 * @visible: Whether the lookup_table is visible.
 *
 * Fast update for big lookup table.
 *
 * If size of lookup table is not over table page size *4,
 * then it calls ibus_engine_update_lookup_table().
 *
 */
void         ibus_engine_update_lookup_table_fast
(IBusEngine         *engine,
 IBusLookupTable    *lookup_table,
 gboolean            visible);

/**
 * ibus_engine_show_lookup_table:
 * @engine: An IBusEngine.
 *
 * Show the lookup table.
 */
void         ibus_engine_show_lookup_table
(IBusEngine         *engine);

/**
 * ibus_engine_hide_lookup_table:
 * @engine: An IBusEngine.
 *
 * Hide the lookup table.
 */
void         ibus_engine_hide_lookup_table
(IBusEngine         *engine);

/**
 * ibus_engine_forward_key_event:
 * @engine: An IBusEngine.
 * @keyval: KeySym.
 * @state: Key modifier flags.
 *
 * Forward the key event.
 */
void         ibus_engine_forward_key_event
(IBusEngine         *engine,
 guint               keyval,
 guint               state);

/**
 * ibus_engine_register_properties:
 * @engine: An IBusEngine.
 * @prop_list: Property List.
 *
 * Register and show properties in language bar.
 */
void         ibus_engine_register_properties
(IBusEngine         *engine,
 IBusPropList       *prop_list);

/**
 * ibus_engine_update_property:
 * @engine: An IBusEngine.
 * @prop: IBusProperty to be updated.
 *
 * Update the state displayed in language bar.
 */
void         ibus_engine_update_property(IBusEngine         *engine,
        IBusProperty       *prop);

/**
 * ibus_engine_get_name:
 * @engine: An IBusEngine.
 * @returns: Name of IBusEngine.
 *
 * Return the name of IBusEngine.
 */
const gchar *ibus_engine_get_name       (IBusEngine         *engine);

G_END_DECLS
#endif

