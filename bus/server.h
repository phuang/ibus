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
#ifndef __SERVER_H_
#define __SERVER_H_

#include <ibus.h>
#include "dbusimpl.h"
#include "ibusimpl.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_SERVER             \
    (bus_server_get_type ())
#define BUS_SERVER(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_SERVER, BusServer))
#define BUS_SERVER_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_SERVER, BusServerClass))
#define BUS_IS_SERVER(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_SERVER))
#define BUS_IS_SERVER_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_SERVER))
#define BUS_SERVER_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_SERVER, BusServerClass))
#define BUS_DEFAULT_SERVER          \
    (bus_server_get_default ())

G_BEGIN_DECLS

typedef struct _BusServer BusServer;
typedef struct _BusServerClass BusServerClass;

struct _BusServer {
    IBusServer parent;

    /* instance members */
    GMainLoop *loop;

    BusDBusImpl *dbus;
    BusIBusImpl *ibus;

};

struct _BusServerClass {
  IBusServerClass parent;

  /* class members */
};

GType            bus_server_get_type        (void);
BusServer       *bus_server_get_default     (void);
gboolean         bus_server_listen          (BusServer  *server);
void             bus_server_run             (BusServer  *server);
void             bus_server_quit            (BusServer  *server);

G_END_DECLS
#endif

