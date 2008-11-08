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
#include "ibuslookuptable.h"

GType
ibus_lookup_table_get_type ()
{
    static GType type = 0;
    if (type == 0) {
        type = g_boxed_type_register_static ("IBusLookupTable",
                    (GBoxedCopyFunc)ibus_lookup_table_copy,
                    (GBoxedFreeFunc)ibus_lookup_table_unref);
        
        ibus_message_register_type (type,
                                    (IBusSerializeFunc) ibus_lookup_table_to_dbus_message,
                                    (IBusDeserializeFunc) ibus_lookup_table_from_dbus_message);
    }
    return type;
}

IBusLookupTable *
ibus_lookup_table_new (gint page_size, gint cursor_pos, gboolean cursor_visible)
{
    IBusLookupTable *table = g_slice_new (IBusLookupTable);
    table->page_size = page_size;
    table->cursor_pos = cursor_pos;
    table->cursor_visible = cursor_visible;
    table->candidates = g_array_new (TRUE, TRUE, sizeof (IBusCandidate *));
    return table;
}

IBusLookupTable *
ibus_lookup_table_copy (IBusLookupTable *table)
{
    return ibus_lookup_table_new (table->page_size, table->cursor_pos,
                table->cursor_visible);
}

IBusLookupTable *
ibus_lookup_table_ref (IBusLookupTable *table)
{
    if (table == NULL)
        return NULL;
    table->refcount ++;

    return table;
}

void
ibus_lookup_table_unref (IBusLookupTable *table)
{
    if (table == NULL)
        return;
    table->refcount --;
    if (table->refcount <= 0) {
        IBusCandidate *candidate;
        gint i;
        for (i = 0; i < table->candidates->len; i++) {
            candidate = g_array_index (table->candidates, IBusCandidate *, i);
            g_free (candidate->text);
            ibus_attr_list_unref (candidate->attr_list);
            g_slice_free (IBusCandidate, candidate);
        }
        g_array_free (table->candidates, TRUE);
        g_slice_free (IBusLookupTable, table);
    }
}

void
ibus_lookup_table_append_candidate (IBusLookupTable *table, const gchar *text, IBusAttrList *attr_list)
{
    g_assert (table != NULL);
    g_assert (text != NULL);
    
    IBusCandidate *candidate;

    candidate = g_slice_new (IBusCandidate);
    
    candidate->text = g_strdup (text);
    if (attr_list)
        candidate->attr_list = ibus_attr_list_ref (attr_list);
    else
        candidate->attr_list = ibus_attr_list_new ();
    
    g_array_append_val (table->candidates, candidate);
}

IBusLookupTable *
ibus_lookup_table_from_dbus_message (DBusMessageIter *iter)
{
    g_assert (iter != NULL);
    
    DBusMessageIter sub_iter, sub_sub_iter;
    IBusLookupTable *table;
    gint page_size, cursor_pos;
    gboolean cursor_visible;

    g_return_val_if_fail (dbus_message_iter_get_arg_type (iter) == DBUS_TYPE_STRUCT, NULL);


    dbus_message_iter_recurse (iter, &sub_iter);

    g_return_val_if_fail (dbus_message_iter_get_arg_type (&sub_iter) == DBUS_TYPE_INT32, NULL);
    dbus_message_iter_get_basic (&sub_iter, &page_size);
    dbus_message_iter_next (&sub_iter);
    
    g_return_val_if_fail (dbus_message_iter_get_arg_type (&sub_iter) == DBUS_TYPE_INT32, NULL);
    dbus_message_iter_get_basic (&sub_iter, &cursor_pos);
    dbus_message_iter_next (&sub_iter);
    
    g_return_val_if_fail (dbus_message_iter_get_arg_type (&sub_iter) == DBUS_TYPE_BOOLEAN, NULL);
    dbus_message_iter_get_basic (&sub_iter, &cursor_visible);
    dbus_message_iter_next (&sub_iter);

    g_return_val_if_fail (dbus_message_iter_get_arg_type (&sub_iter) == DBUS_TYPE_ARRAY, NULL);
    table = ibus_lookup_table_new (page_size, cursor_pos, cursor_visible);
    
    dbus_message_iter_recurse (&sub_iter, &sub_sub_iter);
    
    while (1) {
        gchar *text;
        IBusAttrList *attr_list;
        DBusMessageIter sub_sub_sub_iter;
        
        if (dbus_message_iter_get_arg_type (&sub_sub_iter) != DBUS_TYPE_STRUCT)
            break;
        
        dbus_message_iter_recurse (&sub_sub_iter, &sub_sub_sub_iter);
        if (dbus_message_iter_get_arg_type (&sub_sub_sub_iter) != DBUS_TYPE_STRING)
            break;
        dbus_message_iter_get_basic (&sub_sub_sub_iter, &text);
        dbus_message_iter_next (&sub_sub_sub_iter);
        
        attr_list = ibus_attr_list_from_dbus_message (&sub_sub_sub_iter);
        if (attr_list == NULL)
            break;
        ibus_lookup_table_append_candidate (table, text, attr_list);

        dbus_message_iter_next (&sub_sub_iter);
    }
    
    dbus_message_iter_next (iter);
    return table;
}

gboolean
ibus_lookup_table_to_dbus_message (IBusLookupTable *table, DBusMessageIter *iter)
{
    g_assert (table != NULL);
    g_assert (iter != NULL);
    
    gint i;
    DBusMessageIter sub_iter, sub_sub_iter, sub_sub_sub_iter;

    dbus_message_iter_open_container (iter, DBUS_TYPE_STRUCT, 0, &sub_iter);

    dbus_message_iter_append_basic (&sub_iter, DBUS_TYPE_INT32, &table->page_size);
    dbus_message_iter_append_basic (&sub_iter, DBUS_TYPE_INT32, &table->cursor_pos);
    dbus_message_iter_append_basic (&sub_iter, DBUS_TYPE_BOOLEAN, &table->cursor_visible);
    
    dbus_message_iter_open_container (&sub_iter, DBUS_TYPE_ARRAY, "(sa(uuuu))", &sub_sub_iter);

    for (i = 0; i < table->candidates->len; i++) {
        dbus_message_iter_open_container (&sub_sub_iter, DBUS_TYPE_STRUCT, 0, &sub_sub_sub_iter);
        
        IBusCandidate *candidate = &g_array_index (table->candidates, IBusCandidate, i);
        dbus_message_iter_append_basic (&sub_sub_sub_iter, DBUS_TYPE_STRING, &candidate->text);
        ibus_attr_list_to_dbus_message (candidate->attr_list, &sub_sub_sub_iter);
        dbus_message_iter_close_container (&sub_sub_iter, &sub_sub_sub_iter);
    }
    
    dbus_message_iter_close_container (&sub_iter, &sub_sub_iter);
    dbus_message_iter_close_container (iter, &sub_iter);
    return TRUE;
}


