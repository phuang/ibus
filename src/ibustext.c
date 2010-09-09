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
#include "ibustext.h"

/* functions prototype */
static void         ibus_text_destroy      (IBusText            *text);
static gboolean     ibus_text_serialize    (IBusText            *text,
                                            IBusMessageIter     *iter);
static gboolean     ibus_text_deserialize  (IBusText            *text,
                                            IBusMessageIter     *iter);
static gboolean     ibus_text_copy         (IBusText            *dest,
                                            const IBusText      *src);

G_DEFINE_TYPE (IBusText, ibus_text, IBUS_TYPE_SERIALIZABLE)

static void
ibus_text_class_init (IBusTextClass *klass)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (klass);

    ibus_text_parent_class = (IBusSerializableClass *) g_type_class_peek_parent (klass);

    object_class->destroy = (IBusObjectDestroyFunc) ibus_text_destroy;

    serializable_class->serialize   = (IBusSerializableSerializeFunc) ibus_text_serialize;
    serializable_class->deserialize = (IBusSerializableDeserializeFunc) ibus_text_deserialize;
    serializable_class->copy        = (IBusSerializableCopyFunc) ibus_text_copy;

    g_string_append (serializable_class->signature, "sv");
}

static void
ibus_text_init (IBusText *text)
{
    text->is_static = TRUE;
    text->text = "";
    text->attrs = NULL;
}

static void
ibus_text_destroy (IBusText *text)
{
    if (text->text != NULL && text->is_static == FALSE) {
        g_free (text->text);
        text->text = NULL;
    }

    if (text->attrs) {
        g_object_unref (text->attrs);
        text->attrs = NULL;
    }

    IBUS_OBJECT_CLASS (ibus_text_parent_class)->destroy ((IBusObject *)text);
}

static gboolean
ibus_text_serialize (IBusText        *text,
                     IBusMessageIter *iter)
{
    gboolean retval;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_text_parent_class)->serialize (
                        (IBusSerializable *)text, iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &text->text);
    g_return_val_if_fail (retval, FALSE);

    if (text->attrs == NULL) {
        text->attrs = ibus_attr_list_new ();
        g_object_ref_sink (text->attrs);
    }

    retval = ibus_message_iter_append (iter, IBUS_TYPE_ATTR_LIST, &text->attrs);
    g_return_val_if_fail (retval, FALSE);

    return TRUE;
}

static gboolean
ibus_text_deserialize (IBusText        *text,
                       IBusMessageIter *iter)
{
    gboolean retval;
    gchar *str;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_text_parent_class)->deserialize (
                            (IBusSerializable *)text, iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &str);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);

    text->is_static = FALSE;
    text->text = g_strdup (str);

    if (text->attrs) {
        g_object_unref (text->attrs);
        text->attrs = NULL;
    }

    retval = ibus_message_iter_get (iter, IBUS_TYPE_ATTR_LIST, &text->attrs);
    g_object_ref_sink (text->attrs);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);

    return TRUE;
}

static gboolean
ibus_text_copy (IBusText       *dest,
                const IBusText *src)
{
    gboolean retval;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_text_parent_class)->copy (
                            (IBusSerializable *)dest,
                            (IBusSerializable *)src);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_TEXT (dest), FALSE);
    g_return_val_if_fail (IBUS_IS_TEXT (src), FALSE);

    dest->text = g_strdup (src->text);
    dest->is_static = FALSE;
    if (src->attrs) {
        dest->attrs = (IBusAttrList *)ibus_serializable_copy ((IBusSerializable *)src->attrs);
        g_object_ref_sink (dest->attrs);
    }

    return TRUE;
}

IBusText *
ibus_text_new_from_string (const gchar *str)
{
    g_assert (str);

    IBusText *text;

    text= g_object_new (IBUS_TYPE_TEXT, NULL);

    text->is_static = FALSE;
    text->text = g_strdup (str);

    return text;
}

IBusText *
ibus_text_new_from_ucs4 (const gunichar *str)
{
    g_assert (str);

    IBusText *text;
    gchar *buf;

    buf = g_ucs4_to_utf8 (str, -1, NULL, NULL, NULL);

    if (buf == NULL) {
        return NULL;
    }

    text= g_object_new (IBUS_TYPE_TEXT, NULL);

    text->is_static = FALSE;
    text->text = buf;

    return text;
}

IBusText *
ibus_text_new_from_static_string (const gchar *str)
{
    g_assert (str);

    IBusText *text;

    text= g_object_new (IBUS_TYPE_TEXT, NULL);

    text->is_static = TRUE;
    text->text = (gchar *)str;

    return text;
}

IBusText *
ibus_text_new_from_printf (const gchar *format,
                           ...)
{
    g_assert (format);

    gchar *str;
    IBusText *text;
    va_list args;

    va_start (args, format);
    str = g_strdup_vprintf (format, args);
    va_end (args);

    if (str == NULL)
        return NULL;

    text= g_object_new (IBUS_TYPE_TEXT, NULL);
    text->is_static = FALSE;
    text->text = (gchar *)str;

    return text;
}

IBusText *
ibus_text_new_from_unichar (gunichar c)
{
    IBusText *text;
    gint len;

    if (!g_unichar_validate (c)) {
        return NULL;
    }

    text= g_object_new (IBUS_TYPE_TEXT, NULL);

    text->is_static = FALSE;
    text->text = (gchar *)g_malloc (12);
    len = g_unichar_to_utf8 (c, text->text);
    text->text[len] =  0;

    return text;
}

void
ibus_text_append_attribute (IBusText *text,
                            guint     type,
                            guint     value,
                            guint     start_index,
                            gint      end_index)
{
    g_assert (IBUS_IS_TEXT (text));

    IBusAttribute *attr;

    if (end_index < 0) {
        end_index  += g_utf8_strlen(text->text, -1) + 1;
    }

    if (end_index <= 0) {
        return;
    }

    if (text->attrs == NULL) {
        text->attrs = ibus_attr_list_new ();
    }

    attr = ibus_attribute_new (type, value, start_index, end_index);
    ibus_attr_list_append (text->attrs, attr);
}

guint
ibus_text_get_length (IBusText *text)
{
    return g_utf8_strlen (text->text, -1);
}
