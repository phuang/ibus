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
#include "ibusinternal.h"

static gboolean
_watch_event_cb (GIOChannel *channel, GIOCondition condition, DBusWatch *watch)
{
    guint flags = 0;

    if (condition & G_IO_IN)
        flags |= DBUS_WATCH_READABLE;
    if (condition & G_IO_OUT)
        flags |= DBUS_WATCH_WRITABLE;
    if (condition & G_IO_ERR)
        flags |= DBUS_WATCH_ERROR;
    if (condition & G_IO_HUP)
        flags |= DBUS_WATCH_HANGUP;

    if (!dbus_watch_handle (watch, flags))
        g_warning ("Out of memory!");

    return TRUE;
}

static gboolean
_add_watch_cb (DBusWatch *watch, gpointer user_data)
{
    guint flags;
    GIOCondition condition;
    GIOChannel *channel;
    guint source_id;

    if (!dbus_watch_get_enabled (watch))
        return TRUE;

    g_assert (dbus_watch_get_data (watch) == NULL);

    flags = dbus_watch_get_flags (watch);

    condition = G_IO_ERR | G_IO_HUP;
    if (flags & DBUS_WATCH_READABLE)
        condition |= G_IO_IN;
    if (flags & DBUS_WATCH_WRITABLE)
        condition |= G_IO_OUT;

    channel = g_io_channel_unix_new (dbus_watch_get_unix_fd (watch));

    source_id = g_io_add_watch (channel, condition,
                (GIOFunc) _watch_event_cb, watch);

    dbus_watch_set_data (watch, (void *) source_id, NULL);

    g_io_channel_unref (channel);

    return TRUE;
}

static void
_remove_watch_cb (DBusWatch *watch, gpointer user_data)
{
    guint source_id;
    source_id = (guint) dbus_watch_get_data (watch);
    g_return_if_fail (source_id != (guint) NULL);

    g_source_remove (source_id);

    dbus_watch_set_data (watch, NULL, NULL);
}

static void
_watch_toggled_cb (DBusWatch *watch, gpointer user_data)
{
    if (dbus_watch_get_enabled (watch))
        _add_watch_cb (watch, user_data);
    else
        _remove_watch_cb (watch, user_data);
}

static gboolean
_timeout_event_cb (DBusTimeout *timeout)
{
    if (!dbus_timeout_handle (timeout))
        g_warning ("Out of memory!");
    return TRUE;
}


static gboolean
_add_timeout_cb (DBusTimeout *timeout, gpointer user_data)
{
    guint source_id;

    if (!dbus_timeout_get_enabled (timeout))
        return TRUE;

    g_assert (dbus_timeout_get_data (timeout) == NULL);

    source_id = g_timeout_add (dbus_timeout_get_interval (timeout),
                                (GSourceFunc)_timeout_event_cb, timeout);

    dbus_timeout_set_data (timeout, (void *)source_id, NULL);
    return TRUE;
}

static void
_remove_timeout_cb (DBusTimeout *timeout, gpointer user_data)
{
    guint source_id;
    source_id = (guint) dbus_timeout_get_data (timeout);
    g_return_if_fail (source_id != (guint) NULL);

    g_source_remove (source_id);
    dbus_timeout_set_data (timeout, NULL, NULL);
}

static void
_timeout_toggled_cb (DBusTimeout *timeout, gpointer user_data)
{
    if (dbus_timeout_get_enabled (timeout))
        _add_timeout_cb (timeout, user_data);
    else
        _remove_timeout_cb (timeout, user_data);
}



void
dbus_setup_server (DBusServer     *server)
{
    gboolean result;
    
    result = dbus_server_set_watch_functions (server,
                    (DBusAddWatchFunction) _add_watch_cb,
                    (DBusRemoveWatchFunction) _remove_watch_cb,
                    (DBusWatchToggledFunction) _watch_toggled_cb,
                    NULL, NULL);
    g_warn_if_fail (result);

    result = dbus_server_set_timeout_functions (server,
                    (DBusAddTimeoutFunction) _add_timeout_cb,
                    (DBusRemoveTimeoutFunction) _remove_timeout_cb,
                    (DBusTimeoutToggledFunction) _timeout_toggled_cb,
                    NULL, NULL);
    g_warn_if_fail (result);
}

void
dbus_setup_connection (DBusConnection *connection)
{
    gboolean result;
    
    result = dbus_connection_set_watch_functions (connection,
                    (DBusAddWatchFunction) _add_watch_cb,
                    (DBusRemoveWatchFunction) _remove_watch_cb,
                    (DBusWatchToggledFunction) _watch_toggled_cb,
                    NULL, NULL);
    g_warn_if_fail (result);

    result = dbus_connection_set_timeout_functions (connection,
                    (DBusAddTimeoutFunction) _add_timeout_cb,
                    (DBusRemoveTimeoutFunction) _remove_timeout_cb,
                    (DBusTimeoutToggledFunction) _timeout_toggled_cb,
                    NULL, NULL);
    g_warn_if_fail (result);
}

