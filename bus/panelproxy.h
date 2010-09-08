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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __PANEL_PROXY_H_
#define __PANEL_PROXY_H_

#include <ibus.h>
#include "connection.h"
#include "inputcontext.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_PANEL_PROXY             \
    (bus_panel_proxy_get_type ())
#define BUS_PANEL_PROXY(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_PANEL_PROXY, BusPanelProxy))
#define BUS_PANEL_PROXY_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_PANEL_PROXY, BusPanelProxyClass))
#define BUS_IS_PANEL_PROXY(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_PANEL_PROXY))
#define BUS_IS_PANEL_PROXY_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_PANEL_PROXY))
#define BUS_PANEL_PROXY_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_PANEL_PROXY, BusPanelProxyClass))

G_BEGIN_DECLS

typedef struct _BusPanelProxy BusPanelProxy;
typedef struct _BusPanelProxyClass BusPanelProxyClass;

struct _BusPanelProxy {
    IBusProxy parent;

    /* instance members */
    BusInputContext *focused_context;
};

struct _BusPanelProxyClass {
    IBusProxyClass parent;
    /* class members */

    void (* page_up)            (BusPanelProxy   *panel);
    void (* page_down)          (BusPanelProxy   *panel);
    void (* cursor_up)          (BusPanelProxy   *panel);
    void (* cursor_down)        (BusPanelProxy   *panel);
    void (* candidate_clicked)  (BusPanelProxy   *panel,
                                 guint            index,
                                 guint            button,
                                 guint            state);

    void (* property_activate)  (BusPanelProxy   *panel,
                                 const gchar     *prop_name,
                                 gint             prop_state);
};

GType            bus_panel_proxy_get_type           (void);
BusPanelProxy   *bus_panel_proxy_new                (BusConnection      *connection);
void             bus_panel_proxy_focus_in           (BusPanelProxy      *panel,
                                                     BusInputContext    *context);
void             bus_panel_proxy_focus_out          (BusPanelProxy      *panel,
                                                     BusInputContext    *context);
void             bus_panel_proxy_set_cursor_location
                                                    (BusPanelProxy      *panel,
                                                     gint32              x,
                                                     gint32              y,
                                                     gint32              w,
                                                     gint32              h);
void             bus_panel_proxy_update_preedit_text(BusPanelProxy      *panel,
                                                     IBusText           *text,
                                                     guint               cursor_pos,
                                                     gboolean            visible);
void             bus_panel_proxy_show_preedit_text  (BusPanelProxy      *panel);
void             bus_panel_proxy_hide_preedit_text  (BusPanelProxy      *panel);
void             bus_panel_proxy_update_auxiliary_text
                                                    (BusPanelProxy      *panel,
                                                     IBusText           *text,
                                                     gboolean            visible);
void             bus_panel_proxy_show_auxiliary_text(BusPanelProxy      *panel);
void             bus_panel_proxy_hide_auxiliary_text(BusPanelProxy      *panel);
void             bus_panel_proxy_update_lookup_table(BusPanelProxy      *panel,
                                                     IBusLookupTable    *table,
                                                     gboolean           visible);
void             bus_panel_proxy_show_lookup_table  (BusPanelProxy      *panel);
void             bus_panel_proxy_hide_lookup_table  (BusPanelProxy      *panel);
void             bus_panel_proxy_page_up_lookup_table
                                                    (BusPanelProxy      *panel);
void             bus_panel_proxy_page_down_lookup_table
                                                    (BusPanelProxy      *panel);
void             bus_panel_proxy_cursor_up_lookup_table
                                                    (BusPanelProxy      *panel);
void             bus_panel_proxy_cursor_down_lookup_table
                                                    (BusPanelProxy      *panel);
void             bus_panel_proxy_register_properties(BusPanelProxy      *panel,
                                                     IBusPropList       *prop_list);
void             bus_panel_proxy_update_property    (BusPanelProxy      *panel,
                                                     IBusProperty       *prop);
G_END_DECLS
#endif

