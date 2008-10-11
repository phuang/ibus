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
#include "ibuslookuptable.h"

GType
ibus_lookup_table_get_type ()
{
    static GType type = 0;
    if (type == 0) {
        type = g_boxed_type_register_static ("IBusLookupTable",
                    (GBoxedCopyFunc)ibus_lookup_table_copy,
                    (GBoxedFreeFunc)ibus_lookup_table_free);
    }
    return type;
}

IBusLookupTable *
ibus_lookup_table_new ()
{
    IBusLookupTable *table = g_slice_new (IBusLookupTable);
    return table;
}

IBusLookupTable *
ibus_lookup_table_copy (IBusLookupTable *table)
{
    return ibus_lookup_table_new ();
}

void
ibus_lookup_table_free (IBusLookupTable *table)
{
    g_assert (table != NULL);
    g_slice_free (IBusLookupTable, table);
}
