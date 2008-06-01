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
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include "gikimcontext.h"
#include "gikimclient.h"

/* define GOBJECT macros */
#define GIK_TYPE_IM_CONTEXT             \
    (_gik_type_im_context)
#define GIK_IM_CONTEXT(obj)             \
    (GTK_CHECK_CAST ((obj), GIK_TYPE_IM_CONTEXT, GikIMContext))
#define GIK_IM_CONTEXT_CLASS(klass)     \
    (GTK_CHECK_CLASS_CAST ((klass), GIK_TYPE_IM_CONTEXT, GikIMContextClass))
#define GIK_IS_IM_CONTEXT(obj)          \
    (GTK_CHECK_TYPE ((obj), GIK_TYPE_IM_CONTEXT))
#define GIK_IS_IM_CONTEXT_CLASS(klass)  \
    (GTK_CHECK_CLASS_TYPE ((klass), GIK_TYPE_IM_CONTEXT))
#define GIK_IM_CONTEXT_GET_CLASS(obj)   \
    (GTK_CHECK_GET_CLASS ((obj), GIK_TYPE_IM_CONTEXT, GikIMContextClass))

#define CURRENT_CONTEXT (gik_im_client_get_im_context (priv->client))

/* GikIMContextPriv */
struct _GikIMContextPrivate {
    GtkIMContext *slave;
    GikIMClient *client;
    GdkWindow *client_window;
};


/* functions prototype */
static void     gik_im_context_class_init   (GikIMContextClass  *klass);
static void     gik_im_context_init         (GikIMContext       *obj);
static void     gik_im_context_finalize     (GObject            *obj);
static void     gik_im_context_reset        (GtkIMContext       *context);
static gboolean gik_im_context_filter_keypress
                                            (GtkIMContext       *context,
                                             GdkEventKey        *key);
static void     gik_im_context_focus_in     (GtkIMContext       *context);
static void     gik_im_context_focus_out    (GtkIMContext       *context);
static void     gik_im_context_get_preedit_string
                                            (GtkIMContext       *context,
                                             gchar              **str,
                                             PangoAttrList      **attrs,
                                             gint               *cursor_pos);
static void     gik_im_context_set_client_window
                                            (GtkIMContext       *context,
                                             GdkWindow          *client);
static void     gik_im_context_set_cursor_location
                                            (GtkIMContext       *context,
                                             GdkRectangle       *area);

/* callback functions for slave context */
static void     _slave_commit_cb            (GtkIMContext       *slave,
                                             gchar              *string,
                                             GikIMContext       *context);
static void     _slave_preedit_changed_cb   (GtkIMContext       *slave,
                                             GikIMContext       *context);
static void     _slave_preedit_start_cb     (GtkIMContext       *slave,
                                             GikIMContext       *context);
static void     _slave_preedit_end_cb       (GtkIMContext       *slave,
                                             GikIMContext       *context);
static void     _slave_retrieve_surrounding_cb
                                            (GtkIMContext       *slave,
                                             GikIMContext       *context);
static void     _slave_delete_surrounding_cb
                                            (GtkIMContext       *slave,
                                             gint               arg1,
                                             gint               arg2,
                                             GikIMContext       *context);



static GType                _gik_type_im_context = 0;
static GtkIMContextClass    *parent_class = NULL;

void 
gik_im_context_register_type (GTypeModule *type_module)
{
    static const GTypeInfo gik_im_context_info = {
        sizeof (GikIMContextClass),
        (GBaseInitFunc)        NULL,
        (GBaseFinalizeFunc)     NULL,
        (GClassInitFunc)     gik_im_context_class_init,
        NULL,            /* class finialize */
        NULL,            /* class data */
        sizeof (GikIMContext),
        0,
        (GInstanceInitFunc)    gik_im_context_init,
    };
    
    if (! _gik_type_im_context ) {
        _gik_type_im_context = 
            g_type_module_register_type (type_module,
                GTK_TYPE_IM_CONTEXT,
                "GikIMContext",
                &gik_im_context_info,
                (GTypeFlags)0);
    }
}

GtkIMContext *
gik_im_context_new (void)
{
    GikIMContext *obj;
    
    obj = GIK_IM_CONTEXT(g_object_new (GIK_TYPE_IM_CONTEXT, NULL));
    
    return GTK_IM_CONTEXT(obj);
}

static void 
gik_im_context_class_init     (GikIMContextClass *klass)
{
    GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    parent_class = (GtkIMContextClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (GikIMContextPrivate));

    im_context_class->reset = &gik_im_context_reset;
    im_context_class->focus_in = &gik_im_context_focus_in;
    im_context_class->focus_out = &gik_im_context_focus_out;
    im_context_class->filter_keypress = &gik_im_context_filter_keypress;
    im_context_class->get_preedit_string = &gik_im_context_get_preedit_string;
    im_context_class->set_client_window = &gik_im_context_set_client_window;
    im_context_class->set_cursor_location = &gik_im_context_set_cursor_location;
    gobject_class->finalize = &gik_im_context_finalize;
}

static void 
gik_im_context_init     (GikIMContext *obj)
{
    DEBUG_FUNCTION_IN;
    
    GError *error;
    GikIMContext *gik = GIK_IM_CONTEXT (obj);
    gik->priv = G_TYPE_INSTANCE_GET_PRIVATE (gik, GIK_TYPE_IM_CONTEXT, GikIMContextPrivate);
    
    gik->priv->client = gik_im_client_get_client ();
    gik->priv->client_window = NULL;

    // Create slave im context
    gik->priv->slave = gtk_im_context_simple_new ();
    g_signal_connect (gik->priv->slave, 
                "commit", G_CALLBACK (_slave_commit_cb), obj);
    g_signal_connect (gik->priv->slave, 
                "preedit-start", G_CALLBACK (_slave_preedit_start_cb), obj);
    g_signal_connect (gik->priv->slave,
                "preedit-end", G_CALLBACK (_slave_preedit_end_cb), obj);
    g_signal_connect (gik->priv->slave,
                "preedit-changed", G_CALLBACK (_slave_preedit_changed_cb), obj);
    g_signal_connect (gik->priv->slave,
                "retrieve-surrounding", G_CALLBACK (_slave_retrieve_surrounding_cb), obj);
    g_signal_connect (gik->priv->slave,
                "delete-surrounding", G_CALLBACK (_slave_delete_surrounding_cb), obj);
}

static void
gik_im_context_finalize (GObject *obj)
{
    DEBUG_FUNCTION_IN;

    GikIMContext *gik = GIK_IM_CONTEXT (obj);
    GikIMContextPrivate *priv = gik->priv;

    if (GTK_IM_CONTEXT (gik) == CURRENT_CONTEXT) {
        gik_im_client_focus_out (priv->client);
        gik_im_client_set_im_context (priv->client, NULL);
    }
    
    g_object_unref (priv->slave);
    g_object_unref (priv->client);
    
    G_OBJECT_CLASS(parent_class)->finalize (obj);
}

static gboolean
gik_im_context_filter_keypress (GtkIMContext *context,
                GdkEventKey  *event)
{
    DEBUG_FUNCTION_IN;
    
    GikIMContext *gik = GIK_IM_CONTEXT (context);
    GikIMContextPrivate *priv = gik->priv;

    if (context != CURRENT_CONTEXT)
        return FALSE;

    if (gik_im_client_filter_keypress (priv->client, event))
        return TRUE;
    return gtk_im_context_filter_keypress (priv->slave, event);
}

static void
gik_im_context_focus_in (GtkIMContext *context)
{
    DEBUG_FUNCTION_IN;

    GikIMContext *gik = GIK_IM_CONTEXT (context);
    GikIMContextPrivate *priv = gik->priv;

    if (context != CURRENT_CONTEXT) {
        gik_im_client_focus_out (priv->client); 
        gik_im_client_set_im_context (priv->client, context);
    }
    
    gik_im_client_focus_in (priv->client);
    gtk_im_context_focus_in (priv->slave);
}

static void
gik_im_context_focus_out (GtkIMContext *context)
{
    DEBUG_FUNCTION_IN;

    GikIMContext *gik = GIK_IM_CONTEXT (context);
    GikIMContextPrivate *priv = gik->priv;

    gik_im_client_focus_out (priv->client); 
    gtk_im_context_focus_out (priv->slave);
}

static void
gik_im_context_reset (GtkIMContext *context)
{
    DEBUG_FUNCTION_IN;

    GikIMContext *gik = GIK_IM_CONTEXT (context);
    GikIMContextPrivate *priv = gik->priv;
    
    if (context == CURRENT_CONTEXT) {
        gik_im_client_reset (priv->client);
    }
    gtk_im_context_reset (priv->slave);
}


static void     
gik_im_context_get_preedit_string (GtkIMContext   *context,
                   gchar         **str,
                   PangoAttrList **attrs,
                   gint           *cursor_pos)
{
    DEBUG_FUNCTION_IN;
    
    GikIMContext *gik = GIK_IM_CONTEXT (context);
    GikIMContextPrivate *priv = gik->priv;
   
    if (context == CURRENT_CONTEXT && 
        gik_im_client_is_enabled (priv->client)) {
        gik_im_client_get_preedit_string (priv->client, str, attrs, cursor_pos);
        return;
    }
    gtk_im_context_get_preedit_string (gik->priv->slave, str, attrs, cursor_pos);
}


static void
gik_im_context_set_client_window  (GtkIMContext *context, GdkWindow *client)
{
    DEBUG_FUNCTION_IN;
    
    GikIMContext *gik = GIK_IM_CONTEXT (context);
    GikIMContextPrivate *priv = gik->priv;
    priv->client_window = client;
    gtk_im_context_set_client_window (gik->priv->slave, client);
}

static void
gik_im_context_set_cursor_location (GtkIMContext *context, GdkRectangle *area)
{
    DEBUG_FUNCTION_IN;
    
    GikIMContext *gik = GIK_IM_CONTEXT (context);
    GikIMContextPrivate *priv = gik->priv;
    if (context == CURRENT_CONTEXT && gik_im_client_is_enabled (priv->client)) {
        /* It is the focused context */
        gint x, y;
        if(priv->client_window) {
            gdk_window_get_origin (priv->client_window, &x, &y);
            area->x += x;
            area->y += y;
        }
        gik_im_client_set_cursor_location (priv->client, area);
    }
    gtk_im_context_set_cursor_location (priv->slave, area);
}

/* Callback functions for slave context */
static void
_slave_commit_cb (GtkIMContext *slave, gchar *string, GikIMContext *context)
{
    DEBUG_FUNCTION_IN;
    
    GikIMContextPrivate *priv = context->priv;
#if 0
    if ((GtkIMContext *)context == CURRENT_CONTEXT && gik_im_client_is_enabled (priv->client))
        return;
#endif
    g_signal_emit_by_name (context, "commit", string);
}

static void
_slave_preedit_changed_cb (GtkIMContext *slave, GikIMContext *context)
{
    DEBUG_FUNCTION_IN;
    GikIMContextPrivate *priv = context->priv;
    if ((GtkIMContext *)context == CURRENT_CONTEXT && gik_im_client_is_enabled (priv->client))
        return;
    g_signal_emit_by_name (context, "preedit-changed");
}

static void
_slave_preedit_start_cb (GtkIMContext *slave, GikIMContext *context)
{
    DEBUG_FUNCTION_IN;
    GikIMContextPrivate *priv = context->priv;
    if ((GtkIMContext *)context == CURRENT_CONTEXT && gik_im_client_is_enabled (priv->client))
        return;
    g_signal_emit_by_name (context, "preedit-start");
}

static void
_slave_preedit_end_cb (GtkIMContext *slave, GikIMContext *context)
{
    DEBUG_FUNCTION_IN;
    GikIMContextPrivate *priv = context->priv;
    if ((GtkIMContext *)context == CURRENT_CONTEXT && gik_im_client_is_enabled (priv->client))
        return;
    g_signal_emit_by_name (context, "preedit-end");
}

static void
_slave_retrieve_surrounding_cb (GtkIMContext *slave, GikIMContext *context)
{
    DEBUG_FUNCTION_IN;
    GikIMContextPrivate *priv = context->priv;
    if ((GtkIMContext *)context == CURRENT_CONTEXT && gik_im_client_is_enabled (priv->client))
        return;
    g_signal_emit_by_name (context, "retrieve-surrounding");
}

static void
_slave_delete_surrounding_cb (GtkIMContext *slave, gint a1, gint a2, GikIMContext *context)
{
    DEBUG_FUNCTION_IN;
    GikIMContextPrivate *priv = context->priv;
    if ((GtkIMContext *)context == CURRENT_CONTEXT && gik_im_client_is_enabled (priv->client))
        return;
    g_signal_emit_by_name (context, "delete-surrounding", a1, a2);
}

