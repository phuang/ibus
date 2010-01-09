/* vim:set et sts=4: */
/* IBus - The Input Bus
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
#include "ibusattribute.h"

/* functions prototype */
static void         ibus_attribute_class_init   (IBusAttributeClass     *klass);
static void         ibus_attribute_init         (IBusAttribute          *attr);
// static void         ibus_attribute_destroy      (IBusAttribute          *attr);
static gboolean     ibus_attribute_serialize    (IBusAttribute          *attr,
                                                 IBusMessageIter        *iter);
static gboolean     ibus_attribute_deserialize  (IBusAttribute          *attr,
                                                 IBusMessageIter        *iter);
static gboolean     ibus_attribute_copy         (IBusAttribute          *dest,
                                                 const IBusAttribute    *src);

static void         ibus_attr_list_class_init   (IBusAttrListClass      *klass);
static void         ibus_attr_list_init         (IBusAttrList           *attr_list);
static void         ibus_attr_list_destroy      (IBusAttrList           *attr_list);
static gboolean     ibus_attr_list_serialize    (IBusAttrList           *attr_list,
                                                 IBusMessageIter        *iter);
static gboolean     ibus_attr_list_deserialize  (IBusAttrList           *attr_list,
                                                 IBusMessageIter        *iter);
static gboolean     ibus_attr_list_copy         (IBusAttrList           *dest,
                                                 const IBusAttrList     *src);

static IBusSerializableClass *parent_class = NULL;

GType
ibus_attribute_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusAttributeClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_attribute_class_init,
        NULL,               /* class finialize */
        NULL,               /* class data */
        sizeof (IBusAttribute),
        0,
        (GInstanceInitFunc) ibus_attribute_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERIALIZABLE,
                                       "IBusAttribute",
                                       &type_info,
                                       0);
    }

    return type;
}

static void
ibus_attribute_class_init (IBusAttributeClass *klass)
{
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (klass);

    parent_class = (IBusSerializableClass *) g_type_class_peek_parent (klass);

    // object_class->destroy = (IBusObjectDestroyFunc) ibus_attribute_destroy;

    serializable_class->serialize   = (IBusSerializableSerializeFunc) ibus_attribute_serialize;
    serializable_class->deserialize = (IBusSerializableDeserializeFunc) ibus_attribute_deserialize;
    serializable_class->copy        = (IBusSerializableCopyFunc) ibus_attribute_copy;

    g_string_append (serializable_class->signature, "uuuu");
}

static void
ibus_attribute_init (IBusAttribute *attr)
{
}

// static void
// ibus_attribute_destroy (IBusAttribute *attr)
// {
//     IBUS_OBJECT (parent_class)->destroy ((IBusObject *)attr);
// }

static gboolean
ibus_attribute_serialize (IBusAttribute   *attr,
                          IBusMessageIter *iter)
{
    gboolean retval;

    retval = parent_class->serialize ((IBusSerializable *) attr, iter);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_ATTRIBUTE (attr), FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_UINT, &attr->type);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_UINT, &attr->value);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_UINT, &attr->start_index);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_UINT, &attr->end_index);
    g_return_val_if_fail (retval, FALSE);

    return TRUE;
}

static gboolean
ibus_attribute_deserialize (IBusAttribute   *attr,
                            IBusMessageIter *iter)
{
    gboolean retval;

    retval = parent_class->deserialize ((IBusSerializable *) attr, iter);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_ATTRIBUTE (attr), FALSE);

    retval = ibus_message_iter_get (iter, G_TYPE_UINT, &attr->type);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);

    retval = ibus_message_iter_get (iter, G_TYPE_UINT, &attr->value);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);

    retval = ibus_message_iter_get (iter, G_TYPE_UINT, &attr->start_index);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);

    retval = ibus_message_iter_get (iter, G_TYPE_UINT, &attr->end_index);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);

    return TRUE;

}

static gboolean
ibus_attribute_copy (IBusAttribute       *dest,
                     const IBusAttribute *src)
{
    gboolean retval;

    retval = parent_class->copy ((IBusSerializable *)dest,
                                 (IBusSerializable *)src);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_ATTRIBUTE (dest), FALSE);
    g_return_val_if_fail (IBUS_IS_ATTRIBUTE (src), FALSE);

    dest->type  = src->type;
    dest->value = src->value;
    dest->start_index = src->start_index;
    dest->end_index = src->end_index;

    return TRUE;
}

IBusAttribute *
ibus_attribute_new (guint type,
                    guint value,
                    guint start_index,
                    guint end_index)
{
    g_return_val_if_fail (
        type == IBUS_ATTR_TYPE_UNDERLINE  ||
        type == IBUS_ATTR_TYPE_FOREGROUND ||
        type == IBUS_ATTR_TYPE_BACKGROUND, NULL);

    IBusAttribute *attr = IBUS_ATTRIBUTE (g_object_new (IBUS_TYPE_ATTRIBUTE, NULL));

    attr->type = type;
    attr->value = value;
    attr->start_index = start_index;
    attr->end_index = end_index;

    return attr;
}

IBusAttribute *
ibus_attr_underline_new (guint underline_type,
                         guint start_index,
                         guint end_index)
{
    g_return_val_if_fail (
        underline_type == IBUS_ATTR_UNDERLINE_NONE   ||
        underline_type == IBUS_ATTR_UNDERLINE_SINGLE ||
        underline_type == IBUS_ATTR_UNDERLINE_DOUBLE ||
        underline_type == IBUS_ATTR_UNDERLINE_LOW, NULL);

    return ibus_attribute_new (IBUS_ATTR_TYPE_UNDERLINE,
                               underline_type,
                               start_index,
                               end_index);
}

IBusAttribute *
ibus_attr_foreground_new (guint color,
                          guint start_index,
                          guint end_index)
{
    return ibus_attribute_new (IBUS_ATTR_TYPE_FOREGROUND,
                               color,
                               start_index,
                               end_index);
}

IBusAttribute *
ibus_attr_background_new (guint color,
                          guint start_index,
                          guint end_index)
{
    return ibus_attribute_new (IBUS_ATTR_TYPE_BACKGROUND,
                               color,
                               start_index,
                               end_index);
}

GType
ibus_attr_list_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusAttrListClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_attr_list_class_init,
        NULL,               /* class finialize */
        NULL,               /* class data */
        sizeof (IBusAttrList),
        0,
        (GInstanceInitFunc) ibus_attr_list_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERIALIZABLE,
                                       "IBusAttrList",
                                       &type_info,
                                       0);
    }

    return type;
}

static void
ibus_attr_list_class_init (IBusAttrListClass *klass)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (klass);

    parent_class = (IBusSerializableClass *) g_type_class_peek_parent (klass);

    object_class->destroy = (IBusObjectDestroyFunc) ibus_attr_list_destroy;

    serializable_class->serialize   = (IBusSerializableSerializeFunc) ibus_attr_list_serialize;
    serializable_class->deserialize = (IBusSerializableDeserializeFunc) ibus_attr_list_deserialize;
    serializable_class->copy        = (IBusSerializableCopyFunc) ibus_attr_list_copy;

    g_string_append (serializable_class->signature, "av");
}

static void
ibus_attr_list_init (IBusAttrList *attr_list)
{
    attr_list->attributes = g_array_new (TRUE, TRUE, sizeof (IBusAttribute *));
}

static void
ibus_attr_list_destroy (IBusAttrList *attr_list)
{
    g_assert (IBUS_IS_ATTR_LIST (attr_list));

    gint i;
    for (i = 0;; i++) {
        IBusAttribute *attr;

        attr = ibus_attr_list_get (attr_list, i);
        if (attr == NULL)
            break;

        g_object_unref (attr);
    }

    g_array_free (attr_list->attributes, TRUE);

    IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *)attr_list);
}

static gboolean
ibus_attr_list_serialize (IBusAttrList    *attr_list,
                          IBusMessageIter *iter)
{
    IBusMessageIter array_iter;
    gboolean retval;
    guint i;

    retval = parent_class->serialize ((IBusSerializable *)attr_list, iter);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_ATTR_LIST (attr_list), FALSE);

    retval = ibus_message_iter_open_container (iter,
                                               IBUS_TYPE_ARRAY,
                                               "v",
                                               &array_iter);
    g_return_val_if_fail (retval, FALSE);

    for (i = 0;; i++) {
        IBusAttribute *attr;

        attr = ibus_attr_list_get (attr_list, i);
        if (attr == NULL)
            break;

        retval = ibus_message_iter_append (&array_iter, IBUS_TYPE_ATTRIBUTE, &attr);
        g_return_val_if_fail (retval, FALSE);
    }

    retval = ibus_message_iter_close_container (iter, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    return TRUE;
}

static gboolean
ibus_attr_list_deserialize (IBusAttrList    *attr_list,
                            IBusMessageIter *iter)
{
    DBusMessageIter array_iter;
    gboolean retval;

    retval = parent_class->deserialize ((IBusSerializable *)attr_list, iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_ARRAY, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    while (ibus_message_iter_get_arg_type (&array_iter) != G_TYPE_INVALID) {
        IBusAttribute *attr;

        retval = ibus_message_iter_get (&array_iter, IBUS_TYPE_ATTRIBUTE, &attr);
        g_return_val_if_fail (retval, FALSE);
        ibus_message_iter_next (&array_iter);

        ibus_attr_list_append (attr_list, attr);
        g_object_unref (attr);
    }

    ibus_message_iter_next (iter);

    return TRUE;
}



static gboolean
ibus_attr_list_copy (IBusAttrList       *dest,
                     const IBusAttrList *src)
{
    gboolean retval;

    retval = parent_class->copy ((IBusSerializable *)dest,
                                 (IBusSerializable *)src);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_ATTRIBUTE (dest), FALSE);
    g_return_val_if_fail (IBUS_IS_ATTRIBUTE (src), FALSE);

    gint i;
    for (i = 0; ; i++) {
        IBusAttribute *attr = ibus_attr_list_get ((IBusAttrList *)src, i);
        if (attr == NULL) {
            break;
        }

        attr = (IBusAttribute *) ibus_serializable_copy ((IBusSerializable *) attr);
        if (attr == NULL) {
            g_warning ("can not copy attribute");
            continue;
        }

        ibus_attr_list_append (dest, attr);
    }
    return TRUE;
}

IBusAttrList *
ibus_attr_list_new ()
{
    IBusAttrList *attr_list;
    attr_list = g_object_new (IBUS_TYPE_ATTR_LIST, NULL);
    return attr_list;
}

void
ibus_attr_list_append (IBusAttrList  *attr_list,
                       IBusAttribute *attr)
{
    g_assert (IBUS_IS_ATTR_LIST (attr_list));
    g_assert (IBUS_IS_ATTRIBUTE (attr));

    g_object_ref (attr);
    g_array_append_val (attr_list->attributes, attr);
}

IBusAttribute *
ibus_attr_list_get (IBusAttrList *attr_list,
                    guint         index)
{
    g_assert (IBUS_IS_ATTR_LIST (attr_list));
    IBusAttribute *attr = NULL;

    if (index < attr_list->attributes->len) {
        attr = g_array_index (attr_list->attributes, IBusAttribute *, index);
    }

    return attr;
}


