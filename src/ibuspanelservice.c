/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (c) 2009, Google Inc. All rights reserved.
 * Copyright (C) 2010 Peng Huang <shawn.p.huang@gmail.com>
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
#include "ibuspanelservice.h"

enum {
    LAST_SIGNAL,
};

enum {
    PROP_0,
};

/* functions prototype */
static void      ibus_panel_service_set_property          (IBusPanelService       *panel,
                                                           guint                   prop_id,
                                                           const GValue           *value,
                                                           GParamSpec             *pspec);
static void      ibus_panel_service_get_property          (IBusPanelService       *panel,
                                                           guint                   prop_id,
                                                           GValue                 *value,
                                                           GParamSpec             *pspec);
static void      ibus_panel_service_real_destroy          (IBusPanelService       *panel);
static void      ibus_panel_service_service_method_call   (IBusService            *service,
                                                           GDBusConnection        *connection,
                                                           const gchar            *sender,
                                                           const gchar            *object_path,
                                                           const gchar            *interface_name,
                                                           const gchar            *method_name,
                                                           GVariant               *parameters,
                                                           GDBusMethodInvocation  *invocation);
static GVariant *ibus_panel_service_service_get_property  (IBusService            *service,
                                                           GDBusConnection        *connection,
                                                           const gchar            *sender,
                                                           const gchar            *object_path,
                                                           const gchar            *interface_name,
                                                           const gchar            *property_name,
                                                           GError                **error);
static gboolean  ibus_panel_service_service_set_property  (IBusService            *service,
                                                           GDBusConnection        *connection,
                                                           const gchar            *sender,
                                                           const gchar            *object_path,
                                                           const gchar            *interface_name,
                                                           const gchar            *property_name,
                                                           GVariant               *value,
                                                           GError                **error);
static void      ibus_panel_service_not_implemented       (IBusPanelService      *panel);
static void      ibus_panel_service_focus_in              (IBusPanelService      *panel,
                                                           const gchar           *input_context_path);
static void      ibus_panel_service_focus_out             (IBusPanelService      *panel,
                                                           const gchar           *input_context_path);
static void      ibus_panel_service_register_properties   (IBusPanelService      *panel,
                                                           IBusPropList          *prop_list);
static void      ibus_panel_service_set_cursor_location   (IBusPanelService      *panel,
                                                           gint                   x,
                                                           gint                   y,
                                                           gint                   w,
                                                           gint                   h);
static void      ibus_panel_service_update_auxiliary_text (IBusPanelService      *panel,
                                                           IBusText              *text,
                                                           gboolean               visible);
static void      ibus_panel_service_update_lookup_table   (IBusPanelService      *panel,
                                                           IBusLookupTable       *lookup_table,
                                                           gboolean               visible);
static void      ibus_panel_service_update_preedit_text   (IBusPanelService      *panel,
                                                           IBusText              *text,
                                                           guint                  cursor_pos,
                                                           gboolean               visible);
static void      ibus_panel_service_update_property       (IBusPanelService      *panel,
                                                           IBusProperty          *prop);

G_DEFINE_TYPE (IBusPanelService, ibus_panel_service, IBUS_TYPE_SERVICE)

static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='org.freedesktop.IBus.Panel'>"
    /* Methods */
    "    <method name='UpdatePreeditText'>"
    "      <arg direction='in'  type='v' name='text' />"
    "      <arg direction='in'  type='u' name='cursor_pos' />"
    "      <arg direction='in'  type='b' name='visible' />"
    "    </method>"
    "    <method name='ShowPreeditText' />"
    "    <method name='HidePreeditText' />"
    "    <method name='UpdateAuxiliaryText'>"
    "      <arg direction='in'  type='v' name='text' />"
    "      <arg direction='in'  type='b' name='visible' />"
    "    </method>"
    "    <method name='ShowAuxiliaryText' />"
    "    <method name='HideAuxiliaryText' />"
    "    <method name='UpdateLookupTable'>"
    "      <arg direction='in' type='v' name='table' />"
    "      <arg direction='in' type='b' name='visible' />"
    "    </method>"
    "    <method name='ShowLookupTable' />"
    "    <method name='HideLookupTable' />"
    "    <method name='CursorUpLookupTable' />"
    "    <method name='CursorDownLookupTable' />"
    "    <method name='PageUpLookupTable' />"
    "    <method name='PageDownLookupTable' />"
    "    <method name='RegisterProperties'>"
    "      <arg direction='in'  type='v' name='props' />"
    "    </method>"
    "    <method name='UpdateProperty'>"
    "      <arg direction='in'  type='v' name='prop' />"
    "    </method>"
    "    <method name='FocusIn'>"
    "      <arg direction='in'  type='o' name='ic' />"
    "    </method>"
    "    <method name='FocusOut'>"
    "      <arg direction='in'  type='o' name='ic' />"
    "    </method>"
    "    <method name='SetCursorLocation'>"
    "      <arg direction='in' type='i' name='x' />"
    "      <arg direction='in' type='i' name='y' />"
    "      <arg direction='in' type='i' name='w' />"
    "      <arg direction='in' type='i' name='h' />"
    "    </method>"
    "    <method name='Reset' />"
    "    <method name='StartSetup' />"
    "    <method name='StateChanged' />"
    "    <method name='HideLanguageBar' />"
    "    <method name='ShowLanguageBar' />"
    /* Signals */
    "    <signal name='CursorUp' />"
    "    <signal name='CursorDown' />"
    "    <signal name='PageUp' />"
    "    <signal name='PageDown' />"
    "    <signal name='PropertyActivate'>"
    "      <arg type='s' name='prop_name' />"
    "      <arg type='i' name='prop_state' />"
    "    </signal>"
    "    <signal name='PropertyShow'>"
    "      <arg type='s' name='prop_name' />"
    "    </signal>"
    "    <signal name='PropertyHide'>"
    "      <arg type='s' name='prop_name' />"
    "    </signal>"
    "    <signal name='CandidateClicked'>"
    "      <arg type='u' name='index' />"
    "      <arg type='u' name='button' />"
    "      <arg type='u' name='state' />"
    "    </signal>"
    "  </interface>"
    "</node>";

static void
ibus_panel_service_class_init (IBusPanelServiceClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);

    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_panel_service_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_panel_service_get_property;

    IBUS_OBJECT_CLASS (gobject_class)->destroy = (IBusObjectDestroyFunc) ibus_panel_service_real_destroy;

    IBUS_SERVICE_CLASS (class)->service_method_call  = ibus_panel_service_service_method_call;
    IBUS_SERVICE_CLASS (class)->service_get_property = ibus_panel_service_service_get_property;
    IBUS_SERVICE_CLASS (class)->service_set_property = ibus_panel_service_service_set_property;

    ibus_service_class_add_interfaces (IBUS_SERVICE_CLASS (class), introspection_xml);

    class->focus_in              = ibus_panel_service_focus_in;
    class->focus_out             = ibus_panel_service_focus_out;
    class->register_properties   = ibus_panel_service_register_properties;
    class->set_cursor_location   = ibus_panel_service_set_cursor_location;
    class->update_lookup_table   = ibus_panel_service_update_lookup_table;
    class->update_auxiliary_text = ibus_panel_service_update_auxiliary_text;
    class->update_preedit_text   = ibus_panel_service_update_preedit_text;
    class->update_property       = ibus_panel_service_update_property;

    class->cursor_down_lookup_table = ibus_panel_service_not_implemented;
    class->cursor_up_lookup_table   = ibus_panel_service_not_implemented;
    class->hide_auxiliary_text      = ibus_panel_service_not_implemented;
    class->hide_language_bar        = ibus_panel_service_not_implemented;
    class->hide_lookup_table        = ibus_panel_service_not_implemented;
    class->hide_preedit_text        = ibus_panel_service_not_implemented;
    class->page_down_lookup_table   = ibus_panel_service_not_implemented;
    class->page_up_lookup_table     = ibus_panel_service_not_implemented;
    class->reset                    = ibus_panel_service_not_implemented;
    class->show_auxiliary_text      = ibus_panel_service_not_implemented;
    class->show_language_bar        = ibus_panel_service_not_implemented;
    class->show_lookup_table        = ibus_panel_service_not_implemented;
    class->show_preedit_text        = ibus_panel_service_not_implemented;
    class->start_setup              = ibus_panel_service_not_implemented;
    class->state_changed            = ibus_panel_service_not_implemented;
}

static void
ibus_panel_service_init (IBusPanelService *panel)
{
}

static void
ibus_panel_service_set_property (IBusPanelService *panel,
                                 guint             prop_id,
                                 const GValue     *value,
                                 GParamSpec       *pspec)
{
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (panel, prop_id, pspec);
    }
}

static void
ibus_panel_service_get_property (IBusPanelService *panel,
                                 guint             prop_id,
                                 GValue           *value,
                                 GParamSpec       *pspec)
{
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (panel, prop_id, pspec);
    }
}

static void
ibus_panel_service_real_destroy (IBusPanelService *panel)
{
    IBUS_OBJECT_CLASS(ibus_panel_service_parent_class)->destroy (IBUS_OBJECT (panel));
}


static void
_g_object_unref_if_floating (gpointer instance)
{
    if (g_object_is_floating (instance))
        g_object_unref (instance);
}

static void
ibus_panel_service_service_method_call (IBusService           *service,
                                        GDBusConnection       *connection,
                                        const gchar           *sender,
                                        const gchar           *object_path,
                                        const gchar           *interface_name,
                                        const gchar           *method_name,
                                        GVariant              *parameters,
                                        GDBusMethodInvocation *invocation)
{
    IBusPanelService *panel = IBUS_PANEL_SERVICE (service);

    if (g_strcmp0 (interface_name, IBUS_INTERFACE_PANEL) != 0) {
        IBUS_SERVICE_CLASS (ibus_panel_service_parent_class)->
                service_method_call (service,
                                     connection,
                                     sender,
                                     object_path,
                                     interface_name,
                                     method_name,
                                     parameters,
                                     invocation);
        return;
    }

    if (g_strcmp0 (method_name, "UpdatePreeditText") == 0) {
        GVariant *variant = NULL;
        guint cursor = 0;
        gboolean visible = FALSE;

        g_variant_get (parameters, "(vub)", &variant, &cursor, &visible);
        IBusText *text = IBUS_TEXT (ibus_serializable_deserialize (variant));
        g_variant_unref (variant);

        IBUS_PANEL_SERVICE_GET_CLASS (panel)->update_preedit_text (panel, text, cursor, visible);
        _g_object_unref_if_floating (text);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "UpdateAuxiliaryText") == 0) {
        GVariant *variant = NULL;
        gboolean visible = FALSE;

        g_variant_get (parameters, "(vb)", &variant, &visible);
        IBusText *text = IBUS_TEXT (ibus_serializable_deserialize (variant));
        g_variant_unref (variant);

        IBUS_PANEL_SERVICE_GET_CLASS (panel)->update_auxiliary_text (panel, text, visible);
        _g_object_unref_if_floating (text);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "UpdateLookupTable") == 0) {
        GVariant *variant = NULL;
        gboolean visible = FALSE;

        g_variant_get (parameters, "(vb)", &variant, &visible);
        IBusLookupTable *table = IBUS_LOOKUP_TABLE (ibus_serializable_deserialize (variant));
        g_variant_unref (variant);

        IBUS_PANEL_SERVICE_GET_CLASS (panel)->update_lookup_table (panel, table, visible);
        _g_object_unref_if_floating (table);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "FocusIn") == 0) {
        const gchar *path;
        g_variant_get (parameters, "(&o)", &path);
        IBUS_PANEL_SERVICE_GET_CLASS (panel)->focus_in (panel, path);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "FocusOut") == 0) {
        const gchar *path;
        g_variant_get (parameters, "(&o)", &path);
        IBUS_PANEL_SERVICE_GET_CLASS (panel)->focus_out (panel, path);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "RegisterProperties") == 0) {
        GVariant *variant = g_variant_get_child_value (parameters, 0);
        IBusPropList *prop_list = IBUS_PROP_LIST (ibus_serializable_deserialize (variant));
        g_variant_unref (variant);

        IBUS_PANEL_SERVICE_GET_CLASS (panel)->register_properties (panel, prop_list);
        _g_object_unref_if_floating (prop_list);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "UpdateProperty") == 0) {
        GVariant *variant = g_variant_get_child_value (parameters, 0);
        IBusProperty *property = IBUS_PROPERTY (ibus_serializable_deserialize (variant));
        g_variant_unref (variant);

        IBUS_PANEL_SERVICE_GET_CLASS (panel)->update_property (panel, property);
        _g_object_unref_if_floating (property);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "SetCursorLocation") == 0) {
        guint x, y, w, h;
        g_variant_get (parameters, "(uuuu)", &x, &y, &w, &h);
        IBUS_PANEL_SERVICE_GET_CLASS (panel)->set_cursor_location (panel, x, y, w, h);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    const static struct {
        const gchar *name;
        const gint offset;
    } no_arg_methods [] = {
        { "CursorUpLookupTable"  , G_STRUCT_OFFSET (IBusPanelServiceClass, cursor_down_lookup_table) },
        { "CursorDownLookupTable", G_STRUCT_OFFSET (IBusPanelServiceClass, cursor_up_lookup_table) },
        { "HideAuxiliaryText",     G_STRUCT_OFFSET (IBusPanelServiceClass, hide_auxiliary_text) },
        { "HideLanguageBar",       G_STRUCT_OFFSET (IBusPanelServiceClass, hide_language_bar) },
        { "HideLookupTable",       G_STRUCT_OFFSET (IBusPanelServiceClass, hide_lookup_table) },
        { "HidePreeditText",       G_STRUCT_OFFSET (IBusPanelServiceClass, hide_preedit_text) },
        { "PageDownLookupTable",   G_STRUCT_OFFSET (IBusPanelServiceClass, page_down_lookup_table) },
        { "PageUpLookupTable",     G_STRUCT_OFFSET (IBusPanelServiceClass, page_up_lookup_table) },
        { "Reset",                 G_STRUCT_OFFSET (IBusPanelServiceClass, reset) },
        { "ShowAuxiliaryText",     G_STRUCT_OFFSET (IBusPanelServiceClass, show_auxiliary_text) },
        { "ShowLanguageBar",       G_STRUCT_OFFSET (IBusPanelServiceClass, show_language_bar) },
        { "ShowLookupTable",       G_STRUCT_OFFSET (IBusPanelServiceClass, show_lookup_table) },
        { "ShowPreeditText",       G_STRUCT_OFFSET (IBusPanelServiceClass, show_preedit_text) },
        { "StartSetup",            G_STRUCT_OFFSET (IBusPanelServiceClass, start_setup) },
        { "StateChanged",          G_STRUCT_OFFSET (IBusPanelServiceClass, state_changed) },
    };

    gint i;
    for (i = 0; i < G_N_ELEMENTS (no_arg_methods); i++) {
        if (g_strcmp0 (method_name, no_arg_methods[i].name) == 0) {
            typedef gboolean (* NoArgFunc) (IBusPanelService *);
            NoArgFunc func;
            func = G_STRUCT_MEMBER (NoArgFunc,
                                    IBUS_PANEL_SERVICE_GET_CLASS (panel),
                                    no_arg_methods[i].offset);
            func (panel);
            g_dbus_method_invocation_return_value (invocation, NULL);
            return;
        }
    }

    /* should not be reached */
    g_return_if_reached ();
}

static GVariant *
ibus_panel_service_service_get_property (IBusService        *service,
                                         GDBusConnection    *connection,
                                         const gchar        *sender,
                                         const gchar        *object_path,
                                         const gchar        *interface_name,
                                         const gchar        *property_name,
                                         GError            **error)
{
    return IBUS_SERVICE_CLASS (ibus_panel_service_parent_class)->
                service_get_property (service,
                                      connection,
                                      sender,
                                      object_path,
                                      interface_name,
                                      property_name,
                                      error);
}

static gboolean
ibus_panel_service_service_set_property (IBusService        *service,
                                         GDBusConnection    *connection,
                                         const gchar        *sender,
                                         const gchar        *object_path,
                                         const gchar        *interface_name,
                                         const gchar        *property_name,
                                         GVariant           *value,
                                         GError            **error)
{
    return IBUS_SERVICE_CLASS (ibus_panel_service_parent_class)->
                service_set_property (service,
                                      connection,
                                      sender,
                                      object_path,
                                      interface_name,
                                      property_name,
                                      value,
                                      error);
}


static void
ibus_panel_service_not_implemented (IBusPanelService *panel)
{
    /* g_debug ("not implemented"); */
}

static void
ibus_panel_service_focus_in (IBusPanelService    *panel,
                             const gchar         *input_context_path)
{
    ibus_panel_service_not_implemented(panel);
}

static void
ibus_panel_service_focus_out (IBusPanelService    *panel,
                              const gchar         *input_context_path)
{
    ibus_panel_service_not_implemented(panel);
}

static void
ibus_panel_service_register_properties (IBusPanelService *panel,
                                        IBusPropList     *prop_list)
{
    ibus_panel_service_not_implemented(panel);
}

static void
ibus_panel_service_set_cursor_location (IBusPanelService *panel,
                                        gint              x,
                                        gint              y,
                                        gint              w,
                                        gint              h)
{
    ibus_panel_service_not_implemented(panel);
}

static void
ibus_panel_service_update_auxiliary_text (IBusPanelService *panel,
                                          IBusText         *text,
                                          gboolean          visible)
{
    ibus_panel_service_not_implemented(panel);
}

static void
ibus_panel_service_update_lookup_table (IBusPanelService *panel,
                                        IBusLookupTable  *lookup_table,
                                        gboolean          visibl)
{
    ibus_panel_service_not_implemented(panel);
}

static void
ibus_panel_service_update_preedit_text (IBusPanelService *panel,
                                        IBusText         *text,
                                        guint             cursor_pos,
                                        gboolean          visible)
{
    ibus_panel_service_not_implemented(panel);
}

static void
ibus_panel_service_update_property (IBusPanelService *panel,
                                    IBusProperty     *prop)
{
    ibus_panel_service_not_implemented(panel);
}

IBusPanelService *
ibus_panel_service_new (GDBusConnection *connection)
{
    g_return_val_if_fail (G_IS_DBUS_CONNECTION (connection), NULL);

    GObject *object = g_object_new (IBUS_TYPE_PANEL_SERVICE,
                                    "object-path", IBUS_PATH_PANEL,
                                    "connection", connection,
                                    NULL);

    return IBUS_PANEL_SERVICE (object);
}

void
ibus_panel_service_candidate_clicked (IBusPanelService *panel,
                                      guint             index,
                                      guint             button,
                                      guint             state)
{
    g_return_if_fail (IBUS_IS_PANEL_SERVICE (panel));
    ibus_service_emit_signal ((IBusService *) panel,
                              NULL,
                              IBUS_INTERFACE_PANEL,
                              "CandidateClicked",
                              g_variant_new ("(uuu)", index, button, state),
                              NULL);
}

void
ibus_panel_service_property_active (IBusPanelService *panel,
                                    const gchar      *prop_name,
                                    guint             prop_state)
{
    g_return_if_fail (IBUS_IS_PANEL_SERVICE (panel));
    ibus_service_emit_signal ((IBusService *) panel,
                              NULL,
                              IBUS_INTERFACE_PANEL,
                              "PropertyActive",
                              g_variant_new ("(su)", prop_name, prop_state),
                              NULL);
}

void
ibus_panel_service_property_show (IBusPanelService *panel,
                                  const gchar      *prop_name)
{
    g_return_if_fail (IBUS_IS_PANEL_SERVICE (panel));
    ibus_service_emit_signal ((IBusService *) panel,
                              NULL,
                              IBUS_INTERFACE_PANEL,
                              "PropertyShow",
                              g_variant_new ("(s)", prop_name),
                              NULL);
}

void
ibus_panel_service_property_hide (IBusPanelService *panel,
                                  const gchar      *prop_name)
{
    g_return_if_fail (IBUS_IS_PANEL_SERVICE (panel));
    ibus_service_emit_signal ((IBusService *) panel,
                              NULL,
                              IBUS_INTERFACE_PANEL,
                              "PropertyHide",
                              g_variant_new ("(s)", prop_name),
                              NULL);
}

#define DEFINE_FUNC(name, Name)                             \
    void                                                    \
    ibus_panel_service_##name (IBusPanelService *panel)     \
    {                                                       \
        g_return_if_fail (IBUS_IS_PANEL_SERVICE (panel));   \
        ibus_service_emit_signal ((IBusService *) panel,    \
                                  NULL,                     \
                                  IBUS_INTERFACE_PANEL,     \
                                  #Name,                    \
                                  NULL,                     \
                                  NULL);                    \
    }
DEFINE_FUNC (cursor_down, CursorDown)
DEFINE_FUNC (cursor_up, CursorUp)
DEFINE_FUNC (page_down, PageDown)
DEFINE_FUNC (page_up, PageUp)
#undef DEFINE_FUNC

