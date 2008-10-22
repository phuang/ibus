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
#include "connection.h"
#include "dbusimpl.h"
#include "ibusimpl.h"

#define BUS_SERVER_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_SERVER, BusServerPrivate))
#define DECLARE_PRIV BusServerPrivate *priv = BUS_SERVER_GET_PRIVATE(server)

enum {
    NEW_CONNECTION,
    LAST_SIGNAL,
};


/* BusServerPriv */
struct _BusServerPrivate {
    GMainLoop   *loop;
    BusDBusImpl *dbus;
    BusIBusImpl *ibus;
};
typedef struct _BusServerPrivate BusServerPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void      bus_server_class_init  (BusServerClass     *klass);
static void      bus_server_init        (BusServer          *server);
static void      bus_server_destroy     (BusServer          *server);
static void      bus_server_new_connection
                                        (BusServer          *server,
                                         BusConnection      *connection);

static IBusObjectClass  *_parent_class = NULL;
static BusServer        *_server = NULL;

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
        type = g_type_register_static (IBUS_TYPE_SERVER,
                    "BusServer",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}

BusServer *
bus_server_get_default (void)
{
    static BusServer *server = NULL;

    if (server == NULL) {
        server = BUS_SERVER (g_object_new (BUS_TYPE_SERVER,
                        "connection-type", BUS_TYPE_CONNECTION,
                        NULL));
    }
    return server;
}

gboolean
bus_server_listen (BusServer *server)
{
    g_assert (BUS_IS_SERVER (server));

    // const gchar *address = "unix:abstract=/tmp/ibus-c"
    const gchar *address = ibus_get_address ();

    return ibus_server_listen (IBUS_SERVER (server), address);
}

void
bus_server_run (BusServer *server)
{
    g_assert (BUS_IS_SERVER (server));
    
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);

    g_main_loop_run (priv->loop);
}

void
bus_server_quit (BusServer *server)
{
    g_assert (BUS_IS_SERVER (server));

    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);

    g_main_loop_quit (priv->loop);
}

static void
bus_server_class_init (BusServerClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    _parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusServerPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_server_destroy;

    IBUS_SERVER_CLASS (klass)->new_connection = (IBusNewConnectionFunc) bus_server_new_connection;
}

static void
bus_server_init (BusServer *server)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);
    
    priv->loop = g_main_loop_new (NULL, FALSE);
    priv->dbus = bus_dbus_impl_get_default ();
    priv->ibus = bus_ibus_impl_get_default ();
}

static void
bus_server_new_connection   (BusServer          *server,
                             BusConnection      *connection)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (BUS_IS_SERVER (server));

    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);

    bus_dbus_impl_new_connection (priv->dbus, connection);
    bus_ibus_impl_new_connection (priv->ibus, connection);
}

static void
bus_server_destroy (BusServer *server)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);
    
    while (g_main_loop_is_running (priv->loop)) {
        g_main_loop_quit (priv->loop);
    }
    g_main_loop_unref (priv->loop);
    g_object_unref (G_OBJECT (priv->dbus));
    g_object_unref (G_OBJECT (priv->ibus));
    
    IBUS_OBJECT_CLASS (_parent_class)->destroy (IBUS_OBJECT (server));
}
