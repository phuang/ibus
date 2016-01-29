/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2016 Takao Fujiwara <takao.fujiwara1@gmail.com>
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

extern const IBusComposeTableCompact ibus_compose_table_compact;

gboolean ibus_check_algorithmically (const guint16              *compose_buffer,
                                     gint                        n_compose,
                                     gunichar                   *output);
gboolean ibus_check_compact_table   (const IBusComposeTableCompact
                                                                *table,
                                     guint16                    *compose_buffer,
                                     gint                        n_compose,
                                     gboolean                   *compose_finish,
                                     gunichar                   *output_char);

G_END_DECLS


#endif /* __IBUS_IM_CONTEXT_SIMPLE_PRIVATE_H__ */
