/* vim:set et sts=4: */
/* ibus - The Input Bus
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
#ifndef __ENGINE_PROXY_H_
#define __ENGINE_PROXY_H_

#include <ibus.h>

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
    (G_TYPE_CHECK_GET_CLASS ((obj), BUS_TYPE_ENGINE_PROXY, BusEngineProxyClass))

G_BEGIN_DECLS

typedef struct _BusEngineProxy BusEngineProxy;
typedef struct _BusEngineProxyClass BusEngineProxyClass;

struct _BusEngineProxy {
  IBusProxy parent;
  /* instance members */
};

struct _BusEngineProxyClass {
    IBusProxyClass parent;
    /* class members */
};

GType            bus_engine_proxy_get_type          (void);
BusEngineProxy  *bus_engine_proxy_new               (void);
gboolean         bus_engine_proxy_handle_message    (BusEngineProxy *engine,
                                                     DBusMessage    *message);

G_END_DECLS
#endif

