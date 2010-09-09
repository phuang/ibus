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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

#include "server.h"
#include "connection.h"
#include "dbusimpl.h"
#include "ibusimpl.h"

/* functions prototype */
static void      bus_server_destroy     (BusServer          *server);
static void      bus_server_new_connection
                                        (BusServer          *server,
                                         BusConnection      *connection);

G_DEFINE_TYPE (BusServer, bus_server, IBUS_TYPE_SERVER)

static void
bus_server_class_init (BusServerClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_server_destroy;

    IBUS_SERVER_CLASS (klass)->new_connection =
            (IBusNewConnectionFunc) bus_server_new_connection;
}

BusServer *
bus_server_get_default (void)
{
    static BusServer *server = NULL;

    if (server == NULL) {
        server = (BusServer *) g_object_new (BUS_TYPE_SERVER,
                                             "connection-type", BUS_TYPE_CONNECTION,
                                             NULL);
        bus_dbus_impl_get_default ();
        bus_ibus_impl_get_default ();
    }
    return server;
}

gboolean
bus_server_listen (BusServer *server)
{
    g_assert (BUS_IS_SERVER (server));

    const gchar *mechanisms[] = {
        "EXTERNAL",
        NULL
    };

    const gchar *address = "unix:tmpdir=/tmp/";
    gboolean retval;

#if 0
    path = ibus_get_socket_folder ();
    mkdir (path, 0700);
    chmod (path, 0700);

    address = ibus_get_address ();
#endif

    retval = ibus_server_listen (IBUS_SERVER (server), address);

#if 0
    chmod (ibus_get_socket_path (), 0600);
#endif

    ibus_server_set_auth_mechanisms ((IBusServer *)server, mechanisms);

    if (!retval) {
#if 0
        g_printerr ("Can not listen on %s! Please try remove directory %s and run again.", address, path);
#else
        g_printerr ("Can not listen on %s!", address);
#endif
        exit (-1);
    }

    ibus_write_address (ibus_server_get_address (IBUS_SERVER (server)));

    return retval;
}

void
bus_server_run (BusServer *server)
{
    g_assert (BUS_IS_SERVER (server));

    g_main_loop_run (server->loop);
}

void
bus_server_quit (BusServer *server)
{
    g_assert (BUS_IS_SERVER (server));

    g_main_loop_quit (server->loop);
}

static void
bus_server_init (BusServer *server)
{
    server->loop = g_main_loop_new (NULL, FALSE);
    server->dbus = bus_dbus_impl_get_default ();
    server->ibus = bus_ibus_impl_get_default ();
}

static void
bus_server_new_connection (BusServer     *server,
                           BusConnection *connection)
{
    g_assert (BUS_IS_SERVER (server));
    bus_dbus_impl_new_connection (server->dbus, connection);
}

static void
bus_server_destroy (BusServer *server)
{
    g_assert (BUS_IS_SERVER (server));

    ibus_object_destroy ((IBusObject *) server->dbus);
    g_object_unref (server->dbus);
    ibus_object_destroy ((IBusObject *) server->ibus);
    g_object_unref (server->ibus);

    while (g_main_loop_is_running (server->loop)) {
        g_main_loop_quit (server->loop);
    }
    g_main_loop_unref (server->loop);

    IBUS_OBJECT_CLASS (bus_server_parent_class)->destroy (IBUS_OBJECT (server));
}
