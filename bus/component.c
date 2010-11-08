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
    FACTORY_CHANGED,
    LAST_SIGNAL,
};

static guint             _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_component_destroy   (BusComponent        *component);

G_DEFINE_TYPE(BusComponent, bus_component, IBUS_TYPE_OBJECT)

static void
bus_component_class_init(BusComponentClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS(class);

    _signals[FACTORY_CHANGED] =
        g_signal_new (I_("factory-changed"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    ibus_object_class->destroy = (IBusObjectDestroyFunc)bus_component_destroy;
}

static void
bus_component_init(BusComponent *component)
{
}

static void
bus_component_destroy(BusComponent *component)
{
    IBUS_OBJECT_CLASS(bus_component_parent_class)->destroy(IBUS_OBJECT (component));
}

BusComponent *
bus_component_new(IBusComponent *component)
{
    g_assert(IBUS_IS_COMPONENT(component));

    BusComponent *buscomponent;
    buscomponent = (BusComponent *)g_object_new (BUS_TYPE_COMPONENT, NULL);

    buscomponent->component = (IBusComponent *)g_object_ref_sink (component);

    return buscomponent;
}

static void
bus_component_factory_destroy_cb(BusFactoryProxy *factory,
                                 BusComponent    *component)
{
    g_return_if_fail(component->factory == factory);

    g_object_unref (component->factory);
    component->factory = NULL;
    g_signal_emit (component, _signals[FACTORY_CHANGED], 0);
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
    g_signal_emit (component, _signals[FACTORY_CHANGED], 0);
}

