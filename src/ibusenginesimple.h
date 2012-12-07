/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#if !defined (__IBUS_H_INSIDE__) && !defined (IBUS_COMPILATION)
#error "Only <ibus.h> can be included directly"
#endif

/**
 * SECTION: ibussimpleengine
 * @short_description: Input method engine supporting table-based input method
 * @title: IBusEngineSimple
 * @stability: Stable
 *
 * An IBusEngineSimple provides table-based input method logic.
 *
 * see_also: #IBusEngine
 */
#ifndef __IBUS_ENGINE_SIMPLE_H__
#define __IBUS_ENGINE_SIMPLE_H__

#include "ibusengine.h"

G_BEGIN_DECLS

#define IBUS_MAX_COMPOSE_LEN 7

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_ENGINE_SIMPLE             \
    (ibus_engine_simple_get_type ())
#define IBUS_ENGINE_SIMPLE(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_ENGINE_SIMPLE, IBusEngineSimple))
#define IBUS_ENGINE_SIMPLE_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_ENGINE_SIMPLE, IBusEngineSimpleClass))
#define IBUS_IS_ENGINE_SIMPLE(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_ENGINE_SIMPLE))
#define IBUS_IS_ENGINE_SIMPLE_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_ENGINE_SIMPLE))
#define IBUS_ENGINE_SIMPLE_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_ENGINE_SIMPLE, IBusEngineSimpleClass))

typedef struct _IBusEngineSimple IBusEngineSimple;
typedef struct _IBusEngineSimpleClass IBusEngineSimpleClass;
typedef struct _IBusEngineSimplePrivate IBusEngineSimplePrivate;

/**
 * IBusEngineSimple:
 *
 * IBusEngineSimple properties.
 */
struct _IBusEngineSimple {
    /*< private >*/
    IBusEngine parent;
    IBusEngineSimplePrivate *priv;

    /* instance members */
    /*< public >*/
};

struct _IBusEngineSimpleClass {
    /*< private >*/
    IBusEngineClass parent;

    /* class members */
    /*< public >*/
    /* signals */
    
    /*< private >*/
    /* padding */
    gpointer pdummy[8];
};

GType   ibus_engine_simple_get_type       (void);

/**
 * ibus_engine_simple_add_table:
 * @simple: An IBusEngineSimple.
 * @data: (element-type guint16) (array): The table which must be available
 *      during the whole life of the simple engine.
 * @max_seq_len: Maximum length of a swquence in the table (cannot be greater
 *      than %IBUS_MAX_COMPOSE_LEN)
 *
 * Adds an additional table to search to the engine. Each row of the table
 * consists of max_seq_len key symbols followed by two guint16 interpreted as
 * the high and low words of a gunicode value. Tables are searched starting from
 * the last added.
 *
 * The table must be sorted in dictionary order on the numeric value of the key
 * symbol fields. (Values beyond the length of the sequence should be zero.)
 */
void    ibus_engine_simple_add_table      (IBusEngineSimple     *simple,
                                           const guint16        *data,
                                           gint                  max_seq_len,
                                           gint                  n_seqs);

G_END_DECLS

#endif // __IBUS_ENGINE_SIMPLE_H__
