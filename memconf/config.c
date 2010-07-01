/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (c) 2010, Google Inc. All rights reserved.
 * Copyright (C) 2010 Peng Huang <shawn.p.huang@gmail.com>
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
#include <string.h>
#include <ibus.h>
#include "config.h"


typedef struct _IBusConfigMemconfClass IBusConfigMemconfClass;

struct _IBusConfigMemconf {
    IBusConfigService parent;
    GHashTable *values;
    GHashTable *unread;
    GHashTable *unwritten;
};

struct _IBusConfigMemconfClass {
    IBusConfigServiceClass parent;
};

/* functions prototype */
static void         ibus_config_memconf_class_init  (IBusConfigMemconfClass *class);
static void         ibus_config_memconf_init        (IBusConfigMemconf      *config);
static void         ibus_config_memconf_destroy     (IBusConfigMemconf      *config);
static gboolean     ibus_config_memconf_set_value   (IBusConfigService      *config,
                                                     const gchar            *section,
                                                     const gchar            *name,
                                                     GVariant               *value,
                                                     GError                **error);
static GVariant    *ibus_config_memconf_get_value   (IBusConfigService      *config,
                                                     const gchar            *section,
                                                     const gchar            *name,
                                                     GError                **error);
static gboolean     ibus_config_memconf_unset_value (IBusConfigService      *config,
                                                     const gchar            *section,
                                                     const gchar            *name,
                                                     GError                **error);
static gboolean     ibus_config_memconf_get_unused  (IBusConfigService      *config,
                                                     GVariant              **unread,
                                                     GVariant              **unwritten,
                                                     GError                **error);

G_DEFINE_TYPE (IBusConfigMemconf, ibus_config_memconf, IBUS_TYPE_CONFIG_SERVICE)

static void
ibus_config_memconf_class_init (IBusConfigMemconfClass *class)
{
    GObjectClass *object_class = G_OBJECT_CLASS (class);

    IBUS_OBJECT_CLASS (object_class)->destroy = (IBusObjectDestroyFunc) ibus_config_memconf_destroy;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->set_value   = ibus_config_memconf_set_value;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->get_value   = ibus_config_memconf_get_value;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->unset_value = ibus_config_memconf_unset_value;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->get_unused  = ibus_config_memconf_get_unused;
}

static void
ibus_config_memconf_init (IBusConfigMemconf *config)
{
    config->values = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            (GDestroyNotify) g_free,
                                            (GDestroyNotify) g_variant_unref);
    config->unread = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            (GDestroyNotify) g_free,
                                            (GDestroyNotify) NULL);
    config->unwritten
                   = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            (GDestroyNotify) g_free,
                                            (GDestroyNotify) NULL);
}

static void
ibus_config_memconf_destroy (IBusConfigMemconf *config)
{
    g_hash_table_destroy (config->values);
    g_hash_table_destroy (config->unread);
    g_hash_table_destroy (config->unwritten);
    IBUS_OBJECT_CLASS (ibus_config_memconf_parent_class)->destroy ((IBusObject *)config);
}

static gboolean
ibus_config_memconf_set_value (IBusConfigService *config,
                               const gchar       *section,
                               const gchar       *name,
                               GVariant          *value,
                               GError           **error)
{
    g_assert (IBUS_IS_CONFIG_MEMCONF (config));
    g_assert (section);
    g_assert (name);
    g_assert (value);
    g_assert (error == NULL || *error == NULL);

    gchar *key = g_strdup_printf ("%s%s", section, name);


    if (g_hash_table_lookup (
            IBUS_CONFIG_MEMCONF (config)->values, key) == NULL) {
        g_hash_table_insert (IBUS_CONFIG_MEMCONF (config)->unread,
                             g_strdup (key), NULL);
    }

    g_hash_table_insert (IBUS_CONFIG_MEMCONF (config)->values,
                         key, g_variant_ref_sink (value));

    ibus_config_service_value_changed (config, section, name, value);

    return TRUE;
}

static GVariant *
ibus_config_memconf_get_value (IBusConfigService *config,
                               const gchar       *section,
                               const gchar       *name,
                               GError           **error)
{
    g_assert (IBUS_IS_CONFIG_MEMCONF (config));
    g_assert (section);
    g_assert (name);
    g_assert (error == NULL || *error == NULL);

    gchar *key = g_strdup_printf ("%s%s", section, name);

    GVariant *value = (GVariant *)g_hash_table_lookup (IBUS_CONFIG_MEMCONF (config)->values, key);

    if (value == NULL) {
        g_hash_table_insert (IBUS_CONFIG_MEMCONF (config)->unwritten,
                             g_strdup (key), NULL);
        if (error != NULL) {
            *error = g_error_new (G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                  "Config value [%s:%s] does not exist.", section, name);
        }
    } else {
        g_hash_table_remove (IBUS_CONFIG_MEMCONF (config)->unread, key);
        g_variant_ref (value);
    }
    g_free (key);
    return value;
}

static gboolean
ibus_config_memconf_unset_value (IBusConfigService *config,
                                 const gchar       *section,
                                 const gchar       *name,
                                 GError            **error)
{
    g_assert (IBUS_IS_CONFIG_MEMCONF (config));
    g_assert (section);
    g_assert (name);
    g_assert (error == NULL || *error == NULL);

    gchar *key = g_strdup_printf ("%s%s", section, name);
    gboolean retval = g_hash_table_remove (IBUS_CONFIG_MEMCONF (config)->values, key);

    if (retval) {
        g_hash_table_remove (IBUS_CONFIG_MEMCONF (config)->unread, key);
    }
    else {
        if (error && *error) {
            *error = g_error_new (G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                  "Config value [%s:%s] does not exist.", section, name);
        }
    }
    g_free (key);
    return retval;
}

static gboolean
ibus_config_memconf_get_unused (IBusConfigService      *config,
                                GVariant              **unread,
                                GVariant              **unwritten,
                                GError                **error)
{
    GList *keys;
    GList *p;
    GVariantBuilder builder;

    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

    keys = g_hash_table_get_keys (IBUS_CONFIG_MEMCONF (config)->unread);
    for (p = keys; p != NULL; p = p->next) {
        g_variant_builder_add (&builder, "s", (const gchar *)p->data);
    }
    g_list_free (keys);

    *unread = g_variant_builder_end (&builder);

    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

    keys = g_hash_table_get_keys (IBUS_CONFIG_MEMCONF (config)->unwritten);
    for (p = keys; p != NULL; p = p->next) {
        g_variant_builder_add (&builder, "s", (const gchar *)p->data);
    }
    g_list_free (keys);

    *unwritten = g_variant_builder_end (&builder);

    return TRUE;
}

IBusConfigMemconf *
ibus_config_memconf_new (GDBusConnection *connection)
{
    IBusConfigMemconf *config;
    config = (IBusConfigMemconf *) g_object_new (IBUS_TYPE_CONFIG_MEMCONF,
                                                 "object-path", IBUS_PATH_CONFIG,
                                                 "connection", connection,
                                                 NULL);
    return config;
}
