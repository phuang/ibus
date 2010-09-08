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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/**
 * SECTION: ibusobject
 * @short_description: Base object of IBus.
 * @title: IBusObject
 * @stability: Stable
 *
 * IBusObject is the base object for all objects in IBus.
 */
#ifndef __IBUS_OBJECT_H_
#define __IBUS_OBJECT_H_

#include <glib-object.h>
#include "ibusmarshalers.h"
#include "ibustypes.h"
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
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_OBJECT, IBusObjectClass))

typedef enum {
    IBUS_IN_DESTRUCTION = (1 << 0),
    IBUS_DESTROYED      = (1 << 1),
    IBUS_RESERVED_1     = (1 << 2),
    IBUS_RESERVED_2     = (1 << 3),
} IBusObjectFlags;

#define IBUS_OBJECT_FLAGS(obj)             (IBUS_OBJECT (obj)->flags)
#define IBUS_OBJECT_SET_FLAGS(obj,flag)    G_STMT_START{ (IBUS_OBJECT_FLAGS (obj) |= (flag)); }G_STMT_END
#define IBUS_OBJECT_UNSET_FLAGS(obj,flag)  G_STMT_START{ (IBUS_OBJECT_FLAGS (obj) &= ~(flag)); }G_STMT_END
#define IBUS_OBJECT_DESTROYED(obj)         (IBUS_OBJECT_FLAGS (obj) & IBUS_DESTROYED)

G_BEGIN_DECLS

typedef struct _IBusObject IBusObject;
typedef struct _IBusObjectClass IBusObjectClass;
/**
 * IBusObject:
 *
 * All the fields in the <structname>IBusObject</structname> structure are
 * private to the #IBusObject and should never be accessed directly.
 */
struct _IBusObject {
  GInitiallyUnowned parent;
  /* instance members */
  guint32 flags;
};

typedef void ( *IBusObjectDestroyFunc) (IBusObject *);

struct _IBusObjectClass {
    GInitiallyUnownedClass parent;

    /* signals */
    void (* destroy)        (IBusObject   *object);

    /*< private >*/
    /* padding */
    gpointer pdummy[7];
};

GType           ibus_object_get_type            (void);

/**
 * ibus_object_new:
 * @returns: A newly allocated IBusObject
 *
 * New an IBusObject.
 */
IBusObject     *ibus_object_new                 (void);

/**
 * ibus_object_destroy:
 * @object: an #IBusObject to destroy.
 *
 * Emit the "destory" signal notifying all reference holders that they should
 * release the #IBusObject.
 *
 * The memory for the object itself won't be deleted until its reference count
 * actually drops to 0; ibus_object_destroy merely asks reference holders to
 * release their references. It does not free the object.
 */
void            ibus_object_destroy             (IBusObject     *object);

G_END_DECLS
#endif

