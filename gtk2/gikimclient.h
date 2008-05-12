/* vim:set et ts=4: */
/* GIK - The G Input Toolkit
 * Copyright (C) 2008-2009 Huang Peng
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
#ifndef __GIK_IM_CLIENT_H_
#define __GIK_IM_CLIENT_H_

#include <gtk/gtk.h>

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define GIK_TYPE_IM_CLIENT             \
    (gik_im_client_get_type ())
#define GIK_IM_CLIENT(obj)             \
    (GTK_CHECK_CAST ((obj), GIK_TYPE_IM_CLIENT, GikIMClient))
#define GIK_IM_CLIENT_CLASS(klass)     \
    (GTK_CHECK_CLASS_CAST ((klass), GIK_TYPE_IM_CLIENT, GikIMClientClass))
#define GIK_IS_IM_CLIENT(obj)          \
    (GTK_CHECK_TYPE ((obj), GIK_TYPE_IM_CLIENT))
#define GIK_IS_IM_CLIENT_CLASS(klass)  \
    (GTK_CHECK_CLASS_TYPE ((klass), GIK_TYPE_IM_CLIENT))
#define GIK_IM_CLIENT_GET_CLASS(obj)   \
    (GTK_CHECK_GET_CLASS ((obj), GIK_TYPE_IM_CLIENT, GikIMClientClass))

#if 0
#define DEBUG_FUNCTION_IN   g_debug("%s IN", __FUNCTION__);
#define DEBUG_FUNCTION_OUT   g_debug("%s OUT", __FUNCTION__);
#else
#define DEBUG_FUNCTION_IN
#define DEBUG_FUNCTION_OUT
#endif



#define GIK_DBUS_SERVICE    "org.freedesktop.gik"
#define GIK_DBUS_INTERFACE  "org.freedesktop.gik.Manager"
#define GIK_DBUS_PATH       "/org/freedesktop/gik/Manager"

G_BEGIN_DECLS
typedef struct _GikIMClient GikIMClient;
typedef struct _GikIMClientClass GikIMClientClass;
typedef struct _GikIMClientPrivate GikIMClientPrivate;

struct _GikIMClient {
  GtkObject parent;
  /* instance members */
  GikIMClientPrivate *priv;
};

struct _GikIMClientClass {
  GtkObjectClass parent;
  /* class members */
};

GType           gik_im_client_get_type          (void);
GikIMClient     *gik_im_client_get_client       (void);
void            gik_im_client_register_type     (GTypeModule    *type_module);
void            gik_im_client_shutdown          (void);
void            gik_im_client_focus_in          (GikIMClient    *client);
void            gik_im_client_focus_out         (GikIMClient    *client);
void            gik_im_client_set_im_context    (GikIMClient    *client,
                                                 GtkIMContext   *context);
GtkIMContext   *gik_im_client_get_im_context    (GikIMClient    *client);
void            gik_im_client_reset             (GikIMClient    *client);
gboolean        gik_im_client_filter_keypress   (GikIMClient    *client,
                                                 GdkEventKey    *key);
gboolean        gik_im_client_get_preedit_string
                                                (GikIMClient    *client,
                                                 gchar          **str,
                                                 PangoAttrList  **attrs,
                                                 gint           *cursor_pos);
void            gik_im_client_set_cursor_location
                                                (GikIMClient    *client,
                                                 GdkRectangle   *area);
gboolean        gik_im_client_is_enabled        (GikIMClient    *client);


G_END_DECLS
#endif

