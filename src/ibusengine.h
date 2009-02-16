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

struct _IBusEngine {
    IBusService parent;
    /* instance members */
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
IBusEngine  *ibus_engine_new            (const gchar        *name,
                                         const gchar        *path,
                                         IBusConnection     *connection);
void         ibus_engine_commit_text    (IBusEngine         *engine,
                                         IBusText           *text);
void         ibus_engine_update_preedit_text
                                        (IBusEngine         *engine,
                                         IBusText           *text,
                                         guint               cursor_pos,
                                         gboolean            visible);
void         ibus_engine_show_preedit_text
                                        (IBusEngine         *engine);
void         ibus_engine_hide_preedit_text
                                        (IBusEngine         *engine);
void         ibus_engine_update_auxiliary_text
                                         (IBusEngine        *engine,
                                          IBusText          *text,
                                          gboolean           visible);
void         ibus_engine_show_auxiliary_text
                                        (IBusEngine         *engine);
void         ibus_engine_hide_auxiliary_text
                                        (IBusEngine         *engine);
void         ibus_engine_update_lookup_table
                                        (IBusEngine         *engine,
                                         IBusLookupTable    *lookup_table,
                                         gboolean            visible);
void         ibus_engine_show_lookup_table
                                        (IBusEngine         *engine);
void         ibus_engine_hide_lookup_table
                                        (IBusEngine         *engine);

void         ibus_engine_forward_key_event
                                        (IBusEngine         *engine,
                                         guint               keyval,
                                         gboolean            is_press,
                                         guint               state);

void         ibus_engine_register_properties
                                        (IBusEngine         *engine,
                                         IBusPropList       *prop_list);
void         ibus_engine_update_property(IBusEngine         *engine,
                                         IBusProperty       *prop);
const gchar *ibus_engine_get_name       (IBusEngine         *engine);

G_END_DECLS
#endif

