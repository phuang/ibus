/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
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
#include "ibusmainloop.h"
#include "ibusinternal.h"

static DBusConnectionSetupFunc _connection_setup_func = (DBusConnectionSetupFunc) dbus_connection_setup;
static DBusServerSetupFunc _server_setup_func = (DBusServerSetupFunc) dbus_server_setup;
static gpointer _user_data = NULL;

void
ibus_mainloop_setup (DBusConnectionSetupFunc connection_func,
                     DBusServerSetupFunc     server_func,
                     gpointer                user_data)
{
    _connection_setup_func = connection_func;
    _server_setup_func = server_func;
    _user_data = user_data;
}

void
ibus_dbus_connection_setup (DBusConnection *connection)
{
    if (_connection_setup_func != NULL)
        (_connection_setup_func) (connection, _user_data);
}

void
ibus_dbus_server_setup (DBusServer *server)
{
    if (_server_setup_func != NULL)
        (_server_setup_func) (server, _user_data);
}

