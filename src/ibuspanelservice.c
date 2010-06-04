/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (c) 2009, Google Inc. All rights reserved.
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
#include "ibusshare.h"
#include "ibuspanelservice.h"

enum {
    PROP_0,
    PROP_CONNECTION,
};

/* functions prototype */
static void     ibus_panel_service_service_set_property  (IBusPanelService      *panel,
                                                          guint                  prop_id,
                                                          const GValue          *value,
                                                          GParamSpec            *pspec);
static void     ibus_panel_service_service_get_property  (IBusPanelService      *panel,
                                                          guint                  prop_id,
                                                          GValue                *value,
                                                          GParamSpec            *pspec);
static void     ibus_panel_service_real_destroy          (IBusPanelService      *panel);
static gboolean ibus_panel_service_ibus_message          (IBusPanelService      *panel,
                                                          IBusConnection        *connection,
                                                          IBusMessage           *message);
static gboolean ibus_panel_service_not_implemented       (IBusPanelService      *panel,
                                                          IBusError            **error);
static gboolean ibus_panel_service_focus_in              (IBusPanelService      *panel,
                                                          const gchar           *input_context_path,
                                                          IBusError            **error);
static gboolean ibus_panel_service_focus_out             (IBusPanelService      *panel,
                                                          const gchar           *input_context_path,
                                                          IBusError            **error);
static gboolean ibus_panel_service_register_properties  (IBusPanelService       *panel,
                                                         IBusPropList           *prop_list,
                                                         IBusError             **error);
static gboolean ibus_panel_service_set_cursor_location   (IBusPanelService      *panel,
                                                          gint                   x,
                                                          gint                   y,
                                                          gint                   w,
                                                          gint                   h,
                                                          IBusError            **error);
static gboolean ibus_panel_service_update_auxiliary_text (IBusPanelService      *panel,
                                                          IBusText              *text,
                                                          gboolean              visible,
                                                          IBusError            **error);
static gboolean ibus_panel_service_update_lookup_table   (IBusPanelService      *panel,
                                                          IBusLookupTable       *lookup_table,
                                                          gboolean               visible,
                                                          IBusError            **error);
static gboolean ibus_panel_service_update_preedit_text   (IBusPanelService      *panel,
                                                          IBusText              *text,
                                                          guint                  cursor_pos,
                                                          gboolean               visible,
                                                          IBusError            **error);
static gboolean ibus_panel_service_update_property       (IBusPanelService      *panel,
                                                          IBusProperty          *prop,
                                                          IBusError            **error);

G_DEFINE_TYPE (IBusPanelService, ibus_panel_service, IBUS_TYPE_SERVICE)

IBusPanelService *
ibus_panel_service_new (IBusConnection *connection)
{
    g_assert (IBUS_IS_CONNECTION (connection));

    IBusPanelService *panel;

    panel = (IBusPanelService*) g_object_new (IBUS_TYPE_PANEL_SERVICE,
                                              "path", IBUS_PATH_PANEL,
                                              "connection", connection,
                                              NULL);

    return panel;
}

static void
ibus_panel_service_class_init (IBusPanelServiceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_panel_service_service_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_panel_service_service_get_property;

    IBUS_OBJECT_CLASS (gobject_class)->destroy = (IBusObjectDestroyFunc) ibus_panel_service_real_destroy;

    IBUS_SERVICE_CLASS (klass)->ibus_message = (ServiceIBusMessageFunc) ibus_panel_service_ibus_message;

    klass->focus_in              = ibus_panel_service_focus_in;
    klass->focus_out             = ibus_panel_service_focus_out;
    klass->register_properties   = ibus_panel_service_register_properties;
    klass->set_cursor_location   = ibus_panel_service_set_cursor_location;
    klass->update_lookup_table   = ibus_panel_service_update_lookup_table;
    klass->update_auxiliary_text = ibus_panel_service_update_auxiliary_text;
    klass->update_preedit_text   = ibus_panel_service_update_preedit_text;
    klass->update_property       = ibus_panel_service_update_property;

    klass->cursor_down_lookup_table = ibus_panel_service_not_implemented;
    klass->cursor_up_lookup_table   = ibus_panel_service_not_implemented;
    klass->destroy                  = ibus_panel_service_not_implemented;
    klass->hide_auxiliary_text      = ibus_panel_service_not_implemented;
    klass->hide_language_bar        = ibus_panel_service_not_implemented;
    klass->hide_lookup_table        = ibus_panel_service_not_implemented;
    klass->hide_preedit_text        = ibus_panel_service_not_implemented;
    klass->page_down_lookup_table   = ibus_panel_service_not_implemented;
    klass->page_up_lookup_table     = ibus_panel_service_not_implemented;
    klass->reset                    = ibus_panel_service_not_implemented;
    klass->show_auxiliary_text      = ibus_panel_service_not_implemented;
    klass->show_language_bar        = ibus_panel_service_not_implemented;
    klass->show_lookup_table        = ibus_panel_service_not_implemented;
    klass->show_preedit_text        = ibus_panel_service_not_implemented;
    klass->start_setup              = ibus_panel_service_not_implemented;
    klass->state_changed            = ibus_panel_service_not_implemented;

    /* install properties */
    /**
     * IBusPanelService:connection:
     *
     * Connection of this IBusPanelService.
     */
    g_object_class_install_property (gobject_class,
                                     PROP_CONNECTION,
                                     g_param_spec_object ("connection",
                                                          "connection",
                                                          "The connection of service object",
                                                          IBUS_TYPE_CONNECTION,
                                                          G_PARAM_READWRITE |  G_PARAM_CONSTRUCT_ONLY));
}

static void
ibus_panel_service_init (IBusPanelService *panel)
{
}

static void
ibus_panel_service_service_set_property (IBusPanelService *panel,
                                         guint             prop_id,
                                         const GValue     *value,
                                         GParamSpec       *pspec)
{
    switch (prop_id) {
    case PROP_CONNECTION:
        ibus_service_add_to_connection ((IBusService *) panel,
                                        g_value_get_object (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (panel, prop_id, pspec);
    }
}

static void
ibus_panel_service_service_get_property (IBusPanelService *panel,
                                         guint             prop_id,
                                         GValue           *value,
                                         GParamSpec       *pspec)
{
    switch (prop_id) {
    case PROP_CONNECTION:
        {
            GList *connections = ibus_service_get_connections ((IBusService *) panel);
            if (connections) {
                g_value_set_object (value, connections->data);
            }
            else {
                g_value_set_object (value, NULL);
            }
            g_list_free (connections);
        }
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (panel, prop_id, pspec);
    }
}

static void
ibus_panel_service_real_destroy (IBusPanelService *panel)
{
    IBUS_OBJECT_CLASS(ibus_panel_service_parent_class)->destroy (IBUS_OBJECT (panel));
}

static gboolean
ibus_panel_service_ibus_message (IBusPanelService *panel,
                                 IBusConnection   *connection,
                                 IBusMessage      *message)
{
    g_assert (IBUS_IS_PANEL_SERVICE (panel));
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    const static struct {
        const gchar *name;
        const gint offset;
    } no_arg_methods [] = {
        { "CursorUpLookupTable"  , G_STRUCT_OFFSET (IBusPanelServiceClass, cursor_down_lookup_table) },
        { "CursorDownLookupTable", G_STRUCT_OFFSET (IBusPanelServiceClass, cursor_up_lookup_table) },
        { "Destroy",               G_STRUCT_OFFSET (IBusPanelServiceClass, destroy) },
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

    IBusMessage *reply = NULL;

    gint i;
    for (i = 0; i < G_N_ELEMENTS (no_arg_methods); i++) {
        if (!ibus_message_is_method_call (message, IBUS_INTERFACE_PANEL,
                                          no_arg_methods[i].name))
            continue;

        IBusMessageIter iter;
        ibus_message_iter_init (message, &iter);
        if (ibus_message_iter_has_next (&iter)) {
            reply = ibus_message_new_error_printf (message,
                                                   DBUS_ERROR_INVALID_ARGS,
                                                   "%s.%s: Method does not have arguments",
                                                   IBUS_INTERFACE_PANEL,
                                                   no_arg_methods[i].name);
        }
        else {
            IBusError *error = NULL;
            typedef gboolean (* NoArgFunc) (IBusPanelService *, IBusError **);
            NoArgFunc func;
            func = G_STRUCT_MEMBER (NoArgFunc,
                                    IBUS_PANEL_SERVICE_GET_CLASS (panel),
                                    no_arg_methods[i].offset);
            if (!func (panel, &error)) {
                reply = ibus_message_new_error (message,
                                                error->name,
                                                error->message);
                ibus_error_free (error);
            }
            else {
                reply = ibus_message_new_method_return (message);
            }
        }
        ibus_connection_send (connection, reply);
        ibus_message_unref (reply);
        return TRUE;
    }

    if (ibus_message_is_method_call (message, IBUS_INTERFACE_PANEL, "FocusIn")) {
        const gchar* input_context_path = NULL;
        IBusError *error = NULL;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_OBJECT_PATH,
                                        &input_context_path,
                                        G_TYPE_INVALID);

        if (!retval || !IBUS_PANEL_SERVICE_GET_CLASS (panel)->focus_in (panel,
                                                                        input_context_path,
                                                                        &error)) {
            reply = ibus_message_new_error (message,
                                            error->name,
                                            error->message);
            ibus_error_free (error);
        }
        else {
            reply = ibus_message_new_method_return (message);
        }
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_PANEL, "FocusOut")) {
        const gchar* input_context_path = NULL;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_OBJECT_PATH,
                                        &input_context_path,
                                        G_TYPE_INVALID);

        if (!retval || !IBUS_PANEL_SERVICE_GET_CLASS (panel)->focus_out (panel,
                                                                         input_context_path,
                                                                         &error)) {
            reply = ibus_message_new_error(message,
                                           error->name,
                                           error->message);
            ibus_error_free (error);
        }
        else {
            reply = ibus_message_new_method_return (message);
        }
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_PANEL, "RegisterProperties")) {
        IBusPropList *prop_list = NULL;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_PROP_LIST, &prop_list,
                                        G_TYPE_INVALID);

        if (!retval || !IBUS_PANEL_SERVICE_GET_CLASS (panel)->register_properties (panel,
                                                                                   prop_list,
                                                                                   &error)) {
            reply = ibus_message_new_error(message,
                                           error->name,
                                           error->message);
            ibus_error_free (error);
        }
        else {
            reply = ibus_message_new_method_return (message);
        }

        if (prop_list != NULL && g_object_is_floating (prop_list))
            g_object_unref (prop_list);
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_PANEL, "UpdateAuxiliaryText")) {
        IBusText *text = NULL;
        gboolean visible;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_TEXT, &text,
                                        G_TYPE_BOOLEAN, &visible,
                                        G_TYPE_INVALID);

        if (!retval || !IBUS_PANEL_SERVICE_GET_CLASS (panel)->update_auxiliary_text (panel,
                                                                                     text,
                                                                                     visible,
                                                                                     &error)) {
            reply = ibus_message_new_error(message,
                                           error->name,
                                           error->message);
            ibus_error_free (error);
        }
        else {
            reply = ibus_message_new_method_return (message);
        }

        if (text != NULL && g_object_is_floating (text))
            g_object_unref (text);
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_PANEL, "UpdateLookupTable")) {
        IBusLookupTable *table = NULL;
        gboolean visible = FALSE;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_LOOKUP_TABLE, &table,
                                        G_TYPE_BOOLEAN, &visible,
                                        G_TYPE_INVALID);

        if (!retval || !IBUS_PANEL_SERVICE_GET_CLASS (panel)->update_lookup_table (panel,
                                                                                   table,
                                                                                   visible,
                                                                                   &error)) {
            reply = ibus_message_new_error(message,
                                           error->name,
                                           error->message);
            ibus_error_free (error);
        }
        else {
            reply = ibus_message_new_method_return (message);
        }

        if (table != NULL && g_object_is_floating (table))
            g_object_unref (table);
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_PANEL, "UpdatePreeditText")) {
        IBusText *text = NULL;
        guint cursor_pos;
        gboolean visible;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_TEXT, &text,
                                        G_TYPE_UINT, &cursor_pos,
                                        G_TYPE_BOOLEAN, &visible,
                                        G_TYPE_INVALID);

        if (!retval || !IBUS_PANEL_SERVICE_GET_CLASS (panel)->update_preedit_text (panel,
                                                                                   text,
                                                                                   cursor_pos,
                                                                                   visible,
                                                                                   &error)) {
            reply = ibus_message_new_error(message,
                                           error->name,
                                           error->message);
            ibus_error_free (error);
        }
        else {
            reply = ibus_message_new_method_return (message);
        }

        if (text != NULL && g_object_is_floating (text))
            g_object_unref (text);
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_PANEL, "UpdateProperty")) {
        IBusProperty *property = NULL;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        IBUS_TYPE_PROPERTY, &property,
                                        G_TYPE_INVALID);

        if (!retval || !IBUS_PANEL_SERVICE_GET_CLASS (panel)->update_property (panel,
                                                                               property,
                                                                               &error)) {
            reply = ibus_message_new_error(message,
                                           error->name,
                                           error->message);
            ibus_error_free (error);
        }
        else {
            reply = ibus_message_new_method_return (message);
        }

        if (property != NULL && g_object_is_floating (property))
            g_object_unref (property);
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_PANEL, "SetCursorLocation")) {
        guint x, y, w, h;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_INT, &x,
                                        G_TYPE_INT, &y,
                                        G_TYPE_INT, &w,
                                        G_TYPE_INT, &h,
                                        G_TYPE_INVALID);

        if (!retval || !IBUS_PANEL_SERVICE_GET_CLASS (panel)->set_cursor_location (panel,
                                                                                   x,
                                                                                   y,
                                                                                   w,
                                                                                   h,
                                                                                   &error)) {
            reply = ibus_message_new_error(message,
                                           error->name,
                                           error->message);
            ibus_error_free (error);
        }
        else {
            reply = ibus_message_new_method_return (message);
        }
    }

    if (reply) {
        ibus_connection_send (connection, reply);
        ibus_message_unref (reply);
        return TRUE;
    }

    return TRUE;
}

static gboolean
ibus_panel_service_not_implemented (IBusPanelService *panel,
                                    IBusError       **error) {
    if (error) {
        *error = ibus_error_new_from_printf (DBUS_ERROR_FAILED,
                                             "Not implemented");
    }
    return FALSE;
}

static gboolean
ibus_panel_service_focus_in (IBusPanelService    *panel,
                             const gchar         *input_context_path,
                             IBusError          **error)
{
    return ibus_panel_service_not_implemented(panel, error);
}

static gboolean
ibus_panel_service_focus_out (IBusPanelService    *panel,
                              const gchar         *input_context_path,
                              IBusError          **error)
{
    return ibus_panel_service_not_implemented(panel, error);
}

static gboolean
ibus_panel_service_register_properties (IBusPanelService *panel,
                                        IBusPropList     *prop_list,
                                        IBusError       **error)
{
    return ibus_panel_service_not_implemented(panel, error);
}

static gboolean
ibus_panel_service_set_cursor_location (IBusPanelService *panel,
                                        gint              x,
                                        gint              y,
                                        gint              w,
                                        gint              h,
                                        IBusError       **error)
{
    return ibus_panel_service_not_implemented(panel, error);
}

static gboolean
ibus_panel_service_update_auxiliary_text (IBusPanelService *panel,
                                          IBusText         *text,
                                          gboolean          visible,
                                          IBusError       **error)
{
    return ibus_panel_service_not_implemented(panel, error);
}

static gboolean
ibus_panel_service_update_lookup_table (IBusPanelService *panel,
                                        IBusLookupTable  *lookup_table,
                                        gboolean          visible,
                                        IBusError       **error)
{
    return ibus_panel_service_not_implemented(panel, error);
}

static gboolean
ibus_panel_service_update_preedit_text (IBusPanelService *panel,
                                        IBusText         *text,
                                        guint             cursor_pos,
                                        gboolean          visible,
                                        IBusError       **error)
{
    return ibus_panel_service_not_implemented(panel, error);
}

static gboolean
ibus_panel_service_update_property (IBusPanelService *panel,
                                    IBusProperty     *prop,
                                    IBusError       **error)
{
    return ibus_panel_service_not_implemented(panel, error);
}

void
ibus_panel_service_candidate_clicked (IBusPanelService *panel,
                                      guint             index,
                                      guint             button,
                                      guint             state) {
    ibus_service_send_signal ((IBusService *) panel,
                              IBUS_INTERFACE_PANEL,
                              "CandidateClicked",
                              G_TYPE_UINT, &index,
                              G_TYPE_UINT, &button,
                              G_TYPE_UINT, &state,
                              G_TYPE_INVALID);
}

void
ibus_panel_service_cursor_down (IBusPanelService *panel) {
    ibus_service_send_signal ((IBusService *) panel,
                              IBUS_INTERFACE_PANEL,
                              "CursorDown",
                              G_TYPE_INVALID);
}

void
ibus_panel_service_cursor_up (IBusPanelService *panel) {
    ibus_service_send_signal ((IBusService *) panel,
                              IBUS_INTERFACE_PANEL,
                              "CursorUp",
                              G_TYPE_INVALID);
}

void
ibus_panel_service_page_down (IBusPanelService *panel) {
    ibus_service_send_signal ((IBusService *) panel,
                              IBUS_INTERFACE_PANEL,
                              "PageDown",
                              G_TYPE_INVALID);
}

void
ibus_panel_service_page_up (IBusPanelService *panel) {
    ibus_service_send_signal ((IBusService *) panel,
                              IBUS_INTERFACE_PANEL,
                              "PageUp",
                              G_TYPE_INVALID);
}

void
ibus_panel_service_property_active (IBusPanelService *panel,
                                    const gchar      *prop_name,
                                    gint              prop_state) {
    ibus_service_send_signal ((IBusService *) panel,
                              IBUS_INTERFACE_PANEL,
                              "PropertyActivate",
                              G_TYPE_STRING, &prop_name,
                              G_TYPE_INT,    &prop_state,
                              G_TYPE_INVALID);
}

void
ibus_panel_service_property_show (IBusPanelService *panel,
                                  const gchar      *prop_name) {
    ibus_service_send_signal ((IBusService *) panel,
                              IBUS_INTERFACE_PANEL,
                              "PropertyShow",
                              G_TYPE_STRING, &prop_name,
                              G_TYPE_INVALID);
}

void
ibus_panel_service_property_hide (IBusPanelService *panel,
                                  const gchar      *prop_name) {
    ibus_service_send_signal ((IBusService *) panel,
                              IBUS_INTERFACE_PANEL,
                              "PropertyHide",
                              G_TYPE_STRING, &prop_name,
                              G_TYPE_INVALID);
}
/* For Emacs:
 * Local Variables:
 * c-file-style: "gnu"
 * c-basic-offset: 4
 * End:
 */
