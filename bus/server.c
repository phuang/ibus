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

#define BUS_SERVER_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_SERVER, BusServerPrivate))
#define DECLARE_PRIV BusServerPrivate *priv = BUS_SERVER_GET_PRIVATE(server)

enum {
    NEW_CONNECTION,
    LAST_SIGNAL,
};


/* BusServerPriv */
struct _BusServerPrivate {
    GHashTable *unique_names;
    GHashTable *names;
    gint id;
};
typedef struct _BusServerPrivate BusServerPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_server_class_init  (BusServerClass      *klass);
static void     bus_server_init        (BusServer           *server);
static void     bus_server_dispose     (BusServer           *server);
static void     bus_server_new_connection
                                        (BusServer          *server,
                                         BusConnection     *connection);

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
        type = g_type_register_static (IBUS_TYPE_SERVER,
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
    server = BUS_SERVER (g_object_new (BUS_TYPE_SERVER,
                    "connection-type", BUS_TYPE_CONNECTION,
                    NULL));
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

    priv->unique_names = g_hash_table_new (g_str_hash, g_str_equal);
    priv->names = g_hash_table_new (g_str_hash, g_str_equal);
    priv->id = 1;
}


static gboolean
_dbus_no_implement (BusServer      *server,
                    DBusMessage    *message,
                    BusConnection  *connection)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);
    
    DBusMessage *reply_message;

    reply_message = dbus_message_new_error_printf (message,
                                    dbus_message_get_member (message),
                                    "IBus does not support it.");
    
    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);

    return TRUE;
}


static gboolean
_dbus_hello (BusServer      *server,
             DBusMessage    *message,
             BusConnection  *connection)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);

    DBusMessage *reply_message;

    if (bus_connection_get_unique_name (connection) != NULL) {
        reply_message = dbus_message_new_error_printf (message,
                                    "Hello",
                                    "Already handled a Hello message");
    }
    else {
        gchar *name = g_strdup_printf (":1.%d", priv->id ++);
        bus_connection_set_unique_name (connection, name);
        
        reply_message = dbus_message_new_method_return (message);
        dbus_message_append_args (reply_message,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_INVALID);
        g_free (name);
    }

    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);

    return TRUE;
}

static gboolean
_dbus_list_names (BusServer      *server,
                  DBusMessage    *message,
                  BusConnection  *connection)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);

    DBusMessage *reply_message;
    DBusMessageIter iter, sub_iter;
    GList *name, *names;

    reply_message = dbus_message_new_method_return (message);

    dbus_message_iter_init_append (message, &iter);
    dbus_message_iter_open_container (&iter, DBUS_TYPE_ARRAY, "s", &sub_iter);
    
    // append unique names
    names = g_hash_table_get_keys (priv->unique_names);
    names = g_list_sort (names, (GCompareFunc) g_strcmp0);
    for (name = names; name != NULL; name = name->next) {
        dbus_message_iter_append_basic (&sub_iter, DBUS_TYPE_STRING, &(name->data));
    }
    g_list_free (names);
    
    // append well-known names
    names = g_hash_table_get_keys (priv->names);
    names = g_list_sort (names, (GCompareFunc) g_strcmp0);
    for (name = names; name != NULL; name = name->next) {
        dbus_message_iter_append_basic (&sub_iter, DBUS_TYPE_STRING, &(name->data));
    }
    g_list_free (names);

    dbus_message_iter_close_container (&iter, &sub_iter);
    
    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);

    return TRUE;
}

static gboolean
_dbus_name_has_owner (BusServer      *server,
                      DBusMessage    *message,
                      BusConnection  *connection)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);
    
    gchar *name;
    gboolean retval;
    gboolean has_owner;
    DBusMessage *reply_message;
    DBusError error;

    dbus_error_init (&error);
    retval = dbus_message_get_args (message, &error,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_INVALID);

    if (! retval) {
        reply_message = dbus_message_new_error_printf (message,
                                    "NameHasOwner",
                                    "%d", error.message);
        dbus_error_free (&error);
    }
    else {
        if (name[0] == ':') {
            has_owner = g_hash_table_lookup (priv->unique_names, name) != NULL;
        }
        else {
            has_owner = g_hash_table_lookup (priv->names, name) != NULL;
        }
        reply_message = dbus_message_new_method_return (message);
        dbus_message_append_args (reply_message,
                    DBUS_TYPE_BOOLEAN, &has_owner,
                    DBUS_TYPE_INVALID);
    }

    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);

    return TRUE;
}


static gboolean
_dbus_get_name_owner (BusServer      *server,
                      DBusMessage    *message,
                      BusConnection  *connection)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);
    
    gchar *name;
    gboolean retval;
    const gchar *owner_name;
    DBusMessage *reply_message;
    DBusError error;

    dbus_error_init (&error);
    retval = dbus_message_get_args (message, &error,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_INVALID);

    if (! retval) {
        reply_message = dbus_message_new_error_printf (message,
                                    "NameHasOwner",
                                    "%d", error.message);
        dbus_error_free (&error);
    }
    else {
        BusConnection *owner;
        if (name[0] == ':') {
            owner = BUS_CONNECTION (g_hash_table_lookup (priv->unique_names, name));
        }
        else {
            owner = BUS_CONNECTION (g_hash_table_lookup (priv->names, name));
        }

        if (owner != NULL) {
            owner_name = bus_connection_get_unique_name (owner);
            reply_message = dbus_message_new_method_return (message);
            dbus_message_append_args (reply_message,
                    DBUS_TYPE_STRING, &owner_name,
                    DBUS_TYPE_INVALID);
        }
        else {
            reply_message = dbus_message_new_error_printf (message,
                                    "GetNameOwner",
                                    "org.freedesktop.DBus.Error.NameHasNoOwner");
        }
    }

    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);

    return TRUE;
}

static gboolean
_dbus_get_id (BusServer      *server,
              DBusMessage    *message,
              BusConnection  *connection)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);

    DBusMessage *reply_message;
    const gchar *name;

    name = bus_connection_get_unique_name (connection);

    if (name == NULL) {
        reply_message = dbus_message_new_error_printf (message,
                                    "GetId",
                                    "Can not GetId before Hello");
    }
    else {
        reply_message = dbus_message_new_method_return (message);
        dbus_message_append_args (reply_message,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_INVALID);
    }

    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);

    return TRUE;
}

static gboolean
_dbus_add_match (BusServer      *server,
                 DBusMessage    *message,
                 BusConnection  *connection)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);

    DBusMessage *reply_message;
    DBusError error;
    gchar *rule;

    dbus_error_init (&error);
    if (!dbus_message_get_args (message, &error,
            DBUS_TYPE_STRING, &rule,
            DBUS_TYPE_INVALID)) {
        reply_message = dbus_message_new_error_printf (message,
                                    "AddMatch",
                                    "%s", error.message);
        dbus_error_free (&error);
    }
    else {
        reply_message = dbus_message_new_method_return (message);
    }

    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);
    return TRUE;
}

static gboolean
_dbus_remove_match (BusServer      *server,
                    DBusMessage    *message,
                    BusConnection  *connection)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);

    DBusMessage *reply_message;
    DBusError error;
    gchar *rule;

    dbus_error_init (&error);
    if (!dbus_message_get_args (message, &error,
            DBUS_TYPE_STRING, &rule,
            DBUS_TYPE_INVALID)) {
        reply_message = dbus_message_new_error_printf (message,
                                    "RemoveMatch",
                                    "%s", error.message);
        dbus_error_free (&error);
    }
    else {
        reply_message = dbus_message_new_method_return (message);
    }

    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);
    return TRUE;
}

static gboolean
_dbus_request_name (BusServer      *server,
                    DBusMessage    *message,
                    BusConnection  *connection)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);

    DBusMessage *reply_message;
    DBusError error;
    gchar *name;
    guint flags;
    guint retval;

    dbus_error_init (&error);
    if (!dbus_message_get_args (message, &error,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID)) {
        reply_message = dbus_message_new_error_printf (message,
                                    "RequestName",
                                    "%s", error.message);
        dbus_error_free (&error);
    }
    else {
        if (g_hash_table_lookup (priv->names, name)) {
            reply_message = dbus_message_new_error_printf (message,
                        "RequestName",
                        "Name %s has owner", name);
        }
        else {
            retval = 1;
            g_hash_table_insert (priv->names,
                    (gpointer )bus_connection_add_name (connection, name),
                    connection);
            reply_message = dbus_message_new_method_return (message);
            dbus_message_append_args (message,
                    DBUS_TYPE_UINT32, &retval,
                    DBUS_TYPE_INVALID);
        }
    }

    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);
    return TRUE;
}

static gboolean
_dbus_release_name (BusServer      *server,
                    DBusMessage    *message,
                    BusConnection  *connection)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);

    DBusMessage *reply_message;
    DBusError error;
    gchar *name;
    guint retval;

    dbus_error_init (&error);
    if (!dbus_message_get_args (message, &error,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_INVALID)) {
        reply_message = dbus_message_new_error_printf (message,
                                    "ReleaseName",
                                    "%s", error.message);
        dbus_error_free (&error);
    }
    else {
        reply_message = dbus_message_new_method_return (message);
        if (bus_connection_remove_name (connection, name)) {
            retval = 1;
        }
        else {
            retval = 2;
        }
        dbus_message_append_args (message,
                    DBUS_TYPE_UINT32, &retval,
                    DBUS_TYPE_INVALID);
    }

    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);
    return TRUE;
}

static gboolean
_connection_dbus_message_cb     (BusConnection  *connection,
                                 DBusMessage    *message,
                                 BusServer      *server)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (BUS_IS_SERVER (server));
    g_assert (message != NULL);

    gint i;
    gboolean retval = FALSE;

    struct {
        const gchar *interface;
        const gchar *name;
        gboolean (* handler) (BusServer *, DBusMessage *, BusConnection *);
    } handlers[] =  {
        /* dbus interface */
        { DBUS_INTERFACE_DBUS, "Hello",     _dbus_hello },
        { DBUS_INTERFACE_DBUS, "ListNames", _dbus_list_names },
        { DBUS_INTERFACE_DBUS, "ListActivatableNames", 
                                            _dbus_no_implement },
        { DBUS_INTERFACE_DBUS, "NameHasOwner",
                                            _dbus_name_has_owner },
        { DBUS_INTERFACE_DBUS, "StartServiceByName",
                                            _dbus_no_implement },
        { DBUS_INTERFACE_DBUS, "GetNameOwner",
                                            _dbus_get_name_owner },
        { DBUS_INTERFACE_DBUS, "GetConnectionUnixUser",
                                            _dbus_no_implement },
        { DBUS_INTERFACE_DBUS, "AddMatch",  _dbus_add_match },
        { DBUS_INTERFACE_DBUS, "RemoveMatch",
                                            _dbus_remove_match },
        { DBUS_INTERFACE_DBUS, "GetId",     _dbus_get_id },
        { DBUS_INTERFACE_DBUS, "RequestName", _dbus_request_name },
        { DBUS_INTERFACE_DBUS, "ReleaseName", _dbus_release_name },
        /* ibus interface */
        { IBUS_INTERFACE_IBUS, "GetAddress",          _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "CreateInputContext",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "ReleaseInputContext", _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "ProcessKeyEvent",     _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "SetCursorLocation",   _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "FocusIn",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "FocusOut",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "Reset",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "GetIsEnabled",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "SetCapabilites",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterFactories",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "UnregisterFactories",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "GetFactoryInfo",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "SetFactory",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "GetInputContextStates",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterListEngines",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterReloadEngines",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterStartEngine",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterRestartEngine",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterStopEngine",  _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "Kill",  _ibus_get_address },
 
        {NULL, NULL, NULL},
    };

    while (handlers[i].interface != NULL ) {
        if (dbus_message_is_method_call (message,
                                         handlers[i].interface,
                                         handlers[i].name)) {
            retval = handlers[i].handler (server, message, connection);
            g_signal_stop_emission_by_name (connection, "dbus-message");
            return retval;
        }

    }
    return retval;
}

static void
_connection_destroy_cb (BusConnection   *connection,
                        BusServer       *server)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (BUS_IS_SERVER (server));

    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);

    const gchar *unique_name = bus_connection_get_unique_name (connection);
    if (unique_name != NULL)
        g_hash_table_remove (priv->unique_names, unique_name);

    const GSList *name = bus_connection_get_names (connection);

    while (name != NULL) {
        g_hash_table_remove (priv->names, name->data);
        name = name->next;
    }
}


static void
bus_server_new_connection   (BusServer          *server,
                             BusConnection      *connection)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (BUS_IS_SERVER (server));

    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);
    
    g_signal_connect (connection, "dbus_message",
                      (GCallback) _connection_dbus_message_cb,
                      server);
    g_signal_connect (connection, "destroy",
                      (GCallback) _connection_destroy_cb,
                      server);
    g_object_ref (connection);
}

static void
bus_server_dispose (BusServer *server)
{
    BusServerPrivate *priv;
    priv = BUS_SERVER_GET_PRIVATE (server);
    
    G_OBJECT_CLASS (_parent_class)->dispose (G_OBJECT (server));
    
    g_hash_table_unref (priv->unique_names);
    g_hash_table_unref (priv->names);
}
