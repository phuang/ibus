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
#ifndef __IBUS_LOOKUP_TABLE_H_
#define __IBUS_LOOKUP_TABLE_H_

#include "ibusserializable.h"
#include "ibustext.h"

/*
 * Type macros.
 */
/* define IBusLookupTable macros */
#define IBUS_TYPE_LOOKUP_TABLE             \
    (ibus_lookup_table_get_type ())
#define IBUS_LOOKUP_TABLE(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_LOOKUP_TABLE, IBusLookupTable))
#define IBUS_LOOKUP_TABLE_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_LOOKUP_TABLE, IBusLookupTableClass))
#define IBUS_IS_LOOKUP_TABLE(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_LOOKUP_TABLE))
#define IBUS_IS_LOOKUP_TABLE_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_LOOKUP_TABLE))
#define IBUS_LOOKUP_TABLE_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_LOOKUP_TABLE, IBusLookupTableClass))


G_BEGIN_DECLS

typedef struct _IBusLookupTable IBusLookupTable;
typedef struct _IBusLookupTableClass IBusLookupTableClass;

struct _IBusLookupTable {
    IBusSerializable parent;

    guint page_size;
    guint cursor_pos;
    gboolean cursor_visible;
    gboolean round;

    GArray *candidates;
};

struct _IBusLookupTableClass {
    IBusSerializableClass parent;
};


GType                ibus_lookup_table_get_type (void);
IBusLookupTable     *ibus_lookup_table_new      (guint               page_size,
                                                 guint               cursor_pos,
                                                 gboolean            cursor_visible,
                                                 gboolean            round);
void                 ibus_lookup_table_append_candidate
                                                (IBusLookupTable    *table,
                                                 IBusText           *text);
IBusText            *ibus_lookup_table_get_candidate
                                                (IBusLookupTable    *table,
                                                 guint               index);
void                 ibus_lookup_table_set_cursor_pos
                                                (IBusLookupTable    *table,
                                                 guint               cursor_pos);
void                 ibus_lookup_table_set_page_size
                                                (IBusLookupTable    *table,
                                                 guint               page_size);
void                 ibus_lookup_table_clear    (IBusLookupTable    *table);
gboolean             ibus_lookup_table_page_up  (IBusLookupTable    *table);
gboolean             ibus_lookup_table_page_down(IBusLookupTable    *table);
gboolean             ibus_lookup_table_cursor_up(IBusLookupTable    *table);
gboolean             ibus_lookup_table_cursor_down
                                                (IBusLookupTable    *table);
G_END_DECLS
#endif

