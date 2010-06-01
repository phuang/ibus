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
 * SECTION: ibusproxy
 * @short_description: Base proxy object.
 * @stability: Stable
 *
 * An IBusProxy is the base of all proxy objects,
 * which communicate the corresponding #IBusServices on the other end of IBusConnection.
 * For example, IBus clients (such as editors, web browsers) invoke the proxy object,
 * IBusInputContext to communicate with the InputContext service of the ibus-daemon.
 *
 * Almost all services have corresponding proxies, except very simple services.
 */
#ifndef __IBUS_PROXY_H_
#define __IBUS_PROXY_H_

#include <dbus/dbus.h>
#include "ibusobject.h"
#include "ibusconnection.h"
#include "ibusmessage.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_PROXY             \
    (ibus_proxy_get_type ())
#define IBUS_PROXY(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_PROXY, IBusProxy))
#define IBUS_PROXY_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_PROXY, IBusProxyClass))
#define IBUS_IS_PROXY(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_PROXY))
#define IBUS_IS_PROXY_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_PROXY))
#define IBUS_PROXY_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_PROXY, IBusProxyClass))

G_BEGIN_DECLS

typedef struct _IBusProxy IBusProxy;
typedef struct _IBusProxyClass IBusProxyClass;

/**
 * IBusProxy:
 *
 * An opaque data type representing an IBusProxy.
 */
struct _IBusProxy {
    IBusObject parent;
    /* instance members */
};

struct _IBusProxyClass {
    IBusObjectClass parent;

    /* class members */
    gboolean    (* ibus_signal)     (IBusProxy      *proxy,
                                     IBusMessage    *message);
    /*< private >*/
    /* padding */
    gpointer pdummy[7];
};

GType            ibus_proxy_get_type        (void);

/**
 * ibus_proxy_new:
 * @name: The service name of proxy object.
 * @path: The path of proxy object.
 * @connection: An IBusConnection.
 * @returns: A newly allocated IBusProxy instance.
 *
 * New an IBusProxy instance.
 * Property IBusProxy:name is set as @name, and
 * IBusProxy:path is set as @path.
 */
IBusProxy       *ibus_proxy_new             (const gchar        *name,
                                             const gchar        *path,
                                             IBusConnection     *connection);

/**
 * ibus_proxy_send:
 * @proxy: An IBusProxy.
 * @message: The IBusMessage to be sent.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Send an #IBusMessage to the corresponding service.
 *
 * @see_also ibus_proxy_call(), ibus_proxy_send_with_reply(), ibus_proxy_send_with_reply_and_block().
 */
gboolean         ibus_proxy_send            (IBusProxy          *proxy,
                                             IBusMessage        *message);

/**
 * ibus_proxy_call:
 * @proxy: An IBusProxy.
 * @method: The method to be called.
 * @first_arg_type: Type of first argument.
 * @...: Rest of arguments, NULL to mark the end.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Call a method of the corresponding service.
 *
 * @see_also ibus_proxy_send(), ibus_proxy_call_with_reply(), ibus_proxy_call_with_reply_and_block().
 */
gboolean         ibus_proxy_call            (IBusProxy          *proxy,
                                             const gchar        *method,
                                             GType               first_arg_type,
                                             ...);

/**
 * ibus_proxy_call_with_reply:
 * @proxy: An IBusProxy.
 * @method: The method to be called.
 * @pending: Return location of a IBusPendingCall object, or NULL if connection is disconnected.
 * @timeout_milliseconds: Time out in milliseconds.
 * @error: Returned error is stored here; NULL to ignore error.
 * @first_arg_type: Type of first argument.
 * @...: Rest of arguments, NULL to mark the end.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Call a method of the corresponding service, and returns an IBusPendingCall used to receive a reply to the message.
 * This function calls ibus_connection_send_with_reply() to do the actual sending.
 *
 * @see_also: ibus_connection_send_with_reply(),
 * @see_also: ibus_proxy_call(), ibus_proxy_send_with_reply(), ibus_proxy_call_with_reply_and_block().
 */
gboolean         ibus_proxy_call_with_reply (IBusProxy          *proxy,
                                             const gchar        *method,
                                             IBusPendingCall   **pending,
                                             gint                timeout_milliseconds,
                                             IBusError         **error,
                                             GType              first_arg_type,
                                             ...);

/**
 * ibus_proxy_call_with_reply_and_block:
 * @proxy: An IBusProxy.
 * @method: The method to be called.
 * @timeout_milliseconds: Time out in milliseconds.
 * @error: Returned error is stored here; NULL to ignore error.
 * @first_arg_type: Type of first argument.
 * @...: Rest of arguments, NULL to mark the end.
 * @returns: An IBusMessage that is the reply or NULL with an error code if the function fails.
 *
 * Call a method of the corresponding service and blocks a certain time period while waiting for
 * an IBusMessage as reply.
 * If the IBusMessage is not NULL, it calls ibus_connection_send_with_reply_and_block() to do the
 * actual sending.
 *
 * @see_also: ibus_connection_send_with_reply_and_block(),
 * @see_also: ibus_proxy_call(), ibus_proxy_send_with_reply(), ibus_proxy_call_with_reply_and_block().
 */
IBusMessage     *ibus_proxy_call_with_reply_and_block
                                            (IBusProxy          *proxy,
                                             const gchar        *method,
                                             gint                timeout_milliseconds,
                                             IBusError         **error,
                                             GType               first_arg_type,
                                            ...);

/**
 * ibus_proxy_send_with_reply:
 * @proxy: An IBusProxy.
 * @message: The IBusMessage to be sent.
 * @pending: Return location of a IBusPendingCall object, or NULL if connection is disconnected.
 * @timeout_milliseconds: Time out in milliseconds.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Send an IBusMessage to the corresponding service and returns
 * an IBusPendingCall used to receive a reply to the message.
 * This function calls ibus_connection_send_with_reply() to do the actual sending.
 *
 * @see_also: ibus_connection_send_with_reply(),
 * @see_also: ibus_proxy_send(), ibus_proxy_call_with_reply(), ibus_proxy_send_with_reply_and_block().
 */
gboolean         ibus_proxy_send_with_reply (IBusProxy          *proxy,
                                             IBusMessage        *message,
                                             IBusPendingCall   **pending,
                                             gint                timeout_milliseconds);

/**
 * ibus_proxy_send_with_reply_and_block:
 * @proxy: An IBusProxy.
 * @message: The IBusMessage to be sent.
 * @returns: An IBusMessage that is the reply or NULL with an error code if the function fails.
 *
 * Send an IBusMessage to the corresponding service and blocks a certain time period while waiting for
 * an IBusMessage as reply.
 * If the IBusMessage is not NULL, it calls ibus_connection_send_with_reply_and_block() to do the
 * actual sending.
 *
 * @see_also: ibus_connection_send_with_reply_and_block(),
 * @see_also: ibus_proxy_send(), ibus_proxy_send_with_reply(), ibus_proxy_call_with_reply_and_block().
 */
IBusMessage     *ibus_proxy_send_with_reply_and_block
                                            (IBusProxy          *proxy,
                                             IBusMessage        *message);

/**
 * ibus_proxy_handle_signal:
 * @proxy: An IBusProxy.
 * @message: The IBusMessage to be sent.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Handle a signal by emitting IBusProxy::ibus-signal.
 *
 * If signal name is <constant>NameOwnerChanged</constant>
 * and the service name is identical to the old name, then
 * @proxy will be destroyed by ibus_object_destroy () and FALSE is returned.
 * Otherwise TRUE is returned.
 *
 * Note that if the path of of message is not identical to the IBusProxy:path
 * this function will not emit IBusProxy::ibus-signal.
 *
 */
gboolean         ibus_proxy_handle_signal   (IBusProxy          *proxy,
                                             IBusMessage        *message);

/**
 * ibus_proxy_get_name:
 * @proxy: An IBusProxy.
 * @returns: The service name of the proxy object.
 *
 * Get the service name of a proxy object.
 */
const gchar     *ibus_proxy_get_name        (IBusProxy          *proxy);

/**
 * ibus_proxy_get_unique_name:
 * @proxy: An IBusProxy.
 * @returns: The service name of the proxy object.
 *
 * Get the unique name of the proxy object.
 */
const gchar     *ibus_proxy_get_unique_name (IBusProxy          *proxy);

/**
 * ibus_proxy_get_path:
 * @proxy: An IBusProxy.
 * @returns: The path of proxy object.
 *
 * Get the path of a proxy object.
 */
const gchar     *ibus_proxy_get_path        (IBusProxy          *proxy);

/**
 * ibus_proxy_get_interface:
 * @proxy: An IBusProxy.
 * @returns: The service name of the proxy object.
 *
 * Get interface of a proxy object.
 */
const gchar     *ibus_proxy_get_interface   (IBusProxy          *proxy);

/**
 * ibus_proxy_get_connection:
 * @proxy: An IBusProxy.
 * @returns: The connection of the proxy object.
 *
 * Get the connection of a proxy object.
 */
IBusConnection  *ibus_proxy_get_connection  (IBusProxy          *proxy);

G_END_DECLS
#endif

