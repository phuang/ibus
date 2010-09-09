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
#include "ibusserializable.h"

#define IBUS_SERIALIZABLE_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_SERIALIZABLE, IBusSerializablePrivate))

enum {
    LAST_SIGNAL,
};

typedef struct _IBusSerializablePrivate IBusSerializablePrivate;
struct _IBusSerializablePrivate {
    GData   *attachments;
};

// static guint    object_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void      ibus_serializable_base_init        (IBusSerializableClass  *klass);
static void      ibus_serializable_base_fini        (IBusSerializableClass  *klass);
static void      ibus_serializable_class_init       (IBusSerializableClass  *klass);
static void      ibus_serializable_init             (IBusSerializable       *object);
static void      ibus_serializable_destroy          (IBusSerializable       *object);
static gboolean  ibus_serializable_real_serialize   (IBusSerializable       *object,
                                                     IBusMessageIter        *iter);
static gboolean  ibus_serializable_real_deserialize (IBusSerializable       *object,
                                                     IBusMessageIter        *iter);
static gboolean  ibus_serializable_real_copy        (IBusSerializable       *dest,
                                                     const IBusSerializable *src);

static IBusObjectClass *parent_class = NULL;

GType
ibus_serializable_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusSerializableClass),
        (GBaseInitFunc)     ibus_serializable_base_init,
        (GBaseFinalizeFunc) ibus_serializable_base_fini,
        (GClassInitFunc)    ibus_serializable_class_init,
        NULL,               /* class finialize */
        NULL,               /* class data */
        sizeof (IBusSerializable),
        0,
        (GInstanceInitFunc) ibus_serializable_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                                       "IBusSerializable",
                                       &type_info,
                                       0);
    }

    return type;
}

IBusSerializable *
ibus_serializable_new (void)
{
    return IBUS_SERIALIZABLE (g_object_new (IBUS_TYPE_SERIALIZABLE, NULL));
}

static void
ibus_serializable_base_init     (IBusSerializableClass *klass)
{
    /* init signature */
    klass->signature = g_string_new ("a{sv}");
}

static void
ibus_serializable_base_fini     (IBusSerializableClass *klass)
{
    /* init signature */
    g_string_free (klass->signature, TRUE);
    klass->signature = NULL;
}

static void
ibus_serializable_class_init     (IBusSerializableClass *klass)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusSerializablePrivate));

    object_class->destroy = (IBusObjectDestroyFunc) ibus_serializable_destroy;

    klass->serialize = ibus_serializable_real_serialize;
    klass->deserialize = ibus_serializable_real_deserialize;
    klass->copy = ibus_serializable_real_copy;
}

static void
ibus_serializable_init (IBusSerializable *object)
{
    IBusSerializablePrivate *priv;
    priv = IBUS_SERIALIZABLE_GET_PRIVATE (object);

    priv->attachments = NULL;
    g_datalist_init (&priv->attachments);
}

static void
ibus_serializable_destroy (IBusSerializable *object)
{
    IBusSerializablePrivate *priv;
    priv = IBUS_SERIALIZABLE_GET_PRIVATE (object);

    g_datalist_clear (&priv->attachments);

    parent_class->destroy (IBUS_OBJECT (object));
}

static GValue *
ibus_g_value_dup (const GValue *value)
{
    GValue *new_value;

    new_value = g_slice_new0 (GValue);
    g_value_init (new_value, G_VALUE_TYPE (value));
    g_value_copy (value, new_value);

    return new_value;
}

static void
ibus_g_value_free (GValue *value)
{
    g_value_unset (value);
    g_slice_free (GValue, value);
}

static gboolean
_g_value_serialize (GValue          *value,
                    IBusMessageIter *iter)
{
    gboolean retval;
    GType type;

    type = G_VALUE_TYPE (value);
    g_return_val_if_fail (type != G_TYPE_INVALID, FALSE);


    if (g_type_is_a (type, IBUS_TYPE_SERIALIZABLE)) {
        IBusSerializable *object;
        object = IBUS_SERIALIZABLE (g_value_get_object (value));
        retval = ibus_message_iter_append (iter,
                                           type,
                                           &object);
        g_return_val_if_fail (retval, FALSE);
        return TRUE;
    }

    typedef const gchar *gstring;
    switch (type) {
#define CASE_ENTRY(TYPE, _type, signature)                                          \
    case G_TYPE_##TYPE:                                                             \
        {                                                                           \
            g##_type v;                                                             \
            IBusMessageIter variant_iter;                                           \
                                                                                    \
            retval = ibus_message_iter_open_container (iter,                        \
                                                       IBUS_TYPE_VARIANT,           \
                                                       signature,                   \
                                                       &variant_iter);              \
            g_return_val_if_fail (retval, FALSE);                                   \
                                                                                    \
            v = g_value_get_##_type (value);                                        \
            retval = ibus_message_iter_append (&variant_iter,                       \
                                                G_TYPE_##TYPE,                      \
                                                &v);                                \
            g_return_val_if_fail (retval, FALSE);                                   \
                                                                                    \
            retval = ibus_message_iter_close_container (iter, &variant_iter);       \
            g_return_val_if_fail (retval, FALSE);                                   \
                                                                                    \
            return TRUE;                                                            \
        }
    CASE_ENTRY(CHAR, char, "y");
    CASE_ENTRY(BOOLEAN, boolean, "b");
    CASE_ENTRY(INT, int, "i");
    CASE_ENTRY(UINT, uint, "u");
    CASE_ENTRY(INT64, int64,  "x");
    CASE_ENTRY(UINT64, uint64, "t");
    CASE_ENTRY(FLOAT, float,  "d");
    CASE_ENTRY(DOUBLE, double, "d");
    CASE_ENTRY(STRING, string, "s");
#undef CASE_ENTRY
    }

    g_return_val_if_reached (FALSE);
}

static GValue *
_g_value_deserialize (IBusMessageIter *iter)
{
    IBusMessageIter variant_iter;
    gboolean retval;
    GValue *value = NULL;
    GType type;

    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_VARIANT, &variant_iter);
    g_return_val_if_fail (retval, NULL);

    type = ibus_message_iter_get_arg_type (&variant_iter);

    if (type == IBUS_TYPE_STRUCT) {
        IBusSerializable *object;
        retval = ibus_message_iter_get (iter, IBUS_TYPE_SERIALIZABLE, &object);
        g_return_val_if_fail (retval, NULL);
        ibus_message_iter_next (iter);

        value = g_slice_new0 (GValue);
        g_value_init (value, G_OBJECT_TYPE (object));
        g_value_take_object (value, object);
        return value;
    }

    typedef gchar *gstring;
    switch (type) {
#define CASE_ENTRY(TYPE, _type)                                 \
    case G_TYPE_##TYPE:                                         \
        {                                                       \
            g##_type v;                                         \
            ibus_message_iter_get_basic (&variant_iter, &v);    \
            ibus_message_iter_next (&variant_iter);             \
            value = g_slice_new0 (GValue);                      \
            g_value_init (value, G_TYPE_##TYPE);                \
            g_value_set_##_type (value, v);                     \
            ibus_message_iter_next (iter);                      \
            return value;                                       \
        }
    CASE_ENTRY(CHAR, char);
    CASE_ENTRY(BOOLEAN, boolean);
    CASE_ENTRY(INT, int);
    CASE_ENTRY(UINT, uint);
    CASE_ENTRY(INT64, int64);
    CASE_ENTRY(UINT64, uint64);
    CASE_ENTRY(FLOAT, float);
    CASE_ENTRY(DOUBLE, double);
    CASE_ENTRY(STRING, string);
    }
    g_return_val_if_reached (NULL);
}

static void
_serialize_cb (GQuark           key,
               GValue          *value,
               IBusMessageIter *iter)
{
    IBusMessageIter dict_entry;
    gboolean retval;
    const gchar *name;

    retval = ibus_message_iter_open_container (iter,
                                               IBUS_TYPE_DICT_ENTRY,
                                               NULL,
                                               &dict_entry);
    g_return_if_fail (retval);
    name = g_quark_to_string (key);
    retval = ibus_message_iter_append (&dict_entry,
                                       G_TYPE_STRING,
                                       &name);
    g_return_if_fail (retval);

    retval = _g_value_serialize (value, &dict_entry);
    g_return_if_fail (retval);

    retval = ibus_message_iter_close_container (iter, &dict_entry);
    g_return_if_fail (retval);
}

static gboolean
ibus_serializable_real_serialize (IBusSerializable *object,
                                  IBusMessageIter  *iter)
{
    IBusSerializablePrivate *priv;
    IBusMessageIter array_iter;
    gboolean retval;

    priv = IBUS_SERIALIZABLE_GET_PRIVATE (object);

    retval = ibus_message_iter_open_container (iter,
                                               IBUS_TYPE_ARRAY,
                                               "{sv}",
                                               &array_iter);
    g_return_val_if_fail (retval, FALSE);

    g_datalist_foreach (&priv->attachments,
                        (GDataForeachFunc) _serialize_cb,
                        &array_iter);

    retval = ibus_message_iter_close_container (iter, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    return TRUE;
}

static gboolean
ibus_serializable_real_deserialize (IBusSerializable *object,
                                    IBusMessageIter  *iter)
{
    IBusMessageIter array_iter;
    gboolean retval;

    retval = ibus_message_iter_recurse (iter,
                                        IBUS_TYPE_ARRAY,
                                        &array_iter);
    g_return_val_if_fail (retval, FALSE);

    while (ibus_message_iter_get_arg_type (&array_iter) != G_TYPE_INVALID) {
        gchar *name;
        GValue *value;
        IBusMessageIter dict_entry;

        retval = ibus_message_iter_recurse (&array_iter,
                                            IBUS_TYPE_DICT_ENTRY,
                                            &dict_entry);
        g_return_val_if_fail (retval, FALSE);

        retval = ibus_message_iter_get (&dict_entry,
                                        G_TYPE_STRING,
                                        &name);
        g_return_val_if_fail (retval, FALSE);
        ibus_message_iter_next (&dict_entry);

        value = _g_value_deserialize (&dict_entry);
        g_return_val_if_fail (value != NULL, FALSE);

        ibus_serializable_set_attachment (object, name, value);

        ibus_message_iter_next (&array_iter);
    }

    ibus_message_iter_next (iter);

    return TRUE;
}

static void
_copy_cb (GQuark   key,
          GValue  *value,
          GData  **datalist)
{
    g_datalist_id_set_data_full (datalist,
                                 key,
                                 ibus_g_value_dup (value),
                                 (GDestroyNotify) ibus_g_value_free);
}

static gboolean
ibus_serializable_real_copy (IBusSerializable *dest,
                             const IBusSerializable *src)
{
    IBusSerializablePrivate *src_priv;
    IBusSerializablePrivate *dest_priv;
    src_priv = IBUS_SERIALIZABLE_GET_PRIVATE (src);
    dest_priv = IBUS_SERIALIZABLE_GET_PRIVATE (dest);

    g_datalist_foreach (&src_priv->attachments,
                        (GDataForeachFunc) _copy_cb,
                        &dest_priv->attachments);
    return TRUE;
}

gboolean
ibus_serializable_set_qattachment (IBusSerializable *object,
                                   GQuark            key,
                                   const GValue     *value)
{
    g_return_val_if_fail (IBUS_IS_SERIALIZABLE (object), FALSE);
    g_return_val_if_fail (key != 0, FALSE);
    g_return_val_if_fail (G_IS_VALUE (value), FALSE);

    IBusSerializablePrivate *priv;
    priv = IBUS_SERIALIZABLE_GET_PRIVATE (object);

    GType type = G_VALUE_TYPE (value);

    switch (type) {
    case G_TYPE_CHAR:
    case G_TYPE_INT:
    case G_TYPE_INT64:
    case G_TYPE_UINT:
    case G_TYPE_UINT64:
    case G_TYPE_BOOLEAN:
    case G_TYPE_DOUBLE:
    case G_TYPE_FLOAT:
    case G_TYPE_STRING:
        g_datalist_id_set_data_full (&priv->attachments,
                                      key,
                                      ibus_g_value_dup (value),
                                      (GDestroyNotify) ibus_g_value_free);
        return TRUE;
    }

    if (g_type_is_a (type, IBUS_TYPE_SERIALIZABLE)) {
        g_datalist_id_set_data_full (&priv->attachments,
                                      key,
                                      ibus_g_value_dup (value),
                                      (GDestroyNotify) ibus_g_value_free);
        return TRUE;
    }

    g_warning ("The value of %s is not support serializing", g_type_name (type));
    return FALSE;
}

const GValue *
ibus_serializable_get_qattachment (IBusSerializable *object,
                                   GQuark            key)
{

    g_return_val_if_fail (IBUS_IS_SERIALIZABLE (object), NULL);
    g_return_val_if_fail (key != 0, NULL);

    IBusSerializablePrivate *priv;
    priv = IBUS_SERIALIZABLE_GET_PRIVATE (object);

    return (const GValue *) g_datalist_id_get_data (&priv->attachments, key);
}

void
ibus_serializable_remove_qattachment (IBusSerializable *object,
                                      GQuark            key)
{

    g_return_if_fail (IBUS_IS_SERIALIZABLE (object));
    g_return_if_fail (key != 0);

    IBusSerializablePrivate *priv;
    priv = IBUS_SERIALIZABLE_GET_PRIVATE (object);

    g_datalist_id_remove_no_notify (&priv->attachments, key);
}

IBusSerializable *
ibus_serializable_copy (IBusSerializable *object)
{
    g_return_val_if_fail (IBUS_IS_SERIALIZABLE (object), NULL);

    GType type;
    IBusSerializable *new_object;

    type = G_OBJECT_TYPE (object);

    new_object = g_object_new (type, NULL);
    g_return_val_if_fail (new_object != NULL, NULL);

    if (IBUS_SERIALIZABLE_GET_CLASS (new_object)->copy (new_object, object)) {
        return new_object;
    }

    g_object_unref (new_object);
    g_return_val_if_reached (NULL);
}

gboolean
ibus_serializable_serialize (IBusSerializable *object,
                             IBusMessageIter  *iter)
{
    g_return_val_if_fail (IBUS_IS_SERIALIZABLE (object), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    IBusMessageIter variant_iter;
    IBusMessageIter sub_iter;
    gboolean retval;

    gchar *signature;

    signature = g_strdup_printf ("(s%s)", IBUS_SERIALIZABLE_GET_CLASS (object)->signature->str);
    retval = ibus_message_iter_open_container (iter,
                                               IBUS_TYPE_VARIANT,
                                               signature,
                                               &variant_iter);
    g_free (signature);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_open_container (&variant_iter,
                                               IBUS_TYPE_STRUCT,
                                               NULL,
                                               &sub_iter);
    g_return_val_if_fail (retval, FALSE);

    const gchar *type_name = g_type_name (G_OBJECT_TYPE (object));
    g_return_val_if_fail (type_name != NULL, FALSE);

    retval = ibus_message_iter_append (&sub_iter,
                                       G_TYPE_STRING,
                                       &type_name);
    g_return_val_if_fail (retval, FALSE);

    retval = IBUS_SERIALIZABLE_GET_CLASS (object)->serialize (object, &sub_iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_close_container (&variant_iter, &sub_iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_close_container (iter, &variant_iter);
    g_return_val_if_fail (retval, FALSE);

    return TRUE;
}

IBusSerializable *
ibus_serializable_deserialize (IBusMessageIter *iter)
{
    g_return_val_if_fail (iter != NULL, NULL);

    gboolean retval;
    IBusMessageIter variant_iter;
    IBusMessageIter sub_iter;
    gchar *type_name;
    GType type;
    IBusSerializable *object;

    type = ibus_message_iter_get_arg_type (iter);

    if (type == IBUS_TYPE_VARIANT) {
        retval = ibus_message_iter_recurse (iter, IBUS_TYPE_VARIANT, &variant_iter);
        g_return_val_if_fail (retval, NULL);

        retval = ibus_message_iter_recurse (&variant_iter, IBUS_TYPE_STRUCT, &sub_iter);
        g_return_val_if_fail (retval, NULL);
    }
    else if (type == IBUS_TYPE_STRUCT) {
        retval = ibus_message_iter_recurse (iter, IBUS_TYPE_STRUCT, &sub_iter);
        g_return_val_if_fail (retval, NULL);
    }
    else
        g_return_val_if_reached (NULL);

    retval = ibus_message_iter_get (&sub_iter, G_TYPE_STRING, &type_name);
    g_return_val_if_fail (retval, NULL);
    ibus_message_iter_next (&sub_iter);

    type = g_type_from_name (type_name);

    g_return_val_if_fail (g_type_is_a (type, IBUS_TYPE_SERIALIZABLE), NULL);

    object = g_object_new (type, NULL);

    retval = IBUS_SERIALIZABLE_GET_CLASS (object)->deserialize (object, &sub_iter);
    if (retval)
        return object;

    g_object_unref (object);
    g_return_val_if_reached (NULL);
}

