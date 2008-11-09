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
#include <glib.h>
#include "ibusmessage.h"
#include "ibusproperty.h"

static gboolean      ibus_property_serialize      (IBusProperty    *prop,
                                                   IBusMessageIter *iter);
static IBusProperty *ibus_property_deserialize    (IBusMessageIter *iter);
static gboolean      ibus_prop_list_serialize     (IBusPropList    *prop_list,
                                                   IBusMessageIter *iter);
static IBusPropList *ibus_prop_list_deserialize   (IBusMessageIter *iter);


GType
ibus_property_get_type ()
{
    static GType type = 0;
    if (type == 0) {
        type = g_boxed_type_register_static ("IBusProperty",
                    (GBoxedCopyFunc)ibus_property_copy,
                    (GBoxedFreeFunc)ibus_property_free);

        ibus_message_register_type (type,
                                    (IBusSerializeFunc) ibus_property_serialize,
                                    (IBusDeserializeFunc) ibus_property_deserialize);
    }
    return type;
}

IBusProperty *
ibus_property_new (const gchar      *name,
                   IBusPropType      type,
                   const gchar      *label,
                   const gchar      *icon,
                   const gchar      *tooltip,
                   gboolean          sensitive,
                   gboolean          visible,
                   IBusPropState     state,
                   IBusPropList     *prop_list)
{
    IBusProperty *prop = g_slice_new (IBusProperty);
    prop->name = g_strdup (name);
    prop->type = type;
    prop->label = g_strdup (label);
    prop->icon = g_strdup (icon);
    prop->tooltip = g_strdup (tooltip);
    prop->sensitive = sensitive;
    prop->visible = visible;
    prop->state = state;
    if (prop_list)
        prop->sub_props = prop_list;
    else
        prop->sub_props = ibus_prop_list_new ();

    return prop;
}


IBusProperty *
ibus_property_copy (IBusProperty *prop)
{
    g_assert (prop != NULL);
    
    IBusProperty *prop_new;

    prop_new = ibus_property_new (prop->name,
                                  prop->type,
                                  prop->label,
                                  prop->icon,
                                  prop->tooltip,
                                  prop->sensitive,
                                  prop->visible,
                                  prop->state,
                                  prop->sub_props);
    if (prop->sub_props)
        prop_new->sub_props = ibus_prop_list_copy (prop->sub_props);

    return prop_new;
}

void
ibus_property_free (IBusProperty *prop)
{
    if (prop == NULL)
        return;

    ibus_prop_list_unref (prop->sub_props);

    g_free (prop->name);
    g_free (prop->label);
    g_free (prop->icon);
    g_free (prop->tooltip);
    g_slice_free (IBusProperty, prop);
}

void
ibus_property_set_sub_props (IBusProperty   *prop,
                             IBusPropList   *prop_list)
{
    ibus_prop_list_unref (prop->sub_props);
    prop->sub_props = ibus_prop_list_ref (prop_list);
}

GType
ibus_prop_list_get_type ()
{
    static GType type = 0;
    if (type == 0) {
        type = g_boxed_type_register_static ("IBusPropList",
                    (GBoxedCopyFunc)ibus_prop_list_copy,
                    (GBoxedFreeFunc)ibus_prop_list_unref);
        
        ibus_message_register_type (type,
                                    (IBusSerializeFunc) ibus_prop_list_serialize,
                                    (IBusDeserializeFunc) ibus_prop_list_deserialize);
    }
    return type;
}

IBusPropList *
ibus_prop_list_new ()
{
    IBusPropList *prop_list = g_slice_new(IBusPropList);
    prop_list->refcount = 1;
    prop_list->properties = g_array_new (TRUE, TRUE, sizeof (IBusProperty *));
    return prop_list;
}

IBusPropList *
ibus_prop_list_copy (IBusPropList *prop_list)
{
    IBusPropList *new_list = ibus_prop_list_new ();
    guint i;
    for (i = 0;; i++) {
        IBusProperty *prop = ibus_prop_list_get (prop_list, i);
        if (prop == NULL) {
            break;
        }
        ibus_prop_list_append (new_list, ibus_property_copy (prop));
    }
    return new_list;
}

IBusPropList *
ibus_prop_list_ref (IBusPropList *prop_list)
{
    if (prop_list == NULL) {
        return NULL;
    }
    prop_list->refcount ++;
    return prop_list;
}

void
ibus_prop_list_unref (IBusPropList *prop_list)
{
    if (prop_list == NULL) {
        return;
    }

    prop_list->refcount --;
    if (prop_list->refcount <= 0) {
        guint i;
        for (i = 0; i < prop_list->properties->len; i++) {
            IBusProperty *prop = ibus_prop_list_get (prop_list, i);
            ibus_property_free (prop);
        }
        g_array_free (prop_list->properties, TRUE);
        g_slice_free (IBusPropList, prop_list);
    }
}

void
ibus_prop_list_append (IBusPropList *prop_list, IBusProperty *prop)
{
    g_return_if_fail (prop_list != NULL);
    g_return_if_fail (prop != NULL);
    g_array_append_val (prop_list->properties, prop);
}

IBusProperty *
ibus_prop_list_get (IBusPropList *prop_list, guint index)
{
    g_return_val_if_fail (prop_list != NULL, NULL);
    IBusProperty *prop = NULL;
    if (index < prop_list->properties->len) {
        prop = g_array_index (prop_list->properties, IBusProperty *, index);
    }
    return prop;
}

static IBusProperty *
ibus_property_deserialize (IBusMessageIter *iter)
{
    gchar *name;
    guint32 type;
    gchar *label;
    gchar *icon;
    gchar *tooltip;
    gboolean sensitive;
    gboolean visible;
    guint32 state;
    IBusPropList *prop_list = NULL;
    IBusProperty *prop;
    gboolean retval;
    
    IBusMessageIter sub_iter;

    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_STRUCT, &sub_iter);
    g_assert (retval);

    // get name
    ibus_message_iter_get (&sub_iter, G_TYPE_STRING, &name);
    
    // get type
    ibus_message_iter_get (&sub_iter, G_TYPE_UINT, &type);
    
    // get label
    ibus_message_iter_get (&sub_iter, G_TYPE_STRING, &label);
    
    // get icon
    ibus_message_iter_get (&sub_iter, G_TYPE_STRING, &icon);
    
    // get tooltip
    ibus_message_iter_get (&sub_iter, G_TYPE_STRING, &tooltip);
    
    // get sensitive
    ibus_message_iter_get (&sub_iter, G_TYPE_BOOLEAN, &sensitive);
    
    // get visible
    ibus_message_iter_get (&sub_iter, G_TYPE_BOOLEAN, &visible);
    
    // get visible
    ibus_message_iter_get (&sub_iter, G_TYPE_UINT, &state);

    // get sub prop
    ibus_message_iter_get (&sub_iter, IBUS_TYPE_PROP_LIST, &prop_list);
    
    prop = ibus_property_new (name, type, label, icon, tooltip, sensitive, visible, state, prop_list);
    
    return prop;
}

IBusPropList *
ibus_prop_list_deserialize (IBusMessageIter *iter)
{
    IBusMessageIter sub_iter;
    IBusPropList *prop_list;
    gboolean retval;

    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_ARRAY, &sub_iter);
    g_assert (retval);
    
    prop_list = ibus_prop_list_new ();

    while (ibus_message_iter_has_next (&sub_iter)) {
        IBusProperty *prop = NULL;
        ibus_message_iter_get (&sub_iter, IBUS_TYPE_PROPERTY, &prop);
        if (prop == NULL)
            break;
        ibus_prop_list_append (prop_list, prop);
    }
    return prop_list;
}

gboolean
ibus_property_serialize (IBusProperty    *prop,
                         IBusMessageIter *iter)
{
    g_assert (prop != NULL);
    g_assert (iter != NULL);

    IBusMessageIter sub_iter;
    gboolean retval;

    retval = ibus_message_iter_open_container(iter, IBUS_TYPE_STRUCT, 0, &sub_iter);
    g_assert (retval);

    ibus_message_iter_append (&sub_iter, G_TYPE_STRING, &prop->name);
    ibus_message_iter_append (&sub_iter, G_TYPE_UINT, &prop->type);
    ibus_message_iter_append (&sub_iter, G_TYPE_STRING, &prop->label);
    ibus_message_iter_append (&sub_iter, G_TYPE_STRING, &prop->icon);
    ibus_message_iter_append (&sub_iter, G_TYPE_STRING, &prop->tooltip);
    ibus_message_iter_append (&sub_iter, G_TYPE_BOOLEAN, &prop->sensitive);
    ibus_message_iter_append (&sub_iter, G_TYPE_BOOLEAN, &prop->visible);
    ibus_message_iter_append (&sub_iter, G_TYPE_UINT, &prop->state);
    ibus_message_iter_append (&sub_iter, IBUS_TYPE_PROP_LIST, &prop->sub_props);
    ibus_message_iter_close_container (iter, &sub_iter);

    return TRUE;
}


gboolean
ibus_prop_list_serialize (IBusPropList    *prop_list,
                          IBusMessageIter *iter)
{
    g_assert (prop_list != NULL);
    g_assert (iter != NULL);

    gint i;
    IBusProperty *prop;
    IBusMessageIter sub_iter;
    gboolean retval;

    retval = ibus_message_iter_open_container (iter, IBUS_TYPE_ARRAY, "(susssbbuav)", &sub_iter);
    for (i = 0;; i++) {
        prop = ibus_prop_list_get (prop_list, i);
        if (prop == NULL)
            break;
        ibus_message_iter_append (&sub_iter, IBUS_TYPE_PROPERTY, &prop);
    }
    ibus_message_iter_close_container (iter, &sub_iter);
    return TRUE;
}

