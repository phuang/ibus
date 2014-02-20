/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2013 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2013 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
#include "inputcontext.h"

#include <string.h>

#include "engineproxy.h"
#include "factoryproxy.h"
#include "global.h"
#include "ibusimpl.h"
#include "marshalers.h"
#include "types.h"

struct _SetEngineByDescData {
    /* context related to the data */
    BusInputContext *context;
    /* set engine by desc result, cancellable */
    GSimpleAsyncResult *simple;
    /* a object to cancel bus_engine_proxy_new call */
    GCancellable *cancellable;
    /* a object being passed to the bus_input_context_set_engine_by_desc function. if origin_cancellable is cancelled by someone,
     * we cancel the cancellable above as well. */
    GCancellable *origin_cancellable;
    gulong cancelled_handler_id;
};
typedef struct _SetEngineByDescData SetEngineByDescData;

struct _BusInputContext {
    IBusService parent;

    /* instance members */
    BusConnection *connection;
    BusEngineProxy *engine;
    gchar *client;

    gboolean has_focus;

    /* client capabilities */
    guint capabilities;

    /* cursor location */
    gint x;
    gint y;
    gint w;
    gint h;

    /* prev key event that are used for handling hot-keys */
    guint prev_keyval;
    guint prev_modifiers;

    /* preedit text */
    IBusText *preedit_text;
    guint     preedit_cursor_pos;
    gboolean  preedit_visible;
    guint     preedit_mode;

    /* auxiliary text */
    IBusText *auxiliary_text;
    gboolean  auxiliary_visible;

    /* lookup table */
    IBusLookupTable *lookup_table;
    gboolean lookup_table_visible;

    /* filter release */
    gboolean filter_release;

    /* is fake context */
    gboolean fake;

    /* incompleted set engine by desc request */
    SetEngineByDescData *data;

    /* content-type (primary purpose and hints) */
    guint    purpose;
    guint    hints;
};

struct _BusInputContextClass {
    IBusServiceClass parent;

    /* class members */
    IBusEngineDesc *default_engine_desc;
};

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
static void     bus_input_context_destroy
                                   (BusInputContext       *context);
static void     bus_input_context_service_method_call
                                   (IBusService           *service,
                                    GDBusConnection       *connection,
                                    const gchar           *sender,
                                    const gchar           *object_path,
                                    const gchar           *interface_name,
                                    const gchar           *method_name,
                                    GVariant              *parameters,
                                    GDBusMethodInvocation *invocation);
static gboolean bus_input_context_service_set_property
                                   (IBusService           *service,
                                    GDBusConnection       *connection,
                                    const gchar           *sender,
                                    const gchar           *object_path,
                                    const gchar           *interface_name,
                                    const gchar           *property_name,
                                    GVariant              *value,
                                    GError               **error);
static void     bus_input_context_unset_engine
                                   (BusInputContext       *context);
static void     bus_input_context_commit_text
                                   (BusInputContext       *context,
                                    IBusText              *text);
static void     bus_input_context_update_preedit_text
                                   (BusInputContext       *context,
                                    IBusText              *text,
                                    guint                  cursor_pos,
                                    gboolean               visible,
                                    guint                  mode);
static void     bus_input_context_show_preedit_text
                                   (BusInputContext       *context);
static void     bus_input_context_hide_preedit_text
                                   (BusInputContext       *context);
static void     bus_input_context_update_auxiliary_text
                                   (BusInputContext       *context,
                                    IBusText              *text,
                                    gboolean               visible);
static void     bus_input_context_show_auxiliary_text
                                   (BusInputContext       *context);
static void     bus_input_context_hide_auxiliary_text
                                   (BusInputContext       *context);
static void     bus_input_context_update_lookup_table
                                   (BusInputContext       *context,
                                    IBusLookupTable       *table,
                                    gboolean               visible);
static void     bus_input_context_show_lookup_table
                                   (BusInputContext       *context);
static void     bus_input_context_hide_lookup_table
                                   (BusInputContext       *context);
static void     bus_input_context_page_up_lookup_table
                                   (BusInputContext       *context);
static void     bus_input_context_page_down_lookup_table
                                   (BusInputContext       *context);
static void     bus_input_context_cursor_up_lookup_table
                                   (BusInputContext       *context);
static void     bus_input_context_cursor_down_lookup_table
                                   (BusInputContext       *context);
static void     bus_input_context_register_properties
                                   (BusInputContext       *context,
                                    IBusPropList          *props);
static void     bus_input_context_update_property
                                   (BusInputContext       *context,
                                    IBusProperty          *prop);
static void     _engine_destroy_cb (BusEngineProxy        *factory,
                                    BusInputContext       *context);

static IBusText *text_empty = NULL;
static IBusLookupTable *lookup_table_empty = NULL;
static IBusPropList    *props_empty = NULL;

/* The interfaces available in this class, which consists of a list of
 * methods this class implements and a list of signals this class may
 * emit. Method calls to the interface that are not defined in this
 * XML will be automatically rejected by the GDBus library (see
 * src/ibusservice.c for details.) */
static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='org.freedesktop.IBus.InputContext'>"
    /* methods */
    "    <method name='ProcessKeyEvent'>"
    "      <arg direction='in'  type='u' name='keyval' />"
    "      <arg direction='in'  type='u' name='keycode' />"
    "      <arg direction='in'  type='u' name='state' />"
    "      <arg direction='out' type='b' name='handled' />"
    "    </method>"
    "    <method name='SetCursorLocation'>"
    "      <arg direction='in' type='i' name='x' />"
    "      <arg direction='in' type='i' name='y' />"
    "      <arg direction='in' type='i' name='w' />"
    "      <arg direction='in' type='i' name='h' />"
    "    </method>"
    "    <method name='ProcessHandWritingEvent'>"
    "      <arg direction='in' type='ad' name='coordinates' />"
    "    </method>"
    "    <method name='CancelHandWriting'>"
    "      <arg direction='in' type='u' name='n_strokes' />"
    "    </method>"
    "    <method name='FocusIn' />"
    "    <method name='FocusOut' />"
    "    <method name='Reset' />"
    "    <method name='SetCapabilities'>"
    "      <arg direction='in' type='u' name='caps' />"
    "    </method>"
    "    <method name='PropertyActivate'>"
    "      <arg direction='in' type='s' name='name' />"
    "      <arg direction='in' type='u' name='state' />"
    "    </method>"
    "    <method name='SetEngine'>"
    "      <arg direction='in' type='s' name='name' />"
    "    </method>"
    "    <method name='GetEngine'>"
    "      <arg direction='out' type='v' name='desc' />"
    "    </method>"
    "    <method name='SetSurroundingText'>"
    "      <arg direction='in' type='v' name='text' />"
    "      <arg direction='in' type='u' name='cursor_pos' />"
    "      <arg direction='in' type='u' name='anchor_pos' />"
    "    </method>"

    /* signals */
    "    <signal name='CommitText'>"
    "      <arg type='v' name='text' />"
    "    </signal>"
    "    <signal name='ForwardKeyEvent'>"
    "      <arg type='u' name='keyval' />"
    "      <arg type='u' name='keycode' />"
    "      <arg type='u' name='state' />"
    "    </signal>"
    "    <signal name='UpdatePreeditText'>"
    "      <arg type='v' name='text' />"
    "      <arg type='u' name='cursor_pos' />"
    "      <arg type='b' name='visible' />"
    "    </signal>"
    "    <signal name='ShowPreeditText'/>"
    "    <signal name='HidePreeditText'/>"
    "    <signal name='UpdateAuxiliaryText'>"
    "      <arg type='v' name='text' />"
    "      <arg type='b' name='visible' />"
    "    </signal>"
    "    <signal name='ShowAuxiliaryText'/>"
    "    <signal name='HideAuxiliaryText'/>"
    "    <signal name='UpdateLookupTable'>"
    "      <arg type='v' name='table' />"
    "      <arg type='b' name='visible' />"
    "    </signal>"
    "    <signal name='ShowLookupTable'/>"
    "    <signal name='HideLookupTable'/>"
    "    <signal name='PageUpLookupTable'/>"
    "    <signal name='PageDownLookupTable'/>"
    "    <signal name='CursorUpLookupTable'/>"
    "    <signal name='CursorDownLookupTable'/>"
    "    <signal name='RegisterProperties'>"
    "      <arg type='v' name='props' />"
    "    </signal>"
    "    <signal name='UpdateProperty'>"
    "      <arg type='v' name='prop' />"
    "    </signal>"

    /* properties */
    "    <property name='ContentType' type='(uu)' access='write' />"
    "  </interface>"
    "</node>";

G_DEFINE_TYPE (BusInputContext, bus_input_context, IBUS_TYPE_SERVICE)

/* TRUE if we can send preedit text to client. FALSE if the panel has to handle it. Note that we check IBUS_CAP_FOCUS here since
 * when the capability is not set, the client has to handle a preedit text regardless of the embed_preedit_text config. */
#define PREEDIT_CONDITION  \
    ((context->capabilities & IBUS_CAP_PREEDIT_TEXT) && \
     (bus_ibus_impl_is_embed_preedit_text (BUS_DEFAULT_IBUS) || (context->capabilities & IBUS_CAP_FOCUS) == 0))

static void
_connection_destroy_cb (BusConnection   *connection,
                        BusInputContext *context)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    ibus_object_destroy (IBUS_OBJECT (context));
}

static void
bus_input_context_class_init (BusInputContextClass *class)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (class);

    class->default_engine_desc = ibus_engine_desc_new ("dummy",
                                                       "",
                                                       "",
                                                       "",
                                                       "",
                                                       "",
                                                       "ibus-engine",
                                                       "");
    g_object_ref_sink (class->default_engine_desc);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_input_context_destroy;

    /* override the parent class's implementation. */
    IBUS_SERVICE_CLASS (class)->service_method_call =
        bus_input_context_service_method_call;
    IBUS_SERVICE_CLASS (class)->service_set_property =
        bus_input_context_service_set_property;
    /* register the xml so that bus_ibus_impl_service_method_call will be called on a method call defined in the xml (e.g. 'FocusIn'.) */
    ibus_service_class_add_interfaces (IBUS_SERVICE_CLASS (class), introspection_xml);

    /* install glib signals that would be handled by other classes like ibusimpl.c and panelproxy.c. */
    context_signals[PROCESS_KEY_EVENT] =
        g_signal_new (I_("process-key-event"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_BOOL__UINT_UINT_UINT,
            G_TYPE_BOOLEAN,
            3,
            G_TYPE_UINT,
            G_TYPE_UINT,
            G_TYPE_UINT);

    context_signals[SET_CURSOR_LOCATION] =
        g_signal_new (I_("set-cursor-location"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__INT_INT_INT_INT,
            G_TYPE_NONE,
            4,
            G_TYPE_INT,
            G_TYPE_INT,
            G_TYPE_INT,
            G_TYPE_INT);

    context_signals[FOCUS_IN] =
        g_signal_new (I_("focus-in"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[FOCUS_OUT] =
        g_signal_new (I_("focus-out"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[UPDATE_PREEDIT_TEXT] =
        g_signal_new (I_("update-preedit-text"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__OBJECT_UINT_BOOLEAN,
            G_TYPE_NONE,
            3,
            IBUS_TYPE_TEXT,
            G_TYPE_UINT,
            G_TYPE_BOOLEAN);

    context_signals[SHOW_PREEDIT_TEXT] =
        g_signal_new (I_("show-preedit-text"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[HIDE_PREEDIT_TEXT] =
        g_signal_new (I_("hide-preedit-text"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[UPDATE_AUXILIARY_TEXT] =
        g_signal_new (I_("update-auxiliary-text"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__OBJECT_BOOLEAN,
            G_TYPE_NONE,
            2,
            IBUS_TYPE_TEXT,
            G_TYPE_BOOLEAN);

    context_signals[SHOW_AUXILIARY_TEXT] =
        g_signal_new (I_("show-auxiliary-text"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[HIDE_AUXILIARY_TEXT] =
        g_signal_new (I_("hide-auxiliary-text"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    context_signals[UPDATE_LOOKUP_TABLE] =
        g_signal_new (I_("update-lookup-table"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__OBJECT_BOOLEAN,
            G_TYPE_NONE,
            2,
            IBUS_TYPE_LOOKUP_TABLE,
            G_TYPE_BOOLEAN);

    context_signals[SHOW_LOOKUP_TABLE] =
        g_signal_new (I_("show-lookup-table"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[HIDE_LOOKUP_TABLE] =
        g_signal_new (I_("hide-lookup-table"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[PAGE_UP_LOOKUP_TABLE] =
        g_signal_new (I_("page-up-lookup-table"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[PAGE_DOWN_LOOKUP_TABLE] =
        g_signal_new (I_("page-down-lookup-table"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[CURSOR_UP_LOOKUP_TABLE] =
        g_signal_new (I_("cursor-up-lookup-table"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[CURSOR_DOWN_LOOKUP_TABLE] =
        g_signal_new (I_("cursor-down-lookup-table"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[REGISTER_PROPERTIES] =
        g_signal_new (I_("register-properties"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__OBJECT,
            G_TYPE_NONE,
            1,
            IBUS_TYPE_PROP_LIST);

    context_signals[UPDATE_PROPERTY] =
        g_signal_new (I_("update-property"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__OBJECT,
            G_TYPE_NONE,
            1,
            IBUS_TYPE_PROPERTY);

    context_signals[ENGINE_CHANGED] =
        g_signal_new (I_("engine-changed"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /* This signal is not for notifying an event on this object, but is for requesting an engine as the name shows.
     * On the signal emission, ibusimpl.c will immediately update the context->engine variable. */
    context_signals[REQUEST_ENGINE] =
        g_signal_new (I_("request-engine"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_OBJECT__STRING,
            IBUS_TYPE_ENGINE_DESC,
            1,
            G_TYPE_STRING);

    text_empty = ibus_text_new_from_string ("");
    g_object_ref_sink (text_empty);
    lookup_table_empty = ibus_lookup_table_new (9 /* page size */, 0, FALSE, FALSE);
    g_object_ref_sink (lookup_table_empty);
    props_empty = ibus_prop_list_new ();
    g_object_ref_sink (props_empty);
}

static void
bus_input_context_init (BusInputContext *context)
{
    context->prev_keyval = IBUS_KEY_VoidSymbol;
    g_object_ref_sink (text_empty);
    context->preedit_text = text_empty;
    context->preedit_mode = IBUS_ENGINE_PREEDIT_CLEAR;
    g_object_ref_sink (text_empty);
    context->auxiliary_text = text_empty;
    g_object_ref_sink (lookup_table_empty);
    context->lookup_table = lookup_table_empty;
    /* other member variables will automatically be zero-cleared. */
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

    IBUS_OBJECT_CLASS (bus_input_context_parent_class)->destroy (IBUS_OBJECT (context));
}

static gboolean
bus_input_context_send_signal (BusInputContext *context,
                               const gchar     *interface_name,
                               const gchar     *signal_name,
                               GVariant        *parameters,
                               GError         **error)
{
    if (context->connection == NULL)
        return TRUE;

    GDBusMessage *message = g_dbus_message_new_signal (ibus_service_get_object_path ((IBusService *)context),
                                                       interface_name,
                                                       signal_name);
    g_dbus_message_set_sender (message, "org.freedesktop.IBus");
    g_dbus_message_set_destination (message, bus_connection_get_unique_name (context->connection));
    if (parameters != NULL)
        g_dbus_message_set_body (message, parameters);

    gboolean retval =  g_dbus_connection_send_message (bus_connection_get_dbus_connection (context->connection),
                                                       message,
                                                       G_DBUS_SEND_MESSAGE_FLAGS_NONE,
                                                       NULL, error);
    g_object_unref (message);
    return retval;
}

/**
 * bus_input_context_emit_signal:
 * @signal_name: The D-Bus signal name to emit which is in the introspection_xml.
 *
 * Emit the D-Bus signal.
 */
static gboolean
bus_input_context_emit_signal (BusInputContext *context,
                               const gchar     *signal_name,
                               GVariant        *parameters,
                               GError         **error)
{
    if (context->connection == NULL)
        return TRUE;

    return bus_input_context_send_signal (context,
                                          "org.freedesktop.IBus.InputContext",
                                          signal_name,
                                          parameters,
                                          error);
}

/**
 * bus_input_context_property_changed:
 * @context: a #BusInputContext
 * @property_name: The D-Bus property name which has changed
 * @value: The new value of the property
 *
 * Emit the D-Bus "PropertiesChanged" signal for a property.
 * Returns: %TRUE on success, %FALSE on failure
 */
static gboolean
bus_input_context_property_changed (BusInputContext *context,
                                    const gchar     *property_name,
                                    GVariant        *value,
                                    GError         **error)
{
    if (context->connection == NULL)
        return TRUE;

    GVariantBuilder *builder = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add (builder, "{sv}", property_name, value);
    return bus_input_context_send_signal (context,
                                          "org.freedesktop.DBus.Properties",
                                          "PropertiesChanged",
                                          g_variant_new ("(sa{sv}as)",
                                                         "org.freedesktop.IBus",
                                                         builder,
                                                         NULL),
                                          error);
}

/**
 * _ic_process_key_event_reply_cb:
 *
 * A GAsyncReadyCallback function to be called when bus_engine_proxy_process_key_event() is finished.
 */
static void
_ic_process_key_event_reply_cb (GObject               *source,
                                GAsyncResult          *res,
                                GDBusMethodInvocation *invocation)
{
    GError *error = NULL;
    GVariant *value = g_dbus_proxy_call_finish ((GDBusProxy *)source,
                                                 res,
                                                 &error);
    if (value != NULL) {
        g_dbus_method_invocation_return_value (invocation, value);
        g_variant_unref (value);
    }
    else {
        g_dbus_method_invocation_return_gerror (invocation, error);
        g_error_free (error);
    }
}

/**
 * _ic_process_key_event:
 *
 * Implement the "ProcessKeyEvent" method call of the org.freedesktop.IBus.InputContext interface.
 */
static void
_ic_process_key_event  (BusInputContext       *context,
                        GVariant              *parameters,
                        GDBusMethodInvocation *invocation)
{
    guint keyval = IBUS_KEY_VoidSymbol;
    guint keycode = 0;
    guint modifiers = 0;

    g_variant_get (parameters, "(uuu)", &keyval, &keycode, &modifiers);
    if (G_UNLIKELY (!context->has_focus)) {
        /* workaround: set focus if context does not have focus */
        BusInputContext *focused_context = bus_ibus_impl_get_focused_input_context (BUS_DEFAULT_IBUS);
        if (focused_context == NULL ||
            focused_context->fake == TRUE ||
            context->fake == FALSE) {
            /* grab focus, if context is a real IC or current focused IC is fake */
            bus_input_context_focus_in (context);
        }
    }

    /* ignore key events, if it is a fake input context */
    if (context->has_focus && context->engine && context->fake == FALSE) {
        bus_engine_proxy_process_key_event (context->engine,
                                            keyval,
                                            keycode,
                                            modifiers,
                                            (GAsyncReadyCallback) _ic_process_key_event_reply_cb,
                                            invocation);
    }
    else {
        g_dbus_method_invocation_return_value (invocation, g_variant_new ("(b)", FALSE));
    }
}

/**
 * _ic_set_cursor_location:
 *
 * Implement the "SetCursorLocation" method call of the org.freedesktop.IBus.InputContext interface.
 */
static void
_ic_set_cursor_location (BusInputContext       *context,
                         GVariant              *parameters,
                         GDBusMethodInvocation *invocation)
{
    g_dbus_method_invocation_return_value (invocation, NULL);

    g_variant_get (parameters, "(iiii)",
                   &context->x, &context->y, &context->w, &context->h);

    if (context->has_focus && context->engine) {
        bus_engine_proxy_set_cursor_location (context->engine,
                        context->x, context->y, context->w, context->h);
    }

    if (context->capabilities & IBUS_CAP_FOCUS) {
        g_signal_emit (context,
                       context_signals[SET_CURSOR_LOCATION],
                       0,
                       context->x,
                       context->y,
                       context->w,
                       context->h);
    }
}

static void
_ic_process_hand_writing_event (BusInputContext       *context,
                                GVariant              *parameters,
                                GDBusMethodInvocation *invocation)
{
    /* do nothing if it is a fake input context */
    if (context->has_focus &&
        context->engine && context->fake == FALSE) {
        bus_engine_proxy_process_hand_writing_event (context->engine, parameters);
    }
    g_dbus_method_invocation_return_value (invocation, NULL);
}

static void
_ic_cancel_hand_writing (BusInputContext       *context,
                         GVariant              *parameters,
                         GDBusMethodInvocation *invocation)
{
    guint n_strokes = 0;
    g_variant_get (parameters, "(u)", &n_strokes);

    /* do nothing if it is a fake input context */
    if (context->has_focus &&
        context->engine && context->fake == FALSE) {
        bus_engine_proxy_cancel_hand_writing (context->engine, n_strokes);
    }
    g_dbus_method_invocation_return_value (invocation, NULL);
}

/**
 * _ic_focus_in:
 *
 * Implement the "FocusIn" method call of the org.freedesktop.IBus.InputContext interface.
 */
static void
_ic_focus_in (BusInputContext       *context,
              GVariant              *parameters,
              GDBusMethodInvocation *invocation)
{
    if (context->capabilities & IBUS_CAP_FOCUS) {
        bus_input_context_focus_in (context);
        g_dbus_method_invocation_return_value (invocation, NULL);
    }
    else {
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "The input context does not support focus.");
    }
}

/**
 * _ic_focus_out:
 *
 * Implement the "FocusOut" method call of the org.freedesktop.IBus.InputContext interface.
 */
static void
_ic_focus_out (BusInputContext       *context,
               GVariant              *parameters,
               GDBusMethodInvocation *invocation)
{
    if (context->capabilities & IBUS_CAP_FOCUS) {
        bus_input_context_focus_out (context);
        g_dbus_method_invocation_return_value (invocation, NULL);
    }
    else {
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "The input context does not support focus.");
    }
}

/**
 * _ic_reset:
 *
 * Implement the "Reset" method call of the org.freedesktop.IBus.InputContext interface.
 */
static void
_ic_reset (BusInputContext       *context,
           GVariant              *parameters,
           GDBusMethodInvocation *invocation)
{
    if (context->engine) {
        bus_engine_proxy_reset (context->engine);
    }
    g_dbus_method_invocation_return_value (invocation, NULL);
}

/**
 * _ic_set_capabilities:
 *
 * Implement the "SetCapabilities" method call of the org.freedesktop.IBus.InputContext interface.
 */
static void
_ic_set_capabilities (BusInputContext       *context,
                      GVariant              *parameters,
                      GDBusMethodInvocation *invocation)
{
    guint caps = 0;
    g_variant_get (parameters, "(u)", &caps);

    bus_input_context_set_capabilities (context, caps);

    g_dbus_method_invocation_return_value (invocation, NULL);
}

/**
 * _ic_property_activate:
 *
 * Implement the "PropertyActivate" method call of the org.freedesktop.IBus.InputContext interface.
 */
static void
_ic_property_activate (BusInputContext       *context,
                       GVariant              *parameters,
                       GDBusMethodInvocation *invocation)
{
    gchar *prop_name = NULL;
    guint prop_state = 0;
    g_variant_get (parameters, "(&su)", &prop_name, &prop_state);

    if (context->engine) {
        bus_engine_proxy_property_activate (context->engine, prop_name, prop_state);
    }

#ifdef OS_CHROMEOS
    /* Global engine is always enabled in chromeos,
     * so pass PropertyActivate signal to the focused context.
     */
    else if (context->fake) {
        BusInputContext *focused_context = bus_ibus_impl_get_focused_input_context (BUS_DEFAULT_IBUS);
        if (focused_context && focused_context->engine)
            bus_engine_proxy_property_activate (focused_context->engine, prop_name, prop_state);
    }
#endif

    g_dbus_method_invocation_return_value (invocation, NULL);
}

static void
_ic_set_engine_done (BusInputContext       *context,
                     GAsyncResult          *res,
                     GDBusMethodInvocation *invocation)
{
    gboolean retval = FALSE;
    GError *error = NULL;

    retval = bus_input_context_set_engine_by_desc_finish (context,
                    res, &error);

    if (!retval) {
        g_dbus_method_invocation_return_gerror (invocation, error);
        g_error_free (error);
    }
    else {
        g_dbus_method_invocation_return_value (invocation, NULL);
    }
}

/**
 * _ic_set_engine:
 *
 * Implement the "SetEngine" method call of the org.freedesktop.IBus.InputContext interface.
 */
static void
_ic_set_engine (BusInputContext       *context,
                GVariant              *parameters,
                GDBusMethodInvocation *invocation)
{
    gchar *engine_name = NULL;
    g_variant_get (parameters, "(&s)", &engine_name);

    if (!bus_input_context_has_focus (context)) {
        g_dbus_method_invocation_return_error (invocation,
                G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                "Context which does not has focus can not change engine to %s.",
                engine_name);
        return;
    }

    IBusEngineDesc *desc = NULL;
    g_signal_emit (context,
                   context_signals[REQUEST_ENGINE], 0,
                   engine_name,
                   &desc);
    if (desc == NULL) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "Can not find engine %s.", engine_name);
        return;
    }

    bus_input_context_set_engine_by_desc (context,
                            desc,
                            g_gdbus_timeout,
                            NULL,
                            (GAsyncReadyCallback)_ic_set_engine_done,
                            invocation);
}

/**
 * _ic_get_engine:
 *
 * Implement the "GetEngine" method call of the org.freedesktop.IBus.InputContext interface.
 */
static void
_ic_get_engine (BusInputContext       *context,
                GVariant              *parameters,
                GDBusMethodInvocation *invocation)
{
    IBusEngineDesc *desc = context->engine ?
            bus_engine_proxy_get_desc (context->engine) :
            BUS_INPUT_CONTEXT_GET_CLASS (context)->default_engine_desc;


    g_dbus_method_invocation_return_value (invocation,
            g_variant_new ("(v)", ibus_serializable_serialize ((IBusSerializable *)desc)));
}

static void
_ic_set_surrounding_text (BusInputContext       *context,
                          GVariant              *parameters,
                          GDBusMethodInvocation *invocation)
{
    GVariant *variant = NULL;
    IBusText *text;
    guint cursor_pos = 0;
    guint anchor_pos = 0;

    g_variant_get (parameters,
                   "(vuu)",
                   &variant,
                   &cursor_pos,
                   &anchor_pos);
    text = IBUS_TEXT (ibus_serializable_deserialize (variant));
    g_variant_unref (variant);

    if ((context->capabilities & IBUS_CAP_SURROUNDING_TEXT) &&
         context->has_focus && context->engine) {
        bus_engine_proxy_set_surrounding_text (context->engine,
                                               text,
                                               cursor_pos,
                                               anchor_pos);
    }

    if (g_object_is_floating (text))
        g_object_unref (text);

    g_dbus_method_invocation_return_value (invocation, NULL);
}

/**
 * bus_input_context_service_method_call:
 *
 * Handle a D-Bus method call whose destination and interface name are both "org.freedesktop.IBus.InputContext"
 */
static void
bus_input_context_service_method_call (IBusService            *service,
                                       GDBusConnection        *connection,
                                       const gchar            *sender,
                                       const gchar            *object_path,
                                       const gchar            *interface_name,
                                       const gchar            *method_name,
                                       GVariant               *parameters,
                                       GDBusMethodInvocation  *invocation)
{
    if (g_strcmp0 (interface_name, IBUS_INTERFACE_INPUT_CONTEXT) != 0) {
        IBUS_SERVICE_CLASS (bus_input_context_parent_class)->service_method_call (
                        service,
                        connection,
                        sender,
                        object_path,
                        interface_name,
                        method_name,
                        parameters,
                        invocation);
        return;
    }

    static const struct {
        const gchar *method_name;
        void (* method_callback) (BusInputContext *, GVariant *, GDBusMethodInvocation *);
    } methods [] =  {
        { "ProcessKeyEvent",   _ic_process_key_event },
        { "SetCursorLocation", _ic_set_cursor_location },
        { "ProcessHandWritingEvent",
                               _ic_process_hand_writing_event },
        { "CancelHandWriting", _ic_cancel_hand_writing },
        { "FocusIn",           _ic_focus_in },
        { "FocusOut",          _ic_focus_out },
        { "Reset",             _ic_reset },
        { "SetCapabilities",   _ic_set_capabilities },
        { "PropertyActivate",  _ic_property_activate },
        { "SetEngine",         _ic_set_engine },
        { "GetEngine",         _ic_get_engine },
        { "SetSurroundingText", _ic_set_surrounding_text }
    };

    gint i;
    for (i = 0; i < G_N_ELEMENTS (methods); i++) {
        if (g_strcmp0 (method_name, methods[i].method_name) == 0) {
            methods[i].method_callback ((BusInputContext *)service, parameters, invocation);
            return;
        }
    }

    g_return_if_reached ();
}

static gboolean
bus_input_context_service_set_property (IBusService     *service,
                                        GDBusConnection *connection,
                                        const gchar     *sender,
                                        const gchar     *object_path,
                                        const gchar     *interface_name,
                                        const gchar     *property_name,
                                        GVariant        *value,
                                        GError         **error)
{
    if (g_strcmp0 (interface_name, IBUS_INTERFACE_INPUT_CONTEXT) != 0) {
        return IBUS_SERVICE_CLASS (bus_input_context_parent_class)->
            service_set_property (service,
                                  connection,
                                  sender,
                                  object_path,
                                  interface_name,
                                  property_name,
                                  value,
                                  error);
    }

    if (g_strcmp0 (property_name, "ContentType") == 0) {
        BusInputContext *context = (BusInputContext *) service;
        guint purpose = 0;
        guint hints = 0;

        g_variant_get (value, "(uu)", &purpose, &hints);
        if (purpose != context->purpose || hints != context->hints) {
            GError *error;
            gboolean retval;

            context->purpose = purpose;
            context->hints = hints;

            if (context->has_focus && context->engine)
                bus_engine_proxy_set_content_type (context->engine,
                                                   purpose,
                                                   hints);

            error = NULL;
            retval = bus_input_context_property_changed (context,
                                                         "ContentType",
                                                         value,
                                                         &error);
            if (!retval) {
                g_warning ("Failed to emit PropertiesChanged signal: %s",
                           error->message);
                g_error_free (error);
            }
        }
        return TRUE;
    }

    g_return_val_if_reached (FALSE);
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

    /* To make sure that we won't use an old value left before we losing focus
     * last time. */
    context->prev_keyval = IBUS_KEY_VoidSymbol;
    context->prev_modifiers = 0;

    if (context->engine) {
        bus_engine_proxy_focus_in (context->engine);
        bus_engine_proxy_enable (context->engine);
        bus_engine_proxy_set_capabilities (context->engine, context->capabilities);
        bus_engine_proxy_set_cursor_location (context->engine, context->x, context->y, context->w, context->h);
        bus_engine_proxy_set_content_type (context->engine, context->purpose, context->hints);
    }

    if (context->capabilities & IBUS_CAP_FOCUS) {
        g_signal_emit (context, context_signals[FOCUS_IN], 0);
        if (context->engine) {
            /* if necessary, emit glib signals to the context object to update panel status. see the comment for PREEDIT_CONDITION
             * for details. */
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

/**
 * bus_input_context_clear_preedit_text:
 *
 * Clear context->preedit_text. If the preedit mode is IBUS_ENGINE_PREEDIT_COMMIT, commit it before clearing.
 */
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

    if (context->engine) {
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
        if (context->has_focus && context->engine) {    \
            bus_engine_proxy_##name (context->engine);                      \
        }                                                                   \
    }

DEFINE_FUNC (page_up)
DEFINE_FUNC (page_down)
DEFINE_FUNC (cursor_up)
DEFINE_FUNC (cursor_down)

#undef DEFINE_FUNC

void
bus_input_context_candidate_clicked (BusInputContext *context,
                                     guint            index,
                                     guint            button,
                                     guint            state)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->engine) {
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

    if (context->engine) {
        bus_engine_proxy_property_activate (context->engine, prop_name, prop_state);
    }
}

static void
bus_input_context_commit_text (BusInputContext *context,
                               IBusText        *text)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (text == text_empty || text == NULL)
        return;

    GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)text);
    bus_input_context_emit_signal (context,
                                   "CommitText",
                                   g_variant_new ("(v)", variant),
                                   NULL);
}

/**
 * bus_input_context_update_preedit_text:
 *
 * Update a preedit text. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
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
        GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)context->preedit_text);
        bus_input_context_emit_signal (context,
                                       "UpdatePreeditText",
                                       g_variant_new ("(vub)", variant, context->preedit_cursor_pos, context->preedit_visible),
                                       NULL);
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

/**
 * bus_input_context_show_preedit_text:
 *
 * Show a preedit text. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_show_preedit_text (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->preedit_visible) {
        return;
    }

    context->preedit_visible = TRUE;

    if (PREEDIT_CONDITION) {
        bus_input_context_emit_signal (context,
                                       "ShowPreeditText",
                                       NULL,
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[SHOW_PREEDIT_TEXT],
                       0);
    }
}

/**
 * bus_input_context_hide_preedit_text:
 *
 * Hide a preedit text. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_hide_preedit_text (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!context->preedit_visible) {
        return;
    }

    context->preedit_visible = FALSE;

    if (PREEDIT_CONDITION) {
        bus_input_context_emit_signal (context,
                                       "HidePreeditText",
                                       NULL,
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[HIDE_PREEDIT_TEXT],
                       0);
    }
}

/**
 * bus_input_context_update_auxiliary_text:
 *
 * Update an aux text. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
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
        GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)text);
        bus_input_context_emit_signal (context,
                                       "UpdateAuxiliaryText",
                                       g_variant_new ("(vb)", variant, visible),
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[UPDATE_AUXILIARY_TEXT],
                       0,
                       context->auxiliary_text,
                       context->auxiliary_visible);
    }
}

/**
 * bus_input_context_show_auxiliary_text:
 *
 * Show an aux text. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_show_auxiliary_text (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->auxiliary_visible) {
        return;
    }

    context->auxiliary_visible = TRUE;

    if ((context->capabilities & IBUS_CAP_AUXILIARY_TEXT) == IBUS_CAP_AUXILIARY_TEXT) {
        bus_input_context_emit_signal (context,
                                       "ShowAuxiliaryText",
                                       NULL,
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[SHOW_AUXILIARY_TEXT],
                       0);
    }
}

/**
 * bus_input_context_hide_auxiliary_text:
 *
 * Hide an aux text. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_hide_auxiliary_text (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!context->auxiliary_visible) {
        return;
    }

    context->auxiliary_visible = FALSE;

    if ((context->capabilities & IBUS_CAP_AUXILIARY_TEXT) == IBUS_CAP_AUXILIARY_TEXT) {
        bus_input_context_emit_signal (context,
                                       "HideAuxiliaryText",
                                       NULL,
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[HIDE_AUXILIARY_TEXT],
                       0);
    }
}

/**
 * bus_input_context_update_lookup_table:
 *
 * Update contents in the lookup table.
 * Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
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
        GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)table);
        bus_input_context_emit_signal (context,
                                       "UpdateLookupTable",
                                       g_variant_new ("(vb)", variant, visible),
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[UPDATE_LOOKUP_TABLE],
                       0,
                       context->lookup_table,
                       context->lookup_table_visible);
    }
}

/**
 * bus_input_context_show_lookup_table:
 *
 * Show the lookup table. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_show_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context->lookup_table_visible) {
        return;
    }

    context->lookup_table_visible = TRUE;

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_emit_signal (context,
                                       "ShowLookupTable",
                                       NULL,
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[SHOW_LOOKUP_TABLE],
                       0);
    }
}

/**
 * bus_input_context_hide_lookup_table:
 *
 * Hide the lookup table. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_hide_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!context->lookup_table_visible) {
        return;
    }

    context->lookup_table_visible = FALSE;

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_emit_signal (context,
                                       "HideLookupTable",
                                       NULL,
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[HIDE_LOOKUP_TABLE],
                       0);
    }
}

/**
 * bus_input_context_page_up_lookup_table:
 *
 * Change cursor position. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_page_up_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!ibus_lookup_table_page_up (context->lookup_table)) {
        return;
    }

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_emit_signal (context,
                                       "PageUpLookupTable",
                                       NULL,
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[PAGE_UP_LOOKUP_TABLE],
                       0);
    }
}

/**
 * bus_input_context_page_down_lookup_table:
 *
 * Change cursor position. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_page_down_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!ibus_lookup_table_page_down (context->lookup_table)) {
        return;
    }

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_emit_signal (context,
                                       "PageDownLookupTable",
                                       NULL,
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[PAGE_DOWN_LOOKUP_TABLE],
                       0);
    }
}

/**
 * bus_input_context_cursor_up_lookup_table:
 *
 * Change cursor position. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_cursor_up_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!ibus_lookup_table_cursor_up (context->lookup_table)) {
        return;
    }

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_emit_signal (context,
                                       "CursorUpLookupTable",
                                       NULL,
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[CURSOR_UP_LOOKUP_TABLE],
                       0);
    }
}

/**
 * bus_input_context_cursor_down_lookup_table:
 *
 * Change cursor position. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_cursor_down_lookup_table (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!ibus_lookup_table_cursor_down (context->lookup_table)) {
        return;
    }

    if ((context->capabilities & IBUS_CAP_LOOKUP_TABLE) == IBUS_CAP_LOOKUP_TABLE) {
        bus_input_context_emit_signal (context,
                                       "CursorDownLookupTable",
                                       NULL,
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[CURSOR_DOWN_LOOKUP_TABLE],
                       0);
    }
}

/**
 * bus_input_context_register_properties:
 *
 * Register properties. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_register_properties (BusInputContext *context,
                                       IBusPropList    *props)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (IBUS_IS_PROP_LIST (props));

    if (context->capabilities & IBUS_CAP_PROPERTY) {
        GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)props);
        bus_input_context_emit_signal (context,
                                       "RegisterProperties",
                                       g_variant_new ("(v)", variant),
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[REGISTER_PROPERTIES],
                       0,
                       props);
    }
}

/**
 * bus_input_context_update_property:
 *
 * Update property. Send D-Bus signal to update status of client or send glib signal to the panel, depending on capabilities of the client.
 */
static void
bus_input_context_update_property (BusInputContext *context,
                                   IBusProperty    *prop)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (IBUS_IS_PROPERTY (prop));

    if (context->capabilities & IBUS_CAP_PROPERTY) {
        GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)prop);
        bus_input_context_emit_signal (context,
                                       "UpdateProperty",
                                       g_variant_new ("(v)", variant),
                                       NULL);
    }
    else {
        g_signal_emit (context,
                       context_signals[UPDATE_PROPERTY],
                       0,
                       prop);
    }
}

/**
 * _engine_destroy_cb:
 *
 * A function to be called when "destroy" glib signal is sent to the engine object.
 * Remove the engine from the context.
 */
static void
_engine_destroy_cb (BusEngineProxy  *engine,
                    BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

    bus_input_context_set_engine (context, NULL);
}

/**
 * _engine_commit_text_cb:
 *
 * A function to be called when "commit-text" glib signal is sent to the engine object.
 */
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

/**
 * _engine_forward_key_event_cb:
 *
 * A function to be called when "forward-key-event" glib signal is sent to the engine object.
 */
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

    bus_input_context_emit_signal (context,
                                   "ForwardKeyEvent",
                                   g_variant_new ("(uuu)", keyval, keycode, state),
                                   NULL);
}

/**
 * _engine_delete_surrounding_text_cb:
 *
 * A function to be called when "delete-surrounding-text" glib signal is sent to the engine object.
 */
static void
_engine_delete_surrounding_text_cb (BusEngineProxy    *engine,
                                    gint               offset_from_cursor,
                                    guint              nchars,
                                    BusInputContext   *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

    bus_input_context_emit_signal (context,
                                   "DeleteSurroundingText",
                                   g_variant_new ("(iu)", offset_from_cursor, nchars),
                                   NULL);
}

/**
 * _engine_require_surrounding_text_cb:
 *
 * A function to be called when "require-surrounding-text" glib signal is sent to the engine object.
 */
static void
_engine_require_surrounding_text_cb (BusEngineProxy    *engine,
                                     BusInputContext   *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

    bus_input_context_emit_signal (context,
                                   "RequireSurroundingText",
                                   NULL,
                                   NULL);
}

/**
 * _engine_update_preedit_text_cb:
 *
 * A function to be called when "update-preedit-text" glib signal is sent to the engine object.
 */
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

    bus_input_context_update_preedit_text (context, text, cursor_pos, visible, mode);
}

/**
 * _engine_update_auxiliary_text_cb:
 *
 * A function to be called when "update-auxiliary-text" glib signal is sent to the engine object.
 */
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

    bus_input_context_update_auxiliary_text (context, text, visible);
}

/**
 * _engine_update_lookup_table_cb:
 *
 * A function to be called when "update-lookup-table" glib signal is sent to the engine object.
 */
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

    bus_input_context_update_lookup_table (context, table, visible);
}

/**
 * _engine_register_properties_cb:
 *
 * A function to be called when "register-properties" glib signal is sent to the engine object.
 */
static void
_engine_register_properties_cb (BusEngineProxy  *engine,
                                IBusPropList    *props,
                                BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (IBUS_IS_PROP_LIST (props));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

    bus_input_context_register_properties (context, props);
}

/**
 * _engine_update_property_cb:
 *
 * A function to be called when "update-property" glib signal is sent to the engine object.
 */
static void
_engine_update_property_cb (BusEngineProxy  *engine,
                            IBusProperty    *prop,
                            BusInputContext *context)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (IBUS_IS_PROPERTY (prop));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (context->engine == engine);

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

BusInputContext *
bus_input_context_new (BusConnection    *connection,
                       const gchar      *client)
{
    static guint id = 0;

    g_assert (connection == NULL || BUS_IS_CONNECTION (connection));
    g_assert (client != NULL);

    gchar *path = g_strdup_printf (IBUS_PATH_INPUT_CONTEXT, ++id);

    BusInputContext *context = NULL;
    if (connection) {
        context = (BusInputContext *) g_object_new (BUS_TYPE_INPUT_CONTEXT,
                                                    "object-path", path,
                                                    "connection", bus_connection_get_dbus_connection (connection),
                                                    NULL);
    }
    else {
        context = (BusInputContext *) g_object_new (BUS_TYPE_INPUT_CONTEXT,
                                                    "object-path", path,
                                                    NULL);
    }
    g_free (path);

    context->client = g_strdup (client);

    /* it is a fake input context, just need process hotkey */
    context->fake = (strncmp (client, "fake", 4) == 0);

    if (connection) {
        g_object_ref_sink (connection);
        context->connection = connection;
        g_signal_connect (context->connection,
                          "destroy",
                          (GCallback) _connection_destroy_cb,
                          context);
    }

    return context;
}

void
bus_input_context_enable (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (!context->has_focus) {
        return;
    }

    if (context->engine == NULL) {
        IBusEngineDesc *desc = NULL;
        g_signal_emit (context,
                       context_signals[REQUEST_ENGINE], 0,
                       NULL,
                       &desc);
        if (desc != NULL) {
            bus_input_context_set_engine_by_desc (context,
                            desc,
                            g_gdbus_timeout, /* timeout in msec. */
                            NULL, /* we do not cancel the call. */
                            NULL, /* use the default callback function. */
                            NULL);
        }
    }

    if (context->engine == NULL)
        return;

    bus_engine_proxy_focus_in (context->engine);
    bus_engine_proxy_enable (context->engine);
    bus_engine_proxy_set_capabilities (context->engine, context->capabilities);
    bus_engine_proxy_set_cursor_location (context->engine, context->x, context->y, context->w, context->h);
    bus_engine_proxy_set_content_type (context->engine, context->purpose, context->hints);
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
}

/* A list of signals (and their handler functions) that could be emit by the engine proxy object. */
const static struct {
    const gchar *name;
    GCallback    callback;
} engine_signals [] = {
    { "commit-text",              G_CALLBACK (_engine_commit_text_cb) },
    { "forward-key-event",        G_CALLBACK (_engine_forward_key_event_cb) },
    { "delete-surrounding-text",  G_CALLBACK (_engine_delete_surrounding_text_cb) },
    { "require-surrounding-text", G_CALLBACK (_engine_require_surrounding_text_cb) },
    { "update-preedit-text",      G_CALLBACK (_engine_update_preedit_text_cb) },
    { "show-preedit-text",        G_CALLBACK (_engine_show_preedit_text_cb) },
    { "hide-preedit-text",        G_CALLBACK (_engine_hide_preedit_text_cb) },
    { "update-auxiliary-text",    G_CALLBACK (_engine_update_auxiliary_text_cb) },
    { "show-auxiliary-text",      G_CALLBACK (_engine_show_auxiliary_text_cb) },
    { "hide-auxiliary-text",      G_CALLBACK (_engine_hide_auxiliary_text_cb) },
    { "update-lookup-table",      G_CALLBACK (_engine_update_lookup_table_cb) },
    { "show-lookup-table",        G_CALLBACK (_engine_show_lookup_table_cb) },
    { "hide-lookup-table",        G_CALLBACK (_engine_hide_lookup_table_cb) },
    { "page-up-lookup-table",     G_CALLBACK (_engine_page_up_lookup_table_cb) },
    { "page-down-lookup-table",   G_CALLBACK (_engine_page_down_lookup_table_cb) },
    { "cursor-up-lookup-table",   G_CALLBACK (_engine_cursor_up_lookup_table_cb) },
    { "cursor-down-lookup-table", G_CALLBACK (_engine_cursor_down_lookup_table_cb) },
    { "register-properties",      G_CALLBACK (_engine_register_properties_cb) },
    { "update-property",          G_CALLBACK (_engine_update_property_cb) },
    { "destroy",                  G_CALLBACK (_engine_destroy_cb) },
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
        /* uninstall signal handlers for the engine. */
        for (i = 0; i < G_N_ELEMENTS(engine_signals); i++) {
            g_signal_handlers_disconnect_by_func (context->engine,
                    engine_signals[i].callback, context);
        }
        /* focus out engine so that the next call of
           bus_engine_proxy_focus_in() will take effect and trigger
           RegisterProperties. */
        bus_engine_proxy_focus_out (context->engine);
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
        g_object_ref (context->engine);

        /* handle signals from the engine. */
        for (i = 0; i < G_N_ELEMENTS(engine_signals); i++) {
            g_signal_connect (context->engine,
                              engine_signals[i].name,
                              engine_signals[i].callback,
                              context);
        }
        if (context->has_focus) {
            bus_engine_proxy_focus_in (context->engine);
            bus_engine_proxy_enable (context->engine);
            bus_engine_proxy_set_capabilities (context->engine, context->capabilities);
            bus_engine_proxy_set_cursor_location (context->engine, context->x, context->y, context->w, context->h);
            bus_engine_proxy_set_content_type (context->engine, context->purpose, context->hints);
        }
    }
    g_signal_emit (context,
                   context_signals[ENGINE_CHANGED],
                   0);
}

static void set_engine_by_desc_data_free (SetEngineByDescData *data)
{
    if (data->context != NULL) {
        if (data->context->data == data)
            data->context->data = NULL;
        g_object_unref (data->context);
    }

    if (data->simple != NULL) {
        g_object_unref (data->simple);
    }

    if (data->cancellable != NULL)
        g_object_unref (data->cancellable);

    if (data->origin_cancellable != NULL) {
        if (data->cancelled_handler_id != 0)
            g_cancellable_disconnect (data->origin_cancellable,
                data->cancelled_handler_id);
        g_object_unref (data->origin_cancellable);
    }

    g_slice_free (SetEngineByDescData, data);
}

/**
 * new_engine_cb:
 *
 * A callback function to be called when bus_engine_proxy_new() is finished.
 */
static void
new_engine_cb (GObject             *obj,
               GAsyncResult        *res,
               SetEngineByDescData *data)
{
    GError *error = NULL;
    BusEngineProxy *engine = bus_engine_proxy_new_finish (res, &error);

    if (engine == NULL) {
        g_simple_async_result_set_from_error (data->simple, error);
        g_error_free (error);
    }
    else {
        if (data->context->data != data) {
            /* Request has been overriden or cancelled */
            g_object_unref (engine);
            g_simple_async_result_set_error (data->simple,
                                             G_IO_ERROR,
                                             G_IO_ERROR_CANCELLED,
                                             "Opertation was cancelled");
        }
        else {
            /* Let BusEngineProxy call a Disable signal. */
            bus_input_context_disable (data->context);
            bus_input_context_set_engine (data->context, engine);
            g_object_unref (engine);
            bus_input_context_enable (data->context);
            g_simple_async_result_set_op_res_gboolean (data->simple, TRUE);
        }
    }

    /* Call the callback function for bus_input_context_set_engine_by_desc(). */
    g_simple_async_result_complete_in_idle (data->simple);

    set_engine_by_desc_data_free (data);
}

static void
cancel_set_engine_by_desc (SetEngineByDescData *data)
{
    if (data->context->data == data)
        data->context->data = NULL;

    if (data->origin_cancellable != NULL) {
        if (data->cancelled_handler_id != 0) {
            g_cancellable_disconnect (data->origin_cancellable,
                                      data->cancelled_handler_id);
            data->cancelled_handler_id = 0;
        }

        g_object_unref (data->origin_cancellable);
        data->origin_cancellable = NULL;
    }

    if (data->cancellable != NULL) {
        g_cancellable_cancel (data->cancellable);
        g_object_unref (data->cancellable);
        data->cancellable = NULL;
    }
}

static gboolean
set_engine_by_desc_cancelled_idle_cb (SetEngineByDescData *data)
{
    cancel_set_engine_by_desc (data);
    return FALSE;
}

static void
set_engine_by_desc_cancelled_cb (GCancellable        *cancellable,
                                 SetEngineByDescData *data)
{
    /* Cancel in idle to avoid deadlock */
    g_idle_add ((GSourceFunc) set_engine_by_desc_cancelled_idle_cb, data);
}

/**
 * set_engine_by_desc_ready_cb:
 *
 * A default callback function for bus_input_context_set_engine_by_desc().
 */
static void
set_engine_by_desc_ready_cb (BusInputContext *context,
                             GAsyncResult    *res,
                             gpointer         user_data)
{
    GError *error = NULL;
    if (!bus_input_context_set_engine_by_desc_finish (context, res, &error)) {
        g_warning ("Set context engine failed: %s", error->message);
        g_error_free (error);
    }
}

void
bus_input_context_set_engine_by_desc (BusInputContext    *context,
                                      IBusEngineDesc     *desc,
                                      gint                timeout,
                                      GCancellable       *cancellable,
                                      GAsyncReadyCallback callback,
                                      gpointer            user_data)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (IBUS_IS_ENGINE_DESC (desc));
    g_assert (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

    if (context->data != NULL) {
        /* Cancel previous set_engine_by_desc() request */
        cancel_set_engine_by_desc (context->data);
    }

    /* Previous request must be completed or cancelled */
    g_assert (context->data == NULL);

    if (callback == NULL)
        callback = (GAsyncReadyCallback) set_engine_by_desc_ready_cb;

    GSimpleAsyncResult *simple =
            g_simple_async_result_new ((GObject *) context,
                                       callback,
                                       user_data,
                                       bus_input_context_set_engine_by_desc);

    if (g_cancellable_is_cancelled (cancellable)) {
        g_simple_async_result_set_error (simple,
                                         G_IO_ERROR,
                                         G_IO_ERROR_CANCELLED,
                                         "Operation was cancelled");
        g_simple_async_result_complete_in_idle (simple);
        g_object_unref (simple);
        return;
    }

    SetEngineByDescData *data = g_slice_new0 (SetEngineByDescData);
    context->data = data;
    data->context = context;
    g_object_ref (context);
    data->simple = simple;

    if (cancellable != NULL) {
        data->origin_cancellable = cancellable;
        g_object_ref (cancellable);
        data->cancelled_handler_id =
                g_cancellable_connect (data->origin_cancellable,
                                       (GCallback) set_engine_by_desc_cancelled_cb,
                                       data,
                                       NULL);
    }

    data->cancellable = g_cancellable_new ();
    /* We can cancel the bus_engine_proxy_new() call by data->cancellable;
     * See cancel_set_engine_by_desc() and set_engine_by_desc_cancelled_cb(). */
    bus_engine_proxy_new (desc,
                          timeout,
                          data->cancellable,
                          (GAsyncReadyCallback) new_engine_cb,
                          data);
}

gboolean
bus_input_context_set_engine_by_desc_finish (BusInputContext  *context,
                                             GAsyncResult     *res,
                                             GError          **error)
{
    GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (res);

    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (g_simple_async_result_get_source_tag (simple) ==
                bus_input_context_set_engine_by_desc);

    if (g_simple_async_result_propagate_error (simple, error))
        return FALSE;
    return TRUE;
}

BusEngineProxy *
bus_input_context_get_engine (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    return context->engine;
}

IBusEngineDesc *
bus_input_context_get_engine_desc (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    if (context->engine)
        return bus_engine_proxy_get_desc (context->engine);
    return NULL;
}

guint
bus_input_context_get_capabilities (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    return context->capabilities;
}

void
bus_input_context_set_capabilities (BusInputContext    *context,
                                    guint               capabilities)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    /* If the context does not support IBUS_CAP_FOCUS, then the client application have to handle all information such as
     * preedit and auxiliary text. */
    if ((capabilities & IBUS_CAP_FOCUS) == 0) {
        capabilities |= (IBUS_CAP_PREEDIT_TEXT | IBUS_CAP_AUXILIARY_TEXT | IBUS_CAP_LOOKUP_TABLE | IBUS_CAP_PROPERTY);
    }

    if (context->capabilities != capabilities) {
        context->capabilities = capabilities;

        /* If the context does not support IBUS_CAP_FOCUS, then we always assume
         * it has focus. */
        if ((capabilities & IBUS_CAP_FOCUS) == 0) {
            bus_input_context_focus_in (context);
        }

        if (context->engine) {
            bus_engine_proxy_set_capabilities (context->engine, capabilities);
        }
    }

    context->capabilities = capabilities;
}


const gchar *
bus_input_context_get_client (BusInputContext *context)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    return context->client;
}
