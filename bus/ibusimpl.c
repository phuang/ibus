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
#include "server.h"
#include "connection.h"
#include "factoryproxy.h"
#include "inputcontext.h"

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
    GSList *factories;
    GSList *contexts;
    gint id;

    BusFactoryProxy *default_factory;
};

typedef struct _BusIBusImplPrivate BusIBusImplPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_ibus_impl_class_init      (BusIBusImplClass     *klass);
static void     bus_ibus_impl_init            (BusIBusImpl          *ibus);
static void     bus_ibus_impl_destroy         (BusIBusImpl          *ibus);
static gboolean bus_ibus_impl_dbus_message    (BusIBusImpl          *ibus,
                                               BusConnection        *connection,
                                               DBusMessage          *message);
static void     _connection_destroy_cb        (BusConnection        *connection,
                                               BusIBusImpl          *ibus);

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
bus_ibus_impl_get_default (void)
{
    // BusIBusImplPrivate *priv;
    static BusIBusImpl *ibus = NULL;

    if (ibus == NULL) {
        ibus = BUS_IBUS_IMPL (g_object_new (BUS_TYPE_IBUS_IMPL,
                    "path", IBUS_PATH_IBUS,
                    NULL));
    }
    return ibus;
}

static void
bus_ibus_impl_class_init (BusIBusImplClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    _parent_class = (IBusServiceClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusIBusImplPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_ibus_impl_destroy;

    IBUS_SERVICE_CLASS (klass)->dbus_message = (ServiceDBusMessageFunc) bus_ibus_impl_dbus_message;

}

static void
bus_ibus_impl_init (BusIBusImpl *ibus)
{
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    priv->unique_names = g_hash_table_new (g_str_hash, g_str_equal);
    priv->names = g_hash_table_new (g_str_hash, g_str_equal);
    priv->connections = NULL;
    priv->factories = NULL;
    priv->contexts = NULL;
    priv->default_factory = NULL;
    priv->id = 1;
}

static void
bus_ibus_impl_destroy (BusIBusImpl *ibus)
{
    BusConnection *connection;
    GSList *p;
    
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    for (p = priv->connections; p != NULL; p = p->next) {
        connection = BUS_CONNECTION (p->data);
        g_signal_handlers_disconnect_by_func (connection, 
                                              (GCallback) _connection_destroy_cb,
                                              ibus);
        ibus_object_destroy (IBUS_OBJECT (connection));
        g_object_unref (connection);
    }

    g_slist_free (priv->connections);
    priv->connections = NULL;
    
    bus_server_quit (BUS_DEFAULT_SERVER);
    
    IBUS_OBJECT_CLASS(_parent_class)->destroy (IBUS_OBJECT (ibus));
}

/* introspectable interface */
static DBusMessage *
_ibus_introspect (BusIBusImpl     *ibus,
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



static DBusMessage *
_ibus_get_address (BusIBusImpl     *ibus,
                   DBusMessage     *message,
                   BusConnection   *connection)
{
    const gchar *address;
    DBusMessage *reply;
    
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);
    
    address = ibus_server_get_address (BUS_DEFAULT_SERVER);

    reply = dbus_message_new_method_return (message);
    dbus_message_append_args (message,
                              DBUS_TYPE_STRING, &address,
                              DBUS_TYPE_INVALID);

    return reply;
}

static void
_context_destroy_cb (BusInputContext    *context,
                     BusIBusImpl        *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    priv->contexts = g_slist_remove (priv->contexts, context);

    g_object_unref (context);
}

static DBusMessage *
_ibus_create_input_context (BusIBusImpl     *ibus,
                            DBusMessage     *message,
                            BusConnection   *connection)
{
    gchar *client;
    DBusError error;
    DBusMessage *reply;
    BusInputContext *context;
    const gchar *path;
    
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);
    
    dbus_error_init (&error);
    if (!dbus_message_get_args (message, &error,
                        DBUS_TYPE_STRING, &client,
                        DBUS_TYPE_INVALID)) {
        reply = dbus_message_new_error (message,
                                error.name,
                                "RegisterFactories shoule pass an object_path array as arugment");
        dbus_error_free (&error);
        return reply;;
    }

    context = bus_input_context_new (connection, client);
    priv->contexts = g_slist_append (priv->contexts, context);
    g_signal_connect (context,
                      "destroy",
                      (GCallback) _context_destroy_cb,
                      ibus);
    
    path = ibus_service_get_path (IBUS_SERVICE (context));
    reply = dbus_message_new_method_return (message);
    dbus_message_append_args (message,
                              DBUS_TYPE_OBJECT_PATH, &path,
                              DBUS_TYPE_INVALID);

    return reply;
}

static void
_factory_destroy_cb (BusFactoryProxy    *factory,
                     BusIBusImpl        *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_FACTORY_PROXY (factory));
    
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    priv->factories = g_slist_remove (priv->factories, factory);

    if (priv->default_factory == factory) {
        g_object_unref (priv->default_factory);
        priv->default_factory = NULL;
    }

    g_object_unref (factory);
}

static int
_factory_cmp (BusFactoryProxy   *a,
              BusFactoryProxy   *b)
{
    g_assert (BUS_IS_FACTORY_PROXY (a));
    g_assert (BUS_IS_FACTORY_PROXY (b));
    
    gint retval;

    retval = g_strcmp0 (bus_factory_proxy_get_lang (a), bus_factory_proxy_get_lang (b));
    if (retval != 0)
        return retval;
    retval = g_strcmp0 (bus_factory_proxy_get_name (a), bus_factory_proxy_get_name (b));
    return retval;
}

static DBusMessage *
_ibus_register_factories (BusIBusImpl     *ibus,
                          DBusMessage     *message,
                          BusConnection   *connection)
{
    gchar **paths;
    gint n;
    DBusError error; 
    DBusMessage *reply;
    
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    dbus_error_init (&error);
    if (!dbus_message_get_args (message, &error,
                        DBUS_TYPE_ARRAY, DBUS_TYPE_OBJECT_PATH,
                        &paths, &n,
                        DBUS_TYPE_INVALID)) {
        reply = dbus_message_new_error (message,
                                error.name,
                                "RegisterFactories shoule pass an object_path array as arugment");
        dbus_error_free (&error);
        return reply;;
    }

    reply = dbus_message_new_method_return (message);
    ibus_connection_send (IBUS_CONNECTION (connection), reply);
    ibus_connection_flush (IBUS_CONNECTION (connection));
    dbus_message_unref (reply);

    gint i;
    for (i = 0; i < n; i++ ) {
        BusFactoryProxy *factory;
        factory = bus_factory_proxy_new (paths[i], connection);
        priv->factories = g_slist_append (priv->factories, factory);
        g_signal_connect (factory,
                          "destroy",
                          (GCallback) _factory_destroy_cb,
                          ibus);
    }

    if (i > 0) {
        priv->factories = g_slist_sort (priv->factories, (GCompareFunc) _factory_cmp);
    }

    return NULL;
}

static DBusMessage *
_ibus_get_factories (BusIBusImpl     *ibus,
                     DBusMessage     *message,
                     BusConnection   *connection)
{
    DBusMessage *reply;
    DBusMessageIter iter, sub_iter, sub_sub_iter;
    GSList *p;
    
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    reply = dbus_message_new_method_return (message);

    dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_open_container (&iter, DBUS_TYPE_ARRAY, "(os)", &sub_iter);
    
    for (p = priv->factories; p != NULL; p = p->next) {
        BusFactoryProxy *factory;
        IBusConnection *connection;
        const gchar *path;
        const gchar *unique_name;

        factory = BUS_FACTORY_PROXY (p->data);
        path = ibus_proxy_get_path (IBUS_PROXY (factory));
        connection = ibus_proxy_get_connection (IBUS_PROXY (factory));
        unique_name = bus_connection_get_unique_name ( BUS_CONNECTION (connection));
        dbus_message_iter_open_container (&sub_iter, DBUS_TYPE_STRUCT, "os", &sub_sub_iter);
        dbus_message_iter_append_basic (&sub_sub_iter, DBUS_TYPE_OBJECT_PATH, &path);
        dbus_message_iter_append_basic (&sub_sub_iter, DBUS_TYPE_STRING, &unique_name);
        dbus_message_iter_close_container (&sub_iter, &sub_sub_iter);
    }
    dbus_message_iter_close_container (&iter, &sub_iter);
    return reply;
}

static DBusMessage *
_ibus_kill (BusIBusImpl     *ibus,
            DBusMessage     *message,
            BusConnection   *connection)
{
    DBusMessage *reply;
    
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    reply = dbus_message_new_method_return (message);
    ibus_connection_send (IBUS_CONNECTION (connection), reply);
    ibus_connection_flush (IBUS_CONNECTION (connection));
    dbus_message_unref (reply);

    ibus_object_destroy (IBUS_OBJECT (ibus));
    return NULL;
}

static gboolean
bus_ibus_impl_dbus_message (BusIBusImpl     *ibus,
                            BusConnection   *connection,
                            DBusMessage     *message)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
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
        /* IBus interface */
        { IBUS_INTERFACE_IBUS, "GetAddress",            _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "CreateInputContext",    _ibus_create_input_context },
        { IBUS_INTERFACE_IBUS, "RegisterFactories",     _ibus_register_factories },
        { IBUS_INTERFACE_IBUS, "GetFactories",          _ibus_get_factories },
#if 0
        { IBUS_INTERFACE_IBUS, "SetFactory",            _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "GetInputContextStates", _ibus_get_address },

        { IBUS_INTERFACE_IBUS, "RegisterListEngines",   _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterReloadEngines", _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterStartEngine",   _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterRestartEngine", _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "RegisterStopEngine",    _ibus_get_address },
#endif
        { IBUS_INTERFACE_IBUS, "Kill",                  _ibus_kill },
        { NULL, NULL, NULL }
    };

    dbus_message_set_sender (message, bus_connection_get_unique_name (connection));
    dbus_message_set_destination (message, DBUS_SERVICE_DBUS);

    for (i = 0; handlers[i].interface != NULL; i++) {
        if (dbus_message_is_method_call (message,
                                         handlers[i].interface,
                                         handlers[i].name)) {

            reply_message = handlers[i].handler (ibus, message, connection);
            if (reply_message) {

                dbus_message_set_sender (reply_message, DBUS_SERVICE_DBUS);
                dbus_message_set_destination (reply_message, bus_connection_get_unique_name (connection));
                dbus_message_set_no_reply (reply_message, TRUE);

                ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
                dbus_message_unref (reply_message);
            }

            g_signal_stop_emission_by_name (ibus, "dbus-message");
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
                        BusIBusImpl     *ibus)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    /*
    ibus_service_remove_from_connection (
                    IBUS_SERVICE (ibus),
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
bus_ibus_impl_new_connection (BusIBusImpl    *ibus,
                              BusConnection  *connection)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_CONNECTION (connection));

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    g_assert (g_slist_find (priv->connections, connection) == NULL);

    g_object_ref (G_OBJECT (connection));
    priv->connections = g_slist_append (priv->connections, connection);

    g_signal_connect (connection, "destroy",
                      (GCallback) _connection_destroy_cb,
                      ibus);

    ibus_service_add_to_connection (
            IBUS_SERVICE (ibus),
            IBUS_CONNECTION (connection));

    return TRUE;
}
