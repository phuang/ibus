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
#ifndef __IBUS_IMPL_H_
#define __IBUS_IMPL_H_

#include <ibus.h>
#include "connection.h"
#include "inputcontext.h"
#include "registry.h"
#include "factoryproxy.h"
#include "panelproxy.h"
#include "engineproxy.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_IBUS_IMPL             \
    (bus_ibus_impl_get_type ())
#define BUS_IBUS_IMPL(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_IBUS_IMPL, BusIBusImpl))
#define BUS_IBUS_IMPL_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_IBUS_IMPL, BusIBusImplClass))
#define BUS_IS_IBUS_IMPL(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_IBUS_IMPL))
#define BUS_IS_IBUS_IMPL_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_IBUS_IMPL))
#define BUS_IBUS_IMPL_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_IBUS_IMPL, BusIBusImplClass))

#define BUS_DEFAULT_IBUS \
    (bus_ibus_impl_get_default ())
#define BUS_DEFAULT_HOTKEY_PROFILE \
    (bus_ibus_impl_get_hotkey_profile (BUS_DEFAULT_IBUS))
#define BUS_DEFAULT_KEYMAP \
    (bus_ibus_impl_get_keymap (BUS_DEFAULT_IBUS))
#define BUS_DEFAULT_REGISTRY \
    (bus_ibus_impl_get_registry (BUS_DEFAULT_IBUS))

G_BEGIN_DECLS

typedef struct _BusIBusImpl BusIBusImpl;
typedef struct _BusIBusImplClass BusIBusImplClass;

struct _BusIBusImpl {
    IBusService parent;
    /* instance members */

    GHashTable *factory_dict;
    GList *factory_list;
    GList *contexts;

    GList *engine_list;
    GList *register_engine_list;
    GList *component_list;

    gboolean use_sys_layout;
    gboolean embed_preedit_text;
    gboolean enable_by_default;

    BusRegistry     *registry;

    BusInputContext *focused_context;
    BusPanelProxy   *panel;
    IBusConfig      *config;
    IBusHotkeyProfile *hotkey_profile;
    IBusKeymap      *keymap;

    gboolean use_global_engine;
    BusEngineProxy  *global_engine;
    gchar           *global_previous_engine_name;
};

struct _BusIBusImplClass {
    IBusServiceClass parent;

    /* class members */
};

GType            bus_ibus_impl_get_type             (void);
BusIBusImpl     *bus_ibus_impl_get_default          (void);
BusFactoryProxy *bus_ibus_impl_get_default_factory  (BusIBusImpl        *ibus);
BusFactoryProxy *bus_ibus_impl_get_next_factory     (BusIBusImpl        *ibus,
                                                     BusFactoryProxy    *factory);
BusFactoryProxy *bus_ibus_impl_get_previous_factory (BusIBusImpl        *ibus,
                                                     BusFactoryProxy    *factory);
BusFactoryProxy *bus_ibus_impl_lookup_factory       (BusIBusImpl        *ibus,
                                                     const gchar        *path);
IBusHotkeyProfile
                *bus_ibus_impl_get_hotkey_profile   (BusIBusImpl        *ibus);
IBusKeymap      *bus_ibus_impl_get_keymap           (BusIBusImpl        *ibus);
BusRegistry     *bus_ibus_impl_get_registry         (BusIBusImpl        *ibus);

gboolean         bus_ibus_impl_filter_keyboard_shortcuts
                                                    (BusIBusImpl        *ibus,
                                                     BusInputContext    *context,
                                                     guint               keyval,
                                                     guint               modifiers,
                                                     guint               prev_keyval,
                                                     guint               prev_modifiers);

G_END_DECLS
#endif
