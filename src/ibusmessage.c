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

static GQuark   quark_serialize = 0;
static GQuark   quark_deserialize = 0;

GType
ibus_type_get_object_path (void)
{
    static GType type = 0;

    if (type == 0) {
        type = g_type_register_static_simple (G_TYPE_STRING,
                                              "IBusObjectPath",
                                              0,
                                              NULL,
                                              0,
                                              NULL,
                                              0);
    }
    return type;
}

GType
ibus_type_get_array (void)
{
    static GType type = 0;

    if (type == 0) {
        type = g_type_register_static_simple (G_TYPE_BOXED,
                                              "IBusArray",
                                              0,
                                              NULL,
                                              0,
                                              NULL,
                                              0);
    }
    return type;
}

GType
ibus_type_get_struct (void)
{
    static GType type = 0;

    if (type == 0) {
        type = g_type_register_static_simple (G_TYPE_BOXED,
                                              "IBusStruct",
                                              0,
                                              NULL,
                                              0,
                                              NULL,
                                              0);
    }
    return type;
}

GType
ibus_type_get_variant (void)
{
    static GType type = 0;

    if (type == 0) {
        type = g_type_register_static_simple (G_TYPE_BOXED,
                                              "IBusVariant",
                                              0,
                                              NULL,
                                              0,
                                              NULL,
                                              0);
    }
    return type;
}

IBusMessage *
ibus_message_new (gint message_type)
{
    return dbus_message_new (message_type);
}

IBusMessage *
ibus_message_ref (IBusMessage *message)
{
    return dbus_message_ref (message);
}

void
ibus_message_unref (IBusMessage *message)
{
    dbus_message_unref (message);
}

IBusMessage *
ibus_message_new_method_call (const gchar   *destination,
                              const gchar   *path,
                              const gchar   *interface,
                              const gchar   *method)
{
    return dbus_message_new_method_call (destination,
                                         path,
                                         interface,
                                         method);
}

IBusMessage *
ibus_message_new_method_return (IBusMessage *reply_to)
{
    return dbus_message_new_method_return (reply_to);
}

IBusMessage *
ibus_message_new_error (IBusMessage *reply_to,
                        const gchar *error_name,
                        const gchar *error_message)
{
    return dbus_message_new_error (reply_to,
                                   error_name,
                                   error_message);
}

IBusMessage *
ibus_message_new_error_printf (IBusMessage *reply_to,
                               const gchar *error_name,
                               const gchar *error_format,
                               ...)
{
    va_list va_args;
    gchar *error_message;
    IBusMessage *message;

    va_start (va_args, error_format);
    error_message = g_strdup_vprintf (error_format, va_args);
    va_end (va_args);

    message = ibus_message_new_error (reply_to,
                                      error_name,
                                      error_message);
    g_free (error_message);

    return message;
}

IBusMessage *
ibus_message_new_signal (const gchar    *path,
                         const gchar    *interface,
                         const gchar    *name)
{
    return dbus_message_new_signal (path,
                                    interface,
                                    name);
}

gboolean
ibus_message_is_method_call (IBusMessage    *message,
                             const gchar    *interface,
                             const gchar    *method)
{
    return dbus_message_is_method_call (message,
                                        interface,
                                        method);
}

gboolean
ibus_message_is_error (IBusMessage  *message,
                       const gchar  *error_name)
{
    return dbus_message_is_error (message, error_name);
}

gboolean
ibus_message_is_signal (IBusMessage     *message,
                        const gchar     *interface,
                        const gchar     *signal_name)
{
    return dbus_message_is_signal (message,
                                   interface,
                                   signal_name);
}

gboolean
ibus_message_set_destination (IBusMessage   *message,
                              const gchar   *destination)
{
    return dbus_message_set_destination (message, destination);
}

gboolean
ibus_message_set_sender (IBusMessage    *message,
                         const gchar    *sender)
{
    return dbus_message_set_sender (message, sender);
}

gboolean
ibus_message_set_error_name (IBusMessage    *message,
                             const gchar    *error_name)
{
    return dbus_message_set_error_name (message, error_name);
}

gboolean
ibus_message_set_interface (IBusMessage     *message,
                            const gchar     *interface)
{
    return dbus_message_set_interface (message, interface);
}

gboolean
ibus_message_set_member (IBusMessage    *message,
                         const gchar    *member)
{
    return dbus_message_set_member (message, member);
}

gboolean
ibus_message_set_path (IBusMessage  *message,
                       const gchar  *path)
{
    return dbus_message_set_path (message, path);
}

void
ibus_message_set_no_reply (IBusMessage  *message,
                           gboolean      no_reply)
{
    return dbus_message_set_no_reply (message, no_reply);
}

gboolean
ibus_message_set_reply_serial (IBusMessage  *message,
                               guint32       reply_serial)
{
    return dbus_message_set_reply_serial (message, reply_serial);
}

gint
ibus_message_get_type (IBusMessage *message)
{
    return dbus_message_get_type (message);
}

const gchar *
ibus_message_get_destination (IBusMessage *message)
{
    return dbus_message_get_destination (message);
}

const gchar *
ibus_message_get_sender (IBusMessage *message)
{
    return dbus_message_get_sender (message);
}

const gchar *
ibus_message_get_error_name (IBusMessage *message)
{
    return dbus_message_get_error_name (message);
}

const gchar *
ibus_message_get_interface (IBusMessage *message)
{
    return dbus_message_get_interface (message);
}

const gchar *
ibus_message_get_member (IBusMessage *message)
{
    return dbus_message_get_member (message);
}

const gchar *
ibus_message_get_path (IBusMessage *message)
{
    return dbus_message_get_path (message);
}

gboolean
ibus_message_get_no_reply (IBusMessage *message)
{
    return dbus_message_get_no_reply (message);
}

guint32
ibus_message_get_reply_serial (IBusMessage *message)
{
    return dbus_message_get_reply_serial (message);
}

guint32
ibus_message_get_serial (IBusMessage *message)
{
    return dbus_message_get_serial (message);
}

gboolean
ibus_message_append_args (IBusMessage *message,
                          GType        first_arg_type,
                          ...)
{
    gboolean retval;
    va_list va_args;

    va_start (va_args, first_arg_type);
    retval = ibus_message_append_args_valist (message,
                                              first_arg_type,
                                              va_args);
    va_end (va_args);

    return retval;
}

gboolean
ibus_message_append_args_valist (IBusMessage *message,
                                 GType        first_arg_type,
                                 va_list      va_args)
{
    GType type;
    gboolean retval;
    IBusMessageIter iter;

    ibus_message_iter_init_append (message, &iter);
    
    type = first_arg_type;
    while (type != G_TYPE_INVALID) {
        gpointer value = va_arg (va_args, gpointer);
        retval = ibus_message_iter_append (&iter, type, value);
        type = va_arg (va_args, GType);
        
        if (!retval)
            return FALSE;
    }
    
    return TRUE;
}

gboolean
ibus_message_get_args (IBusMessage  *message,
                       IBusError    **error,
                       GType         first_arg_type,
                       ...)
{
    gboolean retval;
    va_list va_args;

    va_start (va_args, first_arg_type);
    retval = ibus_message_get_args_valist (message,
                                           error,
                                           first_arg_type,
                                           va_args);
    va_end (va_args);

    return retval;

}

gboolean
ibus_message_get_args_valist (IBusMessage *message,
                              IBusError   **error,
                              GType        first_arg_type,
                              va_list      va_args)
{
    g_assert (message != NULL);
    
    gboolean retval;
    IBusMessageIter iter;
    GType type;
    gpointer value;
    
    retval = ibus_message_iter_init (message, &iter);

    if (!retval)
        return FALSE;

    type = first_arg_type;
    while (type != G_TYPE_INVALID) {
        value = va_arg (va_args, gpointer);
        retval = ibus_message_iter_get (&iter, type, value);
        if (!retval) {
            if (error)
                *error = ibus_error_from_text (DBUS_ERROR_INVALID_ARGS, "");
            return FALSE;
        }
        type = va_arg (va_args, GType);
    }
    return TRUE;
}

void
ibus_message_iter_init_append (IBusMessage      *message,
                               IBusMessageIter  *iter)
{
    dbus_message_iter_init_append (message, iter);
}

gboolean
ibus_message_iter_append (IBusMessageIter *iter,
                          GType            type,
                          gconstpointer    value)
{
    g_assert (iter != NULL);
    g_assert (type != G_TYPE_INVALID);
    g_assert (value != NULL);

    switch (type) {
    case G_TYPE_INT:
        {
            dbus_int32_t v;
            v = * (gint *)value;
            return dbus_message_iter_append_basic (iter, DBUS_TYPE_INT32, &v);
        }
    case G_TYPE_UINT:
        {
            dbus_uint32_t v;
            v = * (guint *)value;
            return dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT32, &v);
        }
    case G_TYPE_BOOLEAN:
        {
            dbus_bool_t v;
            v = * (gboolean *)value;
            return dbus_message_iter_append_basic (iter, DBUS_TYPE_BOOLEAN, &v);
        }
    case G_TYPE_STRING:
        {
            const gchar *v;
            v = * (gchar **)value;
            return dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, &v);
        }
    case G_TYPE_INT64:
        {
            dbus_int64_t v;
            v = * (gint64 *)value;
            return dbus_message_iter_append_basic (iter, DBUS_TYPE_INT64, &v);
        }
    case G_TYPE_UINT64:
        {
            dbus_uint64_t v;
            v = * (guint64 *)value;
            return dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT64, &v);
        }
    case G_TYPE_FLOAT:
        {
            double v;
            v = * (gfloat *)value;
            return dbus_message_iter_append_basic (iter, DBUS_TYPE_DOUBLE, &v);
        }

    case G_TYPE_DOUBLE:
        {
            double v;
            v = * (gdouble *)value;
            return dbus_message_iter_append_basic (iter, DBUS_TYPE_DOUBLE, &v);
        }
    default:
        if (type == IBUS_TYPE_OBJECT_PATH) {
            const gchar *v;
            v = * (gchar **)value;
            return dbus_message_iter_append_basic (iter, DBUS_TYPE_OBJECT_PATH, &v);
        }
    }

    IBusSerializeFunc func;
    func = (IBusSerializeFunc) g_type_get_qdata (type, quark_serialize);

    if (func == NULL) {
        g_warning ("Type %s doesn't support serizlize", g_type_name (type));
        return FALSE;
    }

    return func (*(gconstpointer *)value, iter);
}

gboolean
ibus_message_iter_init (IBusMessage     *message,
                        IBusMessageIter *iter)
{
    return dbus_message_iter_init (message, iter);
}

void
ibus_message_iter_get_basic (IBusMessageIter *iter,
                             gpointer         value)
{
    dbus_message_iter_get_basic (iter, value);
}

gboolean
ibus_message_iter_get (IBusMessageIter *iter,
                       GType            type,
                       gpointer         value)
{
    gboolean retval;

    retval = ibus_message_iter_peek (iter, type, value);
    ibus_message_iter_next (iter);

    return retval;
}

gboolean
ibus_message_iter_peek (IBusMessageIter *iter,
                        GType            type,
                        gpointer         value)
{
    g_assert (iter != NULL);
    g_assert (type != G_TYPE_INVALID);
    g_assert (value != NULL);

    switch (type) {
    case G_TYPE_INT:
        {
            dbus_int32_t v;
            if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_INT32)
                return FALSE;
            dbus_message_iter_get_basic (iter, &v);
            *(gint *) value = (gint) v;
            return TRUE;
        }
    case G_TYPE_UINT:
        {
            dbus_uint32_t v;
            if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_UINT32)
                return FALSE;
            dbus_message_iter_get_basic (iter, &v);
            *(guint *) value = (guint) v;
            return TRUE;
        }
    case G_TYPE_BOOLEAN:
        {
            dbus_bool_t v;
            if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_BOOLEAN)
                return FALSE;
            dbus_message_iter_get_basic (iter, &v);
            *(gboolean *) value = (gboolean) v;
            return TRUE;
        }
    case G_TYPE_STRING:
        {
            gchar *v;
            if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_STRING)
                return FALSE;
            dbus_message_iter_get_basic (iter, &v);
            *(gchar **) value = (gchar *) v;
            return TRUE;
        }
    case G_TYPE_INT64:
        {
            dbus_int64_t v;
            if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_INT64)
                return FALSE;
            dbus_message_iter_get_basic (iter, &v);
            *(gint64 *) value = (gint64) v;
            return TRUE;
        }
    case G_TYPE_UINT64:
        {
            dbus_uint64_t v;
            if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_UINT64)
                return FALSE;
            dbus_message_iter_get_basic (iter, &v);
            *(guint64 *) value = (guint64) v;
            return TRUE;
        }
    case G_TYPE_FLOAT:
        {
            double v;
            if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_DOUBLE)
                return FALSE;
            dbus_message_iter_get_basic (iter, &v);
            *(gfloat *) value = (gfloat) v;
            return TRUE;
        }

    case G_TYPE_DOUBLE:
        {
            double v;
            if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_DOUBLE)
                return FALSE;
            dbus_message_iter_get_basic (iter, &v);
            *(gdouble *) value = (gdouble) v;
            return TRUE;
        }
    default:
        if (type == IBUS_TYPE_OBJECT_PATH) {
            gchar *v;
            if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_OBJECT_PATH)
                return FALSE;
            dbus_message_iter_get_basic (iter, &v);
            *(gchar **) value = (gchar *) v;
            return TRUE;
        }
    }

    IBusDeserializeFunc func;
    gpointer v;
    func = (IBusDeserializeFunc) g_type_get_qdata (type, quark_deserialize);

    if (func == NULL) {
        g_warning ("Type %s doesn't support deserizlize", g_type_name (type));
        return FALSE;
    }

    v = func (iter);

    if (v != NULL) {
        *(gpointer *)value = v;
        return TRUE;
    }
    return FALSE;
}

gboolean
ibus_message_iter_next (IBusMessageIter *iter)
{
    return dbus_message_iter_next (iter);
}

gboolean
ibus_message_iter_has_next (IBusMessageIter *iter)
{
    return dbus_message_iter_has_next (iter);
}

gboolean
ibus_message_iter_open_container (IBusMessageIter   *iter,
                                  GType              type,
                                  const gchar       *contained_signature,
                                  IBusMessageIter   *sub)
{
    gint dbus_type;
    
    if (type == IBUS_TYPE_ARRAY) {
        dbus_type = DBUS_TYPE_ARRAY;
    }
    else if (type == IBUS_TYPE_STRUCT) {
        dbus_type = DBUS_TYPE_STRUCT;
    }
    else
        g_return_val_if_reached (FALSE);

    return dbus_message_iter_open_container (iter,
                                             dbus_type,
                                             contained_signature,
                                             sub);
}

gboolean
ibus_message_iter_close_container (IBusMessageIter *iter,
                                   IBusMessageIter *sub)
{
    return dbus_message_iter_close_container (iter, sub);
}

gboolean
ibus_message_iter_recurse (IBusMessageIter   *iter,
                           GType              type,
                           IBusMessageIter   *sub)
{
    gint dbus_type;
    
    if (type == IBUS_TYPE_ARRAY) {
        dbus_type = DBUS_TYPE_ARRAY;
    }
    else if (type == IBUS_TYPE_STRUCT) {
        dbus_type = DBUS_TYPE_STRUCT;
    }
    else
        g_return_val_if_reached (FALSE);

    g_return_val_if_fail (dbus_type == dbus_message_iter_get_arg_type (iter), FALSE);
    
    dbus_message_iter_recurse (iter, sub);
    
    return TRUE;
}

GType
ibus_message_iter_get_arg_type (IBusMessageIter *iter)
{
    gint type;

    type = dbus_message_iter_get_arg_type (iter);
    switch (type) {
#define TYPE_TABLE(a, b) case a: return (b);
        TYPE_TABLE (DBUS_TYPE_BYTE, G_TYPE_CHAR);
        TYPE_TABLE (DBUS_TYPE_INT32, G_TYPE_INT);
        TYPE_TABLE (DBUS_TYPE_INT64, G_TYPE_INT64);
        TYPE_TABLE (DBUS_TYPE_UINT32, G_TYPE_UINT);
        TYPE_TABLE (DBUS_TYPE_UINT64, G_TYPE_UINT64);
        TYPE_TABLE (DBUS_TYPE_BOOLEAN, G_TYPE_BOOLEAN);
        TYPE_TABLE (DBUS_TYPE_DOUBLE, G_TYPE_DOUBLE);
        TYPE_TABLE (DBUS_TYPE_STRING, G_TYPE_STRING);
        TYPE_TABLE (DBUS_TYPE_OBJECT_PATH, IBUS_TYPE_OBJECT_PATH);
        TYPE_TABLE (DBUS_TYPE_ARRAY, IBUS_TYPE_ARRAY);
        TYPE_TABLE (DBUS_TYPE_STRUCT, IBUS_TYPE_STRUCT);
        TYPE_TABLE (DBUS_TYPE_VARIANT, IBUS_TYPE_VARIANT);
#undef TYPE_TABLE
    }
    return G_TYPE_INVALID;
}

gboolean
ibus_message_register_type (GType               type,
                            IBusSerializeFunc   serialize_func,
                            IBusDeserializeFunc deserialize_func)
{
    g_assert (type != G_TYPE_INVALID);
    g_assert (serialize_func != NULL);
    g_assert (deserialize_func != NULL);

    if (quark_serialize == 0 || quark_deserialize) {
        quark_serialize = g_quark_from_static_string ("serialize");
        quark_deserialize = g_quark_from_static_string ("deserialize");
    }

    g_assert (g_type_get_qdata (type, quark_serialize) == NULL);
    g_assert (g_type_get_qdata (type, quark_deserialize) == NULL);

    g_type_set_qdata (type, quark_serialize, serialize_func);
    g_type_set_qdata (type, quark_deserialize, deserialize_func);

    return TRUE;
}
