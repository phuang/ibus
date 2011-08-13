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

#if !defined (__IBUS_H_INSIDE__) && !defined (IBUS_COMPILATION)
#error "Only <ibus.h> can be included directly"
#endif

/**
 * SECTION: ibusservice
 * @short_description: IBus service back-end.
 * @stability: Stable
 *
 * An IBusService is a base class for services.
 */

#ifndef __IBUS_SERVICE_H_
#define __IBUS_SERVICE_H_

#include <gio/gio.h>
#include "ibusobject.h"

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
typedef struct _IBusServicePrivate IBusServicePrivate;

/**
 * IBusService:
 *
 * An opaque data type representing an IBusService.
 */
struct _IBusService {
    /*< private >*/
    IBusObject parent;
    IBusServicePrivate *priv;
};

struct _IBusServiceClass {
    /*< private >*/
    IBusObjectClass parent;

    /*< public >*/
    /* virtual functions */
    void        (* service_method_call)
                                    (IBusService        *service,
                                     GDBusConnection    *connection,
                                     const gchar        *sender,
                                     const gchar        *object_path,
                                     const gchar        *interface_name,
                                     const gchar        *method_name,
                                     GVariant           *parameters,
                                     GDBusMethodInvocation
                                                        *invocation);
    GVariant *  (* service_get_property)
                                    (IBusService        *service,
                                     GDBusConnection    *connection,
                                     const gchar        *sender,
                                     const gchar        *object_path,
                                     const gchar        *interface_name,
                                     const gchar        *property_name,
                                     GError            **error);
    gboolean    (* service_set_property)
                                    (IBusService        *service,
                                     GDBusConnection    *connection,
                                     const gchar        *sender,
                                     const gchar        *object_path,
                                     const gchar        *interface_name,
                                     const gchar        *property_name,
                                     GVariant           *value,
                                     GError            **error);
    /*< private >*/
    GArray *interfaces;

    /* padding */
    gpointer pdummy[4];
};


GType            ibus_service_get_type          (void);

/**
 * ibus_service_new:
 * @path: Object path.
 * @returns: A newly allocated IBusService
 *
 * New an IBusService.
 */
IBusService     *ibus_service_new               (GDBusConnection    *connection,
                                                 const gchar        *path);
/**
 * ibus_service_get_object_path:
 * @service: An IBusService.
 * @returns: The object path of @service
 *
 * Returns the object path of an IBusService.
 */
const gchar     *ibus_service_get_object_path   (IBusService        *service);

/**
 * ibus_service_get_connection:
 * @service: An IBusService.
 * @returns: (transfer none): A #GDBusConnection of an #IBusService instance.
 *
 * Returns a connections.
 */
GDBusConnection *ibus_service_get_connection    (IBusService        *service);

/**
 * ibus_service_register:
 * @service: An IBusService.
 * @connection: A GDBusConnection the service will be registered to.
 * @error: Return location for error or NULL.
 * @returns: TRUE if the service was registered, FALSE otherwise.
 *
 * Registers service to a connection.
 */
gboolean         ibus_service_register          (IBusService        *service,
                                                 GDBusConnection    *connection,
                                                 GError            **error);
/**
 * ibus_service_unregister:
 * @service: An IBusService.
 * @connection: A GDBusConnection the service was registered with.
 *
 * Unregisters service from a connection.
 */
void             ibus_service_unregister        (IBusService        *service,
                                                 GDBusConnection    *connection);



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
 * @see_also: g_dbus_connection_emit_signal()
 */
gboolean         ibus_service_emit_signal       (IBusService        *service,
                                                 const gchar        *dest_bus_name,
                                                 const gchar        *interface_name,
                                                 const gchar        *signal_name,
                                                 GVariant           *parameters,
                                                 GError            **error);
/**
 * ibus_service_class_add_interfaces:
 * @klass: An IBusServiceClass.
 * @xml_data: The introspection xml data.
 *
 * Set the interface introspection information with the service class.
 */
gboolean         ibus_service_class_add_interfaces
                                                (IBusServiceClass   *klass,
                                                 const gchar        *xml_data);


G_END_DECLS
#endif

