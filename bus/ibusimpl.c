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
#include "dbusimpl.h"
#include "server.h"
#include "connection.h"
#include "factoryproxy.h"
#include "panelproxy.h"
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
    GHashTable *factory_dict;
    GList *factory_list;
    GList *contexts;

    BusFactoryProxy *default_factory;
    BusInputContext *focused_context;
    BusPanelProxy   *panel;
};

typedef struct _BusIBusImplPrivate BusIBusImplPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_ibus_impl_class_init      (BusIBusImplClass     *klass);
static void     bus_ibus_impl_init            (BusIBusImpl          *ibus);
static void     bus_ibus_impl_destroy         (BusIBusImpl          *ibus);
static gboolean bus_ibus_impl_ibus_message    (BusIBusImpl          *ibus,
                                               BusConnection        *connection,
                                               IBusMessage          *message);
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

    IBUS_SERVICE_CLASS (klass)->ibus_message = (ServiceIBusMessageFunc) bus_ibus_impl_ibus_message;

}

static void
_panel_destroy_cb (BusPanelProxy *panel,
                   BusIBusImpl   *ibus)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    g_assert (priv->panel == panel);

    priv->panel = NULL;
    g_object_unref (panel);
}

static void
_dbus_name_owner_changed (BusDBusImpl *dbus,
                          const gchar *name,
                          const gchar *old_name,
                          const gchar *new_name,
                          BusIBusImpl *ibus)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (name != NULL);
    g_assert (old_name != NULL);
    g_assert (new_name != NULL);
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    if (g_strcmp0 (name, IBUS_SERVICE_PANEL) == 0 &&
        g_strcmp0 (new_name, "") != 0) {
        
        BusConnection *connection;

        if (priv->panel != NULL) {
            ibus_object_destroy (IBUS_OBJECT (priv->panel));
            priv->panel = NULL;
        }

        connection = bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS, new_name); 

        g_return_if_fail (connection != NULL);
        
        priv->panel = bus_panel_proxy_new (connection);

        g_signal_connect (priv->panel,
                          "destroy",
                          G_CALLBACK (_panel_destroy_cb),
                          ibus);

        if (priv->focused_context != NULL) {
            bus_panel_proxy_focus_in (priv->panel, priv->focused_context);
        }
    }
}

static void
bus_ibus_impl_init (BusIBusImpl *ibus)
{
    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    priv->factory_dict = g_hash_table_new (g_str_hash, g_str_equal);
    priv->factory_list = NULL;
    priv->contexts = NULL;
    priv->default_factory = NULL;
    priv->focused_context = NULL;
    priv->panel = NULL;

    g_signal_connect (BUS_DEFAULT_DBUS,
                      "name-owner-changed",
                      G_CALLBACK (_dbus_name_owner_changed),
                      ibus);
}

static void
bus_ibus_impl_destroy (BusIBusImpl *ibus)
{
    BusConnection *connection;
    GList *p;

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    bus_server_quit (BUS_DEFAULT_SERVER);

    IBUS_OBJECT_CLASS(_parent_class)->destroy (IBUS_OBJECT (ibus));
}

/* introspectable interface */
static IBusMessage *
_ibus_introspect (BusIBusImpl     *ibus,
                  IBusMessage     *message,
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

    IBusMessage *reply_message;
    reply_message = ibus_message_new_method_return (message);
    ibus_message_append_args (reply_message,
                              G_TYPE_STRING, &introspect,
                              G_TYPE_INVALID);

    return reply_message;
}



static IBusMessage *
_ibus_get_address (BusIBusImpl     *ibus,
                   IBusMessage     *message,
                   BusConnection   *connection)
{
    const gchar *address;
    IBusMessage *reply;

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    address = ibus_server_get_address (IBUS_SERVER (BUS_DEFAULT_SERVER));

    reply = ibus_message_new_method_return (message);
    ibus_message_append_args (message,
                              G_TYPE_STRING, &address,
                              G_TYPE_INVALID);

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

    priv->contexts = g_list_remove (priv->contexts, context);
    g_object_unref (context);
}

static void
_context_focus_in_cb (BusInputContext    *context,
                      BusIBusImpl        *ibus)
{    
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);
   
    if (priv->focused_context == context)
        return;

    if (priv->focused_context) {
        bus_input_context_focus_out (priv->focused_context);
        g_object_unref (priv->focused_context);
    }

    g_object_ref (context);
    priv->focused_context = context;
    
    if (priv->panel != NULL) {
        bus_panel_proxy_focus_in (priv->panel, priv->focused_context);
    }

}

static void
_context_focus_out_cb (BusInputContext    *context,
                       BusIBusImpl        *ibus)
{    
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);
    
    if (priv->focused_context != context)
        return;

    if (priv->panel != NULL) {
        bus_panel_proxy_focus_out (priv->panel, priv->focused_context);
    }

    if (priv->focused_context) {
        g_object_unref (priv->focused_context);
        priv->focused_context = NULL;
    }
}

static IBusMessage *
_ibus_create_input_context (BusIBusImpl     *ibus,
                            IBusMessage     *message,
                            BusConnection   *connection)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    gint i;
    gchar *client;
    IBusError *error;
    IBusMessage *reply;
    BusInputContext *context;
    const gchar *path;

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    if (!ibus_message_get_args (message, 
                                &error,
                                G_TYPE_STRING, &client,
                                G_TYPE_INVALID)) {
        reply = ibus_message_new_error (message,
                                        DBUS_ERROR_INVALID_ARGS,
                                        "Argument 1 of CreateInputContext should be an string");
        ibus_error_free (error);
        return reply;
    }

    context = bus_input_context_new (connection, client);
    priv->contexts = g_list_append (priv->contexts, context);

    static const struct {
        gchar *name;
        GCallback callback;
    } signals [] = {
        { "focus-in",   G_CALLBACK (_context_focus_in_cb) },
        { "focus-out",  G_CALLBACK (_context_focus_out_cb) },
        { "destroy",    G_CALLBACK (_context_destroy_cb) },
        { NULL, NULL }
    };

    for (i = 0; signals[i].name != NULL; i++) {
        g_signal_connect (context,
                          signals[i].name,
                          signals[i].callback,
                          ibus);
    }
    
    path = ibus_service_get_path (IBUS_SERVICE (context));
    reply = ibus_message_new_method_return (message);
    ibus_message_append_args (reply,
                              IBUS_TYPE_OBJECT_PATH, &path,
                              G_TYPE_INVALID);

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

    g_hash_table_remove (priv->factory_dict,
                         ibus_proxy_get_path (IBUS_PROXY (factory)));

    priv->factory_list = g_list_remove (priv->factory_list, factory);
    g_object_unref (factory);

    if (priv->default_factory == factory) {
        g_object_unref (priv->default_factory);
        priv->default_factory = NULL;
    }
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

static IBusMessage *
_ibus_register_factories (BusIBusImpl     *ibus,
                          IBusMessage     *message,
                          BusConnection   *connection)
{
    gint n;
    IBusMessageIter iter, sub_iter;
    IBusMessage *reply;
    gboolean retval;
    BusFactoryProxy *factory;

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    retval = ibus_message_iter_init (message, &iter);
    g_assert (retval);

    retval = ibus_message_iter_recurse (&iter, IBUS_TYPE_ARRAY, &sub_iter);
    g_assert (retval);

    while (1) {
        gchar *path;
        if (ibus_message_iter_get_arg_type (&sub_iter) != IBUS_TYPE_OBJECT_PATH)
            break;
        retval = ibus_message_iter_get (&sub_iter,
                                        IBUS_TYPE_OBJECT_PATH,
                                        &path);
        g_assert (retval);

        if (g_hash_table_lookup (priv->factory_dict, path) != NULL) {
            reply = ibus_message_new_error_printf (message,
                                                   DBUS_ERROR_FAILED,
                                                   "Factory %s has been registered!",
                                                   path);
            return reply;
        }
        
        factory = bus_factory_proxy_new (path, connection);
        g_hash_table_insert (priv->factory_dict,
                             (gpointer) ibus_proxy_get_path (IBUS_PROXY (factory)),
                             factory);
        priv->factory_list = g_list_append (priv->factory_list, factory);
        
        g_signal_connect (factory,
                          "destroy",
                          (GCallback) _factory_destroy_cb,
                          ibus);
    }

    reply = ibus_message_new_method_return (message);
    ibus_connection_send (IBUS_CONNECTION (connection), reply);
    ibus_connection_flush (IBUS_CONNECTION (connection));

    priv->factory_list = g_list_sort (priv->factory_list, (GCompareFunc) _factory_cmp);
    
    return NULL;
}

static IBusMessage *
_ibus_get_factories (BusIBusImpl     *ibus,
                     IBusMessage     *message,
                     BusConnection   *connection)
{
    IBusMessage *reply;
    IBusMessageIter iter, sub_iter, sub_sub_iter;
    GList *p;

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    reply = ibus_message_new_method_return (message);

    ibus_message_iter_init_append (reply, &iter);
    ibus_message_iter_open_container (&iter, IBUS_TYPE_ARRAY, "(os)", &sub_iter);

    for (p = priv->factory_list; p != NULL; p = p->next) {
        BusFactoryProxy *factory;
        IBusConnection *connection;
        const gchar *path;
        const gchar *unique_name;

        factory = BUS_FACTORY_PROXY (p->data);
        path = ibus_proxy_get_path (IBUS_PROXY (factory));
        connection = ibus_proxy_get_connection (IBUS_PROXY (factory));
        unique_name = bus_connection_get_unique_name ( BUS_CONNECTION (connection));
        ibus_message_iter_open_container (&sub_iter, IBUS_TYPE_STRUCT, "os", &sub_sub_iter);
        ibus_message_iter_append (&sub_sub_iter, IBUS_TYPE_OBJECT_PATH, &path);
        ibus_message_iter_append (&sub_sub_iter, G_TYPE_STRING, &unique_name);
        ibus_message_iter_close_container (&sub_iter, &sub_sub_iter);
    }
    ibus_message_iter_close_container (&iter, &sub_iter);
    return reply;
}

static IBusMessage *
_ibus_set_factory (BusIBusImpl      *ibus,
                   IBusMessage      *message,
                   BusConnection    *connection)
{
    return NULL;
}

static IBusMessage *
_ibus_kill (BusIBusImpl     *ibus,
            IBusMessage     *message,
            BusConnection   *connection)
{
    IBusMessage *reply;

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    reply = ibus_message_new_method_return (message);
    ibus_connection_send (IBUS_CONNECTION (connection), reply);
    ibus_connection_flush (IBUS_CONNECTION (connection));
    ibus_message_unref (reply);

    ibus_object_destroy (IBUS_OBJECT (ibus));
    return NULL;
}

static gboolean
bus_ibus_impl_ibus_message (BusIBusImpl     *ibus,
                            BusConnection   *connection,
                            IBusMessage     *message)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    gint i;
    IBusMessage *reply_message = NULL;

    static const struct {
        const gchar *interface;
        const gchar *name;
        IBusMessage *(* handler) (BusIBusImpl *, IBusMessage *, BusConnection *);
    } handlers[] =  {
        /* Introspectable interface */
        { DBUS_INTERFACE_INTROSPECTABLE,
                               "Introspect", _ibus_introspect },
        /* IBus interface */
        { IBUS_INTERFACE_IBUS, "GetAddress",            _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "CreateInputContext",    _ibus_create_input_context },
        { IBUS_INTERFACE_IBUS, "RegisterFactories",     _ibus_register_factories },
        { IBUS_INTERFACE_IBUS, "GetFactories",          _ibus_get_factories },
        { IBUS_INTERFACE_IBUS, "SetFactory",            _ibus_set_factory },
#if 0
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

    ibus_message_set_sender (message, bus_connection_get_unique_name (connection));
    ibus_message_set_destination (message, DBUS_SERVICE_DBUS);

    for (i = 0; handlers[i].interface != NULL; i++) {
        if (ibus_message_is_method_call (message,
                                         handlers[i].interface,
                                         handlers[i].name)) {

            reply_message = handlers[i].handler (ibus, message, connection);
            if (reply_message) {

                ibus_message_set_sender (reply_message, DBUS_SERVICE_DBUS);
                ibus_message_set_destination (reply_message, bus_connection_get_unique_name (connection));
                ibus_message_set_no_reply (reply_message, TRUE);

                ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
                ibus_message_unref (reply_message);
            }

            g_signal_stop_emission_by_name (ibus, "ibus-message");
            return TRUE;
        }
    }

    reply_message = ibus_message_new_error_printf (message,
                                                   DBUS_ERROR_UNKNOWN_METHOD,
                                                   "%s is not implemented",
                                                   ibus_message_get_member (message));

    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    ibus_message_unref (reply_message);
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
}


gboolean
bus_ibus_impl_new_connection (BusIBusImpl    *ibus,
                              BusConnection  *connection)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_CONNECTION (connection));

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    ibus_service_add_to_connection (
            IBUS_SERVICE (ibus),
            IBUS_CONNECTION (connection));

    return TRUE;
}

BusFactoryProxy *
bus_ibus_impl_get_default_factory (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    BusIBusImplPrivate *priv;
    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);

    if (priv->default_factory == NULL && priv->factory_list != NULL) {
        priv->default_factory = BUS_FACTORY_PROXY (priv->factory_list->data);
    }

    if (priv->default_factory == NULL) {
        /* TODO */
    }

    return priv->default_factory;
}

BusFactoryProxy *
bus_ibus_impl_get_next_factory (BusIBusImpl     *ibus,
                                BusFactoryProxy *factory)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_FACTORY_PROXY (factory) || factory == NULL);

    GList *link;
    BusIBusImplPrivate *priv;
    
    if (factory == NULL) {
        return bus_ibus_impl_get_default_factory (ibus);
    }

    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);
    link = g_list_find (priv->factory_list, factory);

    g_assert (link != NULL);

    link = link->next;

    if (link != NULL) {
        link = priv->factory_list;
    }
    
    return BUS_FACTORY_PROXY (link->data);
}

BusFactoryProxy *
bus_ibus_impl_get_previous_factory (BusIBusImpl     *ibus,
                                    BusFactoryProxy *factory)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_FACTORY_PROXY (factory) || factory == NULL);

    GList *link;
    BusIBusImplPrivate *priv;
    
    if (factory == NULL) {
        return bus_ibus_impl_get_default_factory (ibus);
    }

    priv = BUS_IBUS_IMPL_GET_PRIVATE (ibus);
    link = g_list_find (priv->factory_list, factory);

    g_assert (link != NULL);

    link = link->prev;

    if (link != NULL) {
        link = g_list_last (priv->factory_list);
    }
    
    return BUS_FACTORY_PROXY (link->data);
}

