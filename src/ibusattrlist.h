/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* IBus - The Input Bus
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
 * SECTION: ibusattrlist
 * @Title: IBusAttrList
 * @Short_description: AttrList of IBusText.
 * @See_also: #IBusAttribute #IBusText
 * @Stability: Stable
 *
 */
#ifndef __IBUS_ATTRIBUTE_LIST_H_
#define __IBUS_ATTRIBUTE_LIST_H_

#include "ibusattribute.h"

G_BEGIN_DECLS

/*
 * Type macros.
 */
/* define IBusAttrList macros */
#define IBUS_TYPE_ATTR_LIST             \
    (ibus_attr_list_get_type ())
#define IBUS_ATTR_LIST(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_ATTR_LIST, IBusAttrList))
#define IBUS_ATTR_LIST_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_ATTR_LIST, IBusAttrListClass))
#define IBUS_IS_ATTR_LIST(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_ATTR_LIST))
#define IBUS_IS_ATTR_LIST_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_ATTR_LIST))
#define IBUS_ATTR_LIST_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_ATTR_LIST, IBusAttrListClass))


typedef struct _IBusAttrList IBusAttrList;
typedef struct _IBusAttrListClass IBusAttrListClass;

/**
 * IBusAttrList:
 * @attributes: GArray that holds #IBusAttribute.
 *
 * Array of IBusAttribute.
 */
struct _IBusAttrList {
    IBusSerializable parent;

    /*< public >*/
    GArray *attributes;
};

struct _IBusAttrListClass {
    IBusSerializableClass parent;
};

/**
 * ibus_attr_list_get_type:
 * @returns: GType of IBusAttrList.
 *
 * Returns GType of IBusAttrList.
 */
GType                ibus_attr_list_get_type    ();

/**
 * ibus_attr_list_new:
 * @returns: A newly allocated IBusAttrList.
 *
 * New an IBusAttrList.
 */
IBusAttrList        *ibus_attr_list_new         ();

/**
 * ibus_attr_list_append:
 * @attr_list: An IBusAttrList instance.
 * @attr: The IBusAttribute instance to be appended.
 *
 * Append an IBusAttribute to IBusAttrList, and increase reference.
 */
void                 ibus_attr_list_append      (IBusAttrList   *attr_list,
                                                 IBusAttribute  *attr);
/**
 * ibus_attr_list_get:
 * @attr_list: An IBusAttrList instance.
 * @index: Index of the @attr_list.
 * @returns: (transfer none): IBusAttribute at given index, NULL if no such IBusAttribute.
 *
 * Returns IBusAttribute at given index. Borrowed reference.
 */
IBusAttribute       *ibus_attr_list_get         (IBusAttrList   *attr_list,
                                                 guint           index);

G_END_DECLS
#endif

