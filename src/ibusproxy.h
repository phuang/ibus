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
#ifndef __IBUS_PROXY_H_
#define __IBUS_PROXY_H_

#include <dbus/dbus.h>
#include "ibusobject.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_PROXY             \
    (ibus_proxy_get_type ())
#define IBUS_PROXY(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_PROXY, IBusProxy))
#define IBUS_PROXY_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_PROXY, IBusProxyClass))
#define IBUS_IS_PROXY(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_PROXY))
#define IBUS_IS_PROXY_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_PROXY))
#define IBUS_PROXY_GET_CLASS(obj)   \
    (G_TYPE_CHECK_GET_CLASS ((obj), IBUS_TYPE_PROXY, IBusProxyClass))

G_BEGIN_DECLS

typedef struct _IBusProxy IBusProxy;
typedef struct _IBusProxyClass IBusProxyClass;

struct _IBusProxy {
  IBusObject parent;
  /* instance members */
};

struct _IBusProxyClass {
  IBusObjectClass parent;

  /* class members */
  gboolean  (* dbus_signal)     (IBusProxy   *proxy,
                                 DBusMessage      *message);
};

GType        ibus_proxy_get_type        (void);
IBusProxy   *ibus_proxy_new             (void);
gboolean     ibus_proxy_handle_signal   (IBusProxy      *proxy,
                                         DBusMessage    *message);

G_END_DECLS
#endif

