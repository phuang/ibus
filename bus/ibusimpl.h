/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2008-2013 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2013 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
#ifndef __BUS_IBUS_IMPL_H_
#define __BUS_IBUS_IMPL_H_

#include <ibus.h>
#include "connection.h"
#include "component.h"
#include "inputcontext.h"
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
#define BUS_DEFAULT_KEYMAP \
    (bus_ibus_impl_get_keymap (BUS_DEFAULT_IBUS))

G_BEGIN_DECLS

typedef struct _BusIBusImpl BusIBusImpl;
typedef struct _BusIBusImplClass BusIBusImplClass;

GType            bus_ibus_impl_get_type             (void);

/**
 * bus_ibus_impl_get_default:
 * @returns: a BusIBusImpl object which is a singleton.
 *
 * Instantiate a BusIBusImpl object (if necessary) and return the object.
 */
BusIBusImpl     *bus_ibus_impl_get_default          (void);


/* accessors */
BusFactoryProxy *bus_ibus_impl_lookup_factory       (BusIBusImpl        *ibus,
                                                     const gchar        *path);
IBusKeymap      *bus_ibus_impl_get_keymap           (BusIBusImpl        *ibus);
BusComponent    *bus_ibus_impl_lookup_component_by_name
                                                    (BusIBusImpl        *ibus,
                                                     const gchar        *name);
gboolean         bus_ibus_impl_is_use_sys_layout    (BusIBusImpl        *ibus);
gboolean         bus_ibus_impl_is_embed_preedit_text
                                                    (BusIBusImpl        *ibus);
BusInputContext *bus_ibus_impl_get_focused_input_context
                                                    (BusIBusImpl        *ibus);

G_END_DECLS
#endif
