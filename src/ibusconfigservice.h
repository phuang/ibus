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
#ifndef __IBUS_CONFIG_SERVICE_H_
#define __IBUS_CONFIG_SERVICE_H_

#include "ibuserror.h"
#include "ibusservice.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_CONFIG_SERVICE             \
    (ibus_config_service_get_type ())
#define IBUS_CONFIG_SERVICE(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_CONFIG_SERVICE, IBusConfigService))
#define IBUS_CONFIG_SERVICE_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_CONFIG_SERVICE, IBusConfigServiceClass))
#define IBUS_IS_CONFIG_SERVICE(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_CONFIG_SERVICE))
#define IBUS_IS_CONFIG_SERVICE_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_CONFIG_SERVICE))
#define IBUS_CONFIG_SERVICE_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_CONFIG_SERVICE, IBusConfigServiceClass))

G_BEGIN_DECLS

typedef struct _IBusConfigService IBusConfigService;
typedef struct _IBusConfigServiceClass IBusConfigServiceClass;

struct _IBusConfigService {
    IBusService parent;

    /* instance members */
};

struct _IBusConfigServiceClass {
    IBusServiceClass parent;

    /* class members */
    gboolean    (* set_value) (IBusConfigService    *config,
                               const gchar          *section,
                               const gchar          *name,
                               const GValue         *value,
                               IBusError           **error);
    gboolean    (* get_value) (IBusConfigService    *config,
                               const gchar          *section,
                               const gchar          *name,
                               GValue               *value,
                               IBusError           **error);

    /*< private >*/
    /* padding */
    gpointer pdummy[14];
};

GType                ibus_config_service_get_type   (void);
IBusConfigService   *ibus_config_service_new        (IBusConnection     *connection);
void                 ibus_config_service_value_changed
                                                    (IBusConfigService  *config,
                                                     const gchar        *section,
                                                     const gchar        *name,
                                                     const GValue       *value);

G_END_DECLS
#endif

