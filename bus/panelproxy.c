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
#include <dbus/dbus.h>
#include <ibusinternal.h>
#include <ibusmarshalers.h>
#include "panelproxy.h"

enum {
    PAGE_UP,
    PAGE_DOWN,
    CURSOR_UP,
    CURSOR_DOWN,
    CANDIDATE_CLICKED,
    PROPERTY_ACTIVATE,
    PROPERTY_SHOW,
    PROPERTY_HIDE,
    LAST_SIGNAL,
};

static guint    panel_signals[LAST_SIGNAL] = { 0 };
// static guint            engine_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_panel_proxy_init            (BusPanelProxy          *panel);
static void     bus_panel_proxy_real_destroy    (BusPanelProxy          *panel);

static gboolean bus_panel_proxy_ibus_signal     (IBusProxy              *proxy,
                                                 IBusMessage            *message);
static void     bus_panel_proxy_page_up         (BusPanelProxy          *panel);
static void     bus_panel_proxy_page_down       (BusPanelProxy          *panel);
static void     bus_panel_proxy_cursor_up       (BusPanelProxy          *panel);
static void     bus_panel_proxy_cursor_down     (BusPanelProxy          *panel);
static void     bus_panel_proxy_candidate_clicked
                                                (BusPanelProxy          *panel,
                                                 guint                   index,
                                                 guint                   button,
                                                 guint                   state);
static void     bus_panel_proxy_property_activate
                                                (BusPanelProxy          *panel,
                                                 const gchar            *prop_name,
                                                 gint                    prop_state);


G_DEFINE_TYPE(BusPanelProxy, bus_panel_proxy, IBUS_TYPE_PROXY)

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

    klass->page_up     = bus_panel_proxy_page_up;
    klass->page_down   = bus_panel_proxy_page_down;
    klass->cursor_up   = bus_panel_proxy_cursor_up;
    klass->cursor_down = bus_panel_proxy_cursor_down;

    klass->candidate_clicked = bus_panel_proxy_candidate_clicked;
    klass->property_activate = bus_panel_proxy_property_activate;

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_panel_proxy_real_destroy;

    proxy_class->ibus_signal = bus_panel_proxy_ibus_signal;

    /* install signals */
    panel_signals[PAGE_UP] =
        g_signal_new (I_("page-up"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(BusPanelProxyClass, page_up),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    panel_signals[PAGE_DOWN] =
        g_signal_new (I_("page-down"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(BusPanelProxyClass, page_down),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    panel_signals[CURSOR_UP] =
        g_signal_new (I_("cursor-up"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(BusPanelProxyClass, cursor_up),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    panel_signals[CURSOR_DOWN] =
        g_signal_new (I_("cursor-down"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(BusPanelProxyClass, cursor_down),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    panel_signals[CANDIDATE_CLICKED] =
        g_signal_new (I_("candidate-clicked"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(BusPanelProxyClass, candidate_clicked),
            NULL, NULL,
            ibus_marshal_VOID__UINT_UINT_UINT,
            G_TYPE_NONE, 3,
            G_TYPE_UINT,
            G_TYPE_UINT,
            G_TYPE_UINT);

    panel_signals[PROPERTY_ACTIVATE] =
        g_signal_new (I_("property-activate"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(BusPanelProxyClass, property_activate),
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
    panel->focused_context = NULL;
}

static void
bus_panel_proxy_real_destroy (BusPanelProxy *panel)
{
    if (ibus_proxy_get_connection ((IBusProxy *)panel) != NULL) {
        ibus_proxy_call ((IBusProxy *) panel,
                         "Destroy",
                         G_TYPE_INVALID);
    }

    if (panel->focused_context) {
        bus_panel_proxy_focus_out (panel, panel->focused_context);
        panel->focused_context = NULL;
    }

    IBUS_OBJECT_CLASS(bus_panel_proxy_parent_class)->destroy (IBUS_OBJECT (panel));
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
    };

    panel = BUS_PANEL_PROXY (proxy);

    for (i = 0; i < G_N_ELEMENTS (signals); i++) {
        if (ibus_message_is_signal (message, IBUS_INTERFACE_PANEL, signals[i].member)) {
            g_signal_emit (panel, panel_signals[signals[i].signal_id], 0);
            goto handled;
        }
    }

    if (ibus_message_is_signal (message, IBUS_INTERFACE_PANEL, "CandidateClicked")) {
        guint index, button, state;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_UINT, &index,
                                        G_TYPE_UINT, &button,
                                        G_TYPE_UINT, &state,
                                        G_TYPE_INVALID);
        if (!retval)
            goto failed;

        g_signal_emit (panel, panel_signals[CANDIDATE_CLICKED], 0, index, button, state);
    }
    else if (ibus_message_is_signal (message, IBUS_INTERFACE_PANEL, "PropertyActivate")) {
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
    g_signal_stop_emission_by_name (panel, "ibus-signal");
    return TRUE;

failed:
    g_warning ("%s: %s", error->name, error->message);
    ibus_error_free (error);
    return FALSE;
}



void
bus_panel_proxy_set_cursor_location (BusPanelProxy *panel,
                                     gint           x,
                                     gint           y,
                                     gint           w,
                                     gint           h)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));

    ibus_proxy_call ((IBusProxy *) panel,
                     "SetCursorLocation",
                     G_TYPE_INT, &x,
                     G_TYPE_INT, &y,
                     G_TYPE_INT, &w,
                     G_TYPE_INT, &h,
                     G_TYPE_INVALID);
}

void
bus_panel_proxy_update_preedit_text (BusPanelProxy  *panel,
                                     IBusText       *text,
                                     guint           cursor_pos,
                                     gboolean        visible)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (text != NULL);

    ibus_proxy_call ((IBusProxy *) panel,
                     "UpdatePreeditText",
                     IBUS_TYPE_TEXT, &text,
                     G_TYPE_UINT, &cursor_pos,
                     G_TYPE_BOOLEAN, &visible,
                     G_TYPE_INVALID);
}

void
bus_panel_proxy_update_auxiliary_text (BusPanelProxy *panel,
                                       IBusText      *text,
                                       gboolean       visible)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (text != NULL);

    ibus_proxy_call ((IBusProxy *) panel,
                     "UpdateAuxiliaryText",
                     IBUS_TYPE_TEXT, &text,
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

    ibus_proxy_call ((IBusProxy *) panel,
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

    ibus_proxy_call ((IBusProxy *) panel,
                     "RegisterProperties",
                     IBUS_TYPE_PROP_LIST, &prop_list,
                     G_TYPE_INVALID);
    ibus_connection_flush (ibus_proxy_get_connection((IBusProxy *)panel));
}

void
bus_panel_proxy_update_property (BusPanelProxy  *panel,
                                 IBusProperty   *prop)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (prop != NULL);

    ibus_proxy_call ((IBusProxy *) panel,
                     "UpdateProperty",
                     IBUS_TYPE_PROPERTY, &prop,
                     G_TYPE_INVALID);
}

#define DEFINE_FUNC(name)                                       \
    static void                                                 \
    bus_panel_proxy_##name (BusPanelProxy *panel)               \
    {                                                           \
        g_assert (BUS_IS_PANEL_PROXY (panel));                  \
                                                                \
        if (panel->focused_context) {                           \
            bus_input_context_##name (panel->focused_context);  \
        }                                                       \
    }

DEFINE_FUNC(page_up)
DEFINE_FUNC(page_down)
DEFINE_FUNC(cursor_up)
DEFINE_FUNC(cursor_down)
#undef DEFINE_FUNC

static void
bus_panel_proxy_candidate_clicked (BusPanelProxy *panel,
                                   guint          index,
                                   guint          button,
                                   guint          state)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));

    if (panel->focused_context) {
        bus_input_context_candidate_clicked (panel->focused_context,
                                             index,
                                             button,
                                             state);
    }
}

static void
bus_panel_proxy_property_activate (BusPanelProxy *panel,
                                   const gchar   *prop_name,
                                   gint          prop_state)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));

    if (panel->focused_context) {
        bus_input_context_property_activate (panel->focused_context, prop_name, prop_state);
    }
}

#define DEFINE_FUNCTION(Name, name)                     \
    void bus_panel_proxy_##name (BusPanelProxy *panel)  \
    {                                                   \
        g_assert (BUS_IS_PANEL_PROXY (panel));          \
        ibus_proxy_call ((IBusProxy *) panel,           \
                     #Name,                             \
                     G_TYPE_INVALID);                   \
    }

DEFINE_FUNCTION (ShowPreeditText, show_preedit_text)
DEFINE_FUNCTION (HidePreeditText, hide_preedit_text)
DEFINE_FUNCTION (ShowAuxiliaryText, show_auxiliary_text)
DEFINE_FUNCTION (HideAuxiliaryText, hide_auxiliary_text)
DEFINE_FUNCTION (ShowLookupTable, show_lookup_table)
DEFINE_FUNCTION (HideLookupTable, hide_lookup_table)
DEFINE_FUNCTION (PageUpLookupTable, page_up_lookup_table)
DEFINE_FUNCTION (PageDownLookupTable, page_down_lookup_table)
DEFINE_FUNCTION (CursorUpLookupTable, cursor_up_lookup_table)
DEFINE_FUNCTION (CursorDownLookupTable, cursor_down_lookup_table)
DEFINE_FUNCTION (StateChanged, state_changed)

#undef DEFINE_FUNCTION

static void
_context_set_cursor_location_cb (BusInputContext *context,
                                 gint             x,
                                 gint             y,
                                 gint             w,
                                 gint             h,
                                 BusPanelProxy   *panel)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (BUS_IS_PANEL_PROXY (panel));

    g_return_if_fail (panel->focused_context == context);

    bus_panel_proxy_set_cursor_location (panel, x, y, w, h);
}

static void
_context_update_preedit_text_cb (BusInputContext *context,
                                 IBusText        *text,
                                 guint            cursor_pos,
                                 gboolean         visible,
                                 BusPanelProxy   *panel)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (text != NULL);
    g_assert (BUS_IS_PANEL_PROXY (panel));

    g_return_if_fail (panel->focused_context == context);

    bus_panel_proxy_update_preedit_text (panel,
                                         text,
                                         cursor_pos,
                                         visible);
}

static void
_context_update_auxiliary_text_cb (BusInputContext *context,
                                   IBusText        *text,
                                   gboolean         visible,
                                   BusPanelProxy   *panel)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (BUS_IS_PANEL_PROXY (panel));

    g_return_if_fail (panel->focused_context == context);

    bus_panel_proxy_update_auxiliary_text (panel,
                                           text,
                                           visible);
}

static void
_context_update_lookup_table_cb (BusInputContext *context,
                                 IBusLookupTable *table,
                                 gboolean         visible,
                                 BusPanelProxy   *panel)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (BUS_IS_PANEL_PROXY (panel));

    g_return_if_fail (panel->focused_context == context);

    bus_panel_proxy_update_lookup_table (panel,
                                         table,
                                         visible);
}

static void
_context_register_properties_cb (BusInputContext *context,
                                 IBusPropList    *prop_list,
                                 BusPanelProxy   *panel)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (BUS_IS_PANEL_PROXY (panel));

    g_return_if_fail (panel->focused_context == context);

    bus_panel_proxy_register_properties (panel,
                                         prop_list);
}

static void
_context_update_property_cb (BusInputContext *context,
                             IBusProperty    *prop,
                             BusPanelProxy   *panel)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (BUS_IS_PANEL_PROXY (panel));

    g_return_if_fail (panel->focused_context == context);

    bus_panel_proxy_update_property (panel,
                                     prop);
}

#if 0
static void
_context_destroy_cb (BusInputContext *context,
                     BusPanelProxy   *panel)
{
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    g_assert (BUS_IS_PANEL_PROXY (panel));

    g_assert (context == panel->focused_context);

    bus_panel_proxy_focus_out (panel, context);
}
#endif

#define DEFINE_FUNCTION(name)                                   \
    static void _context_##name##_cb (BusInputContext *context, \
                                      BusPanelProxy   *panel)   \
    {                                                           \
        g_assert (BUS_IS_INPUT_CONTEXT (context));              \
        g_assert (BUS_IS_PANEL_PROXY (panel));                  \
                                                                \
        g_return_if_fail (panel->focused_context == context);   \
                                                                \
        bus_panel_proxy_##name (panel);                         \
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
DEFINE_FUNCTION (state_changed)

#undef DEFINE_FUNCTION

static const struct _SignalCallbackTable {
    gchar *name;
    GCallback callback;
} __signals[] = {
    { "set-cursor-location",        G_CALLBACK (_context_set_cursor_location_cb) },

    { "update-preedit-text",        G_CALLBACK (_context_update_preedit_text_cb) },
    { "show-preedit-text",          G_CALLBACK (_context_show_preedit_text_cb) },
    { "hide-preedit-text",          G_CALLBACK (_context_hide_preedit_text_cb) },

    { "update-auxiliary-text",      G_CALLBACK (_context_update_auxiliary_text_cb) },
    { "show-auxiliary-text",        G_CALLBACK (_context_show_auxiliary_text_cb) },
    { "hide-auxiliary-text",        G_CALLBACK (_context_hide_auxiliary_text_cb) },

    { "update-lookup-table",        G_CALLBACK (_context_update_lookup_table_cb) },
    { "show-lookup-table",          G_CALLBACK (_context_show_lookup_table_cb) },
    { "hide-lookup-table",          G_CALLBACK (_context_hide_lookup_table_cb) },
    { "page-up-lookup-table",       G_CALLBACK (_context_page_up_lookup_table_cb) },
    { "page-down-lookup-table",     G_CALLBACK (_context_page_down_lookup_table_cb) },
    { "cursor-up-lookup-table",     G_CALLBACK (_context_cursor_up_lookup_table_cb) },
    { "cursor-down-lookup-table",   G_CALLBACK (_context_cursor_down_lookup_table_cb) },

    { "register-properties",        G_CALLBACK (_context_register_properties_cb) },
    { "update-property",            G_CALLBACK (_context_update_property_cb) },

    { "enabled",                    G_CALLBACK (_context_state_changed_cb) },
    { "disabled",                   G_CALLBACK (_context_state_changed_cb) },
    { "engine-changed",             G_CALLBACK (_context_state_changed_cb) },

    //    { "destroy",                    G_CALLBACK (_context_destroy_cb) },
};

void
bus_panel_proxy_focus_in (BusPanelProxy     *panel,
                          BusInputContext   *context)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (panel->focused_context == context)
        return;

    if (panel->focused_context != NULL)
        bus_panel_proxy_focus_out (panel, panel->focused_context);

    g_object_ref_sink (context);
    panel->focused_context = context;

    const gchar *path = ibus_service_get_path ((IBusService *)context);

    ibus_proxy_call ((IBusProxy *) panel,
                     "FocusIn",
                     IBUS_TYPE_OBJECT_PATH, &path,
                     G_TYPE_INVALID);

    /* install signal handlers */
    gint i;
    for (i = 0; i < G_N_ELEMENTS (__signals); i++) {
        g_signal_connect (context,
                          __signals[i].name,
                          __signals[i].callback,
                          panel);
    }
}

void
bus_panel_proxy_focus_out (BusPanelProxy    *panel,
                           BusInputContext  *context)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    g_assert (panel->focused_context == context);

    /* uninstall signal handlers */
    gint i;
    for (i = 0; i < G_N_ELEMENTS (__signals); i++) {
        g_signal_handlers_disconnect_by_func (context,
                                              __signals[i].callback,
                                              panel);
    }

    const gchar *path = ibus_service_get_path ((IBusService *)context);

    ibus_proxy_call ((IBusProxy *) panel,
                     "FocusOut",
                     IBUS_TYPE_OBJECT_PATH, &path,
                     G_TYPE_INVALID);

    g_object_unref (panel->focused_context);
    panel->focused_context = NULL;
}

