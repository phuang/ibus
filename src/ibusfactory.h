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
/**
 * SECTION: ibusfactory
 * @short_description: Factory for creating engine instances.
 * @stability: Stable
 * @see_also: #IBusEngine
 *
 * An IBusFactory is an #IBusService that creates input method engine (IME) instance.
 * It provides CreateEngine remote method, which creates an IME instance by name,
 * and returns the D-Bus object path to IBus daemon.
 */
#ifndef __IBUS_FACTORY_H_
#define __IBUS_FACTORY_H_

#include <dbus/dbus.h>
#include "ibusservice.h"
#include "ibusserializable.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */

/**
 * IBUS_TYPE_FACTORY:
 *
 * Return GType of IBus factory.
 */
#define IBUS_TYPE_FACTORY               \
    (ibus_factory_get_type ())

/**
 * IBUS_FACTORY:
 * @obj: An object which is subject to casting.
 *
 * Casts an IBUS_FACTORY or derived pointer into a (IBusFactory*) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define IBUS_FACTORY(obj)               \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_FACTORY, IBusFactory))

/**
 * IBUS_FACTORY_CLASS:
 * @klass: A class to be casted.
 *
 * Casts a derived IBusFactoryClass structure into a IBusFactoryClass structure.
 */
#define IBUS_FACTORY_CLASS(klass)       \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_FACTORY, IBusFactoryClass))

/**
 * IBUS_IS_FACTORY:
 * @obj: Instance to check for being a IBUS_FACTORY.
 *
 * Checks whether a valid GTypeInstance pointer is of type IBUS_FACTORY.
 */
#define IBUS_IS_FACTORY(obj)            \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_FACTORY))

/**
 * IBUS_IS_FACTORY_CLASS:
 * @klass: A class to be checked.
 *
 * Checks whether class "is a" valid IBusFactoryClass structure of type IBUS_FACTORY or derived.
 */
#define IBUS_IS_FACTORY_CLASS(klass)    \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_FACTORY))

/**
 * IBUS_FACTORY_GET_CLASS:
 * @obj: An object.
 *
 * Get the class of a given object and cast the class to IBusFactoryClass.
 */
#define IBUS_FACTORY_GET_CLASS(obj)     \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_FACTORY, IBusFactoryClass))

#if 0
#define IBUS_TYPE_FACTORY_INFO              \
    (ibus_factory_info_get_type ())
#define IBUS_FACTORY_INFO(obj)              \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_FACTORY_INFO, IBusFactoryInfo))
#define IBUS_FACTORY_INFO_CLASS(klass)      \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_FACTORY_INFO, IBusFactoryInfoClass))
#define IBUS_IS_FACTORY_INFO(obj)           \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_FACTORY_INFO))
#define IBUS_IS_FACTORY_INFO_CLASS(klass)   \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_FACTORY_INFO))
#define IBUS_FACTORY_INFO_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_FACTORY_INFO, IBusFactoryInfoClass))
#endif

G_BEGIN_DECLS

typedef struct _IBusFactory IBusFactory;
typedef struct _IBusFactoryClass IBusFactoryClass;

#if 0
typedef struct _IBusFactoryInfo IBusFactoryInfo;
typedef struct _IBusFactoryInfoClass IBusFactoryInfoClass;
#endif

/**
 * IBusFactory:
 *
 * An opaque data type representing an IBusFactory.
 */
struct _IBusFactory {
    IBusService parent;

    /* instance members */
};

struct _IBusFactoryClass {
    IBusServiceClass parent;

    /* signals */

    /*< private >*/
    /* padding */
    gpointer pdummy[8];
};

#if 0
/**
 * IBusFactoryInfo:
 * @path: D-Bus path for the IME.
 * @name: D-Bus name for the IME.
 * @lang: Supporting language of the IME.
 * @icon: Icon file of the IME.
 * @authors: Authors of the IME.
 * @credits: Credits of the IME.
 *
 * An IBusFactoryInfo stores information about an IME.
 * So CreateEngine method can create instances of that IME.
 */
struct _IBusFactoryInfo {
    IBusSerializable parent;

    /* instance members */
    /*< public >*/
    gchar *path;
    gchar *name;
    gchar *lang;
    gchar *icon;
    gchar *authors;
    gchar *credits;
};

struct _IBusFactoryInfoClass {
    IBusSerializableClass parent;

    /* signals */

    /*< private >*/
    /* padding */
    gpointer pdummy[8];
};
#endif

GType            ibus_factory_get_type          (void);

/**
 * ibus_factory_new:
 * @connection: An IBusConnection.
 * @returns: A newly allocated IBusFactory.
 *
 * New an IBusFactory.
 */
IBusFactory     *ibus_factory_new               (IBusConnection *connection);

/**
 * ibus_factory_add_engine:
 * @factory: An IBusFactory.
 * @engine_name: Name of an engine.
 * @engine_type: GType of an engine.
 *
 * Add an engine to the factory.
 */
void             ibus_factory_add_engine        (IBusFactory    *factory,
                                                 const gchar    *engine_name,
                                                 GType           engine_type);

#if 0
/**
 * ibus_factory_get_info:
 * @factory: An IBusFactory.
 * @returns: A corresponding IbusFactoryInfo.
 *
 * Get IBusFactoryInfo out of IBusFactory.
 *
 * <note><para>This function is currently commented out</para></note>
 */
IBusFactoryInfo *ibus_factory_get_info          (IBusFactory    *factory);

GType            ibus_factory_info_get_type     (void);

/**
 * ibus_factory_info_new:
 * @path: D-Bus path for the IME.
 * @name: IME name.
 * @lang: Supporting language of the IME.
 * @icon: Icon file of the IME.
 * @authors: Authors of the IME.
 * @credits: Credits of the IME.
 * @returns: A newly allocated IBusFactoryInfo.
 *
 * New an IBusFactoryInfo.
 */
IBusFactoryInfo *ibus_factory_info_new          (const gchar    *path,
                                                 const gchar    *name,
                                                 const gchar    *lang,
                                                 const gchar    *icon,
                                                 const gchar    *authors,
                                                 const gchar    *credits);

#endif

G_END_DECLS
#endif

