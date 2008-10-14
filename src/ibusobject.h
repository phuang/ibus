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
#ifndef __IBUS_OBJECT_H_
#define __IBUS_OBJECT_H_

#include <glib-object.h>
#include "ibusmarshalers.h"
#include "ibusdebug.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_OBJECT             \
    (ibus_object_get_type ())
#define IBUS_OBJECT(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_OBJECT, IBusObject))
#define IBUS_OBJECT_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_OBJECT, IBusObjectClass))
#define IBUS_IS_OBJECT(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_OBJECT))
#define IBUS_IS_OBJECT_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_OBJECT))
#define IBUS_OBJECT_GET_CLASS(obj)   \
    (G_TYPE_CHECK_GET_CLASS ((obj), IBUS_TYPE_OBJECT, IBusObjectClass))

#if 0
#define DEBUG_FUNCTION_IN   g_debug("%s IN", __FUNCTION__);
#define DEBUG_FUNCTION_OUT  g_debug("%s OUT", __FUNCTION__);
#else
#define DEBUG_FUNCTION_IN
#define DEBUG_FUNCTION_OUT
#endif

G_BEGIN_DECLS

typedef struct _IBusObject IBusObject;
typedef struct _IBusObjectClass IBusObjectClass;

struct _IBusObject {
  GObject parent;
  /* instance members */
};

struct _IBusObjectClass {
  GObjectClass parent;

  /* class members */
  void (* destroy)        (IBusObject   *client);
};

GType           ibus_object_get_type            (void);
IBusObject     *ibus_object_new                 (void);
void            ibus_object_destroy             (IBusObject     *object);
gboolean        ibus_object_is_destroyed        (IBusObject     *object);

G_END_DECLS
#endif

