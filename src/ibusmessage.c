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
#include "ibusmessage.h"
IBusMessage     *ibus_message_new               (gint                message_type);
IBusMessage     *ibus_message_new_method_call   (const gchar        *destination,
                                                 const gchar        *path,
                                                 const gchar        *interface,
                                                 const gchar        *method);
IBusMessage     *ibus_message_new_method_return (IBusMessage        *reply_to);
IBusMessage     *ibus_message_new_error         (IBusMessage        *reply_to,
                                                 const gchar        *error_name,
                                                 const gchar        *error_message);
IBusMessage     *ibus_message_new_signal        (const gchar        *path,
                                                 const gchar        *interface,
                                                 const gchar        *method);
gboolean         ibus_message_append_args       (IBusMesssage       *message,
                                                 GType               first_arg_type,
                                                 ...);
gboolean         ibus_message_append_args_valist(IBusMessage        *message,
                                                 GType               first_arg_type,
                                                 va_list             va_args);
gboolean         ibus_message_get_args          (IBusMesssage       *message,
                                                 IBusError          **error,
                                                 GType               first_arg_type,
                                                 ...);
gboolean         ibus_message_get_args_valist   (IBusMessage        *message,
                                                 IBusError          **error,
                                                 GType               first_arg_type,
                                                 va_list             va_args);
void             ibus_message_iter_init_append  (IBusMessage        *message,
                                                 IBusMessageIter    *iter);
gboolean         ibus_message_iter_append       (IBusMessageIter    *iter,
                                                 GType               type,
                                                 gconstpointer       value);
gboolean         ibus_message_iter_init         (IBusMessage        *message,
                                                 IBusMessageIter    *iter);
gboolean         ibus_message_iter_get          (IBusMessageIter    *iter,
                                                 GType               type,
                                                 gpointer            value);
gboolean         ibus_message_iter_next         (IBusMessageIter    *iter);
gboolean         ibus_message_iter_has_next     (IBusMessageIter    *iter);
gboolean         ibus_message_iter_open_container
                                                (IBusMessageIter    *iter,
                                                 GType               type,
                                                 const gchar        *contained_signature,
                                                 IBusMessageIter    *sub);
void             ibus_message_iter_recurse      (IBusMessageIter    *iter,
                                                 IBusMessageIter    *sub);
GType            ibus_message_iter_get_arg_type (IBusMessageIter    *iter);

