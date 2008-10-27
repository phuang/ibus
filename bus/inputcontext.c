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

#include <ibusinternal.h>
#include <ibusmarshalers.h>
#include "inputcontext.h"
#include "engineproxy.h"

#define BUS_INPUT_CONTEXT_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_INPUT_CONTEXT, BusInputContextPrivate))

enum {
    FOCUS_IN,
    FOCUS_OUT,
    LAST_SIGNAL,
};

enum {
    PROP_0,
};


/* IBusInputContextPriv */
struct _BusInputContextPrivate {
    BusConnection *connection;
    BusEngineProxy *engine;
    gchar *client;
    gboolean has_focus;
};

typedef struct _BusInputContextPrivate BusInputContextPrivate;

static guint    context_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_input_context_class_init    (BusInputContextClass   *klass);
static void     bus_input_context_init          (BusInputContext        *context);
static void     bus_input_context_destroy       (BusInputContext        *context);
static gboolean bus_input_context_dbus_message  (BusInputContext        *context,
                                                 BusConnection          *connection,
                                                 DBusMessage            *message);

static IBusServiceClass  *_parent_class = NULL;
static guint id = 0;

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

static void
_connection_destroy_cb (BusConnection   *connection,
                        BusInputContext *context)
{
    BUS_IS_CONNECTION (connection);
    BUS_IS_INPUT_CONTEXT (context);

    ibus_object_destroy (IBUS_OBJECT (context));
}


BusInputContext *
bus_input_context_new (BusConnection    *connection,
                       const gchar      *client)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (client != NULL);

    BusInputContext *context;
    gchar *path;
    BusInputContextPrivate *priv;

    path = g_strdup_printf (IBUS_PATH_IBUS "/InputContext%d", ++id);

    context = BUS_INPUT_CONTEXT (g_object_new (BUS_TYPE_INPUT_CONTEXT,
                    "path", path,
                    NULL));
    g_free (path);    
    
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    ibus_service_add_to_connection (IBUS_SERVICE (context),
                                 IBUS_CONNECTION (connection));

    g_object_ref (connection);
    priv->connection = connection;
    priv->client = g_strdup (client);

    g_signal_connect (priv->connection,
                      "destroy",
                      (GCallback) _connection_destroy_cb,
                      context);

    return context;
}

static void
bus_input_context_class_init (BusInputContextClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    _parent_class = (IBusServiceClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusInputContextPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_input_context_destroy;

    IBUS_SERVICE_CLASS (klass)->dbus_message = (ServiceDBusMessageFunc) bus_input_context_dbus_message;

    /* install signals */
    context_signals[FOCUS_IN] =
        g_signal_new (I_("focus-in"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    
    context_signals[FOCUS_OUT] =
        g_signal_new (I_("focus-out"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);


}

static void
bus_input_context_init (BusInputContext *context)
{
    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    priv->connection = NULL;
    priv->client = NULL;
    priv->engine = NULL;
    priv->has_focus = FALSE;
}

static void
bus_input_context_destroy (BusInputContext *context)
{
    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->connection) {
        g_signal_handlers_disconnect_by_func (priv->connection,
                                         (GCallback) _connection_destroy_cb,
                                         context);
        g_object_unref (priv->connection);
        priv->connection = NULL;
    }

    if (priv->client) {
        g_free (priv->client);
        priv->client = NULL;
    }

    IBUS_OBJECT_CLASS(_parent_class)->destroy (IBUS_OBJECT (context));
}

/* introspectable interface */
static DBusMessage *
_ibus_introspect (BusInputContext   *context,
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
    dbus_message_append_args (reply,
                              DBUS_TYPE_BOOLEAN, &retval,
                              DBUS_TYPE_INVALID);

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
    ibus_object_destroy (IBUS_OBJECT (context));
    reply = dbus_message_new_method_return (message);
    
    return reply;
}

static gboolean
bus_input_context_dbus_message (BusInputContext *context,
                                BusConnection   *connection,
                                DBusMessage     *message)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
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

            reply_message = handlers[i].handler (context, message, connection);
            if (reply_message) {

                dbus_message_set_sender (reply_message,
                                         DBUS_SERVICE_DBUS);
                dbus_message_set_destination (reply_message,
                                              bus_connection_get_unique_name (connection));
                dbus_message_set_no_reply (reply_message, TRUE);

                ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
                dbus_message_unref (reply_message);
            }

            g_signal_stop_emission_by_name (context, "dbus-message");
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


gboolean
bus_input_context_is_focus (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    return priv->has_focus;
}

void
bus_input_context_focus_in (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->has_focus)
        return;

    g_signal_emit (context, context_signals[FOCUS_IN], 0);
}

void
bus_input_context_focus_out (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (!priv->has_focus)
        return;

    g_signal_emit (context, context_signals[FOCUS_OUT], 0);
}
