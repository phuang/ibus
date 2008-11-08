/* vim:set et sts=4: */
/* IBus - The Input Bus
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
#ifndef __IBUS_PROPERTY_H_
#define __IBUS_PROPERTY_H_

#include <glib-object.h>
#include "ibusmessage.h"

/*
 * Type macros.
 */
#define IBUS_TYPE_PROPERTY          (ibus_property_get_type ())
#define IBUS_TYPE_PROP_LIST         (ibus_prop_list_get_type ())

/* define GOBJECT macros */

typedef enum {
    PROP_TYPE_NORMAL = 0,
    PROP_TYPE_TOGGLE = 1,
    PROP_TYPE_RADIO = 2,
    PROP_TYPE_MENU = 3,
    PROP_TYPE_SEPARATOR = 4,
} IBusPropType;

typedef enum {
    PROP_STATE_UNCHECKED = 0,
    PROP_STATE_CHECKED = 1,
    PROP_STATE_INCONSISTENT = 2,
} IBusPropState;

G_BEGIN_DECLS
typedef struct _IBusProperty IBusProperty;
typedef struct _IBusPropList IBusPropList;

struct _IBusProperty {
    gchar *name;
    IBusPropType type;
    gchar *label;
    gchar *icon;
    gchar *tooltip;
    gboolean sensitive;
    gboolean visible;
    IBusPropState state;
    IBusPropList *sub_props;
};

struct _IBusPropList {
    gint   refcount;
    GArray *properties;
};

GType            ibus_property_get_type     ();
IBusProperty    *ibus_property_new          (const gchar    *name,
                                             IBusPropType    type,
                                             const gchar    *label,
                                             const gchar    *icon,
                                             const gchar    *tooltip,
                                             gboolean        sensitive,
                                             gboolean        visbile,
                                             IBusPropState   state,
                                             IBusPropList   *prop_list);
void             ibus_property_set_sub_props(IBusProperty   *prop,
                                             IBusPropList   *prop_list);
IBusProperty    *ibus_property_copy         (IBusProperty   *prop);
void             ibus_property_free         (IBusProperty   *prop);

GType            ibus_prop_list_get_type    ();
IBusPropList    *ibus_prop_list_new         ();
IBusPropList    *ibus_prop_list_copy        (IBusPropList   *prop_list);
IBusPropList    *ibus_prop_list_ref         (IBusPropList   *prop_list);
void             ibus_prop_list_unref       (IBusPropList   *prop_list);
void             ibus_prop_list_append      (IBusPropList   *prop_list,
                                             IBusProperty   *prop);
IBusProperty    *ibus_prop_list_get         (IBusPropList   *prop_list,
                                             guint           index);
G_END_DECLS
#endif

