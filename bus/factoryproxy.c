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

    IBusComponent *component;
    GList *engine_list;
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
    factory->component = NULL;
}

static void
bus_factory_proxy_destroy (IBusProxy *proxy)
{
    BusFactoryProxy *factory = (BusFactoryProxy *)proxy;
    GList *p;

    for (p = factory->engine_list; p != NULL ; p = p->next) {
        IBusEngineDesc *desc = (IBusEngineDesc *)p->data;
        g_object_steal_data ((GObject *)desc, "factory");
        g_object_unref (desc);
    }
    g_list_free (factory->engine_list);
    factory->engine_list = NULL;

    if (factory->component) {
        g_object_steal_data ((GObject *)factory->component, "factory");
        g_object_unref (factory->component);
        factory->component = NULL;
    }

    IBUS_PROXY_CLASS(bus_factory_proxy_parent_class)->destroy (IBUS_PROXY (factory));
}

BusFactoryProxy *
bus_factory_proxy_new (IBusComponent *component,
                       BusConnection *connection)
{
    g_assert (IBUS_IS_COMPONENT (component));

    BusFactoryProxy *factory;
    GList *p;

    if (connection == NULL) {
        const gchar *name = ibus_component_get_name (component);
        connection = bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS, name);
    }

    if (connection == NULL) {
        return NULL;
    }

    factory = g_object_new (BUS_TYPE_FACTORY_PROXY,
                            "g-object-path", IBUS_PATH_FACTORY,
                            "g-interface-name", IBUS_INTERFACE_FACTORY,
                            "g-connection", bus_connection_get_dbus_connection (connection),
                            NULL);

    g_object_ref_sink (component);
    factory->component = component;
    g_object_set_data ((GObject *)factory->component, "factory", factory);

    factory->engine_list = ibus_component_get_engines (factory->component);

    for (p = factory->engine_list; p != NULL; p = p->next) {
        IBusEngineDesc *desc = (IBusEngineDesc *)p->data;
        g_object_ref (desc);
        g_object_set_data ((GObject *)desc, "factory", factory);
        g_assert (g_object_get_data ((GObject *)desc, "factory") == factory);
    }

    return factory;
}

IBusComponent *
bus_factory_proxy_get_component (BusFactoryProxy *factory)
{
    return factory->component;
}

BusFactoryProxy *
bus_factory_proxy_get_from_component (IBusComponent *component)
{
    IBUS_IS_COMPONENT (component);

    BusFactoryProxy *factory;

    factory = (BusFactoryProxy *) g_object_get_data ((GObject *)component, "factory");

    return factory;
}

BusFactoryProxy *
bus_factory_proxy_get_from_engine (IBusEngineDesc *desc)
{

    IBUS_IS_ENGINE_DESC (desc);

    BusFactoryProxy *factory;

    factory = (BusFactoryProxy *) g_object_get_data ((GObject *)desc, "factory");

    return factory;
}

BusEngineProxy *
bus_factory_proxy_create_engine (BusFactoryProxy *factory,
                                 IBusEngineDesc  *desc)
{
    g_assert (BUS_IS_FACTORY_PROXY (factory));
    g_assert (IBUS_IS_ENGINE_DESC (desc));

    if (g_list_find (factory->component->engines, desc) == NULL) {
        return NULL;
    }

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
                    desc, bus_connection_lookup (connection));
    g_variant_unref (retval);
    return engine;
}

