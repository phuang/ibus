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

#include "ibusobject.h"
#include "ibusinternal.h"

#define IBUS_OBJECT_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_OBJECT, IBusObjectPrivate))

enum {
    DESTROY,
    LAST_SIGNAL,
};

typedef struct _IBusObjectPrivate IBusObjectPrivate;
struct _IBusObjectPrivate {
    gboolean in_destructor;
};

static guint            object_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_object_class_init      (IBusObjectClass    *klass);
static void     ibus_object_init            (IBusObject         *obj);
static void     ibus_object_dispose         (IBusObject         *obj);
static void     ibus_object_finalize        (IBusObject         *obj);
static void     ibus_object_real_destroy    (IBusObject         *obj);

static GObjectClass *_parent_class = NULL;


GType
ibus_object_get_type (void)
{
    static GType type = 0;
    
    static const GTypeInfo type_info = {
        sizeof (IBusObjectClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_object_class_init,
        NULL,               /* class finialize */
        NULL,               /* class data */
        sizeof (IBusObject),
        0,
        (GInstanceInitFunc) ibus_object_init,
    };

    if (type == 0) {
        type = g_type_register_static (G_TYPE_OBJECT,
                    "IBusObject",
                    &type_info,
                    (GTypeFlags)0);
    }
    
    return type;
}

IBusObject *
ibus_object_new (void)
{
    return IBUS_OBJECT (g_object_new (IBUS_TYPE_OBJECT, NULL));
}

static void
ibus_object_class_init     (IBusObjectClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    _parent_class = (GObjectClass *) g_type_class_peek_parent (klass);
    
    g_type_class_add_private (klass, sizeof (IBusObjectPrivate));

    gobject_class->dispose = (GObjectFinalizeFunc) ibus_object_dispose;
    gobject_class->finalize = (GObjectFinalizeFunc) ibus_object_finalize;

    klass->destroy = ibus_object_real_destroy;

    /* install signals */
    object_signals[DESTROY] =
        g_signal_new (I_("destroy"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusObjectClass, destroy),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

static void
ibus_object_init (IBusObject *obj)
{
    IBusObjectPrivate *priv;
    priv = IBUS_OBJECT_GET_PRIVATE (obj);

    priv->in_destructor = FALSE;
}

static void
ibus_object_dispose (IBusObject *obj)
{
    IBusObjectPrivate *priv;
    priv = IBUS_OBJECT_GET_PRIVATE (obj);

    
    if (!priv->in_destructor) {
        priv->in_destructor = TRUE;
        g_signal_emit (obj, object_signals[DESTROY], 0);
        priv->in_destructor = FALSE;
    }

    G_OBJECT_CLASS(_parent_class)->dispose (G_OBJECT (obj));
}

static void
ibus_object_finalize (IBusObject *obj)
{
    g_debug ("%s finalize", G_OBJECT_TYPE_NAME(obj));
    G_OBJECT_CLASS(_parent_class)->finalize (G_OBJECT (obj));
}

static void
ibus_object_real_destroy (IBusObject *obj)
{
    g_signal_handlers_destroy (obj);
}

void
ibus_object_destroy (IBusObject *obj)
{
    IBusObjectPrivate *priv;
    priv = IBUS_OBJECT_GET_PRIVATE (obj);

    if (!priv->in_destructor) {
            g_object_run_dispose (G_OBJECT (obj));
    }
}
