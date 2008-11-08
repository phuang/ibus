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

#include <glib-object.h>
#include "ibusmessage.h"
#include "ibusattribute.h"

/*
 * Type macros.
 */
#define IBUS_TYPE_LOOKUP_TABLE          (ibus_lookup_table_get_type ())

/* define GOBJECT macros */

G_BEGIN_DECLS

typedef struct _IBusLookupTable IBusLookupTable;

struct _IBusLookupTable {
    gint refcount;
    gint page_size;
    gint cursor_pos; 
    gboolean cursor_visible;
    GArray *candidates;
};

typedef struct _IBusCandidate IBusCandidate;

struct _IBusCandidate {
    gchar *text;
    IBusAttrList *attr_list;
};

GType                ibus_lookup_table_get_type ();
IBusLookupTable     *ibus_lookup_table_new      (gint                page_size,
                                                 gint                cursor_pos,
                                                 gboolean            cursor_visible);
IBusLookupTable     *ibus_lookup_table_copy     (IBusLookupTable    *table);
IBusLookupTable     *ibus_lookup_table_ref      (IBusLookupTable    *table);
void                 ibus_lookup_table_unref    (IBusLookupTable    *table);
void                 ibus_lookup_table_append_candidate
                                                (IBusLookupTable    *table,
                                                 const gchar        *text,
                                                 IBusAttrList       *attr_list);
G_END_DECLS
#endif

