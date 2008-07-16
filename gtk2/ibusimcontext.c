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
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include "ibusimcontext.h"
#include "ibusimclient.h"

/* IBusIMContextPriv */
struct _IBusIMContextPrivate {
    GtkIMContext *slave;
    GdkWindow *client_window;

    /* enabled */
    gboolean        enable;
    gchar           *ic;

    /* preedit status */
    gchar           *preedit_string;
    PangoAttrList   *preedit_attrs;
    gint             preedit_cursor_pos;
    gboolean         preedit_visible;
};


/* functions prototype */
static void     ibus_im_context_class_init   (IBusIMContextClass  *klass);
static void     ibus_im_context_init         (IBusIMContext       *obj);
static void     ibus_im_context_finalize     (GObject            *obj);
static void     ibus_im_context_reset        (GtkIMContext       *context);
static gboolean ibus_im_context_filter_keypress
                                            (GtkIMContext       *context,
                                             GdkEventKey        *key);
static void     ibus_im_context_focus_in     (GtkIMContext       *context);
static void     ibus_im_context_focus_out    (GtkIMContext       *context);
static void     ibus_im_context_get_preedit_string
                                            (GtkIMContext       *context,
                                             gchar              **str,
                                             PangoAttrList      **attrs,
                                             gint               *cursor_pos);
static void     ibus_im_context_set_client_window
                                            (GtkIMContext       *context,
                                             GdkWindow          *client);
static void     ibus_im_context_set_cursor_location
                                            (GtkIMContext       *context,
                                             GdkRectangle       *area);

/* callback functions for slave context */
static void     _slave_commit_cb            (GtkIMContext       *slave,
                                             gchar              *string,
                                             IBusIMContext       *context);
static void     _slave_preedit_changed_cb   (GtkIMContext       *slave,
                                             IBusIMContext       *context);
static void     _slave_preedit_start_cb     (GtkIMContext       *slave,
                                             IBusIMContext       *context);
static void     _slave_preedit_end_cb       (GtkIMContext       *slave,
                                             IBusIMContext       *context);
static void     _slave_retrieve_surrounding_cb
                                            (GtkIMContext       *slave,
                                             IBusIMContext       *context);
static void     _slave_delete_surrounding_cb
                                            (GtkIMContext       *slave,
                                             gint               arg1,
                                             gint               arg2,
                                             IBusIMContext       *context);



GType                _ibus_type_im_context = 0;
static GtkIMContextClass    *parent_class = NULL;

void
ibus_im_context_register_type (GTypeModule *type_module)
{
    static const GTypeInfo ibus_im_context_info = {
        sizeof (IBusIMContextClass),
        (GBaseInitFunc)        NULL,
        (GBaseFinalizeFunc)     NULL,
        (GClassInitFunc)     ibus_im_context_class_init,
        NULL,            /* class finialize */
        NULL,            /* class data */
        sizeof (IBusIMContext),
        0,
        (GInstanceInitFunc)    ibus_im_context_init,
    };

    if (! _ibus_type_im_context ) {
        _ibus_type_im_context =
            g_type_module_register_type (type_module,
                GTK_TYPE_IM_CONTEXT,
                "IBusIMContext",
                &ibus_im_context_info,
                (GTypeFlags)0);
    }
}

GtkIMContext *
ibus_im_context_new (void)
{
    IBusIMContext *obj;

    obj = IBUS_IM_CONTEXT(g_object_new (IBUS_TYPE_IM_CONTEXT, NULL));

    return GTK_IM_CONTEXT(obj);
}

static void
ibus_im_context_class_init     (IBusIMContextClass *klass)
{
    GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    parent_class = (GtkIMContextClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusIMContextPrivate));

    im_context_class->reset = &ibus_im_context_reset;
    im_context_class->focus_in = &ibus_im_context_focus_in;
    im_context_class->focus_out = &ibus_im_context_focus_out;
    im_context_class->filter_keypress = &ibus_im_context_filter_keypress;
    im_context_class->get_preedit_string = &ibus_im_context_get_preedit_string;
    im_context_class->set_client_window = &ibus_im_context_set_client_window;
    im_context_class->set_cursor_location = &ibus_im_context_set_cursor_location;
    gobject_class->finalize = &ibus_im_context_finalize;
}

static void
ibus_im_context_init     (IBusIMContext *obj)
{
    DEBUG_FUNCTION_IN;

    GError *error;
    IBusIMContext *ibus = IBUS_IM_CONTEXT (obj);
    IBusIMContextPrivate *priv = ibus->priv =
        G_TYPE_INSTANCE_GET_PRIVATE (ibus, IBUS_TYPE_IM_CONTEXT, IBusIMContextPrivate);

    priv->client_window = NULL;

    // Init ibus status
    priv->enable = FALSE;

    // Init preedit status
    priv->preedit_string = NULL;
    priv->preedit_attrs = NULL;
    priv->preedit_cursor_pos = 0;
    priv->preedit_visible = FALSE;


    // Create slave im context
    ibus->priv->slave = gtk_im_context_simple_new ();
    g_signal_connect (ibus->priv->slave,
                "commit", G_CALLBACK (_slave_commit_cb), obj);
    g_signal_connect (ibus->priv->slave,
                "preedit-start", G_CALLBACK (_slave_preedit_start_cb), obj);
    g_signal_connect (ibus->priv->slave,
                "preedit-end", G_CALLBACK (_slave_preedit_end_cb), obj);
    g_signal_connect (ibus->priv->slave,
                "preedit-changed", G_CALLBACK (_slave_preedit_changed_cb), obj);
    g_signal_connect (ibus->priv->slave,
                "retrieve-surrounding", G_CALLBACK (_slave_retrieve_surrounding_cb), obj);
    g_signal_connect (ibus->priv->slave,
                "delete-surrounding", G_CALLBACK (_slave_delete_surrounding_cb), obj);
}

static void
ibus_im_context_finalize (GObject *obj)
{
    DEBUG_FUNCTION_IN;

    IBusIMContext *ibus = IBUS_IM_CONTEXT (obj);
    IBusIMContextPrivate *priv = ibus->priv;

    ibus_im_client_release_im_context (_client, ibus);

    g_object_unref (priv->slave);

    // release preedit
    if (priv->preedit_string) g_free (priv->preedit_string);
    if (priv->preedit_attrs) pango_attr_list_unref (priv->preedit_attrs);

    G_OBJECT_CLASS(parent_class)->finalize (obj);
}

static gboolean
ibus_im_context_filter_keypress (GtkIMContext *context,
                GdkEventKey  *event)
{
    DEBUG_FUNCTION_IN;

    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;

    if (ibus_im_client_filter_keypress (_client, ibus, event))
        return TRUE;
    else
        return gtk_im_context_filter_keypress (priv->slave, event);
}

static void
ibus_im_context_focus_in (GtkIMContext *context)
{
    DEBUG_FUNCTION_IN;

    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;

    ibus_im_client_focus_in (_client, ibus);
    gtk_im_context_focus_in (priv->slave);
}

static void
ibus_im_context_focus_out (GtkIMContext *context)
{
    DEBUG_FUNCTION_IN;

    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;

    ibus_im_client_focus_out (_client, ibus);
    gtk_im_context_focus_out (priv->slave);
}

static void
ibus_im_context_reset (GtkIMContext *context)
{
    DEBUG_FUNCTION_IN;

    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;

    ibus_im_client_reset (_client, ibus);
    gtk_im_context_reset (priv->slave);
}


static void
ibus_im_context_get_preedit_string (GtkIMContext   *context,
                   gchar         **str,
                   PangoAttrList **attrs,
                   gint           *cursor_pos)
{
    DEBUG_FUNCTION_IN;

    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;

    if (priv->enable) {
        if (priv->preedit_visible) {
            if (str) {
                *str = g_strdup (priv->preedit_string ? priv->preedit_string: "");
            }

            if (attrs) {
                *attrs = priv->preedit_attrs ?
                            pango_attr_list_ref (priv->preedit_attrs):
                            pango_attr_list_new ();
            }

            if (cursor_pos) {
                *cursor_pos = priv->preedit_cursor_pos;
            }
        }
        else {
            if (str) *str = g_strdup ("");
            if (attrs) *attrs = pango_attr_list_new ();
            if (cursor_pos) *cursor_pos = 0;
        }
    }
    else {
        gtk_im_context_get_preedit_string (priv->slave, str, attrs, cursor_pos);
    }
}


static void
ibus_im_context_set_client_window  (GtkIMContext *context, GdkWindow *client)
{
    DEBUG_FUNCTION_IN;

    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;

    priv->client_window = client;
    gtk_im_context_set_client_window (priv->slave, client);
}

static void
ibus_im_context_set_cursor_location (GtkIMContext *context, GdkRectangle *area)
{
    DEBUG_FUNCTION_IN;

    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;

    if (priv->enable) {
        /* It is the focused context */
        gint x, y;
        if(priv->client_window) {
            gdk_window_get_origin (priv->client_window, &x, &y);
            area->x += x;
            area->y += y;
        }
        ibus_im_client_set_cursor_location (_client, ibus, area);
    }
    gtk_im_context_set_cursor_location (priv->slave, area);
}

/* Callback functions for slave context */
static void
_slave_commit_cb (GtkIMContext *slave, gchar *string, IBusIMContext *context)
{
    DEBUG_FUNCTION_IN;

    IBusIMContextPrivate *priv = context->priv;
#if 0
    if ((GtkIMContext *)context == CURRENT_CONTEXT && ibus_im_client_is_enabled (_client))
        return;
#endif
    g_signal_emit_by_name (context, "commit", string);
}

static void
_slave_preedit_changed_cb (GtkIMContext *slave, IBusIMContext *context)
{
    DEBUG_FUNCTION_IN;
    IBusIMContextPrivate *priv = context->priv;

    if (priv->enable && priv->ic)
        return;

    g_signal_emit_by_name (context, "preedit-changed");
}

static void
_slave_preedit_start_cb (GtkIMContext *slave, IBusIMContext *context)
{
    DEBUG_FUNCTION_IN;
    IBusIMContextPrivate *priv = context->priv;

    if (priv->enable && priv->ic)
        return;
    g_signal_emit_by_name (context, "preedit-start");
}

static void
_slave_preedit_end_cb (GtkIMContext *slave, IBusIMContext *context)
{
    DEBUG_FUNCTION_IN;
    IBusIMContextPrivate *priv = context->priv;

    if (priv->enable && priv->ic)
        return;
    g_signal_emit_by_name (context, "preedit-end");
}

static void
_slave_retrieve_surrounding_cb (GtkIMContext *slave, IBusIMContext *context)
{
    DEBUG_FUNCTION_IN;
    IBusIMContextPrivate *priv = context->priv;

    if (priv->enable && priv->ic)
        return;
    g_signal_emit_by_name (context, "retrieve-surrounding");
}

static void
_slave_delete_surrounding_cb (GtkIMContext *slave, gint a1, gint a2, IBusIMContext *context)
{
    DEBUG_FUNCTION_IN;
    IBusIMContextPrivate *priv = context->priv;

    if (priv->enable && priv->ic)
        return;
    g_signal_emit_by_name (context, "delete-surrounding", a1, a2);
}

gchar *
ibus_im_context_get_ic (IBusIMContext *context)
{
    IBusIMContextPrivate *priv = context->priv;
    return priv->ic;
}

void
ibus_im_context_set_ic (IBusIMContext *context, const gchar *ic)
{
    IBusIMContextPrivate *priv = context->priv;
    if (priv->ic) g_free (priv->ic);
    priv->ic = g_strdup (ic);
}

void
ibus_im_context_enable (IBusIMContext *context)
{
    IBusIMContextPrivate *priv = context->priv;
    priv->enable = TRUE;
}

void
ibus_im_context_disable (IBusIMContext *context)
{
    IBusIMContextPrivate *priv = context->priv;
    priv->enable = FALSE;
}


void
ibus_im_context_commit_string (IBusIMContext  *context, const gchar *string)
{
    g_signal_emit_by_name (context, "commit", string);
}

void
ibus_im_context_update_preedit (IBusIMContext *context, const gchar *string,
        PangoAttrList *attrs, gint cursor_pos, gboolean show)
{
    IBusIMContextPrivate *priv = context->priv;

    priv->preedit_string = g_strdup (string);
    priv->preedit_attrs = pango_attr_list_ref (attrs);
    priv->preedit_cursor_pos = cursor_pos;
    priv->preedit_visible = show;

    g_signal_emit_by_name (context, "preedit-changed");
}
