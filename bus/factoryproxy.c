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
#include <dbus/dbus.h>
#include <ibusinternal.h>
#include <ibusmarshalers.h>
#include "dbusimpl.h"
#include "factoryproxy.h"
#include "option.h"

/* functions prototype */
static void      bus_factory_proxy_destroy      (BusFactoryProxy        *factory);

G_DEFINE_TYPE (BusFactoryProxy, bus_factory_proxy, IBUS_TYPE_PROXY)

BusFactoryProxy *
bus_factory_proxy_new (IBusComponent *component,
                       BusConnection *connection)
{
    g_assert (IBUS_IS_COMPONENT (component));

    BusFactoryProxy *factory;
    GList *p;

    if (connection == NULL) {
        connection = bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS, component->name);
    }

    if (connection == NULL) {
        return NULL;
    }

    factory = g_object_new (BUS_TYPE_FACTORY_PROXY,
                            "name", NULL,
                            "path", "/org/freedesktop/IBus/Factory",
                            "connection", connection,
                            NULL);

    g_object_ref_sink (component);
    factory->component = component;
    g_object_set_data ((GObject *)factory->component, "factory", factory);

    factory->engine_list = ibus_component_get_engines (factory->component);

    for (p = factory->engine_list; p != NULL; p = p->next) {
        IBusEngineDesc *desc = (IBusEngineDesc *)p->data;
        g_object_ref (desc);
        g_object_set_data ((GObject *)desc, "factory", factory);
    }

    return factory;
}

static void
bus_factory_proxy_class_init (BusFactoryProxyClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_factory_proxy_destroy;
}

static void
bus_factory_proxy_init (BusFactoryProxy *factory)
{
    factory->component = NULL;
}

static void
bus_factory_proxy_destroy (BusFactoryProxy *factory)
{
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

    IBUS_OBJECT_CLASS(bus_factory_proxy_parent_class)->destroy (IBUS_OBJECT (factory));
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

    IBusPendingCall *pending = NULL;
    IBusMessage *reply_message;
    IBusError *error;
    BusEngineProxy *engine;
    gchar *object_path;
    gboolean retval;

    if (g_list_find (factory->component->engines, desc) == NULL) {
        return NULL;
    }

    retval = ibus_proxy_call_with_reply ((IBusProxy *) factory,
                                         "CreateEngine",
                                         &pending,
                                         g_dbus_timeout,
                                         &error,
                                         G_TYPE_STRING, &(desc->name),
                                         G_TYPE_INVALID);

    if (!retval) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        return NULL;
    }

    ibus_pending_call_wait (pending);
    reply_message = ibus_pending_call_steal_reply (pending);
    ibus_pending_call_unref (pending);

    if (reply_message == NULL) {
        IBusObject *connection;
        connection = (IBusObject *) ibus_proxy_get_connection ((IBusProxy *)factory);
        ibus_object_destroy (connection);
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        return NULL;
    }

    if ((error = ibus_error_new_from_message (reply_message)) != NULL) {
        if (g_strcmp0 (error->name, DBUS_ERROR_NO_REPLY) == 0) {
            IBusObject *connection;
            connection = (IBusObject *) ibus_proxy_get_connection ((IBusProxy *)factory);
            ibus_object_destroy (connection);
        }
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        ibus_message_unref (reply_message);
        return NULL;
    }

    if (!ibus_message_get_args (reply_message,
                                &error,
                                IBUS_TYPE_OBJECT_PATH, &object_path,
                                G_TYPE_INVALID)) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        ibus_message_unref (reply_message);

        return NULL;
    }

    IBusConnection *connection = ibus_proxy_get_connection ((IBusProxy *) factory);
    engine = bus_engine_proxy_new (object_path, desc, (BusConnection *) connection);
    ibus_message_unref (reply_message);

    return engine;
}

