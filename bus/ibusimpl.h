/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang@gmail.com>
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
    (G_TYPE_CHECK_GET_CLASS ((obj), BUS_TYPE_IBUS_IMPL, BusIBusImplClass))

G_BEGIN_DECLS

typedef struct _BusIBusImpl BusIBusImpl;
typedef struct _BusIBusImplClass BusIBusImplClass;

struct _BusIBusImpl {
    IBusService parent;
    /* instance members */
};

struct _BusIBusImplClass {
    IBusServiceClass parent;

    /* class members */
};

GType            bus_ibus_impl_get_type         (void);
BusIBusImpl     *bus_ibus_impl_new              (void);
gboolean         bus_ibus_impl_new_connection   (BusIBusImpl    *ibus_impl,
                                                 BusConnection  *connection);

G_END_DECLS
#endif

