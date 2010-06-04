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
 * SECTION: ibuserror
 * @short_description: Error message output.
 * @stability: Stable
 *
 * An IBusError is actually a #DBusError.
 * Functions listed here are convenient wrapper for IBusError new and free.
 */
#ifndef __IBUS_ERROR_H_
#define __IBUS_ERROR_H_

#include <glib.h>
#include "ibusdbus.h"

G_BEGIN_DECLS

/**
 * IBusError:
 *
 * A data type representing an IBusError.
 * An IBusError is actually a #DBusError.
 *
 * @see_also: #DBusError for detail structure definition.
 */
typedef DBusError IBusError;

/**
 * ibus_error_new:
 * @returns: A newly allocated IBusError.
 *
 * New an empty IBusError.
 */
IBusError       *ibus_error_new             (void);

/**
 * ibus_error_new_from_text:
 * @name: The error name.
 * @message: Detailed error message.
 * @returns: A newly allocated IBusError.
 *
 * New an IBusError from error name and message.
 */
IBusError       *ibus_error_new_from_text   (const gchar    *name,
                                             const gchar    *message);

/**
 * ibus_error_new_from_printf:
 * @name: The error name.
 * @format_message: printf() formatted error message.
 * @...: Formatting parameters.
 * @returns: A newly allocated IBusError.
 *
 * New an IBusError from error name and a printf-formatted message.
 */
IBusError       *ibus_error_new_from_printf (const gchar    *name,
                                             const gchar    *format_message,
                                             ...);

/**
 * ibus_error_new_from_message:
 * @message: A DBusMessage
 * @returns: A newly allocated IBusError.
 *
 * New an IBusError from a #IBusMessage.
 */
IBusError       *ibus_error_new_from_message
                                            (DBusMessage    *message);

/**
 * ibus_error_free:
 * @error: An IBusError
 *
 * Free an IBusError.
 */
void             ibus_error_free            (IBusError      *error);

G_END_DECLS
#endif
