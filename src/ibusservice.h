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
 * SECTION: ibusservice
 * @short_description: IBus service back-end.
 * @stability: Stable
 *
 * An IBusService is a base class for services.
 */

#ifndef __IBUS_SERVICE_H_
#define __IBUS_SERVICE_H_

#include "ibusobject.h"
#include "ibusconnection.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_SERVICE             \
    (ibus_service_get_type ())
#define IBUS_SERVICE(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_SERVICE, IBusService))
#define IBUS_SERVICE_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_SERVICE, IBusServiceClass))
#define IBUS_IS_SERVICE(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_SERVICE))
#define IBUS_IS_SERVICE_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_SERVICE))
#define IBUS_SERVICE_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_SERVICE, IBusServiceClass))

G_BEGIN_DECLS

typedef struct _IBusService IBusService;
typedef struct _IBusServiceClass IBusServiceClass;

/**
 * IBusService:
 *
 * An opaque data type representing an IBusService.
 */
struct _IBusService {
    IBusObject parent;
    /* instance members */
};

/**
 * ServiceIBusMessageFunc:
 * @service: An IBsService.
 * @connection: Connection to IBus daemon.
 * @message: IBusMessage to be sent.
 * @returns: %TRUE if succeed; %FALSE if failed.
 *
 * Prototype of IBus service message sending callback function.
 */
typedef gboolean  (* ServiceIBusMessageFunc)    (IBusService    *service,
                                                 IBusConnection *connection,
                                                 IBusMessage    *message);

/**
 * ServiceIBusSignalFunc:
 * @service: An IBsService.
 * @connection: Connection to IBus daemon.
 * @message: IBusMessage to be sent.
 * @returns: %TRUE if succeed; %FALSE if failed.
 *
 * Prototype of IBus service signal sending callback function.
 */
typedef gboolean  (* ServiceIBusSignalFunc)     (IBusService    *service,
                                                 IBusConnection *connection,
                                                 IBusMessage    *message);

struct _IBusServiceClass {
    IBusObjectClass parent;

    /* signals */
    gboolean  (* ibus_message)      (IBusService    *service,
                                     IBusConnection *connection,
                                     IBusMessage    *message);
    gboolean  (* ibus_signal)       (IBusService    *service,
                                     IBusConnection *connection,
                                     IBusMessage    *message);
    /*< private >*/
    /* padding */
    gpointer pdummy[6];
};


GType            ibus_service_get_type          (void);

/**
 * ibus_service_new:
 * @path: Object path.
 * @returns: A newly allocated IBusService
 *
 * New an IBusService.
 */
IBusService     *ibus_service_new               (const gchar    *path);

/**
 * ibus_service_get_path:
 * @service: An IBusService.
 * @returns: The object path of @service
 *
 * Returns the object path of an IBusService.
 */
const gchar     *ibus_service_get_path          (IBusService    *service);

/**
 * ibus_service_handle_message:
 * @service: An IBusService.
 * @connection: Corresponding IBusCOnnection
 * @message: IBusMessage to be handled.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Emit an IBusMessage on an IBusConnection.
 */
gboolean         ibus_service_handle_message    (IBusService    *service,
                                                 IBusConnection *connection,
                                                 IBusMessage    *message);

/**
 * ibus_service_add_to_connection:
 * @service: An IBusService.
 * @connection: Corresponding IBusCOnnection
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Add an IBus Service to an IBusConnection.
 * This function also connects the service to the signal IBusConnection::destroy of the connection.
 */
gboolean         ibus_service_add_to_connection (IBusService    *service,
                                                 IBusConnection *connection);

/**
 * ibus_service_get_connections:
 * @service: An IBusService.
 * @returns: A newly allocated list of connections.
 *
 * Returns a copy of list of connections.
 * List elements need to be unref by g_object_unref().
 */
GList           *ibus_service_get_connections   (IBusService    *service);

/**
 * ibus_service_remove_from_connection:
 * @service: An IBusService.
 * @connection: Corresponding IBusCOnnection
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Remove an IBusService from an IBusConnection.
 * This function also disconnects the signal IBusConnection::destroy.
 */
gboolean         ibus_service_remove_from_connection
                                                (IBusService    *service,
                                                 IBusConnection *connection);

/**
 * ibus_service_remove_from_all_connections:
 * @service: An IBusService.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Remove an IBusService from all connections.
 * This function also disconnects the signal IBusConnection::destroy.
 */
gboolean         ibus_service_remove_from_all_connections
                                                (IBusService    *service);

/**
 * ibus_service_send_signal:
 * @service: An IBusService.
 * @interface: The interface the signal is emitted from.
 * @name: Name of the signal.
 * @first_arg_type: Type of first argument.
 * @...: Rest of arguments, NULL to mark the end.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Send signal to all the IBusConnections of an IBusService.
 *
 * @see_also: ibus_connection_send_signal()
 */
gboolean         ibus_service_send_signal       (IBusService    *service,
                                                 const gchar    *interface,
                                                 const gchar    *name,
                                                 GType           first_arg_type,
                                                 ...);
G_END_DECLS
#endif

