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
#include <dbus/dbus.h>
#include "ibusattrlist.h"

/* functions prototype */
static void         ibus_attr_list_destroy      (IBusAttrList           *attr_list);
static gboolean     ibus_attr_list_serialize    (IBusAttrList           *attr_list,
                                                 IBusMessageIter        *iter);
static gboolean     ibus_attr_list_deserialize  (IBusAttrList           *attr_list,
                                                 IBusMessageIter        *iter);
static gboolean     ibus_attr_list_copy         (IBusAttrList           *dest,
                                                 const IBusAttrList     *src);

G_DEFINE_TYPE (IBusAttrList, ibus_attr_list, IBUS_TYPE_SERIALIZABLE)

static void
ibus_attr_list_class_init (IBusAttrListClass *klass)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (klass);

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

    IBUS_OBJECT_CLASS (ibus_attr_list_parent_class)->destroy ((IBusObject *)attr_list);
}

static gboolean
ibus_attr_list_serialize (IBusAttrList    *attr_list,
                          IBusMessageIter *iter)
{
    IBusMessageIter array_iter;
    gboolean retval;
    guint i;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_attr_list_parent_class)->serialize ((IBusSerializable *)attr_list, iter);
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

    retval = IBUS_SERIALIZABLE_CLASS (ibus_attr_list_parent_class)->deserialize ((IBusSerializable *)attr_list, iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_ARRAY, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    while (ibus_message_iter_get_arg_type (&array_iter) != G_TYPE_INVALID) {
        IBusAttribute *attr;

        retval = ibus_message_iter_get (&array_iter, IBUS_TYPE_ATTRIBUTE, &attr);
        g_return_val_if_fail (retval, FALSE);
        ibus_message_iter_next (&array_iter);

        ibus_attr_list_append (attr_list, attr);
    }

    ibus_message_iter_next (iter);

    return TRUE;
}



static gboolean
ibus_attr_list_copy (IBusAttrList       *dest,
                     const IBusAttrList *src)
{
    gboolean retval;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_attr_list_parent_class)->copy ((IBusSerializable *)dest,
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


