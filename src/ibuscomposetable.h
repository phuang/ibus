/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2013-2014 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2013-2015 Takao Fujiwara <takao.fujiwara1@gmail.com>
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

/* if ibus_compose_seqs[N - 1] is an outputed compose character,
 * ibus_compose_seqs[N * 2 - 1] is also an outputed compose character.
 * and ibus_compose_seqs[0] to ibus_compose_seqs[0 + N - 3] are the
 * sequences and call ibus_engine_simple_add_table:
 * ibus_engine_simple_add_table(engine, ibus_compose_seqs,
 *                              N - 2, G_N_ELEMENTS(ibus_compose_seqs) / N)
 * The compose sequences are allowed within G_MAXUINT16 */

#define IBUS_MAX_COMPOSE_LEN 7

typedef struct _IBusComposeTable IBusComposeTable;
struct _IBusComposeTable
{
    const guint16 *data;
    gint max_seq_len;
    gint n_seqs;
};

typedef struct _IBusComposeTableCompact IBusComposeTableCompact;
struct _IBusComposeTableCompact
{
    const guint16 *data;
    gint max_seq_len;
    gint n_index_size;
    gint n_index_stride;
};

/* This file contains the table of the compose sequences,
 * static const guint16 ibus_compose_seqs_compact[] = {}
 * IT is generated from the compose-parse.py script.
 */
#include "gtkimcontextsimpleseqs.h"

/* From the values below, the value 30 means the number of different first keysyms
 * that exist in the Compose file (from Xorg). When running compose-parse.py without
 * parameters, you get the count that you can put here. Needed when updating the
 * gtkimcontextsimpleseqs.h header file (contains the compose sequences).
 */
static const IBusComposeTableCompact ibus_compose_table_compact = {
    gtk_compose_seqs_compact,
    5,
    30,
    6
};

IBusComposeTable* ibus_compose_table_new_with_file (const gchar *compose_file);
#endif
