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
 * SECTION: ibusmessage
 * @short_description: A DBusMessage in IBus.
 * @stability: Stable
 *
 * An IBusMessage is essentially a DBusMessage, which representing a message received from or to
 * be sent to another application.
 *
 * Besides DBusMessage functions, An IBusMessage can be manipulated
 * with its own specific functions, which are defined in this section.
 */
#ifndef __IBUS_MESSAGE_H_
#define __IBUS_MESSAGE_H_

#include <glib.h>
#include <glib-object.h>
#include <dbus/dbus.h>
#include "ibuserror.h"

#define IBUS_TYPE_OBJECT_PATH   (ibus_type_get_object_path ())
#define IBUS_TYPE_ARRAY         (ibus_type_get_array ())
#define IBUS_TYPE_STRUCT        (ibus_type_get_struct ())
#define IBUS_TYPE_DICT_ENTRY    (ibus_type_get_dict_entry ())
#define IBUS_TYPE_VARIANT       (ibus_type_get_variant ())

G_BEGIN_DECLS

/**
 * IBusMessage:
 *
 * An opaque data structure that represents IBusMessage.
 */
typedef DBusMessage IBusMessage;

/**
 * IBusMessageIter:
 *
 * An opaque data structure that represents IBusMessageIter.
 */
typedef DBusMessageIter IBusMessageIter;

GType            ibus_type_get_object_path      (void);
GType            ibus_type_get_array            (void);
GType            ibus_type_get_struct           (void);
GType            ibus_type_get_dict_entry       (void);
GType            ibus_type_get_variant          (void);

/**
 * ibus_message_new:
 * @message_type: Type of the message.
 * @returns: A newly allocated IBusMessage according to @message_type.
 *
 * New an IBusMessage.
 * Valid D-Bus message types include:
 * <itemizedlist>
 *     <listitem>
 *         <para>#DBUS_MESSAGE_TYPE_METHOD_CALL</para>
 *     </listitem>
 *     <listitem>
 *         <para>#DBUS_MESSAGE_TYPE_METHOD_RETURN</para>
 *     </listitem>
 *     <listitem>
 *         <para>#DBUS_MESSAGE_TYPE_METHOD_ERROR</para>
 *     </listitem>
 *     <listitem>
 *         <para>#DBUS_MESSAGE_TYPE_METHOD_SIGNAL</para>
 *     </listitem>
 * </itemizedlist>
 * These are defined in dbus-protocol.h in D-Bus.
 */
IBusMessage     *ibus_message_new               (gint                message_type);

/**
 * ibus_message_ref:
 * @message An IBusMessage
 *
 * ncrements the reference count of an IBusMessage.
 */
IBusMessage     *ibus_message_ref               (IBusMessage        *message);
void             ibus_message_unref             (IBusMessage        *message);
IBusMessage     *ibus_message_new_method_call   (const gchar        *destination,
                                                 const gchar        *path,
                                                 const gchar        *interface,
                                                 const gchar        *method);
IBusMessage     *ibus_message_new_method_return (IBusMessage        *reply_to);
IBusMessage     *ibus_message_new_error         (IBusMessage        *reply_to,
                                                 const gchar        *error_name,
                                                 const gchar        *error_message);
IBusMessage     *ibus_message_new_error_printf  (IBusMessage        *reply_to,
                                                 const gchar        *error_name,
                                                 const gchar        *error_format,
                                                 ...);
IBusMessage     *ibus_message_new_signal        (const gchar        *path,
                                                 const gchar        *interface,
                                                 const gchar        *method);
gboolean         ibus_message_is_method_call    (IBusMessage        *message,
                                                 const gchar        *interface,
                                                 const gchar        *method);
gboolean         ibus_message_is_error          (IBusMessage        *message,
                                                 const gchar        *error_name);
gboolean         ibus_message_is_signal         (IBusMessage        *message,
                                                 const gchar        *interface,
                                                 const gchar        *signal_name);
gboolean         ibus_message_set_destination   (IBusMessage        *message,
                                                 const gchar        *destination);
gboolean         ibus_message_set_sender        (IBusMessage        *message,
                                                 const gchar        *sender);
gboolean         ibus_message_set_error_name    (IBusMessage        *message,
                                                 const gchar        *error_name);
gboolean         ibus_message_set_interface     (IBusMessage        *message,
                                                 const gchar        *interface);
gboolean         ibus_message_set_member        (IBusMessage        *message,
                                                 const gchar        *member);
gboolean         ibus_message_set_path          (IBusMessage        *message,
                                                 const gchar        *path);
void             ibus_message_set_no_reply      (IBusMessage        *message,
                                                 gboolean            no_reply);
gboolean         ibus_message_set_reply_serial  (IBusMessage        *message,
                                                 guint32             reply_serial);
gint             ibus_message_get_type          (IBusMessage        *message);
const gchar     *ibus_message_get_destination   (IBusMessage        *message);
const gchar     *ibus_message_get_sender        (IBusMessage        *message);
const gchar     *ibus_message_get_error_name    (IBusMessage        *message);
const gchar     *ibus_message_get_error_message (IBusMessage        *message);
const gchar     *ibus_message_get_interface     (IBusMessage        *message);
const gchar     *ibus_message_get_member        (IBusMessage        *message);
const gchar     *ibus_message_get_path          (IBusMessage        *message);
gboolean         ibus_message_get_no_reply      (IBusMessage        *message);
guint32          ibus_message_get_reply_serial  (IBusMessage        *message);
guint32          ibus_message_get_serial        (IBusMessage        *message);
gboolean         ibus_message_append_args       (IBusMessage        *message,
                                                 GType               first_arg_type,
                                                 ...);
gboolean         ibus_message_append_args_valist(IBusMessage        *message,
                                                 GType               first_arg_type,
                                                 va_list             va_args);
gboolean         ibus_message_get_args          (IBusMessage        *message,
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
gboolean         ibus_message_iter_peek         (IBusMessageIter    *iter,
                                                 GType               type,
                                                 gpointer            value);
void             ibus_message_iter_get_basic    (IBusMessageIter    *iter,
                                                 gpointer            value);
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
gboolean         ibus_message_iter_close_container
                                                (IBusMessageIter    *iter,
                                                 IBusMessageIter    *sub);
gboolean         ibus_message_iter_recurse      (IBusMessageIter    *iter,
                                                 GType               type,
                                                 IBusMessageIter    *sub);
GType            ibus_message_iter_get_arg_type (IBusMessageIter    *iter);
GType            ibus_message_iter_get_element_type
                                                (IBusMessageIter    *iter);
gchar           *ibus_message_to_string         (IBusMessage *message);

G_END_DECLS
#endif
