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
#ifndef __IBUS_CONFIG_H_
#define __IBUS_CONFIG_H_

#include "ibusproxy.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_CONFIG             \
    (ibus_config_get_type ())
#define IBUS_CONFIG(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_CONFIG, IBusConfig))
#define IBUS_CONFIG_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_CONFIG, IBusConfigClass))
#define IBUS_IS_CONFIG(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_CONFIG))
#define IBUS_IS_CONFIG_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_CONFIG))
#define IBUS_CONFIG_GET_CLASS(obj)   \
    (G_TYPE_CHECK_GET_CLASS ((obj), IBUS_TYPE_CONFIG, IBusConfigClass))

G_BEGIN_DECLS

typedef struct _IBusConfig IBusConfig;
typedef struct _IBusConfigClass IBusConfigClass;

struct _IBusConfig {
  IBusProxy parent;
  /* instance members */
};

struct _IBusConfigClass {
    IBusProxyClass parent;
    /* class members */
};

GType        ibus_config_get_type    (void);
IBusConfig
            *ibus_config_new         (const gchar        *path,
                                             IBusConnection     *connection);
gboolean     ibus_config_process_key_event
                                            (IBusConfig   *context,
                                             guint32             keyval,
                                             gboolean            is_press,
                                             guint32             state);
void         ibus_config_set_cursor_location
                                            (IBusConfig   *context,
                                             gint32              x,
                                             gint32              y,
                                             gint32              w,
                                             gint32              h);
void         ibus_config_set_capabilities
                                            (IBusConfig   *context,
                                             guint32             capabilites);
void         ibus_config_focus_in    (IBusConfig   *context);
void         ibus_config_focus_out   (IBusConfig   *context);
void         ibus_config_reset       (IBusConfig   *context);
void         ibus_config_enable      (IBusConfig   *context);
void         ibus_config_disable     (IBusConfig   *context);
void         ibus_config_destroy     (IBusConfig   *context);

G_END_DECLS
#endif

