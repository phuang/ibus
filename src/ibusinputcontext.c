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
#include "ibusshare.h"
#include "ibusinternal.h"
#include "ibusinputcontext.h"
#include "ibusattribute.h"
#include "ibuslookuptable.h"
#include "ibusproperty.h"

#define IBUS_INPUT_CONTEXT_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_INPUT_CONTEXT, IBusInputContextPrivate))

enum {
    ENABLED,
    DISABLED,
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


/* BusInputContextPriv */
struct _IBusInputContextPrivate {
    void *pad;
};
typedef struct _IBusInputContextPrivate IBusInputContextPrivate;

static guint            context_signals[LAST_SIGNAL] = { 0 };
// static guint            context_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_input_context_class_init   (IBusInputContextClass  *klass);
static void     ibus_input_context_init         (IBusInputContext       *context);
static void     ibus_input_context_real_destroy (IBusInputContext       *context);
static gboolean ibus_input_context_ibus_signal  (IBusProxy              *proxy,
                                                 DBusMessage            *message);

static IBusProxyClass  *parent_class = NULL;

GType
ibus_input_context_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusInputContextClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_input_context_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusInputContext),
        0,
        (GInstanceInitFunc) ibus_input_context_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_PROXY,
                    "IBusInputContext",
                    &type_info,
                    (GTypeFlags)0);
    }
    return type;
}

IBusInputContext *
ibus_input_context_new (const gchar     *path,
                        IBusConnection  *connection)
{
    g_assert (path != NULL);
    g_assert (IBUS_IS_CONNECTION (connection));
    GObject *obj;

    obj = g_object_new (IBUS_TYPE_INPUT_CONTEXT,
                        "name", IBUS_SERVICE_IBUS,
                        "path", path,
                        "connection", connection,
                        NULL);

    return IBUS_INPUT_CONTEXT (obj);
}

static void
ibus_input_context_class_init (IBusInputContextClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusProxyClass *proxy_class = IBUS_PROXY_CLASS (klass);


    parent_class = (IBusProxyClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusInputContextPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_input_context_real_destroy;

    proxy_class->ibus_signal = ibus_input_context_ibus_signal;

    /* install signals */
    context_signals[ENABLED] =
        g_signal_new (I_("enabled"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[DISABLED] =
        g_signal_new (I_("disabled"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[COMMIT_TEXT] =
        g_signal_new (I_("commit-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__OBJECT,
            G_TYPE_NONE,
            1,
            IBUS_TYPE_TEXT);

    context_signals[FORWARD_KEY_EVENT] =
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
            G_TYPE_NONE, 0);

    context_signals[HIDE_PREEDIT_TEXT] =
        g_signal_new (I_("hide-preedit-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[UPDATE_AUXILIARY_TEXT] =
        g_signal_new (I_("update-auxiliary-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__OBJECT_BOOLEAN,
            G_TYPE_NONE, 2,
            IBUS_TYPE_TEXT,
            G_TYPE_BOOLEAN);

    context_signals[SHOW_AUXILIARY_TEXT] =
        g_signal_new (I_("show-auxiliary-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[HIDE_AUXILIARY_TEXT] =
        g_signal_new (I_("hide-auxiliary-text"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    context_signals[UPDATE_LOOKUP_TABLE] =
        g_signal_new (I_("update-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__OBJECT_BOOLEAN,
            G_TYPE_NONE, 2,
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
            G_TYPE_NONE,
            0);

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
}

static void
ibus_input_context_init (IBusInputContext *context)
{
    IBusInputContextPrivate *priv;
    priv = IBUS_INPUT_CONTEXT_GET_PRIVATE (context);
}

static void
ibus_input_context_real_destroy (IBusInputContext *context)
{
    if (ibus_proxy_get_connection ((IBusProxy *) context) != NULL) {
        ibus_proxy_call (IBUS_PROXY (context),
                         "Destroy",
                         G_TYPE_INVALID);
    }

    IBUS_OBJECT_CLASS(parent_class)->destroy (IBUS_OBJECT (context));
}

static gboolean
ibus_input_context_ibus_signal (IBusProxy           *proxy,
                                IBusMessage         *message)
{
    g_assert (IBUS_IS_INPUT_CONTEXT (proxy));
    g_assert (message != NULL);

    IBusInputContext *context;
    IBusError *error = NULL;
    gint i;

    context = IBUS_INPUT_CONTEXT (proxy);

    static const struct {
        const gchar *member;
        guint signal_id;
    } signals [] = {
        { "Enabled",                ENABLED                 },
        { "Disabled",               DISABLED                },
        { "ShowPreeditText",        SHOW_PREEDIT_TEXT       },
        { "HidePreeditText",        HIDE_PREEDIT_TEXT       },
        { "ShowAuxiliaryText",      SHOW_AUXILIARY_TEXT     },
        { "HideAuxiliaryText",      HIDE_AUXILIARY_TEXT     },
        { "ShowLookupTable",        SHOW_LOOKUP_TABLE       },
        { "HideLookupTable",        HIDE_LOOKUP_TABLE       },
        { "PageUpLookupTable",      PAGE_UP_LOOKUP_TABLE    },
        { "PageDownLookupTable",    PAGE_DOWN_LOOKUP_TABLE  },
        { "CursorUpLookupTable",    CURSOR_UP_LOOKUP_TABLE  },
        { "CursorDownLookupTable",  CURSOR_DOWN_LOOKUP_TABLE},
        { NULL, 0},
    };

    for (i = 0; ; i++) {
        if (signals[i].member == NULL)
            break;
        if (ibus_message_is_signal (message,
                                    IBUS_INTERFACE_INPUT_CONTEXT,
                                    signals[i].member)) {
            g_signal_emit (context, context_signals[signals[i].signal_id], 0);
            goto handled;
        }
    }

    if (ibus_message_is_signal (message,
                                IBUS_INTERFACE_INPUT_CONTEXT,
                                "CommitText")) {
        IBusText *text;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_TEXT, &text,
                                        G_TYPE_INVALID);
        if (!retval)
            goto failed;

        g_signal_emit (context, context_signals[COMMIT_TEXT], 0, text);
        g_object_unref (text);
    }
    else if (ibus_message_is_signal (message,
                                     IBUS_INTERFACE_INPUT_CONTEXT,
                                     "UpdatePreeditText")) {
        IBusText *text;
        gint32 cursor_pos;
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

        g_signal_emit (context,
                       context_signals[UPDATE_PREEDIT_TEXT],
                       0,
                       text,
                       cursor_pos,
                       visible);
        g_object_unref (text);
    }
    else if (ibus_message_is_signal (message,
                                     IBUS_INTERFACE_INPUT_CONTEXT,
                                     "ForwardKeyEvent")) {
        guint32 keyval;
        guint32 state;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_UINT, &keyval,
                                        G_TYPE_UINT, &state,
                                        G_TYPE_INVALID);

        if (!retval)
            goto failed;
        g_signal_emit (context,
                       context_signals[FORWARD_KEY_EVENT],
                       0,
                       keyval,
                       state | IBUS_FORWARD_MASK);
    }
    else if (ibus_message_is_signal (message,
                                     IBUS_INTERFACE_INPUT_CONTEXT,
                                     "UpdateAuxiliaryText")) {
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

        g_signal_emit (context,
                       context_signals[UPDATE_AUXILIARY_TEXT],
                       0,
                       text,
                       visible);
        g_object_unref (text);
    }
    else if (ibus_message_is_signal (message,
                                     IBUS_INTERFACE_INPUT_CONTEXT,
                                     "UpdateLookupTable")) {
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

        g_signal_emit (context,
                       context_signals[UPDATE_LOOKUP_TABLE],
                       0,
                       table,
                       visible);
        g_object_unref (table);
    }
    else if (ibus_message_is_signal (message,
                                     IBUS_INTERFACE_INPUT_CONTEXT,
                                     "RegisterProperties")) {
        IBusPropList *prop_list;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_PROP_LIST, &prop_list,
                                        G_TYPE_INVALID);

        if (!retval)
            goto failed;

        g_signal_emit (context,
                       context_signals[REGISTER_PROPERTIES],
                       0,
                       prop_list);
        g_object_unref (prop_list);
    }
    else if (ibus_message_is_signal (message,
                                     IBUS_INTERFACE_INPUT_CONTEXT,
                                     "UpdateProperty")) {
        IBusProperty *prop;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_PROPERTY, &prop,
                                        G_TYPE_INVALID);
        if (!retval)
            goto failed;

        g_signal_emit (context, context_signals[UPDATE_PROPERTY], 0, prop);
        g_object_unref (prop);
    }
    else {
        return FALSE;
    }

handled:
    g_signal_stop_emission_by_name (context, "ibus-signal");
    return TRUE;

failed:
    if (error) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
    }
    return FALSE;
}


gboolean
ibus_input_context_process_key_event (IBusInputContext *context,
                                      guint32           keyval,
                                      guint32           keycode,
                                      guint32           state)
{
    g_assert (IBUS_IS_INPUT_CONTEXT (context));

    IBusMessage *reply_message;
    IBusPendingCall *pending = NULL;
    IBusError *error = NULL;
    gboolean retval;

    if (state & IBUS_HANDLED_MASK)
        return TRUE;

    if (state & IBUS_IGNORED_MASK)
        return FALSE;

    retval = ibus_proxy_call_with_reply ((IBusProxy *) context,
                                         "ProcessKeyEvent",
                                         &pending,
                                         -1,
                                         &error,
                                         G_TYPE_UINT, &keyval,
                                         G_TYPE_UINT, &keycode,
                                         G_TYPE_UINT, &state,
                                         G_TYPE_INVALID);
    if (!retval) {
        g_debug ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        return FALSE;
    }

    /* wait reply or timeout */
    IBusConnection *connection = ibus_proxy_get_connection ((IBusProxy *) context);
    while (!ibus_pending_call_get_completed (pending)) {
        ibus_connection_read_write_dispatch (connection, -1);
    }

    reply_message = ibus_pending_call_steal_reply (pending);
    ibus_pending_call_unref (pending);

    if (reply_message == NULL) {
        g_debug ("%s: Do not recevie reply of ProcessKeyEvent", DBUS_ERROR_NO_REPLY);
        retval = FALSE;
    }
    else if ((error = ibus_error_new_from_message (reply_message)) != NULL) {
        g_debug ("%s: %s", error->name, error->message);
        ibus_message_unref (reply_message);
        ibus_error_free (error);
        retval = FALSE;
    }
    else {

        if (!ibus_message_get_args (reply_message,
                                    &error,
                                    G_TYPE_BOOLEAN, &retval,
                                    G_TYPE_INVALID)) {
            g_debug ("%s: %s", error->name, error->message);
            ibus_error_free (error);
            retval = FALSE;
        }
        ibus_message_unref (reply_message);
    }
    return retval;
}

void
ibus_input_context_set_cursor_location (IBusInputContext *context,
                                        gint32            x,
                                        gint32            y,
                                        gint32            w,
                                        gint32            h)
{
    g_assert (IBUS_IS_INPUT_CONTEXT (context));

    ibus_proxy_call ((IBusProxy *) context,
                     "SetCursorLocation",
                     G_TYPE_INT, &x,
                     G_TYPE_INT, &y,
                     G_TYPE_INT, &w,
                     G_TYPE_INT, &h,
                     G_TYPE_INVALID);
}

void
ibus_input_context_set_capabilities (IBusInputContext   *context,
                                     guint32             capabilites)
{
    g_assert (IBUS_IS_INPUT_CONTEXT (context));

    ibus_proxy_call ((IBusProxy *) context,
                     "SetCapabilities",
                     G_TYPE_UINT, &capabilites,
                     G_TYPE_INVALID);
}

void
ibus_input_context_property_activate (IBusInputContext *context,
                                      const gchar      *prop_name,
                                      gint32            state)
{
    g_assert (IBUS_IS_INPUT_CONTEXT (context));

    ibus_proxy_call ((IBusProxy *) context,
                     "PropertyActivate",
                     G_TYPE_STRING, &prop_name,
                     G_TYPE_INT, &state,
                     G_TYPE_INVALID);
}

void
ibus_input_context_property_show (IBusInputContext *context,
                                  const gchar     *prop_name)
{
    g_assert (IBUS_IS_INPUT_CONTEXT (context));

    ibus_proxy_call ((IBusProxy *) context,
                     "PropertyShow",
                     G_TYPE_STRING, &prop_name,
                     G_TYPE_INVALID);
}

void
ibus_input_context_property_hide (IBusInputContext *context,
                                       const gchar      *prop_name)
{
    g_assert (IBUS_IS_INPUT_CONTEXT (context));

    ibus_proxy_call ((IBusProxy *) context,
                     "PropertyHide",
                     G_TYPE_STRING, &prop_name,
                     G_TYPE_INVALID);
}

#define DEFINE_FUNC(name,Name)                              \
    void                                                    \
    ibus_input_context_##name (IBusInputContext *context)   \
    {                                                       \
        g_assert (IBUS_IS_INPUT_CONTEXT (context));         \
        ibus_proxy_call ((IBusProxy *) context,             \
                         #Name,                             \
                         G_TYPE_INVALID);                   \
    }

DEFINE_FUNC(focus_in, FocusIn);
DEFINE_FUNC(focus_out, FocusOut);
DEFINE_FUNC(reset, Reset);
DEFINE_FUNC(page_up, PageUp);
DEFINE_FUNC(page_down, PageDown);
DEFINE_FUNC(cursor_up, CursorUp);
DEFINE_FUNC(cursor_down, CursorDown);
DEFINE_FUNC(enable, Enable);
DEFINE_FUNC(disable, Disable);

#undef DEFINE_FUNC

