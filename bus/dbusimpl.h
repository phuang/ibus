/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2008-2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2010 Red Hat, Inc.
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
#ifndef __DBUS_IMPL_H_
#define __DBUS_IMPL_H_

#include <ibus.h>
#include "connection.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_DBUS_IMPL             \
    (bus_dbus_impl_get_type ())
#define BUS_DBUS_IMPL(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_DBUS_IMPL, BusDBusImpl))
#define BUS_DBUS_IMPL_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_DBUS_IMPL, BusDBusImplClass))
#define BUS_IS_DBUS_IMPL(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_DBUS_IMPL))
#define BUS_IS_DBUS_IMPL_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_DBUS_IMPL))
#define BUS_DBUS_IMPL_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_DBUS_IMPL, BusDBusImplClass))

#define BUS_DEFAULT_DBUS \
    (bus_dbus_impl_get_default ())

G_BEGIN_DECLS

typedef struct _BusDBusImpl BusDBusImpl;
typedef struct _BusDBusImplClass BusDBusImplClass;

struct _BusDBusImpl {
    IBusService parent;
    /* instance members */
    GHashTable *unique_names;
    GHashTable *names;
    GHashTable *objects;
    GList *connections;
    GList *rules;
    gint id;
};

struct _BusDBusImplClass {
    IBusServiceClass parent;

    /* class members */
    void    (* name_owner_changed) (BusDBusImpl     *dbus,
                                    gchar           *name,
                                    gchar           *old_name,
                                    gchar           *new_name);
};

GType            bus_dbus_impl_get_type         (void);
BusDBusImpl     *bus_dbus_impl_get_default      (void);
gboolean         bus_dbus_impl_new_connection   (BusDBusImpl    *dbus,
                                                 BusConnection  *connection);
BusConnection   *bus_dbus_impl_get_connection_by_name
                                                (BusDBusImpl    *dbus,
                                                 const gchar    *name);
void             bus_dbus_impl_dispatch_message (BusDBusImpl    *dbus,
                                                 DBusMessage    *message);
void             bus_dbus_impl_dispatch_message_by_rule
                                                (BusDBusImpl    *dbus,
                                                 DBusMessage    *message,
                                                 BusConnection  *skip_connection);
gboolean         bus_dbus_impl_register_object  (BusDBusImpl    *dbus,
                                                 IBusService    *object);
gboolean         bus_dbus_impl_unregister_object(BusDBusImpl    *dbus,
                                                 IBusService    *object);

G_END_DECLS
#endif

