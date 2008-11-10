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

#define BUS_ENGINE_PROXY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_ENGINE_PROXY, BusEngineProxyPrivate))

enum {
    COMMIT_STRING,
    FORWARD_KEY_EVENT,
    UPDATE_PREEDIT,
    SHOW_PREEDIT,
    HIDE_PREEDIT,
    UPDATE_AUX_STRING,
    SHOW_AUX_STRING,
    HIDE_AUX_STRING,
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


/* BusEngineProxyPriv */
struct _BusEngineProxyPrivate {
    gboolean enabled;
};
typedef struct _BusEngineProxyPrivate BusEngineProxyPrivate;

static guint    engine_signals[LAST_SIGNAL] = { 0 };
// static guint            engine_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_engine_proxy_class_init     (BusEngineProxyClass    *klass);
static void     bus_engine_proxy_init           (BusEngineProxy         *engine);
static void     bus_engine_proxy_real_destroy   (BusEngineProxy         *engine);

static gboolean bus_engine_proxy_ibus_signal    (IBusProxy              *proxy,
                                                 IBusMessage            *message);

static IBusProxyClass  *_parent_class = NULL;

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
bus_engine_proxy_new (const gchar       *path,
                      BusConnection     *connection)
{
    g_assert (path != NULL);
    g_assert (BUS_IS_CONNECTION (connection));
    GObject *obj;

    obj = g_object_new (BUS_TYPE_ENGINE_PROXY,
                        "name", NULL,
                        "path", path,
                        "connection", connection,
                        NULL);

    return BUS_ENGINE_PROXY (obj);
}

static void
bus_engine_proxy_class_init (BusEngineProxyClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusProxyClass *proxy_class = IBUS_PROXY_CLASS (klass);


    _parent_class = (IBusProxyClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusEngineProxyPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_engine_proxy_real_destroy;

    proxy_class->ibus_signal = bus_engine_proxy_ibus_signal;
    
    /* install signals */
    engine_signals[COMMIT_STRING] =
        g_signal_new (I_("commit-string"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING,
            G_TYPE_NONE,
            1,
            G_TYPE_STRING);
    
    engine_signals[FORWARD_KEY_EVENT] =
        g_signal_new (I_("forward-key-event"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__UINT_BOOLEAN_UINT,
            G_TYPE_NONE,
            3,
            G_TYPE_UINT,
            G_TYPE_BOOLEAN,
            G_TYPE_UINT);
    
    engine_signals[UPDATE_PREEDIT] =
        g_signal_new (I_("update-preedit"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING_BOXED_INT_BOOLEAN,
            G_TYPE_NONE,
            4,
            G_TYPE_STRING,
            IBUS_TYPE_ATTR_LIST | G_SIGNAL_TYPE_STATIC_SCOPE,
            G_TYPE_INT,
            G_TYPE_BOOLEAN);
    
    engine_signals[SHOW_PREEDIT] =
        g_signal_new (I_("show-preedit"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);
    
    engine_signals[HIDE_PREEDIT] =
        g_signal_new (I_("hide-preedit"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);
    
    engine_signals[UPDATE_AUX_STRING] =
        g_signal_new (I_("update-aux-string"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING_BOXED_BOOLEAN,
            G_TYPE_NONE,
            3,
            G_TYPE_STRING,
            IBUS_TYPE_ATTR_LIST | G_SIGNAL_TYPE_STATIC_SCOPE,
            G_TYPE_BOOLEAN);
    
    engine_signals[SHOW_AUX_STRING] =
        g_signal_new (I_("show-aux-string"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);
    
    engine_signals[HIDE_AUX_STRING] =
        g_signal_new (I_("hide-aux-string"),
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
            IBUS_TYPE_LOOKUP_TABLE | G_SIGNAL_TYPE_STATIC_SCOPE,
            G_TYPE_BOOLEAN);
    
    engine_signals[SHOW_LOOKUP_TABLE] =
        g_signal_new (I_("show-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    
    engine_signals[HIDE_LOOKUP_TABLE] =
        g_signal_new (I_("hide-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    engine_signals[PAGE_UP_LOOKUP_TABLE] =
        g_signal_new (I_("page-up-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    
    engine_signals[PAGE_DOWN_LOOKUP_TABLE] =
        g_signal_new (I_("page-down-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    
    engine_signals[CURSOR_UP_LOOKUP_TABLE] =
        g_signal_new (I_("cursor-up-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    
    engine_signals[CURSOR_DOWN_LOOKUP_TABLE] =
        g_signal_new (I_("cursor-down-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    
    engine_signals[REGISTER_PROPERTIES] =
        g_signal_new (I_("register-properties"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__BOXED,
            G_TYPE_NONE,
            1,
            IBUS_TYPE_PROP_LIST | G_SIGNAL_TYPE_STATIC_SCOPE);
    
    engine_signals[UPDATE_PROPERTY] =
        g_signal_new (I_("update-property"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__BOXED,
            G_TYPE_NONE,
            1,
            IBUS_TYPE_PROPERTY | G_SIGNAL_TYPE_STATIC_SCOPE);

}

static void
bus_engine_proxy_init (BusEngineProxy *engine)
{
    BusEngineProxyPrivate *priv;
    priv = BUS_ENGINE_PROXY_GET_PRIVATE (engine);

    priv->enabled = FALSE;
}

static void
bus_engine_proxy_real_destroy (BusEngineProxy *engine)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "Destroy",
                     DBUS_TYPE_INVALID);
    
    IBUS_OBJECT_CLASS(_parent_class)->destroy (IBUS_OBJECT (engine));
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

    struct {
        const gchar *member;
        const guint signal_id;
    } signals [] = {
        { "ShowPreedit",            SHOW_PREEDIT },
        { "HidePreedit",            HIDE_PREEDIT },
        { "ShowAuxString",          SHOW_AUX_STRING },
        { "HideAuxString",          HIDE_AUX_STRING },
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
    
    if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "CommitString")) {
        gchar *text;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &text,
                                        G_TYPE_INVALID);
        if (!retval)
            goto failed;
        g_signal_emit (engine, engine_signals[COMMIT_STRING], 0, text);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "ForwardKeyEvent")) {
        guint32 keyval;
        gboolean is_press;
        guint32 states;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_UINT, &keyval,
                                        G_TYPE_BOOLEAN, &is_press,
                                        G_TYPE_UINT, &states,
                                        G_TYPE_INVALID);

        if (!retval)
            goto failed;
        g_signal_emit (engine, engine_signals[FORWARD_KEY_EVENT], keyval, is_press, states);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "UpdatePreedit")) {
        gchar *text;
        IBusAttrList *attr_list;
        gint32 cursor_pos;
        gboolean visible;
        gboolean retval;
        
        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &text,
                                        IBUS_TYPE_ATTR_LIST, &attr_list,
                                        G_TYPE_INT, &cursor_pos,
                                        G_TYPE_BOOLEAN, &visible,
                                        G_TYPE_INVALID);

        if (!retval)
            goto failed;

        g_signal_emit (engine, engine_signals[UPDATE_PREEDIT], 0, text, attr_list, cursor_pos, visible);
        ibus_attr_list_unref (attr_list);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "UpdateAuxString")) {
        gchar *text;
        IBusAttrList *attr_list;
        gboolean visible;
        gboolean retval;
        
        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &text,
                                        IBUS_TYPE_ATTR_LIST, &attr_list,
                                        G_TYPE_BOOLEAN, &visible,
                                        G_TYPE_INVALID);

        if (!retval)
            goto failed;
        
        g_signal_emit (engine, engine_signals[UPDATE_AUX_STRING], 0, text, attr_list, visible);
        ibus_attr_list_unref (attr_list);
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
        ibus_lookup_table_unref (table);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "RegisterProperties")) {
        IBusPropList *prop_list;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_PROP_LIST, &prop_list,
                                        G_TYPE_INVALID);
        
        g_signal_emit (engine, engine_signals[REGISTER_PROPERTIES], 0, prop_list);
        ibus_prop_list_unref (prop_list);
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
        ibus_property_free (prop);
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

gboolean
bus_engine_proxy_process_key_event (BusEngineProxy *engine,
                                    guint32         keyval,
                                    gboolean        is_press,
                                    guint32         state)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    
    IBusMessage *reply_message;
    IBusError *error;
    gboolean retval;

    reply_message = ibus_proxy_call_with_reply_and_block (IBUS_PROXY (engine),
                                                          "ProcessKeyEvent",
                                                          -1,
                                                          &error,
                                                          G_TYPE_UINT, &keyval,
                                                          G_TYPE_BOOLEAN, &is_press,
                                                          G_TYPE_UINT, &state,
                                                          G_TYPE_INVALID);
    if (reply_message == NULL) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        retval = FALSE;
    }

    if (error = ibus_error_from_message (reply_message)) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_message_unref (reply_message);
        ibus_error_free (error);
        return FALSE;
    }
    
    if (!ibus_message_get_args (reply_message,
                                &error,
                                G_TYPE_BOOLEAN, &retval,
                                G_TYPE_INVALID)) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_message_unref (reply_message);
        ibus_error_free (error);
        return FALSE;
    }

    return retval;
}

void
bus_engine_proxy_set_cursor_location (BusEngineProxy *engine,
                                      gint32          x,
                                      gint32          y,
                                      gint32          w,
                                      gint32          h)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    
    ibus_proxy_call (IBUS_PROXY (engine),
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
    
    ibus_proxy_call (IBUS_PROXY (engine),
                     "SetCapabilites",
                     G_TYPE_UINT, &caps,
                     G_TYPE_INVALID);

}

void
bus_engine_proxy_property_activate (BusEngineProxy *engine,
                                    const gchar    *prop_name,
                                    gint32          state)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (prop_name != NULL);

    ibus_proxy_call (IBUS_PROXY (engine),
                     "PropertyActivate",
                     G_TYPE_STRING, &prop_name,
                     G_TYPE_INT, &state,
                     G_TYPE_INVALID);
}

void
bus_engine_proxy_property_show (BusEngineProxy *engine,
                                const gchar    *prop_name)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (prop_name != NULL);

    ibus_proxy_call (IBUS_PROXY (engine),
                     "PropertyShow",
                     G_TYPE_STRING, &prop_name,
                     G_TYPE_INVALID);
}

void bus_engine_proxy_property_hide (BusEngineProxy *engine,
                                     const gchar    *prop_name)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (prop_name != NULL);

    ibus_proxy_call (IBUS_PROXY (engine),
                     "PropertyHide",
                     G_TYPE_STRING, &prop_name,
                     G_TYPE_INVALID);
}

#define DEFINE_FUNCTION(Name, name)                         \
    void bus_engine_proxy_##name (BusEngineProxy *engine)   \
    {                                                       \
        g_assert (BUS_IS_ENGINE_PROXY (engine));            \
        ibus_proxy_call (IBUS_PROXY (engine),               \
                     #Name,                                 \
                     DBUS_TYPE_INVALID);                    \
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
