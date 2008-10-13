/* vim:set et sts=4: */
/* bus - The Input Bus
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

#include "server.h"

#define BUS_SERVER_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_SERVER, BusServerPrivate))
#define DECLARE_PRIV BusServerPrivate *priv = BUS_SERVER_GET_PRIVATE(server)

enum {
    NEW_CONNECTION,
    LAST_SIGNAL,
};


/* BusServerPriv */
struct _BusServerPrivate {
    DBusServer *server;
};
typedef struct _BusServerPrivate BusServerPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_server_class_init  (BusServerClass      *klass);
static void     bus_server_init        (BusServer           *server);
static void     bus_server_dispose     (BusServer           *server);
static void     bus_server_new_connection
                                        (BusServer          *server,
                                         IBusConnection     *connection);

static IBusObjectClass  *_parent_class = NULL;

GType
bus_server_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusServerClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_server_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusServer),
        0,
        (GInstanceInitFunc) bus_server_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "BusServer",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}

BusServer *
bus_server_new (void)
{
    BusServer *server;
    server = BUS_SERVER (g_object_new (BUS_TYPE_SERVER, NULL));
    return server;
}

gboolean
bus_server_listen (BusServer *server)
{
    g_assert (BUS_IS_SERVER (server));

    const gchar *address = ibus_get_address ();

    return ibus_server_listen (IBUS_SERVER (server), address);
}

static void
bus_server_class_init (BusServerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    _parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusServerPrivate));

    gobject_class->dispose = (GObjectFinalizeFunc) bus_server_dispose;

    IBUS_SERVER_CLASS (klass)->new_connection = (IBusNewConnectionFunc) bus_server_new_connection;
}

static void
bus_server_init (BusServer *server)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);
}

static void
bus_server_new_connection   (BusServer          *server,
                             IBusConnection     *connection)
{
}

static void
bus_server_dispose (BusServer *server)
{
    G_OBJECT_CLASS (_parent_class)->dispose (G_OBJECT (server));
}
