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
 * SECTION: ibusmessage
 * @Title: IBusMessage
 * @Short_description: A DBusMessage in IBus.
 * @Stability: Stable
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
#include "ibusdbus.h"
#include "ibuserror.h"

/**
 * IBUS_TYPE_OBJECT_PATH:
 *
 * Type of object path.
 */
#define IBUS_TYPE_OBJECT_PATH   (ibus_type_get_object_path ())

/**
 * IBUS_TYPE_ARRAY:
 *
 * Type of IBusArray.
 */
#define IBUS_TYPE_ARRAY         (ibus_type_get_array ())

/**
 * IBUS_TYPE_STRUCT:
 *
 * Type of IBusStruct.
 */
#define IBUS_TYPE_STRUCT        (ibus_type_get_struct ())

/**
 * IBUS_TYPE_DICT_ENTRY:
 *
 * Type of IBusDictEntry.
 */
#define IBUS_TYPE_DICT_ENTRY    (ibus_type_get_dict_entry ())

/**
 * IBUS_TYPE_VARIANT:
 *
 * Type of IBusVariant.
 */
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

/**
 * ibus_type_get_object_path:
 * @returns: Type of object path.
 *
 * Gets the type of object path.
 */
GType            ibus_type_get_object_path      (void);

/**
 * ibus_type_get_array:
 * @returns: Type of IBusArray.
 *
 * Gets the type of IBusArray.
 */
GType            ibus_type_get_array            (void);

/**
 * ibus_type_get_struct:
 * @returns: Type of IBusStruct.
 *
 * Gets the type of IBusStruct.
 */
GType            ibus_type_get_struct           (void);

/**
 * ibus_type_get_dict_entry:
 * @returns: Type of IBusDictEntry.
 *
 * Gets the type of IBusDictEntry.
 */
GType            ibus_type_get_dict_entry       (void);

/**
 * ibus_type_get_variant:
 * @returns: Type of IBusVariant.
 *
 * Gets the type of IBusVariant.
 */
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
 *         <para>#DBUS_MESSAGE_TYPE_ERROR</para>
 *     </listitem>
 *     <listitem>
 *         <para>#DBUS_MESSAGE_TYPE_SIGNAL</para>
 *     </listitem>
 * </itemizedlist>
 * These are defined in dbus-protocol.h in D-Bus.
 */
IBusMessage     *ibus_message_new               (gint                message_type);

/**
 * ibus_message_ref:
 * @message: An IBusMessage.
 * @returns: The IBusMessage.
 *
 * Increments the reference count of an IBusMessage.
 */
IBusMessage     *ibus_message_ref               (IBusMessage        *message);

/**
 * ibus_message_unref:
 * @message: An IBusMessage.
 *
 * Decrements the reference count of a DBusMessage, freeing the message if the count reaches 0.
 */
void             ibus_message_unref             (IBusMessage        *message);

/**
 * ibus_message_new_method_call:
 * @destination: Where this message to be sent to or %NULL for no destination.
 * @path: Object path the message should be sent to.
 * @interface: 	Interface to invoke method on, or %NULL.
 * @method: The method to be invoked.
 * @returns: A newly allocate IBusMessage; or %NULL if memory cannot be allocated.
 *
 * Constructs a new message to invoke a method on a remote object.
 *
 * The destination may be %NULL in which case no destination is set;
 * this is appropriate when using IBus/D-Bus in a peer-to-peer context (no message bus).
 * The interface may be %NULL, which means that if multiple methods with the given name
 * exist it is undefined which one will be invoked.
 *
 * The path and method names may not be %NULL.
 *
 * Destination, path, interface, and method name can't contain any invalid characters
 * (see the D-Bus specification).
 */
IBusMessage     *ibus_message_new_method_call   (const gchar        *destination,
                                                 const gchar        *path,
                                                 const gchar        *interface,
                                                 const gchar        *method);

/**
 * ibus_message_new_method_return:
 * @reply_to: The IBusMessage being replied to.
 * @returns: A newly allocate IBusMessage; or %NULL if memory cannot be allocated.
 *
 * Constructs a message that is a reply to a method call.
 */
IBusMessage     *ibus_message_new_method_return (IBusMessage        *reply_to);

/**
 * ibus_message_new_error:
 * @reply_to: The IBusMessage being replied to.
 * @error_name: Name of the error.
 * @error_message: Detailed error message string (or %NULL for none, but please give a message).
 * @returns: A newly allocate IBusMessage with the error information; or %NULL if memory cannot be allocated.
 *
 * Creates a new message that is an error reply to another message.
 * Error replies are most common in response to method calls, but can be returned in reply to any message.
 * The error name must be a valid error name according to the syntax given in the D-Bus specification.
 * If you don't want to make up an error name just use %DBUS_ERROR_FAILED.
 *
 * Use ibus_message_unref() to free the produced IBusMessage.
 */
IBusMessage     *ibus_message_new_error         (IBusMessage        *reply_to,
                                                 const gchar        *error_name,
                                                 const gchar        *error_message);

/**
 * ibus_message_new_error_printf:
 * @reply_to: The IBusMessage being replied to.
 * @error_name: Name of the error.
 * @error_format: Error format string as in printf() format.
 * @...: Format arguments, as in printf().
 * @returns: A newly allocate IBusMessage with the error information; or %NULL if memory cannot be allocated.
 *
 * Creates a new message that is an error reply to another message.
 * Error replies are most common in response to method calls, but can be returned in reply to any message.
 * The error name must be a valid error name according to the syntax given in the D-Bus specification.
 * If you don't want to make up an error name just use %DBUS_ERROR_FAILED.
 */
IBusMessage     *ibus_message_new_error_printf  (IBusMessage        *reply_to,
                                                 const gchar        *error_name,
                                                 const gchar        *error_format,
                                                 ...);

/**
 * ibus_message_new_signal:
 * @path: Object path the message should be sent to.
 * @interface: 	Interface to invoke method on, or %NULL.
 * @method: The method to invoke.
 * @returns: A newly allocate IBusMessage with the error information; or %NULL if memory cannot be allocated.
 *
 * Constructs a new message representing a signal emission.
 * Returns NULL if memory can't be allocated for the message.
 * A signal is identified by its originating object path, interface, and the name of the signal.
 * Path, interface, and signal name must all be valid (the D-Bus specification defines the syntax of these fields).
 */
IBusMessage     *ibus_message_new_signal        (const gchar        *path,
                                                 const gchar        *interface,
                                                 const gchar        *method);

/**
 * ibus_message_is_method_call:
 * @message: An IBusMessage.
 * @interface: 	The interface to check. Cannot be %NULL.
 * @method: The method to check. Cannot be %NULL.
 * @returns: %TRUE if @message is DBUS_MESSAGE_TYPE_METHOD_CALL and the invoked method is matched with @method;
 *           %FALSE otherwise.
 *
 * Checks whether the message is a method call with the given interface and member fields.
 *
 * If the message is not DBUS_MESSAGE_TYPE_METHOD_CALL,
 * or has a different interface or member field, returns FALSE.
 * If the interface field is missing, then it will be assumed equal to the provided interface.
 * The D-Bus protocol allows method callers to leave out the interface name.
 */
gboolean         ibus_message_is_method_call    (IBusMessage        *message,
                                                 const gchar        *interface,
                                                 const gchar        *method);
/**
 * ibus_message_is_error:
 * @message: An IBusMessage.
 * @error_name: Name of the error to check.
 * @returns: %TRUE if @message is DBUS_MESSAGE_TYPE_ERROR and the error name is matched with @error_name;
 *           %FALSE otherwise.
 *
 * Checks whether the message is an error reply with the given error name.
 * If the message is not DBUS_MESSAGE_TYPE_ERROR, or has a different name, returns FALSE.
 */
gboolean         ibus_message_is_error          (IBusMessage        *message,
                                                 const gchar        *error_name);

/**
 * ibus_message_is_signal:
 * @message: An IBusMessage.
 * @interface: The interface to checked. Cannot be %NULL.
 * @signal_name: The signal name to check.
 * @returns: %TRUE if @message is %DBUS_MESSAGE_SIGNAL and the signal name is matched with @signal_name;
 *           %FALSE otherwise.
 *
 * Checks whether the message is a signal with the given interface and member fields.
 * If the message is not %DBUS_MESSAGE_TYPE_SIGNAL, or has a different interface or member field, returns %FALSE.
 */
gboolean         ibus_message_is_signal         (IBusMessage        *message,
                                                 const gchar        *interface,
                                                 const gchar        *signal_name);

/**
 * ibus_message_set_destination:
 * @message: An IBusMessage.
 * @destination: Destination to set; or %NULL to unset.
 * @returns: %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Sets the message's destination.
 *
 * The destination is the name of another connection on the bus
 * and may be either the unique name assigned by the bus to each connection,
 * or a well-known name specified in advance.
 *
 * The destination name must contain only valid characters as defined in the D-Bus specification.
 */
gboolean         ibus_message_set_destination   (IBusMessage        *message,
                                                 const gchar        *destination);
/**
 * ibus_message_set_sender:
 * @message: An IBusMessage.
 * @sender: Sender to set; or %NULL to unset.
 * @returns: %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Sets the message sender.
 *
 * The sender must be a valid bus name as defined in the D-Bus specification.
 *
 * Usually you don't want to call this. The message bus daemon will call it to set the origin of each message.
 * If you aren't implementing a message bus daemon you shouldn't need to set the sender.
 */
gboolean         ibus_message_set_sender        (IBusMessage        *message,
                                                 const gchar        *sender);

/**
 * ibus_message_set_error_name:
 * @message: An IBusMessage.
 * @error_name: Error name to set; or %NULL to unset.
 * @returns: %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Sets the name of the error (%DBUS_MESSAGE_TYPE_ERROR).
 *
 * The name is fully-qualified (namespaced).
 *
 * The error name must contain only valid characters as defined in the D-Bus specification.
 */
gboolean         ibus_message_set_error_name    (IBusMessage        *message,
                                                 const gchar        *error_name);

/**
 * ibus_message_set_interface:
 * @message: An IBusMessage.
 * @interface: Interface to set; or %NULL to unset.
 * @returns: %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Sets the interface this message is being sent to
 * (for %DBUS_MESSAGE_TYPE_METHOD_CALL) or the interface
 * a signal is being emitted from (for %DBUS_MESSAGE_TYPE_SIGNAL).
 *
 * The interface name must contain only valid characters as defined in the D-Bus specification.
 */
gboolean         ibus_message_set_interface     (IBusMessage        *message,
                                                 const gchar        *interface);

/**
 * ibus_message_set_member:
 * @message: An IBusMessage.
 * @member: Member to set; or %NULL to unset.
 * @returns: %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Sets the interface member being invoked (%DBUS_MESSAGE_TYPE_METHOD_CALL)
 * or emitted (%DBUS_MESSAGE_TYPE_SIGNAL).
 *
 * The member name must contain only valid characters as defined in the D-Bus specification.
 */
gboolean         ibus_message_set_member        (IBusMessage        *message,
                                                 const gchar        *member);

/**
 * ibus_message_set_path:
 * @message: An IBusMessage.
 * @path: Path to set; or %NULL to unset.
 * @returns: %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Sets the object path this message is being sent to (for $DBUS_MESSAGE_TYPE_METHOD_CALL)
 * or the one a signal is being emitted from (for %DBUS_MESSAGE_TYPE_SIGNAL).
 *
 * The path must contain only valid characters as defined in the D-Bus specification.
 */
gboolean         ibus_message_set_path          (IBusMessage        *message,
                                                 const gchar        *path);

/**
 * ibus_message_set_no_reply:
 * @message: An IBusMessage.
 * @no_reply: %TRUE if no reply is desired.
 *
 * Sets a flag indicating that the message does not want a reply;
 * if this flag is set, the other end of the connection may (but is not required to)
 * optimize by not sending method return or error replies.
 *
 * If this flag is set, there is no way to know whether the message successfully arrived
 * at the remote end.
 * Normally you know a message was received when you receive the reply to it.
 *
 * The flag is FALSE by default, that is by default the other end is required to reply.
 *
 * On the protocol level this toggles %DBUS_HEADER_FLAG_NO_REPLY_EXPECTED.
 */
void             ibus_message_set_no_reply      (IBusMessage        *message,
                                                 gboolean            no_reply);

/**
 * ibus_message_set_reply_serial:
 * @message: An IBusMessage.
 * @reply_serial: The serial to be replied.
 * @returns: %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Sets the reply serial of a message (the serial of the message this is a reply to).
 */
gboolean         ibus_message_set_reply_serial  (IBusMessage        *message,
                                                 guint32             reply_serial);

/**
 * ibus_message_get_type:
 * @message: An IBusMessage.
 * @returns: Type of the IBusMessage.
 *
 * Gets the type of an IBusMessage.
 */
gint             ibus_message_get_type          (IBusMessage        *message);

/**
 * ibus_message_get_destination:
 * @message: An IBusMessage.
 * @returns: Destination of the IBusMessage; NULL if there is none set.
 *
 * Gets the destination of a message or %NULL if there is none set.
 *
 * The returned string becomes invalid if the message is modified,
 * since it points into the wire-marshaled message data.
 */
const gchar     *ibus_message_get_destination   (IBusMessage        *message);

/**
 * ibus_message_get_sender:
 * @message: An IBusMessage.
 * @returns: Sender of the IBusMessage; %NULL if unknown or inapplicable.
 *
 * Gets the unique name of the connection which originated this message,
 * or %NULL if unknown or inapplicable.
 *
 * The sender is filled in by the message bus.
 *
 * Note, the returned sender is always the unique bus name.
 * Connections may own multiple other bus names, but those are not found in the sender field.
 *
 * The returned string becomes invalid if the message is modified,
 * since it points into the wire-marshaled message data.
 */
const gchar     *ibus_message_get_sender        (IBusMessage        *message);

/**
 * ibus_message_get_error_name:
 * @message: An IBusMessage.
 * @returns: Error name of the IBusMessage; %NULL if none.
 *
 * Gets the error name (%DBUS_MESSAGE_TYPE_ERROR only) or %NULL if none.
 *
 * The returned string becomes invalid if the message is modified,
 * since it points into the wire-marshaled message data.
 */
const gchar     *ibus_message_get_error_name    (IBusMessage        *message);

/**
 * ibus_message_get_error_message:
 * @message: An IBusMessage.
 * @returns: Error message of the IBusMessage; %NULL if none.
 *
 * Gets the error message (%DBUS_MESSAGE_TYPE_ERROR only) or %NULL if none.
 *
 * The returned string becomes invalid if the message is modified,
 * since it points into the wire-marshaled message data.
 */
const gchar     *ibus_message_get_error_message (IBusMessage        *message);

/**
 * ibus_message_get_interface:
 * @message: An IBusMessage.
 * @returns: Interface name of the IBusMessage; %NULL if none.
 *
 * Gets the interface this message is being sent to (for %DBUS_MESSAGE_TYPE_METHOD_CALL)
 * or being emitted from (for %DBUS_MESSAGE_TYPE_SIGNAL).
 *
 * The interface name is fully-qualified (namespaced). Returns %NULL if none.
 *
 * The returned string becomes invalid if the message is modified,
 * since it points into the wire-marshaled message data.
 */
const gchar     *ibus_message_get_interface     (IBusMessage        *message);

/**
 * ibus_message_get_member:
 * @message: An IBusMessage.
 * @returns: Member name of the IBusMessage; %NULL if none.
 *
 * Gets the interface member being invoked (%DBUS_MESSAGE_TYPE_METHOD_CALL)
 * or emitted (%DBUS_MESSAGE_TYPE_SIGNAL).
 *
 * Returns %NULL if none.
 *
 * The returned string becomes invalid if the message is modified,
 * since it points into the wire-marshaled message data.
 */
const gchar     *ibus_message_get_member        (IBusMessage        *message);

/**
 * ibus_message_get_path:
 * @message: An IBusMessage.
 * @returns: Object path of the IBusMessage; %NULL if none.
 *
 * Gets the object path this message is being sent to (for %DBUS_MESSAGE_TYPE_METHOD_CALL)
 * or being emitted from (for %DBUS_MESSAGE_TYPE_SIGNAL).
 *
 * Returns %NULL if none.
 *
 * See also dbus_message_get_path_decomposed().
 *
 * The returned string becomes invalid if the message is modified,
 * since it points into the wire-marshaled message data.
 */
const gchar     *ibus_message_get_path          (IBusMessage        *message);

/**
 * ibus_message_get_no_reply:
 * @message: An IBusMessage.
 * @returns: %TRUE if the message does not expect a reply; %FALSE otherwise.
 *
 * Returns TRUE if the message does not expect a reply.
 */
gboolean         ibus_message_get_no_reply      (IBusMessage        *message);

/**
 * ibus_message_get_reply_serial:
 * @message: An IBusMessage.
 * @returns: The serial that the message is a reply to or 0 if none.
 *
 * Returns the serial that the message is a reply to or 0 if none.
 */
guint32          ibus_message_get_reply_serial  (IBusMessage        *message);

/**
 * ibus_message_get_serial:
 * @message: An IBusMessage.
 * @returns: The serial of a message or 0 if none has been specified.
 *
 * Returns the serial of a message or 0 if none has been specified.
 *
 * The message's serial number is provided by the application sending the message
 * and is used to identify replies to this message.
 *
 * All messages received on a connection will have a serial provided by the remote application.
 *
 * For messages you're sending, dbus_connection_send() will assign a serial and return it to you.
 */
guint32          ibus_message_get_serial        (IBusMessage        *message);

/**
 * ibus_message_append_args:
 * @message: An IBusMessage.
 * @first_arg_type: Type of the first argument.
 * @...: Rest of arguments.
 * @returns: %TRUE if succeed; %FALSE otherwise.
 *
 * Appends fields to a message given a variable argument list.
 *
 * The variable argument list should contain the type of each argument followed by the value to append.
 * Appendable types are basic types, and arrays of fixed-length basic types.
 * To append variable-length basic types, or any more complex value,
 * you have to use an iterator rather than this function.
 *
 * To append a basic type, specify its type code followed by the address of the value. For example:
 * <informalexample>
 *    <programlisting>
 *     dbus_int32_t v_INT32 = 42;
 *     const char *v_STRING = "Hello World";
 *     dbus_message_append_args (message,
 *     DBUS_TYPE_INT32, &v_INT32,
 *     DBUS_TYPE_STRING, &v_STRING,
 *     DBUS_TYPE_INVALID);
 *    </programlisting>
 * </informalexample>
 *
 * To append an array of fixed-length basic types, pass in the %DBUS_TYPE_ARRAY typecode,
 * the element typecode, the address of the array pointer,
 * and a 32-bit integer giving the number of elements in the array. So for example:
 * <informalexample>
 *    <programlisting>
 *     const dbus_int32_t array[] = { 1, 2, 3 };
 *     const dbus_int32_t *v_ARRAY = array;
 *     dbus_message_append_args (message,
 *     DBUS_TYPE_ARRAY, DBUS_TYPE_INT32, &v_ARRAY, 3,
 *     DBUS_TYPE_INVALID);
 *    </programlisting>
 * </informalexample>
 *
 *
 * <note><para>
 * in C, given "int array[]", "&array == array" (the comp.lang.c FAQ says otherwise, but gcc and the FAQ don't agree).
 * So if you're using an array instead of a pointer you have to create a pointer variable,
 * assign the array to it, then take the address of the pointer variable.
 * For strings it works to write const char *array = "Hello" and then use &amp;array though.
 * </para></note>
 *
 * The last argument to this function must be %DBUS_TYPE_INVALID, marking the end of the argument list.
 * If you don't do this then libdbus won't know to stop and will read invalid memory.
 *
 * String/signature/path arrays should be passed in as "const char*** address_of_array" and "int n_elements"
 *
 * @see_also: ibus_message_append_args_valist().
 */
gboolean         ibus_message_append_args       (IBusMessage        *message,
                                                 GType               first_arg_type,
                                                 ...);

/**
 * ibus_message_append_args_valist:
 * @message: An IBusMessage.
 * @first_arg_type: Type of the first argument.
 * @va_args: Rest of arguments.
 * @returns: %TRUE if succeed; %FALSE otherwise.
 *
 * Like ibus_message_append_args() but takes a va_list for use by language bindings.
 *
 * @see_also: ibus_message_append_args().
 */
gboolean         ibus_message_append_args_valist(IBusMessage        *message,
                                                 GType               first_arg_type,
                                                 va_list             va_args);

/**
 * ibus_message_get_args:
 * @message: An IBusMessage.
 * @error: Error to be filled in on failure.
 * @first_arg_type: Type of the first argument.
 * @...: Rest of arguments.
 * @returns: %TRUE if succeed; F%ALSE otherwise.
 *
 * Gets arguments from a message given a variable argument list.
 *
 * The supported types include those supported by ibus_message_append_args();
 * that is, basic types and arrays of fixed-length basic types.
 * The arguments are the same as they would be for ibus_message_iter_get_basic()
 * or ibus_message_iter_get_fixed_array().
 *
 * In addition to those types, arrays of string, object path, and signature are supported;
 * but these are returned as allocated memory and must be freed with dbus_free_string_array(),
 * while the other types are returned as const references.
 * To get a string array pass in "char ***array_location" and "int *n_elements".
 *
 * The variable argument list should contain the type of the argument followed by a pointer to
 * where the value should be stored. The list is terminated with %DBUS_TYPE_INVALID.
 *
 * Except for string arrays, the returned values are constant; do not free them.
 * They point into the IBusMessage.
 *
 * If the requested arguments are not present, or do not have the requested types, then an error will be set.
 *
 * If more arguments than requested are present,
 * the requested arguments are returned and the extra arguments are ignored.
 *
 * @see_also: ibus_message_append_args(), ibus_message_get_args_valist().
 */
gboolean         ibus_message_get_args          (IBusMessage        *message,
                                                 IBusError          **error,
                                                 GType               first_arg_type,
                                                 ...);

/**
 * ibus_message_get_args_valist:
 * @message: An IBusMessage.
 * @error:  Error message is outputted here; or %NULL to suppress error.
 * @first_arg_type: Type of the first argument.
 * @va_args: Rest of arguments.
 * @returns: %TRUE if succeed; %FALSE otherwise.
 *
 * Like ibus_message_get_args but takes a va_list for use by language bindings.
 *
 * @see_also: ibus_message_append_args_valist(), ibus_message_get_args().
 */
gboolean         ibus_message_get_args_valist   (IBusMessage        *message,
                                                 IBusError          **error,
                                                 GType               first_arg_type,
                                                 va_list             va_args);

/**
 * ibus_message_iter_init_append:
 * @message: An IBusMessage.
 * @iter: An IBusMessageIter to to initialize.
 *
 * Initializes a #IBusMessageIter for appending arguments to the end of a message.
 */
void             ibus_message_iter_init_append  (IBusMessage        *message,
                                                 IBusMessageIter    *iter);
/**
 * ibus_message_iter_append:
 * @iter: An IBusMessageIter.
 * @type: The type of the value.
 * @value: The pointer to the value.
 * @returns: %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Appends a basic-typed value to the message.
 *
 * The basic types are the non-container types such as integer and string.
 *
 * The "value" argument should be the address of a basic-typed value.
 * So for string, const char**. For integer, dbus_int32_t*.
 */
gboolean         ibus_message_iter_append       (IBusMessageIter    *iter,
                                                 GType               type,
                                                 gconstpointer       value);

/**
 * ibus_message_iter_copy_data:
 * @dst: Destination to be copy to.
 * @src: Source to be copy from.
 * @returns: %TRUE if succeed; %FALSE if failed.
 *
 * Deep copy an IBusMessageIter to another IBusMessageIter.
 *
 * Since: 1.2.0.20090719
 */
gboolean         ibus_message_iter_copy_data    (IBusMessageIter    *dst,
                                                 IBusMessageIter    *src);

/**
 * ibus_message_iter_init:
 * @message: An IBusMessage.
 * @iter: An IBusMessageIter.
 * @returns: %TRUE if succeed; %FALSE if the message has no arguments.
 *
 * Initializes an #IBusMessageIter for reading the arguments of the message passed in.
 *
 * When possible, ibus_message_get_args() is much more convenient.
 * Some types of argument can only be read with IBusMessageIter however.
 *
 * The easiest way to iterate is like this:
 * <informalexample>
 *    <programlisting>
 *     dbus_message_iter_init (&iter);
 *     while ((current_type = dbus_message_iter_get_arg_type (&iter)) != DBUS_TYPE_INVALID)
 *     dbus_message_iter_next (&iter);
 *    </programlisting>
 * </informalexample>
 *
 * IBusMessageIter contains no allocated memory;
 * it need not be freed, and can be copied by assignment or memcpy().
 */
gboolean         ibus_message_iter_init         (IBusMessage        *message,
                                                 IBusMessageIter    *iter);

/**
 * ibus_message_iter_get_basic:
 * @iter: An IBusMessageIter.
 * @value: Result value stores here. Cannot be %NULL.
 *
 * Reads a basic-typed value from the message iterator.
 *
 * Basic types are the non-containers such as integer and string.
 *
 * The value argument should be the address of a location to store the returned value.
 * So for int32 it should be a "dbus_int32_t*" and for string a "const char**".
 * The returned value is by reference and should not be freed.
 *
 * Be sure you have somehow checked that dbus_message_iter_get_arg_type() matches the type you are expecting,
 * or you'll crash when you try to use an integer as a string or something.
 *
 * To read any container type (array, struct, dict) you will need to recurse into the container with
 * dbus_message_iter_recurse().
 * If the container is an array of fixed-length values,
 * you can get all the array elements at once with dbus_message_iter_get_fixed_array().
 * Otherwise, you have to iterate over the container's contents one value at a time.
 *
 * All basic-typed values are guaranteed to fit in 8 bytes. So you can write code like this:
 * <informalexample>
 *    <programlisting>
 *     dbus_uint64_t value;
 *     int type;
 *     dbus_message_iter_get_basic (&read_iter, &value);
 *     type = dbus_message_iter_get_arg_type (&read_iter);
 *     dbus_message_iter_append_basic (&write_iter, type, &value);
 *    </programlisting>
 * </informalexample>
 *
 * On some really obscure platforms dbus_uint64_t might not exist,
 * if you need to worry about this you will know.
 * dbus_uint64_t is just one example of a type that's large enough to hold any possible value,
 * you could use a struct or char[8] instead if you like.
 */
void             ibus_message_iter_get_basic    (IBusMessageIter    *iter,
                                                 gpointer            value);

/**
 * ibus_message_iter_get:
 * @iter: An IBusMessageIter.
 * @type: The type of the value. Cannot be %NULL.
 * @value: Result value stores here. Cannot be %NULL.
 * @returns: %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Gets an value from an IBusMessageIter, then move on to the next element.
 *
 */
gboolean         ibus_message_iter_get          (IBusMessageIter    *iter,
                                                 GType               type,
                                                 gpointer            value);

/**
 * ibus_message_iter_next:
 * @iter: An IBusMessageIter.
 * @returns: %TRUE if the iterator moves forward successfully; %FALSE if next element does not exist.
 *
 * Moves the iterator to the next field, if any.
 *
 * If there's no next field, returns %FALSE. If the iterator moves forward, returns %TRUE.
 */
gboolean         ibus_message_iter_next         (IBusMessageIter    *iter);

/**
 * ibus_message_iter_has_next:
 * @iter: An IBusMessageIter.
 * @returns: %TRUE if next element exists; %FALSE otherwise.
 *
 * Checks if an iterator has any more fields.
 */
gboolean         ibus_message_iter_has_next     (IBusMessageIter    *iter);

/**
 * ibus_message_iter_open_container:
 * @iter: An IBusMessageIter.
 * @type: The type of the value.
 * @contained_signature: The type of container contents.
 * @sub: Sub-iterator to initialize.
 * @returns: %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Appends a container-typed value to the message;
 * you are required to append the contents of the container using the returned sub-iterator,
 * and then call dbus_message_iter_close_container().
 *
 * Container types are for example struct, variant, and array.
 * For variants, the contained_signature should be the type of the single value inside the variant.
 * For structs and dict entries, contained_signature should be %NULL;
 * it will be set to whatever types you write into the struct.
 * For arrays, contained_signature should be the type of the array elements.
 */
gboolean         ibus_message_iter_open_container
                                                (IBusMessageIter    *iter,
                                                 GType               type,
                                                 const gchar        *contained_signature,
                                                 IBusMessageIter    *sub);

/**
 * ibus_message_iter_close_container:
 * @iter: An IBusMessageIter.
 * @sub: Sub-iterator to close.
 * @returns: %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Closes a container-typed value appended to the message;
 * may write out more information to the message known only after the entire container is written,
 * and may free resources created by dbus_message_iter_open_container().
 */
gboolean         ibus_message_iter_close_container
                                                (IBusMessageIter    *iter,
                                                 IBusMessageIter    *sub);

/**
 * ibus_message_iter_recurse:
 * @iter: An IBusMessageIter.
 * @type: The type of the value.
 * @sub: Sub-iterator to initialize.
 * @returns:  %TRUE if succeed; %FALSE if insufficient memory.
 *
 * Recurses into a container value when reading values from a message,
 * initializing a sub-iterator to use for traversing the child values of the container.
 *
 * Note that this recurses into a value, not a type, so you can only recurse if the value exists.
 * The main implication of this is that if you have for example an empty array of array of int32,
 * you can recurse into the outermost array, but it will have no values, so you won't be able to recurse further.
 * There's no array of int32 to recurse into.
 *
 * If a container is an array of fixed-length types, it is much more efficient to use
 *  dbus_message_iter_get_fixed_array() to get the whole array in one shot,
 *  rather than individually walking over the array elements.
 *
 * Be sure you have somehow checked that dbus_message_iter_get_arg_type()
 * matches the type you are expecting to recurse into.
 * Results of this function are undefined if there is no container to recurse into at the current iterator position.
 */
gboolean         ibus_message_iter_recurse      (IBusMessageIter    *iter,
                                                 GType               type,
                                                 IBusMessageIter    *sub);

/**
 * ibus_message_iter_get_arg_type:
 * @iter: An IBusMessageIter.
 * @returns:  The argument type.
 *
 * Returns the argument type of the argument that the message iterator points to.
 *
 * If the iterator is at the end of the message, returns %DBUS_TYPE_INVALID.
 * You can thus write a loop as follows:
 * <informalexample>
 *    <programlisting>
 *     dbus_message_iter_init (&iter);
 *     while ((current_type = dbus_message_iter_get_arg_type (&iter)) != DBUS_TYPE_INVALID)
 *     dbus_message_iter_next (&iter);
 *    </programlisting>
 * </informalexample>
 */
GType            ibus_message_iter_get_arg_type (IBusMessageIter    *iter);

/**
 * ibus_message_iter_get_element_type:
 * @iter: An IBusMessageIter.
 * @returns:  The argument type.
 *
 * Returns the element type of the array that the message iterator points to.
 * Note that you need to check that the iterator points to an array prior to using this function.
 */
GType            ibus_message_iter_get_element_type
                                                (IBusMessageIter    *iter);

/**
 * ibus_message_to_string:
 * @message: An IBusMessage.
 * @returns: A string which shows the information of the message.
 *
 * Produces a pretty formatted string which show the information of the IBusMessage.
 * This string is suitable for debugging information print out.
 *
 * Free the string by g_free() after use.
 */
gchar           *ibus_message_to_string         (IBusMessage *message);

G_END_DECLS
#endif
