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
/**
 * SECTION: ibusmainloop
 * @short_description: DBus server and connection setup functions.
 * @stability: Stable
 *
 * This section defines the DBus server and connection setup functions,
 * and prototypes of their callback functions.
 */

#ifndef __IBUS_MAINLOOP_H_
#define __IBUS_MAINLOOP_H_

#include <glib.h>
#include "ibusdbus.h"

/**
 * DBusConnectionSetupFunc:
 * @connection: A DBusConnection
 * @user_data: User data to be passed to callback function.
 *
 * A prototype of callback to DBus connection setup function.
 */
typedef void (* DBusConnectionSetupFunc)    (DBusConnection *connection,
                                             gpointer        user_data);

/**
 * DBusServerSetupFunc:
 * @server: A DBusConnection
 * @user_data: User data to be passed to callback function.
 *
 * A prototype of DBus server setup function.
 */
typedef void (* DBusServerSetupFunc)        (DBusServer     *server,
                                             gpointer        user_data);

/**
 * ibus_mainloop_setup:
 * @connection_func: A DBus connection setup function.
 * @server_func: A prototype of DBus server setup function.
 * @user_data: User data to be passed to callback function.
 *
 * Sets the watch and timeout functions of a #DBusConnection
 * and #DBusServer to integrate the connection with the GLib main loop.
 *
 * Parameter @user_data should be in type #GMainContext.
 * It will be passed to both callback functions,
 * however, normally %NULL is sufficient.
 *
 * If called twice for the same user_data, does nothing the second
 * time. If called once with user_data A and once with user_data B,
 * user_data B replaces user_data A as the context monitoring the
 * connection.
 *
 * @see_also: ibus_dbus_connection_setup(), ibus_dbus_server_setup().
 */
void    ibus_mainloop_setup         (DBusConnectionSetupFunc      connection_func,
                                     DBusServerSetupFunc          server_func,
                                     gpointer                     user_data);

/**
 * ibus_dbus_server_setup:
 * @server: A DBusServer.
 *
 * Sets the watch and timeout functions of a #DBusServer
 * to integrate the server with the GLib main loop.
 *
 * This function uses the parameter @user_data and
 * server_func set with ibus_mainloop_setup(),
 * or fall back to NULL and dbus_server_setup() if those are not defined.
 *
 * @see_also: ibus_mainloop_setup(), dbus_server_setup().
 */
void    ibus_dbus_server_setup      (DBusServer                  *server);

/**
 * ibus_dbus_connection_setup:
 * @connection: A DBusConnection.
 *
 * Sets the watch and timeout functions of a #DBusConnection
 * to integrate the connection with the GLib main loop.
 *
 * This function uses the parameter @user_data and
 * connection_func set with ibus_mainloop_setup(),
 * or fall back to NULL and dbus_connection_setup() if those are not defined.
 *
 * @see_also: ibus_mainloop_setup(), dbus_connection_setup().
 */
void    ibus_dbus_connection_setup  (DBusConnection              *connection);

#endif

