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
/**
 * SECTION: ibusconnection
 * @short_description: DBusConnection wrapper.
 * @title: IBusConnection
 * @stability: Stable
 * <ulink url="http://dbus.freedesktop.org/doc/api/html/structDBusConnection.html">DBusConnection</ulink>
 *
 * An IBusConnection provides #DBusConnection wrapper, and is used to connect to either D-Bus or IBus daemon.
 * Usually, IBusConnection is set to a #DBusConnection and emitting ibus-message when
 * receiving incoming messages from the #DBusConnection.
 *
 * @see_also: #IBusMessage
 *
 */

#ifndef __IBUS_CONNECTION_H_
#define __IBUS_CONNECTION_H_

#include "ibusdbus.h"
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

/**
 * IBusIBusMessageFunc:
 * @connection: An IBusConnection.
 * @message: An IBusMessage.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Prototype of an IBusIBusMessage callback function.
 */
typedef gboolean (* IBusIBusMessageFunc)(IBusConnection     *connection,
                                         IBusMessage        *message);

/**
 * IBusIBusSignalFunc:
 * @connection: An IBusConnection.
 * @message: An IBusMessage.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Prototype of an IBusIBusSignal callback function.
 */
typedef gboolean (* IBusIBusSignalFunc) (IBusConnection     *connection,
                                         IBusMessage        *message);

/**
 * IBusMessageFunc:
 * @connection: An IBusConnection.
 * @message: An IBusMessage.
 * @user_data: User data for the callback function.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Prototype of an IBusMessage callback function.
 */
typedef gboolean (* IBusMessageFunc)    (IBusConnection     *connection,
                                         IBusMessage        *message,
                                         gpointer            user_data);

/**
 * IBusConnectionReplyFunc:
 * @connection: An IBusConnection.
 * @reply: An IBusMessage.
 * @user_data: User data for the callback function.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Prototype of an IBusConnectionReplyFunc callback function.
 */
typedef void     (* IBusConnectionReplyFunc)
                                        (IBusConnection     *connection,
                                         IBusMessage        *reply,
                                         gpointer            user_data);

/**
 * IBusConnection:
 *
 * An opaque data type representing an IBusConnection.
 */
struct _IBusConnection {
  IBusObject parent;
  /* instance members */
};

struct _IBusConnectionClass {
    IBusObjectClass parent;

    /* signals */
    gboolean    (* authenticate_unix_user)
                                    (IBusConnection   *connection,
                                     gulong            uid);
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

/**
 * ibus_connection_set_connection:
 * @connection: An IBusConnection.
 * @dbus_connection: A D-Bus connection.
 * @shared: Whether the @dbus_connection is shared.
 *
 * Set an IBusConnection as data of a D-Bus connection.
 * Emit signal <constant>ibus-message</constant> when receiving incoming message from @dbus_connection.
 */
void             ibus_connection_set_connection     (IBusConnection     *connection,
                                                     DBusConnection     *dbus_connection,
                                                     gboolean            shared);

/**
 * ibus_connection_open:
 * @address: A remote address.
 * @returns: A newly allocated IBusConnection which is set to a D-Bus connection corresponding to @address.
 *
 * Open an IBusConnection that is set to a D-Bus connection to the specified address.
 * Use ibus_connection_open_private() to get a dedicated connection not shared with other callers of
 * ibus_connection_open().
 *
 * @see_also: ibus_connection_open_private().
 */
IBusConnection  *ibus_connection_open               (const gchar        *address);

/**
 * ibus_connection_open_private:
 * @address: A remote address.
 * @returns: A newly allocated IBusConnection which is set to a D-Bus connection corresponding to @address.
 *
 * Open an IBusConnection that is set to a D-Bus connection to the specified address.
 * Unlike ibus_connection_open(), this function always creates a new D-Bus connection.
 * The D-Bus connection will not be saved or recycled by libdbus.
 *
 * In D-Bus documentation, dbus_connection_open() is preferred over dbus_connection_open_private(),
 * so should ibus_connection_open() be preferred over ibus_connection_open_private().
 *
 * @see_also: ibus_connection_open().
 */
IBusConnection  *ibus_connection_open_private       (const gchar        *address);

/**
 * ibus_connection_close:
 * @connection: An IBusConnection.
 *
 * Close an IBusCOnnection and corresponding D-Bus connection.
 */
void             ibus_connection_close              (IBusConnection     *connection);

/**
 * ibus_connection_is_connected:
 * @connection: An IBusConnection.
 * @returns: TRUE for connected; FALSE otherwise.
 *
 * Whether an IBusConnection is connected.
 */
gboolean         ibus_connection_is_connected       (IBusConnection     *connection);

/**
 * ibus_connection_is_authenticated:
 * @connection: An IBusConnection.
 * @returns: TRUE for authenticated; FALSE otherwise.
 *
 * Whether an IBusConnection is authenticated.
 */
gboolean         ibus_connection_is_authenticated   (IBusConnection     *connection);

/**
 * ibus_connection_get_connection:
 * @connection: An IBusConnection.
 * @returns: The corresponding DBusConnection.
 *
 * Return corresponding DBusConnection.
 */
DBusConnection  *ibus_connection_get_connection     (IBusConnection     *connection);

/**
 * ibus_connection_get_unix_user:
 * @connection: An IBusConnection.
 * @returns: The UNIX UID of peer user.
 *
 * Return The UNIX UID of peer user.
 */
glong            ibus_connection_get_unix_user      (IBusConnection     *connection);

/**
 * ibus_connection_read_write_dispatch:
 * @connection: An IBusConnection.
 * @timeout: Maximum time to block or -1 for infinite.
 * @returns: TRUE if the disconnect message has not been processed; FALSE otherwise.
 *
 * Return TRUE if the disconnect message has not been processed.
 * This function is a wrapper of dbus_connection_read_write_dispatch(),
 * which is also intended for use with applications that don't want to
 * write a main loop and deal with DBusWatch and DBusTimeout.
 * Following text is
 * from the documentation of dbus_connection_read_write_dispatch():
 * An example usage would be:
 * <informalexample>
 *     <programlisting>
 *  while (dbus_connection_read_write_dispatch (connection, -1))
 * ; // empty loop body
 *     </programlisting>
 * </informalexample>
 * In this usage you would normally have set up a filter function to look at each message as it is dispatched.
 * The loop terminates when the last message from the connection (the disconnected signal) is processed.
 *
 * If there are messages to dispatch, this function will dbus_connection_dispatch() once, and return.
 * If there are no messages to dispatch, this function will block until it can read or write,
 * then read or write, then return.
 *
 * The way to think of this function is that it either makes some sort of progress,
 * or it blocks. Note that, while it is blocked on I/O, it cannot be interrupted (even by other threads),
 * which makes this function unsuitable for applications that do more than just react to received messages.
 *
 * @see_also: dbus_connection_read_write_dispatch().
 */
gboolean         ibus_connection_read_write_dispatch(IBusConnection     *connection,
                                                     gint                timeout);

/**
 * ibus_connection_send:
 * @connection: An IBusConnection.
 * @message: IBusMessage to be sent.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Send an IBusMessage to an IBusConnection.
 * If succeed, signal <constant>ibus-message-sent</constant> is emitted.
 *
 * @see_also: ibus_connection_send_with_reply(), ibus_connection_send_with_reply_and_block(),
 * ibus_connection_send_signal(), ibus_connection_send_signal_valist(), ibus_connection_send_valist(),
 * dbus_connection_send().
 */
gboolean         ibus_connection_send               (IBusConnection     *connection,
                                                     IBusMessage        *message);

/**
 * ibus_connection_send_signal:
 * @connection: An IBusConnection.
 * @path: The path to the object emitting the signal.
 * @interface: The interface the signal is emitted from.
 * @name: Name of the signal.
 * @first_arg_type: Type of first argument.
 * @...: Rest of arguments, NULL to mark the end.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Send a wrapped D-Bus signal to an IBusConnection.
 * This function wraps a signal as an IBusMessage, then sent the IBusMessage
 * via ibus_connection_send().
 *
 * @see_also: ibus_connection_send(), ibus_connection_send_signal_valist(), ibus_message_new_signal().
 */
gboolean         ibus_connection_send_signal        (IBusConnection     *connection,
                                                     const gchar        *path,
                                                     const gchar        *interface,
                                                     const gchar        *name,
                                                     GType               first_arg_type,
                                                     ...);

/**
 * ibus_connection_send_signal_valist:
 * @connection: An IBusConnection.
 * @path: The path to the object emitting the signal.
 * @interface: The interface the signal is emitted from.
 * @name: Name of the signal.
 * @first_arg_type: Type of first arg.
 * @args: Ret of arguments.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Send a wrapped D-Bus signal to an IBusConnection.
 * This function wraps a signal as an IBusMessage, then sent the IBusMessage
 * via ibus_connection_send().
 *
 * @see_also: ibus_connection_send(), ibus_connection_send_signal(), ibus_connection_send_valist(),
 *  ibus_message_new_signal().
 */
gboolean         ibus_connection_send_signal_valist (IBusConnection     *connection,
                                                     const gchar        *path,
                                                     const gchar        *interface,
                                                     const gchar        *name,
                                                     GType               first_arg_type,
                                                     va_list             args);

/**
 * ibus_connection_send_valist:
 * @connection: An IBusConnection.
 * @message_type: Message type.
 * @path: The path to the object emitting the signal.
 * @interface: The interface the signal is emitted from.
 * @name: Name of the signal.
 * @first_arg_type: Type of first arg.
 * @args: Ret of arguments.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Send a wrapped D-Bus message to an IBusConnection.
 *
 * This function wraps a D-Bus message as an IBusMessage, then sent the IBusMessage
 * via ibus_connection_send().
 *
 * Message type can be specified with @message_type.
 * Types include <constant>DBUS_MESSAGE_TYPE_METHOD_CALL</constant>,
 * <constant>DBUS_MESSAGE_TYPE_METHOD_RETURN</constant>,
 * <constant>DBUS_MESSAGE_TYPE_ERROR</constant>,
 * <constant>DBUS_MESSAGE_TYPE_SIGNAL</constant>,
 * but other types are allowed and all code must silently ignore messages of unknown type.
 * <constant>DBUS_MESSAGE_TYPE_INVALID</constant> will never be returned.
 *
 * @see_also: ibus_connection_send(), ibus_connection_send_singal_valist(),
 * ibus_connection_call(),
 * ibus_message_new_signal(),
 * dbus_message_get_type().
 */
gboolean         ibus_connection_send_valist        (IBusConnection     *connection,
                                                     gint                message_type,
                                                     const gchar        *path,
                                                     const gchar        *interface,
                                                     const gchar        *name,
                                                     GType               first_arg_type,
                                                     va_list             args);

/**
 * ibus_connection_send_with_reply:
 * @connection: An IBusConnection.
 * @message: An IBusMessage.
 * @pending_return: Return location of a IBusPendingCall object, or NULL if connection is disconnected.
 * @timeout_milliseconds: timeout in milliseconds or -1 for default.
 * @returns: FALSE if no memory, TRUE otherwise.
 *
 * Queues an IBusMessage to send,  and returns a IBusPendingCall used to receive a reply to the message.
 * This function is a wrapper of dbus_connection_send_with_reply().
 *
 * @see_also: ibus_connection_send(), ibus_connection_send_with_reply_and_block(),
 * ibus_proxy_call_with_reply(),
 * #IBusPendingCall, dbus_connection_send_with_reply()
 */
gboolean         ibus_connection_send_with_reply    (IBusConnection     *connection,
                                                     IBusMessage        *message,
                                                     IBusPendingCall   **pending_return,
                                                     gint                timeout_milliseconds);

/**
 * ibus_connection_send_with_reply_and_block:
 * @connection: An IBusConnection.
 * @message: An IBusMessage.
 * @timeout_milliseconds: timeout in milliseconds or -1 for default.
 * @error: Returned error is stored here; NULL to ignore error.
 * @returns: An IBusMessage that is the reply or NULL with an error code if the function fails.
 *
 * Sends an IBus message and blocks a certain time period while waiting for
 * an IBusMessage as reply.
 * If the IBusMessage is not NULL,  signal <constant>ibus-message-sent</constant> is emitted.
 *
 * @see_also: ibus_connection_send(), ibus_connection_send_with_reply(),
 * dbus_connection_send_with_reply_and_block()
 */
IBusMessage     *ibus_connection_send_with_reply_and_block
                                                    (IBusConnection     *connection,
                                                     IBusMessage        *message,
                                                     gint                timeout_milliseconds,
                                                     IBusError          **error);

/**
 * ibus_connection_call:
 * @connection: An IBusConnection.
 * @name: Name of the signal.
 * @path: The path to the object emitting the signal.
 * @interface: The interface the signal is emitted from.
 * @member: The name of the member function to be called.
 * @error: Returned error is stored here; NULL to ignore error.
 * @first_arg_type: Type of first argument.
 * @...: Rest of arguments, NULL to mark the end.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Invoke a member function by sending an IBusMessage. This method does not
 * support reply message, use ibus_connection_call_with_reply instead.
 *
 * @see_also: ibus_connection_send_valist().
 */
gboolean         ibus_connection_call               (IBusConnection     *connection,
                                                     const gchar        *name,
                                                     const gchar        *path,
                                                     const gchar        *interface,
                                                     const gchar        *member,
                                                     IBusError          **error,
                                                     GType              first_arg_type,
                                                     ...);

/**
 * ibus_connection_call_with_reply:
 * @connection: An IBusConnection.
 * @name: Name of the signal.
 * @path: The path to the object emitting the signal.
 * @interface: The interface the signal is emitted from.
 * @member: The name of the member function to be called.
 * @error: Returned error is stored here; NULL to ignore error.
 * @first_arg_type: Type of first argument.
 * @...: Rest of arguments, NULL to mark the end.
 * @returns: Reply message, or NULL when fail. The returned message must be
 * freed with ibus_message_unref().
 *
 * Invoke a member function by sending an IBusMessage.
 *
 * @see_also: ibus_connection_send_valist().
 */
IBusMessage     *ibus_connection_call_with_reply    (IBusConnection     *connection,
                                                     const gchar        *name,
                                                     const gchar        *path,
                                                     const gchar        *interface,
                                                     const gchar        *member,
                                                     IBusError          **error,
                                                     GType              first_arg_type,
                                                     ...);

/**
 * ibus_connection_flush:
 * @connection: An IBusConnection.
 *
 * Blocks until the outgoing message queue is empty.
 * This function is a wrapper of dbus_connection_flush().
 *
 * @see_also: dbus_connection_flush()
 */
void             ibus_connection_flush              (IBusConnection     *connection);

/**
 * ibus_connection_register_object_path:
 * @connection: An IBusConnection.
 * @path: Object path to be register.
 * @message_func: Callback function for message handling.
 * @user_data: User data for @message_func.
 * @returns: FALSE if fail because of out of memory; TRUE otherwise.
 *
 * Registers a handler for a given path in the object hierarchy.
 * The given vtable handles messages sent to exactly the given path.
 * This function is a wrapper of dbus_connection_register_object_path().
 *
 * @see_also: ibus_connection_register_object_path()
 */
gboolean         ibus_connection_register_object_path
                                                    (IBusConnection     *connection,
                                                     const gchar        *path,
                                                     IBusMessageFunc    message_func,
                                                     gpointer           user_data);

/**
 * ibus_connection_unregister_object_path:
 * @connection: An IBusConnection.
 * @path: Object path to be unregister.
 * @returns: FALSE if fail because of out of memory; TRUE otherwise.
 *
 * Unregisters the handler registered with exactly the given path.
 * It's a bug to call this function for a path that isn't registered.
 * Can unregister both fallback paths and object paths.
 * This function is a wrapper of dbus_connection_unregister_object_path()
 */
gboolean         ibus_connection_unregister_object_path
                                                    (IBusConnection     *connection,
                                                     const gchar        *path);

G_END_DECLS
#endif
