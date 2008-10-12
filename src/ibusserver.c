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

#include "ibusserver.h"
#include "ibusinternal.h"

#define IBUS_SERVER_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_SERVER, IBusServerPrivate))
#define DECLARE_PRIV IBusServerPrivate *priv = IBUS_SERVER_GET_PRIVATE(server)

enum {
    NEW_CONNECTION,
    LAST_SIGNAL,
};


/* IBusServerPriv */
struct _IBusServerPrivate {
    DBusServer *server;
};
typedef struct _IBusServerPrivate IBusServerPrivate;

static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_server_class_init  (IBusServerClass    *klass);
static void     ibus_server_init        (IBusServer         *server);
static void     ibus_server_finalize    (IBusServer         *server);

static void     ibus_server_listen_internal
                                        (IBusServer         *server,
                                         const gchar        *address);
static void     ibus_server_new_connection
                                        (IBusServer         *server,
                                         IBusConnection     *connection);

static IBusObjectClass  *_parent_class = NULL;

GType
ibus_server_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusServerClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_server_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusServer),
        0,
        (GInstanceInitFunc) ibus_server_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "IBusServer",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}

IBusServer *
ibus_server_listen (const gchar *address)
{
    g_assert (address != NULL);

    IBusServer *server;
    
    server = IBUS_SERVER (g_object_new (IBUS_TYPE_SERVER, NULL));

    ibus_server_listen_internal (server, address);

    return server;
}

static void
ibus_server_class_init (IBusServerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    _parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusServerPrivate));

    gobject_class->finalize = (GObjectFinalizeFunc) ibus_server_finalize;

    klass->new_connection = ibus_server_new_connection;

    _signals[NEW_CONNECTION] =
        g_signal_new (I_("new-connection"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusServerClass, new_connection),
            NULL, NULL,
            ibus_marshal_VOID__OBJECT,
            G_TYPE_NONE, 1,
            G_TYPE_OBJECT);
}

static void
ibus_server_init (IBusServer *server)
{
    IBusServerPrivate *priv;
    priv = IBUS_SERVER_GET_PRIVATE (server);
    priv->server = NULL;
}

static void
ibus_server_finalize (IBusServer *server)
{
    IBusServerPrivate *priv;
    priv = IBUS_SERVER_GET_PRIVATE (server);

    if (priv->server) {
        dbus_server_unref (priv->server);
    }

    G_OBJECT_CLASS(_parent_class)->finalize (G_OBJECT (server));
}

static void
ibus_server_new_connection (IBusServer      *server,
                            IBusConnection  *connection)
{
}

static void
_new_connection_cb (DBusServer      *dbus_server,
                    DBusConnection  *new_connection,
                    IBusServer      *server)
{
    IBusConnection *connection;
    connection = ibus_connection_new (new_connection, FALSE);
    g_signal_emit (server, _signals[NEW_CONNECTION], 0, connection);
    
    g_object_unref (connection);
}

static void
ibus_server_listen_internal (IBusServer     *server, 
                             const gchar    *address)
{
    g_assert (IBUS_IS_SERVER (server));
    g_assert (address != NULL);
    
    IBusServerPrivate *priv;
    DBusError error;

    priv = IBUS_SERVER_GET_PRIVATE (server);

    g_assert (priv->server == NULL);
    
    dbus_error_init (&error);
    priv->server = dbus_server_listen (address, &error);

    if (priv->server == NULL) {
        g_error ("Can not listen on '%s':\n"
                 "  %s:%s", 
                 address, error.name, error.message);
    }

    dbus_setup_server (priv->server);

    dbus_server_set_new_connection_function (priv->server,
                (DBusNewConnectionFunction) _new_connection_cb,
                server, NULL);
}

