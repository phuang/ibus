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
#include "ibusinternal.h"
#include "ibusserializable.h"

#define IBUS_SERIALIZABLE_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_SERIALIZABLE, IBusSerializablePrivate))

enum {
    LAST_SIGNAL,
};

struct _IBusSerializablePrivate {
    GData   *attachments;
};

// static guint    object_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void      ibus_serializable_base_init        (IBusSerializableClass  *class);
static void      ibus_serializable_base_fini        (IBusSerializableClass  *class);
static void      ibus_serializable_class_init       (IBusSerializableClass  *class);
static void      ibus_serializable_init             (IBusSerializable       *object);
static void      ibus_serializable_destroy          (IBusSerializable       *object);
static gboolean  ibus_serializable_real_serialize   (IBusSerializable       *object,
                                                     GVariantBuilder        *builder);
static gint      ibus_serializable_real_deserialize (IBusSerializable       *object,
                                                     GVariant               *variant);
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
ibus_serializable_base_init     (IBusSerializableClass *class)
{
}

static void
ibus_serializable_base_fini     (IBusSerializableClass *class)
{
}

static void
ibus_serializable_class_init     (IBusSerializableClass *class)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (class);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (class);

    g_type_class_add_private (class, sizeof (IBusSerializablePrivate));

    object_class->destroy = (IBusObjectDestroyFunc) ibus_serializable_destroy;

    class->serialize = ibus_serializable_real_serialize;
    class->deserialize = ibus_serializable_real_deserialize;
    class->copy = ibus_serializable_real_copy;
}

static void
ibus_serializable_init (IBusSerializable *serializable)
{
    serializable->priv = IBUS_SERIALIZABLE_GET_PRIVATE (serializable);
    serializable->priv->attachments = NULL;
    g_datalist_init (&serializable->priv->attachments);
}

static void
ibus_serializable_destroy (IBusSerializable *serializable)
{
    g_datalist_clear (&serializable->priv->attachments);
    parent_class->destroy (IBUS_OBJECT (serializable));
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

static GVariant *
_g_value_serialize (GValue          *value)
{
    GType type;

    type = G_VALUE_TYPE (value);
    g_return_val_if_fail (type != G_TYPE_INVALID, FALSE);

    if (g_type_is_a (type, IBUS_TYPE_SERIALIZABLE)) {
        IBusSerializable *object;
        object = IBUS_SERIALIZABLE (g_value_get_object (value));
        return ibus_serializable_serialize (object);
    }

    typedef const gchar *gstring;
    switch (type) {
#define CASE_ENTRY(TYPE, _type, signature)                              \
    case G_TYPE_##TYPE:                                                 \
        {                                                               \
            g##_type v;                                                 \
            v = g_value_get_##_type (value);                            \
            return g_variant_new ("v", g_variant_new (signature, v));   \
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

    g_assert_not_reached ();
}

static GValue *
_g_value_deserialize (GVariant *variant)
{
    GValue *value = NULL;
    const GVariantType *type;

    type = g_variant_get_type (variant);
    if (type == G_VARIANT_TYPE_TUPLE) {
        IBusSerializable *object;
        object = ibus_serializable_deserialize (variant);
        value = g_slice_new0 (GValue);
        g_value_init (value, G_OBJECT_TYPE (object));
        g_value_take_object (value, object);
        return value;
    }

    typedef gchar *gstring;
#define IF_ENTRY(TYPE, _type, signature)                    \
    if (type ==  G_VARIANT_TYPE_##TYPE) {                   \
        g##_type v;                                         \
        g_variant_get (variant, signature, &v);             \
        value = g_slice_new0 (GValue);                      \
        g_value_init (value, G_TYPE_##TYPE);                \
        g_value_set_##_type (value, v);                     \
        return value;                                       \
    }
#define G_VARIANT_TYPE_CHAR G_VARIANT_TYPE_BYTE
    IF_ENTRY(CHAR, char, "y");
#undef G_VARIANT_TYPE_CHAR
    IF_ENTRY(BOOLEAN, boolean, "b");
#define G_VARIANT_TYPE_INT G_VARIANT_TYPE_INT32
#define G_VARIANT_TYPE_UINT G_VARIANT_TYPE_UINT32
    IF_ENTRY(INT, int, "i");
    IF_ENTRY(UINT, uint, "u");
#undef G_VARIANT_TYPE_INT
#undef G_VARIANT_TYPE_UINT
    IF_ENTRY(INT64, int64, "x");
    IF_ENTRY(UINT64, uint64, "t");
    IF_ENTRY(DOUBLE, double, "d");
    IF_ENTRY(STRING, string, "s");

    g_return_val_if_reached (NULL);
}

static void
_serialize_cb (GQuark           key,
               GValue          *value,
               GVariantBuilder *array)
{
    g_variant_builder_add (array, "{sv}",
                g_quark_to_string (key), _g_value_serialize (value));
}

static gboolean
ibus_serializable_real_serialize (IBusSerializable *serializable,
                                  GVariantBuilder  *builder)
{
    GVariantBuilder array;
    g_variant_builder_init (&array, G_VARIANT_TYPE ("a{sv}"));

    g_datalist_foreach (&serializable->priv->attachments,
                        (GDataForeachFunc) _serialize_cb,
                        &array);
    g_variant_builder_add (builder, "a{sv}", &array);
    return TRUE;
}

static gint
ibus_serializable_real_deserialize (IBusSerializable *object,
                                    GVariant         *variant)
{
    const gchar *key;
    GVariant *value;
    GVariantIter *iter = NULL;
    g_variant_get_child (variant, 1, "a{sv}", &iter);
    while (g_variant_iter_loop (iter, "{&sv}", &key, &value)) {
        ibus_serializable_set_attachment (object, key, _g_value_deserialize (value));
    }
    g_variant_iter_free (iter);
    return 2;
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
ibus_serializable_get_qattachment (IBusSerializable *serializable,
                                   GQuark            key)
{

    g_return_val_if_fail (IBUS_IS_SERIALIZABLE (serializable), NULL);
    g_return_val_if_fail (key != 0, NULL);

    return (const GValue *) g_datalist_id_get_data (&serializable->priv->attachments, key);
}

void
ibus_serializable_remove_qattachment (IBusSerializable *serializable,
                                      GQuark            key)
{

    g_return_if_fail (IBUS_IS_SERIALIZABLE (serializable));
    g_return_if_fail (key != 0);

    g_datalist_id_remove_no_notify (&serializable->priv->attachments, key);
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

GVariant *
ibus_serializable_serialize (IBusSerializable *object)
{
    g_return_val_if_fail (IBUS_IS_SERIALIZABLE (object), FALSE);
    gboolean retval;

    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE_TUPLE);

    g_variant_builder_add (&builder, "s", g_type_name (G_OBJECT_TYPE (object)));
    retval = IBUS_SERIALIZABLE_GET_CLASS (object)->serialize (object, &builder);
    g_assert (retval);

    return g_variant_builder_end (&builder);
}

IBusSerializable *
ibus_serializable_deserialize (GVariant *variant)
{
    g_return_val_if_fail (variant != NULL, NULL);

    GVariant *var = NULL;
    switch (g_variant_classify (variant)) {
    case G_VARIANT_CLASS_VARIANT:
        var = g_variant_get_variant (variant);
        break;
    case G_VARIANT_CLASS_TUPLE:
        var = g_variant_ref (variant);
        break;
    default:
        g_return_val_if_reached (NULL);
    }

    gchar *type_name = NULL;
    g_variant_get_child (var, 0, "&s", &type_name);
    GType type = g_type_from_name (type_name);

    g_return_val_if_fail (g_type_is_a (type, IBUS_TYPE_SERIALIZABLE), NULL);

    IBusSerializable *object = g_object_new (type, NULL);

    gint retval = IBUS_SERIALIZABLE_GET_CLASS (object)->deserialize (object, var);
    g_variant_unref (var);
    if (retval)
        return object;

    g_object_unref (object);
    g_return_val_if_reached (NULL);
}

