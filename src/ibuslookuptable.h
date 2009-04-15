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
/**
 * SECTION: ibuslookuptable
 * @short_description: Candidate word/phrase lookup table.
 * @stability: Stable
 * @see_also: #IBusEngine
 *
 * An IBusLookuptable stores the candidate words or phrases for users to choose from.
 *
 * Use ibus_engine_update_lookup_table(), ibus_engine_show_lookup_table(),
 * and ibus_engine_hide_lookup_table() to update, show and hide the lookup
 * table.
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

/**
 * IBusLookupTable:
 * @page_size: number of candidate shown per page.
 * @cursor_pos: position index of cursor.
 * @cursor_visible: whether the cursor is visible.
 * @round: TRUE for lookup table wrap around.
 * @candidates: Candidate words/phrases/
 *
 * An IBusLookuptable stores the candidate words or phrases for users to choose from.
 */
struct _IBusLookupTable {
    IBusSerializable parent;

    /*< public >*/
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

/**
 * ibus_lookup_table_new:
 * @page_size: number of candidate shown per page.
 * @cursor_pos: position index of cursor.
 * @cursor_visible: whether the cursor is visible.
 * @round: TRUE for lookup table wrap around.
 * @returns: A newly allocated IBusLookupTable.
 *
 * New a IBusLookupTable.
 */
IBusLookupTable     *ibus_lookup_table_new      (guint               page_size,
                                                 guint               cursor_pos,
                                                 gboolean            cursor_visible,
                                                 gboolean            round);

/**
 * ibus_lookup_table_append_candidate:
 * @table: An IBusLookupTable.
 * @text: candidate word/phrase to be appended (in IBusText format).
 *
 * Append a candidate word/phrase to IBusLookupTable.
 */
void                 ibus_lookup_table_append_candidate
                                                (IBusLookupTable    *table,
                                                 IBusText           *text);

/**
 * ibus_lookup_table_get_candidate:
 * @table: An IBusLookupTable.
 * @index: Index in the Lookup table.
 * @returns: IBusText at the given index; NULL if no such IBusText.
 *
 * Return IBusText at the given index.
 */
IBusText            *ibus_lookup_table_get_candidate
                                                (IBusLookupTable    *table,
                                                 guint               index);

/**
 * ibus_lookup_table_set_cursor_pos:
 * @table: An IBusLookupTable.
 * @cursor_pos: The position of cursor.
 *
 * Set the cursor position of IBusLookupTable.
 */
void                 ibus_lookup_table_set_cursor_pos
                                                (IBusLookupTable    *table,
                                                 guint               cursor_pos);

/**
 * ibus_lookup_table_get_cursor_pos:
 * @table: An IBusLookupTable.
 * @returns: The position of cursor.
 *
 * Get the cursor position of IBusLookupTable.
 */
guint                ibus_lookup_table_get_cursor_pos
                                                (IBusLookupTable    *table);

/**
 * ibus_lookup_table_set_cursor_visible:
 * @table: An IBusLookupTable.
 * @visible: Whether to make the cursor of @table visible.
 *
 * Set whether to make the cursor of an IBusLookupTable visible or not.
 */
void                 ibus_lookup_table_set_cursor_visible
                                                (IBusLookupTable    *table,
                                                 gboolean            visible);

/**
 * ibus_lookup_table_is_cursor_visible:
 * @table: An IBusLookupTable.
 * @returns: Whether the cursor of @table is visible.
 *
 * Returns whether the cursor of an IBusLookupTable is visible.
 */
gboolean             ibus_lookup_table_is_cursor_visible
                                                (IBusLookupTable    *table);

/**
 * ibus_lookup_table_get_cursor_in_page:
 * @table: An IBusLookupTable.
 * @returns: The position of cursor in current page.
 *
 * Get the cursor position in current page of IBusLookupTable.
 */
guint                ibus_lookup_table_get_cursor_in_page
                                                (IBusLookupTable    *table);

/**
 * ibus_lookup_table_set_page_size:
 * @table: An IBusLookupTable.
 * @page_size: number of candidate shown per page.
 *
 * Set the number of candidate shown per page.
 */
void                 ibus_lookup_table_set_page_size
                                                (IBusLookupTable    *table,
                                                 guint               page_size);
/**
 * ibus_lookup_table_get_page_size:
 * @table: An IBusLookupTable.
 * @returns: Page size, i.e., number of candidate shown per page.
 *
 * Get the number of candidate shown per page.
 */
guint                ibus_lookup_table_get_page_size
                                                (IBusLookupTable    *table);

/**
 * ibus_lookup_table_clear:
 * @table: An IBusLookupTable.
 *
 * Clear and remove all candidate from an IBusLookupTable.
 */
void                 ibus_lookup_table_clear    (IBusLookupTable    *table);

/**
 * ibus_lookup_table_page_up:
 * @table: An IBusLookupTable.
 * @returns: TRUE if succeed.
 *
 * Go to previous page of an IBusLookupTable.
 *
 * It returns FALSE if it is already at the first page,
 * unless  <code>table&gt;-round==TRUE</code>, where it will go
 * to the last page.
 */
gboolean             ibus_lookup_table_page_up  (IBusLookupTable    *table);

/**
 * ibus_lookup_table_page_down:
 * @table: An IBusLookupTable.
 * @returns: TRUE if succeed.
 *
 * Go to next page of an IBusLookupTable.
 *
 * It returns FALSE if it is already at the last page,
 * unless  <code>table&gt;-round==TRUE</code>, where it will go
 * to the first page.
 */
gboolean             ibus_lookup_table_page_down(IBusLookupTable    *table);

/**
 * ibus_lookup_table_cursor_up:
 * @table: An IBusLookupTable.
 * @returns: TRUE if succeed.
 *
 * Go to previous candidate of an IBusLookupTable.
 *
 * It returns FALSE if it is already at the first candidate,
 * unless  <code>table&gt;-round==TRUE</code>, where it will go
 * to the last candidate.
 */
gboolean             ibus_lookup_table_cursor_up(IBusLookupTable    *table);

/**
 * ibus_lookup_table_cursor_down:
 * @table: An IBusLookupTable.
 * @returns: TRUE if succeed.
 *
 * Go to next candidate of an IBusLookupTable.
 *
 * It returns FALSE if it is already at the last candidate,
 * unless  <code>table&gt;-round==TRUE</code>, where it will go
 * to the first candidate.
 */
gboolean             ibus_lookup_table_cursor_down
                                                (IBusLookupTable    *table);
G_END_DECLS
#endif

