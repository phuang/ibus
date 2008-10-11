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
#ifndef __IBUS_FACTORY_H_
#define __IBUS_FACTORY_H_

#include <dbus/dbus.h>
#include "ibusservice.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_FACTORY             \
    (ibus_factory_get_type ())
#define IBUS_FACTORY(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_FACTORY, IBusFactory))
#define IBUS_FACTORY_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_FACTORY, IBusFactoryClass))
#define IBUS_IS_FACTORY(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_FACTORY))
#define IBUS_IS_FACTORY_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_FACTORY))
#define IBUS_FACTORY_GET_CLASS(obj)   \
    (G_TYPE_CHECK_GET_CLASS ((obj), IBUS_TYPE_FACTORY, IBusFactoryClass))

G_BEGIN_DECLS

typedef struct _IBusFactory IBusFactory;
typedef struct _IBusFactoryClass IBusFactoryClass;

struct _IBusFactory {
  IBusService parent;
  /* instance members */
};

struct _IBusFactoryClass {
  IBusServiceClass parent;

  /* class members */
};

GType        ibus_factory_get_type          (void);
IBusFactory *ibus_factory_new               (const gchar    *path);
gboolean     ibus_factory_handle_message    (IBusFactory    *factory,
                                             IBusConnection *connection,
                                             DBusMessage    *message);

G_END_DECLS
#endif

