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

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <strings.h>
#include "types.h"
#include "ibusimpl.h"
#include "dbusimpl.h"
#include "server.h"
#include "connection.h"
#include "registry.h"
#include "factoryproxy.h"
#include "panelproxy.h"
#include "inputcontext.h"
#include "option.h"

struct _BusIBusImpl {
    IBusService parent;
    /* instance members */
    GHashTable *factory_dict;
    GList *factory_list;
    GList *contexts;

    GList *engine_list;
    GList *register_engine_list;
    GList *component_list;

    gboolean use_sys_layout;
    gboolean embed_preedit_text;
    gboolean enable_by_default;

    BusRegistry     *registry;

    BusInputContext *focused_context;
    BusPanelProxy   *panel;
    IBusConfig      *config;
    IBusHotkeyProfile *hotkey_profile;
    IBusKeymap      *keymap;

    gboolean use_global_engine;
    BusEngineProxy  *global_engine;
    gchar           *global_previous_engine_name;

    IBusHotkeyProfile *engines_hotkey_profile;
    GHashTable      *hotkey_to_engines_map;
};

struct _BusIBusImplClass {
    IBusServiceClass parent;

    /* class members */
};

enum {
    LAST_SIGNAL,
};

enum {
    PROP_0,
};

/*
static guint            _signals[LAST_SIGNAL] = { 0 };
*/

/* functions prototype */
static void      bus_ibus_impl_destroy           (BusIBusImpl        *ibus);
static void      bus_ibus_impl_service_method_call
                                              (IBusService        *service,
                                               GDBusConnection    *connection,
                                               const gchar        *sender,
                                               const gchar        *object_path,
                                               const gchar        *interface_name,
                                               const gchar        *method_name,
                                               GVariant           *parameters,
                                               GDBusMethodInvocation
                                                                  *invocation);
/* FIXME */
#if 0
static GVariant *ibus_ibus_impl_service_get_property
                                             (IBusService        *service,
                                              GDBusConnection    *connection,
                                              const gchar        *sender,
                                              const gchar        *object_path,
                                              const gchar        *interface_name,
                                              const gchar        *property_name,
                                              GError            **error);
static gboolean  ibus_ibus_impl_service_set_property
                                             (IBusService        *service,
                                              GDBusConnection    *connection,
                                              const gchar        *sender,
                                              const gchar        *object_path,
                                              const gchar        *interface_name,
                                              const gchar        *property_name,
                                              GVariant           *value,
                                              GError            **error);
#endif
static void     bus_ibus_impl_add_factory       (BusIBusImpl        *ibus,
                                                 BusFactoryProxy    *factory);
static void     bus_ibus_impl_set_trigger       (BusIBusImpl        *ibus,
                                                 GVariant           *value);
static void     bus_ibus_impl_set_next_engine_in_menu
                                                (BusIBusImpl        *ibus,
                                                 GVariant           *value);
static void     bus_ibus_impl_set_previous_engine
                                                (BusIBusImpl        *ibus,
                                                 GVariant           *value);
static void     bus_ibus_impl_set_preload_engines
                                                (BusIBusImpl        *ibus,
                                                 GVariant           *value);
static void     bus_ibus_impl_set_use_sys_layout
                                                (BusIBusImpl        *ibus,
                                                 GVariant           *value);
static void     bus_ibus_impl_set_embed_preedit_text
                                                (BusIBusImpl        *ibus,
                                                 GVariant           *value);
static void     bus_ibus_impl_set_enable_by_default
                                                (BusIBusImpl        *ibus,
                                                 GVariant           *value);
static void     bus_ibus_impl_set_use_global_engine
                                                (BusIBusImpl        *ibus,
                                                 GVariant           *value);
static void     bus_ibus_impl_set_global_engine (BusIBusImpl        *ibus,
                                                 BusEngineProxy     *engine);

static void     bus_ibus_impl_registry_changed  (BusIBusImpl        *ibus);
static void     bus_ibus_impl_global_engine_changed
                                                (BusIBusImpl        *ibus);

static void     _factory_destroy_cb             (BusFactoryProxy    *factory,
                                                 BusIBusImpl        *ibus);

static void     bus_ibus_impl_set_context_engine_from_desc
                                                (BusIBusImpl        *ibus,
                                                 BusInputContext    *context,
                                                 IBusEngineDesc     *engine_desc);
static void     bus_ibus_impl_set_context_engine(BusIBusImpl        *ibus,
                                                 BusInputContext    *context,
                                                 BusEngineProxy     *engine);

static gchar   *bus_ibus_impl_load_global_engine_name_from_config
                                                (BusIBusImpl        *ibus);
static void     bus_ibus_impl_save_global_engine_name_to_config
                                                (BusIBusImpl        *ibus);

static gchar   *bus_ibus_impl_load_global_previous_engine_name_from_config
                                                (BusIBusImpl        *ibus);
static void     bus_ibus_impl_save_global_previous_engine_name_to_config
                                                (BusIBusImpl        *ibus);
static void     bus_ibus_impl_update_engines_hotkey_profile
                                                (BusIBusImpl        *ibus);

static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='org.freedesktop.IBus'>"
    "    <method name='GetAddress'>"
    "      <arg direction='out' type='s' name='address' />"
    "    </method>"
    "    <method name='CreateInputContext'>"
    "      <arg direction='in'  type='s' name='client_name' />"
    "      <arg direction='out' type='o' name='object_path' />"
    "    </method>"
    "    <method name='CurrentInputContext'>"
    "      <arg direction='out' type='o' name='object_path' />"
    "    </method>"
    "    <method name='RegisterComponent'>"
    "      <arg direction='in'  type='v' name='component' />"
    "    </method>"
    "    <method name='ListEngines'>"
    "      <arg direction='out' type='av' name='engines' />"
    "    </method>"
    "    <method name='ListActiveEngines'>"
    "      <arg direction='out' type='av' name='engines' />"
    "    </method>"
    "    <method name='Exit'>"
    "      <arg direction='in'  type='b' name='restart' />"
    "    </method>"
    "    <method name='Ping'>"
    "      <arg direction='in'  type='v' name='data' />"
    "      <arg direction='out' type='v' name='data' />"
    "    </method>"
    "    <method name='GetUseSysLayout'>"
    "      <arg direction='out' type='b' name='enabled' />"
    "    </method>"
    "    <method name='GetUseGlobalEngine'>"
    "      <arg direction='out' type='b' name='enabled' />"
    "    </method>"
    "    <method name='GetGlobalEngine'>"
    "      <arg direction='out' type='v' name='desc' />"
    "    </method>"
    "    <method name='SetGlobalEngine'>"
    "      <arg direction='in'  type='s' name='engine_name' />"
    "    </method>"
    "    <method name='IsGlobalEngineEnabled'>"
    "      <arg direction='out' type='b' name='enabled' />"
    "    </method>"
    "    <signal name='RegistryChanged'>"
    "    </signal>"
    "    <signal name='GlobalEngineChanged'>"
    "    </signal>"
    "  </interface>"
    "</node>";


G_DEFINE_TYPE(BusIBusImpl, bus_ibus_impl, IBUS_TYPE_SERVICE)

static void
bus_ibus_impl_class_init (BusIBusImplClass *class)
{
    IBUS_OBJECT_CLASS(class)->destroy = (IBusObjectDestroyFunc) bus_ibus_impl_destroy;

    IBUS_SERVICE_CLASS (class)->service_method_call = bus_ibus_impl_service_method_call;
    ibus_service_class_add_interfaces (IBUS_SERVICE_CLASS (class), introspection_xml);
}

static void
_panel_destroy_cb (BusPanelProxy *panel,
                   BusIBusImpl   *ibus)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    g_return_if_fail (ibus->panel == panel);

    ibus->panel = NULL;
    g_object_unref (panel);
}

static void
bus_ibus_impl_set_hotkey (BusIBusImpl *ibus,
                          GQuark       hotkey,
                          GVariant     *value)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    ibus_hotkey_profile_remove_hotkey_by_event (ibus->hotkey_profile, hotkey);

    if (value == NULL) {
        return;
    }

    GVariantIter iter;
    g_variant_iter_init (&iter, value);
    const gchar *str = NULL;
    while (g_variant_iter_loop (&iter,"&s", &str)) {
       ibus_hotkey_profile_add_hotkey_from_string (ibus->hotkey_profile,
                                                   str,
                                                   hotkey);
    }

}

static void
bus_ibus_impl_set_trigger (BusIBusImpl *ibus,
                           GVariant    *value)
{
    GQuark hotkey = g_quark_from_static_string ("trigger");
    if (value != NULL) {
        bus_ibus_impl_set_hotkey (ibus, hotkey, value);
    }
    else {
        /* set defaint trigger */
        ibus_hotkey_profile_add_hotkey (ibus->hotkey_profile,
                                        IBUS_space,
                                        IBUS_CONTROL_MASK,
                                        hotkey);
    }
}

static void
bus_ibus_impl_set_next_engine_in_menu (BusIBusImpl *ibus,
                                       GVariant     *value)
{
    GQuark hotkey = g_quark_from_static_string ("next-engine-in-menu");
    bus_ibus_impl_set_hotkey (ibus, hotkey, value);
}

static void
bus_ibus_impl_set_previous_engine (BusIBusImpl *ibus,
                                   GVariant    *value)
{
    GQuark hotkey = g_quark_from_static_string ("previous-engine");
    bus_ibus_impl_set_hotkey (ibus, hotkey, value);
}

static void
bus_ibus_impl_set_preload_engines (BusIBusImpl *ibus,
                                   GVariant    *value)
{
    GList *engine_list = NULL;

    g_list_foreach (ibus->engine_list, (GFunc) g_object_unref, NULL);
    g_list_free (ibus->engine_list);

    if (value != NULL && g_variant_classify (value) == G_VARIANT_CLASS_ARRAY) {
        GVariantIter iter;
        g_variant_iter_init (&iter, value);
        const gchar *engine_name = NULL;
        while (g_variant_iter_loop (&iter, "&s", &engine_name)) {
            IBusEngineDesc *engine = bus_registry_find_engine_by_name (ibus->registry, engine_name);
            if (engine == NULL || g_list_find (engine_list, engine) != NULL)
                continue;
            engine_list = g_list_append (engine_list, engine);
        }
    }

    g_list_foreach (engine_list, (GFunc) g_object_ref, NULL);
    ibus->engine_list = engine_list;

    if (ibus->engine_list) {
        IBusComponent *component = ibus_component_get_from_engine ((IBusEngineDesc *) ibus->engine_list->data);
        if (component && !ibus_component_is_running (component)) {
            ibus_component_start (component, g_verbose);
        }
    }

    bus_ibus_impl_update_engines_hotkey_profile (ibus);
}

static void
bus_ibus_impl_set_use_sys_layout (BusIBusImpl *ibus,
                                  GVariant    *value)
{
    if (value != NULL && g_variant_classify (value) == G_VARIANT_CLASS_BOOLEAN) {
        ibus->use_sys_layout = g_variant_get_boolean (value);
    }
}

static void
bus_ibus_impl_set_embed_preedit_text (BusIBusImpl *ibus,
                                      GVariant    *value)
{
    if (value != NULL && g_variant_classify (value) == G_VARIANT_CLASS_BOOLEAN) {
        ibus->embed_preedit_text = g_variant_get_boolean (value);
    }
}

static void
bus_ibus_impl_set_enable_by_default (BusIBusImpl *ibus,
                                     GVariant    *value)
{
    if (value != NULL && g_variant_classify (value) == G_VARIANT_CLASS_BOOLEAN) {
        ibus->enable_by_default = g_variant_get_boolean (value);
    }
}

static void
bus_ibus_impl_set_use_global_engine (BusIBusImpl *ibus,
                                     GVariant    *value)
{
    if (value == NULL || g_variant_classify (value) != G_VARIANT_CLASS_BOOLEAN)
        return;

    gboolean new_value = g_variant_get_boolean (value);
    if (ibus->use_global_engine == new_value)
        return;

    if (new_value) {
        /* turn on use_global_engine option */
        ibus->use_global_engine = TRUE;
        BusEngineProxy *engine = ibus->focused_context != NULL ?
                    bus_input_context_get_engine (ibus->focused_context) : NULL;
        if (engine != NULL) {
            bus_ibus_impl_set_global_engine (ibus, engine);
        }
    }
    else {
        /* turn off use_global_engine option */
        bus_ibus_impl_set_global_engine (ibus, NULL);
        ibus->use_global_engine = FALSE;
        g_free (ibus->global_previous_engine_name);
    }
}

static gint
_engine_desc_cmp (IBusEngineDesc *desc1,
                  IBusEngineDesc *desc2)
{
    return - ((gint) desc1->rank) + ((gint) desc2->rank);
}

static void
bus_ibus_impl_set_default_preload_engines (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    static gboolean done = FALSE;

    if (done || ibus->config == NULL) {
        return;
    }

    GVariant *variant = ibus_config_get_value (ibus->config, "general", "preload_engines");
    if (variant != NULL) {
        done = TRUE;
        g_variant_unref (variant);
        return;
    }

    done = TRUE;
    gchar *lang = g_strdup (setlocale (LC_ALL, NULL));
    gchar *p = index (lang, '.');
    if (p) {
        *p = '\0';
    }

    GList *engines = bus_registry_get_engines_by_language (ibus->registry, lang);
    if (engines == NULL) {
        p = index (lang, '_');
        if (p) {
            *p = '\0';
            engines = bus_registry_get_engines_by_language (ibus->registry, lang);
        }
    }
    g_free (lang);

    /* sort engines by rank */
    engines = g_list_sort (engines, (GCompareFunc) _engine_desc_cmp);

    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));
    GList *list;
    for (list = engines; list != NULL; list = list->next) {
        IBusEngineDesc *desc = (IBusEngineDesc *)list->data;
        /* ignore engines with rank <== 0 */
        if (desc->rank > 0)
            g_variant_builder_add (&builder, "s", desc->name);
    }
    ibus_config_set_value (ibus->config,
                    "general", "preload_engines", g_variant_builder_end (&builder));
    g_list_free (engines);
}

const static struct {
    gchar *section;
    gchar *key;
    void ( *func) (BusIBusImpl *, GVariant *);
} bus_ibus_impl_config_items [] = {
    { "general/hotkey", "trigger",              bus_ibus_impl_set_trigger },
    { "general/hotkey", "next_engine_in_menu",  bus_ibus_impl_set_next_engine_in_menu },
    { "general/hotkey", "previous_engine",      bus_ibus_impl_set_previous_engine },
    { "general", "preload_engines",             bus_ibus_impl_set_preload_engines },
    { "general", "use_system_keyboard_layout",  bus_ibus_impl_set_use_sys_layout },
    { "general", "use_global_engine",           bus_ibus_impl_set_use_global_engine },
    { "general", "embed_preedit_text",          bus_ibus_impl_set_embed_preedit_text },
    { "general", "enable_by_default",           bus_ibus_impl_set_enable_by_default },
};

static void
bus_ibus_impl_reload_config (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    gint i;
    for (i = 0; i < G_N_ELEMENTS (bus_ibus_impl_config_items); i++) {
        GVariant *variant = NULL;
        if (ibus->config != NULL)
            variant = ibus_config_get_value (ibus->config,
                            bus_ibus_impl_config_items[i].section,
                            bus_ibus_impl_config_items[i].key);
        bus_ibus_impl_config_items[i].func (ibus, variant);
        if (variant) g_variant_unref (variant);
    }
}

static void
_config_value_changed_cb (IBusConfig  *config,
                          gchar       *section,
                          gchar       *key,
                          GVariant    *value,
                          BusIBusImpl *ibus)
{
    g_assert (IBUS_IS_CONFIG (config));
    g_assert (section);
    g_assert (key);
    g_assert (value);
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    gint i;
    for (i = 0; i < G_N_ELEMENTS (bus_ibus_impl_config_items); i++) {
        if (g_strcmp0 (bus_ibus_impl_config_items[i].section, section) == 0 &&
            g_strcmp0 (bus_ibus_impl_config_items[i].key, key) == 0) {
            bus_ibus_impl_config_items[i].func (ibus, value);
            break;
        }
    }
}

static void
_config_destroy_cb (IBusConfig  *config,
                    BusIBusImpl *ibus)
{
    g_assert (IBUS_IS_CONFIG (config));
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    g_assert (ibus->config == config);

    g_object_unref (ibus->config);
    ibus->config = NULL;
}

static void
_registry_changed_cb (BusRegistry *registry,
                      BusIBusImpl *ibus)
{
    bus_ibus_impl_registry_changed (ibus);
}

static void
_dbus_name_owner_changed_cb (BusDBusImpl *dbus,
                             const gchar *name,
                             const gchar *old_name,
                             const gchar *new_name,
                             BusIBusImpl *ibus)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (name != NULL);
    g_assert (old_name != NULL);
    g_assert (new_name != NULL);
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    BusFactoryProxy *factory;

    if (g_strcmp0 (name, IBUS_SERVICE_PANEL) == 0) {
        if (g_strcmp0 (new_name, "") != 0) {
            BusConnection *connection;

            if (ibus->panel != NULL) {
                ibus_proxy_destroy ((IBusProxy *)ibus->panel);
                /* panel should be NULL after destroy */
                g_assert (ibus->panel == NULL);
            }

            connection = bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS, new_name);
            g_return_if_fail (connection != NULL);

            ibus->panel = bus_panel_proxy_new (connection);
            g_object_ref_sink (ibus->panel);

            g_signal_connect (ibus->panel,
                              "destroy",
                              G_CALLBACK (_panel_destroy_cb),
                              ibus);

            if (ibus->focused_context != NULL) {
                bus_panel_proxy_focus_in (ibus->panel, ibus->focused_context);
            }
        }
    }
    else if (g_strcmp0 (name, IBUS_SERVICE_CONFIG) == 0) {
        if (g_strcmp0 (new_name, "") != 0) {
            BusConnection *connection;

            if (ibus->config != NULL) {
                ibus_proxy_destroy ((IBusProxy *)ibus->config);
                /* config should be NULL */
                g_assert (ibus->config == NULL);
            }

            connection = bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS, new_name);
            g_return_if_fail (connection != NULL);

            ibus->config = g_object_new (IBUS_TYPE_CONFIG,
                                         "g-object-path", "/org/freedesktop/IBus/Config",
                                         "g-interface-name", "org.freedesktop.IBus.Config",
                                         "g-connection", bus_connection_get_dbus_connection (connection),
                                         NULL);
            g_object_ref_sink (ibus->config);

            g_signal_connect (ibus->config,
                              "value-changed",
                              G_CALLBACK (_config_value_changed_cb),
                              ibus);

            g_signal_connect (ibus->config,
                              "destroy",
                              G_CALLBACK (_config_destroy_cb),
                              ibus);

            bus_ibus_impl_set_default_preload_engines (ibus);
            bus_ibus_impl_reload_config (ibus);
        }
    }

    factory = bus_registry_name_owner_changed (ibus->registry, name, old_name, new_name);

    if (factory) {
        bus_ibus_impl_add_factory (ibus, factory);
    }
}

static void
bus_ibus_impl_init (BusIBusImpl *ibus)
{
    ibus->factory_dict = g_hash_table_new_full (
                            g_str_hash,
                            g_str_equal,
                            NULL,
                            (GDestroyNotify) g_object_unref);

    ibus->engine_list = NULL;
    ibus->register_engine_list = NULL;
    ibus->contexts = NULL;
    ibus->focused_context = NULL;
    ibus->panel = NULL;
    ibus->config = NULL;

    ibus->registry = bus_registry_new ();

    g_signal_connect (ibus->registry,
                      "changed",
                      G_CALLBACK (_registry_changed_cb),
                      ibus);
#ifdef G_THREADS_ENABLED
    extern gint g_monitor_timeout;
    if (g_monitor_timeout != 0) {
        /* Start the monitor of registry changes. */
        bus_registry_start_monitor_changes (ibus->registry);
    }
#endif

    ibus->hotkey_profile = ibus_hotkey_profile_new ();
    ibus->keymap = ibus_keymap_get ("us");

    ibus->use_sys_layout = FALSE;
    ibus->embed_preedit_text = TRUE;
    ibus->enable_by_default = FALSE;
    ibus->use_global_engine = FALSE;
    ibus->global_engine = NULL;
    ibus->global_previous_engine_name = NULL;

    ibus->engines_hotkey_profile = NULL;
    ibus->hotkey_to_engines_map = NULL;

    bus_ibus_impl_reload_config (ibus);

    g_signal_connect (BUS_DEFAULT_DBUS,
                      "name-owner-changed",
                      G_CALLBACK (_dbus_name_owner_changed_cb),
                      ibus);
}

static void
bus_ibus_impl_destroy (BusIBusImpl *ibus)
{
    pid_t pid;
    glong timeout;
    gint status;
    gboolean flag;

    bus_registry_stop_all_components (ibus->registry);

    pid = 0;
    timeout = 0;
    flag = FALSE;
    while (1) {
        while ((pid = waitpid (0, &status, WNOHANG)) > 0);

        if (pid == -1) { /* all children finished */
            break;
        }
        if (pid == 0) { /* no child status changed */
            g_usleep (1000);
            timeout += 1000;
            if (timeout >= G_USEC_PER_SEC) {
                if (flag == FALSE) {
                    gpointer old;
                    old = signal (SIGTERM, SIG_IGN);
                    kill (-getpid (), SIGTERM);
                    signal (SIGTERM, old);
                    flag = TRUE;
                }
                else {
                    g_warning ("Not every child processes exited!");
                    break;
                }
            }
        }
    };

    g_list_foreach (ibus->engine_list, (GFunc) g_object_unref, NULL);
    g_list_free (ibus->engine_list);
    ibus->engine_list = NULL;

    g_list_foreach (ibus->register_engine_list, (GFunc) g_object_unref, NULL);
    g_list_free (ibus->register_engine_list);
    ibus->register_engine_list = NULL;

    if (ibus->factory_dict != NULL) {
        g_hash_table_destroy (ibus->factory_dict);
        ibus->factory_dict = NULL;
    }

    if (ibus->hotkey_profile != NULL) {
        g_object_unref (ibus->hotkey_profile);
        ibus->hotkey_profile = NULL;
    }

    if (ibus->keymap != NULL) {
        g_object_unref (ibus->keymap);
        ibus->keymap = NULL;
    }

    if (ibus->global_engine) {
        g_object_unref (ibus->global_engine);
        ibus->global_engine = NULL;
    }

    g_free (ibus->global_previous_engine_name);

    if (ibus->engines_hotkey_profile != NULL) {
        g_object_unref (ibus->engines_hotkey_profile);
        ibus->engines_hotkey_profile = NULL;
    }

    if (ibus->hotkey_to_engines_map) {
        g_hash_table_unref (ibus->hotkey_to_engines_map);
        ibus->hotkey_to_engines_map = NULL;
    }

    bus_server_quit ();
    IBUS_OBJECT_CLASS(bus_ibus_impl_parent_class)->destroy (IBUS_OBJECT (ibus));
}

static void
_ibus_get_address (BusIBusImpl           *ibus,
                   GVariant              *parameters,
                   GDBusMethodInvocation *invocation)
{
    /* FIXME */
    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(s)", "FIXME"));
}


static gboolean
_timeout_cb (gpointer data)
{
    return TRUE;
}

static BusFactoryProxy *
_get_factory_proxy(IBusEngineDesc *engine_desc)
{
    BusFactoryProxy *factory = NULL;

    /* Add a timeout to wake up g_main_context_iteration in every 0.5 second,
     * and then to check the factory assocated with the engine_desc */
    guint timeout_id = g_timeout_add (500, _timeout_cb, NULL);

    GTimer *timer = g_timer_new ();

    /* Leave the loop, if it spends more than 5 seconds */
    while (g_timer_elapsed (timer, NULL) <= 5.0) {
        if (g_main_context_iteration (NULL, TRUE)) {
            factory = bus_factory_proxy_get_from_engine (engine_desc);
            if (factory != NULL) {
                break;
            }
        }
    }

    g_source_remove (timeout_id);
    g_timer_destroy (timer);

    return factory;
}

static BusEngineProxy *
bus_ibus_impl_create_engine (IBusEngineDesc *engine_desc)
{
    IBusComponent *comp;
    BusFactoryProxy *factory;
    BusEngineProxy *engine;

    factory = bus_factory_proxy_get_from_engine (engine_desc);

    if (factory == NULL) {
        /* try to execute the engine */
        comp = ibus_component_get_from_engine (engine_desc);
        g_assert (comp);

        if (!ibus_component_is_running (comp)) {
            ibus_component_start (comp, g_verbose);
        }
        factory = _get_factory_proxy (engine_desc);
    }

    if (factory == NULL) {
        return NULL;
    }

    g_object_ref (factory);
    engine = bus_factory_proxy_create_engine (factory, engine_desc);
    g_object_unref (factory);

    return engine;
}

static IBusEngineDesc *
_find_engine_desc_by_name(BusIBusImpl *ibus,
                          const gchar *engine_name)
{
    IBusEngineDesc *engine_desc = NULL;
    GList *p;

    /* find engine in registered engine list */
    for (p = ibus->register_engine_list; p != NULL; p = p->next) {
        engine_desc = (IBusEngineDesc *)p->data;
        if (g_strcmp0 (engine_desc->name, engine_name) == 0)
            return engine_desc;
    }

    /* find engine in preload engine list */
    for (p = ibus->engine_list; p != NULL; p = p->next) {
        engine_desc = (IBusEngineDesc *)p->data;
        if (g_strcmp0 (engine_desc->name, engine_name) == 0)
            return engine_desc;
    }

    return NULL;
}

static void
_context_request_engine_cb (BusInputContext *context,
                            const gchar     *engine_name,
                            BusIBusImpl     *ibus)
{
    IBusEngineDesc *engine_desc = NULL;

    /* context should has focus before request an engine */
    g_return_if_fail (bus_input_context_has_focus (context));

    if (engine_name == NULL || engine_name[0] == '\0') {
        /* Use global engine if possible. */
        if (ibus->use_global_engine) {
            if (ibus->global_engine) {
                bus_ibus_impl_set_context_engine (ibus, context, ibus->global_engine);
                return;
            }
            else {
                gchar *name = bus_ibus_impl_load_global_engine_name_from_config (ibus);
                if (name) {
                    engine_desc = _find_engine_desc_by_name (ibus, name);
                    g_free (name);
                }
            }
        }
        /* request default engine */
        if (!engine_desc) {
            if (ibus->register_engine_list) {
                engine_desc = (IBusEngineDesc *)ibus->register_engine_list->data;
            }
            else if (ibus->engine_list) {
                engine_desc = (IBusEngineDesc *)ibus->engine_list->data;
            }
         }
    }
    else {
        /* request engine by name */
        engine_desc = _find_engine_desc_by_name (ibus, engine_name);
    }

    bus_ibus_impl_set_context_engine_from_desc (ibus, context, engine_desc);
}

static void
bus_ibus_impl_context_request_next_engine_in_menu (BusIBusImpl     *ibus,
                                                   BusInputContext *context)
{
    BusEngineProxy *engine;
    IBusEngineDesc *desc;
    IBusEngineDesc *next_desc = NULL;
    GList *p;

    engine = bus_input_context_get_engine (context);
    if (engine == NULL) {
        _context_request_engine_cb (context, NULL, ibus);
        return;
    }

    desc = bus_engine_proxy_get_desc (engine);

    p = g_list_find (ibus->register_engine_list, desc);
    if (p != NULL) {
        p = p->next;
    }
    if (p == NULL) {
        p = g_list_find (ibus->engine_list, desc);
        if (p != NULL) {
            p = p->next;
        }
    }

    if (p != NULL) {
        next_desc = (IBusEngineDesc*) p->data;
    }
    else {
        if (ibus->register_engine_list) {
            next_desc = (IBusEngineDesc *)ibus->register_engine_list->data;
        }
        else if (ibus->engine_list) {
            next_desc = (IBusEngineDesc *)ibus->engine_list->data;
        }
    }

    bus_ibus_impl_set_context_engine_from_desc (ibus, context, next_desc);
}

static void
bus_ibus_impl_context_request_previous_engine (BusIBusImpl     *ibus,
                                               BusInputContext *context)
{
    gchar *engine_name = NULL;
    if (!ibus->use_global_engine) {
        engine_name = (gchar *) g_object_get_data (G_OBJECT (context), "previous-engine-name");
    }
    else {
        if (!ibus->global_previous_engine_name) {
            ibus->global_previous_engine_name = bus_ibus_impl_load_global_previous_engine_name_from_config (ibus);
        }
        engine_name = ibus->global_previous_engine_name;
    }

    /*
     * If the previous engine name is not found, switch to the next engine
     * in the menu. This behavior is better than doing nothing.
     */
    if (!engine_name) {
        bus_ibus_impl_context_request_next_engine_in_menu(ibus, context);
        return;
    }
    _context_request_engine_cb (context, engine_name, ibus);
}

static void
_global_engine_destroy_cb (BusEngineProxy *engine,
                           BusIBusImpl    *ibus)
{
    if (ibus->global_engine != engine) {
        return;
    }

    g_signal_handlers_disconnect_by_func (ibus->global_engine,
                G_CALLBACK (_global_engine_destroy_cb), ibus);
    g_object_unref (ibus->global_engine);
    ibus->global_engine = NULL;
}

static void
bus_ibus_impl_set_global_engine (BusIBusImpl    *ibus,
                                 BusEngineProxy *engine)
{
    g_assert (ibus->use_global_engine == TRUE);

    if (ibus->global_engine == engine) {
        return;
    }

    g_free (ibus->global_previous_engine_name);
    ibus->global_previous_engine_name = NULL;
    if (ibus->global_engine) {
        /* Save the current global engine's name as previous engine. */
        ibus->global_previous_engine_name = g_strdup (bus_engine_proxy_get_desc (ibus->global_engine)->name);

        ibus_proxy_destroy ((IBusProxy *)ibus->global_engine);
        /* global_engine should be NULL */
        g_assert (ibus->global_engine == NULL);
    }

    if (engine != NULL && !IBUS_PROXY_DESTROYED (engine)) {
        g_object_ref (engine);
        ibus->global_engine = engine;
        g_signal_connect (ibus->global_engine, "destroy",
                G_CALLBACK (_global_engine_destroy_cb), ibus);
    }

    bus_ibus_impl_save_global_engine_name_to_config (ibus);
    bus_ibus_impl_save_global_previous_engine_name_to_config (ibus);
    bus_ibus_impl_global_engine_changed (ibus);
}

static void
bus_ibus_impl_set_context_engine_from_desc (BusIBusImpl     *ibus,
                                            BusInputContext *context,
                                            IBusEngineDesc  *engine_desc)
{
    if (engine_desc != NULL) {
        BusEngineProxy *engine = bus_ibus_impl_create_engine (engine_desc);
        if (engine != NULL) {
            bus_ibus_impl_set_context_engine (ibus, context, engine);
        }
    }
}

static void
bus_ibus_impl_set_context_engine (BusIBusImpl     *ibus,
                                  BusInputContext *context,
                                  BusEngineProxy  *engine) {
  g_object_set_data (G_OBJECT (context), "previous-engine-name", NULL);

  /* If use_global_engine is disabled, then we need to save the previous engine
   * of each input context. */
  if (!ibus->use_global_engine) {
      BusEngineProxy *previous_engine = bus_input_context_get_engine (context);
      if (previous_engine) {
          g_object_set_data_full (G_OBJECT (context), "previous-engine-name",
                                  g_strdup (bus_engine_proxy_get_desc (previous_engine)->name),
                                  g_free);
      }
  }

  bus_input_context_set_engine (context, engine);
}

static void
_context_engine_changed_cb (BusInputContext *context,
                            BusIBusImpl     *ibus)
{
    BusEngineProxy *engine;

    if (context != ibus->focused_context || !ibus->use_global_engine) {
        return;
    }

    engine = bus_input_context_get_engine (context);
    if (engine != NULL) {
        /* only set global engine if engine is not NULL */
        bus_ibus_impl_set_global_engine (ibus, engine);
    }
}

static void
_context_focus_out_cb (BusInputContext    *context,
                       BusIBusImpl        *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    /* Do noting if context does not support focus.
     * Actually, the context should emit focus signals, if it does not support focus */
    if ((bus_input_context_get_capabilities (context) & IBUS_CAP_FOCUS) == 0) {
        return;
    }

    /* Do noting if it is not focused context. */
    if (ibus->focused_context != context) {
        return;
    }

    ibus->focused_context = NULL;

    if (ibus->panel != NULL) {
        bus_panel_proxy_focus_out (ibus->panel, context);
    }

    /* If the use_global_engine option is enabled,
     * the global engine shoule be detached from the focused context. */
    if (ibus->use_global_engine) {
        bus_ibus_impl_set_context_engine (ibus, context, NULL);
    }

    g_object_unref (context);
}

static void
_context_focus_in_cb (BusInputContext *context,
                      BusIBusImpl     *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    /* Do nothing if context does not support focus.
     * The global engine shoule be detached from the focused context. */
    if ((bus_input_context_get_capabilities (context) & IBUS_CAP_FOCUS) == 0) {
        return;
    }

    /* Do nothing if it is focused context already. */
    if (ibus->focused_context == context) {
        return;
    }

    if (ibus->focused_context) {
        /* focus out context */
        bus_input_context_focus_out (ibus->focused_context);
        g_assert (ibus->focused_context == NULL);
    }

    /* If the use_global_engine option is enabled, then we need:
     * - Switch the context to use the global engine or save the context's
     *   existing engine as global engine.
     * - Set the context's enabled state according to the saved state.
     * Note: we get this signal only if the context supports IBUS_CAP_FOCUS. */
    if (ibus->use_global_engine) {
        if (!ibus->global_engine) {
            bus_ibus_impl_set_global_engine (ibus, bus_input_context_get_engine (context));
        }
        else {
            bus_ibus_impl_set_context_engine (ibus, context, ibus->global_engine);
            if (ibus->global_engine && bus_engine_proxy_is_enabled (ibus->global_engine)) {
                bus_input_context_enable (context);
            }
        }
    }

    if (ibus->panel != NULL) {
        bus_panel_proxy_focus_in (ibus->panel, context);
    }

    g_object_ref (context);
    ibus->focused_context = context;
}

static void
_context_destroy_cb (BusInputContext    *context,
                     BusIBusImpl        *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context == ibus->focused_context) {
        /* focus out context */
        bus_input_context_focus_out (ibus->focused_context);
        g_assert (ibus->focused_context == NULL);
    }

    ibus->contexts = g_list_remove (ibus->contexts, context);
    g_object_unref (context);
}

#if 0
static void
_context_enabled_cb (BusInputContext    *context,
                     BusIBusImpl        *ibus)
{
}

static void
_context_disabled_cb (BusInputContext    *context,
                      BusIBusImpl        *ibus)
{
}
#endif

static void
_ibus_create_input_context (BusIBusImpl           *ibus,
                            GVariant              *parameters,
                            GDBusMethodInvocation *invocation)
{
    const gchar *client_name = NULL;
    g_variant_get (parameters, "(&s)", &client_name);

    BusConnection *connection = bus_connection_lookup (g_dbus_method_invocation_get_connection (invocation));
    BusInputContext *context = bus_input_context_new (connection, client_name);
    g_object_ref_sink (context);
    ibus->contexts = g_list_append (ibus->contexts, context);

    static const struct {
        gchar *name;
        GCallback callback;
    } signals [] = {
        { "request-engine",      G_CALLBACK (_context_request_engine_cb) },
        { "engine-changed", G_CALLBACK (_context_engine_changed_cb) },
        { "focus-in",       G_CALLBACK (_context_focus_in_cb) },
        { "focus-out",      G_CALLBACK (_context_focus_out_cb) },
        { "destroy",        G_CALLBACK (_context_destroy_cb) },
    #if 0
        { "enabled",        G_CALLBACK (_context_enabled_cb) },
        { "disabled",       G_CALLBACK (_context_disabled_cb) },
    #endif
    };

    gint i;
    for (i = 0; i < G_N_ELEMENTS (signals); i++) {
        g_signal_connect (context,
                          signals[i].name,
                          signals[i].callback,
                          ibus);
    }

    if (ibus->enable_by_default) {
        bus_input_context_enable (context);
    }

    const gchar *path = ibus_service_get_object_path ((IBusService *) context);
    bus_dbus_impl_register_object (BUS_DEFAULT_DBUS,
                                   (IBusService *)context);
    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(o)", path));
}

static void
_ibus_current_input_context (BusIBusImpl           *ibus,
                             GVariant              *parameters,
                             GDBusMethodInvocation *invocation)
{
    if (!ibus->focused_context)
    {
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "No focused input context");
    }
    else {
        const gchar *path = ibus_service_get_object_path ((IBusService *)ibus->focused_context);
        g_dbus_method_invocation_return_value (invocation, g_variant_new ("(o)", path));
    }
}

static void
_factory_destroy_cb (BusFactoryProxy    *factory,
                     BusIBusImpl        *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_FACTORY_PROXY (factory));

    IBusComponent *component;
    GList *engines, *p;

    ibus->factory_list = g_list_remove (ibus->factory_list, factory);

    component = bus_factory_proxy_get_component (factory);

    if (component != NULL) {
        p = engines = ibus_component_get_engines (component);
        for (; p != NULL; p = p->next) {
            if (g_list_find (ibus->register_engine_list, p->data)) {
                ibus->register_engine_list = g_list_remove (ibus->register_engine_list, p->data);
                g_object_unref (p->data);
            }
        }
        g_list_free (engines);
    }

    g_object_unref (factory);

    bus_ibus_impl_update_engines_hotkey_profile (ibus);
}

static void
bus_ibus_impl_add_factory (BusIBusImpl     *ibus,
                           BusFactoryProxy *factory)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_FACTORY_PROXY (factory));

    g_object_ref_sink (factory);
    ibus->factory_list = g_list_append (ibus->factory_list, factory);

    g_signal_connect (factory, "destroy", G_CALLBACK (_factory_destroy_cb), ibus);
}


static void
_ibus_register_component (BusIBusImpl           *ibus,
                          GVariant              *parameters,
                          GDBusMethodInvocation *invocation)
{
    GVariant *variant = g_variant_get_child_value (parameters, 0);
    IBusComponent *component = (IBusComponent *)ibus_serializable_deserialize (variant);

    if (!IBUS_IS_COMPONENT (component)) {
        if (component) g_object_unref (component);
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "The first argument should be an IBusComponent.");
        return;
    }

    g_object_ref_sink (component);
    BusConnection *connection = bus_connection_lookup (g_dbus_method_invocation_get_connection (invocation));
    BusFactoryProxy *factory = bus_factory_proxy_new (component, connection);

    if (factory == NULL) {
        g_object_unref (component);
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "Create factory failed.");
        return;
    }

    bus_ibus_impl_add_factory (ibus, factory);

    GList *engines = ibus_component_get_engines (component);
    g_list_foreach (engines, (GFunc) g_object_ref, NULL);
    ibus->register_engine_list = g_list_concat (ibus->register_engine_list, engines);
    g_object_unref (component);

    bus_ibus_impl_update_engines_hotkey_profile (ibus);

    g_dbus_method_invocation_return_value (invocation, NULL);
}

static void
_ibus_list_engines (BusIBusImpl           *ibus,
                    GVariant              *parameters,
                    GDBusMethodInvocation *invocation)
{
    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("av"));

    GList *engines = bus_registry_get_engines (ibus->registry);
    GList *p;
    for (p = engines; p != NULL; p = p->next) {
        g_variant_builder_add (&builder, "v", ibus_serializable_serialize ((IBusSerializable *)p->data));
    }
    g_list_free (engines);
    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(av)", &builder));
}

static void
_ibus_list_active_engines (BusIBusImpl           *ibus,
                           GVariant              *parameters,
                           GDBusMethodInvocation *invocation)
{
    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("av"));

    GList *p;
    for (p = ibus->engine_list; p != NULL; p = p->next) {
        g_variant_builder_add (&builder, "v", ibus_serializable_serialize ((IBusSerializable *)p->data));
    }
    for (p = ibus->register_engine_list; p != NULL; p = p->next) {
        g_variant_builder_add (&builder, "v", ibus_serializable_serialize ((IBusSerializable *)p->data));
    }
    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(av)", &builder));
}


static void
_ibus_exit (BusIBusImpl     *ibus,
            GVariant              *parameters,
            GDBusMethodInvocation *invocation)
{
    gboolean restart = FALSE;
    g_variant_get (parameters, "(b)", &restart);

    g_dbus_method_invocation_return_value (invocation, NULL);

    bus_server_quit ();

    if (!restart) {
        exit (0);
    }
    else {
        extern gchar **g_argv;
        gchar *exe;
        gint fd;

        exe = g_strdup_printf ("/proc/%d/exe", getpid ());
        exe = g_file_read_link (exe, NULL);

        if (exe == NULL)
            exe = BINDIR"/ibus-daemon";

        /* close all fds except stdin, stdout, stderr */
        for (fd = 3; fd <= sysconf (_SC_OPEN_MAX); fd ++) {
            close (fd);
        }

        execv (exe, g_argv);

        /* If the server binary is replaced while the server is running,
         * "readlink /proc/[pid]/exe" might return a path with " (deleted)"
         * suffix. */
        const gchar suffix[] = " (deleted)";
        if (g_str_has_suffix (exe, suffix)) {
            exe [strlen (exe) - sizeof (suffix) + 1] = '\0';
            execv (exe, g_argv);
        }
        g_warning ("execv %s failed!", g_argv[0]);
        exit (-1);
    }

    /* should not reach here */
    g_assert_not_reached ();
}

static void
_ibus_ping (BusIBusImpl           *ibus,
            GVariant              *parameters,
            GDBusMethodInvocation *invocation)
{
    g_dbus_method_invocation_return_value (invocation, parameters);
}

static void
_ibus_get_use_sys_layout (BusIBusImpl           *ibus,
                          GVariant              *parameters,
                          GDBusMethodInvocation *invocation)
{
    g_dbus_method_invocation_return_value (invocation,
                    g_variant_new ("(b)", ibus->use_sys_layout));
}

static void
_ibus_get_use_global_engine (BusIBusImpl           *ibus,
                             GVariant              *parameters,
                             GDBusMethodInvocation *invocation)
{
    g_dbus_method_invocation_return_value (invocation,
                    g_variant_new ("(b)", ibus->use_global_engine));
}

static void
_ibus_get_global_engine (BusIBusImpl           *ibus,
                         GVariant              *parameters,
                         GDBusMethodInvocation *invocation)
{
    if (ibus->use_global_engine && ibus->global_engine) {
        IBusEngineDesc *desc = bus_engine_proxy_get_desc (ibus->global_engine);
        if (desc != NULL) {
            GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)desc);
            g_dbus_method_invocation_return_value (invocation,
                            g_variant_new ("(v)", variant));
            return;
        }
    }
    g_dbus_method_invocation_return_error (invocation,
                    G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                    "No global engine.");
}

static void
_ibus_set_global_engine (BusIBusImpl           *ibus,
                         GVariant              *parameters,
                         GDBusMethodInvocation *invocation)
{
    if (!ibus->use_global_engine) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "Global engine feature is disabled.");
        return;
    }

    const gchar *new_engine_name = NULL;
    g_variant_get (parameters, "(&s)", &new_engine_name);
    const gchar *old_engine_name = NULL;

    if (ibus->global_engine) {
        old_engine_name = bus_engine_proxy_get_desc (ibus->global_engine)->name;
    }

    if (g_strcmp0 (new_engine_name, old_engine_name) == 0) {
        /* If the user requested the same global engine, then we just enable the
         * original one. */
        if (ibus->focused_context) {
            bus_input_context_enable (ibus->focused_context);
        }
        else if (ibus->global_engine) {
            bus_engine_proxy_enable (ibus->global_engine);
        }
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    /* If there is a focused input context, then we just change the engine of
     * the focused context, which will then change the global engine
     * automatically. Otherwise, we need to change the global engine directly.
     */
    if (ibus->focused_context) {
        _context_request_engine_cb (ibus->focused_context, new_engine_name, ibus);
    }
    else {
        IBusEngineDesc *engine_desc = _find_engine_desc_by_name (ibus, new_engine_name);
        if (engine_desc != NULL) {
            BusEngineProxy *new_engine = bus_ibus_impl_create_engine (engine_desc);
            if (new_engine != NULL) {
                /* Enable the global engine by default, because the user
                 * selected it explicitly. */
                bus_engine_proxy_enable (new_engine);

                /* Assume the ownership of the new global engine. Normally it's
                 * done by the input context. But as we need to change the global
                 * engine directly, so we need to do it here. */
                g_object_ref_sink (new_engine);
                bus_ibus_impl_set_global_engine (ibus, new_engine);

                /* The global engine should already be referenced. */
                g_object_unref (new_engine);
            }
        }
    }
    g_dbus_method_invocation_return_value (invocation, NULL);
}

static void
_ibus_is_global_engine_enabled (BusIBusImpl           *ibus,
                                GVariant              *parameters,
                                GDBusMethodInvocation *invocation)
{
    gboolean enabled = (ibus->use_global_engine && ibus->global_engine &&
                        bus_engine_proxy_is_enabled (ibus->global_engine));
    g_dbus_method_invocation_return_value (invocation,
                    g_variant_new ("(b)", enabled));
}

static void
bus_ibus_impl_service_method_call (IBusService           *service,
                                   GDBusConnection       *connection,
                                   const gchar           *sender,
                                   const gchar           *object_path,
                                   const gchar           *interface_name,
                                   const gchar           *method_name,
                                   GVariant              *parameters,
                                   GDBusMethodInvocation *invocation)
{
    if (g_strcmp0 (interface_name, "org.freedesktop.IBus") != 0) {
        IBUS_SERVICE_CLASS(bus_ibus_impl_parent_class)->service_method_call (
                        service, connection, sender, object_path, interface_name, method_name,
                        parameters, invocation);
        return;
    }

    static const struct {
        const gchar *method_name;
        void (* method_callback) (BusIBusImpl *, GVariant *, GDBusMethodInvocation *);
    } methods [] =  {
        /* IBus interface */
        { "GetAddress",            _ibus_get_address },
        { "CreateInputContext",    _ibus_create_input_context },
        { "CurrentInputContext",   _ibus_current_input_context },
        { "RegisterComponent",     _ibus_register_component },
        { "ListEngines",           _ibus_list_engines },
        { "ListActiveEngines",     _ibus_list_active_engines },
        { "Exit",                  _ibus_exit },
        { "Ping",                  _ibus_ping },
        { "GetUseSysLayout",       _ibus_get_use_sys_layout },
        { "GetUseGlobalEngine",    _ibus_get_use_global_engine },
        { "GetGlobalEngine",       _ibus_get_global_engine },
        { "SetGlobalEngine",       _ibus_set_global_engine },
        { "IsGlobalEngineEnabled", _ibus_is_global_engine_enabled },
    };

    gint i;
    for (i = 0; i < G_N_ELEMENTS (methods); i++) {
        if (g_strcmp0 (methods[i].method_name, method_name) == 0) {
            methods[i].method_callback ((BusIBusImpl *)service, parameters, invocation);
            return;
        }
    }
    g_return_if_reached ();
}

BusIBusImpl *
bus_ibus_impl_get_default (void)
{
    static BusIBusImpl *ibus = NULL;

    if (ibus == NULL) {
        ibus = (BusIBusImpl *) g_object_new (BUS_TYPE_IBUS_IMPL,
                                             "object-path", IBUS_PATH_IBUS,
                                             NULL);
    }
    return ibus;
}

BusFactoryProxy *
bus_ibus_impl_lookup_factory (BusIBusImpl *ibus,
                              const gchar *path)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    BusFactoryProxy *factory;

    factory = (BusFactoryProxy *) g_hash_table_lookup (ibus->factory_dict, path);

    return factory;
}

IBusHotkeyProfile *
bus_ibus_impl_get_hotkey_profile (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    return ibus->hotkey_profile;
}

IBusKeymap *
bus_ibus_impl_get_keymap (BusIBusImpl *ibus)
{

    g_assert (BUS_IS_IBUS_IMPL (ibus));

    return ibus->keymap;
}

BusRegistry *
bus_ibus_impl_get_registry (BusIBusImpl *ibus)
{

    g_assert (BUS_IS_IBUS_IMPL (ibus));

    return ibus->registry;
}

static void
bus_ibus_impl_emit_signal (BusIBusImpl *ibus,
                           const gchar *signal_name,
                           GVariant     *parameters)
{

    GDBusMessage *message = g_dbus_message_new_signal ("/org/freedesktop/IBus",
                                                       "org.freedesktop.DBus",
                                                       signal_name);
    g_dbus_message_set_sender (message, "org.freedesktop.DBus");
    if (parameters)
        g_dbus_message_set_body (message, parameters);
    bus_dbus_impl_dispatch_message_by_rule (BUS_DEFAULT_DBUS, message, NULL);
    g_object_unref (message);
}

static void
bus_ibus_impl_registry_changed (BusIBusImpl *ibus)
{
    bus_ibus_impl_emit_signal (ibus, "RegistryChanged", NULL);
}

static void
bus_ibus_impl_global_engine_changed (BusIBusImpl *ibus)
{
    bus_ibus_impl_emit_signal (ibus, "GlobalEngineChanged", NULL);
}

gboolean
bus_ibus_impl_filter_keyboard_shortcuts (BusIBusImpl     *ibus,
                                         BusInputContext *context,
                                         guint           keyval,
                                         guint           modifiers,
                                         guint           prev_keyval,
                                         guint           prev_modifiers)
{
    static GQuark trigger = 0;
    static GQuark next = 0;
    static GQuark previous = 0;

    GQuark event;
    GList *engine_list;

    if (trigger == 0) {
        trigger = g_quark_from_static_string ("trigger");
        next = g_quark_from_static_string ("next-engine-in-menu");
        previous = g_quark_from_static_string ("previous-engine");
    }

    /* Try global hotkeys first. */
    event = ibus_hotkey_profile_filter_key_event (ibus->hotkey_profile,
                                                  keyval,
                                                  modifiers,
                                                  prev_keyval,
                                                  prev_modifiers,
                                                  0);

    if (event == trigger) {
        gboolean enabled = bus_input_context_is_enabled (context);
        if (enabled) {
            bus_input_context_disable (context);
        }
        else {
            bus_input_context_enable (context);
        }
        return (enabled != bus_input_context_is_enabled (context));
    }
    if (event == next) {
        if (bus_input_context_is_enabled(context)) {
            bus_ibus_impl_context_request_next_engine_in_menu (ibus, context);
        }
        else {
            bus_input_context_enable (context);
        }
        return TRUE;
    }
    if (event == previous) {
        if (bus_input_context_is_enabled(context)) {
            bus_ibus_impl_context_request_previous_engine (ibus, context);
        }
        else {
            bus_input_context_enable (context);
        }
        return TRUE;
    }

    if (!ibus->engines_hotkey_profile || !ibus->hotkey_to_engines_map) {
        return FALSE;
    }

    /* Then try engines hotkeys. */
    event = ibus_hotkey_profile_filter_key_event (ibus->engines_hotkey_profile,
                                                  keyval,
                                                  modifiers,
                                                  prev_keyval,
                                                  prev_modifiers,
                                                  0);
    if (event == 0) {
        return FALSE;
    }

    engine_list = g_hash_table_lookup (ibus->hotkey_to_engines_map,
                                       GUINT_TO_POINTER (event));
    if (engine_list) {
        BusEngineProxy *current_engine = bus_input_context_get_engine (context);
        IBusEngineDesc *current_engine_desc =
            (current_engine ? bus_engine_proxy_get_desc (current_engine) : NULL);
        IBusEngineDesc *new_engine_desc = (IBusEngineDesc *) engine_list->data;

        g_assert (new_engine_desc);

        /* Find out what engine we should switch to. If the current engine has
         * the same hotkey, then we should switch to the next engine with the
         * same hotkey in the list. Otherwise, we just switch to the first
         * engine in the list. */
        GList *p = engine_list;
        for (; p->next != NULL; p = p->next) {
            if (current_engine_desc == (IBusEngineDesc *) p->data) {
                new_engine_desc = (IBusEngineDesc *) p->next->data;
                break;
            }
        }

        if (current_engine_desc != new_engine_desc) {
            bus_ibus_impl_set_context_engine_from_desc (ibus, context, new_engine_desc);
        }

        return TRUE;
    }

    return FALSE;
}

static gchar*
bus_ibus_impl_load_global_engine_name_from_config (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_return_val_if_fail (IBUS_IS_CONFIG (ibus->config), NULL);

    GVariant *variant = ibus_config_get_value (ibus->config, "general", "global_engine");
    gchar *engine_name = NULL;
    if (variant != NULL) {
        g_variant_get (variant, "s", &engine_name);
        g_variant_unref (variant);
    }
    return engine_name;
}

static void
bus_ibus_impl_save_global_engine_name_to_config (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_return_if_fail (IBUS_IS_CONFIG (ibus->config));

    if (ibus->use_global_engine && ibus->global_engine) {
        ibus_config_set_value (ibus->config,
                        "general", "global_engine",
                        g_variant_new ("s", bus_engine_proxy_get_desc (ibus->global_engine)->name));
    }
}

static gchar*
bus_ibus_impl_load_global_previous_engine_name_from_config (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_return_val_if_fail (IBUS_IS_CONFIG (ibus->config), NULL);

    GVariant *value = ibus_config_get_value (ibus->config, "general", "global_previous_engine");
    if (value == NULL)
        return NULL;
    gchar *engine_name = NULL;
    g_variant_get (value, "(s)", &engine_name);
    g_variant_unref (value);
    return engine_name;
}

static void
bus_ibus_impl_save_global_previous_engine_name_to_config (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_return_if_fail (IBUS_IS_CONFIG (ibus->config));

    if (ibus->use_global_engine && ibus->global_previous_engine_name) {
        ibus_config_set_value (ibus->config,
                        "general", "global_previous_engine",
                        g_variant_new ("s", ibus->global_previous_engine_name));
    }
}

static void
_add_engine_hotkey (IBusEngineDesc *engine, BusIBusImpl *ibus)
{
    gchar **hotkey_list;
    gchar **p;
    gchar *hotkey;
    GList *engine_list;

    GQuark event;
    guint keyval;
    guint modifiers;

    if (!engine || !engine->hotkeys || !*engine->hotkeys) {
        return;
    }

    hotkey_list = g_strsplit_set (engine->hotkeys, ";,", 0);

    for (p = hotkey_list; p && *p; ++p) {
        hotkey = g_strstrip (*p);
        if (!*hotkey || !ibus_key_event_from_string (hotkey, &keyval, &modifiers)) {
            continue;
        }

        /* If the hotkey already exists, we won't need to add it again. */
        event = ibus_hotkey_profile_lookup_hotkey (ibus->engines_hotkey_profile,
                                                   keyval, modifiers);
        if (event == 0) {
            event = g_quark_from_string (hotkey);
            ibus_hotkey_profile_add_hotkey (ibus->engines_hotkey_profile,
                                            keyval, modifiers, event);
        }

        engine_list = g_hash_table_lookup (ibus->hotkey_to_engines_map,
                                           GUINT_TO_POINTER (event));

        /* As we will rebuild the engines hotkey map whenever an engine was
         * added or removed, we don't need to hold a reference of the engine
         * here. */
        engine_list = g_list_append (engine_list, engine);

        /* We need to steal the value before adding it back, otherwise it will
         * be destroyed. */
        g_hash_table_steal (ibus->hotkey_to_engines_map, GUINT_TO_POINTER (event));

        g_hash_table_insert (ibus->hotkey_to_engines_map,
                             GUINT_TO_POINTER (event), engine_list);
    }

    g_strfreev (hotkey_list);
}

static void
bus_ibus_impl_update_engines_hotkey_profile (BusIBusImpl *ibus)
{
    if (ibus->engines_hotkey_profile) {
        g_object_unref (ibus->engines_hotkey_profile);
    }

    if (ibus->hotkey_to_engines_map) {
        g_hash_table_unref (ibus->hotkey_to_engines_map);
    }

    ibus->engines_hotkey_profile = ibus_hotkey_profile_new();
    ibus->hotkey_to_engines_map =
        g_hash_table_new_full (NULL, NULL, NULL, (GDestroyNotify) g_list_free);

    g_list_foreach (ibus->register_engine_list, (GFunc) _add_engine_hotkey, ibus);
    g_list_foreach (ibus->engine_list, (GFunc) _add_engine_hotkey, ibus);
}

gboolean
bus_ibus_impl_is_use_sys_layout (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL(ibus));

    return ibus->use_sys_layout;
}

BusInputContext *
bus_ibus_impl_get_focused_input_context (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL(ibus));

    return ibus->focused_context;
}
