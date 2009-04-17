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
/**
 * SECTION: ibusbus
 * @short_description: Connect with IBus daemon.
 * @stability: Stable
 *
 * An IBusBus connects with IBus daemon.
 */
#ifndef __IBUS_BUS_H_
#define __IBUS_BUS_H_

#include <dbus/dbus.h>
#include "ibusinputcontext.h"
#include "ibusconfig.h"
#include "ibuscomponent.h"


/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_BUS             \
    (ibus_bus_get_type ())
#define IBUS_BUS(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_BUS, IBusBus))
#define IBUS_BUS_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_BUS, IBusBusClass))
#define IBUS_IS_BUS(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_BUS))
#define IBUS_IS_BUS_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_BUS))
#define IBUS_BUS_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_BUS, IBusBusClass))

G_BEGIN_DECLS

typedef struct _IBusBus IBusBus;
typedef struct _IBusBusClass IBusBusClass;

/**
 * IBusBus:
 *
 * An opaque data type representing IBus bus (daemon communication) status.
 */
struct _IBusBus {
  IBusObject parent;
  /* instance members */
};

struct _IBusBusClass {
  IBusObjectClass parent;
  /* class members */
};

GType        ibus_bus_get_type          (void);
/**
 * ibus_bus_new:
 * @returns: A newly allocated IBusBus instance.
 *
 * New an IBusBus instance.
 */
IBusBus     *ibus_bus_new               (void);

/**
 * ibus_bus_is_connected:
 * @bus: An IBusBus.
 * @returns: TRUE if @bus is connected, FALSE otherwise.
 *
 * Return TRUE if @bus is connected to IBus daemon.
 */
gboolean     ibus_bus_is_connected      (IBusBus        *bus);

/**
 * ibus_bus_get_connection:
 * @bus: An IBusBus.
 * @returns: TRUE if @bus is connected, FALSE otherwise.
 *
 * Return IBusConnection of an IBusIBus instance.
 */
IBusConnection
            *ibus_bus_get_connection    (IBusBus        *bus);
/* declare dbus methods */
const gchar *ibus_bus_hello             (IBusBus        *bus);

/**
 * ibus_bus_request_name:
 * @bus: the IBusBus instance to be processed.
 * @name: Name to be requested.
 * @flags: Flags (FixMe).
 * @returns: 0 if failed; positive number otherwise.
 *
 * Request a name from IBus daemon.
 */
guint        ibus_bus_request_name      (IBusBus        *bus,
                                         const gchar    *name,
                                         guint           flags);

/**
 * ibus_bus_release_name:
 * @bus: An IBusBus.
 * @name: Name to be released.
 * @returns: 0 if failed; positive number otherwise.
 *
 * Release a name to IBus daemon.
 */
guint        ibus_bus_release_name      (IBusBus        *bus,
                                         const gchar    *name);

/**
 * ibus_bus_name_has_owner:
 * @bus: An IBusBus.
 * @name: Name to be released.
 * @returns: TRUE if the name has owner, FALSE otherwise.
 *
 * Whether the name has owner.
 */
gboolean     ibus_bus_name_has_owner    (IBusBus        *bus,
                                         const gchar    *name);

/**
 * ibus_bus_list_names:
 * @bus: An IBusBus.
 * @returns: Lists that attached to @bus.
 *
 * Return lists that attached to @bus.
 * <note><para>[FixMe] Not implemented yet, only return NULL.</para></note>
 */
GList       *ibus_bus_list_names        (IBusBus        *bus);

/**
 * ibus_bus_add_match:
 * @bus: An IBusBus.
 * @rule: Match rule.
 *
 * Add a match rule to an IBusBus.
 */
void         ibus_bus_add_match         (IBusBus        *bus,
                                         const gchar    *rule);

/**
 * ibus_bus_remove_match:
 * @bus: An IBusBus.
 * @rule: Match rule.
 *
 * Remove a match rule to an IBusBus.
 */
void         ibus_bus_remove_match      (IBusBus        *bus,
                                         const gchar    *rule);

/**
 * ibus_bus_get_name_owner:
 * @bus: An IBusBus.
 * @name: Name.
 * @returns: Owner of the name.
 *
 * Return the name owner.
 */
const gchar *ibus_bus_get_name_owner    (IBusBus        *bus,
                                         const gchar    *name);
/* declare ibus methods */

/**
 * ibus_bus_exit:
 * @bus: An IBusBus.
 * @restart: Whether restarting the ibus.
 * @returns: TRUE if the "Exit" call is suceeded, FALSE otherwise.
 *
 * Exit or restart an IBusBus.
 */
gboolean     ibus_bus_exit              (IBusBus        *bus,
                                         gboolean        restart);

/**
 * ibus_bus_create_input_context:
 * @bus: An IBusBus.
 * @client_name: Name of client.
 * @returns: An newly allocated IBusInputContext if the "CreateInputContext" call
 *            is suceeded, NULL otherwise.
 *
 * Create an input context for client.
 */
IBusInputContext
            *ibus_bus_create_input_context
                                        (IBusBus        *bus,
                                         const gchar    *client_name);
/**
 * ibus_bus_register_component:
 * @bus: An IBusBus.
 * @component: A input engine component.
 * @returns: TRUE if the "RegisterComponent" call is suceeded, FALSE otherwise.
 *
 * Register a componet to an IBusBus.
 */
gboolean     ibus_bus_register_component(IBusBus        *bus,
                                         IBusComponent  *component);

/**
 * ibus_bus_list_engines:
 * @bus: An IBusBus.
 * @returns: A GList of engines.
 *
 * List engines.
 * Note that this function is not yet implemented.
 */
GList       *ibus_bus_list_engines      (IBusBus        *bus);

/**
 * ibus_bus_list_active_engines:
 * @bus: An IBusBus.
 * @returns: A GList of active engines.
 *
 * List active engines.
 * Note that this function is not yet implemented.
 * <note><para>Not yet implemented.</para></note>
 */
GList       *ibus_bus_list_active_engines
                                        (IBusBus        *bus);
/* declare config apis */
/**
 * ibus_bus_get_config:
 * @bus: An IBusBus.
 * @returns: An newly allocated IBusConfig which is configurable with @bus.
 *
 * Get the config instance from IBusBus.
 */
 IBusConfig  *ibus_bus_get_config        (IBusBus        *bus);

G_END_DECLS
#endif

