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
 * SECTION: ibusserver
 * @short_description: Server that listen on a socket and wait for connection requests.
 * @stability: Stable
 *
 * An IBusServer listen on a socket and wait for connections requests,
 * just like DBusServer.
 */
#ifndef __IBUS_SERVER_H_
#define __IBUS_SERVER_H_

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
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_SERVER, IBusServerClass))

G_BEGIN_DECLS

typedef struct _IBusServer IBusServer;
typedef struct _IBusServerClass IBusServerClass;
/**
 * IBusNewConnectionFunc:
 * @server: An IBusServer.
 * @connection: The corresponding IBusConnection.
 *
 * Prototype of new connection callback function.
 *
 * This callback should be connected to signal ::new-connection
 * to handle the event that a new connection is coming in.
 * In this handler, IBus could add a reference and continue processing the connection.
 * If no reference is added, the new connection will be released and closed after this signal.
 *
 * @see_also: ::new-connection
 */

typedef void (* IBusNewConnectionFunc) (IBusServer *server, IBusConnection *connection);

/**
 * IBusServer:
 *
 * An opaque object representing an IBusServer.
 */
struct _IBusServer {
    IBusObject parent;
    /* instance members */
};

struct _IBusServerClass {
    IBusObjectClass parent;

    /* signals */
    void  (* new_connection)    (IBusServer     *server,
                                 IBusConnection *connectin);
    /*< private >*/
    /* padding */
    gpointer pdummy[7];
};

GType            ibus_server_get_type           (void);

/**
 * ibus_server_new:
 * @returns: A newly allocated IBusServer instance.
 *
 * New an IBusServer.
 */
IBusServer      *ibus_server_new                (void);

/**
 * ibus_server_listen:
 * @server: An IBusServer.
 * @address: Address of this server.
 * @returns: TRUE if succeed ; FALSE otherwise.
 *
 * Listens for new connections on the given address.
 *
 * If there are multiple semicolon-separated address entries in the address,
 * tries each one and listens on the first one that works.
 *
 * Returns FALSE if listening fails for any reason.
 *
 * To free the server, applications must call first ibus_server_disconnect() and then dbus_server_unref().
 */
gboolean         ibus_server_listen             (IBusServer     *server,
                                                 const gchar    *address);

/**
 * ibus_server_disconnect:
 * @server: An IBusServer.
 *
 * Releases the server's address and stops listening for new clients.
 *
 * If called more than once, only the first call has an effect. Does not modify the server's reference count.
 */
void             ibus_server_disconnect         (IBusServer     *server);

/**
 * ibus_server_get_address:
 * @server: An IBusServer.
 * @returns: A newly allocated string which contain address.
 *
 * Returns the address of the server, as a newly-allocated string which must be freed by the caller.
 */
const gchar     *ibus_server_get_address        (IBusServer     *server);

/**
 * ibus_server_get_id:
 * @server: An IBusServer.
 * @returns: A newly allocated string which contain address.
 *
 * Returns the unique ID of the server, as a newly-allocated string which must be freed by the caller.
 *
 * This ID is normally used by clients to tell when two IBusConnection would be equivalent
 * (because the server address passed to ibus_connection_open() will have the same guid in the two cases).
 * ibus_connection_open() can re-use an existing connection with the same ID instead of opening a new connection.
 *
 * This is an ID unique to each IBusServer. Remember that an IBusServer represents only one mode of connecting,
 * so e.g. a bus daemon can listen on multiple addresses which will mean it has multiple IBusServer each with
 * their own ID.
 *
 * The ID is not a UUID in the sense of RFC4122; the details are explained in the D-Bus specification.
 * Returns the address of the server, as a newly-allocated string which must be freed by the caller.
 */
const gchar     *ibus_server_get_id             (IBusServer     *server);

/**
 * ibus_server_is_connected:
 * @server: An IBusServer.
 * @returns: TRUE if the server is still listening for new connections; FALSE otherwise.
 *
 * Returns TRUE if the server is still listening for new connections.
 */
gboolean         ibus_server_is_connected       (IBusServer     *server);

/**
 * ibus_server_set_auth_mechanisms:
 * @server: An IBusServer.
 * @mechanisms: NULL-terminated array of mechanisms.
 * @returns:  TRUE if succeed; FALSE if insufficient memory.
 *
 * Sets the authentication mechanisms that this server offers to clients,
 * as a NULL-terminated array of mechanism names.
 *
 * This function only affects connections created after it is called.
 * Pass NULL instead of an array to use all available mechanisms (this is the default behavior).
 *
 * The D-Bus specification describes some of the supported mechanisms.
 */
gboolean         ibus_server_set_auth_mechanisms(IBusServer     *server,
                                                 const gchar   **mechanisms);

G_END_DECLS
#endif

