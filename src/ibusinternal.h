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
 * SECTION: ibusinternal
 * @short_description: IBus DBus setting functions for internal use.
 * @title: IBusInternal
 * @stability: Stable
 *
 * This section contain several IBus house keeping functions.
 *
 * @see_also: #IBusMainLoop
 *
 */
#ifndef __IBUS_INTERNEL_H_
#define __IBUS_INTERNEL_H_

#include <glib.h>
#include "ibusdbus.h"

/**
 * I_:
 * @string: A string
 * @returns: The canonical representation for the string.
 *
 * Returns a canonical representation for string.
 * Interned strings can be compared for equality by comparing the pointers, instead of using strcmp().
 */
#define I_(string) g_intern_static_string (string)

/**
 * dbus_server_setup:
 * @server: the server
 * @context: the #GMainContext or #NULL for default
 *
 * Sets the watch and timeout functions of a #DBusServer
 * to integrate the server with the GLib main loop.
 * In most cases the context argument should be #NULL.
 *
 * If called twice for the same context, does nothing the second
 * time. If called once with context A and once with context B,
 * context B replaces context A as the context monitoring the
 * connection.
 */
void    dbus_server_setup       (DBusServer     *server,
                                 GMainContext   *context);

/**
 * dbus_connection_setup:
 * @connection: the connection
 * @context: the #GMainContext or #NULL for default context
 *
 * Sets the watch and timeout functions of a #DBusConnection
 * to integrate the connection with the GLib main loop.
 * Pass in #NULL for the #GMainContext unless you're
 * doing something specialized.
 *
 * If called twice for the same context, does nothing the second
 * time. If called once with context A and once with context B,
 * context B replaces context A as the context monitoring the
 * connection.
 */
void    dbus_connection_setup   (DBusConnection *connection,
                                 GMainContext   *context);



#endif

