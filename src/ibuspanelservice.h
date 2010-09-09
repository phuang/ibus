/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
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
/**
 * SECTION: ibuspanelservice
 * @short_description: Panel service back-end.
 * @stability: Stable
 *
 * An IBusPanelService is a base class for UI services.
 * Developers can "extend" this class for panel UI development.
 */
#ifndef __IBUS_PANEL_SERVICE_H_
#define __IBUS_PANEL_SERVICE_H_

#include "ibusconnection.h"
#include "ibuslookuptable.h"
#include "ibusservice.h"
#include "ibusproplist.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_PANEL_SERVICE                \
    (ibus_panel_service_get_type ())
#define IBUS_PANEL_SERVICE(obj)                        \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_PANEL_SERVICE, IBusPanelService))
#define IBUS_PANEL_SERVICE_CLASS(klass)        \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_PANEL_SERVICE, IBusPanelServiceClass))
#define IBUS_IS_PANEL_SERVICE(obj)                     \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_PANEL_SERVICE))
#define IBUS_IS_PANEL_SERVICE_CLASS(klass)             \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_PANEL_SERVICE))
#define IBUS_PANEL_SERVICE_GET_CLASS(obj)      \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_PANEL_SERVICE, IBusPanelServiceClass))

G_BEGIN_DECLS

typedef struct _IBusPanelService IBusPanelService;
typedef struct _IBusPanelServiceClass IBusPanelServiceClass;

/**
 * IBusPanelService:
 *
 * An opaque data type representing an IBusPanelService.
 */
struct _IBusPanelService {
    IBusService parent;
    /* instance members */
};

struct _IBusPanelServiceClass {
    IBusServiceClass parent;

    /* class members */
    gboolean (* focus_in)                  (IBusPanelService       *panel,
                                            const gchar            *input_context_path,
                                            IBusError             **error);
    gboolean (* focus_out)                 (IBusPanelService       *panel,
                                            const gchar            *input_context_path,
                                            IBusError             **error);
    gboolean (* register_properties)       (IBusPanelService       *panel,
                                            IBusPropList           *prop_list,
                                            IBusError             **error);
    gboolean (* set_cursor_location)       (IBusPanelService       *panel,
                                            gint                    x,
                                            gint                    y,
                                            gint                    w,
                                            gint                    h,
                                            IBusError             **error);
    gboolean (* update_auxiliary_text)     (IBusPanelService       *panel,
                                            IBusText               *text,
                                            gboolean                visible,
                                            IBusError             **error);
    gboolean (* update_lookup_table)       (IBusPanelService       *panel,
                                            IBusLookupTable        *lookup_table,
                                            gboolean                visible,
                                            IBusError             **error);
    gboolean (* update_preedit_text)       (IBusPanelService       *panel,
                                            IBusText              *text,
                                            guint                  cursor_pos,
                                            gboolean               visible,
                                            IBusError             **error);
    gboolean (* update_property)           (IBusPanelService       *panel,
                                            IBusProperty           *prop,
                                            IBusError             **error);
    gboolean (* cursor_down_lookup_table)  (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* cursor_up_lookup_table)    (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* destroy)                   (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* hide_auxiliary_text)       (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* hide_language_bar)         (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* hide_lookup_table)         (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* hide_preedit_text)         (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* page_down_lookup_table)    (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* page_up_lookup_table)      (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* reset)                     (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* show_auxiliary_text)       (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* show_language_bar)         (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* show_lookup_table)         (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* show_preedit_text)         (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* start_setup)               (IBusPanelService       *panel,
                                            IBusError             **error);
    gboolean (* state_changed)             (IBusPanelService       *panel,
                                            IBusError             **error);

    /*< private >*/
    /* padding */
    gpointer pdummy[8];  // We can add 8 pointers without breaking the ABI.
};

GType            ibus_panel_service_get_type  (void);

/**
 * ibus_panel_service_new:
 * @connection: An IBusConnection.
 * @returns: A newly allocated IBusPanelService.
 *
 * New an IBusPanelService from an IBusConnection.
 */
IBusPanelService *ibus_panel_service_new (IBusConnection    *connection);

/**
 * ibus_panel_service_candidate_clicked
 * @panel: An IBusPanelService
 * @index: Index in the Lookup table
 * @button: GdkEventButton::button (1: left button, etc.)
 * @state: GdkEventButton::state (key modifier flags)
 *
 * Notify that a candidate is clicked
 * by sending a "CandidateClicked" to IBus service.
 */
void ibus_panel_service_candidate_clicked (IBusPanelService *panel,
                                           guint             index,
                                           guint             button,
                                           guint             state);

/**
 * ibus_panel_service_cursor_down
 * @panel: An IBusPanelService
 *
 * Notify that the cursor is down
 * by sending a "CursorDown" to IBus service.
 */
void ibus_panel_service_cursor_down       (IBusPanelService *panel);

/**
 * ibus_panel_service_cursor_up
 * @panel: An IBusPanelService
 *
 * Notify that the cursor is up
 * by sending a "CursorUp" to IBus service.
 */
void ibus_panel_service_cursor_up         (IBusPanelService *panel);

/**
 * ibus_panel_service_page_down
 * @panel: An IBusPanelService
 *
 * Notify that the page is down
 * by sending a "PageDown" to IBus service.
 */
void ibus_panel_service_page_down         (IBusPanelService *panel);

/**
 * ibus_panel_service_page_up
 * @panel: An IBusPanelService
 *
 * Notify that the page is up
 * by sending a "PageUp" to IBus service.
 */
void ibus_panel_service_page_up           (IBusPanelService *panel);

/**
 * ibus_panel_service_property_active
 * @panel: An IBusPanelService
 * @prop_name: A property name
 * @prop_state: State of the property
 *
 * Notify that a property is active
 * by sending a "PropertyActivate" message to IBus service.
 */
void ibus_panel_service_property_active   (IBusPanelService *panel,
                                           const gchar      *prop_name,
                                           int               prop_state);
/**
 * ibus_panel_service_property_show
 * @panel: An IBusPanelService
 * @prop_name: A property name
 *
 * Notify that a property is shown
 * by sending a "ValueChanged" message to IBus service.
 */
void ibus_panel_service_property_show     (IBusPanelService *panel,
                                           const gchar      *prop_name);

/**
 * ibus_panel_service_property_hide
 * @panel: An IBusPanelService
 * @prop_name: A property name
 *
 * Notify that a property is hidden
 * by sending a "ValueChanged" message to IBus service.
 */
void ibus_panel_service_property_hide     (IBusPanelService *panel,
                                           const gchar      *prop_name);


G_END_DECLS
#endif
