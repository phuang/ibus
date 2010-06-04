/* vim:set et sts=4: */
/* ibus - The Input Bus
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

#include <dbus/dbus.h>
#include "ibusmainloop.h"
#include "ibusserver.h"
#include "ibusinternal.h"

#define IBUS_SERVER_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_SERVER, IBusServerPrivate))
#define DECLARE_PRIV IBusServerPrivate *priv = IBUS_SERVER_GET_PRIVATE(server)

enum {
    NEW_CONNECTION,
    LAST_SIGNAL,
};

enum {
    PROP_0,
    PROP_CONNECTION_TYPE,
};

/* IBusServerPriv */
struct _IBusServerPrivate {
    DBusServer *server;
    GType       connection_type;
};
typedef struct _IBusServerPrivate IBusServerPrivate;

static guint            server_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_server_destroy     (IBusServer         *server);
static void     ibus_server_set_property(IBusServer         *server,
                                         guint               prop_id,
                                         const GValue       *value,
                                         GParamSpec         *pspec);
static void     ibus_server_get_property(IBusServer         *server,
                                         guint               prop_id,
                                         GValue             *value,
                                         GParamSpec         *pspec);
static gboolean ibus_server_listen_internal
                                        (IBusServer         *server,
                                         const gchar        *address);
static void     ibus_server_new_connection
                                        (IBusServer         *server,
                                         IBusConnection     *connection);

G_DEFINE_TYPE (IBusServer, ibus_server, IBUS_TYPE_OBJECT)

IBusServer *
ibus_server_new (void)
{
    IBusServer *server;

    server = IBUS_SERVER (g_object_new (IBUS_TYPE_SERVER, NULL));
    return server;
}

gboolean
ibus_server_listen  (IBusServer  *server,
                     const gchar *address)
{
    g_assert (IBUS_IS_SERVER (server));
    g_assert (address != NULL);

    return ibus_server_listen_internal (server, address);
}

static void
ibus_server_class_init (IBusServerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (IBusServerPrivate));

    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_server_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_server_get_property;

    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_server_destroy;

    klass->new_connection = ibus_server_new_connection;

    /* install properties */
    /**
     * IBusServer:connection-type:
     *
     * The connection type of server object.
     */
    g_object_class_install_property (gobject_class,
                    PROP_CONNECTION_TYPE,
                    g_param_spec_gtype ("connection-type",
                        "connection type",
                        "The connection type of server object",
                        IBUS_TYPE_CONNECTION,
                        G_PARAM_READWRITE));

    /* install signals */
    /**
     * IBusServer::new-connection:
     * @server: An IBusServer.
     * @connection: The corresponding IBusConnection.
     *
     * Emitted when a new connection is coming in.
     * In this handler, IBus could add a reference and continue processing the connection.
     * If no reference is added, the new connection will be released and closed after this signal.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
     *
     * See also: IBusNewConnectionFunc().
     */
    server_signals[NEW_CONNECTION] =
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
    priv->connection_type = IBUS_TYPE_CONNECTION;
}

static void
ibus_server_destroy (IBusServer *server)
{
    IBusServerPrivate *priv;
    priv = IBUS_SERVER_GET_PRIVATE (server);

    if (priv->server) {
        dbus_server_unref (priv->server);
        priv->server = NULL;
    }

    IBUS_OBJECT_CLASS(ibus_server_parent_class)->destroy (IBUS_OBJECT (server));
}

static void
ibus_server_set_property    (IBusServer     *server,
                             guint           prop_id,
                             const GValue   *value,
                             GParamSpec     *pspec)
{
    IBusServerPrivate *priv;
    priv = IBUS_SERVER_GET_PRIVATE (server);

    switch (prop_id) {
    case PROP_CONNECTION_TYPE:
    {
        GType type;
        type = g_value_get_gtype (value);
        g_assert (g_type_is_a (type, IBUS_TYPE_CONNECTION));
        priv->connection_type = type;
        break;
    }
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (server, prop_id, pspec);
    }
}

static void
ibus_server_get_property    (IBusServer     *server,
                             guint           prop_id,
                             GValue         *value,
                             GParamSpec     *pspec)
{
    IBusServerPrivate *priv;
    priv = IBUS_SERVER_GET_PRIVATE (server);

    switch (prop_id) {
    case PROP_CONNECTION_TYPE:
        g_value_set_gtype (value, priv->connection_type);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (server, prop_id, pspec);
    }
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
    IBusServerPrivate *priv;
    IBusConnection *connection;

    priv = IBUS_SERVER_GET_PRIVATE (server);
    connection = IBUS_CONNECTION (g_object_new (priv->connection_type, NULL));
    ibus_connection_set_connection (connection, new_connection, FALSE);

    g_signal_emit (server, server_signals[NEW_CONNECTION], 0, connection);

    if (g_object_is_floating (connection)) {
        /* release connection if it is still floating */
        g_object_unref (connection);
    }
}

static gboolean
ibus_server_listen_internal (IBusServer  *server,
                             const gchar *address)
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
        g_warning ("Can not listen on '%s':\n"
                   "  %s:%s",
                   address, error.name, error.message);
        return FALSE;
    }

    dbus_server_set_new_connection_function (priv->server,
                (DBusNewConnectionFunction) _new_connection_cb,
                server, NULL);

    dbus_server_set_auth_mechanisms (priv->server, NULL);

    ibus_dbus_server_setup (priv->server);
    return TRUE;
}

void
ibus_server_disconnect (IBusServer *server)
{
    g_assert (IBUS_IS_SERVER (server));

    IBusServerPrivate *priv;
    priv = IBUS_SERVER_GET_PRIVATE (server);

    g_assert (priv->server != NULL);
    dbus_server_disconnect (priv->server);
}

const gchar *
ibus_server_get_address (IBusServer *server)
{
    g_assert (IBUS_IS_SERVER (server));

    gchar *address, *tmp;
    IBusServerPrivate *priv;
    priv = IBUS_SERVER_GET_PRIVATE (server);

    g_assert (priv->server != NULL);

    tmp = dbus_server_get_address (priv->server);
    address = g_strdup (tmp);
    dbus_free (tmp);
    return address;
}

const gchar *
ibus_server_get_id (IBusServer *server)
{
    g_assert (IBUS_IS_SERVER (server));

    gchar *id, *tmp;
    IBusServerPrivate *priv;
    priv = IBUS_SERVER_GET_PRIVATE (server);

    g_assert (priv->server != NULL);

    tmp = dbus_server_get_id (priv->server);
    id = g_strdup (tmp);
    dbus_free (tmp);
    return id;
}

gboolean
ibus_server_is_connected (IBusServer *server)
{
    g_assert (IBUS_IS_SERVER (server));

    IBusServerPrivate *priv;
    priv = IBUS_SERVER_GET_PRIVATE (server);

    g_assert (priv->server != NULL);

    return dbus_server_get_is_connected (priv->server);
}


gboolean
ibus_server_set_auth_mechanisms (IBusServer   *server,
                                 const gchar **mechanisms)
{
    g_assert (IBUS_IS_SERVER (server));

    IBusServerPrivate *priv;
    priv = IBUS_SERVER_GET_PRIVATE (server);

    g_assert (priv->server != NULL);

    return dbus_server_set_auth_mechanisms (priv->server, mechanisms);
}

