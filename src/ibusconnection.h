/* vim:set et ts=4: */
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
#ifndef __IBUS_CONNECTION_H_
#define __IBUS_CONNECTION_H_

#include "ibusobject.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_CONNECTION             \
    (ibus_connection_get_type ())
#define IBUS_CONNECTION(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_CONNECTION, IBusConnection))
#define IBUS_CONNECTION_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_CONNECTION, IBusConnectionClass))
#define IBUS_IS_CONNECTION(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_CONNECTION))
#define IBUS_IS_CONNECTION_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_CONNECTION))
#define IBUS_CONNECTION_GET_CLASS(obj)   \
    (G_TYPE_CHECK_GET_CLASS ((obj), IBUS_TYPE_CONNECTION, IBusConnectionClass))

#if 0
#define DEBUG_FUNCTION_IN   g_debug("%s IN", __FUNCTION__);
#define DEBUG_FUNCTION_OUT  g_debug("%s OUT", __FUNCTION__);
#else
#define DEBUG_FUNCTION_IN
#define DEBUG_FUNCTION_OUT
#endif

G_BEGIN_DECLS

typedef struct _IBusConnection IBusConnection;
typedef struct _IBusConnectionClass IBusConnectionClass;
typedef struct _IBusConnectionPrivate IBusConnectionPrivate;

struct _IBusConnection {
  IBusObject parent;
  /* instance members */
  IBusConnectionPrivate *priv;
};

struct _IBusConnectionClass {
  IBusObjectClass parent;

  /* class members */
  void (* dbus_message)     (IBusConnection   *client);
  void (* dbus_signal)      (IBusConnection   *client);
};

GType            ibus_connection_get_type           (void);
IBusConnection  *ibus_connection_new                (void);

G_END_DECLS
#endif

