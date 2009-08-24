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
#define IBUS_TYPE_FACTORY               \
    (ibus_factory_get_type ())
#define IBUS_FACTORY(obj)               \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_FACTORY, IBusFactory))
#define IBUS_FACTORY_CLASS(klass)       \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_FACTORY, IBusFactoryClass))
#define IBUS_IS_FACTORY(obj)            \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_FACTORY))
#define IBUS_IS_FACTORY_CLASS(klass)    \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_FACTORY))
#define IBUS_FACTORY_GET_CLASS(obj)     \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_FACTORY, IBusFactoryClass))

/**
 * IBUS_TYPE_FACTORY_INFO:
 *
 * Return GType of IBus factory information.
 */
#define IBUS_TYPE_FACTORY_INFO              \
    (ibus_factory_info_get_type ())

/**
 * IBUS_FACTORY_INFO:
 * @obj: An object which is subject to casting.
 *
 * Casts an IBUS_FACTORY_INFO or derived pointer into a (IBusFactoryInfo*) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define IBUS_FACTORY_INFO(obj)              \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_FACTORY_INFO, IBusFactoryInfo))

/**
 * IBUS_FACTORY_INFO_CLASS:
 * @klass: A class to be casted.
 *
 * Casts a derived IBusFactoryInfoClass structure into a IBusFactoryInfoClass structure.
 */
#define IBUS_FACTORY_INFO_CLASS(klass)      \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_FACTORY_INFO, IBusFactoryInfoClass))

/**
 * IBUS_IS_FACTORY_INFO:
 * @obj: Instance to check for being a IBUS_FACTORY_INFO.
 *
 * Checks whether a valid GTypeInstance pointer is of type IBUS_FACTORY_INFO.
 */
#define IBUS_IS_FACTORY_INFO(obj)           \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_FACTORY_INFO))

/**
 * IBUS_IS_FACTORY_INFO_CLASS:
 * @klass: A class to be checked.
 *
 * Checks whether class "is a" valid IBusFactoryInfoClass structure of type IBUS_FACTORY_INFO or derived.
 */
#define IBUS_IS_FACTORY_INFO_CLASS(klass)   \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_FACTORY_INFO))

/**
 * IBUS_FACTORY_INFO_GET_CLASS:
 * @obj: An object.
 *
 * Get the class of a given object and cast the class to IBusFactoryInfoClass.
 */
#define IBUS_FACTORY_INFO_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_FACTORY_INFO, IBusFactoryInfoClass))


G_BEGIN_DECLS

typedef struct _IBusFactory IBusFactory;
typedef struct _IBusFactoryClass IBusFactoryClass;
typedef struct _IBusFactoryInfo IBusFactoryInfo;
typedef struct _IBusFactoryInfoClass IBusFactoryInfoClass;

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

G_END_DECLS
#endif

