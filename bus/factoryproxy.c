/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
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
#include "factoryproxy.h"
#include "types.h"
#include "marshalers.h"
#include "dbusimpl.h"
#include "option.h"

struct _BusFactoryProxy {
    IBusProxy parent;
    /* instance members */
};

struct _BusFactoryProxyClass {
    IBusProxyClass parent;
    /* class members */
};

/* functions prototype */
static void      bus_factory_proxy_destroy      (IBusProxy        *proxy);

G_DEFINE_TYPE (BusFactoryProxy, bus_factory_proxy, IBUS_TYPE_PROXY)

static void
bus_factory_proxy_class_init (BusFactoryProxyClass *class)
{
    IBUS_PROXY_CLASS(class)->destroy = bus_factory_proxy_destroy;
}

static void
bus_factory_proxy_init (BusFactoryProxy *factory)
{
}

static void
bus_factory_proxy_destroy (IBusProxy *proxy)
{
    IBUS_PROXY_CLASS(bus_factory_proxy_parent_class)->destroy(IBUS_PROXY (proxy));
}

BusFactoryProxy *
bus_factory_proxy_new(BusConnection *connection)
{
    g_assert(BUS_IS_CONNECTION(connection));
    BusFactoryProxy *factory;
    
    factory = g_object_new (BUS_TYPE_FACTORY_PROXY,
                            "g-object-path", IBUS_PATH_FACTORY,
                            "g-interface-name", IBUS_INTERFACE_FACTORY,
                            "g-connection", bus_connection_get_dbus_connection (connection),
                            NULL);
    return factory;
}

BusEngineProxy *
bus_factory_proxy_create_engine (BusFactoryProxy *factory,
                                 IBusEngineDesc  *desc)
{
    g_assert (BUS_IS_FACTORY_PROXY (factory));
    g_assert (IBUS_IS_ENGINE_DESC (desc));

    /* FIXME: should we check it? */
#if 0
    if (g_list_find (factory->component->engines, desc) == NULL) {
        return NULL;
    }
#endif

    GError *error = NULL;
    GVariant *retval = g_dbus_proxy_call_sync ((GDBusProxy *)factory,
                                               "CreateEngine",
                                               g_variant_new ("(s)", ibus_engine_desc_get_name (desc)),
                                               G_DBUS_CALL_FLAGS_NONE,
                                               -1, NULL, &error);
    if (retval == NULL) {
        g_warning ("Create engine failed. %s", error->message);
        g_error_free (error);
        return NULL;
    }

    const gchar *object_path = NULL;
    g_variant_get (retval, "(&o)", &object_path);
    GDBusConnection *connection = g_dbus_proxy_get_connection ((GDBusProxy *) factory);
    BusEngineProxy *engine = bus_engine_proxy_new (object_path,
                                                   desc,
                                                   bus_connection_lookup (connection));
    g_variant_unref (retval);
    return engine;
}

