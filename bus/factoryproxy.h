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
#ifndef __FACTORY_PROXY_H_
#define __FACTORY_PROXY_H_

#include <ibus.h>
#include "connection.h"
#include "engineproxy.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_FACTORY_PROXY             \
    (bus_factory_proxy_get_type ())
#define BUS_FACTORY_PROXY(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_FACTORY_PROXY, BusFactoryProxy))
#define BUS_FACTORY_PROXY_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_FACTORY_PROXY, BusFactoryProxyClass))
#define BUS_IS_FACTORY_PROXY(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_FACTORY_PROXY))
#define BUS_IS_FACTORY_PROXY_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_FACTORY_PROXY))
#define BUS_FACTORY_PROXY_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_FACTORY_PROXY, BusFactoryProxyClass))

G_BEGIN_DECLS

typedef struct _BusFactoryProxy BusFactoryProxy;
typedef struct _BusFactoryProxyClass BusFactoryProxyClass;

struct _BusFactoryProxy {
    IBusProxy parent;
    /* instance members */

    IBusComponent *component;
    GList *engine_list;
};

struct _BusFactoryProxyClass {
    IBusProxyClass parent;
    /* class members */
};

GType            bus_factory_proxy_get_type     (void);
BusFactoryProxy *bus_factory_proxy_new          (IBusComponent      *component,
                                                 BusConnection      *connection);
IBusComponent   *bus_factory_proxy_get_component(BusFactoryProxy    *factory);
BusEngineProxy  *bus_factory_proxy_create_engine(BusFactoryProxy    *factory,
                                                 IBusEngineDesc     *desc);
BusFactoryProxy *bus_factory_proxy_get_from_component
                                                (IBusComponent      *component);
BusFactoryProxy *bus_factory_proxy_get_from_engine
                                                (IBusEngineDesc     *desc);

#if 0
const gchar     *bus_factory_proxy_get_name     (BusFactoryProxy    *factory);
const gchar     *bus_factory_proxy_get_lang     (BusFactoryProxy    *factory);
const gchar     *bus_factory_proxy_get_icon     (BusFactoryProxy    *factory);
const gchar     *bus_factory_proxy_get_authors  (BusFactoryProxy    *factory);
const gchar     *bus_factory_proxy_get_credits  (BusFactoryProxy    *factory);
#endif

G_END_DECLS
#endif

