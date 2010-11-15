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
#include "server.h"
#include <gio/gio.h>
#include "dbusimpl.h"
#include "ibusimpl.h"
#include "option.h"

static GDBusServer *server = NULL;
static GMainLoop *mainloop = NULL;
static BusDBusImpl *dbus = NULL;
static BusIBusImpl *ibus = NULL;
static gchar *address = NULL;

/**
 * bus_new_connection_cb:
 * @user_data: always NULL.
 * @returns: TRUE when the function can handle the connection.
 *
 * Handle incoming connections.
 */
static gboolean
bus_new_connection_cb (GDBusServer     *server,
                       GDBusConnection *dbus_connection,
                       gpointer         user_data)
{
    BusConnection *connection = bus_connection_new (dbus_connection);
    bus_dbus_impl_new_connection (dbus, connection);

    if (g_object_is_floating (connection)) {
        /* bus_dbus_impl_new_connection couldn't handle the connection. just delete the connection and return TRUE
         * (so that other connection handler will not handle the deleted connection.) */
        ibus_object_destroy ((IBusObject *)connection);
        g_object_unref (connection);
    }
    return TRUE;
}

void
bus_server_init (void)
{
    dbus = bus_dbus_impl_get_default ();
    ibus = bus_ibus_impl_get_default ();
    bus_dbus_impl_register_object (dbus, (IBusService *)ibus);

    /* init server */
    GDBusServerFlags flags = G_DBUS_SERVER_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS;
    gchar *guid = g_dbus_generate_guid ();
    server =  g_dbus_server_new_sync (
                    g_address, /* the place where the socket file lives, e.g. /tmp, abstract namespace, etc. */
                    flags, guid,
                    NULL /* observer */,
                    NULL /* cancellable */,
                    NULL /* error */);
    g_free (guid);

    g_signal_connect (server, "new-connection", G_CALLBACK (bus_new_connection_cb), NULL);

    g_dbus_server_start (server);

    address = g_strdup_printf ("%s,guid=%s",
                               g_dbus_server_get_client_address (server),
                               g_dbus_server_get_guid (server));

    /* write address to file */
    ibus_write_address (address);
}

const gchar *
bus_server_get_address (void)
{
    return address;
}

void
bus_server_run (void)
{
    g_return_if_fail (server);

    /* create and run main loop */
    mainloop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (mainloop);

    /* bus_server_quit is called. stop server */
    g_dbus_server_stop (server);

    ibus_object_destroy ((IBusObject *)dbus);
    ibus_object_destroy ((IBusObject *)ibus);

    /* release resources */
    g_object_unref (server);
    g_main_loop_unref (mainloop);
    mainloop = NULL;
    g_free (address);
    address = NULL;
}

void
bus_server_quit (void)
{
    if (mainloop)
        g_main_loop_quit (mainloop);
}
