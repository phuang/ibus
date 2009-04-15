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
 * SECTION: ibusconfigservice
 * @short_description: Configuration service back-end.
 * @stability: Stable
 *
 * An IBusConfigService is a base class for other configuration services such as GConf.
 * Currently, directly known sub class is IBusConfigGConf.
 *
 * IBusConfigServiceClass has following member functions:
 * <itemizedlist>
 *     <listitem>
 *         <para>gboolean set_value(IBusConfigService *config, const gchar *section, const gchar *name,
 *             const GValue *value, IBusError **error)
 *         </para>
 *         <variablelist>
 *             <varlistentry>
 *                 <term>config:</term>
 *                 <listitem>A configure service</listitem>
 *             </varlistentry>
 *             <varlistentry>
 *                 <term>section:</term>
 *                 <listitem>Section name of the configuration option.</listitem>
 *             </varlistentry>
 *             <varlistentry>
 *                 <term>name:</term>
 *                 <listitem>Name of the configuration option.</listitem>
 *             </varlistentry>
 *             <varlistentry>
 *                 <term>value:</term>
 *                 <listitem>GValue that holds the value.</listitem>
 *             </varlistentry>
 *             <varlistentry>
 *                 <term>error:</term>
 *                 <listitem>Error outputs here.</listitem>
 *             </varlistentry>
 *             <varlistentry>
 *                 <term>Returns:</term>
 *                 <listitem>TRUE if succeed; FALSE otherwise.</listitem>
 *             </varlistentry>
 *         </variablelist>
 *         <para>Set a value to a configuration option.
 *         </para>
 *     </listitem>
 *     <listitem>
 *         <para>gboolean get_value(IBusConfigService *config, const gchar *section, const gchar *name,
 *             GValue *value, IBusError **error)
 *         </para>
 *         <variablelist>
 *             <varlistentry>
 *                 <term>config:</term>
 *                 <listitem>A configure service</listitem>
 *             </varlistentry>
 *             <varlistentry>
 *                 <term>section:</term>
 *                 <listitem>Section name of the configuration option.</listitem>
 *             </varlistentry>
 *             <varlistentry>
 *                 <term>name:</term>
 *                 <listitem>Name of the configuration option.</listitem>
 *             </varlistentry>
 *             <varlistentry>
 *                 <term>value:</term>
 *                 <listitem>GValue that holds the value.</listitem>
 *             </varlistentry>
 *             <varlistentry>
 *                 <term>error:</term>
 *                 <listitem>Error outputs here.</listitem>
 *             </varlistentry>
 *             <varlistentry>
 *                 <term>Returns:</term>
 *                 <listitem>TRUE if succeed; FALSE otherwise.</listitem>
*             </varlistentry>
*         </variablelist>
 *        <para>Get value of a configuration option.
 *        </para>
 *     </listitem>
 * </itemizedlist>
 */
#ifndef __IBUS_CONFIG_SERVICE_H_
#define __IBUS_CONFIG_SERVICE_H_

#include "ibuserror.h"
#include "ibusservice.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_CONFIG_SERVICE             \
    (ibus_config_service_get_type ())
#define IBUS_CONFIG_SERVICE(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_CONFIG_SERVICE, IBusConfigService))
#define IBUS_CONFIG_SERVICE_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_CONFIG_SERVICE, IBusConfigServiceClass))
#define IBUS_IS_CONFIG_SERVICE(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_CONFIG_SERVICE))
#define IBUS_IS_CONFIG_SERVICE_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_CONFIG_SERVICE))
#define IBUS_CONFIG_SERVICE_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_CONFIG_SERVICE, IBusConfigServiceClass))

G_BEGIN_DECLS

typedef struct _IBusConfigService IBusConfigService;
typedef struct _IBusConfigServiceClass IBusConfigServiceClass;

/**
 * IBusConfigService:
 *
 * An opaque data type representing a configure service.
 */
struct _IBusConfigService {
    /*< private >*/
    IBusService parent;

    /* instance members */
};

struct _IBusConfigServiceClass {
    IBusServiceClass parent;

    /* class members */
    gboolean    (* set_value) (IBusConfigService    *config,
                               const gchar          *section,
                               const gchar          *name,
                               const GValue         *value,
                               IBusError           **error);
    gboolean    (* get_value) (IBusConfigService    *config,
                               const gchar          *section,
                               const gchar          *name,
                               GValue               *value,
                               IBusError           **error);

    /*< private >*/
    /* padding */
    gpointer pdummy[14];
};

GType                ibus_config_service_get_type   (void);

/**
 * ibus_config_service_new:
 * @connection: An IBusConnection.
 * @returns: A newly allocated IBusConfigServices.
 *
 * New an IBusConfigService from an IBusConnection.
 */
IBusConfigService   *ibus_config_service_new        (IBusConnection     *connection);

/**
 * ibus_config_service_value_change:
 * @config: An IBusConfigService.
 * @section: Section name of the configuration option.
 * @name: Name of the configure option.
 * @value: GValue that holds the value.
 *
 * Change a value of a configuration option
 * by sending a "ValueChanged" message to IBus service.
 */
void                 ibus_config_service_value_changed
                                                    (IBusConfigService  *config,
                                                     const gchar        *section,
                                                     const gchar        *name,
                                                     const GValue       *value);

G_END_DECLS
#endif

