/* vim:set et sts=4: */
/* IBus - The Input Bus
 * Copyright (C) 2008-2009 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2009 Red Hat, Inc.
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
#include "ibuslookuptable.h"

/* functions prototype */
static void         ibus_lookup_table_class_init    (IBusLookupTableClass   *klass);
static void         ibus_lookup_table_init          (IBusLookupTable        *table);
static void         ibus_lookup_table_destroy       (IBusLookupTable        *table);
static gboolean     ibus_lookup_table_serialize     (IBusLookupTable        *table,
                                                     IBusMessageIter        *iter);
static gboolean     ibus_lookup_table_deserialize   (IBusLookupTable        *table,
                                                     IBusMessageIter        *iter);
static gboolean     ibus_lookup_table_copy          (IBusLookupTable        *dest,
                                                     IBusLookupTable        *src);

static IBusSerializableClass *parent_class = NULL;

GType
ibus_lookup_table_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusLookupTableClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_lookup_table_class_init,
        NULL,               /* class finialize */
        NULL,               /* class data */
        sizeof (IBusLookupTable),
        0,
        (GInstanceInitFunc) ibus_lookup_table_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERIALIZABLE,
                                       "IBusLookupTable",
                                       &type_info,
                                       0);
    }

    return type;
}

static void
ibus_lookup_table_class_init (IBusLookupTableClass *klass)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (klass);

    parent_class = (IBusSerializableClass *) g_type_class_peek_parent (klass);

    object_class->destroy = (IBusObjectDestroyFunc) ibus_lookup_table_destroy;

    serializable_class->serialize   = (IBusSerializableSerializeFunc) ibus_lookup_table_serialize;
    serializable_class->deserialize = (IBusSerializableDeserializeFunc) ibus_lookup_table_deserialize;
    serializable_class->copy        = (IBusSerializableCopyFunc) ibus_lookup_table_copy;

    g_string_append (serializable_class->signature, "uubbavav");
}

static void
ibus_lookup_table_init (IBusLookupTable *table)
{
    table->candidates = g_array_new (TRUE, TRUE, sizeof (IBusText *));
    table->labels = g_array_new (TRUE, TRUE, sizeof (IBusText *));
}

static void
ibus_lookup_table_destroy (IBusLookupTable *table)
{
    IBusText **p;
    gint i;

    if (table->candidates != NULL) {
        p = (IBusText **) g_array_free (table->candidates, FALSE);
        table->candidates = NULL;

        for (i = 0; p[i] != NULL; i++) {
            g_object_unref (p[i]);
        }
        g_free (p);
    }

    if (table->labels != NULL) {
        p = (IBusText **) g_array_free (table->labels, FALSE);
        table->labels = NULL;
        for (i = 0; p[i] != NULL; i++) {
            g_object_unref (p[i]);
        }
        g_free (p);
    }

    IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *) table);
}

static gboolean
ibus_lookup_table_serialize (IBusLookupTable *table,
                             IBusMessageIter *iter)
{
    IBusMessageIter array_iter;
    gboolean retval;
    guint i;

    retval = parent_class->serialize ((IBusSerializable *)table, iter);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_LOOKUP_TABLE (table), FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_UINT, &table->page_size);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_UINT, &table->cursor_pos);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_BOOLEAN, &table->cursor_visible);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_BOOLEAN, &table->round);
    g_return_val_if_fail (retval, FALSE);

    // append candidates
    retval = ibus_message_iter_open_container (iter,
                                               IBUS_TYPE_ARRAY,
                                               "v",
                                               &array_iter);
    g_return_val_if_fail (retval, FALSE);

    for (i = 0;; i++) {
        IBusText *text;

        text = ibus_lookup_table_get_candidate (table, i);
        if (text == NULL)
            break;

        retval = ibus_message_iter_append (&array_iter, IBUS_TYPE_TEXT, &text);
        g_return_val_if_fail (retval, FALSE);
    }

    retval = ibus_message_iter_close_container (iter, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    // append labels
    retval = ibus_message_iter_open_container (iter,
                                               IBUS_TYPE_ARRAY,
                                               "v",
                                               &array_iter);
    g_return_val_if_fail (retval, FALSE);

    for (i = 0;; i++) {
        IBusText *text;

        text = ibus_lookup_table_get_label (table, i);
        if (text == NULL)
            break;

        retval = ibus_message_iter_append (&array_iter, IBUS_TYPE_TEXT, &text);
        g_return_val_if_fail (retval, FALSE);
    }

    retval = ibus_message_iter_close_container (iter, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    return TRUE;
}

static gboolean
ibus_lookup_table_deserialize (IBusLookupTable *table,
                               IBusMessageIter *iter)
{
    DBusMessageIter array_iter;
    gboolean retval;

    retval = parent_class->deserialize ((IBusSerializable *)table, iter);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_LOOKUP_TABLE (table), FALSE);

    retval = ibus_message_iter_get (iter, G_TYPE_UINT, &table->page_size);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);

    retval = ibus_message_iter_get (iter, G_TYPE_UINT, &table->cursor_pos);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);

    retval = ibus_message_iter_get (iter, G_TYPE_BOOLEAN, &table->cursor_visible);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);

    retval = ibus_message_iter_get (iter, G_TYPE_BOOLEAN, &table->round);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);

    // deserialize candidates
    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_ARRAY, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    while (ibus_message_iter_get_arg_type (&array_iter) != G_TYPE_INVALID) {
        IBusText *text;
        retval = ibus_message_iter_get (&array_iter, IBUS_TYPE_TEXT, &text);
        g_return_val_if_fail (retval, FALSE);
        ibus_message_iter_next (&array_iter);

        ibus_lookup_table_append_candidate (table, text);
        g_object_unref (text);
    }

    ibus_message_iter_next (iter);

    // deserialize labels
    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_ARRAY, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    while (ibus_message_iter_get_arg_type (&array_iter) != G_TYPE_INVALID) {
        IBusText *text;
        retval = ibus_message_iter_get (&array_iter, IBUS_TYPE_TEXT, &text);
        g_return_val_if_fail (retval, FALSE);
        ibus_message_iter_next (&array_iter);

        ibus_lookup_table_append_label (table, text);
        g_object_unref (text);
    }

    ibus_message_iter_next (iter);

    return TRUE;
}

static gboolean
ibus_lookup_table_copy (IBusLookupTable *dest,
                        IBusLookupTable *src)
{
    gboolean retval;
    guint i;

    retval = parent_class->copy ((IBusSerializable *)dest, (IBusSerializable *)src);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_LOOKUP_TABLE (dest), FALSE);
    g_return_val_if_fail (IBUS_IS_LOOKUP_TABLE (src), FALSE);

    // copy candidates
    for (i = 0;; i++) {
        IBusText *text;

        text = ibus_lookup_table_get_candidate (src, i);
        if (text == NULL)
            break;

        text = (IBusText *) ibus_serializable_copy ((IBusSerializable *) text);

        ibus_lookup_table_append_candidate (dest, text);
        g_object_unref (text);
    }

    // copy labels
    for (i = 0;; i++) {
        IBusText *text;

        text = ibus_lookup_table_get_label (src, i);
        if (text == NULL)
            break;

        text = (IBusText *) ibus_serializable_copy ((IBusSerializable *) text);

        ibus_lookup_table_append_label (dest, text);
        g_object_unref (text);
    }

    return TRUE;
}

IBusLookupTable *
ibus_lookup_table_new (guint page_size,
                       guint cursor_pos,
                       gboolean cursor_visible,
                       gboolean round)
{
    g_assert (page_size > 0);
    g_assert (page_size <= 16);

    IBusLookupTable *table;

    table= g_object_new (IBUS_TYPE_LOOKUP_TABLE, NULL);

    table->page_size = page_size;
    table->cursor_pos = cursor_pos;
    table->cursor_visible = cursor_visible;
    table->round = round;

    return table;
}

guint
ibus_lookup_table_get_number_of_candidates (IBusLookupTable *table)
{
    g_return_val_if_fail (IBUS_IS_LOOKUP_TABLE (table), 0);

    return table->candidates->len;
}

void
ibus_lookup_table_append_candidate (IBusLookupTable *table,
                                    IBusText        *text)
{
    g_return_if_fail (IBUS_IS_LOOKUP_TABLE (table));
    g_return_if_fail (IBUS_IS_TEXT (text));

    g_object_ref (text);
    g_array_append_val (table->candidates, text);
}

IBusText *
ibus_lookup_table_get_candidate (IBusLookupTable *table,
                                 guint            index)
{
    g_return_val_if_fail (IBUS_IS_LOOKUP_TABLE (table), NULL);

    if (index >= table->candidates->len)
        return NULL;

    return g_array_index (table->candidates, IBusText *, index);
}

void
ibus_lookup_table_append_label (IBusLookupTable *table,
                                IBusText        *text)
{
    g_return_if_fail (IBUS_IS_LOOKUP_TABLE (table));
    g_return_if_fail (IBUS_IS_TEXT (text));

    g_object_ref (text);
    g_array_append_val (table->labels, text);
}

IBusText *
ibus_lookup_table_get_label (IBusLookupTable *table,
                             guint            index)
{
    g_return_val_if_fail (IBUS_IS_LOOKUP_TABLE (table), NULL);

    if (index >= table->labels->len)
        return NULL;

    return g_array_index (table->labels, IBusText *, index);
}

void
ibus_lookup_table_clear (IBusLookupTable *table)
{
    g_return_if_fail (IBUS_IS_LOOKUP_TABLE (table));

    gint index;

    for (index = 0; index < table->candidates->len; index ++) {
        g_object_unref (g_array_index (table->candidates, IBusText *, index));
    }

    g_array_set_size (table->candidates, 0);

    table->cursor_pos = 0;
}

void
ibus_lookup_table_set_cursor_pos (IBusLookupTable *table,
                                  guint            cursor_pos)
{
    g_assert (IBUS_IS_LOOKUP_TABLE (table));
    g_assert (cursor_pos < table->candidates->len);

    table->cursor_pos = cursor_pos;
}

guint
ibus_lookup_table_get_cursor_pos (IBusLookupTable *table)
{
    g_assert (IBUS_IS_LOOKUP_TABLE (table));

    return table->cursor_pos;
}

guint
ibus_lookup_table_get_cursor_in_page (IBusLookupTable *table)
{
    g_assert (IBUS_IS_LOOKUP_TABLE (table));

    return table->cursor_pos % table->page_size;
}

void
ibus_lookup_table_set_cursor_visible (IBusLookupTable *table,
                                      gboolean         visible)
{
    g_assert (IBUS_IS_LOOKUP_TABLE (table));

    table->cursor_visible = visible;
}

gboolean
ibus_lookup_table_is_cursor_visible (IBusLookupTable *table)
{
    g_assert (IBUS_IS_LOOKUP_TABLE (table));

    return table->cursor_visible;
}

void
ibus_lookup_table_set_page_size  (IBusLookupTable *table,
                                  guint            page_size)
{
    g_assert (IBUS_IS_LOOKUP_TABLE (table));
    g_assert (page_size > 0);

    table->page_size = page_size;
}

guint
ibus_lookup_table_get_page_size (IBusLookupTable *table)
{
    g_assert (IBUS_IS_LOOKUP_TABLE (table));

    return table->page_size;
}

gboolean
ibus_lookup_table_page_up (IBusLookupTable *table)
{
    g_assert (IBUS_IS_LOOKUP_TABLE (table));

    if (table->cursor_pos < table->page_size) {
        gint i;
        gint page_nr;

        if (!table->round) {
            return FALSE;
        }

        /* cursor index in page */
        i = table->cursor_pos % table->page_size;
        page_nr = (table->candidates->len + table->page_size - 1) / table->page_size;

        table->cursor_pos = page_nr * table->page_size + i;
        if (table->cursor_pos >= table->candidates->len) {
            table->cursor_pos = table->candidates->len - 1;
        }
        return TRUE;
    }

    table->cursor_pos -= table->page_size;
    return TRUE;
}

gboolean
ibus_lookup_table_page_down (IBusLookupTable *table)
{
    g_assert (IBUS_IS_LOOKUP_TABLE (table));

    gint i;
    gint page;
    gint page_nr;

    /* cursor index in page */
    i = table->cursor_pos % table->page_size;
    page = table->cursor_pos  / table->page_size;
    page_nr = (table->candidates->len + table->page_size - 1) / table->page_size;

    if (page == page_nr - 1) {
        if (!table->round)
            return FALSE;

        table->cursor_pos = i;
        return TRUE;
    }

    table->cursor_pos += table->page_size;
    if (table->cursor_pos > table->candidates->len - 1) {
        table->cursor_pos = table->candidates->len - 1;
    }
    return TRUE;
}

gboolean
ibus_lookup_table_cursor_up (IBusLookupTable *table)
{
    g_assert (IBUS_IS_LOOKUP_TABLE (table));

    if (table->cursor_pos == 0) {
        if (!table->round)
            return FALSE;

        table->cursor_pos = table->candidates->len - 1;
        return TRUE;
    }

    table->cursor_pos --;

    return TRUE;
}

gboolean
ibus_lookup_table_cursor_down (IBusLookupTable *table)
{
    g_assert (IBUS_IS_LOOKUP_TABLE (table));

    if (table->cursor_pos == table->candidates->len - 1) {
        if (!table->round)
            return FALSE;

        table->cursor_pos = 0;
        return TRUE;
    }

    table->cursor_pos ++;
    return TRUE;
}
