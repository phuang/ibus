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

#include "ibusproxy.h"
#include "ibusmarshalers.h"
#include "ibusinternal.h"
#include "ibusobject.h"

#define IBUS_PROXY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_PROXY, IBusProxyPrivate))

enum {
    DESTROY,
    LAST_SIGNAL,
};

static guint            proxy_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void      ibus_proxy_constructed     (GObject            *object);
static void      ibus_proxy_dispose         (GObject            *object);
static void      ibus_proxy_real_destroy    (IBusProxy          *proxy);

static void      ibus_proxy_connection_closed_cb
                                            (GDBusConnection    *connection,
                                             gboolean            remote_peer_vanished,
                                             GError             *error,
                                             IBusProxy          *proxy);

G_DEFINE_TYPE (IBusProxy, ibus_proxy, G_TYPE_DBUS_PROXY)

static void
ibus_proxy_class_init (IBusProxyClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);

    gobject_class->constructed = ibus_proxy_constructed;
    gobject_class->dispose = ibus_proxy_dispose;

    class->destroy = ibus_proxy_real_destroy;

    /* install signals */
    /**
     * IBusProxy::destroy:
     * @object: An IBusProxy.
     *
     * Destroy and free an IBusProxy
     *
     * See also:  ibus_proxy_destroy().
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
     */
    proxy_signals[DESTROY] =
        g_signal_new (I_("destroy"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusProxyClass, destroy),
            NULL, NULL,
            _ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

static void
ibus_proxy_init (IBusProxy *proxy)
{
    proxy->own = TRUE;
}

static void
ibus_proxy_constructed (GObject *object)
{
    GDBusConnection *connection;
    connection = g_dbus_proxy_get_connection ((GDBusProxy *)object);

    g_assert (connection != NULL);
    g_assert (!g_dbus_connection_is_closed (connection));

    g_signal_connect (connection, "closed",
            G_CALLBACK (ibus_proxy_connection_closed_cb), object);

    /* FIXME add match rules? */
}

/**
 * ibus_proxy_dispose:
 *
 * Override GObject's dispose function.
 */
static void
ibus_proxy_dispose (GObject *object)
{
    if (! (IBUS_PROXY_FLAGS (object) & IBUS_IN_DESTRUCTION)) {
        IBUS_PROXY_SET_FLAGS (object, IBUS_IN_DESTRUCTION);
        if (! (IBUS_PROXY_FLAGS (object) & IBUS_DESTROYED)) {
            g_signal_emit (object, proxy_signals[DESTROY], 0);
            IBUS_PROXY_SET_FLAGS (object, IBUS_DESTROYED);
        }
        IBUS_PROXY_UNSET_FLAGS (object, IBUS_IN_DESTRUCTION);
    }

    G_OBJECT_CLASS(ibus_proxy_parent_class)->dispose (object);
}

/**
 * ibus_proxy_real_destroy:
 *
 * Handle "destroy" signal which is emitted by ibus_proxy_dispose.
 */
static void
ibus_proxy_real_destroy (IBusProxy *proxy)
{
    GDBusConnection *connection = g_dbus_proxy_get_connection ((GDBusProxy *) proxy);
    g_assert (connection != NULL);
    if (!g_dbus_connection_is_closed (connection) && proxy->own) {
        g_dbus_proxy_call ((GDBusProxy *)proxy,
                           "org.freedesktop.IBus.Service.Destroy",
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1, NULL, NULL, NULL);
    }
    g_signal_handlers_disconnect_by_func (connection,
                                          (GCallback) ibus_proxy_connection_closed_cb,
                                          proxy);
}

static void
ibus_proxy_connection_closed_cb (GDBusConnection *connection,
                                 gboolean         remote_peer_vanished,
                                 GError          *error,
                                 IBusProxy       *proxy)
{
    ibus_proxy_destroy (proxy);
}

void
ibus_proxy_destroy (IBusProxy *proxy)
{
    g_assert (IBUS_IS_PROXY (proxy));

    if (! (IBUS_PROXY_FLAGS (proxy) & IBUS_IN_DESTRUCTION)) {
        g_object_run_dispose (G_OBJECT (proxy));
    }
}

