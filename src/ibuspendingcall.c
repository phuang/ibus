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
#include "ibuspendingcall.h"

IBusPendingCall*
ibus_pending_call_ref (IBusPendingCall *pending)
{
    return dbus_pending_call_ref (pending);
}

void
ibus_pending_call_unref (IBusPendingCall *pending)
{
    dbus_pending_call_unref (pending);
}

gboolean
ibus_pending_call_set_notify (IBusPendingCall                *pending,
                              IBusPendingCallNotifyFunction   function,
                              gpointer                        user_data,
                              GDestroyNotify                  free_user_data)
{
    return dbus_pending_call_set_notify (pending, function, user_data, free_user_data);
}

void
ibus_pending_call_cancel (IBusPendingCall *pending)
{
    dbus_pending_call_cancel (pending);
}

gboolean
ibus_pending_call_get_completed (IBusPendingCall *pending)
{
    return dbus_pending_call_get_completed (pending);
}

IBusMessage*
ibus_pending_call_steal_reply (IBusPendingCall *pending)
{
    return dbus_pending_call_steal_reply (pending);
}

void
ibus_pending_call_block (IBusPendingCall *pending)
{
    return dbus_pending_call_block (pending);
}

void
ibus_pending_call_wait (IBusPendingCall *pending)
{
    g_assert (pending);

    while (!ibus_pending_call_get_completed (pending)) {
        g_main_context_iteration (NULL, TRUE);
    }
}

gboolean
ibus_pending_call_allocate_data_slot (gint *slot_p)
{
    return dbus_pending_call_allocate_data_slot (slot_p);
}

void
ibus_pending_call_free_data_slot (gint *slot_p)
{
    dbus_pending_call_free_data_slot (slot_p);
}

gboolean
ibus_pending_call_set_data (IBusPendingCall *pending,
                            gint             slot,
                            gpointer         data,
                            GDestroyNotify   free_data_func)
{
    return dbus_pending_call_set_data (pending, slot, data, free_data_func);
}

gpointer
ibus_pending_call_get_data (IBusPendingCall *pending,
                            gint             slot)
{
    return dbus_pending_call_get_data (pending, slot);
}
