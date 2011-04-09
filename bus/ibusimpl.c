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

    /* registered components */
    GList *registered_components;
    GList *contexts;

    /* a fake input context for global engine support */
    BusInputContext *fake_context;

    /* a list of engines that are preloaded. */
    GList *engine_list;
    /* a list of engines that are started by a user (without the --ibus command line flag.) */
    GList *register_engine_list;

    /* if TRUE, ibus-daemon uses a keysym translated by the system (i.e. XKB) as-is.
     * otherwise, ibus-daemon itself converts keycode into keysym. */
    gboolean use_sys_layout;

    gboolean embed_preedit_text;
    gboolean enable_by_default;

    BusRegistry     *registry;

    BusInputContext *focused_context;
    BusPanelProxy   *panel;
    IBusConfig      *config;

    /* global hotkeys such as "trigger" and "next_engine_in_menu" */
    IBusHotkeyProfile *hotkey_profile;
    /* a default keymap of ibus-daemon (usually "us") which is used only when use_sys_layout is FALSE. */
    IBusKeymap      *keymap;

    gboolean use_global_engine;
    gchar *global_engine_name;
    gchar *global_previous_engine_name;

    /* engine-specific hotkeys */
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
/* TODO use property to replace some getter and setter in future */
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
static void     bus_ibus_impl_set_global_engine_by_name
                                                (BusIBusImpl        *ibus,
                                                 const gchar        *name);
static void     bus_ibus_impl_check_global_engine
                                                (BusIBusImpl        *ibus);
static void     bus_ibus_impl_registry_changed  (BusIBusImpl        *ibus);
static void     bus_ibus_impl_global_engine_changed
                                                (BusIBusImpl        *ibus);
static void     bus_ibus_impl_set_context_engine_from_desc
                                                (BusIBusImpl        *ibus,
                                                 BusInputContext    *context,
                                                 IBusEngineDesc     *desc);
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
static BusInputContext
               *bus_ibus_impl_create_input_context
                                                (BusIBusImpl        *ibus,
                                                 BusConnection      *connection,
                                                 const gchar        *client);
static IBusEngineDesc
               *bus_ibus_impl_get_engine_desc   (BusIBusImpl        *ibus,
                                                 const gchar        *engine_name);
static void     bus_ibus_impl_set_focused_context
                                                (BusIBusImpl        *ibus,
                                                 BusInputContext    *context);
/* some callback functions */
static void     _context_engine_changed_cb      (BusInputContext    *context,
                                                 BusIBusImpl        *ibus);

/* The interfaces available in this class, which consists of a list of methods this class implements and
 * a list of signals this class may emit. Method calls to the interface that are not defined in this XML
 * will be automatically rejected by the GDBus library (see src/ibusservice.c for details.) */
static const gchar introspection_xml[] =
    "<node>\n"
    "  <interface name='org.freedesktop.IBus'>\n"
    "    <method name='GetAddress'>\n"
    "      <arg direction='out' type='s' name='address' />\n"
    "    </method>\n"
    "    <method name='CreateInputContext'>\n"
    "      <arg direction='in'  type='s' name='client_name' />\n"
    "      <arg direction='out' type='o' name='object_path' />\n"
    "    </method>\n"
    "    <method name='CurrentInputContext'>\n"
    "      <arg direction='out' type='o' name='object_path' />\n"
    "    </method>\n"
    "    <method name='RegisterComponent'>\n"
    "      <arg direction='in'  type='v' name='component' />\n"
    "    </method>\n"
    "    <method name='ListEngines'>\n"
    "      <arg direction='out' type='av' name='engines' />\n"
    "    </method>\n"
    "    <method name='ListActiveEngines'>\n"
    "      <arg direction='out' type='av' name='engines' />\n"
    "    </method>\n"
    "    <method name='Exit'>\n"
    "      <arg direction='in'  type='b' name='restart' />\n"
    "    </method>\n"
    "    <method name='Ping'>\n"
    "      <arg direction='in'  type='v' name='data' />\n"
    "      <arg direction='out' type='v' name='data' />\n"
    "    </method>\n"
    "    <method name='GetUseSysLayout'>\n"
    "      <arg direction='out' type='b' name='enabled' />\n"
    "    </method>\n"
    "    <method name='GetUseGlobalEngine'>\n"
    "      <arg direction='out' type='b' name='enabled' />\n"
    "    </method>\n"
    "    <method name='GetGlobalEngine'>\n"
    "      <arg direction='out' type='v' name='desc' />\n"
    "    </method>\n"
    "    <method name='SetGlobalEngine'>\n"
    "      <arg direction='in'  type='s' name='engine_name' />\n"
    "    </method>\n"
    "    <method name='IsGlobalEngineEnabled'>\n"
    "      <arg direction='out' type='b' name='enabled' />\n"
    "    </method>\n"
    "    <signal name='RegistryChanged'>\n"
    "    </signal>\n"
    "    <signal name='GlobalEngineChanged'>\n"
    "      <arg type='s' name='engine_name' />\n"
    "    </signal>\n"
    "  </interface>\n"
    "</node>\n";


G_DEFINE_TYPE (BusIBusImpl, bus_ibus_impl, IBUS_TYPE_SERVICE)

static void
bus_ibus_impl_class_init (BusIBusImplClass *class)
{
    IBUS_OBJECT_CLASS (class)->destroy = (IBusObjectDestroyFunc) bus_ibus_impl_destroy;

    /* override the parent class's implementation. */
    IBUS_SERVICE_CLASS (class)->service_method_call = bus_ibus_impl_service_method_call;
    /* register the xml so that bus_ibus_impl_service_method_call will be called on a method call defined in the xml (e.g. 'GetAddress'.) */
    ibus_service_class_add_interfaces (IBUS_SERVICE_CLASS (class), introspection_xml);
}

/**
 * _panel_destroy_cb:
 *
 * A callback function which is called when (1) the connection to the panel process is terminated,
 * or (2) ibus_proxy_destroy (ibus->panel); is called. See src/ibusproxy.c for details.
 */
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
                          GVariant    *value)
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

/**
 * bus_ibus_impl_set_trigger:
 *
 * A function to be called when "trigger" config is updated. If the value is NULL, the default trigger (Ctrl+space) is set.
 */
static void
bus_ibus_impl_set_trigger (BusIBusImpl *ibus,
                           GVariant    *value)
{
    GQuark hotkey = g_quark_from_static_string ("trigger");
    if (value != NULL) {
        bus_ibus_impl_set_hotkey (ibus, hotkey, value);
    }
#ifndef OS_CHROMEOS
    else {
        /* set default trigger */
        ibus_hotkey_profile_remove_hotkey_by_event (ibus->hotkey_profile, hotkey);
        ibus_hotkey_profile_add_hotkey (ibus->hotkey_profile,
                                        IBUS_space,
                                        IBUS_CONTROL_MASK,
                                        hotkey);
    }
#endif
}

/**
 * bus_ibus_impl_set_enable_unconditional:
 *
 * A function to be called when "enable_unconditional" config is updated.
 */
static void
bus_ibus_impl_set_enable_unconditional (BusIBusImpl *ibus,
                                        GVariant    *value)
{
    GQuark hotkey = g_quark_from_static_string ("enable-unconditional");
    bus_ibus_impl_set_hotkey (ibus, hotkey, value);
}

/**
 * bus_ibus_impl_set_disable_unconditional:
 *
 * A function to be called when "disable_unconditional" config is updated.
 */
static void
bus_ibus_impl_set_disable_unconditional (BusIBusImpl *ibus,
                                         GVariant    *value)
{
    GQuark hotkey = g_quark_from_static_string ("disable-unconditional");
    bus_ibus_impl_set_hotkey (ibus, hotkey, value);
}

/**
 * bus_ibus_impl_set_next_engine_in_menu:
 *
 * A function to be called when "next_engine_in_menu" config is updated.
 */
static void
bus_ibus_impl_set_next_engine_in_menu (BusIBusImpl *ibus,
                                       GVariant    *value)
{
    GQuark hotkey = g_quark_from_static_string ("next-engine-in-menu");
    bus_ibus_impl_set_hotkey (ibus, hotkey, value);
}

/**
 * bus_ibus_impl_set_previous_engine:
 *
 * A function to be called when "previous_engine" config is updated.
 */
static void
bus_ibus_impl_set_previous_engine (BusIBusImpl *ibus,
                                   GVariant    *value)
{
    GQuark hotkey = g_quark_from_static_string ("previous-engine");
    bus_ibus_impl_set_hotkey (ibus, hotkey, value);
}

/**
 * bus_ibus_impl_set_preload_engines:
 *
 * A function to be called when "preload_engines" config is updated.
 */
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
        BusComponent *component = bus_component_from_engine_desc ((IBusEngineDesc *) ibus->engine_list->data);
        if (component && !bus_component_is_running (component)) {
            bus_component_start (component, g_verbose);
        }
    }

    bus_ibus_impl_check_global_engine (ibus);
    bus_ibus_impl_update_engines_hotkey_profile (ibus);
}

/**
 * bus_ibus_impl_set_use_sys_layout:
 *
 * A function to be called when "use_system_keyboard_layout" config is updated.
 */
static void
bus_ibus_impl_set_use_sys_layout (BusIBusImpl *ibus,
                                  GVariant    *value)
{
    if (value != NULL && g_variant_classify (value) == G_VARIANT_CLASS_BOOLEAN) {
        ibus->use_sys_layout = g_variant_get_boolean (value);
    }
}

/**
 * bus_ibus_impl_set_embed_preedit_text:
 *
 * A function to be called when "use_embed_preedit_text" config is updated.
 */
static void
bus_ibus_impl_set_embed_preedit_text (BusIBusImpl *ibus,
                                      GVariant    *value)
{
    if (value != NULL && g_variant_classify (value) == G_VARIANT_CLASS_BOOLEAN) {
        ibus->embed_preedit_text = g_variant_get_boolean (value);
    }
}

/**
 * bus_ibus_impl_set_enable_by_default:
 *
 * A function to be called when "enable_by_default" config is updated.
 */
static void
bus_ibus_impl_set_enable_by_default (BusIBusImpl *ibus,
                                     GVariant    *value)
{
    if (value != NULL && g_variant_classify (value) == G_VARIANT_CLASS_BOOLEAN) {
        ibus->enable_by_default = g_variant_get_boolean (value);
    }
}

/**
 * bus_ibus_impl_set_use_global_engine:
 *
 * A function to be called when "use_global_engine" config is updated.
 */
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
        if (ibus->panel && ibus->focused_context == NULL) {
            bus_panel_proxy_focus_in (ibus->panel, ibus->fake_context);
        }
    }
    else {
        /* turn off use_global_engine option */
        ibus->use_global_engine = FALSE;

        /* if fake context has the focus, we should focus out it */
        if (ibus->panel && ibus->focused_context == NULL) {
            bus_panel_proxy_focus_out (ibus->panel, ibus->fake_context);
        }
        /* remove engine in fake context */
        bus_input_context_set_engine (ibus->fake_context, NULL);
    }
}

#ifndef OS_CHROMEOS
static gint
_engine_desc_cmp (IBusEngineDesc *desc1,
                  IBusEngineDesc *desc2)
{
    return - ((gint) ibus_engine_desc_get_rank (desc1)) +
              ((gint) ibus_engine_desc_get_rank (desc2));
}
#endif

/**
 * bus_ibus_impl_set_default_preload_engines:
 *
 * If the "preload_engines" config variable is not set yet, set the default value which is determined based on a current locale.
 */
static void
bus_ibus_impl_set_default_preload_engines (BusIBusImpl *ibus)
{
#ifndef OS_CHROMEOS
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

    /* The setlocale call first checks LC_ALL. If it's not available, checks
     * LC_CTYPE. If it's also not available, checks LANG. */
    gchar *lang = g_strdup (setlocale (LC_CTYPE, NULL));
    if (lang == NULL) {
        return;
    }

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
        IBusEngineDesc *desc = (IBusEngineDesc *) list->data;
        /* ignore engines with rank <= 0 */
        if (ibus_engine_desc_get_rank (desc) > 0)
            g_variant_builder_add (&builder, "s", ibus_engine_desc_get_name (desc));
    }

    GVariant *value = g_variant_builder_end (&builder);
    if (value != NULL) {
        if (g_variant_n_children (value) > 0) {
            ibus_config_set_value (ibus->config,
                                   "general", "preload_engines", value);
        } else {
            /* We don't update preload_engines with an empty string for safety.
             * Just unref the floating value. */
            g_variant_unref (value);
        }
    }
    g_list_free (engines);
#endif
}

/* The list of config entries that are related to ibus-daemon. */
const static struct {
    gchar *section;
    gchar *key;
    void (*func) (BusIBusImpl *, GVariant *);
} bus_ibus_impl_config_items [] = {
    { "general/hotkey", "trigger",               bus_ibus_impl_set_trigger },
    { "general/hotkey", "enable_unconditional",  bus_ibus_impl_set_enable_unconditional },
    { "general/hotkey", "disable_unconditional", bus_ibus_impl_set_disable_unconditional },
    { "general/hotkey", "next_engine_in_menu",   bus_ibus_impl_set_next_engine_in_menu },
    { "general/hotkey", "previous_engine",       bus_ibus_impl_set_previous_engine },
    { "general", "preload_engines",              bus_ibus_impl_set_preload_engines },
    { "general", "use_system_keyboard_layout",   bus_ibus_impl_set_use_sys_layout },
    { "general", "use_global_engine",            bus_ibus_impl_set_use_global_engine },
    { "general", "embed_preedit_text",           bus_ibus_impl_set_embed_preedit_text },
    { "general", "enable_by_default",            bus_ibus_impl_set_enable_by_default },
};

/**
 * bus_ibus_impl_reload_config
 *
 * Read config entries (e.g. preload_engines) from the config daemon.
 */
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
        bus_ibus_impl_config_items[i].func (ibus, variant); /* variant could be NULL if the deamon is not ready yet. */
        if (variant) g_variant_unref (variant);
    }
}

/**
 * _config_value_changed_cb:
 *
 * A callback function to be called when the "ValueChanged" D-Bus signal is sent from the config daemon.
 */
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

/**
 * _config_destroy_cb:
 *
 * A callback function which is called when (1) the connection to the config process is terminated,
 * or (2) ibus_proxy_destroy (ibus->config); is called. See src/ibusproxy.c for details.
 */
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

/*
 * _dbus_name_owner_changed_cb:
 *
 * A callback function to be called when the name-owner-changed signal is sent to the dbus object.
 * This usually means a client (e.g. a panel/config/engine process or an application) is connected/disconnected to/from the bus.
 */
static void
_dbus_name_owner_changed_cb (BusDBusImpl   *dbus,
                             BusConnection *orig_connection,
                             const gchar   *name,
                             const gchar   *old_name,
                             const gchar   *new_name,
                             BusIBusImpl   *ibus)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (name != NULL);
    g_assert (old_name != NULL);
    g_assert (new_name != NULL);
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    if (g_strcmp0 (name, IBUS_SERVICE_PANEL) == 0) {
        if (g_strcmp0 (new_name, "") != 0) {
            /* a Panel process is started. */
            BusConnection *connection;

            if (ibus->panel != NULL) {
                ibus_proxy_destroy ((IBusProxy *) ibus->panel);
                /* panel should be NULL after destroy. See _panel_destroy_cb for details. */
                g_assert (ibus->panel == NULL);
            }

            connection = bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS, new_name);
            g_return_if_fail (connection != NULL);

            ibus->panel = bus_panel_proxy_new (connection);

            g_signal_connect (ibus->panel,
                              "destroy",
                              G_CALLBACK (_panel_destroy_cb),
                              ibus);

            if (ibus->focused_context != NULL) {
                bus_panel_proxy_focus_in (ibus->panel, ibus->focused_context);
            }
            else if (ibus->use_global_engine) {
                bus_panel_proxy_focus_in (ibus->panel, ibus->fake_context);
            }
        }
    }
    else if (g_strcmp0 (name, IBUS_SERVICE_CONFIG) == 0) {
        if (g_strcmp0 (new_name, "") != 0) {
            /* a config process is started. */
            BusConnection *connection;

            if (ibus->config != NULL) {
                ibus_proxy_destroy ((IBusProxy *) ibus->config);
                /* config should be NULL after destroy. See _config_destroy_cb for details. */
                g_assert (ibus->config == NULL);
            }

            /* get a connection between ibus-daemon and the config daemon. */
            connection = bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS, new_name);
            g_return_if_fail (connection != NULL);

            ibus->config = g_initable_new (IBUS_TYPE_CONFIG,
                                           NULL,
                                           NULL,
                                           /* The following properties are necessary to initialize GDBusProxy object
                                            * which is a parent of the config object. */
                                           "g-connection",      bus_connection_get_dbus_connection (connection),
                                           "g-flags",           G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START | G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                           "g-interface-name",  IBUS_INTERFACE_CONFIG,
                                           "g-object-path",     IBUS_PATH_CONFIG,
                                           "g-default-timeout", g_gdbus_timeout,
                                           NULL);

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

    bus_registry_name_owner_changed (ibus->registry, name, old_name, new_name);
}

/**
 * bus_ibus_impl_init:
 *
 * The constructor of BusIBusImpl. Initialize all member variables of a BusIBusImpl object.
 */
static void
bus_ibus_impl_init (BusIBusImpl *ibus)
{
    ibus->factory_dict = g_hash_table_new_full (
                            g_str_hash,
                            g_str_equal,
                            NULL,
                            (GDestroyNotify) g_object_unref);

    ibus->fake_context = bus_input_context_new (NULL, "fake");
    g_object_ref_sink (ibus->fake_context);
    bus_dbus_impl_register_object (BUS_DEFAULT_DBUS,
                                   (IBusService *) ibus->fake_context);
    bus_input_context_set_capabilities (ibus->fake_context,
                                        IBUS_CAP_PREEDIT_TEXT |
                                        IBUS_CAP_FOCUS |
                                        IBUS_CAP_SURROUNDING_TEXT);
    g_signal_connect (ibus->fake_context,
                      "engine-changed",
                      G_CALLBACK (_context_engine_changed_cb),
                      ibus);
    bus_input_context_focus_in (ibus->fake_context);

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
    ibus->global_engine_name = NULL;
    ibus->global_previous_engine_name = NULL;

    ibus->engines_hotkey_profile = NULL;
    ibus->hotkey_to_engines_map = NULL;

    /* focus the fake_context, if use_global_engine is enabled. */
    if (ibus->use_global_engine)
        bus_ibus_impl_set_focused_context (ibus, ibus->fake_context);

    g_signal_connect (BUS_DEFAULT_DBUS,
                      "name-owner-changed",
                      G_CALLBACK (_dbus_name_owner_changed_cb),
                      ibus);
}

/**
 * bus_ibus_impl_destroy:
 *
 * The destructor of BusIBusImpl.
 */
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
                    /* send TERM signal to the whole process group (i.e. engines, panel, and config daemon.) */
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
    }

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

    g_free (ibus->global_engine_name);
    ibus->global_engine_name = NULL;

    g_free (ibus->global_previous_engine_name);
    ibus->global_previous_engine_name = NULL;

    if (ibus->engines_hotkey_profile != NULL) {
        g_object_unref (ibus->engines_hotkey_profile);
        ibus->engines_hotkey_profile = NULL;
    }

    if (ibus->hotkey_to_engines_map) {
        g_hash_table_unref (ibus->hotkey_to_engines_map);
        ibus->hotkey_to_engines_map = NULL;
    }

    if (ibus->fake_context) {
        g_object_unref (ibus->fake_context);
        ibus->fake_context = NULL;
    }

    bus_server_quit ();
    IBUS_OBJECT_CLASS (bus_ibus_impl_parent_class)->destroy (IBUS_OBJECT (ibus));
}

/**
 * _ibus_get_address:
 *
 * Implement the "GetAddress" method call of the org.freedesktop.IBus interface.
 */
static void
_ibus_get_address (BusIBusImpl           *ibus,
                   GVariant              *parameters,
                   GDBusMethodInvocation *invocation)
{
    g_dbus_method_invocation_return_value (invocation,
                                           g_variant_new ("(s)", bus_server_get_address ()));
}

static IBusEngineDesc *
_find_engine_desc_by_name (BusIBusImpl *ibus,
                           const gchar *engine_name)
{
    IBusEngineDesc *desc = NULL;
    GList *p;

    /* find engine in registered engine list */
    for (p = ibus->register_engine_list; p != NULL; p = p->next) {
        desc = (IBusEngineDesc *) p->data;
        if (g_strcmp0 (ibus_engine_desc_get_name (desc), engine_name) == 0)
            return desc;
    }

    /* find engine in preload engine list */
    for (p = ibus->engine_list; p != NULL; p = p->next) {
        desc = (IBusEngineDesc *) p->data;
        if (g_strcmp0 (ibus_engine_desc_get_name (desc), engine_name) == 0)
            return desc;
    }

    return NULL;
}

/**
 * _context_request_engine_cb:
 *
 * A callback function to be called when the "request-engine" signal is sent to the context.
 */
static IBusEngineDesc *
_context_request_engine_cb (BusInputContext *context,
                            const gchar     *engine_name,
                            BusIBusImpl     *ibus)
{
    return bus_ibus_impl_get_engine_desc (ibus, engine_name);
}

/**
 * bus_ibus_impl_get_engine_desc:
 *
 * Get the IBusEngineDesc by engine_name. If the engine_name is NULL, return
 * a default engine desc.
 */
static IBusEngineDesc *
bus_ibus_impl_get_engine_desc (BusIBusImpl *ibus,
                               const gchar *engine_name)
{
    IBusEngineDesc *desc = NULL;

    if (engine_name != NULL && engine_name[0] != '\0') {
        /* request engine by name */
        desc = _find_engine_desc_by_name (ibus, engine_name);
        if (desc == NULL) {
            g_warning ("_context_request_engine_cb: Invalid engine '%s' is requested.", engine_name);
            return NULL;
        }
    }
    else {
        /* Use global engine if possible. */
        if (ibus->use_global_engine) {
            gchar *name = g_strdup (ibus->global_engine_name);
            if (name == NULL) {
                name = bus_ibus_impl_load_global_engine_name_from_config (ibus);
            }
            if (name) {
                desc = _find_engine_desc_by_name (ibus, name);
                g_free (name);
            }
        }
        /* request default engine */
        if (!desc) {
            if (ibus->register_engine_list) {
                desc = (IBusEngineDesc *) ibus->register_engine_list->data;
            }
            else if (ibus->engine_list) {
                desc = (IBusEngineDesc *) ibus->engine_list->data;
            }
        }
        if (!desc) {
            /* no engine is available. the user hasn't ran ibus-setup yet and
             * the bus_ibus_impl_set_default_preload_engines() function could
             * not find any default engines. another possiblity is that the
             * user hasn't installed an engine yet? just give up. */
            g_warning ("No engine is available. Run ibus-setup first.");
            return NULL;
        }
    }

    return desc;
}

/**
 * bus_ibus_impl_context_request_next_engine_in_menu:
 *
 * Process the "next_engine_in_menu" hotkey.
 */
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
        desc = bus_ibus_impl_get_engine_desc (ibus, NULL);
        if (desc != NULL)
            bus_ibus_impl_set_context_engine_from_desc (ibus,
                                                        context,
                                                        desc);
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
            next_desc = (IBusEngineDesc *) ibus->register_engine_list->data;
        }
        else if (ibus->engine_list) {
            next_desc = (IBusEngineDesc *) ibus->engine_list->data;
        }
    }

    bus_ibus_impl_set_context_engine_from_desc (ibus, context, next_desc);
}

/**
 * bus_ibus_impl_context_request_previous_engine:
 *
 * Process the "previous_engine" hotkey.
 */
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
        if (engine_name != NULL) {
            /* If the previous engine is removed from the engine list or the
               current engine and the previous engine are the same one, force
               to pick a new one. */
            if (!_find_engine_desc_by_name (ibus, engine_name) ||
                g_strcmp0 (engine_name, ibus->global_engine_name) == 0) {
                g_free (engine_name);
                ibus->global_previous_engine_name = engine_name = NULL;
            }
        }
    }

    /*
     * If the previous engine name is not found, switch to the next engine
     * in the menu. This behavior is better than doing nothing.
     */
    if (!engine_name) {
        bus_ibus_impl_context_request_next_engine_in_menu (ibus, context);
        return;
    }

    IBusEngineDesc *desc = NULL;
    desc = bus_ibus_impl_get_engine_desc (ibus, engine_name);
    if (desc != NULL) {
        bus_ibus_impl_set_context_engine_from_desc (ibus,
                                                    context,
                                                    desc);
    }
}

static void
bus_ibus_impl_set_context_engine_from_desc (BusIBusImpl     *ibus,
                                            BusInputContext *context,
                                            IBusEngineDesc  *desc)
{
    bus_input_context_set_engine_by_desc (context,
                                          desc,
                                          g_gdbus_timeout, /* timeout in msec. */
                                          NULL, /* we do not cancel the call. */
                                          NULL, /* use the default callback function. */
                                          NULL);
}

/**
 * bus_ibus_impl_set_focused_context:
 *
 * Set the current focused context.
 */
static void
bus_ibus_impl_set_focused_context (BusIBusImpl     *ibus,
                                   BusInputContext *context)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (context == NULL || BUS_IS_INPUT_CONTEXT (context));
    g_assert (context == NULL || bus_input_context_get_capabilities (context) & IBUS_CAP_FOCUS);

    /* Do noting if it is not focused context. */
    if (ibus->focused_context == context) {
        return;
    }

    BusEngineProxy *engine = NULL;

    if (ibus->focused_context) {
        if (ibus->use_global_engine) {
            /* dettach engine from the focused context */
            engine = bus_input_context_get_engine (ibus->focused_context);
            if (engine) {
                g_object_ref (engine);
                bus_input_context_set_engine (ibus->focused_context, NULL);
            }
        }

        if (ibus->panel != NULL)
            bus_panel_proxy_focus_out (ibus->panel, ibus->focused_context);

        g_object_unref (ibus->focused_context);
        ibus->focused_context = NULL;
    }

    if (context == NULL && ibus->use_global_engine) {
        context = ibus->fake_context;
    }

    if (context) {
        ibus->focused_context = (BusInputContext *) g_object_ref (context);
        /* attach engine to the focused context */
        if (engine != NULL) {
            bus_input_context_set_engine (context, engine);
            if (bus_engine_proxy_is_enabled (engine))
                bus_input_context_enable (context);
            g_object_unref (engine);
        }

        if (ibus->panel != NULL)
            bus_panel_proxy_focus_in (ibus->panel, context);
    }
}

static void
bus_ibus_impl_set_global_engine (BusIBusImpl    *ibus,
                                 BusEngineProxy *engine)
{
    if (!ibus->use_global_engine)
        return;

    if (ibus->focused_context) {
        bus_input_context_set_engine (ibus->focused_context, engine);
    } else if (ibus->fake_context) {
        bus_input_context_set_engine (ibus->fake_context, engine);
    }
}

static void
bus_ibus_impl_set_global_engine_by_name (BusIBusImpl *ibus,
                                         const gchar *name)
{
    if (!ibus->use_global_engine)
        return;

    BusInputContext *context =
            ibus->focused_context != NULL ? ibus->focused_context : ibus->fake_context;

    if (context == NULL) {
        return;
    }

    if (g_strcmp0 (name, ibus->global_engine_name) == 0) {
        /* If the user requested the same global engine, then we just enable the
         * original one. */
        bus_input_context_enable (context);
        return;
    }

    /* If there is a focused input context, then we just change the engine of
     * the focused context, which will then change the global engine
     * automatically. Otherwise, we need to change the global engine directly.
     */
    IBusEngineDesc *desc = NULL;
    desc = bus_ibus_impl_get_engine_desc (ibus, name);
    if (desc != NULL) {
        bus_ibus_impl_set_context_engine_from_desc (ibus,
                                                    context,
                                                    desc);
    }
}

/* When preload_engines and register_engiens are changed, this function
 * will check the global engine. If necessay, it will change the global engine.
 */
static void
bus_ibus_impl_check_global_engine (BusIBusImpl *ibus)
{
    GList *engine_list = NULL;

    /* do nothing */
    if (!ibus->use_global_engine)
        return;

    /* The current global engine is not removed, so do nothing. */
    if (ibus->global_engine_name != NULL &&
        _find_engine_desc_by_name (ibus, ibus->global_engine_name)) {
        return;
    }

    /* If the previous engine is available, then just switch to it. */
    if (ibus->global_previous_engine_name != NULL &&
        _find_engine_desc_by_name (ibus, ibus->global_previous_engine_name)) {
        bus_ibus_impl_set_global_engine_by_name (
            ibus, ibus->global_previous_engine_name);
        return;
    }

    /* Just switch to the fist engine in the list. */
    engine_list = ibus->register_engine_list;
    if (!engine_list)
        engine_list = ibus->engine_list;

    if (engine_list) {
        IBusEngineDesc *engine_desc = (IBusEngineDesc *)engine_list->data;
        bus_ibus_impl_set_global_engine_by_name (ibus,
                        ibus_engine_desc_get_name (engine_desc));
        return;
    }

    /* No engine available? Just disable global engine. */
    bus_ibus_impl_set_global_engine (ibus, NULL);
}

/**
 * _context_engine_changed_cb:
 *
 * A callback function to be called when the "engine-changed" signal is sent to the context.
 * Update global engine as well if necessary.
 */
static void
_context_engine_changed_cb (BusInputContext *context,
                            BusIBusImpl     *ibus)
{
    if (!ibus->use_global_engine)
        return;

    if ((context == ibus->focused_context) ||
        (ibus->focused_context == NULL && context == ibus->fake_context)) {
        BusEngineProxy *engine = bus_input_context_get_engine (context);
        if (engine != NULL) {
            /* only set global engine if engine is not NULL */
            const gchar *name = ibus_engine_desc_get_name (bus_engine_proxy_get_desc (engine));
            if (g_strcmp0 (name, ibus->global_engine_name) == 0)
                return;
            g_free (ibus->global_previous_engine_name);
            ibus->global_previous_engine_name = ibus->global_engine_name;
            ibus->global_engine_name = g_strdup (name);
            /* save changes */
            bus_ibus_impl_save_global_engine_name_to_config (ibus);
            bus_ibus_impl_save_global_previous_engine_name_to_config (ibus);
            bus_ibus_impl_global_engine_changed (ibus);
        }
    }
}

/**
 * _context_focus_in_cb:
 *
 * A callback function to be called when the "focus-in" signal is sent to the context.
 * If necessary, enables the global engine on the context and update ibus->focused_context.
 */
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

    bus_ibus_impl_set_focused_context (ibus, context);
}

/**
 * _context_focus_out_cb:
 *
 * A callback function to be called when the "focus-out" signal is sent to the context.
 */
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


    if (ibus->use_global_engine == FALSE) {
        /* Do not change the focused context, if use_global_engine option is enabled.
         * If focused context swith to NULL, users can not swith engine in panel anymore.
         **/
        bus_ibus_impl_set_focused_context (ibus, NULL);
    }
}

/**
 * _context_destroy_cb:
 *
 * A callback function to be called when the "destroy" signal is sent to the context.
 */
static void
_context_destroy_cb (BusInputContext    *context,
                     BusIBusImpl        *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context == ibus->focused_context) {
        bus_ibus_impl_set_focused_context (ibus, NULL);
    }

    ibus->contexts = g_list_remove (ibus->contexts, context);
    g_object_unref (context);
}

/**
 * _context_enabled_cb:
 *
 * A callback function to be called when the "enabled" signal is sent to the context.
 */
static void
_context_enabled_cb (BusInputContext    *context,
                     BusIBusImpl        *ibus)
{
    /* FIXME implement this. */
}

/**
 * _context_disabled_cb:
 *
 * A callback function to be called when the "disabled" signal is sent to the context.
 */
static void
_context_disabled_cb (BusInputContext    *context,
                      BusIBusImpl        *ibus)
{
    /* FIXME implement this. */
}

/**
 * bus_ibus_impl_create_input_context:
 * @client: A name of a client. e.g. "gtk-im"
 * @returns: A BusInputContext object.
 *
 * Create a new BusInputContext object for the client.
 */
static BusInputContext *
bus_ibus_impl_create_input_context (BusIBusImpl   *ibus,
                                    BusConnection *connection,
                                    const gchar   *client)
{
    BusInputContext *context = bus_input_context_new (connection, client);
    g_object_ref_sink (context);
    ibus->contexts = g_list_append (ibus->contexts, context);

    /* Installs glib signal handlers so that the ibus object could be notified when e.g. an IBus.InputContext D-Bus method is called. */
    static const struct {
        gchar *name;
        GCallback callback;
    } signals [] = {
        { "request-engine", G_CALLBACK (_context_request_engine_cb) },
        { "engine-changed", G_CALLBACK (_context_engine_changed_cb) },
        { "focus-in",       G_CALLBACK (_context_focus_in_cb) },
        { "focus-out",      G_CALLBACK (_context_focus_out_cb) },
        { "destroy",        G_CALLBACK (_context_destroy_cb) },
        { "enabled",        G_CALLBACK (_context_enabled_cb) },
        { "disabled",       G_CALLBACK (_context_disabled_cb) },
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

    /* register the context object so that the object could handle IBus.InputContext method calls. */
    bus_dbus_impl_register_object (BUS_DEFAULT_DBUS,
                                   (IBusService *) context);
    g_object_ref (context);
    return context;
}

/**
 * _ibus_create_input_context:
 *
 * Implement the "CreateInputContext" method call of the org.freedesktop.IBus interface.
 */
static void
_ibus_create_input_context (BusIBusImpl           *ibus,
                            GVariant              *parameters,
                            GDBusMethodInvocation *invocation)
{
    const gchar *client_name = NULL;  // e.g. "gtk-im"
    g_variant_get (parameters, "(&s)", &client_name);

    BusConnection *connection =
            bus_connection_lookup (g_dbus_method_invocation_get_connection (invocation));
    BusInputContext *context =
            bus_ibus_impl_create_input_context (ibus,
                                                connection,
                                                client_name);
    if (context) {
        const gchar *path = ibus_service_get_object_path ((IBusService *) context);
        /* the format-string 'o' is for a D-Bus object path. */
        g_dbus_method_invocation_return_value (invocation, g_variant_new ("(o)", path));
        g_object_unref (context);
    }
    else {
        g_dbus_method_invocation_return_error (invocation,
                                               G_DBUS_ERROR,
                                               G_DBUS_ERROR_FAILED,
                                               "Create input context failed!");
    }
}

/**
 * _ibus_current_input_context:
 *
 * Implement the "CurrentInputContext" method call of the org.freedesktop.IBus interface.
 */
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
        const gchar *path = ibus_service_get_object_path ((IBusService *) ibus->focused_context);
        /* the format-string 'o' is for a D-Bus object path. */
        g_dbus_method_invocation_return_value (invocation, g_variant_new ("(o)", path));
    }
}

static void
_component_destroy_cb (BusComponent *component,
                       BusIBusImpl  *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_COMPONENT (component));

    ibus->registered_components = g_list_remove (ibus->registered_components, component);

    /* remove engines from engine_list */
    GList *engines = bus_component_get_engines (component);
    GList *p;
    for (p = engines; p != NULL; p = p->next) {
        if (g_list_find (ibus->register_engine_list, p->data)) {
            ibus->register_engine_list = g_list_remove (ibus->register_engine_list, p->data);
            g_object_unref (p->data);
        }
    }
    g_list_free (engines);

    g_object_unref (component);

    bus_ibus_impl_check_global_engine (ibus);
    bus_ibus_impl_update_engines_hotkey_profile (ibus);
}

/**
 * _ibus_register_component:
 *
 * Implement the "RegisterComponent" method call of the org.freedesktop.IBus interface.
 */
static void
_ibus_register_component (BusIBusImpl           *ibus,
                          GVariant              *parameters,
                          GDBusMethodInvocation *invocation)
{
    GVariant *variant = g_variant_get_child_value (parameters, 0);
    IBusComponent *component = (IBusComponent *) ibus_serializable_deserialize (variant);

    if (!IBUS_IS_COMPONENT (component)) {
        if (component)
            g_object_unref (component);
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "The first argument should be an IBusComponent.");
        return;
    }

    BusConnection *connection = bus_connection_lookup (g_dbus_method_invocation_get_connection (invocation));
    BusFactoryProxy *factory = bus_factory_proxy_new (connection);

    if (factory == NULL) {
        g_object_unref (component);
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "Create factory failed.");
        return;
    }

    g_object_ref_sink (component);

    BusComponent *buscomp = bus_component_new (component, factory);
    bus_component_set_destroy_with_factory (buscomp, TRUE);
    g_object_unref (component);
    g_object_unref (factory);

    ibus->registered_components = g_list_append (ibus->registered_components,
                                                g_object_ref_sink (buscomp));
    GList *engines = bus_component_get_engines (buscomp);
    g_list_foreach (engines, (GFunc) g_object_ref, NULL);
    ibus->register_engine_list = g_list_concat (ibus->register_engine_list,
                                               engines);

    g_signal_connect (buscomp, "destroy", G_CALLBACK (_component_destroy_cb), ibus);

    bus_ibus_impl_update_engines_hotkey_profile (ibus);

    g_dbus_method_invocation_return_value (invocation, NULL);
}

/**
 * _ibus_list_engines:
 *
 * Implement the "ListEngines" method call of the org.freedesktop.IBus interface.
 */
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
        g_variant_builder_add (&builder, "v", ibus_serializable_serialize ((IBusSerializable *) p->data));
    }
    g_list_free (engines);
    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(av)", &builder));
}

/**
 * _ibus_list_active_engines:
 *
 * Implement the "ListActiveEngines" method call of the org.freedesktop.IBus interface.
 */
static void
_ibus_list_active_engines (BusIBusImpl           *ibus,
                           GVariant              *parameters,
                           GDBusMethodInvocation *invocation)
{
    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("av"));

    GList *p;
    for (p = ibus->engine_list; p != NULL; p = p->next) {
        g_variant_builder_add (&builder, "v", ibus_serializable_serialize ((IBusSerializable *) p->data));
    }
    for (p = ibus->register_engine_list; p != NULL; p = p->next) {
        g_variant_builder_add (&builder, "v", ibus_serializable_serialize ((IBusSerializable *) p->data));
    }
    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(av)", &builder));
}

/**
 * _ibus_exit:
 *
 * Implement the "Exit" method call of the org.freedesktop.IBus interface.
 */
static void
_ibus_exit (BusIBusImpl           *ibus,
            GVariant              *parameters,
            GDBusMethodInvocation *invocation)
{
    gboolean restart = FALSE;
    g_variant_get (parameters, "(b)", &restart);

    g_dbus_method_invocation_return_value (invocation, NULL);

    /* Make sure the reply has been sent out before exit */
    g_dbus_connection_flush_sync (g_dbus_method_invocation_get_connection (invocation),
                                  NULL,
                                  NULL);
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
            exe = BINDIR "/ibus-daemon";

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

/**
 * _ibus_ping:
 *
 * Implement the "Ping" method call of the org.freedesktop.IBus interface.
 */
static void
_ibus_ping (BusIBusImpl           *ibus,
            GVariant              *parameters,
            GDBusMethodInvocation *invocation)
{
    g_dbus_method_invocation_return_value (invocation, parameters);
}

/**
 * _ibus_get_use_sys_layout:
 *
 * Implement the "GetUseSysLayout" method call of the org.freedesktop.IBus interface.
 */
static void
_ibus_get_use_sys_layout (BusIBusImpl           *ibus,
                          GVariant              *parameters,
                          GDBusMethodInvocation *invocation)
{
    g_dbus_method_invocation_return_value (invocation,
                    g_variant_new ("(b)", ibus->use_sys_layout));
}

/**
 * _ibus_get_use_global_engine:
 *
 * Implement the "GetUseGlobalEngine" method call of the org.freedesktop.IBus interface.
 */
static void
_ibus_get_use_global_engine (BusIBusImpl           *ibus,
                             GVariant              *parameters,
                             GDBusMethodInvocation *invocation)
{
    g_dbus_method_invocation_return_value (invocation,
                    g_variant_new ("(b)", ibus->use_global_engine));
}

/**
 * _ibus_get_global_engine:
 *
 * Implement the "GetGlobalEngine" method call of the org.freedesktop.IBus interface.
 */
static void
_ibus_get_global_engine (BusIBusImpl           *ibus,
                         GVariant              *parameters,
                         GDBusMethodInvocation *invocation)
{
    IBusEngineDesc *desc = NULL;

    do {
        if (!ibus->use_global_engine)
            break;
        BusInputContext *context = ibus->focused_context;
        if (context == NULL)
            context = ibus->fake_context;

        desc = bus_input_context_get_engine_desc (context);

        if (desc == NULL)
            break;

        GVariant *variant = ibus_serializable_serialize ((IBusSerializable *) desc);
        g_dbus_method_invocation_return_value (invocation,
                                               g_variant_new ("(v)", variant));
        return;
    } while (0);

    g_dbus_method_invocation_return_error (invocation,
                    G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                    "No global engine.");
}

struct _SetGlobalEngineData {
    BusIBusImpl *ibus;
    GDBusMethodInvocation *invocation;
};
typedef struct _SetGlobalEngineData SetGlobalEngineData;

static void
_ibus_set_global_engine_ready_cb (BusInputContext       *context,
                                  GAsyncResult          *res,
                                  SetGlobalEngineData   *data)
{
    BusIBusImpl *ibus = data->ibus;

    GError *error = NULL;
    if (!bus_input_context_set_engine_by_desc_finish (context, res, &error)) {
        g_error_free (error);
        g_dbus_method_invocation_return_error (data->invocation,
                                               G_DBUS_ERROR,
                                               G_DBUS_ERROR_FAILED,
                                               "Set global engine failed.");

    }
    else {
        g_dbus_method_invocation_return_value (data->invocation, NULL);

        if (ibus->use_global_engine && (context != ibus->focused_context)) {
            /* context and ibus->focused_context don't match. This means that
             * the focus is moved before _ibus_set_global_engine() asynchronous
             * call finishes. In this case, the engine for the context currently
             * being focused hasn't been updated. Update the engine here so that
             * subsequent _ibus_get_global_engine() call could return a
             * consistent engine name. */
            BusEngineProxy *engine = bus_input_context_get_engine (context);
            if (engine && ibus->focused_context != NULL) {
                g_object_ref (engine);
                bus_input_context_set_engine (context, NULL);
                bus_input_context_set_engine (ibus->focused_context, engine);
                g_object_unref (engine);
            }
        }
    }

    g_object_unref (ibus);
    g_slice_free (SetGlobalEngineData, data);
}

/**
 * _ibus_set_global_engine:
 *
 * Implement the "SetGlobalEngine" method call of the org.freedesktop.IBus interface.
 */
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

    BusInputContext *context = ibus->focused_context;
    if (context == NULL)
        context = ibus->fake_context;

    const gchar *engine_name = NULL;
    g_variant_get (parameters, "(&s)", &engine_name);

    IBusEngineDesc *desc = _find_engine_desc_by_name (ibus, engine_name);
    if (desc == NULL) {
        g_dbus_method_invocation_return_error (invocation,
                                               G_DBUS_ERROR,
                                               G_DBUS_ERROR_FAILED,
                                               "Can not find engine %s.",
                                               engine_name);
        return;
    }

    SetGlobalEngineData *data = g_slice_new0 (SetGlobalEngineData);
    data->ibus = g_object_ref (ibus);
    data->invocation = invocation;
    bus_input_context_set_engine_by_desc (context,
                                          desc,
                                          g_gdbus_timeout, /* timeout in msec. */
                                          NULL, /* we do not cancel the call. */
                                          (GAsyncReadyCallback) _ibus_set_global_engine_ready_cb,
                                          data);
}

/**
 * _ibus_is_global_engine_enabled:
 *
 * Implement the "IsGlobalEngineEnabled" method call of the org.freedesktop.IBus interface.
 */
static void
_ibus_is_global_engine_enabled (BusIBusImpl           *ibus,
                                GVariant              *parameters,
                                GDBusMethodInvocation *invocation)
{
    gboolean enabled = FALSE;

    do {
        if (!ibus->use_global_engine)
            break;

        BusInputContext *context = ibus->focused_context;
        if (context == NULL)
            context = ibus->fake_context;
        if (context == NULL)
            break;

        enabled = bus_input_context_is_enabled (context);
    } while (0);

    g_dbus_method_invocation_return_value (invocation,
                    g_variant_new ("(b)", enabled));
}

/**
 * bus_ibus_impl_service_method_call:
 *
 * Handle a D-Bus method call whose destination and interface name are both "org.freedesktop.IBus"
 */
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
        IBUS_SERVICE_CLASS (bus_ibus_impl_parent_class)->service_method_call (
                        service, connection, sender, object_path, interface_name, method_name,
                        parameters, invocation);
        return;
    }

    /* all methods in the xml definition above should be listed here. */
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
            methods[i].method_callback ((BusIBusImpl *) service, parameters, invocation);
            return;
        }
    }

    /* notreached - unknown method calls that are not in the introspection_xml should be handled by the GDBus library. */
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

/**
 * bus_ibus_impl_emit_signal:
 *
 * Send a D-Bus signal to buses (connections) that are listening to the signal.
 */
static void
bus_ibus_impl_emit_signal (BusIBusImpl *ibus,
                           const gchar *signal_name,
                           GVariant    *parameters)
{
    GDBusMessage *message = g_dbus_message_new_signal ("/org/freedesktop/IBus",
                                                       "org.freedesktop.IBus",
                                                       signal_name);
    /* set a non-zero serial to make libdbus happy */
    g_dbus_message_set_serial (message, 1);
    g_dbus_message_set_sender (message, "org.freedesktop.IBus");
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
    const gchar *name = ibus->global_engine_name ? ibus->global_engine_name : "";
    bus_ibus_impl_emit_signal (ibus, "GlobalEngineChanged",
                               g_variant_new ("(s)", name));
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
    static GQuark enable_unconditional = 0;
    static GQuark disable_unconditional = 0;
    static GQuark next = 0;
    static GQuark previous = 0;

    GQuark event;
    GList *engine_list;

    if (trigger == 0) {
        trigger = g_quark_from_static_string ("trigger");
        enable_unconditional = g_quark_from_static_string ("enable-unconditional");
        disable_unconditional = g_quark_from_static_string ("disable-unconditional");
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
    if (event == enable_unconditional) {
        gboolean enabled = bus_input_context_is_enabled (context);
        if (!enabled) {
            bus_input_context_enable (context);
        }
        return bus_input_context_is_enabled (context);
    }
    if (event == disable_unconditional) {
        gboolean enabled = bus_input_context_is_enabled (context);
        if (enabled) {
            bus_input_context_disable (context);
        }
        return !bus_input_context_is_enabled (context);
    }
    if (event == next) {
        if (bus_input_context_is_enabled (context)) {
            bus_ibus_impl_context_request_next_engine_in_menu (ibus, context);
        }
        else {
            bus_input_context_enable (context);
        }
        return TRUE;
    }
    if (event == previous) {
        if (bus_input_context_is_enabled (context)) {
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

/**
 * bus_ibus_impl_load_global_engine_name_from_config:
 *
 * Retrieve the "global_engine" config from the config daemon. Return NULL if the daemon is not ready.
 */
static gchar*
bus_ibus_impl_load_global_engine_name_from_config (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    if (ibus->config == NULL) {
        /* the config component is not started yet. */
        return NULL;
    }
    g_assert (IBUS_IS_CONFIG (ibus->config));

    GVariant *variant = ibus_config_get_value (ibus->config, "general", "global_engine");
    gchar *engine_name = NULL;
    if (variant != NULL) {
        g_variant_get (variant, "s", &engine_name);
        g_variant_unref (variant);
    }
    return engine_name;
}

/**
 * bus_ibus_impl_save_global_engine_name_to_config:
 *
 * Save the "global_engine" config value on the config daemon. No-op if the daemon is not ready.
 */
static void
bus_ibus_impl_save_global_engine_name_to_config (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    if (ibus->config &&
        ibus->use_global_engine &&
        ibus->global_engine_name) {
        ibus_config_set_value (ibus->config,
                        "general", "global_engine",
                        g_variant_new ("s", ibus->global_engine_name));
    }
}

/**
 * bus_ibus_impl_load_global_previous_engine_name_from_config:
 *
 * Retrieve the "global_previous_engine" config from the config daemon. Return NULL if the daemon is not ready.
 */
static gchar*
bus_ibus_impl_load_global_previous_engine_name_from_config (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    if (ibus->config == NULL) {
        /* the config component is not started yet. */
        return NULL;
    }
    g_assert (IBUS_IS_CONFIG (ibus->config));

    GVariant *value = ibus_config_get_value (ibus->config, "general", "global_previous_engine");
    if (value == NULL)
        return NULL;
    gchar *engine_name = NULL;
    g_variant_get (value, "(s)", &engine_name);
    g_variant_unref (value);
    return engine_name;
}

/**
 * bus_ibus_impl_save_global_previous_engine_name_to_config:
 *
 * Save the "global_previous_engine" config value on the config daemon. No-op if the daemon is not ready.
 */
static void
bus_ibus_impl_save_global_previous_engine_name_to_config (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    if (ibus->config &&
        ibus->use_global_engine &&
        ibus->global_previous_engine_name) {
        ibus_config_set_value (ibus->config,
                        "general", "global_previous_engine",
                        g_variant_new ("s", ibus->global_previous_engine_name));
    }
}

/**
 * _add_engine_hotkey:
 *
 * Check the engine-specific hot key of the engine, and update ibus->engines_hotkey_profile.
 */
static void
_add_engine_hotkey (IBusEngineDesc *engine, BusIBusImpl *ibus)
{
    const gchar *hotkeys;
    gchar **hotkey_list;
    gchar **p;
    gchar *hotkey;
    GList *engine_list;

    GQuark event;
    guint keyval;
    guint modifiers;

    if (!engine) {
        return;
    }

    hotkeys = ibus_engine_desc_get_hotkeys (engine);

    if (!hotkeys || !*hotkeys) {
        return;
    }

    hotkey_list = g_strsplit_set (hotkeys, ";,", 0);

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

/**
 * bus_ibus_impl_update_engines_hotkey_profile:
 *
 * Check engine-specific hot keys of all active engines, and update ibus->engines_hotkey_profile.
 */
static void
bus_ibus_impl_update_engines_hotkey_profile (BusIBusImpl *ibus)
{
    if (ibus->engines_hotkey_profile) {
        g_object_unref (ibus->engines_hotkey_profile);
    }

    if (ibus->hotkey_to_engines_map) {
        g_hash_table_unref (ibus->hotkey_to_engines_map);
    }

    ibus->engines_hotkey_profile = ibus_hotkey_profile_new ();
    ibus->hotkey_to_engines_map =
        g_hash_table_new_full (NULL, NULL, NULL, (GDestroyNotify) g_list_free);

    g_list_foreach (ibus->register_engine_list, (GFunc) _add_engine_hotkey, ibus);
    g_list_foreach (ibus->engine_list, (GFunc) _add_engine_hotkey, ibus);
}

gboolean
bus_ibus_impl_is_use_sys_layout (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    return ibus->use_sys_layout;
}

gboolean
bus_ibus_impl_is_embed_preedit_text (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    return ibus->embed_preedit_text;
}

BusInputContext *
bus_ibus_impl_get_focused_input_context (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    return ibus->focused_context;
}
