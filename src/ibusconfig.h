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

#if !defined (__IBUS_H_INSIDE__) && !defined (IBUS_COMPILATION)
#error "Only <ibus.h> can be included directly"
#endif

/**
 * SECTION: ibusconfig
 * @title: IBusConfig
 * @short_description: IBus engine configuration module.
 *
 * An IBusConfig provides engine configuration methods
 * such as get and set the configure settings to configuration file.
 *
 * Currently, IBusConfig supports gconf.
 */
#ifndef __CONFIG_H_
#define __CONFIG_H_

#include "ibusproxy.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_CONFIG             \
    (ibus_config_get_type ())
#define IBUS_CONFIG(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_CONFIG, IBusConfig))
#define IBUS_CONFIG_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_CONFIG, IBusConfigClass))
#define IBUS_IS_CONFIG(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_CONFIG))
#define IBUS_IS_CONFIG_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_CONFIG))
#define IBUS_CONFIG_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_CONFIG, IBusConfigClass))

G_BEGIN_DECLS

typedef struct _IBusConfig IBusConfig;
typedef struct _IBusConfigClass IBusConfigClass;

/**
 * IBusConfig:
 *
 * An opaque data type representing an IBusConfig.
 */
struct _IBusConfig {
  IBusProxy parent;
  /* instance members */
};

struct _IBusConfigClass {
    IBusProxyClass parent;
    /* class members */
};

GType            ibus_config_get_type       (void);

/**
 * ibus_config_new:
 * @connection: An GDBusConnection.
 * @returns: An newly allocated IBusConfig corresponding to @connection.
 *
 * New a IBusConfig from existing GDBusConnection.
 */
IBusConfig      *ibus_config_new            (GDBusConnection    *connection,
                                             GCancellable       *cancellable,
                                             GError            **error);

/**
 * ibus_config_get_value:
 * @config: An IBusConfig
 * @section: Section name of the configuration option.
 * @name: Name of the configure option.
 * @returns: A #GVariant or %NULL. Free with g_variant_unref().
 *
 * Get the value of a configuration option.
 *
 * GConf stores configure options in a tree-like structure,
 * and the IBus related setting is at /desktop/ibus,
 * thus, @section here is a path from there,
 * while @name is the key of that configuration option.
 *
 * ibus-chewing, for example, stores its setting in /desktop/ibus/engine/Chewing,
 * so the section name for it is "engine/Chewing".
 * @see_also: ibus_config_set_value.
 */
GVariant        *ibus_config_get_value      (IBusConfig         *config,
                                             const gchar        *section,
                                             const gchar        *name);

/**
 * ibus_config_get_value_async:
 * @config: An IBusConfig
 * @section: Section name of the configuration option.
 * @name: Name of the configure option.
 * @cancellable: A #GCancellable or %NULL.
 * @callback: Callback function to invoke when the return value is ready.
 *
 * Get the value of a configuration option.
 *
 * @see_also: ibus_config_get_value.
 */
void             ibus_config_get_value_async(IBusConfig         *config,
                                             const gchar        *section,
                                             const gchar        *name,
                                             GCancellable       *cancellable,
                                             GAsyncReadyCallback callback,
                                             gpointer            user_data);

/**
 * ibus_config_get_value_async_finish:
 * @confi: A #IBusConfig.
 * @result: A #GAsyncResult.
 * @error: Return location for error or %NULL.
 * @returns: A #GVariant or %NULL if error is set. Free with g_variant_unref().
 * 
 * Finish get value of a configuration option.
 *
 * @see_also: ibus_config_get_value_async.
 */
GVariant        *ibus_config_get_value_async_finish
                                            (IBusConfig         *config,
                                             GAsyncResult       *result,
                                             GError            **error);

/**
 * ibus_config_set_value:
 * @config: An IBusConfig
 * @section: Section name of the configuration option.
 * @name: Name of the configure option its self.
 * @value: A #GVariant that holds the value.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Set the value of a configuration option.
 * @see_also: ibus_config_get_value.
 */
gboolean         ibus_config_set_value      (IBusConfig         *config,
                                             const gchar        *section,
                                             const gchar        *name,
                                             GVariant           *value);

/**
 * ibus_config_set_value_async:
 * @config: An #IBusConfig
 * @section: Section name of the configuration option.
 * @name: Name of the configure option.
 * @value: A #GVariant that holds the value.
 * @cancellable: A #GCancellable or %NULL.
 * @callback: Callback function to invoke when the return value is ready.
 *
 * Set the value of a configuration option.
 *
 * @see_also: ibus_config_set_value.
 */
void             ibus_config_set_value_async(IBusConfig         *config,
                                             const gchar        *section,
                                             const gchar        *name,
                                             GVariant           *value,
                                             GCancellable       *cancellable,
                                             GAsyncReadyCallback callback,
                                             gpointer            user_data);

/**
 * ibus_config_set_value_async_finish:
 * @confi: A #IBusConfig.
 * @result: A #GAsyncResult.
 * @error: Return location for error or %NULL.
 * @returns: %TRUE or %FALSE if error is set.
 * 
 * Finish set value of a configuration option.
 *
 * @see_also: ibus_config_set_value_async.
 */
gboolean         ibus_config_set_value_async_finish
                                            (IBusConfig         *config,
                                             GAsyncResult       *result,
                                             GError            **error);

/**
 * ibus_config_unset:
 * @config: An IBusConfig
 * @section: Section name of the configuration option.
 * @name: Name of the configure option its self.
 * @returns: TRUE if succeed; FALSE otherwise.
 *
 * Remove an entry of a configuration option.
 * @see_also: ibus_config_get_value.
 */
gboolean         ibus_config_unset      (IBusConfig         *config,
                                         const gchar        *section,
                                         const gchar        *name);
G_END_DECLS
#endif

