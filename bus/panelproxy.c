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
#include "panelproxy.h"

#define BUS_PANEL_PROXY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_PANEL_PROXY, BusPanelProxyPrivate))

enum {
    PAGE_UP,
    PAGE_DOWN,
    CURSOR_UP,
    CURSOR_DOWN,
    PROPERTY_ACTIVATE,
    PROPERTY_SHOW,
    PROPERTY_HIDE,
    LAST_SIGNAL,
};


/* BusPanelProxyPriv */
struct _BusPanelProxyPrivate {
    BusInputContext *focused_context;
};
typedef struct _BusPanelProxyPrivate BusPanelProxyPrivate;

static guint    panel_signals[LAST_SIGNAL] = { 0 };
// static guint            engine_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_panel_proxy_class_init      (BusPanelProxyClass    *klass);
static void     bus_panel_proxy_init            (BusPanelProxy         *panel);
static void     bus_panel_proxy_real_destroy    (BusPanelProxy         *panel);

static gboolean bus_panel_proxy_ibus_signal     (IBusProxy             *proxy,
                                                 IBusMessage            *message);

static IBusProxyClass  *parent_class = NULL;

GType
bus_panel_proxy_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusPanelProxyClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_panel_proxy_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusPanelProxy),
        0,
        (GInstanceInitFunc) bus_panel_proxy_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_PROXY,
                    "BusPanelProxy",
                    &type_info,
                    (GTypeFlags)0);
    }
    return type;
}

BusPanelProxy *
bus_panel_proxy_new (BusConnection *connection)
{
    g_assert (BUS_IS_CONNECTION (connection));
    
    GObject *obj;
    obj = g_object_new (BUS_TYPE_PANEL_PROXY,
                        "name", NULL,
                        "path", IBUS_PATH_PANEL,
                        "connection", connection,
                        NULL);

    return BUS_PANEL_PROXY (obj);
}

static void
bus_panel_proxy_class_init (BusPanelProxyClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusProxyClass *proxy_class = IBUS_PROXY_CLASS (klass);


    parent_class = (IBusProxyClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusPanelProxyPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_panel_proxy_real_destroy;

    proxy_class->ibus_signal = bus_panel_proxy_ibus_signal;
    
    /* install signals */
    panel_signals[PAGE_UP] =
        g_signal_new (I_("page-up"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
     
    panel_signals[PAGE_DOWN] =
        g_signal_new (I_("page-down"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
   
    panel_signals[CURSOR_UP] =
        g_signal_new (I_("cursor-up"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    panel_signals[CURSOR_DOWN] =
        g_signal_new (I_("cursor-down"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    panel_signals[PROPERTY_ACTIVATE] =
        g_signal_new (I_("property-activate"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING_INT,
            G_TYPE_NONE, 2,
            G_TYPE_STRING,
            G_TYPE_INT);

    panel_signals[PROPERTY_SHOW] =
        g_signal_new (I_("property-show"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING,
            G_TYPE_NONE, 1,
            G_TYPE_STRING);

    panel_signals[PROPERTY_HIDE] =
        g_signal_new (I_("property-hide"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING,
            G_TYPE_NONE, 1,
            G_TYPE_STRING);

}

static void
bus_panel_proxy_init (BusPanelProxy *panel)
{
    BusPanelProxyPrivate *priv;
    priv = BUS_PANEL_PROXY_GET_PRIVATE (panel);

    priv->focused_context = NULL;
}

static void
bus_panel_proxy_real_destroy (BusPanelProxy *panel)
{
    BusPanelProxyPrivate *priv;
    priv = BUS_PANEL_PROXY_GET_PRIVATE (panel);
    
    ibus_proxy_call (IBUS_PROXY (panel),
                     "Destroy",
                     DBUS_TYPE_INVALID);

    if (priv->focused_context) {
        g_object_unref (priv->focused_context);
        priv->focused_context = NULL;
    }
    
    IBUS_OBJECT_CLASS(parent_class)->destroy (IBUS_OBJECT (panel));
}

static gboolean
bus_panel_proxy_ibus_signal (IBusProxy      *proxy,
                             IBusMessage    *message)
{
    g_assert (BUS_IS_PANEL_PROXY (proxy));
    g_assert (message != NULL);
    
    BusPanelProxy *panel;
    IBusError *error;
    gint i;

    static const struct {
        const gchar *member;
        const guint signal_id;
    } signals [] = {
        { "PageUp",         PAGE_UP },
        { "PageDown",       PAGE_DOWN },
        { "CursorUp",       CURSOR_UP },
        { "CursorDown",     CURSOR_DOWN },
        { NULL, 0},
    };

    panel = BUS_PANEL_PROXY (proxy);
    
    for (i = 0; ; i++) {
        if (signals[i].member == NULL)
            break;
        if (ibus_message_is_signal (message, IBUS_INTERFACE_ENGINE, signals[i].member)) {
            g_signal_emit (panel, panel_signals[signals[i].signal_id], 0);
            goto handled;
        }
    }
    
    if (ibus_message_is_signal (message, IBUS_INTERFACE_PANEL, "PropertyActivate")) {
        gchar *prop_name;
        gint prop_state;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &prop_name,
                                        G_TYPE_INT, &prop_state,
                                        G_TYPE_INVALID);
        if (!retval)
            goto failed;

        g_signal_emit (panel, panel_signals[PROPERTY_ACTIVATE], 0, prop_name, prop_state);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_PANEL, "PropertyShow")) {
        gchar *prop_name;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &prop_name,
                                        G_TYPE_INVALID);
        if (!retval)
            goto failed;
        g_signal_emit (panel, panel_signals[PROPERTY_SHOW], 0, prop_name);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_PANEL, "PropertyHide")) {
        gchar *prop_name;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &prop_name,
                                        G_TYPE_INVALID);
        if (!retval)
            goto failed;
        g_signal_emit (panel, panel_signals[PROPERTY_HIDE], 0, prop_name);
    }

handled:
    g_signal_stop_emission_by_name (panel, "dbus-signal");
    return TRUE;
  
failed:
    g_warning ("%s: %s", error->name, error->message);
    ibus_error_free (error);
    return FALSE;
}


void
bus_panel_proxy_focus_in (BusPanelProxy     *panel,
                          BusInputContext   *context)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusPanelProxyPrivate *priv;
    priv = BUS_PANEL_PROXY_GET_PRIVATE (panel);

    if (priv->focused_context == context)
        return;

    if (priv->focused_context != NULL)
        bus_panel_proxy_focus_out (panel, context);

    g_object_ref (context);
    priv->focused_context = context;
}

void
bus_panel_proxy_focus_out (BusPanelProxy    *panel,
                           BusInputContext  *context)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    
    BusPanelProxyPrivate *priv;
    priv = BUS_PANEL_PROXY_GET_PRIVATE (panel);

    g_assert (priv->focused_context == context);
    
    g_object_unref (priv->focused_context);
    priv->focused_context = NULL;
}

void
bus_panel_proxy_set_cursor_location (BusPanelProxy *panel,
                                     gint32         x,
                                     gint32         y,
                                     gint32         w,
                                     gint32         h)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));

    ibus_proxy_call (IBUS_PROXY (panel),
                     "SetCursorLocation",
                     G_TYPE_INT, &x,
                     G_TYPE_INT, &y,
                     G_TYPE_INT, &w,
                     G_TYPE_INT, &h,
                     G_TYPE_INVALID);
}

void
bus_panel_proxy_update_preedit (BusPanelProxy   *panel,
                                const gchar     *text,
                                IBusAttrList    *attr_list,
                                gint             cursor_pos,
                                gboolean         visible)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (text != NULL);
    g_assert (attr_list != NULL);

    ibus_proxy_call (IBUS_PROXY (panel),
                     "UpdatePreedit",
                     G_TYPE_STRING, &text,
                     IBUS_TYPE_ATTR_LIST, &attr_list,
                     G_TYPE_INT, &cursor_pos,
                     G_TYPE_BOOLEAN, &visible,
                     G_TYPE_INVALID);
}

void
bus_panel_proxy_update_aux_string (BusPanelProxy *panel,
                                   const gchar   *text,
                                   IBusAttrList  *attr_list,
                                   gboolean       visible)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (text != NULL);
    g_assert (attr_list != NULL);

    ibus_proxy_call (IBUS_PROXY (panel),
                     "UpdateAuxString",    
                     G_TYPE_STRING, &text,
                     IBUS_TYPE_ATTR_LIST, &attr_list,
                     G_TYPE_BOOLEAN, &visible,
                     G_TYPE_INVALID);
}

void
bus_panel_proxy_update_lookup_table (BusPanelProxy   *panel,
                                     IBusLookupTable *table,
                                     gboolean         visible)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (table != NULL);

    ibus_proxy_call (IBUS_PROXY (panel),
                     "UpdateLookupTable",    
                     IBUS_TYPE_LOOKUP_TABLE, &table,
                     G_TYPE_BOOLEAN, &visible,
                     G_TYPE_INVALID);
}

void
bus_panel_proxy_register_properties (BusPanelProxy  *panel,
                                     IBusPropList   *prop_list)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (prop_list != NULL);

    ibus_proxy_call (IBUS_PROXY (panel),
                     "RegisterProperties",
                     IBUS_TYPE_PROP_LIST, &prop_list,
                     G_TYPE_INVALID);
}

void
bus_panel_proxy_update_property (BusPanelProxy  *panel,
                                 IBusProperty   *prop)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (prop != NULL);
    
    ibus_proxy_call (IBUS_PROXY (panel),
                     "UpdateProperty",
                     IBUS_TYPE_PROPERTY, &prop,
                     G_TYPE_INVALID);
}

#define DEFINE_FUNCTION(Name, name)                     \
    void bus_panel_proxy_##name (BusPanelProxy *panel)  \
    {                                                   \
        g_assert (BUS_IS_PANEL_PROXY (panel));          \
        ibus_proxy_call (IBUS_PROXY (panel),            \
                     #Name,                             \
                     G_TYPE_INVALID);                   \
    }

DEFINE_FUNCTION (ShowPreedit, show_preedit)
DEFINE_FUNCTION (HidePreedit, hide_preedit)
DEFINE_FUNCTION (ShowAuxString, show_aux_string)
DEFINE_FUNCTION (HideAuxString, hide_aux_string)
DEFINE_FUNCTION (ShowLookupTable, show_lookup_table)
DEFINE_FUNCTION (HideLookupTable, hide_lookup_table)
DEFINE_FUNCTION (PageUpLookupTable, page_up_lookup_table)
DEFINE_FUNCTION (PageDownLookupTable, page_down_lookup_table)
DEFINE_FUNCTION (CursorUpLookupTable, cursor_up_lookup_table)
DEFINE_FUNCTION (CursorDownLookupTable, cursor_down_lookup_table)

#undef DEFINE_FUNCTION
