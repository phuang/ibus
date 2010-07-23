/* ibus - The Input Bus
 *
 * Copyright (C) 2008-2010 Red Hat, Inc.
 * Copyright (c) 2010, Google Inc. All rights reserved.
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

#include "config.h"

struct IBusConfigMemConfClass {
  IBusConfigServiceClass parent;
};

static void ibus_config_memconf_class_init(IBusConfigMemConfClass* klass);
static void ibus_config_memconf_init(IBusConfigMemConf* config);
static void ibus_config_memconf_destroy(IBusConfigMemConf* config);
static gboolean ibus_config_memconf_set_value(IBusConfigService* config,
                                              const gchar* section,
                                              const gchar* name,
                                              const GValue* value,
                                              IBusError** error);
static gboolean ibus_config_memconf_get_value(IBusConfigService* config,
                                              const gchar* section,
                                              const gchar* name,
                                              GValue* value,
                                              IBusError** error);
static gboolean ibus_config_memconf_unset(IBusConfigService* config,
                                          const gchar* section,
                                          const gchar* name,
                                          IBusError** error);

// Copied from gconf/config.c.
static IBusConfigServiceClass* parent_class = NULL;

// Copied from gconf/config.c.
GType ibus_config_memconf_get_type() {
  static GType type = 0;

  static const GTypeInfo type_info = {
    sizeof(IBusConfigMemConfClass),
    NULL,
    NULL,
    reinterpret_cast<GClassInitFunc>(ibus_config_memconf_class_init),
    NULL,
    NULL,
    sizeof(IBusConfigMemConf),
    0,
    reinterpret_cast<GInstanceInitFunc>(ibus_config_memconf_init),
  };

  if (type == 0) {
    type = g_type_register_static(IBUS_TYPE_CONFIG_SERVICE,
                                  "IBusConfigMemConf",
                                  &type_info,
                                  static_cast<GTypeFlags>(0));
  }

  return type;
}

// Copied from gconf/config.c.
static void ibus_config_memconf_class_init(IBusConfigMemConfClass* klass) {
  GObjectClass* object_class = G_OBJECT_CLASS(klass);

  parent_class = reinterpret_cast<IBusConfigServiceClass*>(
      g_type_class_peek_parent(klass));

  IBUS_OBJECT_CLASS(object_class)->destroy
      = reinterpret_cast<IBusObjectDestroyFunc>(ibus_config_memconf_destroy);
  IBUS_CONFIG_SERVICE_CLASS(object_class)->set_value
      = ibus_config_memconf_set_value;
  IBUS_CONFIG_SERVICE_CLASS(object_class)->get_value
      = ibus_config_memconf_get_value;
  IBUS_CONFIG_SERVICE_CLASS(object_class)->unset = ibus_config_memconf_unset;
}

static void ibus_config_memconf_init(IBusConfigMemConf* config) {
  config->entries = new std::map<std::string, GValue*>;
}

static void ibus_config_memconf_destroy(IBusConfigMemConf* config) {
  if (config) {
    std::map<std::string, GValue*>::iterator iter;
    for (iter = config->entries->begin();
         iter != config->entries->end();
         ++iter) {
      g_value_unset(iter->second);
      g_free(iter->second);
    }
    delete config->entries;
  }
  IBUS_OBJECT_CLASS(parent_class)->destroy(
      reinterpret_cast<IBusObject*>(config));
}

// Remove an entry associated with |key| from the on-memory config database.
static gboolean do_unset(IBusConfigMemConf* memconf, const std::string& key) {
  std::map<std::string, GValue*>::iterator iter = memconf->entries->find(key);
  if (iter != memconf->entries->end()) {
    g_value_unset(iter->second);
    g_free(iter->second);
    memconf->entries->erase(iter);
    return TRUE;
  }

  return FALSE;
}

// Server side implementation of ibus_config_set_value.
static gboolean ibus_config_memconf_set_value(IBusConfigService* config,
                                              const gchar* section,
                                              const gchar* name,
                                              const GValue* value,
                                              IBusError** error) {
  g_return_val_if_fail(config, FALSE);
  g_return_val_if_fail(section, FALSE);
  g_return_val_if_fail(name, FALSE);
  g_return_val_if_fail(value, FALSE);
  g_return_val_if_fail(error, FALSE);

  const std::string key = std::string(section) + name;
  IBusConfigMemConf* memconf = reinterpret_cast<IBusConfigMemConf*>(config);

  GValue* new_entry = g_new0(GValue, 1);
  g_value_init(new_entry, G_VALUE_TYPE(value));
  g_value_copy(value, new_entry);

  do_unset(memconf, key);  // remove an existing entry (if any) first.
  bool result = memconf->entries->insert(std::make_pair(key, new_entry)).second;
  if (!result) {
    g_value_unset(new_entry);
    g_free(new_entry);
    *error = ibus_error_new_from_printf(
        "org.freedesktop.DBus.Error.Failed", "Can not set value [%s->%s]", section, name);
  }

  // Let ibus-daemon know that a new value is set to ibus-memconf. Note that
  // _config_value_changed_cb() function in bus/ibusimpl.c will handle this RPC.
  ibus_config_service_value_changed(config, section, name, value);

  return result ? TRUE : FALSE;
}

// Server side implementation of ibus_config_get_value.
static gboolean ibus_config_memconf_get_value(IBusConfigService* config,
                                              const gchar* section,
                                              const gchar* name,
                                              GValue* value,
                                              IBusError** error) {
  g_return_val_if_fail(config, FALSE);
  g_return_val_if_fail(section, FALSE);
  g_return_val_if_fail(name, FALSE);
  g_return_val_if_fail(value, FALSE);
  g_return_val_if_fail(error, FALSE);

  const std::string key = std::string(section) + name;
  IBusConfigMemConf* memconf = reinterpret_cast<IBusConfigMemConf*>(config);

  std::map<std::string, GValue*>::const_iterator iter
      = memconf->entries->find(key);
  if (iter == memconf->entries->end()) {
    *error = ibus_error_new_from_printf(
        "org.freedesktop.DBus.Error.Failed", "Can not get value [%s->%s]", section, name);
    return FALSE;
  }

  const GValue* entry = iter->second;
  g_value_init(value, G_VALUE_TYPE(entry));
  g_value_copy(entry, value);

  // |value| will be g_value_unset() in the super class after the value is sent
  // to ibus-daemon. See src/ibusconfigservice.c for details.
  return TRUE;
}

// Server side implementation of ibus_config_unset_value.
static gboolean ibus_config_memconf_unset(IBusConfigService* config,
                                          const gchar* section,
                                          const gchar* name,
                                          IBusError** error) {
  g_return_val_if_fail(config, FALSE);
  g_return_val_if_fail(section, FALSE);
  g_return_val_if_fail(name, FALSE);
  g_return_val_if_fail(error, FALSE);

  const std::string key = std::string(section) + name;
  IBusConfigMemConf* memconf = reinterpret_cast<IBusConfigMemConf*>(config);

  if (!do_unset(memconf, key)) {
    *error = ibus_error_new_from_printf(
        "org.freedesktop.DBus.Error.Failed", "Can not unset value [%s->%s]", section, name);
    return FALSE;
  }

  // Note: It is not allowed to call ibus_config_service_value_changed function
  // with zero-cleared GValue, so we don't call the function here.
  // See src/ibusconfigservice.c for details.
  return TRUE;
}

// Copied from gconf/config.c.
IBusConfigMemConf* ibus_config_memconf_new(IBusConnection* connection) {
  IBusConfigMemConf* config = reinterpret_cast<IBusConfigMemConf*>(
      g_object_new(ibus_config_memconf_get_type(),
                   "path", IBUS_PATH_CONFIG,
                   "connection", connection,
                   NULL));
  return config;
}
// TODO(yusukes): Upstream memconf/ code if possible. We might have to rewrite
// the code to C and have to change the coding style though.
