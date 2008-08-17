/* vim:set et ts=4: */
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
#ifndef __IBUS_ATTRIBUTE_H_
#define __IBUS_ATTRIBUTE_H_

#include <gtk/gtk.h>
/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_IM_CLIENT             \
    (ibus_im_client_get_type ())
#define IBUS_IM_CLIENT(obj)             \
    (GTK_CHECK_CAST ((obj), IBUS_TYPE_IM_CLIENT, IBusIMClient))
#define IBUS_IM_CLIENT_CLASS(klass)     \
    (GTK_CHECK_CLASS_CAST ((klass), IBUS_TYPE_IM_CLIENT, IBusIMClientClass))
#define IBUS_IS_IM_CLIENT(obj)          \
    (GTK_CHECK_TYPE ((obj), IBUS_TYPE_IM_CLIENT))
#define IBUS_IS_IM_CLIENT_CLASS(klass)  \
    (GTK_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_IM_CLIENT))
#define IBUS_IM_CLIENT_GET_CLASS(obj)   \
    (GTK_CHECK_GET_CLASS ((obj), IBUS_TYPE_IM_CLIENT, IBusIMClientClass))


#define IBUS_ATTR_TYPE_UNDERLINE    1
#define IBUS_ATTR_TYPE_FOREGROUND   2
#define IBUS_ATTR_TYPE_BACKGROUND   3

#define IBUS_ATTR_UNDERLINE_NONE    0
#define IBUS_ATTR_UNDERLINE_SINGLE  1
#define IBUS_ATTR_UNDERLINE_DOUBLE  2
#define IBUS_ATTR_UNDERLINE_LOW     3

G_BEGIN_DECLS
typedef struct _IBusAttribute IBusAttribute;
typedef struct _IBusAttrList IBusAttrList;

struct _IBusAttribute {
    guint type;
    guint value;
    guint start_index;
    guint end_index;
};

struct _IBusAttribute {
    
    GArray *attributes;
};

IBusAttribute   *ibus_attribute_new        (guint       type,
                                            guint       value,
                                            guint       start_index,
                                            guint       end_index);
void            *ibus_attribute_free       (IBusAttribute
                                                       *attr);
IBusAttribute   *ibus_attr_underline_new   (guint       underline_type,
                                            guint       start_index,
                                            guint       end_index);
IBusAttribute   *ibus_attr_foreground_new  (guint       color,
                                            guint       start_index,
                                            guint       end_index);
IBusAttribute   *ibus_attr_background_new  (guint       color,
                                            guint       start_index,
                                            guint       end_index);


G_END_DECLS
#endif

