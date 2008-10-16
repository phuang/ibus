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

#include "engineproxy.h"

#define BUS_ENGINE_PROXY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_ENGINE_PROXY, BusEngineProxyPrivate))

enum {
    DBUS_MESSAGE,
    LAST_SIGNAL,
};


/* BusEngineProxyPriv */
struct _BusEngineProxyPrivate {
    void *pad;
};
typedef struct _BusEngineProxyPrivate BusEngineProxyPrivate;

static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_engine_proxy_class_init   (BusEngineProxyClass     *klass);
static void     bus_engine_proxy_init         (BusEngineProxy          *engine_proxy);
static void     bus_engine_proxy_destroy      (BusEngineProxy          *engine_proxy);

static gboolean bus_engine_proxy_dbus_message (BusEngineProxy          *engine_proxy,
                                         DBusMessage        *message);

static IBusProxyClass  *_parent_class = NULL;

GType
bus_engine_proxy_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusEngineProxyClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_engine_proxy_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusEngineProxy),
        0,
        (GInstanceInitFunc) bus_engine_proxy_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_PROXY,
                    "BusEngineProxy",
                    &type_info,
                    (GTypeFlags)0);
    }
    return type;
}

BusEngineProxy *
bus_engine_proxy_new (void)
{
    return BUS_ENGINE_PROXY (g_object_new (BUS_TYPE_ENGINE_PROXY, NULL));
}

static void
bus_engine_proxy_class_init (BusEngineProxyClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusProxyClass *proxy_class = IBUS_PROXY_CLASS (klass);


    _parent_class = (IBusProxyClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusEngineProxyPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_engine_proxy_destroy;

    proxy_class->dbus_message = bus_engine_proxy_dbus_message;
}

static void
bus_engine_proxy_init (BusEngineProxy *engine_proxy)
{
    BusEngineProxyPrivate *priv;
    priv = BUS_ENGINE_PROXY_GET_PRIVATE (engine_proxy);
}

static void
bus_engine_proxy_destroy (BusEngineProxy *engine_proxy)
{
    IBUS_OBJECT_CLASS(_parent_class)->destroy (IBUS_OBJECT (engine_proxy));
}

gboolean
bus_engine_proxy_handle_message (BusEngineProxy *engine_proxy, DBusMessage *message)
{
    gboolean retval = FALSE;
    g_return_val_if_fail (message != NULL, FALSE);
    
    g_signal_emit (engine_proxy, _signals[DBUS_MESSAGE], 0, message, &retval);
    return retval ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static gboolean
bus_engine_proxy_dbus_message (BusEngineProxy *engine_proxy, DBusMessage *message)
{
    return FALSE;
}
