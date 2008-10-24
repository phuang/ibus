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
    void *pad;
};
typedef struct _BusEngineProxyPrivate BusEngineProxyPrivate;

static guint            engine_signals[LAST_SIGNAL] = { 0 };
// static guint            engine_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_engine_proxy_class_init     (BusEngineProxyClass    *klass);
static void     bus_engine_proxy_init           (BusEngineProxy         *engine_proxy);
static void     _bus_engine_proxy_destroy       (BusEngineProxy         *engine_proxy);

static gboolean bus_engine_proxy_dbus_signal    (BusEngineProxy         *engine_proxy,
                                                 DBusMessage            *message);

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
                        "name", "",
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

    ibus_object_class->destroy = (IBusObjectDestroyFunc) _bus_engine_proxy_destroy;

    proxy_class->dbus_signal = bus_engine_proxy_dbus_signal;
    
    /* install signals */
    engine_signals[COMMIT_STRING] =
        g_signal_new (I_("commit-string"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING,
            G_TYPE_NONE, 1,
            G_TYPE_STRING);
    
    engine_signals[FORWARD_KEY_EVENT] =
        g_signal_new (I_("forward-key-event"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__UINT_BOOLEAN_UINT,
            G_TYPE_NONE, 3,
            G_TYPE_UINT,
            G_TYPE_BOOLEAN,
            G_TYPE_UINT);
    
    engine_signals[UPDATE_PREEDIT] =
        g_signal_new (I_("update-preedit"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING_POINTER_INT_BOOLEAN,
            G_TYPE_NONE, 4,
            G_TYPE_STRING,
            G_TYPE_POINTER,
            G_TYPE_INT,
            G_TYPE_BOOLEAN);
    
    engine_signals[SHOW_PREEDIT] =
        g_signal_new (I_("show-preedit"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    
    engine_signals[HIDE_PREEDIT] =
        g_signal_new (I_("hide-preedit"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    
    engine_signals[UPDATE_AUX_STRING] =
        g_signal_new (I_("update-aux-string"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING_POINTER_BOOLEAN,
            G_TYPE_NONE, 3,
            G_TYPE_STRING,
            G_TYPE_POINTER,
            G_TYPE_BOOLEAN);
    
    engine_signals[SHOW_AUX_STRING] =
        g_signal_new (I_("show-aux-string"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    
    engine_signals[HIDE_AUX_STRING] =
        g_signal_new (I_("hide-aux-string"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    engine_signals[UPDATE_LOOKUP_TABLE] =
        g_signal_new (I_("update-lookup-table"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__POINTER_BOOLEAN,
            G_TYPE_NONE, 2,
            G_TYPE_POINTER,
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
            ibus_marshal_VOID__POINTER,
            G_TYPE_NONE, 1,
            G_TYPE_POINTER);
    
    engine_signals[UPDATE_PROPERTY] =
        g_signal_new (I_("update-property"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__POINTER,
            G_TYPE_NONE, 1,
            G_TYPE_POINTER);

}

static void
bus_engine_proxy_init (BusEngineProxy *engine_proxy)
{
    BusEngineProxyPrivate *priv;
    priv = BUS_ENGINE_PROXY_GET_PRIVATE (engine_proxy);
}

static void
_bus_engine_proxy_destroy (BusEngineProxy *engine_proxy)
{
    IBUS_OBJECT_CLASS(_parent_class)->destroy (IBUS_OBJECT (engine_proxy));
}

static gboolean
bus_engine_proxy_dbus_signal (BusEngineProxy    *engine,
                              DBusMessage       *message)
{
    g_assert (BUS_IS_ENGINE_PROXY (engine));
    g_assert (message != NULL);
    
    DBusError error;
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
    
    for (i = 0; ; i++) {
        if (signals[i].member == NULL)
            break;
        if (dbus_message_is_signal (message, IBUS_INTERFACE_ENGINE, signals[i].member)) {
            g_signal_emit (engine, engine_signals[signals[i].signal_id], 0);
            goto handled;
        }
    }
    
    dbus_error_init (&error);
    if (dbus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "CommitString")) {
        gchar *text;
        gboolean retval;

        retval = dbus_message_get_args (message, &error,
                                DBUS_TYPE_STRING, &text,
                                DBUS_TYPE_INVALID);
        if (!retval)
            goto failed;
        g_signal_emit (engine, engine_signals[COMMIT_STRING], 0, text);
    }
    else if (dbus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "ForwardKeyEvent")) {
        guint32 keyval;
        gboolean is_press;
        guint32 states;
        gboolean retval;

        retval = dbus_message_get_args (message, &error,
                                DBUS_TYPE_UINT32, &keyval,
                                DBUS_TYPE_BOOLEAN, &is_press,
                                DBUS_TYPE_UINT32, &states,
                                DBUS_TYPE_INVALID);

        if (!retval)
            goto failed;
        g_signal_emit (engine, engine_signals[FORWARD_KEY_EVENT], keyval, is_press, states);
    }
    else if (dbus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "UpdatePreedit")) {
        gchar *text;
        IBusAttrList *attr_list;
        gint32 cursor_pos;
        gboolean visible;
        DBusMessageIter iter;
        gboolean retval;

        retval = dbus_message_iter_init (message, &iter);
        if (!retval)
            goto failed;
        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_STRING)
            goto failed;
        dbus_message_iter_get_basic (&iter, &text);
        attr_list = ibus_attr_list_from_dbus_message (&iter);
        if (attr_list == NULL)
            goto failed;
        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INT32) {
            ibus_attr_list_unref (attr_list);
            goto failed;
        }
        dbus_message_iter_get_basic (&iter, &cursor_pos);
        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_BOOLEAN) {
            ibus_attr_list_unref (attr_list);
            goto failed;
        }
        dbus_message_iter_get_basic (&iter, &visible);

        g_signal_emit (engine, engine_signals[UPDATE_PREEDIT], 0, text, attr_list, cursor_pos, visible);
        ibus_attr_list_unref (attr_list);
    }
    else if (dbus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "UpdateAuxString")) {
        gchar *text;
        IBusAttrList *attr_list;
        gboolean visible;
        DBusMessageIter iter;
        gboolean retval;

        retval = dbus_message_iter_init (message, &iter);
        if (!retval)
            goto failed;
        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_STRING)
            goto failed;
        dbus_message_iter_get_basic (&iter, &text);
        
        attr_list = ibus_attr_list_from_dbus_message (&iter);
        if (attr_list == NULL)
            goto failed;
        
        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_BOOLEAN) {
            ibus_attr_list_unref (attr_list);
            goto failed;
        }
        dbus_message_iter_get_basic (&iter, &visible);

        g_signal_emit (engine, engine_signals[UPDATE_AUX_STRING], 0, text, attr_list, visible);
        ibus_attr_list_unref (attr_list);
    }
    else if (dbus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "UpdateLookupTable")) {
        IBusLookupTable *table;
        gboolean visible;
        DBusMessageIter iter;
        gboolean retval;

        retval = dbus_message_iter_init (message, &iter);
        if (!retval)
            goto failed;
        
        table = ibus_lookup_table_from_dbus_message (&iter);
        if (table == NULL)
            goto failed;
        
        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_BOOLEAN) {
            ibus_lookup_table_unref (table);
            goto failed;
        }
        dbus_message_iter_get_basic (&iter, &visible);

        g_signal_emit (engine, engine_signals[UPDATE_LOOKUP_TABLE], 0, table, visible);
        ibus_lookup_table_unref (table);
    }
    else if (dbus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "RegisterProperties")) {
        IBusPropList *prop_list;
        DBusMessageIter iter;
        gboolean retval;

        retval = dbus_message_iter_init (message, &iter);
        if (!retval)
            goto failed;
        
        prop_list = ibus_prop_list_from_dbus_message (&iter);
        if (prop_list == NULL)
            goto failed;
        
        g_signal_emit (engine, engine_signals[REGISTER_PROPERTIES], 0, prop_list);
        ibus_prop_list_unref (prop_list);
    }
    else if (dbus_message_is_signal (message, IBUS_INTERFACE_ENGINE, "UpdateProperty")) {
        IBusProperty *prop;
        DBusMessageIter iter;
        gboolean retval;

        retval = dbus_message_iter_init (message, &iter);
        if (!retval)
            goto failed;
        
        prop = ibus_property_from_dbus_message (&iter);
        if (prop == NULL)
            goto failed;
        
        g_signal_emit (engine, engine_signals[UPDATE_PROPERTY], 0, prop);
        ibus_property_free (prop);
    }

handled:
    g_signal_stop_emission_by_name (engine, "dbus-signal");
    return TRUE;
  
failed:
    g_warning ("%s: %s", error.name, error.message);
    dbus_error_free (&error);
    return FALSE;
}

gboolean
bus_engine_proxy_process_key_event (BusEngineProxy *engine,
                                    guint32         keyval,
                                    gboolean        is_press,
                                    guint32         state)
{
    DBusMessage *reply_message;
    IBusError *error;
    gboolean retval;

    reply_message = ibus_proxy_call_with_reply_and_block (IBUS_PROXY (engine),
                                                  "ProcessKeyEvent",
                                                  -1,
                                                  &error,
                                                  DBUS_TYPE_UINT32, &keyval,
                                                  DBUS_TYPE_BOOLEAN, &is_press,
                                                  DBUS_TYPE_UINT32, &state,
                                                  DBUS_TYPE_INVALID);
    if (reply_message == NULL) {
        g_debug ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        retval = FALSE;
    }

    if (dbus_message_get_type (reply_message) == DBUS_MESSAGE_TYPE_ERROR) {
        g_debug ("%s",
                 dbus_message_get_error_name (reply_message));
        dbus_message_unref (reply_message);
        retval = FALSE;
    }
    else {
        DBusError error;
        dbus_error_init (&error);
        if (!dbus_message_get_args (reply_message, &error,
                                   DBUS_TYPE_BOOLEAN, &retval,
                                   DBUS_TYPE_INVALID)) {
            g_debug ("%s: %s", error.name, error.message);
            dbus_error_free (&error);
            retval = FALSE;
        }
        dbus_message_unref (reply_message);
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
    ibus_proxy_call (IBUS_PROXY (engine),
                     "SetCursorLocation",
                     DBUS_TYPE_INT32, &x,
                     DBUS_TYPE_INT32, &y,
                     DBUS_TYPE_INT32, &w,
                     DBUS_TYPE_INT32, &h,
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_focus_in (BusEngineProxy *engine)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "FocusIn",
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_focus_out (BusEngineProxy *engine)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "FocusOut",
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_reset (BusEngineProxy *engine)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "Reset",
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_page_up (BusEngineProxy *engine)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "PageUp",
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_page_down (BusEngineProxy *engine)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "PageDown",
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_cursor_up (BusEngineProxy *engine)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "CursorUp",
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_cursor_down (BusEngineProxy *engine)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "CursorDown",
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_enable (BusEngineProxy *engine)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "Enable",
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_disable (BusEngineProxy *engine)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "Disable",
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_property_activate (BusEngineProxy *engine,
                                    const gchar    *prop_name,
                                    gint32          state)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "PropertyActivate",
                     DBUS_TYPE_STRING, &prop_name,
                     DBUS_TYPE_INT32, &state,
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_property_show (BusEngineProxy *engine,
                                const gchar    *prop_name)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "PropertyShow",
                     DBUS_TYPE_STRING, &prop_name,
                     DBUS_TYPE_INVALID);
}

void bus_engine_proxy_property_hide (BusEngineProxy *engine,
                                     const gchar    *prop_name)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "PropertyHide",
                     DBUS_TYPE_STRING, &prop_name,
                     DBUS_TYPE_INVALID);
}

void
bus_engine_proxy_destroy (BusEngineProxy *engine)
{
    ibus_proxy_call (IBUS_PROXY (engine),
                     "Destroy",
                     DBUS_TYPE_INVALID);
}


