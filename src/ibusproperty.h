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

#include "ibusserializable.h"
#include "ibustext.h"

G_BEGIN_DECLS

/*
 * Type macros.
 */
#define IBUS_TYPE_PROPERTY          (ibus_property_get_type ())
#define IBUS_TYPE_PROP_LIST         (ibus_prop_list_get_type ())

/* define IBusProperty macros */
#define IBUS_TYPE_PROPERTY             \
    (ibus_property_get_type ())
#define IBUS_PROPERTY(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_PROPERTY, IBusProperty))
#define IBUS_PROPERTY_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_PROPERTY, IBusPropertyClass))
#define IBUS_IS_PROPERTY(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_PROPERTY))
#define IBUS_IS_PROPERTY_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_PROPERTY))
#define IBUS_PROPERTY_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_PROPERTY, IBusPropertyClass))

/* define IBusPropList macros */
#define IBUS_TYPE_PROP_LIST             \
    (ibus_prop_list_get_type ())
#define IBUS_PROP_LIST(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_PROP_LIST, IBusPropList))
#define IBUS_PROP_LIST_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_PROP_LIST, IBusPropListClass))
#define IBUS_IS_PROP_LIST(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_PROP_LIST))
#define IBUS_IS_PROP_LIST_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_PROP_LIST))
#define IBUS_PROP_LIST_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_PROP_LIST, IBusPropListClass))


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


typedef struct _IBusProperty IBusProperty;
typedef struct _IBusPropertyClass IBusPropertyClass;
typedef struct _IBusPropList IBusPropList;
typedef struct _IBusPropListClass IBusPropListClass;

struct _IBusProperty {
    IBusSerializable parent;

    gchar    *key;
    gchar    *icon;
    IBusText *label;
    IBusText *tooltip;

    gboolean sensitive;
    gboolean visible;
    guint type;
    guint state;

    IBusPropList *sub_props;
};

struct _IBusPropertyClass {
    IBusSerializableClass parent;
};

struct _IBusPropList {
    IBusSerializable parent;

    GArray *properties;
};

struct _IBusPropListClass {
    IBusSerializableClass parent;
};

GType            ibus_property_get_type     ();
IBusProperty    *ibus_property_new          (const gchar    *key,
                                             IBusPropType    type,
                                             IBusText       *label,
                                             const gchar    *icon,
                                             IBusText       *tooltip,
                                             gboolean        sensitive,
                                             gboolean        visible,
                                             IBusPropState   state,
                                             IBusPropList   *prop_list);
void             ibus_property_set_label    (IBusProperty   *prop,
                                             IBusText       *label);
void             ibus_property_set_visible  (IBusProperty   *prop,
                                             gboolean        visible);
void             ibus_property_set_sub_props(IBusProperty   *prop,
                                             IBusPropList   *prop_list);

GType            ibus_prop_list_get_type    ();
IBusPropList    *ibus_prop_list_new         ();
void             ibus_prop_list_append      (IBusPropList   *prop_list,
                                             IBusProperty   *prop);
IBusProperty    *ibus_prop_list_get         (IBusPropList   *prop_list,
                                             guint           index);
gboolean         ibus_prop_list_update_property
                                            (IBusPropList   *prop_list,
                                             IBusProperty   *prop);
G_END_DECLS
#endif
