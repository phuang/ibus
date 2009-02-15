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
#include "ibusimpl.h"
#include "inputcontext.h"
#include "engineproxy.h"
#include "factoryproxy.h"

#define BUS_INPUT_CONTEXT_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_INPUT_CONTEXT, BusInputContextPrivate))

enum {
    PROCESS_KEY_EVENT,
    SET_CURSOR_LOCATION,
    FOCUS_IN,
    FOCUS_OUT,
    UPDATE_PREEDIT_TEXT,
    SHOW_PREEDIT_TEXT,
    HIDE_PREEDIT_TEXT,
    UPDATE_AUXILIARY_TEXT,
    SHOW_AUXILIARY_TEXT,
    HIDE_AUXILIARY_TEXT,
    UPDATE_LOOKUP_TABLE,
    SHOW_LOOKUP_TABLE,
    HIDE_LOOKUP_TABLE,
    PAGE_UP_LOOKUP_TABLE,
    PAGE_DOWN_LOOKUP_TABLE,
    CURSOR_UP_LOOKUP_TABLE,
    CURSOR_DOWN_LOOKUP_TABLE,
    REGISTER_PROPERTIES,
    UPDATE_PROPERTY,
    ENABLED,
    DISABLED,
    ENGINE_CHANGED,
    REQUEST_ENGINE,
    REQUEST_NEXT_ENGINE,
    REQUEST_PREV_ENGINE,
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
    gboolean enabled;

    /* capabilities */
    guint capabilities;

    /* cursor location */
    gint x;
    gint y;
    gint w;
    gint h;
};

typedef struct _BusInputContextPrivate BusInputContextPrivate;

static guint    context_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_input_context_class_init    (BusInputContextClass   *klass);
static void     bus_input_context_init          (BusInputContext        *context);
static void     bus_input_context_destroy       (BusInputContext        *context);
static gboolean bus_input_context_ibus_message  (BusInputContext        *context,
                                                 BusConnection          *connection,
                                                 IBusMessage            *message);
static gboolean bus_input_context_filter_keyboard_shortcuts
                                                (BusInputContext        *context,
                                                 guint                   keyval,
                                                 guint                   modifiers);
static gboolean bus_input_context_send_signal   (BusInputContext        *context,
                                                 const gchar            *signal_name,
                                                 GType                   first_arg_type,
                                                 ...);

static void     bus_input_context_unset_engine  (BusInputContext        *context);
static void     _engine_destroy_cb              (BusEngineProxy         *factory,
                                                 BusInputContext        *context);

static IBusServiceClass  *parent_class = NULL;
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

    path = g_strdup_printf (IBUS_PATH_INPUT_CONTEXT, ++id);

    context = (BusInputContext *) g_object_new (BUS_TYPE_INPUT_CONTEXT,
                                                "path", path,
                                                NULL);
    g_free (path);

    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

#if 0
    ibus_service_add_to_connection (IBUS_SERVICE (context),
                                 IBUS_CONNECTION (connection));
#endif

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

    parent_class = (IBusServiceClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusInputContextPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_input_context_destroy;

    IBUS_SERVICE_CLASS (klass)->ibus_message = (ServiceIBusMessageFunc) bus_input_context_ibus_message;

    /* install signals */
    context_signals[PROCESS_KEY_EVENT] =
        g_signal_new (I_("process-key-event"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_BOOL__UINT_UINT,
            G_TYPE_BOOLEAN,
            2,
            G_TYPE_UINT,
            G_TYPE_UINT);

    context_signals[SET_CURSOR_LOCATION] =
        g_signal_new (I_("set-cursor-location"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__INT_INT_INT_INT,
            G_TYPE_NONE,
            4,
            G_TYPE_INT,
            G_TYPE_INT,
            G_TYPE_INT,
            G_TYPE_INT);

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

    context_signals[UPDATE_PREEDIT_TEXT] =
        g_signal_new (I_("update-preedit-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__OBJECT_UINT_BOOLEAN,
            G_TYPE_NONE,
            3,
            IBUS_TYPE_TEXT,
            G_TYPE_UINT,
            G_TYPE_BOOLEAN);

    context_signals[SHOW_PREEDIT_TEXT] =
        g_signal_new (I_("show-preedit-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[HIDE_PREEDIT_TEXT] =
        g_signal_new (I_("hide-preedit-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[UPDATE_AUXILIARY_TEXT] =
        g_signal_new (I_("update-auxiliary-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__OBJECT_BOOLEAN,
            G_TYPE_NONE,
            2,
            IBUS_TYPE_TEXT,
            G_TYPE_BOOLEAN);

    context_signals[SHOW_AUXILIARY_TEXT] =
        g_signal_new (I_("show-auxiliary-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[HIDE_AUXILIARY_TEXT] =
        g_signal_new (I_("hide-auxiliary-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[UPDATE_LOOKUP_TABLE] =
        g_signal_new (I_("update-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__OBJECT_BOOLEAN,
            G_TYPE_NONE,
            2,
            IBUS_TYPE_LOOKUP_TABLE,
            G_TYPE_BOOLEAN);

    context_signals[SHOW_LOOKUP_TABLE] =
        g_signal_new (I_("show-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[HIDE_LOOKUP_TABLE] =
        g_signal_new (I_("hide-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[PAGE_UP_LOOKUP_TABLE] =
        g_signal_new (I_("page-up-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[PAGE_DOWN_LOOKUP_TABLE] =
        g_signal_new (I_("page-down-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[CURSOR_UP_LOOKUP_TABLE] =
        g_signal_new (I_("cursor-up-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[CURSOR_DOWN_LOOKUP_TABLE] =
        g_signal_new (I_("cursor-down-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[REGISTER_PROPERTIES] =
        g_signal_new (I_("register-properties"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__OBJECT,
            G_TYPE_NONE,
            1,
            IBUS_TYPE_PROP_LIST);

    context_signals[UPDATE_PROPERTY] =
        g_signal_new (I_("update-property"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__OBJECT,
            G_TYPE_NONE,
            1,
            IBUS_TYPE_PROPERTY);

    context_signals[ENABLED] =
        g_signal_new (I_("enabled"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[DISABLED] =
        g_signal_new (I_("disabled"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[ENGINE_CHANGED] =
        g_signal_new (I_("factory-changed"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[REQUEST_ENGINE] =
        g_signal_new (I_("request-engine"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING,
            G_TYPE_NONE,
            1,
            G_TYPE_STRING);

    context_signals[REQUEST_NEXT_ENGINE] =
        g_signal_new (I_("request-next-engine"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[REQUEST_PREV_ENGINE] =
        g_signal_new (I_("request-prev-engine"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);
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
    priv->enabled = FALSE;

    priv->capabilities = 0;

    priv->x = 0;
    priv->y = 0;
    priv->w = 0;
    priv->h = 0;

}

static void
bus_input_context_destroy (BusInputContext *context)
{
    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->has_focus) {
        bus_input_context_focus_out (context);
        priv->has_focus = FALSE;
    }

    if (priv->engine) {
        bus_input_context_unset_engine (context);
    }

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

    IBUS_OBJECT_CLASS(parent_class)->destroy (IBUS_OBJECT (context));
}

/* introspectable interface */
static IBusMessage *
_ibus_introspect (BusInputContext   *context,
                  IBusMessage       *message,
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
        "  <interface name=\"org.freedesktop.IBus\">\n"
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

typedef struct {
    BusInputContext *context;
    IBusMessage     *message;
} CallData;

static void
_ic_process_key_event_reply_cb (gpointer data,
                                gpointer user_data)
{
    gboolean retval;
    CallData *call_data;
    IBusMessage *reply;

    retval = (gboolean) GPOINTER_TO_INT (data);
    call_data = (CallData *) user_data;

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (call_data->context);


    reply = ibus_message_new_method_return (call_data->message);
    ibus_message_append_args (reply,
                              G_TYPE_BOOLEAN, &retval,
                              G_TYPE_INVALID);
    ibus_connection_send ((IBusConnection *)priv->connection, reply);

    g_object_unref (call_data->context);
    ibus_message_unref (call_data->message);
    ibus_message_unref (reply);
    g_slice_free (CallData, call_data);
}

static IBusMessage *
_ic_process_key_event (BusInputContext *context,
                       IBusMessage     *message,
                       BusConnection   *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply = NULL;
    guint keyval, modifiers;
    gboolean retval;
    IBusError *error;

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    retval = ibus_message_get_args (message,
                &error,
                G_TYPE_UINT, &keyval,
                G_TYPE_UINT, &modifiers,
                G_TYPE_INVALID);

    if (!retval) {
        reply = ibus_message_new_error (message,
                                        error->name,
                                        error->message);
        ibus_error_free (error);
        return reply;
    }

    retval = bus_input_context_filter_keyboard_shortcuts (context, keyval, modifiers);

    if (retval) {
        reply = ibus_message_new_method_return (message);
        ibus_message_append_args (reply,
                                  G_TYPE_BOOLEAN, &retval,
                                  G_TYPE_INVALID);
    }
    else if (priv->enabled && priv->engine) {
        CallData *call_data;

        call_data = g_slice_new (CallData);

        g_object_ref (context);
        ibus_message_ref (message);

        call_data->context = context;
        call_data->message = message;

        bus_engine_proxy_process_key_event (priv->engine,
                                            keyval,
                                            modifiers,
                                            (GFunc) _ic_process_key_event_reply_cb,
                                            call_data);
    }
    else {
        reply = ibus_message_new_method_return (message);
        ibus_message_append_args (reply,
                                  G_TYPE_BOOLEAN, &retval,
                                  G_TYPE_INVALID);
    }
    return reply;
}

static IBusMessage *
_ic_set_cursor_location (BusInputContext  *context,
                         IBusMessage      *message,
                         BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;
    guint x, y, w, h;
    gboolean retval;
    IBusError *error;

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    retval = ibus_message_get_args (message, &error,
                G_TYPE_INT, &x,
                G_TYPE_INT, &y,
                G_TYPE_INT, &w,
                G_TYPE_INT, &h,
                G_TYPE_INVALID);

    if (!retval) {
        reply = ibus_message_new_error (message,
                                        error->name,
                                        error->message);
        ibus_error_free (error);
        return reply;
    }

    priv->x = x;
    priv->y = y;
    priv->h = h;
    priv->w = w;

    if (priv->engine) {
        bus_engine_proxy_set_cursor_location (priv->engine, x, y, w, h);
    }

    if (priv->capabilities & IBUS_CAP_FOCUS) {
        g_signal_emit (context,
                       context_signals[SET_CURSOR_LOCATION],
                       0,
                       x,
                       y,
                       w,
                       h);
    }

    reply = ibus_message_new_method_return (message);
    return reply;
}

static IBusMessage *
_ic_focus_in (BusInputContext  *context,
              IBusMessage      *message,
              BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;

    bus_input_context_focus_in (context);

    reply = ibus_message_new_method_return (message);

    return reply;
}

static IBusMessage *
_ic_focus_out (BusInputContext  *context,
              IBusMessage      *message,
              BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;

    bus_input_context_focus_out (context);

    reply = ibus_message_new_method_return (message);

    return reply;
}

static IBusMessage *
_ic_reset (BusInputContext  *context,
           IBusMessage      *message,
           BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);


    if (priv->engine) {
        bus_engine_proxy_reset (priv->engine);
    }

    reply = ibus_message_new_method_return (message);
    return reply;
}

static IBusMessage *
_ic_set_capabilities (BusInputContext  *context,
                      IBusMessage      *message,
                      BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;
    guint caps;
    gboolean retval;
    IBusError *error;

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);


    retval = ibus_message_get_args (message,
                &error,
                G_TYPE_UINT, &caps,
                G_TYPE_INVALID);

    if (!retval) {
        reply = ibus_message_new_error (message,
                                        error->name,
                                        error->message);
        ibus_error_free (error);
        return reply;
    }

    if (priv->capabilities != caps) {
        priv->capabilities = caps;

        if (priv->engine) {
            bus_engine_proxy_set_capabilities (priv->engine, caps);
        }
    }

    reply = ibus_message_new_method_return (message);
    return reply;
}

static IBusMessage *
_ic_is_enabled (BusInputContext *context,
                IBusMessage     *message,
                BusConnection   *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    reply = ibus_message_new_method_return (message);
    ibus_message_append_args (reply,
            G_TYPE_BOOLEAN, &priv->enabled,
            G_TYPE_INVALID);

    return reply;
}

static IBusMessage *
_ic_set_engine (BusInputContext  *context,
                 IBusMessage      *message,
                 BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    gboolean retval;
    IBusMessage *reply;
    IBusError *error;
    gchar *engine_name;

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    retval = ibus_message_get_args (message,
                                    &error,
                                    G_TYPE_STRING, &engine_name,
                                    G_TYPE_INVALID);
     if (!retval) {
        reply = ibus_message_new_error (message,
                                        error->name,
                                        error->message);
        ibus_error_free (error);
        return reply;
    }

    g_signal_emit (context, context_signals[REQUEST_ENGINE], 0, engine_name);

    if (priv->engine == NULL) {
        reply = ibus_message_new_error_printf (message,
                                               "org.freedesktop.IBus.NoEngine",
                                               "can not find engine with name %s",
                                               engine_name);
        return reply;
    }

    bus_input_context_enable (context);

    reply = ibus_message_new_method_return (message);
    return reply;
}

static IBusMessage *
_ic_get_engine (BusInputContext  *context,
                IBusMessage      *message,
                BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;
    IBusEngineDesc *desc;

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->engine) {
        desc = bus_engine_proxy_get_desc (priv->engine);
        if (desc != NULL) {
            reply = ibus_message_new_method_return (message);
            ibus_message_append_args (reply,
                                      IBUS_TYPE_ENGINE_DESC, &desc,
                                      G_TYPE_INVALID);
            return reply;
        }
    }

    reply = ibus_message_new_error (message,
                                    DBUS_ERROR_FAILED,
                                    "InputContext does not have factory.");
    return reply;
}

static IBusMessage *
_ic_destroy (BusInputContext  *context,
             IBusMessage      *message,
             BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;
    reply = ibus_message_new_method_return (message);

    ibus_connection_send ((IBusConnection *) connection, reply);
    ibus_connection_flush ((IBusConnection *) connection);
    ibus_message_unref (reply);

    ibus_object_destroy ((IBusObject *) context);

    return NULL;
}

static gboolean
bus_input_context_ibus_message (BusInputContext *context,
                                BusConnection   *connection,
                                IBusMessage     *message)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    gint i;
    IBusMessage *reply_message = NULL;

    static const struct {
        const gchar *interface;
        const gchar *name;
        IBusMessage *(* handler) (BusInputContext *, IBusMessage *, BusConnection *);
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
        { IBUS_INTERFACE_INPUT_CONTEXT, "SetEngine",         _ic_set_engine },
        { IBUS_INTERFACE_INPUT_CONTEXT, "GetEngine",         _ic_get_engine },
        { IBUS_INTERFACE_INPUT_CONTEXT, "Destroy",           _ic_destroy },

        { NULL, NULL, NULL }
    };

    ibus_message_set_sender (message, bus_connection_get_unique_name (connection));
    ibus_message_set_destination (message, DBUS_SERVICE_DBUS);

    for (i = 0; handlers[i].interface != NULL; i++) {
        if (ibus_message_is_method_call (message,
                                         handlers[i].interface,
                                         handlers[i].name)) {

            reply_message = handlers[i].handler (context, message, connection);
            if (reply_message) {

                ibus_message_set_sender (reply_message,
                                         DBUS_SERVICE_DBUS);
                ibus_message_set_destination (reply_message,
                                              bus_connection_get_unique_name (connection));
                ibus_message_set_no_reply (reply_message, TRUE);

                ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
                ibus_message_unref (reply_message);
            }

            g_signal_stop_emission_by_name (context, "ibus-message");
            return TRUE;
        }
    }

    return parent_class->ibus_message ((IBusService *)context,
                                       (IBusConnection *)connection,
                                       message);
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

    priv->has_focus = TRUE;

    if (priv->engine && priv->enabled) {
        bus_engine_proxy_focus_in (priv->engine);
    }

    if (priv->capabilities & IBUS_CAP_FOCUS) {
        g_signal_emit (context, context_signals[FOCUS_IN], 0);
    }
}

void
bus_input_context_focus_out (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (!priv->has_focus)
        return;

    priv->has_focus = FALSE;

    if (priv->engine && priv->enabled) {
        bus_engine_proxy_focus_out (priv->engine);
    }

    if (priv->capabilities & IBUS_CAP_FOCUS) {
        g_signal_emit (context, context_signals[FOCUS_OUT], 0);
    }
}

void
bus_input_context_page_up (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->engine) {
        bus_engine_proxy_page_up (priv->engine);
    }
}

void
bus_input_context_page_down (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->engine) {
        bus_engine_proxy_page_down (priv->engine);
    }
}

void
bus_input_context_cursor_up (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->engine) {
        bus_engine_proxy_cursor_up (priv->engine);
    }
}

void
bus_input_context_cursor_down (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->engine) {
        bus_engine_proxy_cursor_down (priv->engine);
    }
}

void
bus_input_context_property_activate (BusInputContext *context,
                                     const gchar     *prop_name,
                                     gint             prop_state)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->engine) {
        bus_engine_proxy_property_activate (priv->engine, prop_name, prop_state);
    }
}

static void
_engine_destroy_cb (BusEngineProxy  *engine,
                    BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    g_assert (priv->engine == engine);

    bus_input_context_unset_engine (context);
}

static void
_engine_commit_text_cb (BusEngineProxy  *engine,
                        IBusText        *text,
                        BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (text != NULL);
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    g_assert (priv->engine == engine);

    bus_input_context_send_signal (context,
                                   "CommitText",
                                   IBUS_TYPE_TEXT, &text,
                                   G_TYPE_INVALID);

}

static void
_engine_forward_key_event_cb (BusEngineProxy    *engine,
                              guint              keyval,
                              guint              state,
                              BusInputContext   *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    g_assert (priv->engine == engine);

    bus_input_context_send_signal (context,
                                   "ForwardKeyEvent",
                                   G_TYPE_UINT,  &keyval,
                                   G_TYPE_UINT,  &state,
                                   G_TYPE_INVALID);

}

static void
_engine_update_preedit_text_cb (BusEngineProxy  *engine,
                                IBusText        *text,
                                guint            cursor_pos,
                                gboolean         visible,
                                BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (IBUS_IS_TEXT (text));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    g_assert (priv->engine == engine);

    if (priv->capabilities & IBUS_CAP_PREEDIT_TEXT) {
        bus_input_context_send_signal (context,
                                       "UpdatePreeditText",
                                       IBUS_TYPE_TEXT, &text,
                                       G_TYPE_UINT, &cursor_pos,
                                       G_TYPE_BOOLEAN, &visible,
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[UPDATE_PREEDIT_TEXT],
                       0,
                       text,
                       cursor_pos,
                       visible);
    }
}

static void
_engine_update_auxiliary_text_cb (BusEngineProxy   *engine,
                                  IBusText         *text,
                                  gboolean          visible,
                                  BusInputContext  *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (IBUS_IS_TEXT (text));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    g_assert (priv->engine == engine);

    if (priv->capabilities & IBUS_CAP_AUXILIARY_TEXT) {
        bus_input_context_send_signal (context,
                                       "UpdateAuxiliaryText",
                                       IBUS_TYPE_TEXT, &text,
                                       G_TYPE_BOOLEAN, &visible,
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[UPDATE_AUXILIARY_TEXT],
                       0,
                       text,
                       visible);
    }
}

static void
_engine_update_lookup_table_cb (BusEngineProxy   *engine,
                                IBusLookupTable  *table,
                                gboolean          visible,
                                BusInputContext  *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (IBUS_IS_LOOKUP_TABLE (table));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    g_assert (priv->engine == engine);

    if (priv->capabilities & IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_send_signal (context,
                                       "UpdateLookupTable",
                                       IBUS_TYPE_LOOKUP_TABLE, &table,
                                       G_TYPE_BOOLEAN, &visible,
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[UPDATE_LOOKUP_TABLE],
                       0,
                       table,
                       visible);
    }
}

static void
_engine_register_properties_cb (BusEngineProxy  *engine,
                                IBusPropList    *prop_list,
                                BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (IBUS_IS_PROP_LIST (prop_list));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    g_assert (priv->engine == engine);

    if (priv->capabilities & IBUS_CAP_PROPERTY) {
        bus_input_context_send_signal (context,
                                       "RegisterProperties",
                                       IBUS_TYPE_PROP_LIST, &prop_list,
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[REGISTER_PROPERTIES],
                       0,
                       prop_list);
    }
}

static void
_engine_update_property_cb (BusEngineProxy  *engine,
                            IBusProperty    *prop,
                            BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (IBUS_IS_PROPERTY (prop));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    g_assert (priv->engine == engine);

    if (priv->capabilities & IBUS_CAP_PROPERTY) {
        bus_input_context_send_signal (context,
                                       "UpdateProperty",
                                       IBUS_TYPE_PROPERTY, &prop,
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[UPDATE_PROPERTY],
                       0,
                       prop);
    }
}

#define DEFINE_FUNCTION(name, Name, signal_name, cap)           \
    static void                                                 \
    _engine_##name##_cb (BusEngineProxy   *engine,              \
                         BusInputContext *context)              \
    {                                                           \
        g_assert (BUS_IS_ENGINE_PROXY (engine));                \
        g_assert (BUS_IS_INPUT_CONTEXT (context));              \
                                                                \
        BusInputContextPrivate *priv;                           \
        priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);         \
                                                                \
        g_assert (priv->engine == engine);                      \
                                                                \
        if ((priv->capabilities & (cap)) == (cap)) {            \
            bus_input_context_send_signal (context,             \
                                           #Name,               \
                                           G_TYPE_INVALID);     \
        }                                                       \
        else {                                                  \
            g_signal_emit (context,                             \
                       context_signals[signal_name],            \
                       0);                                      \
        }                                                       \
}

DEFINE_FUNCTION (show_preedit_text, ShowPreeditText, SHOW_PREEDIT_TEXT, IBUS_CAP_PREEDIT_TEXT)
DEFINE_FUNCTION (hide_preedit_text, HidePreeditText, HIDE_PREEDIT_TEXT, IBUS_CAP_PREEDIT_TEXT)
DEFINE_FUNCTION (show_auxiliary_text, ShowAuxiliaryText, SHOW_AUXILIARY_TEXT, IBUS_CAP_AUXILIARY_TEXT)
DEFINE_FUNCTION (hide_auxiliary_text, HideAuxiliaryText, HIDE_AUXILIARY_TEXT, IBUS_CAP_AUXILIARY_TEXT)
DEFINE_FUNCTION (show_lookup_table, ShowLookupTable, SHOW_LOOKUP_TABLE, IBUS_CAP_LOOKUP_TABLE)
DEFINE_FUNCTION (hide_lookup_table, HideLookupTable, HIDE_LOOKUP_TABLE, IBUS_CAP_LOOKUP_TABLE)
DEFINE_FUNCTION (page_up_lookup_table, PageUpLookupTable, PAGE_UP_LOOKUP_TABLE, IBUS_CAP_LOOKUP_TABLE)
DEFINE_FUNCTION (page_down_lookup_table, PageDownLookupTable, PAGE_DOWN_LOOKUP_TABLE, IBUS_CAP_LOOKUP_TABLE)
DEFINE_FUNCTION (cursor_up_lookup_table, CursorUpLookupTable, CURSOR_UP_LOOKUP_TABLE, IBUS_CAP_LOOKUP_TABLE)
DEFINE_FUNCTION (cursor_down_lookup_table, CursorDownLookupTable, CURSOR_DOWN_LOOKUP_TABLE, IBUS_CAP_LOOKUP_TABLE)

#undef DEFINE_FUNCTION

void
bus_input_context_enable (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->engine == NULL)
        return;

    priv->enabled = TRUE;

    bus_engine_proxy_enable (priv->engine);
    bus_input_context_send_signal (context,
                                   "Enabled",
                                   G_TYPE_INVALID);
    g_signal_emit (context,
                   context_signals[ENABLED],
                   0);
    if (priv->has_focus) {
        bus_engine_proxy_focus_in (priv->engine);
    }
}

void
bus_input_context_disable (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);


    if (priv->engine) {
        if (priv->has_focus) {
            bus_engine_proxy_focus_out (priv->engine);
        }
        bus_engine_proxy_disable (priv->engine);
    }

    bus_input_context_send_signal (context,
                                   "Disabled",
                                   G_TYPE_INVALID);
    g_signal_emit (context,
                   context_signals[DISABLED],
                   0);

    priv->enabled = FALSE;
}

const static struct {
    const gchar *name;
    GCallback    callback;
} signals [] = {
    { "commit-text",            G_CALLBACK (_engine_commit_text_cb) },
    { "forward-key-event",      G_CALLBACK (_engine_forward_key_event_cb) },
    { "update-preedit-text",    G_CALLBACK (_engine_update_preedit_text_cb) },
    { "show-preedit-text",      G_CALLBACK (_engine_show_preedit_text_cb) },
    { "hide-preedit-text",      G_CALLBACK (_engine_hide_preedit_text_cb) },
    { "update-auxiliary-text",  G_CALLBACK (_engine_update_auxiliary_text_cb) },
    { "show-auxiliary-text",    G_CALLBACK (_engine_show_auxiliary_text_cb) },
    { "hide-auxiliary-text",    G_CALLBACK (_engine_hide_auxiliary_text_cb) },
    { "update-lookup-table",    G_CALLBACK (_engine_update_lookup_table_cb) },
    { "show-lookup-table",      G_CALLBACK (_engine_show_lookup_table_cb) },
    { "hide-lookup-table",      G_CALLBACK (_engine_hide_lookup_table_cb) },
    { "page-up-lookup-table",   G_CALLBACK (_engine_page_up_lookup_table_cb) },
    { "page-down-lookup-table", G_CALLBACK (_engine_page_down_lookup_table_cb) },
    { "cursor-up-lookup-table", G_CALLBACK (_engine_cursor_up_lookup_table_cb) },
    { "cursor-down-lookup-table", G_CALLBACK (_engine_cursor_down_lookup_table_cb) },
    { "register-properties",    G_CALLBACK (_engine_register_properties_cb) },
    { "update-property",        G_CALLBACK (_engine_update_property_cb) },
    { "destroy",                G_CALLBACK (_engine_destroy_cb) },
    { NULL, 0 }
};

static void
bus_input_context_unset_engine (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->engine) {
        gint i;
        for (i = 0; signals[i].name != NULL; i++) {
            g_signal_handlers_disconnect_by_func (priv->engine, signals[i].callback, context);
        }
        g_object_unref (priv->engine);
        priv->engine = NULL;
    }
}

void
bus_input_context_set_engine (BusInputContext *context,
                              BusEngineProxy  *engine)
{

    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    if (priv->engine != NULL) {
        bus_input_context_unset_engine (context);
    }

    if (engine == NULL) {
        bus_input_context_disable (context);
    }
    else {
        gint i;
        priv->engine = engine;
        g_object_ref (priv->engine);

        for (i = 0; signals[i].name != NULL; i++) {
            g_signal_connect (priv->engine,
                              signals[i].name,
                              signals[i].callback,
                              context);
        }
        bus_engine_proxy_set_cursor_location (priv->engine, priv->x, priv->y, priv->w, priv->h);
    }
    g_signal_emit (context,
                   context_signals[ENGINE_CHANGED],
                   0);
}

static gboolean
bus_input_context_filter_keyboard_shortcuts (BusInputContext    *context,
                                             guint               keyval,
                                             guint               modifiers)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusInputContextPrivate *priv;
    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    static GQuark trigger;
    static GQuark next_factory;
    static GQuark prev_factory;

    GQuark event;

    if (trigger == 0) {
        trigger = g_quark_from_static_string ("trigger");
        next_factory = g_quark_from_static_string ("next-engine");
        prev_factory = g_quark_from_static_string ("prev-engine");
    }

    event = ibus_hotkey_profile_filter_key_event (BUS_DEFAULT_HOTKEY_PROFILE,
                                                  keyval,
                                                  modifiers,
                                                  0);

    if (event == trigger) {
        if (priv->engine == NULL) {
            g_signal_emit (context, context_signals[REQUEST_ENGINE], 0, NULL);
        }

        if (priv->engine == NULL) {
            return FALSE;
        }

        if (priv->enabled) {
            bus_input_context_disable (context);
        }
        else {
            bus_input_context_enable (context);
        }

        return TRUE;
    }
    else if (event == next_factory) {
        return TRUE;
    }
    else if (event == prev_factory) {
        return TRUE;
    }
    else
        return FALSE;
}


static gboolean
bus_input_context_send_signal (BusInputContext *context,
                               const gchar     *signal_name,
                               GType            first_arg_type,
                               ...)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (signal_name != NULL);

    va_list args;
    gboolean retval;
    IBusMessage *message;
    BusInputContextPrivate *priv;

    priv = BUS_INPUT_CONTEXT_GET_PRIVATE (context);

    g_assert (priv->connection != NULL);

    message = ibus_message_new_signal (ibus_service_get_path ((IBusService *)context),
                                       IBUS_INTERFACE_INPUT_CONTEXT,
                                       signal_name);

    ibus_message_set_sender (message, IBUS_SERVICE_IBUS);

    va_start (args, first_arg_type);
    ibus_message_append_args_valist (message, first_arg_type, args);
    va_end (args);

    retval = ibus_connection_send ((IBusConnection *)priv->connection, message);
    ibus_message_unref (message);

    return retval;
}
