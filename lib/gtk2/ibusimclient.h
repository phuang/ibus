/* vim:set et ts=4: */
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
#ifndef __IBUS_IM_CLIENT_H_
#define __IBUS_IM_CLIENT_H_

#include <glib-object.h>
/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_IM_CLIENT             \
    (ibus_im_client_get_type ())
#define IBUS_IM_CLIENT(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_IM_CLIENT, IBusIMClient))
#define IBUS_IM_CLIENT_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_IM_CLIENT, IBusIMClientClass))
#define IBUS_IS_IM_CLIENT(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_IM_CLIENT))
#define IBUS_IS_IM_CLIENT_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_IM_CLIENT))
#define IBUS_IM_CLIENT_GET_CLASS(obj)   \
    (G_TYPE_CHECK_GET_CLASS ((obj), IBUS_TYPE_IM_CLIENT, IBusIMClientClass))

#define IBUS_FORWARD_MASK (1 << 25)

#if 0
#define DEBUG_FUNCTION_IN   g_debug("%s IN", __FUNCTION__);
#define DEBUG_FUNCTION_OUT  g_debug("%s OUT", __FUNCTION__);
#else
#define DEBUG_FUNCTION_IN
#define DEBUG_FUNCTION_OUT
#endif



#define IBUS_DBUS_SERVICE    "org.freedesktop.ibus"
#define IBUS_DBUS_INTERFACE  "org.freedesktop.ibus.Manager"
#define IBUS_DBUS_PATH       "/org/freedesktop/ibus/Manager"

G_BEGIN_DECLS
typedef struct _IBusIMClient IBusIMClient;
typedef struct _IBusIMClientClass IBusIMClientClass;
typedef struct _IBusIMClientPrivate IBusIMClientPrivate;

struct _IBusIMClient {
  GObject parent;
  /* instance members */
  IBusIMClientPrivate *priv;
};

struct _IBusIMClientClass {
  GObjectClass parent;
  /* class members */
  void (* connected)        (IBusIMClient   *client);
  void (* disconnected)     (IBusIMClient   *client);
  void (* commit_string)    (IBusIMClient   *client,
                             const gchar    *ic,
                             const gchar    *text);
  void (* update_preedit)   (IBusIMClient   *client,
                             const gchar    *ic,
                             const gchar    *text,
                             gpointer       *attrs,
                             gint            cursor_pos,
                             gboolean        visible);
  void (* show_preedit)     (IBusIMClient   *client,
                             const gchar    *ic);
  void (* hide_preedit)     (IBusIMClient   *client,
                             const gchar    *ic);
  void (* enabled)          (IBusIMClient   *client,
                             const gchar    *ic);
  void (* disabled)         (IBusIMClient   *client,
                             const gchar    *ic);
};

GType           ibus_im_client_get_type          (void);
void            ibus_im_client_register_type     (GTypeModule     *type_module);
IBusIMClient   *ibus_im_client_new               (void);
const gchar    *ibus_im_client_create_input_context
                                                 (IBusIMClient    *client);
void            ibus_im_client_shutdown          (void);
void            ibus_im_client_focus_in          (IBusIMClient    *client,
                                                  const gchar     *ic);
void            ibus_im_client_focus_out         (IBusIMClient    *client,
                                                  const gchar     *ic);
void            ibus_im_client_reset             (IBusIMClient    *client,
                                                  const gchar     *ic);
gboolean        ibus_im_client_filter_keypress   (IBusIMClient    *client,
                                                  const gchar     *ic,
                                                  GdkEventKey     *key,
                                                  gboolean         block);
void            ibus_im_client_set_cursor_location
                                                 (IBusIMClient    *client,
                                                  const gchar     *ic,
                                                  GdkRectangle    *area);
void            ibus_im_client_set_use_preedit   (IBusIMClient    *client,
                                                  const gchar     *ic,
                                                  gboolean         use_preedit);
gboolean        ibus_im_client_is_enabled        (IBusIMClient    *client);
void            ibus_im_client_release_input_context
                                                 (IBusIMClient    *client,
                                                  const gchar     *ic);
void            ibus_im_client_kill_daemon       (IBusIMClient    *client);
gboolean        ibus_im_client_get_connected     (IBusIMClient    *client);


G_END_DECLS
#endif

