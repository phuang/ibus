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
#ifndef __IBUS_TEXT_H_
#define __IBUS_TEXT_H_

#include "ibusserializable.h"
#include "ibusattribute.h"

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

struct _IBusText {
    IBusSerializable parent;

    /* members */
    gboolean is_static;
    gchar  *text;
    IBusAttrList *attrs;
};

struct _IBusTextClass {
    IBusSerializableClass parent;
};

GType            ibus_text_get_type                 (void);
IBusText        *ibus_text_new_from_string          (const gchar    *str);
IBusText        *ibus_text_new_from_ucs4            (const gunichar *str);
IBusText        *ibus_text_new_from_static_string   (const gchar    *str);
IBusText        *ibus_text_new_from_printf          (const gchar    *fmt,
                                                     ...);
IBusText        *ibus_text_new_from_unichar         (gunichar        c);
void             ibus_text_append_attribute         (IBusText       *text,
                                                     guint           type,
                                                     guint           value,
                                                     guint           start_index,
                                                     gint            end_index);
guint            ibus_text_get_length               (IBusText       *text);

G_END_DECLS
#endif

