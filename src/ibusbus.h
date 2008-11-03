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
#ifndef __IBUS_BUS_H_
#define __IBUS_BUS_H_

#include <dbus/dbus.h>
#include "ibusinputcontext.h"
#include "ibusconfig.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_BUS             \
    (ibus_bus_get_type ())
#define IBUS_BUS(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_BUS, IBusBus))
#define IBUS_BUS_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_BUS, IBusBusClass))
#define IBUS_IS_BUS(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_BUS))
#define IBUS_IS_BUS_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_BUS))
#define IBUS_BUS_GET_CLASS(obj)   \
    (G_TYPE_CHECK_GET_CLASS ((obj), IBUS_TYPE_BUS, IBusBusClass))

G_BEGIN_DECLS

typedef struct _IBusBus IBusBus;
typedef struct _IBusBusClass IBusBusClass;

struct _IBusBus {
  IBusObject parent;
  /* instance members */
};

struct _IBusBusClass {
  IBusObjectClass parent;
  /* class members */
};

GType        ibus_bus_get_type          (void);
IBusBus     *ibus_bus_new               (void);
gboolean     ibus_bus_is_connected      (IBusBus        *bus);
void         ibus_bus_set_watch_dbus_signal
                                        (IBusBus        *bus,
                                         gboolean        watch);
IBusConfig  *ibus_bus_get_config        (IBusBus        *bus);

/* declare dbus methods */
const gchar *ibus_bus_hello             (IBusBus        *bus);
guint        ibus_bus_request_name      (IBusBus        *bus,
                                         const gchar    *name,
                                         guint           flags);
guint        ibus_bus_release_name      (IBusBus        *bus,
                                         const gchar    *name);
gboolean     ibus_bus_name_has_owner    (IBusBus        *bus,
                                         const gchar    *name);
GList       *ibus_bus_list_names        (IBusBus        *bus);
void         ibus_bus_add_match         (IBusBus        *bus,
                                         const gchar    *rule);
void         ibus_bus_remove_match      (IBusBus        *bus,
                                         const gchar    *rule);
const gchar *ibus_bus_get_name_owner    (IBusBus        *bus,
                                         const gchar    *name);
/* declare ibus methods */
IBusInputContext
            *ibus_bus_create_input_context
                                        (IBusBus        *bus,
                                         const gchar    *client_name);

G_END_DECLS
#endif

