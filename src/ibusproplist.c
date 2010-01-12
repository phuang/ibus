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
#include "ibusproplist.h"

/* functions prototype */
static void         ibus_prop_list_class_init   (IBusPropListClass  *klass);
static void         ibus_prop_list_init         (IBusPropList       *prop_list);
static void         ibus_prop_list_destroy      (IBusPropList       *prop_list);
static gboolean     ibus_prop_list_serialize    (IBusPropList       *prop_list,
                                                 IBusMessageIter    *iter);
static gboolean     ibus_prop_list_deserialize  (IBusPropList       *prop_list,
                                                 IBusMessageIter    *iter);
static gboolean     ibus_prop_list_copy         (IBusPropList       *dest,
                                                 const IBusPropList *src);

static IBusSerializableClass *parent_class = NULL;

GType
ibus_prop_list_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusPropListClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_prop_list_class_init,
        NULL,               /* class finialize */
        NULL,               /* class data */
        sizeof (IBusProperty),
        0,
        (GInstanceInitFunc) ibus_prop_list_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERIALIZABLE,
                                       "IBusPropList",
                                       &type_info,
                                       0);
    }

    return type;
}

static void
ibus_prop_list_class_init (IBusPropListClass *klass)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (klass);

    parent_class = (IBusSerializableClass *) g_type_class_peek_parent (klass);

    object_class->destroy = (IBusObjectDestroyFunc) ibus_prop_list_destroy;

    serializable_class->serialize   = (IBusSerializableSerializeFunc) ibus_prop_list_serialize;
    serializable_class->deserialize = (IBusSerializableDeserializeFunc) ibus_prop_list_deserialize;
    serializable_class->copy        = (IBusSerializableCopyFunc) ibus_prop_list_copy;

    g_string_append (serializable_class->signature, "av");
}

static void
ibus_prop_list_init (IBusPropList *prop_list)
{
    prop_list->properties = g_array_new (TRUE, TRUE, sizeof (IBusProperty *));
}

static void
ibus_prop_list_destroy (IBusPropList *prop_list)
{
    IBusProperty **p;
    gint i;

    p = (IBusProperty **) g_array_free (prop_list->properties, FALSE);

    for (i = 0; p[i] != NULL; i++) {
        g_object_unref (p[i]);
    }
    g_free (p);

    IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *) prop_list);
}

static gboolean
ibus_prop_list_serialize (IBusPropList    *prop_list,
                          IBusMessageIter *iter)
{
    gboolean retval;
    IBusMessageIter array_iter;
    IBusProperty *prop;
    guint i;

    retval = parent_class->serialize ((IBusSerializable *) prop_list, iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_open_container (iter, IBUS_TYPE_ARRAY, "v", &array_iter);
    g_return_val_if_fail (retval, FALSE);

    i = 0;

    while ((prop = ibus_prop_list_get (prop_list, i)) != NULL) {
        retval = ibus_message_iter_append (&array_iter, IBUS_TYPE_PROPERTY, &prop);
        g_return_val_if_fail (retval, FALSE);
        i ++;
    }

    retval = ibus_message_iter_close_container (iter, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    return TRUE;
}

gboolean
ibus_prop_list_deserialize (IBusPropList    *prop_list,
                            IBusMessageIter *iter)
{
    gboolean retval;
    IBusMessageIter array_iter;
    IBusSerializable *object;

    retval = parent_class->deserialize ((IBusSerializable *) prop_list, iter);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_PROP_LIST (prop_list), FALSE);

    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_ARRAY, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    while (ibus_message_iter_get_arg_type (&array_iter) != G_TYPE_INVALID) {
        retval = ibus_message_iter_get (&array_iter, IBUS_TYPE_PROPERTY, &object);
        g_return_val_if_fail (retval, FALSE);
        ibus_message_iter_next (&array_iter);

        ibus_prop_list_append (prop_list, (IBusProperty *)object);
        g_object_unref (object);
    }

    ibus_message_iter_next (iter);

    return TRUE;
}



static gboolean
ibus_prop_list_copy (IBusPropList       *dest,
                     const IBusPropList *src)
{
    gboolean retval;
    IBusProperty *prop;
    guint i;

    retval = parent_class->copy ((IBusSerializable *) dest,
                                 (const IBusSerializable *) src);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_PROP_LIST (dest), FALSE);
    g_return_val_if_fail (IBUS_IS_PROP_LIST (src), FALSE);

    i = 0;
    while ((prop = ibus_prop_list_get ((IBusPropList *)src, i)) != NULL) {
        prop = (IBusProperty *) ibus_serializable_copy ((IBusSerializable *) prop);
        ibus_prop_list_append (dest, prop);
        g_object_unref (prop);
        i ++;
    }
    return TRUE;
}


IBusPropList *
ibus_prop_list_new ()
{
    IBusPropList *prop_list;

    prop_list = g_object_new (IBUS_TYPE_PROP_LIST, NULL);

    return prop_list;
}

void
ibus_prop_list_append (IBusPropList *prop_list,
                       IBusProperty *prop)
{
    g_assert (IBUS_IS_PROP_LIST (prop_list));
    g_assert (IBUS_IS_PROPERTY (prop));

    g_object_ref (prop);

    g_array_append_val (prop_list->properties, prop);
}

IBusProperty *
ibus_prop_list_get (IBusPropList *prop_list,
                    guint         index)
{
    g_assert (IBUS_IS_PROP_LIST (prop_list));


    if (index >= prop_list->properties->len)
        return NULL;

    return g_array_index (prop_list->properties, IBusProperty *, index);
}



gboolean
ibus_prop_list_update_property (IBusPropList *prop_list,
                                IBusProperty *prop_update)
{
    g_assert (IBUS_IS_PROP_LIST (prop_list));
    g_assert (IBUS_IS_PROPERTY (prop_update));

    gint i;

    for (i = 0; i < prop_list->properties->len; i ++) {
        IBusProperty *prop = g_array_index (prop_list->properties, IBusProperty *, i);
        if (ibus_property_update (prop, prop_update)) {
            return TRUE;
        }
    }

    return FALSE;
}
