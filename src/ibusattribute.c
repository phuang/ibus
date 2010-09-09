/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
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
// static void         ibus_attribute_destroy      (IBusAttribute          *attr);
static gboolean     ibus_attribute_serialize    (IBusAttribute          *attr,
                                                 IBusMessageIter        *iter);
static gboolean     ibus_attribute_deserialize  (IBusAttribute          *attr,
                                                 IBusMessageIter        *iter);
static gboolean     ibus_attribute_copy         (IBusAttribute          *dest,
                                                 const IBusAttribute    *src);

G_DEFINE_TYPE (IBusAttribute, ibus_attribute, IBUS_TYPE_SERIALIZABLE)

static void
ibus_attribute_class_init (IBusAttributeClass *klass)
{
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (klass);

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
//     IBUS_OBJECT (ibus_attribute_parent_class)->destroy ((IBusObject *)attr);
// }

static gboolean
ibus_attribute_serialize (IBusAttribute   *attr,
                          IBusMessageIter *iter)
{
    gboolean retval;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_attribute_parent_class)->serialize ((IBusSerializable *) attr, iter);
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

    retval = IBUS_SERIALIZABLE_CLASS (ibus_attribute_parent_class)->deserialize ((IBusSerializable *) attr, iter);
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

    retval = IBUS_SERIALIZABLE_CLASS (ibus_attribute_parent_class)->copy ((IBusSerializable *)dest,
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


