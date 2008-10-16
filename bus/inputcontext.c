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

#include "inputcontext.h"

#define BUS_INPUT_CONTEXT_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_INPUT_CONTEXT, BusInputContextPrivate))

enum {
    LAST_SIGNAL,
};

enum {
    PROP_0,
};


/* IBusInputContextPriv */
struct _BusInputContextPrivate {
    GHashTable *unique_names;
    GHashTable *names;
    GSList *connections;
    gint id;
};

typedef struct _BusInputContextPrivate BusInputContextPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_input_context_class_init    (BusInputContextClass   *klass);
static void     bus_input_context_init          (BusInputContext        *input_context);
static void     bus_input_context_dispose       (BusInputContext        *input_context);
static gboolean bus_input_context_dbus_message  (BusInputContext        *input_context,
                                                 BusConnection          *connection,
                                                 DBusMessage            *message);

static IBusServiceClass  *_parent_class = NULL;

GType
bus_input_context_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusInputContextClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_input_context_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusInputContext),
        0,
        (GInstanceInitFunc) bus_input_context_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERVICE,
                    "BusInputContext",
                    &type_info,
                    (GTypeFlags) 0);
    }
    return type;
}

BusInputContext *
bus_input_context_new (void)
{
    // BusInputContextPrivate *priv;
    BusInputContext *input_context;

    input_context = BUS_INPUT_CONTEXT (g_object_new (BUS_TYPE_INPUT_CONTEXT,
                    "path", IBUS_PATH_IBUS,
                    NULL));

    return input_context;
}

static void
bus_input_context_class_init (BusInputContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    _parent_class = (IBusServiceClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusInputContextPrivate));

    gobject_class->dispose = (GObjectFinalizeFunc) bus_input_context_dispose;

    IBUS_SERVICE_CLASS (klass)->dbus_message = (ServiceDBusMessageFunc) bus_input_context_dbus_message;

}

static void
bus_input_context_init (BusInputContext *input_context)
{
    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (input_context);

    priv->unique_names = g_hash_table_new (g_str_hash, g_str_equal);
    priv->names = g_hash_table_new (g_str_hash, g_str_equal);
    priv->connections = NULL;
    priv->id = 1;

}

static void
bus_input_context_dispose (BusInputContext *input_context)
{
    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (input_context);

    G_OBJECT_CLASS(_parent_class)->dispose (G_OBJECT (input_context));
}

/* introspectable interface */
static DBusMessage *
_ibus_introspect (BusInputContext   *input_context,
                  DBusMessage       *message,
                  BusConnection     *connection)
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
_ic_process_key_event (BusInputContext  *context,
                       DBusMessage      *message,
                       BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    DBusMessage *reply;
    guint32 keyval, state;
    gboolean is_press;
    gboolean retval;
    DBusError error;

    dbus_error_init (&error);
    retval = dbus_message_get_args (message, &error,
                DBUS_TYPE_UINT32, &keyval,
                DBUS_TYPE_BOOLEAN, &is_press,
                DBUS_TYPE_UINT32, &state,
                DBUS_TYPE_INVALID);

    if (!retval) {
        reply = dbus_message_new_error (message, error.name, error.message);
        dbus_error_free (&error);
        return reply;
    }
    
    /* TODO */

    retval = FALSE;
    reply = dbus_message_new_method_return (message);
    dbus_message_append_args (reply, DBUS_TYPE_BOOLEAN, &retval);

    return reply;
}

static DBusMessage *
_ic_set_cursor_location (BusInputContext  *context,
                         DBusMessage      *message,
                         BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    DBusMessage *reply;
    guint32 x, y, w, h;
    gboolean retval;
    DBusError error;

    dbus_error_init (&error);
    retval = dbus_message_get_args (message, &error,
                DBUS_TYPE_UINT32, &x,
                DBUS_TYPE_UINT32, &y,
                DBUS_TYPE_UINT32, &w,
                DBUS_TYPE_UINT32, &h,
                DBUS_TYPE_INVALID);

    if (!retval) {
        reply = dbus_message_new_error (message, error.name, error.message);
        dbus_error_free (&error);
        return reply;
    }
    
    /* TODO */
    reply = dbus_message_new_method_return (message);
    return reply;
}

static DBusMessage *
_ic_focus_in (BusInputContext  *context,
              DBusMessage      *message,
              BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    DBusMessage *reply;
    /* TODO */
    reply = dbus_message_new_method_return (message);
    return reply;
}

static DBusMessage *
_ic_focus_out (BusInputContext  *context,
              DBusMessage      *message,
              BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    DBusMessage *reply;
    /* TODO */
    reply = dbus_message_new_method_return (message);
    return reply;
}

static DBusMessage *
_ic_reset (BusInputContext  *context,
           DBusMessage      *message,
           BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    DBusMessage *reply;
    /* TODO */
    reply = dbus_message_new_method_return (message);
    return reply;
}

static DBusMessage *
_ic_set_capabilities (BusInputContext  *context,
                      DBusMessage      *message,
                      BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));
    
    DBusMessage *reply;
    guint32 caps;
    gboolean retval;
    DBusError error;

    dbus_error_init (&error);
    retval = dbus_message_get_args (message, &error,
                DBUS_TYPE_UINT32, &caps,
                DBUS_TYPE_INVALID);

    if (!retval) {
        reply = dbus_message_new_error (message, error.name, error.message);
        dbus_error_free (&error);
        return reply;
    }

    /* TODO */
    reply = dbus_message_new_method_return (message);
    return reply;
}

static DBusMessage *
_ic_is_enabled (BusInputContext  *context,
                DBusMessage      *message,
                BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));
    
    DBusMessage *reply;
    gboolean retval;

    retval = FALSE;
    /* TODO */
    reply = dbus_message_new_method_return (message);
    dbus_message_append_args (reply,
            DBUS_TYPE_BOOLEAN, &retval,
            DBUS_TYPE_INVALID);

    return reply;
}

static DBusMessage *
_ic_get_factory (BusInputContext  *context,
                 DBusMessage      *message,
                 BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));
    
    DBusMessage *reply;
    gchar *factory = "";

    /* TODO */
    reply = dbus_message_new_method_return (message);
    dbus_message_append_args (reply,
            DBUS_TYPE_STRING, &factory,
            DBUS_TYPE_INVALID);

    return reply;
}

static DBusMessage *
_ic_destroy (BusInputContext  *context,
             DBusMessage      *message,
             BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));
    
    DBusMessage *reply;
    
    /* TODO */
    reply = dbus_message_new_method_return (message);
    
    return reply;
}

static gboolean
bus_input_context_dbus_message (BusInputContext *input_context,
                                BusConnection   *connection,
                                DBusMessage     *message)
{
    g_assert (BUS_IS_INPUT_CONTEXT (input_context));
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    gint i;
    DBusMessage *reply_message = NULL;

    struct {
        const gchar *interface;
        const gchar *name;
        DBusMessage *(* handler) (BusInputContext *, DBusMessage *, BusConnection *);
    } handlers[] =  {
        /* Introspectable interface */
        { DBUS_INTERFACE_INTROSPECTABLE,
                               "Introspect", _ibus_introspect },
        /* IBus interface */
        { IBUS_INTERFACE_INPUT_CONTEXT, "ProcessKeyEvent",   _ic_process_key_event },
        { IBUS_INTERFACE_INPUT_CONTEXT, "SetCursorLocation", _ic_set_cursor_location },
        { IBUS_INTERFACE_INPUT_CONTEXT, "FocusIn",           _ic_focus_in },
        { IBUS_INTERFACE_INPUT_CONTEXT, "FocusOut",          _ic_focus_out },
        { IBUS_INTERFACE_INPUT_CONTEXT, "Reset",             _ic_reset },
        { IBUS_INTERFACE_INPUT_CONTEXT, "SetCapabilities",   _ic_set_capabilities },
        { IBUS_INTERFACE_INPUT_CONTEXT, "IsEnabled",         _ic_is_enabled },
        { IBUS_INTERFACE_INPUT_CONTEXT, "GetFactory",        _ic_get_factory },
        { IBUS_INTERFACE_INPUT_CONTEXT, "Destroy",           _ic_destroy },
        
        { NULL, NULL, NULL }
    };

    dbus_message_set_sender (message, bus_connection_get_unique_name (connection));
    dbus_message_set_destination (message, DBUS_SERVICE_DBUS);

    for (i = 0; handlers[i].interface != NULL; i++) {
        if (dbus_message_is_method_call (message,
                                         handlers[i].interface,
                                         handlers[i].name)) {

            reply_message = handlers[i].handler (input_context, message, connection);
            if (reply_message) {

                dbus_message_set_sender (reply_message,
                                         DBUS_SERVICE_DBUS);
                dbus_message_set_destination (reply_message,
                                              bus_connection_get_unique_name (connection));
                dbus_message_set_no_reply (reply_message, TRUE);

                ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
                dbus_message_unref (reply_message);
            }

            g_signal_stop_emission_by_name (input_context, "dbus-message");
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
                        BusInputContext     *input_context)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (BUS_IS_INPUT_CONTEXT (input_context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (input_context);

    /*
    ibus_service_remove_from_connection (
                    IBUS_SERVICE (input_context),
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
bus_input_context_new_connection (BusInputContext    *input_context,
                              BusConnection  *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (input_context));
    g_assert (BUS_IS_CONNECTION (connection));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (input_context);

    g_assert (g_slist_find (priv->connections, connection) == NULL);

    g_object_ref (G_OBJECT (connection));
    priv->connections = g_slist_append (priv->connections, connection);

    g_signal_connect (connection, "destroy",
                      (GCallback) _connection_destroy_cb,
                      input_context);

    ibus_service_add_to_connection (
            IBUS_SERVICE (input_context),
            IBUS_CONNECTION (connection));

    return TRUE;
}
