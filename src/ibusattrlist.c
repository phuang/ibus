/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* IBus - The Input Bus
 * Copyright (C) 2008-2013 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2013 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
#include "ibusattrlist.h"

/* functions prototype */
static void         ibus_attr_list_destroy      (IBusAttrList           *attr_list);
static gboolean     ibus_attr_list_serialize    (IBusAttrList           *attr_list,
                                                 GVariantBuilder        *builder);
static gint         ibus_attr_list_deserialize  (IBusAttrList           *attr_list,
                                                 GVariant               *variant);
static gboolean     ibus_attr_list_copy         (IBusAttrList           *dest,
                                                 const IBusAttrList     *src);

G_DEFINE_TYPE (IBusAttrList, ibus_attr_list, IBUS_TYPE_SERIALIZABLE)

static void
ibus_attr_list_class_init (IBusAttrListClass *class)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (class);
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (class);

    object_class->destroy = (IBusObjectDestroyFunc) ibus_attr_list_destroy;

    serializable_class->serialize   = (IBusSerializableSerializeFunc) ibus_attr_list_serialize;
    serializable_class->deserialize = (IBusSerializableDeserializeFunc) ibus_attr_list_deserialize;
    serializable_class->copy        = (IBusSerializableCopyFunc) ibus_attr_list_copy;
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

    IBUS_OBJECT_CLASS (ibus_attr_list_parent_class)->destroy ((IBusObject *)attr_list);
}

static gboolean
ibus_attr_list_serialize (IBusAttrList    *attr_list,
                          GVariantBuilder *builder)
{
    gboolean retval;
    guint i;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_attr_list_parent_class)->serialize ((IBusSerializable *)attr_list, builder);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_ATTR_LIST (attr_list), FALSE);

    GVariantBuilder array;
    g_variant_builder_init (&array, G_VARIANT_TYPE ("av"));

    for (i = 0;; i++) {
        IBusAttribute *attr;
        attr = ibus_attr_list_get (attr_list, i);
        if (attr == NULL)
            break;
        g_variant_builder_add (&array, "v", ibus_serializable_serialize ((IBusSerializable *)attr));
    }
    g_variant_builder_add (builder, "av", &array);

    return TRUE;
}

static gint
ibus_attr_list_deserialize (IBusAttrList    *attr_list,
                            GVariant        *variant)
{
    gint retval = IBUS_SERIALIZABLE_CLASS (ibus_attr_list_parent_class)->deserialize ((IBusSerializable *)attr_list, variant);
    g_return_val_if_fail (retval, 0);

    GVariantIter *iter = NULL;
    g_variant_get_child (variant, retval++, "av", &iter);
    GVariant *var;
    while (g_variant_iter_loop (iter, "v", &var)) {
        IBusAttribute *attr = IBUS_ATTRIBUTE (ibus_serializable_deserialize (var));
        ibus_attr_list_append (attr_list, attr);
    }
    g_variant_iter_free (iter);

    return retval;
}



static gboolean
ibus_attr_list_copy (IBusAttrList       *dest,
                     const IBusAttrList *src)
{
    gboolean retval;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_attr_list_parent_class)->copy ((IBusSerializable *)dest,
                                 (IBusSerializable *)src);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_ATTR_LIST (dest), FALSE);
    g_return_val_if_fail (IBUS_IS_ATTR_LIST (src), FALSE);

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

    g_object_ref_sink (attr);
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


