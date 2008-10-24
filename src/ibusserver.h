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
#ifndef __IBUS_SERVER_H_
#define __IBUS_SERVER_H_

#include <dbus/dbus.h>
#include "ibusobject.h"
#include "ibusconnection.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_SERVER             \
    (ibus_server_get_type ())
#define IBUS_SERVER(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_SERVER, IBusServer))
#define IBUS_SERVER_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_SERVER, IBusServerClass))
#define IBUS_IS_SERVER(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_SERVER))
#define IBUS_IS_SERVER_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_SERVER))
#define IBUS_SERVER_GET_CLASS(obj)   \
    (G_TYPE_CHECK_GET_CLASS ((obj), IBUS_TYPE_SERVER, IBusServerClass))

G_BEGIN_DECLS

typedef struct _IBusServer IBusServer;
typedef struct _IBusServerClass IBusServerClass;

typedef void (* IBusNewConnectionFunc) (IBusServer *server, IBusConnection *connection);

struct _IBusServer {
  IBusObject parent;
  /* instance members */
};

struct _IBusServerClass {
  IBusObjectClass parent;

  /* class members */
  void  (* new_connection)  (IBusServer     *server,
                             IBusConnection *connectin);
};

GType            ibus_server_get_type           (void);
IBusServer      *ibus_server_new                (void);
gboolean         ibus_server_listen             (IBusServer     *server,
                                                 const gchar    *address);
void             ibus_server_disconnect         (IBusServer     *server);
const gchar     *ibus_server_get_address        (IBusServer     *server);
const gchar     *ibus_server_get_id             (IBusServer     *server);
gboolean         ibus_server_is_connected       (IBusServer     *server);

G_END_DECLS
#endif

