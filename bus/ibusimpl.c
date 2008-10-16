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

#include "ibusimpl.h"
#include "connection.h"

#define BUS_IBUS_IMPL_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_IBUS_IMPL, BusIBusImplPrivate))

enum {
    LAST_SIGNAL,
};

enum {
    PROP_0,
};


/* IBusIBusImplPriv */
struct _BusIBusImplPrivate {
    GHashTable *unique_names;
    GHashTable *names;
    GSList *connections;
    gint id;
};

typedef struct _BusIBusImplPrivate BusIBusImplPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_ibus_impl_class_init      (BusIBusImplClass    *klass);
static void     bus_ibus_impl_init            (BusIBusImpl         *ibus_impl);
static void     bus_ibus_impl_dispose         (BusIBusImpl         *ibus_impl);
static gboolean bus_ibus_impl_dbus_message    (BusIBusImpl         *ibus_impl,
                                               BusConnection       *connection,
                                               DBusMessage         *message);

static IBusServiceClass  *_parent_class = NULL;

GType
bus_ibus_impl_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusIBusImplClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_ibus_impl_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusIBusImpl),
        0,
        (GInstanceInitFunc) bus_ibus_impl_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERVICE,
                    "BusIBusImpl",
                    &type_info,
                    (GTypeFlags) 0);
    }
    return type;
}

BusIBusImpl *
bus_ibus_impl_new (void)
{
    // BusIBusImplPrivate *priv;
    BusIBusImpl *ibus_impl;

    ibus_impl = BUS_IBUS_IMPL (g_object_new (BUS_TYPE_IBUS_IMPL,
                    "path", IBUS_PATH_IBUS,
                    NULL));

    return ibus_impl;
}

static void
bus_ibus_impl_class_init (BusIBusImplClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    _parent_class = (IBusServiceClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusIBusImplPrivate));

    gobject_class->dispose = (GObjectFinalizeFunc) bus_ibus_impl_dispose;

    IBUS_SERVICE_CLASS (klass)->dbus_message = (ServiceDBusMessageFunc) bus_ibus_impl_dbus_message;

}

static void
bus_ibus_impl_init (BusIBusImpl *ibus_impl)
{
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus_impl);

    priv->unique_names = g_hash_table_new (g_str_hash, g_str_equal);
    priv->names = g_hash_table_new (g_str_hash, g_str_equal);
    priv->connections = NULL;
    priv->id = 1;

}

static void
bus_ibus_impl_dispose (BusIBusImpl *ibus_impl)
{
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus_impl);

    G_OBJECT_CLASS(_parent_class)->dispose (G_OBJECT (ibus_impl));
}

/* introspectable interface */
static DBusMessage *
_ibus_introspect (BusIBusImpl     *ibus_impl,
                  DBusMessage     *message,
                  BusConnection   *connection)
{
    static const gchar *introspect =
        "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
        "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
        "<node>\n"
        "  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
        "    <method name=\"Introspect\">\n"
        "      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "  </interface>\n"
        "  <interface name=\"org.freedesktop.DBus\">\n"
        "    <method name=\"RequestName\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"in\" type=\"u\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <signal name=\"NameOwnerChanged\">\n"
        "      <arg type=\"s\"/>\n"
        "      <arg type=\"s\"/>\n"
        "      <arg type=\"s\"/>\n"
        "    </signal>\n"
        "  </interface>\n"
        "</node>\n";

    DBusMessage *reply_message;
    reply_message = dbus_message_new_method_return (message);
    dbus_message_append_args (reply_message,
            DBUS_TYPE_STRING, &introspect,
            DBUS_TYPE_INVALID);

    return reply_message;
}

static gboolean
bus_ibus_impl_dbus_message (BusIBusImpl *ibus_impl, BusConnection *connection, DBusMessage *message)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus_impl));
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    gint i;
    DBusMessage *reply_message = NULL;

    struct {
        const gchar *interface;
        const gchar *name;
        DBusMessage *(* handler) (BusIBusImpl *, DBusMessage *, BusConnection *);
    } handlers[] =  {
        /* Introspectable interface */
        { DBUS_INTERFACE_INTROSPECTABLE,
                               "Introspect", _ibus_introspect },
#if 0
        /* IBus interface */
        { IBUS_INTERFACE_IBUS, "GetAddress",            _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "CreateInputContext",    _ibus_create_input_context },

        { IBUS_INTERFACE_IBUS, "RegisterFactories",     _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "UnregisterFactories",   _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "GetFactoryInfo",        _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "SetFactory",            _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "GetInputContextStates", _ibus_get_address },

        { IBUS_INTERFACE_IBUS, "RegisterListEngines",   _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterReloadEngines", _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterStartEngine",   _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterRestartEngine", _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterStopEngine",    _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "Kill",                  _ibus_get_address },
#endif
        { NULL, NULL, NULL }
    };

    dbus_message_set_sender (message, bus_connection_get_unique_name (connection));
    dbus_message_set_destination (message, DBUS_SERVICE_DBUS);

    for (i = 0; handlers[i].interface != NULL; i++) {
        if (dbus_message_is_method_call (message,
                                         handlers[i].interface,
                                         handlers[i].name)) {

            reply_message = handlers[i].handler (ibus_impl, message, connection);
            if (reply_message) {

                dbus_message_set_sender (reply_message, DBUS_SERVICE_DBUS);
                dbus_message_set_destination (reply_message, bus_connection_get_unique_name (connection));
                dbus_message_set_no_reply (reply_message, TRUE);

                ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
                dbus_message_unref (reply_message);
            }

            g_signal_stop_emission_by_name (ibus_impl, "dbus-message");
            return TRUE;
        }
    }

    reply_message = dbus_message_new_error_printf (message,
                                "org.freedesktop.DBus.Error.NoImplement",
                                "%s is not implemented",
                                dbus_message_get_member (message));

    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);
    return FALSE;
}

static void
_connection_destroy_cb (BusConnection   *connection,
                        BusIBusImpl     *ibus_impl)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (BUS_IS_IBUS_IMPL (ibus_impl));

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus_impl);

    /*
    ibus_service_remove_from_connection (
                    IBUS_SERVICE (ibus_impl),
                    IBUS_CONNECTION (connection));
    */

    const gchar *unique_name = bus_connection_get_unique_name (connection);
    if (unique_name != NULL)
        g_hash_table_remove (priv->unique_names, unique_name);

    const GSList *name = bus_connection_get_names (connection);

    while (name != NULL) {
        g_hash_table_remove (priv->names, name->data);
        name = name->next;
    }

    priv->connections = g_slist_remove (priv->connections, connection);
    g_object_unref (connection);
}


gboolean
bus_ibus_impl_new_connection (BusIBusImpl    *ibus_impl,
                              BusConnection  *connection)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus_impl));
    g_assert (BUS_IS_CONNECTION (connection));

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus_impl);

    g_assert (g_slist_find (priv->connections, connection) == NULL);

    g_object_ref (G_OBJECT (connection));
    priv->connections = g_slist_append (priv->connections, connection);

    g_signal_connect (connection, "destroy",
                      (GCallback) _connection_destroy_cb,
                      ibus_impl);

    ibus_service_add_to_connection (
            IBUS_SERVICE (ibus_impl),
            IBUS_CONNECTION (connection));

    return TRUE;
}
