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
/**
 * SECTION: ibusservice
 * @short_description: IBus service back-end.
 * @stability: Stable
 *
 * An IBusService is a base class for services.
 */

#ifndef __IBUS_SERVICE_H_
#define __IBUS_SERVICE_H_

#include <dbus/dbus.h>
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

struct _IBusService {
    IBusObject parent;
    /* instance members */
};

typedef gboolean  (* ServiceIBusMessageFunc)    (IBusService    *service,
                                                 IBusConnection *connection,
                                                 IBusMessage    *message);
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
IBusService     *ibus_service_new               (const gchar    *path);
const gchar     *ibus_service_get_path          (IBusService    *service);
gboolean         ibus_service_handle_message    (IBusService    *service,
                                                 IBusConnection *connection,
                                                 IBusMessage    *message);
gboolean         ibus_service_add_to_connection (IBusService    *service,
                                                 IBusConnection *connection);
GList           *ibus_service_get_connections   (IBusService    *service);
gboolean         ibus_service_remove_from_connection
                                                (IBusService    *service,
                                                 IBusConnection *connection);
gboolean         ibus_service_remove_from_all_connections
                                                (IBusService    *service);
gboolean         ibus_service_send_signal       (IBusService    *service,
                                                 const gchar    *interface,
                                                 const gchar    *name,
                                                 GType           first_arg_type,
                                                 ...);
G_END_DECLS
#endif

