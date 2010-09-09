/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
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
#ifndef __INPUT_CONTEXT_H_
#define __INPUT_CONTEXT_H_

#include <ibus.h>
#include "connection.h"
#include "factoryproxy.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_INPUT_CONTEXT             \
    (bus_input_context_get_type ())
#define BUS_INPUT_CONTEXT(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_INPUT_CONTEXT, BusInputContext))
#define BUS_INPUT_CONTEXT_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_INPUT_CONTEXT, BusInputContextClass))
#define BUS_IS_INPUT_CONTEXT(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_INPUT_CONTEXT))
#define BUS_IS_INPUT_CONTEXT_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_INPUT_CONTEXT))
#define BUS_INPUT_CONTEXT_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_INPUT_CONTEXT, BusInputContextClass))

G_BEGIN_DECLS

typedef struct _BusInputContext BusInputContext;
typedef struct _BusInputContextClass BusInputContextClass;

struct _BusInputContext {
    IBusService parent;

    /* instance members */
    BusConnection *connection;
    BusEngineProxy *engine;
    gchar *client;

    gboolean has_focus;
    gboolean enabled;

    /* capabilities */
    guint capabilities;

    /* cursor location */
    gint x;
    gint y;
    gint w;
    gint h;

    /* prev key event */
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
};

struct _BusInputContextClass {
    IBusServiceClass parent;

    /* class members */
};

GType                bus_input_context_get_type         (void);
BusInputContext     *bus_input_context_new              (BusConnection      *connection,
                                                         const gchar        *client);
void                 bus_input_context_focus_in         (BusInputContext    *context);
void                 bus_input_context_focus_out        (BusInputContext    *context);
gboolean             bus_input_context_has_focus        (BusInputContext    *context);
void                 bus_input_context_enable           (BusInputContext    *context);
void                 bus_input_context_disable          (BusInputContext    *context);
gboolean             bus_input_context_is_enabled       (BusInputContext    *context);
void                 bus_input_context_page_up          (BusInputContext    *context);
void                 bus_input_context_page_down        (BusInputContext    *context);
void                 bus_input_context_cursor_up        (BusInputContext    *context);
void                 bus_input_context_cursor_down      (BusInputContext    *context);
void                 bus_input_context_candidate_clicked(BusInputContext    *context,
                                                         guint               index,
                                                         guint               button,
                                                         guint               state);
void                 bus_input_context_set_engine       (BusInputContext    *context,
                                                         BusEngineProxy     *factory);
BusEngineProxy      *bus_input_context_get_engine       (BusInputContext    *context);
void                 bus_input_context_property_activate(BusInputContext    *context,
                                                         const gchar        *prop_name,
                                                         gint                prop_state);
guint                bus_input_context_get_capabilities (BusInputContext    *context);

G_END_DECLS
#endif
