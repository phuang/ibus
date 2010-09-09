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
#ifndef __REGISTRY_H_
#define __REGISTRY_H_

#include <ibus.h>
#include "factoryproxy.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_REGISTRY             \
    (bus_registry_get_type ())
#define BUS_REGISTRY(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_REGISTRY, BusRegistry))
#define BUS_REGISTRY_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_REGISTRY, BusRegistryClass))
#define BUS_IS_REGISTRY(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_REGISTRY))
#define BUS_IS_REGISTRY_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_REGISTRY))
#define BUS_REGISTRY_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_REGISTRY, BusRegistryClass))

G_BEGIN_DECLS

typedef struct _BusRegistry BusRegistry;
typedef struct _BusRegistryClass BusRegistryClass;

struct _BusRegistry {
    IBusObject parent;

    /* instance members */
    GList *observed_paths;
    GList *components;

    GHashTable *engine_table;
    GList *active_engines;


#ifdef G_THREADS_ENABLED
    GThread *thread;
    gboolean thread_running;
    GMutex  *mutex;
    GCond   *cond;
    gboolean changed;
#endif
};

struct _BusRegistryClass {
    IBusObjectClass parent;

    /* class members */
};

GType            bus_registry_get_type          (void);
BusRegistry     *bus_registry_new               (void);
GList           *bus_registry_get_components    (BusRegistry    *registry);
GList           *bus_registry_get_engines       (BusRegistry    *registry);
GList           *bus_registry_get_engines_by_language
                                                (BusRegistry    *registry,
                                                 const gchar    *language);
void             bus_registry_stop_all_components
                                                (BusRegistry    *registry);

IBusComponent   *bus_registry_lookup_component_by_name
                                                (BusRegistry    *registry,
                                                 const gchar    *name);
IBusEngineDesc  *bus_registry_find_engine_by_name
                                                (BusRegistry    *registry,
                                                 const gchar    *name);
BusFactoryProxy *bus_registry_name_owner_changed(BusRegistry    *registry,
                                                 const gchar    *name,
                                                 const gchar    *old_name,
                                                 const gchar    *new_name);
#ifdef G_THREADS_ENABLED
void             bus_registry_start_monitor_changes
                                                (BusRegistry    *registry);
gboolean         bus_registry_is_changed        (BusRegistry    *registry);
#endif

G_END_DECLS
#endif

