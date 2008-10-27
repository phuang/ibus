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
#ifndef __CONFIG_PROXY_H_
#define __CONFIG_PROXY_H_

#include <ibus.h>
#include "connection.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_CONFIG_PROXY             \
    (bus_config_proxy_get_type ())
#define BUS_CONFIG_PROXY(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_CONFIG_PROXY, BusConfigProxy))
#define BUS_CONFIG_PROXY_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_CONFIG_PROXY, BusConfigProxyClass))
#define BUS_IS_CONFIG_PROXY(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_CONFIG_PROXY))
#define BUS_IS_CONFIG_PROXY_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_CONFIG_PROXY))
#define BUS_CONFIG_PROXY_GET_CLASS(obj)   \
    (G_TYPE_CHECK_GET_CLASS ((obj), BUS_TYPE_CONFIG_PROXY, BusConfigProxyClass))

G_BEGIN_DECLS

typedef struct _BusConfigProxy BusConfigProxy;
typedef struct _BusConfigProxyClass BusConfigProxyClass;

struct _BusConfigProxy {
  IBusProxy parent;
  /* instance members */
};

struct _BusConfigProxyClass {
    IBusProxyClass parent;
    /* class members */
};

GType            bus_config_proxy_get_type          (void);
BusConfigProxy  *bus_config_proxy_new               (BusConnection      *connection);
void             bus_config_proxy_get_value         (BusConfigProxy     *config,
                                                     const gchar        *section,
                                                     const gchar        *name,
                                                     GValue             *value);
void             bus_config_proxy_set_value         (BusConfigProxy     *config,
                                                     const gchar        *section,
                                                     const gchar        *name,
                                                     const GValue       *value);
G_END_DECLS
#endif

