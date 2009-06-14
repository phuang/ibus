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
#include "engineproxy.h"

enum {
    COMMIT_TEXT,
    FORWARD_KEY_EVENT,
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
    LAST_SIGNAL,
};


static guint    engine_signals[LAST_SIGNAL] = { 0 };
// static guint            engine_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_engine_proxy_class_init     (BusEngineProxyClass    *klass);
static void     bus_engine_proxy_init           (BusEngineProxy         *engine);
static void     bus_engine_proxy_real_destroy   (BusEngineProxy         *engine);

static gboolean bus_engine_proxy_ibus_signal    (IBusProxy              *proxy,
                                                 IBusMessage            *message);

static IBusProxyClass  *parent_class = NULL;

GType
bus_engine_proxy_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusEngineProxyClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_engine_proxy_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusEngineProxy),
        0,
        (GInstanceInitFunc) bus_engine_proxy_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_PROXY,
                    "BusEngineProxy",
                    &type_info,
                    (GTypeFlags)0);
    }
    return type;
}

BusEngineProxy *
bus_engine_proxy_new (const gchar    *path,
                      IBusEngineDesc *desc,
                      BusConnection  *connection)
{
    g_assert (path);
    g_assert (IBUS_IS_ENGINE_DESC (desc));
    g_assert (BUS_IS_CONNECTION (connection));

    BusEngineProxy *engine;

    engine = (BusEngineProxy *) g_object_new (BUS_TYPE_ENGINE_PROXY,
                                              "name", NULL,
                                              "path", path,
                                              "connection", connection,
                                              NULL);

    engine->desc = desc;
    g_object_ref (desc);
    if (desc->layout != NULL && desc->layout[0] != '\0') {
        engine->keymap = ibus_keymap_new (desc->layout);
    }

    if (engine->keymap == NULL) {
        engine->keymap = ibus_keymap_new ("en-us");
    }

    return engine;
}

static void
bus_engine_proxy_class_init (BusEngineProxyClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusProxyClass *proxy_class = IBUS_PROXY_CLASS (klass);


    parent_class = (IBusProxyClass *) g_type_class_peek_parent (klass);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_engine_proxy_real_destroy;

    proxy_class->ibus_signal = bus_engine_proxy_ibus_signal;

    /* install signals */
    engine_signals[COMMIT_TEXT] =
        g_signal_new (I_("commit-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__OBJECT,
            G_TYPE_NONE,
            1,
            IBUS_TYPE_TEXT);

    engine_signals[FORWARD_KEY_EVENT] =
        g_signal_new (I_("forward-key-event"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__UINT_UINT,
            G_TYPE_NONE,
            2,
            G_TYPE_UINT,
            G_TYPE_UINT);

    engine_signals[UPDATE_PREEDIT_TEXT] =
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

    engine_signals[SHOW_PREEDIT_TEXT] =
        g_signal_new (I_("show-preedit-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    engine_signals[HIDE_PREEDIT_TEXT] =
        g_signal_new (I_("hide-preedit-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    engine_signals[UPDATE_AUXILIARY_TEXT] =
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

    engine_signals[SHOW_AUXILIARY_TEXT] =
        g_signal_new (I_("show-auxiliary-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    engine_signals[HIDE_AUXILIARY_TEXT] =
        g_signal_new (I_("hide-auxiliary-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    engine_signals[UPDATE_LOOKUP_TABLE] =
        g_signal_new (I_("update-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__BOXED_BOOLEAN,
            G_TYPE_NONE,
            2,
            IBUS_TYPE_LOOKUP_TABLE,
            G_TYPE_BOOLEAN);

    engine_signals[SHOW_LOOKUP_TABLE] =
        g_signal_new (I_("show-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    engine_signals[HIDE_LOOKUP_TABLE] =
        g_signal_new (I_("hide-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    engine_signals[PAGE_UP_LOOKUP_TABLE] =
        g_signal_new (I_("page-up-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    engine_signals[PAGE_DOWN_LOOKUP_TABLE] =
        g_signal_new (I_("page-down-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    engine_signals[CURSOR_UP_LOOKUP_TABLE] =
        g_signal_new (I_("cursor-up-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    engine_signals[CURSOR_DOWN_LOOKUP_TABLE] =
        g_signal_new (I_("cursor-down-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    engine_signals[REGISTER_PROPERTIES] =
        g_signal_new (I_("register-properties"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__OBJECT,
            G_TYPE_NONE,
            1,
            IBUS_TYPE_PROP_LIST);

    engine_signals[UPDATE_PROPERTY] =
        g_signal_new (I_("update-property"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__OBJECT,
            G_TYPE_NONE,
            1,
            IBUS_TYPE_PROPERTY);

}

static void
bus_engine_proxy_init (BusEngineProxy *engine)
{
    engine->enabled = FALSE;
    engine->prop_list = NULL;
    engine->desc = NULL;
    engine->keymap = NULL;
}

static void
bus_engine_proxy_real_destroy (BusEngineProxy *engine)
{
    if (engine->prop_list) {
        g_object_unref (engine->prop_list);
        engine->prop_list = NULL;
    }

    if (ibus_proxy_get_connection ((IBusProxy *) engine)) {
        ibus_proxy_call ((IBusProxy *) engine,
                         "Destroy",
                         G_TYPE_INVALID);
    }

    if (engine->desc) {
        g_object_unref (engine->desc);
        engine->desc = NULL;
    }

    if (engine->keymap) {
        g_object_unref (engine->keymap);
        engine->keymap = NULL;
    }

    IBUS_OBJECT_CLASS(parent_class)->destroy (IBUS_OBJECT (engine));
}

static gboolean
bus_engine_proxy_ibus_signal (IBusProxy     *proxy,
                              IBusMessage   *message)
{
    g_assert (BUS_IS_ENGINE_PROXY (proxy));
    g_assert (message != NULL);

    BusEngineProxy *engine;
    IBusError *error;
    gint i;

    static const struct {
        const gchar *member;
        const guint signal_id;
    } signals [] = {
        { "ShowPreeditText",        SHOW_PREEDIT_TEXT },
        { "HidePreeditText",        HIDE_PREEDIT_TEXT },
        { "ShowAuxiliaryText",      SHOW_AUXILIARY_TEXT },
        { "HideAuxiliaryText",      HIDE_AUXILIARY_TEXT },
        { "ShowLookupTable",        SHOW_LOOKUP_TABLE },
        { "HideLookupTable",        HIDE_LOOKUP_TABLE },
        { "PageUpLookupTable",      PAGE_UP_LOOKUP_TABLE },
        { "PageDownLookupTable",    PAGE_DOWN_LOOKUP_TABLE },
        { "CursorUpLookupTable",    CURSOR_UP_LOOKUP_TABLE },
        { "CursorDownLookupTable",  CURSOR_DOWN_LOOKUP_TABLE },
        { NULL, 0},
    };

    engine = BUS_ENGINE_PROXY (proxy);

    for (i = 0; ; i++) {
        if (signals[i].member == NULL)
            break;
        if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, signals[i].member)) {
            g_signal_emit (engine, engine_signals[signals[i].signal_id], 0);
            goto handled;
        }
    }

    if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "CommitText")) {
        IBusText *text;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_TEXT, &text,
                                        G_TYPE_INVALID);
        if (!retval)
            goto failed;
        g_signal_emit (engine, engine_signals[COMMIT_TEXT], 0, text);
        g_object_unref (text);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "ForwardKeyEvent")) {
        guint keyval;
        guint states;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_UINT, &keyval,
                                        G_TYPE_UINT, &states,
                                        G_TYPE_INVALID);

        if (!retval)
            goto failed;
        g_signal_emit (engine, engine_signals[FORWARD_KEY_EVENT], keyval, states);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "UpdatePreeditText")) {
        IBusText *text;
        gint cursor_pos;
        gboolean visible;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_TEXT, &text,
                                        G_TYPE_UINT, &cursor_pos,
                                        G_TYPE_BOOLEAN, &visible,
                                        G_TYPE_INVALID);

        if (!retval)
            goto failed;

        g_signal_emit (engine, engine_signals[UPDATE_PREEDIT_TEXT], 0,
                       text, cursor_pos, visible);
        g_object_unref (text);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "UpdateAuxiliaryText")) {
        IBusText *text;
        gboolean visible;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_TEXT, &text,
                                        G_TYPE_BOOLEAN, &visible,
                                        G_TYPE_INVALID);

        if (!retval)
            goto failed;

        g_signal_emit (engine, engine_signals[UPDATE_AUXILIARY_TEXT], 0, text, visible);
        g_object_unref (text);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "UpdateLookupTable")) {
        IBusLookupTable *table;
        gboolean visible;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_LOOKUP_TABLE, &table,
                                        G_TYPE_BOOLEAN, &visible,
                                        G_TYPE_INVALID);

        if (!retval)
            goto failed;

        g_signal_emit (engine, engine_signals[UPDATE_LOOKUP_TABLE], 0, table, visible);
        g_object_unref (table);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "RegisterProperties")) {
        gboolean retval;

        if (engine->prop_list) {
            g_object_unref (engine->prop_list);
            engine->prop_list = NULL;
        }

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_PROP_LIST, &engine->prop_list,
                                        G_TYPE_INVALID);
        if (!retval) {
            engine->prop_list = NULL;
            goto failed;
        }
        g_signal_emit (engine, engine_signals[REGISTER_PROPERTIES], 0, engine->prop_list);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "UpdateProperty")) {
        IBusProperty *prop;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_PROPERTY, &prop,
                                        G_TYPE_INVALID);

        if (!retval)
            goto failed;

        g_signal_emit (engine, engine_signals[UPDATE_PROPERTY], 0, prop);
        g_object_unref (prop);
    }
    else {
        return FALSE;
    }

handled:
    g_signal_stop_emission_by_name (engine, "ibus-signal");
    return TRUE;

failed:
    g_warning ("%s: %s", error->name, error->message);
    ibus_error_free (error);
    return FALSE;
}

typedef struct {
    GFunc    func;
    gpointer user_data;
}CallData;

static void
bus_engine_proxy_process_key_event_reply_cb (IBusPendingCall *pending,
                                             CallData        *call_data)
{
    IBusMessage *reply_message;
    IBusError *error;
    gboolean retval;

    reply_message = dbus_pending_call_steal_reply (pending);

    if (reply_message == NULL) {
        call_data->func(FALSE, call_data->user_data);
        return;
    }
    else if ((error = ibus_error_new_from_message (reply_message)) != NULL) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_message_unref (reply_message);
        ibus_error_free (error);
        call_data->func(FALSE, call_data->user_data);
        return;
    }

    if (!ibus_message_get_args (reply_message,
                                &error,
                                G_TYPE_BOOLEAN, &retval,
                                G_TYPE_INVALID)) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_message_unref (reply_message);
        ibus_error_free (error);
        call_data->func (GINT_TO_POINTER (FALSE), call_data->user_data);
        return;
    }

    call_data->func (GINT_TO_POINTER (retval), call_data->user_data);
    g_slice_free (CallData, call_data);
}

void
bus_engine_proxy_process_key_event (BusEngineProxy *engine,
                                    guint           keyval,
                                    guint           keycode,
                                    guint           state,
                                    GFunc           return_cb,
                                    gpointer        user_data)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (return_cb);

    IBusPendingCall *pending = NULL;
    CallData *call_data;
    IBusError *error;
    gboolean retval;

    if (keycode != 0 && engine->keymap != NULL) {
        guint t = ibus_keymap_lookup_keysym (engine->keymap, keycode, state);
        if (t != IBUS_VoidSymbol) {
            keyval = t;
        }
    }

    retval = ibus_proxy_call_with_reply ((IBusProxy *) engine,
                                         "ProcessKeyEvent",
                                         &pending,
                                         -1,
                                         &error,
                                         G_TYPE_UINT, &keyval,
                                         G_TYPE_UINT, &keycode,
                                         G_TYPE_UINT, &state,
                                         G_TYPE_INVALID);
    if (!retval) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        return_cb (GINT_TO_POINTER (FALSE), user_data);
        return;
    }

    call_data = g_slice_new0 (CallData);
    call_data->func = return_cb;
    call_data->user_data = user_data;

    retval = ibus_pending_call_set_notify (pending,
                                           (IBusPendingCallNotifyFunction) bus_engine_proxy_process_key_event_reply_cb,
                                           call_data,
                                           NULL);
    ibus_pending_call_unref (pending);

    if (!retval) {
        g_warning ("%s : ProcessKeyEvent", DBUS_ERROR_NO_MEMORY);
        return_cb (GINT_TO_POINTER (FALSE), user_data);
        return;
    }
}

void
bus_engine_proxy_set_cursor_location (BusEngineProxy *engine,
                                      gint            x,
                                      gint            y,
                                      gint            w,
                                      gint            h)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));

    ibus_proxy_call ((IBusProxy *) engine,
                     "SetCursorLocation",
                     G_TYPE_INT, &x,
                     G_TYPE_INT, &y,
                     G_TYPE_INT, &w,
                     G_TYPE_INT, &h,
                     G_TYPE_INVALID);
}

void
bus_engine_proxy_set_capabilities (BusEngineProxy *engine,
                                   guint           caps)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));

    ibus_proxy_call ((IBusProxy *) engine,
                     "SetCapabilities",
                     G_TYPE_UINT, &caps,
                     G_TYPE_INVALID);

}

void
bus_engine_proxy_property_activate (BusEngineProxy *engine,
                                    const gchar    *prop_name,
                                    guint           prop_state)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (prop_name != NULL);

    ibus_proxy_call ((IBusProxy *) engine,
                     "PropertyActivate",
                     G_TYPE_STRING, &prop_name,
                     G_TYPE_UINT, &prop_state,
                     G_TYPE_INVALID);
}

void
bus_engine_proxy_property_show (BusEngineProxy *engine,
                                const gchar    *prop_name)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (prop_name != NULL);

    ibus_proxy_call ((IBusProxy *) engine,
                     "PropertyShow",
                     G_TYPE_STRING, &prop_name,
                     G_TYPE_INVALID);
}

void bus_engine_proxy_property_hide (BusEngineProxy *engine,
                                     const gchar    *prop_name)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (prop_name != NULL);

    ibus_proxy_call ((IBusProxy *) engine,
                     "PropertyHide",
                     G_TYPE_STRING, &prop_name,
                     G_TYPE_INVALID);
}

#define DEFINE_FUNCTION(Name, name)                         \
    void                                                    \
    bus_engine_proxy_##name (BusEngineProxy *engine)        \
    {                                                       \
        g_assert (BUS_IS_ENGINE_PROXY (engine));            \
        ibus_proxy_call ((IBusProxy *) engine,              \
                     #Name,                                 \
                     G_TYPE_INVALID);                       \
    }

DEFINE_FUNCTION (FocusIn, focus_in)
DEFINE_FUNCTION (FocusOut, focus_out)
DEFINE_FUNCTION (Reset, reset)
DEFINE_FUNCTION (PageUp, page_up)
DEFINE_FUNCTION (PageDown, page_down)
DEFINE_FUNCTION (CursorUp, cursor_up)
DEFINE_FUNCTION (CursorDown, cursor_down)
DEFINE_FUNCTION (Enable, enable)
DEFINE_FUNCTION (Disable, disable)

#undef DEFINE_FUNCTION

void
bus_engine_proxy_candidate_clicked (BusEngineProxy *engine,
                                    guint           index,
                                    guint           button,
                                    guint           state)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));

    ibus_proxy_call ((IBusProxy *) engine,
                     "CandidateClicked",
                     G_TYPE_UINT, &index,
                     G_TYPE_UINT, &button,
                     G_TYPE_UINT, &state,
                     G_TYPE_INVALID);
}

IBusEngineDesc *
bus_engine_proxy_get_desc (BusEngineProxy *engine)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));

    return engine->desc;
}
