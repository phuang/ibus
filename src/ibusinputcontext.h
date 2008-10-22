/* vim:set et sts=4: */
/* ibus - The Input Bus
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
#ifndef __INPUT_CONTEXT_H_
#define __INPUT_CONTEXT_H_

#include "ibusproxy.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_INPUT_CONTEXT             \
    (ibus_input_context_get_type ())
#define IBUS_INPUT_CONTEXT(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_INPUT_CONTEXT, IBusInputContext))
#define IBUS_INPUT_CONTEXT_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_INPUT_CONTEXT, IBusInputContextClass))
#define IBUS_IS_INPUT_CONTEXT(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_INPUT_CONTEXT))
#define IBUS_IS_INPUT_CONTEXT_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_INPUT_CONTEXT))
#define IBUS_INPUT_CONTEXT_GET_CLASS(obj)   \
    (G_TYPE_CHECK_GET_CLASS ((obj), IBUS_TYPE_INPUT_CONTEXT, IBusInputContextClass))

G_BEGIN_DECLS

typedef struct _IBusInputContext IBusInputContext;
typedef struct _IBusInputContextClass IBusInputContextClass;

struct _IBusInputContext {
  IBusProxy parent;
  /* instance members */
};

struct _IBusInputContextClass {
    IBusProxyClass parent;
    /* class members */
};

GType        ibus_input_context_get_type    (void);
IBusInputContext
            *ibus_input_context_new         (const gchar        *path,
                                             IBusConnection     *connection);
gboolean     ibus_input_context_process_key_event
                                            (IBusInputContext   *context,
                                             guint32             keyval,
                                             gboolean            is_press,
                                             guint32             state);
void         ibus_input_context_set_cursor_location
                                            (IBusInputContext   *context,
                                             gint32              x,
                                             gint32              y,
                                             gint32              w,
                                             gint32              h);
void         ibus_input_context_focus_in    (IBusInputContext   *context);
void         ibus_input_context_focus_out   (IBusInputContext   *context);
void         ibus_input_context_reset       (IBusInputContext   *context);
void         ibus_input_context_enable      (IBusInputContext   *context);
void         ibus_input_context_disable     (IBusInputContext   *context);
void         ibus_input_context_destroy     (IBusInputContext   *context);

G_END_DECLS
#endif

