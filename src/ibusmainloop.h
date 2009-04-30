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
#ifndef __IBUS_MAINLOOP_H_
#define __IBUS_MAINLOOP_H_

#include <glib.h>
#include <dbus/dbus.h>

typedef void (* DBusConnectionSetupFunc)    (DBusConnection *connection,
                                             gpointer        user_data);
typedef void (* DBusServerSetupFunc)        (DBusServer     *server,
                                             gpointer        user_data);

void    ibus_mainloop_setup         (DBusConnectionSetupFunc      connection_func,
                                     DBusServerSetupFunc          server_func,
                                     gpointer                     user_data);
void    ibus_dbus_server_setup      (DBusServer                  *server);
void    ibus_dbus_connection_setup  (DBusConnection              *connection);

#endif

