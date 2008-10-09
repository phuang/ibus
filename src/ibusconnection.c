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

#include "ibusconnection.h"
#include "ibusinternel.h"

enum {
    DBUS_SIGNAL,
    DBUS_MESSAGE,
    LAST_SIGNAL,
};


/* IBusConnectionPriv */
struct _IBusConnectionPrivate {
};

static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_connection_class_init  (IBusConnectionClass    *klass);
static void     ibus_connection_init        (IBusConnection         *client);
static void     ibus_connection_finalize    (GObject                *obj);

static IBusObjectClass *_parent_class = NULL;


GType
ibus_connection_get_type (void)
{
    static GType type = 0;
    
    static const GTypeInfo type_info = {
        sizeof (IBusConnectionClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_connection_class_init,
        NULL,               /* class finialize */
        NULL,               /* class data */
        sizeof (IBusConnection),
        0,
        (GInstanceInitFunc) ibus_connection_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "IBusConnection",
                    &type_info,
                    (GTypeFlags)0);
    }
    
    return type;
}

IBusConnection *
ibus_connection_new (void)
{
    return IBUS_CONNECTION (g_object_new (IBUS_TYPE_CONNECTION, NULL));
}

static void
ibus_connection_class_init (IBusConnectionClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    _parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusConnectionPrivate));

    _signals[DBUS_SIGNAL] =
        g_signal_new (I_("dbus-signal"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST,
            G_STRUCT_OFFSET (IBusConnectionClass, dbus_signal),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    
    _signals[DBUS_MESSAGE] =
        g_signal_new (I_("dbus-message"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST,
            G_STRUCT_OFFSET (IBusConnectionClass, dbus_message),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

}

static void
ibus_connection_init (IBusConnection *obj)
{
}

