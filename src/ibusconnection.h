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
 * SECTION: ibusconnection
 * @short_description: DBusConnection wrapper.
 * @see_also: <ulink url="http://dbus.freedesktop.org/doc/api/html/structDBusConnection.html">DBusConnection</ulink>
 *
 * An IBusConnection provides DBusConnection wrapper.
 * It can be used to connect to either dBus or IBus daemon.
 */

#ifndef __IBUS_CONNECTION_H_
#define __IBUS_CONNECTION_H_

#include <dbus/dbus.h>
#include "ibusmessage.h"
#include "ibuspendingcall.h"
#include "ibusobject.h"
#include "ibuserror.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_CONNECTION             \
    (ibus_connection_get_type ())
#define IBUS_CONNECTION(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_CONNECTION, IBusConnection))
#define IBUS_CONNECTION_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_CONNECTION, IBusConnectionClass))
#define IBUS_IS_CONNECTION(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_CONNECTION))
#define IBUS_IS_CONNECTION_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_CONNECTION))
#define IBUS_CONNECTION_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_CONNECTION, IBusConnectionClass))

G_BEGIN_DECLS

typedef struct _IBusConnection IBusConnection;
typedef struct _IBusConnectionClass IBusConnectionClass;

typedef gboolean (* IBusIBusMessageFunc)(IBusConnection     *connection,
                                         IBusMessage        *message);
typedef gboolean (* IBusIBusSignalFunc) (IBusConnection     *connection,
                                         IBusMessage        *message);
typedef gboolean (* IBusMessageFunc)    (IBusConnection     *connection,
                                         IBusMessage        *message,
                                         gpointer            user_data);
typedef void     (* IBusConnectionReplyFunc)
                                        (IBusConnection     *connection,
                                         IBusMessage        *reply,
                                         gpointer            user_data);

struct _IBusConnection {
  IBusObject parent;
  /* instance members */
};

struct _IBusConnectionClass {
    IBusObjectClass parent;

    /* signals */
    gboolean    (* ibus_message)    (IBusConnection   *connection,
                                     IBusMessage      *message);
    gboolean    (* ibus_signal)     (IBusConnection   *connection,
                                     IBusMessage      *message);
    void        (* ibus_message_sent)
                                    (IBusConnection   *connection,
                                     IBusMessage      *message);
    void        (* disconnected)    (IBusConnection   *connection);

    /*< private >*/
    /* padding */
    gpointer pdummy[4];
};

GType            ibus_connection_get_type           (void);

/**
 * ibus_connection_new:
 * @returns: An newly allocated IBusConnection.
 *
 * New an IBusConnection.
 */
IBusConnection  *ibus_connection_new                (void);
void             ibus_connection_set_connection     (IBusConnection     *connection,
                                                     DBusConnection     *dbus_connection,
                                                     gboolean            shared);
IBusConnection  *ibus_connection_open               (const gchar        *address);
IBusConnection  *ibus_connection_open_private       (const gchar        *address);
void             ibus_connection_close              (IBusConnection     *connection);
gboolean         ibus_connection_is_connected       (IBusConnection     *connection);
DBusConnection  *ibus_connection_get_connection     (IBusConnection     *connection);
gboolean         ibus_connection_read_write_dispatch(IBusConnection     *connection,
                                                     gint                timeout);
gboolean         ibus_connection_send               (IBusConnection     *connection,
                                                     IBusMessage        *message);
gboolean         ibus_connection_send_signal        (IBusConnection     *connection,
                                                     const gchar        *path,
                                                     const gchar        *interface,
                                                     const gchar        *name,
                                                     GType               first_arg_type,
                                                     ...);
gboolean         ibus_connection_send_signal_valist (IBusConnection     *connection,
                                                     const gchar        *path,
                                                     const gchar        *interface,
                                                     const gchar        *name,
                                                     GType               first_arg_type,
                                                     va_list             args);
gboolean         ibus_connection_send_valist        (IBusConnection     *connection,
                                                     gint                message_type,
                                                     const gchar        *path,
                                                     const gchar        *interface,
                                                     const gchar        *name,
                                                     GType               first_arg_type,
                                                     va_list             args);
gboolean         ibus_connection_send_with_reply    (IBusConnection     *connection,
                                                     IBusMessage        *message,
                                                     IBusPendingCall   **pending_return,
                                                     gint                timeout_milliseconds);
IBusMessage     *ibus_connection_send_with_reply_and_block
                                                    (IBusConnection     *connection,
                                                     IBusMessage        *message,
                                                     gint                timeout_milliseconds,
                                                     IBusError          **error);
gboolean         ibus_connection_call               (IBusConnection     *connection,
                                                     const gchar        *name,
                                                     const gchar        *path,
                                                     const gchar        *interface,
                                                     const gchar        *member,
                                                     IBusError          **error,
                                                     GType              first_arg_type,
                                                     ...);
void             ibus_connection_flush              (IBusConnection     *connection);
gboolean         ibus_connection_register_object_path
                                                    (IBusConnection     *connection,
                                                     const gchar        *path,
                                                     IBusMessageFunc    message_func,
                                                     gpointer           user_data);
gboolean         ibus_connection_unregister_object_path
                                                    (IBusConnection     *connection,
                                                     const gchar        *path);

G_END_DECLS
#endif

