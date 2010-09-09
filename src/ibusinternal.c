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
#include <dbus/dbus.h>
#include "ibusinternal.h"

/**
 * IBusMessageQueue:
 * @source: The parent GSource.
 * @connection: The connection to dispatch.
 *
 * A GSource subclass for dispatching DBusConnection messages.
 * We need this on top of the IO handlers, because sometimes
 * there are messages to dispatch queued up but no IO pending.
 */
typedef struct
{
    GSource source;
    DBusConnection *connection;
} IBusMessageQueue;

static gboolean message_queue_prepare  (GSource     *source,
                                          gint        *timeout);
static gboolean message_queue_check    (GSource     *source);
static gboolean message_queue_dispatch (GSource     *source,
                                        GSourceFunc  callback,
                                        gpointer     user_data);

static const GSourceFuncs message_queue_funcs = {
    message_queue_prepare,
    message_queue_check,
    message_queue_dispatch,
    NULL
};

static gboolean
message_queue_prepare (GSource *source,
                         gint    *timeout)
{
    DBusConnection *connection = ((IBusMessageQueue *)source)->connection;

    *timeout = -1;

    return (dbus_connection_get_dispatch_status (connection) == DBUS_DISPATCH_DATA_REMAINS);
}

static gboolean
message_queue_check (GSource *source)
{
    return FALSE;
}

static gboolean
message_queue_dispatch (GSource     *source,
                        GSourceFunc  callback,
                        gpointer     user_data)
{
    DBusConnection *connection = ((IBusMessageQueue *)source)->connection;

    dbus_connection_ref (connection);

    /* Only dispatch once - we don't want to starve other GSource */
    dbus_connection_dispatch (connection);

    dbus_connection_unref (connection);

    return TRUE;
}

/**
 * ConnectionSetup:
 * @context: The main context.
 * @ios: All IOHandler.
 * @timeouts: All TimeoutHandler.
 * @connection: NULL if this is really for a server not a connection.
 * @message_queue_source: IBusMessageQueue.
 *
 */
GSource *message_queue_source; /**< IBusMessageQueue */
typedef struct
{
    GMainContext *context;      /**< the main context */
    GSList *ios;                /**< all IOHandler */
    GSList *timeouts;           /**< all TimeoutHandler */
    DBusConnection *connection; /**< NULL if this is really for a server not a connection */
    GSource *message_queue_source; /**< IBusMessageQueue */
} ConnectionSetup;


typedef struct
{
    ConnectionSetup *cs;
    GSource *source;
    DBusWatch *watch;
} IOHandler;

typedef struct
{
    ConnectionSetup *cs;
    GSource *source;
    DBusTimeout *timeout;
} TimeoutHandler;

dbus_int32_t _dbus_gmain_connection_slot = -1;
static dbus_int32_t server_slot = -1;

static ConnectionSetup*
connection_setup_new (GMainContext   *context,
                      DBusConnection *connection)
{
    ConnectionSetup *cs;

    cs = g_slice_new0 (ConnectionSetup);

    g_assert (context != NULL);

    cs->context = context;
    g_main_context_ref (cs->context);

    if (connection) {
        cs->connection = connection;

        cs->message_queue_source = g_source_new ((GSourceFuncs *) &message_queue_funcs,
                                                 sizeof (IBusMessageQueue));
        ((IBusMessageQueue*)cs->message_queue_source)->connection = connection;
        g_source_attach (cs->message_queue_source, cs->context);
    }
    return cs;
}

static void
io_handler_source_finalized (gpointer data)
{
    IOHandler *handler;

    handler = data;

    if (handler->watch)
      dbus_watch_set_data (handler->watch, NULL, NULL);

    g_slice_free (IOHandler, handler);
}

static void
io_handler_destroy_source (void *data)
{
    IOHandler *handler;

    handler = data;

    if (handler->source) {
        GSource *source = handler->source;
        handler->source = NULL;
        handler->cs->ios = g_slist_remove (handler->cs->ios, handler);
        g_source_destroy (source);
        g_source_unref (source);
    }
}

static void
io_handler_watch_freed (void *data)
{
    IOHandler *handler;

    handler = data;

    handler->watch = NULL;

    io_handler_destroy_source (handler);
}

static gboolean
io_handler_dispatch (GIOChannel   *source,
                     GIOCondition  condition,
                     gpointer      data)
{
    IOHandler *handler;
    guint dbus_condition = 0;
    DBusConnection *connection;

    handler = data;

    connection = handler->cs->connection;

    if (connection)
        dbus_connection_ref (connection);

    if (condition & G_IO_IN)
        dbus_condition |= DBUS_WATCH_READABLE;
    if (condition & G_IO_OUT)
        dbus_condition |= DBUS_WATCH_WRITABLE;
    if (condition & G_IO_ERR)
        dbus_condition |= DBUS_WATCH_ERROR;
    if (condition & G_IO_HUP)
        dbus_condition |= DBUS_WATCH_HANGUP;

    /* Note that we don't touch the handler after this, because
     * dbus may have disabled the watch and thus killed the
     * handler.
     */
    dbus_watch_handle (handler->watch, dbus_condition);
    handler = NULL;

    if (connection)
        dbus_connection_unref (connection);

    return TRUE;
}

static void
connection_setup_add_watch (ConnectionSetup *cs,
                            DBusWatch       *watch)
{
    guint flags;
    GIOCondition condition;
    GIOChannel *channel;
    IOHandler *handler;

    if (!dbus_watch_get_enabled (watch))
        return;

    g_assert (dbus_watch_get_data (watch) == NULL);

    flags = dbus_watch_get_flags (watch);

    condition = G_IO_ERR | G_IO_HUP;
    if (flags & DBUS_WATCH_READABLE)
        condition |= G_IO_IN;
    if (flags & DBUS_WATCH_WRITABLE)
        condition |= G_IO_OUT;

    handler = g_slice_new (IOHandler);
    handler->cs = cs;
    handler->watch = watch;

    channel = g_io_channel_unix_new (dbus_watch_get_unix_fd (watch));

    handler->source = g_io_create_watch (channel, condition);
    g_source_set_callback (handler->source, (GSourceFunc) io_handler_dispatch, handler,
                           io_handler_source_finalized);
    g_source_attach (handler->source, cs->context);

    cs->ios = g_slist_prepend (cs->ios, handler);

    dbus_watch_set_data (watch, handler, io_handler_watch_freed);
    g_io_channel_unref (channel);
}

static void
connection_setup_remove_watch (ConnectionSetup *cs,
                               DBusWatch       *watch)
{
    IOHandler *handler;

    handler = dbus_watch_get_data (watch);

    if (handler == NULL)
        return;

    io_handler_destroy_source (handler);
}

static void
timeout_handler_source_finalized (gpointer data)
{
    TimeoutHandler *handler;

    handler = data;

    if (handler->timeout)
        dbus_timeout_set_data (handler->timeout, NULL, NULL);

    g_slice_free (TimeoutHandler, handler);
}

static void
timeout_handler_destroy_source (void *data)
{
    TimeoutHandler *handler;

    handler = data;

    if (handler->source) {
        GSource *source = handler->source;
        handler->source = NULL;
        handler->cs->timeouts = g_slist_remove (handler->cs->timeouts, handler);
        g_source_destroy (source);
        g_source_unref (source);
    }
}

static void
timeout_handler_timeout_freed (void *data)
{
    TimeoutHandler *handler;

    handler = data;

    handler->timeout = NULL;

    timeout_handler_destroy_source (handler);
}

static gboolean
timeout_handler_dispatch (gpointer      data)
{
    TimeoutHandler *handler;

    handler = data;

    dbus_timeout_handle (handler->timeout);

    return TRUE;
}

static void
connection_setup_add_timeout (ConnectionSetup *cs,
                              DBusTimeout     *timeout)
{
    TimeoutHandler *handler;

    if (!dbus_timeout_get_enabled (timeout))
        return;

    g_assert (dbus_timeout_get_data (timeout) == NULL);

    handler = g_slice_new0 (TimeoutHandler);
    handler->cs = cs;
    handler->timeout = timeout;

    handler->source = g_timeout_source_new (dbus_timeout_get_interval (timeout));
    g_source_set_callback (handler->source, timeout_handler_dispatch, handler,
                           timeout_handler_source_finalized);
    g_source_attach (handler->source, handler->cs->context);

    cs->timeouts = g_slist_prepend (cs->timeouts, handler);

    dbus_timeout_set_data (timeout, handler, timeout_handler_timeout_freed);
}

static void
connection_setup_remove_timeout (ConnectionSetup *cs,
                                 DBusTimeout       *timeout)
{
    TimeoutHandler *handler;

    handler = dbus_timeout_get_data (timeout);

    if (handler == NULL)
        return;

    timeout_handler_destroy_source (handler);
}

static void
connection_setup_free (ConnectionSetup *cs)
{
    while (cs->ios)
        io_handler_destroy_source (cs->ios->data);

    while (cs->timeouts)
        timeout_handler_destroy_source (cs->timeouts->data);

    if (cs->message_queue_source) {
        GSource *source;

        source = cs->message_queue_source;
        cs->message_queue_source = NULL;

        g_source_destroy (source);
        g_source_unref (source);
    }

    g_main_context_unref (cs->context);
    g_slice_free (ConnectionSetup, cs);
}

static dbus_bool_t
add_watch (DBusWatch *watch,
       gpointer   data)
{
    ConnectionSetup *cs;

    cs = data;

    connection_setup_add_watch (cs, watch);

    return TRUE;
}

static void
remove_watch (DBusWatch *watch,
          gpointer   data)
{
    ConnectionSetup *cs;

    cs = data;

    connection_setup_remove_watch (cs, watch);
}

static void
watch_toggled (DBusWatch *watch,
                 void      *data)
{
    /* Because we just exit on OOM, enable/disable is
     * no different from add/remove
     */
    if (dbus_watch_get_enabled (watch))
        add_watch (watch, data);
    else
        remove_watch (watch, data);
}

static dbus_bool_t
add_timeout (DBusTimeout *timeout,
         void        *data)
{
    ConnectionSetup *cs;

    cs = data;

    if (!dbus_timeout_get_enabled (timeout))
        return TRUE;

    connection_setup_add_timeout (cs, timeout);

    return TRUE;
}

static void
remove_timeout (DBusTimeout *timeout,
        void        *data)
{
    ConnectionSetup *cs;

    cs = data;

    connection_setup_remove_timeout (cs, timeout);
}

static void
timeout_toggled (DBusTimeout *timeout,
                   void        *data)
{
    /* Because we just exit on OOM, enable/disable is
     * no different from add/remove
     */
    if (dbus_timeout_get_enabled (timeout))
        add_timeout (timeout, data);
    else
        remove_timeout (timeout, data);
}

static void
wakeup_main (void *data)
{
    ConnectionSetup *cs = data;

    g_main_context_wakeup (cs->context);
}


/* Move to a new context */
static ConnectionSetup*
connection_setup_new_from_old (GMainContext    *context,
                               ConnectionSetup *old)
{
    GSList *tmp;
    ConnectionSetup *cs;

    g_assert (old->context != context);

    cs = connection_setup_new (context, old->connection);

    tmp = old->ios;
    while (tmp != NULL) {
        IOHandler *handler = tmp->data;

        connection_setup_add_watch (cs, handler->watch);

        tmp = tmp->next;
    }

    tmp = old->timeouts;
    while (tmp != NULL) {
        TimeoutHandler *handler = tmp->data;

        connection_setup_add_timeout (cs, handler->timeout);

        tmp = tmp->next;
    }

    return cs;
}

/** @} */ /* End of GLib bindings internals */

/* @addtogroup IBusLib
 * @{
 */

/**
 * dbus_connection_setup_with_g_main:
 * @connection: the connection
 * @context: the #GMainContext or #NULL for default context
 *
 * Sets the watch and timeout functions of a #DBusConnection
 * to integrate the connection with the GLib main loop.
 * Pass in #NULL for the #GMainContext unless you're
 * doing something specialized.
 *
 * If called twice for the same context, does nothing the second
 * time. If called once with context A and once with context B,
 * context B replaces context A as the context monitoring the
 * connection.
 */
void
dbus_connection_setup (DBusConnection *connection,
                       GMainContext   *context)
{
    ConnectionSetup *old_setup;
    ConnectionSetup *cs;

    do {
        /* FIXME we never free the slot, so its refcount just keeps growing,
         * which is kind of broken.
         */
        dbus_connection_allocate_data_slot (&_dbus_gmain_connection_slot);
        if (_dbus_gmain_connection_slot < 0)
            break;

        if (context == NULL)
            context = g_main_context_default ();

        cs = NULL;

        old_setup = dbus_connection_get_data (connection, _dbus_gmain_connection_slot);
        if (old_setup != NULL) {
            if (old_setup->context == context)
                return; /* nothing to do */

            cs = connection_setup_new_from_old (context, old_setup);

            /* Nuke the old setup */
            dbus_connection_set_data (connection, _dbus_gmain_connection_slot, NULL, NULL);
            old_setup = NULL;
        }

        if (cs == NULL)
            cs = connection_setup_new (context, connection);

        if (!dbus_connection_set_data (connection, _dbus_gmain_connection_slot, cs,
                                       (DBusFreeFunction)connection_setup_free))
            break;

        if (!dbus_connection_set_watch_functions (connection,
                                                  add_watch,
                                                  remove_watch,
                                                  watch_toggled,
                                                  cs, NULL))
            break;

        if (!dbus_connection_set_timeout_functions (connection,
                                                    add_timeout,
                                                    remove_timeout,
                                                    timeout_toggled,
                                                    cs, NULL))
            break;

        dbus_connection_set_wakeup_main_function (connection,
                            wakeup_main,
                            cs, NULL);

        return;
    } while (0);
    
    g_error ("Not enough memory to set up DBusConnection for use with GLib");
}

/**
 * dbus_server_setup_with_g_main:
 * @server: the server
 * @context: the #GMainContext or #NULL for default
 *
 * Sets the watch and timeout functions of a #DBusServer
 * to integrate the server with the GLib main loop.
 * In most cases the context argument should be #NULL.
 *
 * If called twice for the same context, does nothing the second
 * time. If called once with context A and once with context B,
 * context B replaces context A as the context monitoring the
 * connection.
 */
void
dbus_server_setup (DBusServer   *server,
				   GMainContext *context)
{
    ConnectionSetup *old_setup;
    ConnectionSetup *cs;

    do {
        /* FIXME we never free the slot, so its refcount just keeps growing,
         * which is kind of broken.
         */
        dbus_server_allocate_data_slot (&server_slot);
        if (server_slot < 0)
            break;

        if (context == NULL)
            context = g_main_context_default ();

        cs = NULL;

        old_setup = dbus_server_get_data (server, server_slot);
        if (old_setup != NULL) {
            if (old_setup->context == context)
                return; /* nothing to do */

            cs = connection_setup_new_from_old (context, old_setup);

            /* Nuke the old setup */
            dbus_server_set_data (server, server_slot, NULL, NULL);
            old_setup = NULL;
        }

        if (cs == NULL)
            cs = connection_setup_new (context, NULL);

        if (!dbus_server_set_data (server, server_slot, cs,
                                   (DBusFreeFunction)connection_setup_free))
            break;

        if (!dbus_server_set_watch_functions (server,
                                              add_watch,
                                              remove_watch,
                                              watch_toggled,
                                              cs, NULL))
            break;

        if (!dbus_server_set_timeout_functions (server,
                                                add_timeout,
                                                remove_timeout,
                                                timeout_toggled,
                                                cs, NULL))
            break;

        return;
    } while (0);
    
    g_error ("Not enough memory to set up DBusServer for use with GLib");
}

