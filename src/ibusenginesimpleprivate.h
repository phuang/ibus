/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2016-2019 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2016 Red Hat, Inc.
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __IBUS_ENGINE_SIMPLE_PRIVATE_H__
#define __IBUS_ENGINE_SIMPLE_PRIVATE_H__

#include <glib.h>


G_BEGIN_DECLS

extern const IBusComposeTableCompactEx ibus_compose_table_compact;
extern const IBusComposeTableCompactEx ibus_compose_table_compact_32bit;

struct _IBusComposeTablePrivate
{
    guint16 *data_first;
    guint32 *data_second;
    gsize first_n_seqs;
    gsize second_size;
};

struct _IBusComposeTableCompactPrivate
{
    const guint32 *data2;
};

gboolean ibus_check_algorithmically (const guint16              *compose_buffer,
                                     gint                        n_compose,
                                     gunichar                   *output);
gboolean ibus_compose_table_check   (const IBusComposeTableEx   *table,
                                     guint16                    *compose_buffer,
                                     gint                        n_compose,
                                     gboolean                   *compose_finish,
                                     gboolean                   *compose_match,
                                     GString                    *output,
                                     gboolean                    is_32bit);
gboolean ibus_compose_table_compact_check
                                    (const IBusComposeTableCompactEx
                                                                *table,
                                     guint16                    *compose_buffer,
                                     gint                        n_compose,
                                     gboolean                   *compose_finish,
                                     gunichar                  **output_chars);
gunichar ibus_keysym_to_unicode     (guint16                     keysym,
                                     gboolean                    combining);

G_END_DECLS


#endif /* __IBUS_IM_CONTEXT_SIMPLE_PRIVATE_H__ */
