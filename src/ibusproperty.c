/* vim:set et sts=4: */
/* IBus - The Input Bus
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
#include "ibusproperty.h"

/* functions prototype */
static void         ibus_property_class_init    (IBusPropertyClass  *klass);
static void         ibus_property_init          (IBusProperty       *prop);
static void         ibus_property_destroy       (IBusProperty       *prop);
static gboolean     ibus_property_serialize     (IBusProperty       *prop,
                                                 IBusMessageIter    *iter);
static gboolean     ibus_property_deserialize   (IBusProperty       *prop,
                                                 IBusMessageIter    *iter);
static gboolean     ibus_property_copy          (IBusProperty       *dest,
                                                 const IBusProperty *src);

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
ibus_property_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusPropertyClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_property_class_init,
        NULL,               /* class finialize */
        NULL,               /* class data */
        sizeof (IBusProperty),
        0,
        (GInstanceInitFunc) ibus_property_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERIALIZABLE,
                                       "IBusProperty",
                                       &type_info,
                                       0);
    }

    return type;
}

static void
ibus_property_class_init (IBusPropertyClass *klass)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (klass);

    parent_class = (IBusSerializableClass *) g_type_class_peek_parent (klass);

    object_class->destroy = (IBusObjectDestroyFunc) ibus_property_destroy;

    serializable_class->serialize   = (IBusSerializableSerializeFunc) ibus_property_serialize;
    serializable_class->deserialize = (IBusSerializableDeserializeFunc) ibus_property_deserialize;
    serializable_class->copy        = (IBusSerializableCopyFunc) ibus_property_copy;

    g_string_append (serializable_class->signature, "suvsvbbuv");
}

static void
ibus_property_init (IBusProperty *prop)
{
    prop->key = NULL;
    prop->type = 0;
    prop->label = NULL;
    prop->icon = NULL;
    prop->tooltip = NULL;
    prop->sensitive = FALSE;
    prop->visible = FALSE;
    prop->state = 0;
}

static void
ibus_property_destroy (IBusProperty *prop)
{
    g_free (prop->key);
    g_free (prop->icon);

    prop->key = NULL;
    prop->icon = NULL;

    if (prop->label) {
        g_object_unref (prop->label);
        prop->label = NULL;
    }

    if (prop->tooltip) {
        g_object_unref (prop->tooltip);
        prop->tooltip = NULL;
    }

    IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *)prop);
}

gboolean
ibus_property_serialize (IBusProperty    *prop,
                         IBusMessageIter *iter)
{
    gboolean retval;

    retval = parent_class->serialize ((IBusSerializable *) prop, iter);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_PROPERTY (prop), FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &prop->key);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_UINT, &prop->type);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, IBUS_TYPE_TEXT, &prop->label);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &prop->icon);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, IBUS_TYPE_TEXT, &prop->tooltip);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_BOOLEAN, &prop->sensitive);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_BOOLEAN, &prop->visible);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_UINT, &prop->state);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, IBUS_TYPE_PROP_LIST, &prop->sub_props);
    g_return_val_if_fail (retval, FALSE);

    return TRUE;
}

static gboolean
ibus_property_deserialize (IBusProperty    *prop,
                           IBusMessageIter *iter)
{
    gboolean retval;
    gchar *p;

    retval = parent_class->deserialize ((IBusSerializable *) prop, iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &p);
    g_return_val_if_fail (retval, FALSE);
    prop->key = g_strdup (p);

    retval = ibus_message_iter_get (iter, G_TYPE_UINT, &prop->type);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_get (iter, IBUS_TYPE_TEXT, &prop->label);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &p);
    g_return_val_if_fail (retval, FALSE);
    prop->icon = g_strdup (p);

    retval = ibus_message_iter_get (iter, IBUS_TYPE_TEXT, &prop->tooltip);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_get (iter, G_TYPE_BOOLEAN, &prop->sensitive);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_get (iter, G_TYPE_BOOLEAN, &prop->visible);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_get (iter, G_TYPE_UINT, &prop->state);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_get (iter, IBUS_TYPE_PROP_LIST, &prop->sub_props);
    g_return_val_if_fail (retval, FALSE);

    return TRUE;
}

static gboolean
ibus_property_copy (IBusProperty       *dest,
                    const IBusProperty *src)
{
    gboolean retval;

    retval = parent_class->copy ((IBusSerializable *) dest, (IBusSerializable *) src);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_PROPERTY (dest), FALSE);
    g_return_val_if_fail (IBUS_IS_PROPERTY (src), FALSE);

    dest->key = g_strdup (src->key);
    dest->icon = g_strdup (src->icon);
    if (src->label) {
        dest->label = (IBusText *) ibus_serializable_copy ((IBusSerializable *) src->label);
    }
    else
        dest->label = ibus_text_new_from_static_string ("");
    if (src->tooltip) {
        dest->tooltip = (IBusText *) ibus_serializable_copy ((IBusSerializable *) src->tooltip);
    }
    else
        dest->tooltip = ibus_text_new_from_static_string ("");

    dest->sensitive = src->sensitive;
    dest->visible = src->visible;
    dest->type = src->type;
    dest->state = src->state;

    dest->sub_props = (IBusPropList *) ibus_serializable_copy ((IBusSerializable *) src->sub_props);

    return TRUE;
}

IBusProperty *
ibus_property_new (const gchar   *key,
                   IBusPropType   type,
                   IBusText      *label,
                   const gchar   *icon,
                   IBusText      *tooltip,
                   gboolean       sensitive,
                   gboolean       visible,
                   IBusPropState  state,
                   IBusPropList  *prop_list)
{
    g_return_val_if_fail (key != NULL, NULL);
    g_return_val_if_fail (type >= PROP_TYPE_NORMAL &&
                          type <= PROP_TYPE_SEPARATOR,
                          NULL);
    g_return_val_if_fail (label == NULL || IBUS_IS_TEXT (label), NULL);
    g_return_val_if_fail (tooltip == NULL || IBUS_IS_TEXT (tooltip), NULL);
    g_return_val_if_fail (state == PROP_STATE_UNCHECKED ||
                          state == PROP_STATE_CHECKED ||
                          state == PROP_STATE_INCONSISTENT,
                          NULL);

    IBusProperty *prop;

    prop = (IBusProperty *)g_object_new (IBUS_TYPE_PROPERTY, NULL);

    prop->key = g_strdup (key);
    prop->icon = g_strdup (icon != NULL ? icon : "");
    prop->type = type;

    if (label)
        prop->label = (IBusText *) g_object_ref (label);
    else
        prop->label = ibus_text_new_from_static_string ("");

    if (tooltip)
        prop->tooltip = (IBusText *) g_object_ref (tooltip);
    else
        prop->tooltip = ibus_text_new_from_static_string ("");

    prop->sensitive = sensitive;
    prop->visible = visible;
    prop->state = state;

    if (prop_list)
        prop->sub_props = g_object_ref (prop_list);
    else
        prop->sub_props = ibus_prop_list_new ();

    return prop;
}

void
ibus_property_set_label (IBusProperty *prop,
                         IBusText     *label)
{
    g_assert (IBUS_IS_PROPERTY (prop));

    if (prop->label) {
        g_object_unref (prop->label);
    }

    if (label == NULL) {
        label = ibus_text_new_from_static_string ("");
    }

    prop->label = g_object_ref (label);
}

void
ibus_property_set_visible (IBusProperty *prop,
                           gboolean      visible)
{
    g_assert (IBUS_IS_PROPERTY (prop));
    prop->visible = visible;
}

void
ibus_property_set_sub_props (IBusProperty *prop,
                             IBusPropList *prop_list)
{
    g_assert (IBUS_IS_PROPERTY (prop));
    g_assert (IBUS_IS_PROP_LIST (prop_list) || prop_list == NULL);

    if (prop->sub_props) {
        g_object_unref (prop->sub_props);
    }

    if (prop_list) {
        g_object_ref (prop_list);
        prop->sub_props = prop_list;
    }
    else
        prop->sub_props = ibus_prop_list_new ();
}

gboolean
ibus_property_update (IBusProperty *prop,
                      IBusProperty *prop_update)
{
    g_assert (IBUS_IS_PROPERTY (prop));
    g_assert (IBUS_IS_PROPERTY (prop_update));

    if (g_strcmp0 (prop->key, prop_update->key) != 0) {
        return ibus_prop_list_update_property (prop->sub_props, prop_update);
    }

    g_free (prop->icon);
    prop->icon = g_strdup (prop_update->icon);

    if (prop->label) {
        g_object_unref (prop->label);
    }
    prop->label = (IBusText *) g_object_ref (prop_update->label);

    if (prop->tooltip) {
        g_object_unref (prop->tooltip);
    }
    prop->tooltip = (IBusText *) g_object_ref (prop_update->tooltip);
    prop->visible = prop_update->visible;
    prop->state = prop_update->state;
    prop->sensitive = prop_update->sensitive;

    return TRUE;
}

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
    IBusProperty **ps, **p;
    p = ps = (IBusProperty **) g_array_free (prop_list->properties, FALSE);

    while (*p != NULL) {
        g_object_unref (*p);
        p ++;
    }
    g_free (ps);

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
    g_return_if_fail (IBUS_IS_PROP_LIST (prop_list));
    g_return_if_fail (IBUS_IS_PROPERTY (prop));

    g_object_ref (prop);

    g_array_append_val (prop_list->properties, prop);
}

IBusProperty *
ibus_prop_list_get (IBusPropList *prop_list,
                    guint         index)
{
    g_return_val_if_fail (IBUS_IS_PROP_LIST (prop_list), NULL);


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
        IBusProperty *prop = g_array_index (prop_list->properties, IBusPropList *, i);
        if (ibus_property_update (prop, prop_update)) {
            return TRUE;
        }
    }

    return FALSE;
}
