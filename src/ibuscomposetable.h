/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2013-2014 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2013-2016 Takao Fujiwara <takao.fujiwara1@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef __IBUS_COMPOSETABLE_H_
#define __IBUS_COMPOSETABLE_H_

#include <glib.h>


G_BEGIN_DECLS

typedef struct _IBusComposeTable IBusComposeTable;
typedef struct _IBusComposeTableCompact IBusComposeTableCompact;

struct _IBusComposeTable
{
    guint16 *data;
    gint max_seq_len;
    gint n_seqs;
    guint32 id;
};

struct _IBusComposeTableCompact
{
    const guint16 *data;
    gint max_seq_len;
    gint n_index_size;
    gint n_index_stride;
};

IBusComposeTable *ibus_compose_table_new_with_file (const gchar *compose_file);
GSList           *ibus_compose_table_list_add_array
                                                   (GSList
                                                                *compose_tables,
                                                    const guint16
                                                                *data,
                                                    gint         max_seq_len,
                                                    gint         n_seqs);
GSList           *ibus_compose_table_list_add_file (GSList
                                                                *compose_tables,
                                                    const gchar *compose_file);

G_BEGIN_DECLS
#endif
