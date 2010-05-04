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

#include <ibusinternal.h>
#include <ibusmarshalers.h>
#include "ibusimpl.h"
#include "inputcontext.h"
#include "engineproxy.h"
#include "factoryproxy.h"

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
    LAST_SIGNAL,
};

enum {
    PROP_0,
};

typedef struct _BusInputContextPrivate BusInputContextPrivate;

static guint    context_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_input_context_destroy       (BusInputContext        *context);
static gboolean bus_input_context_ibus_message  (BusInputContext        *context,
                                                 BusConnection          *connection,
                                                 IBusMessage            *message);
static gboolean bus_input_context_filter_keyboard_shortcuts
                                                (BusInputContext        *context,
                                                 guint                   keyval,
                                                 guint                   keycode,
                                                 guint                   modifiers);
static gboolean bus_input_context_send_signal   (BusInputContext        *context,
                                                 const gchar            *signal_name,
                                                 GType                   first_arg_type,
                                                 ...);

static void     bus_input_context_unset_engine  (BusInputContext        *context);
static void     bus_input_context_commit_text   (BusInputContext        *context,
                                                 IBusText               *text);
static void     bus_input_context_update_preedit_text
                                                (BusInputContext        *context,
                                                 IBusText               *text,
                                                 guint                   cursor_pos,
                                                 gboolean                visible,
                                                 guint                   mode);
static void     bus_input_context_show_preedit_text
                                                (BusInputContext        *context);
static void     bus_input_context_hide_preedit_text
                                                (BusInputContext        *context);
static void     bus_input_context_update_auxiliary_text
                                                (BusInputContext        *context,
                                                 IBusText               *text,
                                                 gboolean                visible);
static void     bus_input_context_show_auxiliary_text
                                                (BusInputContext        *context);
static void     bus_input_context_hide_auxiliary_text
                                                (BusInputContext        *context);
static void     bus_input_context_update_lookup_table
                                                (BusInputContext        *context,
                                                 IBusLookupTable        *table,
                                                 gboolean                visible);
static void     bus_input_context_show_lookup_table
                                                (BusInputContext        *context);
static void     bus_input_context_hide_lookup_table
                                                (BusInputContext        *context);
static void     bus_input_context_page_up_lookup_table
                                                (BusInputContext        *context);
static void     bus_input_context_page_down_lookup_table
                                                (BusInputContext        *context);
static void     bus_input_context_cursor_up_lookup_table
                                                (BusInputContext        *context);
static void     bus_input_context_cursor_down_lookup_table
                                                (BusInputContext        *context);
static void     bus_input_context_register_properties
                                                (BusInputContext        *context,
                                                 IBusPropList           *props);
static void     bus_input_context_update_property
                                                (BusInputContext        *context,
                                                 IBusProperty           *prop);
static void     _engine_destroy_cb              (BusEngineProxy         *factory,
                                                 BusInputContext        *context);

static guint id = 0;
static IBusText *text_empty = NULL;
static IBusLookupTable *lookup_table_empty = NULL;
static IBusPropList    *props_empty = NULL;

G_DEFINE_TYPE (BusInputContext, bus_input_context, IBUS_TYPE_SERVICE)

/* when send preedit to client */
#define PREEDIT_CONDITION  \
    ((context->capabilities & IBUS_CAP_PREEDIT_TEXT) && \
    (BUS_DEFAULT_IBUS->embed_preedit_text || (context->capabilities & IBUS_CAP_FOCUS) == 0))

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

    path = g_strdup_printf (IBUS_PATH_INPUT_CONTEXT, ++id);

    context = (BusInputContext *) g_object_new (BUS_TYPE_INPUT_CONTEXT,
                                                "path", path,
                                                NULL);
    g_free (path);

#if 0
    ibus_service_add_to_connection (IBUS_SERVICE (context),
                                 IBUS_CONNECTION (connection));
#endif

    g_object_ref_sink (connection);
    context->connection = connection;
    context->client = g_strdup (client);

    g_signal_connect (context->connection,
                      "destroy",
                      (GCallback) _connection_destroy_cb,
                      context);

    return context;
}

static void
bus_input_context_class_init (BusInputContextClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_input_context_destroy;

    IBUS_SERVICE_CLASS (klass)->ibus_message =
            (ServiceIBusMessageFunc) bus_input_context_ibus_message;

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
        g_signal_new (I_("engine-changed"),
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

    text_empty = ibus_text_new_from_string ("");
    g_object_ref_sink (text_empty);
    lookup_table_empty = ibus_lookup_table_new (9, 0, FALSE, FALSE);
    g_object_ref_sink (lookup_table_empty);
    props_empty = ibus_prop_list_new ();
    g_object_ref_sink (props_empty);
}

static void
bus_input_context_init (BusInputContext *context)
{
    context->connection = NULL;
    context->client = NULL;
    context->engine = NULL;
    context->has_focus = FALSE;
    context->enabled = FALSE;

    context->prev_keyval = IBUS_VoidSymbol;
    context->prev_modifiers = 0;

    context->capabilities = 0;

    context->x = 0;
    context->y = 0;
    context->w = 0;
    context->h = 0;

    g_object_ref_sink (text_empty);
    context->preedit_text = text_empty;
    context->preedit_cursor_pos = 0;
    context->preedit_visible = FALSE;
    context->preedit_mode = IBUS_ENGINE_PREEDIT_CLEAR;

    g_object_ref_sink (text_empty);
    context->auxiliary_text = text_empty;
    context->auxiliary_visible = FALSE;

    g_object_ref_sink (lookup_table_empty);
    context->lookup_table = lookup_table_empty;
    context->lookup_table_visible = FALSE;

}

static void
bus_input_context_destroy (BusInputContext *context)
{
    if (context->has_focus) {
        bus_input_context_focus_out (context);
        context->has_focus = FALSE;
    }

    if (context->engine) {
        bus_input_context_unset_engine (context);
    }

    if (context->preedit_text) {
        g_object_unref (context->preedit_text);
        context->preedit_text = NULL;
    }

    if (context->auxiliary_text) {
        g_object_unref (context->auxiliary_text);
        context->auxiliary_text = NULL;
    }

    if (context->lookup_table) {
        g_object_unref (context->lookup_table);
        context->lookup_table = NULL;
    }

    if (context->connection) {
        g_signal_handlers_disconnect_by_func (context->connection,
                                         (GCallback) _connection_destroy_cb,
                                         context);
        g_object_unref (context->connection);
        context->connection = NULL;
    }

    if (context->client) {
        g_free (context->client);
        context->client = NULL;
    }

    IBUS_OBJECT_CLASS(bus_input_context_parent_class)->destroy (IBUS_OBJECT (context));
}

/* introspectable interface */
static IBusMessage *
_ibus_introspect (BusInputContext   *context,
                  IBusMessage       *message,
                  BusConnection     *connection)
{
    static const gchar *introspect =
        DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE
        "<node>\n"
        "  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
        "    <method name=\"Introspect\">\n"
        "      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "  </interface>\n"
        "  <interface name=\"org.freedesktop.IBus.InputContext\">\n"

        /* methods */
        "    <method name=\"ProcessKeyEvent\">\n"
        "      <arg name=\"keyval\" direction=\"in\" type=\"u\"/>\n"
        "      <arg name=\"keycode\" direction=\"in\" type=\"u\"/>\n"
        "      <arg name=\"state\" direction=\"in\" type=\"u\"/>\n"
        "      <arg name=\"handled\" direction=\"out\" type=\"b\"/>\n"
        "    </method>\n"
        "    <method name=\"SetCursorLocation\">\n"
        "      <arg name=\"x\" direction=\"in\" type=\"i\"/>\n"
        "      <arg name=\"y\" direction=\"in\" type=\"i\"/>\n"
        "      <arg name=\"w\" direction=\"in\" type=\"i\"/>\n"
        "      <arg name=\"h\" direction=\"in\" type=\"i\"/>\n"
        "    </method>\n"
        "    <method name=\"FocusIn\"/>\n"
        "    <method name=\"FocusOut\"/>\n"
        "    <method name=\"Reset\"/>\n"
        "    <method name=\"Enable\"/>\n"
        "    <method name=\"Disable\"/>\n"
        "    <method name=\"IsEnabled\">\n"
        "      <arg name=\"enable\" direction=\"out\" type=\"b\"/>\n"
        "    </method>\n"
        "    <method name=\"SetCapabilities\">\n"
        "      <arg name=\"caps\" direction=\"in\" type=\"u\"/>\n"
        "    </method>\n"
        "    <method name=\"PropertyActivate\">\n"
        "      <arg name=\"name\" direction=\"in\" type=\"s\"/>\n"
        "      <arg name=\"state\" direction=\"in\" type=\"i\"/>\n"
        "    </method>\n"
        "    <method name=\"SetEngine\">\n"
        "      <arg name=\"name\" direction=\"in\" type=\"s\"/>\n"
        "    </method>\n"
        "    <method name=\"GetEngine\">\n"
        "      <arg name=\"desc\" direction=\"out\" type=\"v\"/>\n"
        "    </method>\n"
        "    <method name=\"Destroy\"/>\n"

        /* signals */
        "    <signal name=\"CommitText\">\n"
        "      <arg name=\"text\" type=\"v\"/>\n"
        "    </signal>\n"
        "    <signal name=\"Enabled\"/>\n"
        "    <signal name=\"Disabled\"/>\n"
        "    <signal name=\"ForwardKeyEvent\">\n"
        "      <arg name=\"keyval\" type=\"u\"/>\n"
        "      <arg name=\"keycode\" type=\"u\"/>\n"
        "      <arg name=\"state\" type=\"u\"/>\n"
        "    </signal>\n"

        "    <signal name=\"UpdatePreeditText\">\n"
        "      <arg name=\"text\" type=\"v\"/>\n"
        "      <arg name=\"cursor_pos\" type=\"u\"/>\n"
        "      <arg name=\"visible\" type=\"b\"/>\n"
        "    </signal>\n"
        "    <signal name=\"ShowPreeditText\"/>\n"
        "    <signal name=\"HidePreeditText\"/>\n"

        "    <signal name=\"UpdateAuxiliaryText\">\n"
        "      <arg name=\"text\" type=\"v\"/>\n"
        "      <arg name=\"visible\" type=\"b\"/>\n"
        "    </signal>\n"
        "    <signal name=\"ShowAuxiliaryText\"/>\n"
        "    <signal name=\"HideAuxiliaryText\"/>\n"

        "    <signal name=\"UpdateLookupTable\">\n"
        "      <arg name=\"table\" type=\"v\"/>\n"
        "      <arg name=\"visible\" type=\"b\"/>\n"
        "    </signal>\n"
        "    <signal name=\"ShowLookupTable\"/>\n"
        "    <signal name=\"HideLookupTable\"/>\n"
        "    <signal name=\"PageUpLookupTable\"/>\n"
        "    <signal name=\"PageDownLookupTable\"/>\n"
        "    <signal name=\"CursorUpLookupTable\"/>\n"
        "    <signal name=\"CursorDownLookupTable\"/>\n"

        "    <signal name=\"RegisterProperties\">\n"
        "      <arg name=\"props\" type=\"v\"/>\n"
        "    </signal>\n"
        "    <signal name=\"UpdateProperty\">\n"
        "      <arg name=\"prop\" type=\"v\"/>\n"
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

    retval = (gboolean) GPOINTER_TO_INT (data);
    call_data = (CallData *) user_data;

    /* make sure the connection is alive */
    if (G_LIKELY (call_data->context->connection != NULL)) {
        IBusMessage *reply;
        reply = ibus_message_new_method_return (call_data->message);
        ibus_message_append_args (reply,
                                  G_TYPE_BOOLEAN, &retval,
                                  G_TYPE_INVALID);

        ibus_connection_send ((IBusConnection *)call_data->context->connection, reply);
        ibus_message_unref (reply);
    }

    g_object_unref (call_data->context);
    ibus_message_unref (call_data->message);
    g_slice_free (CallData, call_data);
}

static IBusMessage *
_ic_process_key_event  (BusInputContext *context,
                        IBusMessage     *message,
                        BusConnection   *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;
    guint keyval, keycode, modifiers;
    gboolean retval;
    IBusError *error;

    retval = ibus_message_get_args (message,
                &error,
                G_TYPE_UINT, &keyval,
                G_TYPE_UINT, &keycode,
                G_TYPE_UINT, &modifiers,
                G_TYPE_INVALID);

    if (!retval) {
        reply = ibus_message_new_error (message,
                                        error->name,
                                        error->message);
        ibus_error_free (error);
        return reply;
    }

    if (G_UNLIKELY (!context->has_focus)) {
        /* workaround: set focus if context does not have focus */
        bus_input_context_focus_in (context);
    }

    if (G_LIKELY (context->has_focus)) {
        retval = bus_input_context_filter_keyboard_shortcuts (context, keyval, keycode, modifiers);
        /* If it is keyboard shortcut, reply TRUE to client */
        if (G_UNLIKELY (retval)) {
            reply = ibus_message_new_method_return (message);
            ibus_message_append_args (reply,
                                      G_TYPE_BOOLEAN, &retval,
                                      G_TYPE_INVALID);
            return reply;
        }
    }

    if (context->has_focus && context->enabled && context->engine) {
        CallData *call_data;

        call_data = g_slice_new (CallData);

        g_object_ref (context);
        ibus_message_ref (message);

        call_data->context = context;
        call_data->message = message;

        bus_engine_proxy_process_key_event (context->engine,
                                            keyval,
                                            keycode,
                                            modifiers,
                                            (GFunc) _ic_process_key_event_reply_cb,
                                            call_data);
        return NULL;
    }
    else {
        retval = FALSE;
        reply = ibus_message_new_method_return (message);
        ibus_message_append_args (reply,
                                  G_TYPE_BOOLEAN, &retval,
                                  G_TYPE_INVALID);
        return reply;
    }
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

    context->x = x;
    context->y = y;
    context->h = h;
    context->w = w;

    if (context->has_focus && context->enabled && context->engine) {
        bus_engine_proxy_set_cursor_location (context->engine, x, y, w, h);
    }

    if (context->capabilities & IBUS_CAP_FOCUS) {
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

    if (context->capabilities & IBUS_CAP_FOCUS) {
        bus_input_context_focus_in (context);
        reply = ibus_message_new_method_return (message);
    }
    else {
        reply = ibus_message_new_error (message,
                                        DBUS_ERROR_FAILED,
                                        "The input context does not support focus.");
    }

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

    if (context->capabilities & IBUS_CAP_FOCUS) {
        bus_input_context_focus_out (context);
        reply = ibus_message_new_method_return (message);
    }
    else {
        reply = ibus_message_new_error (message,
                                        DBUS_ERROR_FAILED,
                                        "The input context does not support focus.");
    }

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

    if (context->enabled && context->engine) {
        bus_engine_proxy_reset (context->engine);
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

    if (context->capabilities != caps) {
        context->capabilities = caps;

        /* If the context does not support IBUS_CAP_FOCUS, then we always assume
         * it has focus. */
        if ((caps & IBUS_CAP_FOCUS) == 0) {
            bus_input_context_focus_in (context);
        }

        if (context->engine) {
            bus_engine_proxy_set_capabilities (context->engine, caps);
        }
    }

    reply = ibus_message_new_method_return (message);
    return reply;
}

static IBusMessage *
_ic_property_activate (BusInputContext  *context,
                       IBusMessage      *message,
                       BusConnection    *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;
    gchar *prop_name;
    gint prop_state;
    gboolean retval;
    IBusError *error;

    retval = ibus_message_get_args (message,
                                    &error,
                                    G_TYPE_STRING, &prop_name,
                                    G_TYPE_INT, &prop_state,
                                    G_TYPE_INVALID);

    if (!retval) {
        reply = ibus_message_new_error (message,
                                        error->name,
                                        error->message);
        ibus_error_free (error);
        return reply;
    }

    if (context->enabled && context->engine) {
        bus_engine_proxy_property_activate (context->engine, prop_name, prop_state);
    }

    reply = ibus_message_new_method_return (message);
    return reply;
}

static IBusMessage *
_ic_enable (BusInputContext *context,
            IBusMessage     *message,
            BusConnection   *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;

    bus_input_context_enable (context);

    reply = ibus_message_new_method_return (message);
    return reply;
}

static IBusMessage *
_ic_disable (BusInputContext *context,
            IBusMessage     *message,
            BusConnection   *connection)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;

    bus_input_context_disable (context);

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

    reply = ibus_message_new_method_return (message);
    ibus_message_append_args (reply,
            G_TYPE_BOOLEAN, &context->enabled,
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

    if (context->engine == NULL) {
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

    if (context->engine) {
        desc = bus_engine_proxy_get_desc (context->engine);
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
        { IBUS_INTERFACE_INPUT_CONTEXT, "PropertyActivate",  _ic_property_activate },
        { IBUS_INTERFACE_INPUT_CONTEXT, "Enable",            _ic_enable },
        { IBUS_INTERFACE_INPUT_CONTEXT, "Disable",           _ic_disable },
        { IBUS_INTERFACE_INPUT_CONTEXT, "IsEnabled",         _ic_is_enabled },
        { IBUS_INTERFACE_INPUT_CONTEXT, "SetEngine",         _ic_set_engine },
        { IBUS_INTERFACE_INPUT_CONTEXT, "GetEngine",         _ic_get_engine },
        { IBUS_INTERFACE_INPUT_CONTEXT, "Destroy",           _ic_destroy },
    };

    ibus_message_set_sender (message, bus_connection_get_unique_name (connection));
    ibus_message_set_destination (message, DBUS_SERVICE_DBUS);

    for (i = 0; i < G_N_ELEMENTS (handlers); i++) {
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

    return IBUS_SERVICE_CLASS (bus_input_context_parent_class)->ibus_message (
                                (IBusService *)context,
                                (IBusConnection *)connection,
                                message);
}


gboolean
bus_input_context_has_focus (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    return context->has_focus;
}

void
bus_input_context_focus_in (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->has_focus)
        return;

    context->has_focus = TRUE;

    if (context->engine == NULL && context->enabled) {
        g_signal_emit (context, context_signals[REQUEST_ENGINE], 0, NULL);
    }

    if (context->engine && context->enabled) {
        bus_engine_proxy_focus_in (context->engine);
        bus_engine_proxy_enable (context->engine);
        bus_engine_proxy_set_capabilities (context->engine, context->capabilities);
        bus_engine_proxy_set_cursor_location (context->engine, context->x, context->y, context->w, context->h);
    }

    if (context->capabilities & IBUS_CAP_FOCUS) {
        g_signal_emit (context, context_signals[FOCUS_IN], 0);
        if (context->engine && context->enabled) {
          if (context->preedit_visible && !PREEDIT_CONDITION) {
                g_signal_emit (context,
                               context_signals[UPDATE_PREEDIT_TEXT],
                               0,
                               context->preedit_text,
                               context->preedit_cursor_pos,
                               context->preedit_visible);
            }
            if (context->auxiliary_visible && (context->capabilities & IBUS_CAP_AUXILIARY_TEXT) == 0) {
                g_signal_emit (context,
                               context_signals[UPDATE_AUXILIARY_TEXT],
                               0,
                               context->auxiliary_text,
                               context->auxiliary_visible);
            }
            if (context->lookup_table_visible && (context->capabilities & IBUS_CAP_LOOKUP_TABLE) == 0) {
                g_signal_emit (context,
                               context_signals[UPDATE_LOOKUP_TABLE],
                               0,
                               context->lookup_table,
                               context->lookup_table_visible);
            }
        }
    }
}

static void
bus_input_context_clear_preedit_text (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->preedit_visible &&
        context->preedit_mode == IBUS_ENGINE_PREEDIT_COMMIT) {
        bus_input_context_commit_text (context, context->preedit_text);
    }

    /* always clear preedit text */
    bus_input_context_update_preedit_text (context,
        text_empty, 0, FALSE, IBUS_ENGINE_PREEDIT_CLEAR);
}

void
bus_input_context_focus_out (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!context->has_focus)
        return;

    bus_input_context_clear_preedit_text (context);
    bus_input_context_update_auxiliary_text (context, text_empty, FALSE);
    bus_input_context_update_lookup_table (context, lookup_table_empty, FALSE);
    bus_input_context_register_properties (context, props_empty);

    if (context->engine && context->enabled) {
        bus_engine_proxy_focus_out (context->engine);
    }

    context->has_focus = FALSE;

    if (context->capabilities & IBUS_CAP_FOCUS) {
        g_signal_emit (context, context_signals[FOCUS_OUT], 0);
    }
}

#define DEFINE_FUNC(name)                                                   \
    void                                                                    \
    bus_input_context_##name (BusInputContext *context)                     \
    {                                                                       \
        g_assert (BUS_IS_INPUT_CONTEXT (context));                          \
                                                                            \
        if (context->has_focus && context->enabled && context->engine) {    \
            bus_engine_proxy_##name (context->engine);                      \
        }                                                                   \
    }

DEFINE_FUNC(page_up)
DEFINE_FUNC(page_down)
DEFINE_FUNC(cursor_up)
DEFINE_FUNC(cursor_down)

#undef DEFINE_FUNC

void
bus_input_context_candidate_clicked (BusInputContext *context,
                                     guint            index,
                                     guint            button,
                                     guint            state)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->enabled && context->engine) {
        bus_engine_proxy_candidate_clicked (context->engine,
                                            index,
                                            button,
                                            state);
    }
}

void
bus_input_context_property_activate (BusInputContext *context,
                                     const gchar     *prop_name,
                                     gint             prop_state)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->enabled && context->engine) {
        bus_engine_proxy_property_activate (context->engine, prop_name, prop_state);
    }
}

static void
bus_input_context_commit_text (BusInputContext *context,
                               IBusText        *text)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!context->enabled)
        return;

    if (text == text_empty || text == NULL)
        return;

    bus_input_context_send_signal (context,
                                   "CommitText",
                                   IBUS_TYPE_TEXT, &text,
                                   G_TYPE_INVALID);
}

static void
bus_input_context_update_preedit_text (BusInputContext *context,
                                       IBusText        *text,
                                       guint            cursor_pos,
                                       gboolean         visible,
                                       guint            mode)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->preedit_text) {
        g_object_unref (context->preedit_text);
    }

    context->preedit_text = (IBusText *) g_object_ref_sink (text ? text : text_empty);
    context->preedit_cursor_pos = cursor_pos;
    context->preedit_visible = visible;
    context->preedit_mode = mode;

    if (PREEDIT_CONDITION) {
        bus_input_context_send_signal (context,
                                       "UpdatePreeditText",
                                       IBUS_TYPE_TEXT, &(context->preedit_text),
                                       G_TYPE_UINT, &(context->preedit_cursor_pos),
                                       G_TYPE_BOOLEAN, &(context->preedit_visible),
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[UPDATE_PREEDIT_TEXT],
                       0,
                       context->preedit_text,
                       context->preedit_cursor_pos,
                       context->preedit_visible);
    }
}

static void
bus_input_context_show_preedit_text (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->preedit_visible) {
        return;
    }

    context->preedit_visible = TRUE;

    if (PREEDIT_CONDITION) {
        bus_input_context_send_signal (context,
                                       "ShowPreeditText",
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[SHOW_PREEDIT_TEXT],
                       0);
    }
}

static void
bus_input_context_hide_preedit_text (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!context->preedit_visible) {
        return;
    }

    context->preedit_visible = FALSE;

    if (PREEDIT_CONDITION) {
        bus_input_context_send_signal (context,
                                       "HidePreeditText",
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[HIDE_PREEDIT_TEXT],
                       0);
    }
}

static void
bus_input_context_update_auxiliary_text (BusInputContext *context,
                                         IBusText        *text,
                                         gboolean         visible)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->auxiliary_text) {
        g_object_unref (context->auxiliary_text);
    }

    context->auxiliary_text = (IBusText *) g_object_ref_sink (text ? text : text_empty);
    context->auxiliary_visible = visible;

    if (context->capabilities & IBUS_CAP_AUXILIARY_TEXT) {
        bus_input_context_send_signal (context,
                                       "UpdateAuxiliaryText",
                                       IBUS_TYPE_TEXT, &(context->auxiliary_text),
                                       G_TYPE_BOOLEAN, &(context->auxiliary_visible),
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[UPDATE_AUXILIARY_TEXT],
                       0,
                       context->auxiliary_text,
                       context->auxiliary_visible);
    }
}

static void
bus_input_context_show_auxiliary_text (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->auxiliary_visible) {
        return;
    }

    context->auxiliary_visible = TRUE;

    if ((context->capabilities & IBUS_CAP_AUXILIARY_TEXT) == IBUS_CAP_AUXILIARY_TEXT) {
        bus_input_context_send_signal (context,
                                       "ShowAuxiliaryText",
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[SHOW_AUXILIARY_TEXT],
                       0);
    }
}

static void
bus_input_context_hide_auxiliary_text (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!context->auxiliary_visible) {
        return;
    }

    context->auxiliary_visible = FALSE;

    if ((context->capabilities & IBUS_CAP_AUXILIARY_TEXT) == IBUS_CAP_AUXILIARY_TEXT) {
        bus_input_context_send_signal (context,
                                       "HideAuxiliaryText",
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[HIDE_AUXILIARY_TEXT],
                       0);
    }
}

static void
bus_input_context_update_lookup_table (BusInputContext *context,
                                       IBusLookupTable *table,
                                       gboolean         visible)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->lookup_table) {
        g_object_unref (context->lookup_table);
    }

    context->lookup_table = (IBusLookupTable *) g_object_ref_sink (table ? table : lookup_table_empty);
    context->lookup_table_visible = visible;

    if (context->capabilities & IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_send_signal (context,
                                       "UpdateLookupTable",
                                       IBUS_TYPE_LOOKUP_TABLE, &(context->lookup_table),
                                       G_TYPE_BOOLEAN, &(context->lookup_table_visible),
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[UPDATE_LOOKUP_TABLE],
                       0,
                       context->lookup_table,
                       context->lookup_table_visible);
    }
}

static void
bus_input_context_show_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->lookup_table_visible) {
        return;
    }

    context->lookup_table_visible = TRUE;

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_send_signal (context,
                                       "ShowLookupTable",
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[SHOW_LOOKUP_TABLE],
                       0);
    }
}

static void
bus_input_context_hide_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!context->lookup_table_visible) {
        return;
    }

    context->lookup_table_visible = FALSE;

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_send_signal (context,
                                       "HideLookupTable",
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[HIDE_LOOKUP_TABLE],
                       0);
    }
}

static void
bus_input_context_page_up_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!ibus_lookup_table_page_up (context->lookup_table)) {
        return;
    }

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_send_signal (context,
                                       "PageUpLookupTable",
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[PAGE_UP_LOOKUP_TABLE],
                       0);
    }
}

static void
bus_input_context_page_down_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!ibus_lookup_table_page_down (context->lookup_table)) {
        return;
    }

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_send_signal (context,
                                       "PageDownLookupTable",
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[PAGE_DOWN_LOOKUP_TABLE],
                       0);
    }
}

static void
bus_input_context_cursor_up_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!ibus_lookup_table_cursor_up (context->lookup_table)) {
        return;
    }

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_send_signal (context,
                                       "CursorUpLookupTable",
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[CURSOR_UP_LOOKUP_TABLE],
                       0);
    }
}

static void
bus_input_context_cursor_down_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!ibus_lookup_table_cursor_down (context->lookup_table)) {
        return;
    }

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_send_signal (context,
                                       "CursorDownLookupTable",
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[CURSOR_DOWN_LOOKUP_TABLE],
                       0);
    }
}

static void
bus_input_context_register_properties (BusInputContext *context,
                                       IBusPropList    *props)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (IBUS_IS_PROP_LIST (props));

    if (context->capabilities & IBUS_CAP_PROPERTY) {
        bus_input_context_send_signal (context,
                                       "RegisterProperties",
                                       IBUS_TYPE_PROP_LIST, &props,
                                       G_TYPE_INVALID);
    }
    else {
        g_signal_emit (context,
                       context_signals[REGISTER_PROPERTIES],
                       0,
                       props);
    }
}

static void
bus_input_context_update_property (BusInputContext *context,
                                   IBusProperty    *prop)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (IBUS_IS_PROPERTY (prop));

    if (context->capabilities & IBUS_CAP_PROPERTY) {
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

static void
_engine_destroy_cb (BusEngineProxy  *engine,
                    BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

    bus_input_context_set_engine (context, NULL);
}

static void
_engine_commit_text_cb (BusEngineProxy  *engine,
                        IBusText        *text,
                        BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (text != NULL);
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

    bus_input_context_commit_text (context, text);
}

static void
_engine_forward_key_event_cb (BusEngineProxy    *engine,
                              guint              keyval,
                              guint              keycode,
                              guint              state,
                              BusInputContext   *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

    if (!context->enabled)
        return;

    bus_input_context_send_signal (context,
                                   "ForwardKeyEvent",
                                   G_TYPE_UINT,  &keyval,
                                   G_TYPE_UINT,  &keycode,
                                   G_TYPE_UINT,  &state,
                                   G_TYPE_INVALID);
}

static void
_engine_delete_surrounding_text_cb (BusEngineProxy    *engine,
                                    gint               offset_from_cursor,
                                    guint              nchars,
                                    BusInputContext   *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

    if (!context->enabled)
        return;

    bus_input_context_send_signal (context,
                                   "DeleteSurroundingText",
                                   G_TYPE_INT,   &offset_from_cursor,
                                   G_TYPE_UINT,  &nchars,
                                   G_TYPE_INVALID);
}

static void
_engine_update_preedit_text_cb (BusEngineProxy  *engine,
                                IBusText        *text,
                                guint            cursor_pos,
                                gboolean         visible,
                                guint            mode,
                                BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (IBUS_IS_TEXT (text));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

    if (!context->enabled)
        return;

    bus_input_context_update_preedit_text (context, text, cursor_pos, visible, mode);
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

    g_assert (context->engine == engine);

    if (!context->enabled)
        return;

    bus_input_context_update_auxiliary_text (context, text, visible);
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

    g_assert (context->engine == engine);

    if (!context->enabled)
        return;

    bus_input_context_update_lookup_table (context, table, visible);
}

static void
_engine_register_properties_cb (BusEngineProxy  *engine,
                                IBusPropList    *props,
                                BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (IBUS_IS_PROP_LIST (props));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

    if (!context->enabled)
        return;

    bus_input_context_register_properties (context, props);
}

static void
_engine_update_property_cb (BusEngineProxy  *engine,
                            IBusProperty    *prop,
                            BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (IBUS_IS_PROPERTY (prop));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

    if (!context->enabled)
        return;

    bus_input_context_update_property (context, prop);
}

#define DEFINE_FUNCTION(name)                                   \
    static void                                                 \
    _engine_##name##_cb (BusEngineProxy   *engine,              \
                         BusInputContext *context)              \
    {                                                           \
        g_assert (BUS_IS_ENGINE_PROXY (engine));                \
        g_assert (BUS_IS_INPUT_CONTEXT (context));              \
                                                                \
        g_assert (context->engine == engine);                   \
                                                                \
        if (!context->enabled)                                  \
            return;                                             \
                                                                \
        bus_input_context_##name (context);                     \
    }

DEFINE_FUNCTION (show_preedit_text)
DEFINE_FUNCTION (hide_preedit_text)
DEFINE_FUNCTION (show_auxiliary_text)
DEFINE_FUNCTION (hide_auxiliary_text)
DEFINE_FUNCTION (show_lookup_table)
DEFINE_FUNCTION (hide_lookup_table)
DEFINE_FUNCTION (page_up_lookup_table)
DEFINE_FUNCTION (page_down_lookup_table)
DEFINE_FUNCTION (cursor_up_lookup_table)
DEFINE_FUNCTION (cursor_down_lookup_table)
#undef DEFINE_FUNCTION

void
bus_input_context_enable (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!context->has_focus) {
        context->enabled = TRUE;
        /* TODO: Do we need to emit "enabled" signal? */
        return;
    }

    if (context->engine == NULL) {
        g_signal_emit (context, context_signals[REQUEST_ENGINE], 0, NULL);
    }

    if (context->engine == NULL)
        return;

    context->enabled = TRUE;

    bus_engine_proxy_enable (context->engine);
    bus_engine_proxy_focus_in (context->engine);
    bus_engine_proxy_set_capabilities (context->engine, context->capabilities);
    bus_engine_proxy_set_cursor_location (context->engine, context->x, context->y, context->w, context->h);

    bus_input_context_send_signal (context,
                                   "Enabled",
                                   G_TYPE_INVALID);
    g_signal_emit (context,
                   context_signals[ENABLED],
                   0);
}

void
bus_input_context_disable (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    bus_input_context_clear_preedit_text (context);
    bus_input_context_update_auxiliary_text (context, text_empty, FALSE);
    bus_input_context_update_lookup_table (context, lookup_table_empty, FALSE);
    bus_input_context_register_properties (context, props_empty);

    if (context->engine) {
        bus_engine_proxy_focus_out (context->engine);
        bus_engine_proxy_disable (context->engine);
    }

    bus_input_context_send_signal (context,
                                   "Disabled",
                                   G_TYPE_INVALID);
    g_signal_emit (context,
                   context_signals[DISABLED],
                   0);

    context->enabled = FALSE;
}

gboolean
bus_input_context_is_enabled (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    return context->enabled;
}

const static struct {
    const gchar *name;
    GCallback    callback;
} signals [] = {
    { "commit-text",            G_CALLBACK (_engine_commit_text_cb) },
    { "forward-key-event",      G_CALLBACK (_engine_forward_key_event_cb) },
    { "delete-surrounding-text", G_CALLBACK (_engine_delete_surrounding_text_cb) },
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

    bus_input_context_clear_preedit_text (context);
    bus_input_context_update_auxiliary_text (context, text_empty, FALSE);
    bus_input_context_update_lookup_table (context, lookup_table_empty, FALSE);
    bus_input_context_register_properties (context, props_empty);

    if (context->engine) {
        gint i;
        for (i = 0; signals[i].name != NULL; i++) {
            g_signal_handlers_disconnect_by_func (context->engine, signals[i].callback, context);
        }
        /* Do not destroy the engine anymore, because of global engine feature */
        g_object_unref (context->engine);
        context->engine = NULL;
    }
}

void
bus_input_context_set_engine (BusInputContext *context,
                              BusEngineProxy  *engine)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->engine == engine)
        return;

    if (context->engine != NULL) {
        bus_input_context_unset_engine (context);
    }

    if (engine == NULL) {
        bus_input_context_disable (context);
    }
    else {
        gint i;
        context->engine = engine;
        g_object_ref_sink (context->engine);

        for (i = 0; signals[i].name != NULL; i++) {
            g_signal_connect (context->engine,
                              signals[i].name,
                              signals[i].callback,
                              context);
        }
        if (context->has_focus && context->enabled) {
            bus_engine_proxy_focus_in (context->engine);
            bus_engine_proxy_enable (context->engine);
            bus_engine_proxy_set_capabilities (context->engine, context->capabilities);
            bus_engine_proxy_set_cursor_location (context->engine, context->x, context->y, context->w, context->h);
        }
    }
    g_signal_emit (context,
                   context_signals[ENGINE_CHANGED],
                   0);
}

BusEngineProxy *
bus_input_context_get_engine (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    return context->engine;
}

static gboolean
bus_input_context_filter_keyboard_shortcuts (BusInputContext    *context,
                                             guint               keyval,
                                             guint               keycode,
                                             guint               modifiers)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    gboolean retval = FALSE;

    if (context->filter_release){
        if(modifiers & IBUS_RELEASE_MASK) {
            /* filter release key event */
            return TRUE;
        }
        else {
            /* stop filter release key event */
            context->filter_release = FALSE;
        }
    }

    if (keycode != 0 && !BUS_DEFAULT_IBUS->use_sys_layout) {
        IBusKeymap *keymap = BUS_DEFAULT_KEYMAP;
        if (keymap != NULL) {
            guint tmp = ibus_keymap_lookup_keysym (keymap,
                                                 keycode,
                                                 modifiers);
            if (tmp != IBUS_VoidSymbol)
                keyval = tmp;
        }
    }

    retval = bus_ibus_impl_filter_keyboard_shortcuts (BUS_DEFAULT_IBUS,
                                                      context,
                                                      keyval,
                                                      modifiers,
                                                      context->prev_keyval,
                                                      context->prev_modifiers);
    context->prev_keyval = keyval;
    context->prev_modifiers = modifiers;

    if (retval == TRUE) {
        /* begin filter release key event */
        context->filter_release = TRUE;
    }

    return retval;
}


static gboolean
bus_input_context_send_signal (BusInputContext *context,
                               const gchar     *signal_name,
                               GType            first_arg_type,
                               ...)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (signal_name != NULL);
    g_assert (context->connection != NULL);

    va_list args;
    gboolean retval;
    IBusMessage *message;

    message = ibus_message_new_signal (ibus_service_get_path ((IBusService *)context),
                                       IBUS_INTERFACE_INPUT_CONTEXT,
                                       signal_name);

    ibus_message_set_sender (message, IBUS_SERVICE_IBUS);

    va_start (args, first_arg_type);
    ibus_message_append_args_valist (message, first_arg_type, args);
    va_end (args);

    retval = ibus_connection_send ((IBusConnection *)context->connection, message);
    ibus_message_unref (message);

    return retval;
}

guint
bus_input_context_get_capabilities (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    return context->capabilities;
}
