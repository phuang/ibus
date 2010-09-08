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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/**
 * SECTION: ibustext
 * @short_description: Text with decorating information.
 * @see_also: #IBusAttribute
 *
 * An IBusText is the main text object in IBus.
 * The text is decorated according to associated IBusAttribute,
 * e.g. the foreground/background color, underline, and
 * applied scope.
 */

#ifndef __IBUS_TEXT_H_
#define __IBUS_TEXT_H_

#include "ibusserializable.h"
#include "ibusattrlist.h"

/*
 * Type macros.
 */
/* define IBusText macros */
#define IBUS_TYPE_TEXT             \
    (ibus_text_get_type ())
#define IBUS_TEXT(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_TEXT, IBusText))
#define IBUS_TEXT_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_TEXT, IBusTextClass))
#define IBUS_IS_TEXT(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_TEXT))
#define IBUS_IS_TEXT_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_TEXT))
#define IBUS_TEXT_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_TEXT, IBusTextClass))

G_BEGIN_DECLS

typedef struct _IBusText IBusText;
typedef struct _IBusTextClass IBusTextClass;

/**
 * IBusText:
 * @is_static: Whether @text is static, i.e., no need and will not be freed. Only TRUE if IBusText is newed from ibus_text_new_from_static_string().
 * @text: The string content of IBusText in UTF-8.
 * @attrs: Associated IBusAttributes.
 *
 * A text object in IBus.
 */
struct _IBusText {
    IBusSerializable parent;

    /* members */
    /*< public >*/
    gboolean is_static;
    gchar  *text;
    IBusAttrList *attrs;
};

struct _IBusTextClass {
    IBusSerializableClass parent;
};

GType            ibus_text_get_type                 (void);

/**
 * ibus_text_new_from_string:
 * @str: An text string to be set.
 * @returns: A newly allocated IBusText.
 *
 * New an IBusText from a string.
 *
 * @str will be duplicated in IBusText, so feel free to free @str after this function.
 */
IBusText        *ibus_text_new_from_string          (const gchar    *str);

/**
 * ibus_text_new_from_ucs4:
 * @str: An text string to be set.
 * @returns: A newly allocated IBusText.
 *
 * New an IBusText from an UCS-4 encoded string.
 *
 * @str will be duplicated in IBusText, so feel free to free @str after this function.
 */
IBusText        *ibus_text_new_from_ucs4            (const gunichar *str);

/**
 * ibus_text_new_from_static_string:
 * @str: An text string to be set.
 * @returns: A newly allocated IBusText.
 *
 * New an IBusText from a static string.
 *
 * Since @str is a static string which won't be freed.
 * This function will NOT duplicate @str.
 */
IBusText        *ibus_text_new_from_static_string   (const gchar    *str);

/**
 * ibus_text_new_from_printf:
 * @fmt: printf format string.
 * @...: arguments for @fmt.
 * @returns: A newly allocated IBusText.
 *
 * New an IBusText from a printf expression.
 *
 * The result of printf expression is stored in the new IBusText instance.
 */
IBusText        *ibus_text_new_from_printf          (const gchar    *fmt,
                                                     ...);

/**
 * ibus_text_new_from_unichar:
 * @c: A single UCS4-encoded character.
 * @returns: A newly allocated IBusText.
 *
 * New an IBusText from a single UCS4-encoded character.
 */
IBusText        *ibus_text_new_from_unichar         (gunichar        c);

/**
 * ibus_text_append_attribute:
 * @text: an IBusText
 * @type: IBusAttributeType for @text.
 * @value: Value for the type.
 * @start_index: The starting index, inclusive.
 * @end_index: The ending index, exclusive.
 *
 * Append an IBusAttribute for IBusText.
 */
void             ibus_text_append_attribute         (IBusText       *text,
                                                     guint           type,
                                                     guint           value,
                                                     guint           start_index,
                                                     gint            end_index);
/**
 * ibus_text_get_length:
 * @text: An IBusText.
 * @returns: Number of character in @text, not counted by bytes.
 *
 * Return number of characters in an IBusText.
 * This function is based on g_utf8_strlen(), so unlike strlen(),
 * it does not count by bytes but characters instead.
 */
guint            ibus_text_get_length               (IBusText       *text);

G_END_DECLS
#endif

