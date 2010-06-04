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
#include "ibuserror.h"

IBusError *
ibus_error_new (void)
{
    IBusError *error;

    error = g_slice_new0 (IBusError);
    dbus_error_init (error);

    return error;
}

IBusError *
ibus_error_new_from_text (const gchar *name,
                          const gchar *message)
{
    IBusError *error = ibus_error_new ();

    dbus_set_error (error, name, "%s", message);

    return error;
}

IBusError *
ibus_error_new_from_printf (const gchar *name,
                            const gchar *format_message,
                             ...)
{
    IBusError *error;
    gchar *message;
    va_list va_args;

    va_start (va_args, format_message);
    message = g_strdup_vprintf (format_message, va_args);

    error = ibus_error_new_from_text (name, message);
    g_free (message);

    return error;
}

IBusError *
ibus_error_new_from_message (DBusMessage *message)
{
    g_assert (message != NULL);

    IBusError *error;

    if (dbus_message_get_type (message) != DBUS_MESSAGE_TYPE_ERROR)
        return NULL;

    error = ibus_error_new ();

    if (dbus_set_error_from_message (error, message))
        return error;

    dbus_error_free (error);
    return NULL;
}

void
ibus_error_free (IBusError *error)
{
    dbus_error_free (error);
    g_slice_free (IBusError, error);
}

