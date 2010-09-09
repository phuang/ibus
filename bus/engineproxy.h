/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
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
#ifndef __ENGINE_PROXY_H_
#define __ENGINE_PROXY_H_

#include <ibus.h>
#include "connection.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_ENGINE_PROXY             \
    (bus_engine_proxy_get_type ())
#define BUS_ENGINE_PROXY(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_ENGINE_PROXY, BusEngineProxy))
#define BUS_ENGINE_PROXY_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_ENGINE_PROXY, BusEngineProxyClass))
#define BUS_IS_ENGINE_PROXY(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_ENGINE_PROXY))
#define BUS_IS_ENGINE_PROXY_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_ENGINE_PROXY))
#define BUS_ENGINE_PROXY_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_ENGINE_PROXY, BusEngineProxyClass))

G_BEGIN_DECLS

typedef struct _BusEngineProxy BusEngineProxy;
typedef struct _BusEngineProxyClass BusEngineProxyClass;

struct _BusEngineProxy {
    IBusProxy parent;
    /* instance members */
    gboolean has_focus;
    gboolean enabled;
    guint capabilities;
    /* cursor location */
    gint x;
    gint y;
    gint w;
    gint h;

    IBusEngineDesc *desc;
    IBusKeymap     *keymap;
    IBusPropList *prop_list;

    /* private member */
};

struct _BusEngineProxyClass {
    IBusProxyClass parent;
    /* class members */
};

GType            bus_engine_proxy_get_type          (void);
BusEngineProxy  *bus_engine_proxy_new               (const gchar    *path,
                                                     IBusEngineDesc *desc,
                                                     BusConnection  *connection);
IBusEngineDesc  *bus_engine_proxy_get_desc          (BusEngineProxy *engine);
void             bus_engine_proxy_process_key_event (BusEngineProxy *engine,
                                                     guint           keyval,
                                                     guint           keycode,
                                                     guint           state,
                                                     GFunc           return_cn,
                                                     gpointer        user_data);
void             bus_engine_proxy_set_cursor_location
                                                    (BusEngineProxy *engine,
                                                     gint            x,
                                                     gint            y,
                                                     gint            w,
                                                     gint            h);
void             bus_engine_proxy_focus_in          (BusEngineProxy *engine);
void             bus_engine_proxy_focus_out         (BusEngineProxy *engine);
void             bus_engine_proxy_reset             (BusEngineProxy *engine);
void             bus_engine_proxy_set_capabilities  (BusEngineProxy *engine,
                                                     guint           caps);
void             bus_engine_proxy_page_up           (BusEngineProxy *engine);
void             bus_engine_proxy_page_down         (BusEngineProxy *engine);
void             bus_engine_proxy_cursor_up         (BusEngineProxy *engine);
void             bus_engine_proxy_cursor_down       (BusEngineProxy *engine);
void             bus_engine_proxy_candidate_clicked (BusEngineProxy *engine,
                                                     guint           index,
                                                     guint           button,
                                                     guint           state);
void             bus_engine_proxy_enable            (BusEngineProxy *engine);
void             bus_engine_proxy_disable           (BusEngineProxy *engine);
void             bus_engine_proxy_property_activate (BusEngineProxy *engine,
                                                     const gchar    *prop_name,
                                                     guint           state);
void             bus_engine_proxy_property_show     (BusEngineProxy *engine,
                                                     const gchar    *prop_name);
void             bus_engine_proxy_property_hide     (BusEngineProxy *engine,
                                                     const gchar    *prop_name);
gboolean         bus_engine_proxy_is_enabled        (BusEngineProxy *engine);
G_END_DECLS
#endif
