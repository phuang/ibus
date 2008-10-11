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
    (G_TYPE_CHECK_GET_CLASS ((obj), IBUS_TYPE_ENGINE, IBusEngineClass))

G_BEGIN_DECLS

typedef struct _IBusEngine IBusEngine;
typedef struct _IBusEngineClass IBusEngineClass;

struct _IBusEngine {
    IBusService parent;
    /* instance members */
};

struct _IBusEngineClass {
    IBusServiceClass parent;

    /* class members */
    gboolean    (* key_press)       (IBusEngine     *engine,
                                     guint          keyval,
                                     gboolean       is_press,
                                     guint          state);
    void        (* focus_in)        (IBusEngine     *engine);
    void        (* focus_out)       (IBusEngine     *engine);
    void        (* reset)           (IBusEngine     *engine);
    void        (* enable)          (IBusEngine     *engine);
    void        (* disable)         (IBusEngine     *engine);
    void        (* set_cursor_location)
                                    (IBusEngine     *engine,
                                    gint            x,
                                    gint            y,
                                    gint            w,
                                    gint            h);

    void        (* page_up)         (IBusEngine     *engine);
    void        (* page_down)       (IBusEngine     *engine);
    void        (* cursor_up)       (IBusEngine     *engine);
    void        (* cursor_down)     (IBusEngine     *engine);

    void        (* property_activate)
                                    (IBusEngine     *engine,
                                     const gchar    *prop_name,
                                     gint            prop_state);
    void        (* property_show)   (IBusEngine     *engine);
    void        (* property_hide)   (IBusEngine     *engine);

};

GType        ibus_engine_get_type        (void);
IBusEngine  *ibus_engine_new             (const gchar       *path);
gboolean     ibus_engine_handle_message  (IBusEngine        *engine,
                                          IBusConnection    *connection,
                                          DBusMessage       *message);
void         ibus_engine_commit_string   (IBusEngine        *engine,
                                          const gchar       *text);
void         ibus_engine_update_preedit  (IBusEngine        *engine,
                                          const gchar       *text,
                                          IBusAttrList      *attr_list,
                                          gint               cursor_pos,
                                          gboolean           visible);
void         ibus_engine_show_preedit    (IBusEngine        *engine);
void         ibus_engine_hide_preedit    (IBusEngine        *engine);
void         ibus_engine_update_aux_string
                                         (IBusEngine        *engine,
                                          const gchar       *text,
                                          IBusAttrList      *attr_list,
                                          gboolean           visible);
void         ibus_engine_show_aux_string (IBusEngine        *engine);
void         ibus_engine_hide_aux_string (IBusEngine        *engine);
void         ibus_engine_update_lookup_table
                                         (IBusEngine        *engine,
                                          IBusLookupTable   *lookup_table,
                                          gboolean           visible);
void         ibus_engine_show_lookup_table
                                         (IBusEngine        *engine);
void         ibus_engine_hide_lookup_table
                                         (IBusEngine        *engine);

void         ibus_engine_forward_key_event
                                         (IBusEngine        *engine,
                                          guint             keyval,
                                          gboolean          is_press,
                                          guint             state);

G_END_DECLS
#endif

