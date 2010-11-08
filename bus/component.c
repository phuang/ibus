/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
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
#include "component.h"
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "option.h"
#include "marshalers.h"

enum {
    LAST_SIGNAL,
};

enum {
    PROP_0 = 0,
    PROP_COMPONENT,
    PROP_FACTORY,
};

struct _BusComponent {
    IBusObject parent;

    /* instance members */
    IBusComponent *component;
    BusFactoryProxy *factory;
};

struct _BusComponentClass {
    IBusObjectClass parent;
    /* class members */
};

/* functions prototype */
static GObject* bus_component_constructor   (GType                  type,
                                             guint                  n_construct_params,
                                             GObjectConstructParam *construct_params);
static void     bus_component_set_property  (BusComponent          *component,
                                             guint                  prop_id,
                                             const GValue          *value,
                                             GParamSpec            *pspec);
static void     bus_component_get_property  (BusComponent          *component,
                                             guint                  prop_id,
                                             GValue                *value,
                                             GParamSpec            *pspec);
static void     bus_component_destroy       (BusComponent          *component);

G_DEFINE_TYPE(BusComponent, bus_component, IBUS_TYPE_OBJECT)

static void
bus_component_class_init(BusComponentClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS(class);

    gobject_class->constructor = bus_component_constructor;
    gobject_class->set_property = (GObjectSetPropertyFunc)bus_component_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc)bus_component_get_property;
    ibus_object_class->destroy = (IBusObjectDestroyFunc)bus_component_destroy;
    
    /* install properties */
    /**
     * IBusComponent:name:
     *
     * The name of component
     */
    g_object_class_install_property(gobject_class,
                    PROP_COMPONENT,
                    g_param_spec_object("component",
                        "component",
                        "component",
                        IBUS_TYPE_COMPONENT,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    
    g_object_class_install_property(gobject_class,
                    PROP_FACTORY,
                    g_param_spec_object("factory",
                        "factory",
                        "factory",
                        BUS_TYPE_FACTORY_PROXY,
                        G_PARAM_READWRITE));
}

static void
bus_component_init(BusComponent *component)
{
}

static GObject*
bus_component_constructor(GType                  type,
                          guint                  n_construct_params,
                          GObjectConstructParam *construct_params)
{
    GObject *object;
    object = G_OBJECT_CLASS(bus_component_parent_class)->constructor(type,
                                                                     n_construct_params,
                                                                     construct_params);
    g_assert(((BusComponent *)object)->component);
    return object;
}

static void
bus_component_set_property(BusComponent *component,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    switch (prop_id) {
    case PROP_COMPONENT:
        g_assert(component->component == NULL);
        component->component = g_value_dup_object(value);
        break;
    case PROP_FACTORY:
        bus_component_set_factory(component, (BusFactoryProxy *)g_value_get_object(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(component, prop_id, pspec);
    }
}

static void
bus_component_get_property(BusComponent *component,
                           guint         prop_id,
                           GValue       *value,
                           GParamSpec   *pspec)
{
    switch (prop_id) {
    case PROP_COMPONENT:
        g_value_set_object(value, bus_component_get_component(component));
        break;
    case PROP_FACTORY:
        g_value_set_object(value, bus_component_get_factory(component));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (component, prop_id, pspec);
    }
}

static void
bus_component_destroy(BusComponent *component)
{
    if (component->component != NULL) {
        g_object_unref(component->component);
        component->component = NULL;
    }
    IBUS_OBJECT_CLASS(bus_component_parent_class)->destroy(IBUS_OBJECT (component));
}

BusComponent *
bus_component_new(IBusComponent   *component,
                  BusFactoryProxy *factory)
{
    g_assert(IBUS_IS_COMPONENT(component));

    return (BusComponent *)g_object_new(BUS_TYPE_COMPONENT,
                                        "component", component,
                                        "factory", factory,
                                        NULL);
}

static void
bus_component_factory_destroy_cb(BusFactoryProxy *factory,
                                 BusComponent    *component)
{
    g_return_if_fail(component->factory == factory);

    g_object_unref (component->factory);
    component->factory = NULL;
    g_object_notify((GObject*)component, "factory");
}

IBusComponent *
bus_component_get_component(BusComponent *component)
{
    g_assert(BUS_IS_COMPONENT(component));
    return component->component;
}

void
bus_component_set_factory(BusComponent    *component,
                          BusFactoryProxy *factory)
{
    g_assert(BUS_IS_COMPONENT(component));

    if (component->factory == factory) {
        return;
    }
    
    if (component->factory) {
        g_signal_handlers_disconnect_by_func(factory,
                                             bus_component_factory_destroy_cb,
                                             component);
        g_object_unref(component->factory);
        component->factory = NULL;
    }
    
    if (factory) {
        g_assert(BUS_IS_FACTORY_PROXY(factory));
        component->factory = (BusFactoryProxy*)g_object_ref (factory);
        g_signal_connect(factory, "destroy", 
                         G_CALLBACK(bus_component_factory_destroy_cb), component);
    }
    g_object_notify((GObject*)component, "factory");
}

BusFactoryProxy *
bus_component_get_factory(BusComponent *component)
{
    g_assert(BUS_IS_COMPONENT(component));
    return component->factory;
}

