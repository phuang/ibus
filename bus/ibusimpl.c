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
#include <dbus/dbus.h>
#include "ibusimpl.h"
#include "dbusimpl.h"
#include "server.h"
#include "connection.h"
#include "registry.h"
#include "factoryproxy.h"
#include "panelproxy.h"
#include "inputcontext.h"
#include "option.h"

enum {
    LAST_SIGNAL,
};

enum {
    PROP_0,
};

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_ibus_impl_destroy           (BusIBusImpl        *ibus);
static gboolean bus_ibus_impl_ibus_message      (BusIBusImpl        *ibus,
                                                 BusConnection      *connection,
                                                 IBusMessage        *message);
static void     bus_ibus_impl_add_factory       (BusIBusImpl        *ibus,
                                                 BusFactoryProxy    *factory);
static void     bus_ibus_impl_set_trigger       (BusIBusImpl        *ibus,
                                                 GValue             *value);
static void     bus_ibus_impl_set_next_engine_in_menu
                                                (BusIBusImpl        *ibus,
                                                 GValue             *value);
static void     bus_ibus_impl_set_previous_engine
                                                (BusIBusImpl        *ibus,
                                                 GValue             *value);

static void     bus_ibus_impl_set_preload_engines
                                                (BusIBusImpl        *ibus,
                                                 GValue             *value);
static void     bus_ibus_impl_set_use_sys_layout
                                                (BusIBusImpl        *ibus,
                                                 GValue             *value);
static void     bus_ibus_impl_set_embed_preedit_text
                                                (BusIBusImpl        *ibus,
                                                 GValue             *value);
static void     bus_ibus_impl_set_enable_by_default
                                                (BusIBusImpl        *ibus,
                                                 GValue             *value);

static void     bus_ibus_impl_set_use_global_engine
                                                (BusIBusImpl        *ibus,
                                                 GValue             *value);
static void     bus_ibus_impl_set_global_engine (BusIBusImpl        *ibus,
                                                 BusEngineProxy     *engine);

static void     bus_ibus_impl_registry_changed  (BusIBusImpl        *ibus);
static void     bus_ibus_impl_global_engine_changed
                                                (BusIBusImpl        *ibus);

static void     _factory_destroy_cb             (BusFactoryProxy    *factory,
                                                 BusIBusImpl        *ibus);

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

G_DEFINE_TYPE(BusIBusImpl, bus_ibus_impl, IBUS_TYPE_SERVICE)

BusIBusImpl *
bus_ibus_impl_get_default (void)
{
    static BusIBusImpl *ibus = NULL;

    if (ibus == NULL) {
        ibus = (BusIBusImpl *) g_object_new (BUS_TYPE_IBUS_IMPL,
                                             "path", IBUS_PATH_IBUS,
                                             NULL);
        bus_dbus_impl_register_object (BUS_DEFAULT_DBUS,
                                       (IBusService *)ibus);
    }
    return ibus;
}

static void
bus_ibus_impl_class_init (BusIBusImplClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_ibus_impl_destroy;

    IBUS_SERVICE_CLASS (klass)->ibus_message = (ServiceIBusMessageFunc) bus_ibus_impl_ibus_message;

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
                          GValue      *value)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    GValueArray *array;
    gint i;

    ibus_hotkey_profile_remove_hotkey_by_event (ibus->hotkey_profile, hotkey);

    if (value == NULL) {
        return;
    }

    g_return_if_fail (G_VALUE_TYPE (value) == G_TYPE_VALUE_ARRAY);
    array = g_value_get_boxed (value);

    for (i = 0; i < array->n_values; i++) {
       GValue *str;

       str = g_value_array_get_nth (array, i);
       g_return_if_fail (G_VALUE_TYPE (str) == G_TYPE_STRING);

       ibus_hotkey_profile_add_hotkey_from_string (ibus->hotkey_profile,
                                                   g_value_get_string (str),
                                                   hotkey);
   }

}

static void
bus_ibus_impl_set_trigger (BusIBusImpl *ibus,
                           GValue      *value)
{
    GQuark hotkey = g_quark_from_static_string ("trigger");
    bus_ibus_impl_set_hotkey (ibus, hotkey, value);
    if (value == NULL) {
        ibus_hotkey_profile_add_hotkey (ibus->hotkey_profile,
                                        IBUS_space,
                                        IBUS_CONTROL_MASK,
                                        hotkey);
    }
}

static void
bus_ibus_impl_set_next_engine_in_menu (BusIBusImpl *ibus,
                                       GValue      *value)
{
    GQuark hotkey = g_quark_from_static_string ("next-engine-in-menu");
    bus_ibus_impl_set_hotkey (ibus, hotkey, value);
}

static void
bus_ibus_impl_set_previous_engine (BusIBusImpl *ibus,
                                   GValue      *value)
{
    GQuark hotkey = g_quark_from_static_string ("previous-engine");
    bus_ibus_impl_set_hotkey (ibus, hotkey, value);
}

static void
bus_ibus_impl_set_preload_engines (BusIBusImpl *ibus,
                                   GValue      *value)
{
    GList *engine_list = NULL;

    g_list_foreach (ibus->engine_list, (GFunc) g_object_unref, NULL);
    g_list_free (ibus->engine_list);

    if (value != NULL && G_VALUE_TYPE (value) == G_TYPE_VALUE_ARRAY) {
        GValueArray *array;
        gint i;

        array = (GValueArray *) g_value_get_boxed (value);
        for (i = 0; array && i < array->n_values; i++) {
            const gchar *engine_name;
            IBusEngineDesc *engine;

            if (G_VALUE_TYPE (&array->values[i]) != G_TYPE_STRING)
                continue;

            engine_name = g_value_get_string (&array->values[i]);

            engine = bus_registry_find_engine_by_name (ibus->registry, engine_name);

            if (engine == NULL || g_list_find (engine_list, engine) != NULL)
                continue;

            engine_list = g_list_append (engine_list, engine);
        }
    }

    g_list_foreach (engine_list, (GFunc) g_object_ref, NULL);
    ibus->engine_list = engine_list;

    if (ibus->engine_list) {
        IBusComponent *component;

        component = ibus_component_get_from_engine ((IBusEngineDesc *) ibus->engine_list->data);
        if (component && !ibus_component_is_running (component)) {
            ibus_component_start (component, g_verbose);
        }
    }
}

static void
bus_ibus_impl_set_use_sys_layout (BusIBusImpl *ibus,
                                  GValue      *value)
{
    if (value != NULL && G_VALUE_TYPE (value) == G_TYPE_BOOLEAN) {
        ibus->use_sys_layout = g_value_get_boolean (value);
    }
}

static void
bus_ibus_impl_set_embed_preedit_text (BusIBusImpl *ibus,
                                      GValue      *value)
{
    if (value != NULL && G_VALUE_TYPE (value) == G_TYPE_BOOLEAN) {
        ibus->embed_preedit_text = g_value_get_boolean (value);
    }
}

static void
bus_ibus_impl_set_enable_by_default (BusIBusImpl *ibus,
                                     GValue      *value)
{
    if (value != NULL && G_VALUE_TYPE (value) == G_TYPE_BOOLEAN) {
        ibus->enable_by_default = g_value_get_boolean (value);
    }
}

static void
bus_ibus_impl_set_use_global_engine (BusIBusImpl *ibus,
                                     GValue      *value)
{
    gboolean new_value;

    if (value == NULL || G_VALUE_TYPE (value) != G_TYPE_BOOLEAN) {
        return;
    }

    new_value = g_value_get_boolean (value);
    if (ibus->use_global_engine == new_value) {
        return;
    }

    if (new_value == TRUE) {
        BusEngineProxy *engine;
        /* turn on use_global_engine option */
        ibus->use_global_engine = TRUE;
        engine = ibus->focused_context != NULL ?
                    bus_input_context_get_engine (ibus->focused_context) : NULL;
        if (engine) {
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
    GValue value = { 0 };
    GList *engines, *list;
    gchar *lang, *p;
    GValueArray *array;

    if (done || ibus->config == NULL) {
        return;
    }

    if (ibus_config_get_value (ibus->config, "general", "preload_engines", &value)) {
        done = TRUE;
        g_value_unset (&value);
        return;
    }

    done = TRUE;
    lang = g_strdup (setlocale (LC_ALL, NULL));
    p = index (lang, '.');
    if (p) {
        *p = '\0';
    }

    engines = bus_registry_get_engines_by_language (ibus->registry, lang);
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

    g_value_init (&value, G_TYPE_VALUE_ARRAY);
    array = g_value_array_new (5);
    for (list = engines; list != NULL; list = list->next) {
        IBusEngineDesc *desc;
        GValue name = { 0 };
        desc = (IBusEngineDesc *)list->data;

        /* ignore engines with rank <== 0 */
        if (desc->rank <= 0)
            break;
        g_value_init (&name, G_TYPE_STRING);
        g_value_set_string (&name, desc->name);
        g_value_array_append (array, &name);
    }
    g_value_take_boxed (&value, array);
    ibus_config_set_value (ibus->config, "general", "preload_engines", &value);
    g_value_unset (&value);
    g_list_free (engines);
}

static void
bus_ibus_impl_reload_config (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    gint i;
    GValue value = { 0 };

    const static struct {
        gchar *section;
        gchar *key;
        void ( *func) (BusIBusImpl *, GValue *);
    } entries [] = {
        { "general/hotkey", "trigger", bus_ibus_impl_set_trigger },
        /* Only for backward compatibility, shall be removed later. */
        { "general/hotkey", "next_engine", bus_ibus_impl_set_next_engine_in_menu },
        { "general/hotkey", "next_engine_in_menu", bus_ibus_impl_set_next_engine_in_menu },
        /* Only for backward compatibility, shall be removed later. */
        { "general/hotkey", "prev_engine", bus_ibus_impl_set_previous_engine },
        { "general/hotkey", "previous_engine", bus_ibus_impl_set_previous_engine },
        { "general", "preload_engines", bus_ibus_impl_set_preload_engines },
        { "general", "use_system_keyboard_layout", bus_ibus_impl_set_use_sys_layout },
        { "general", "use_global_engine", bus_ibus_impl_set_use_global_engine },
        { "general", "embed_preedit_text", bus_ibus_impl_set_embed_preedit_text },
        { "general", "enable_by_default", bus_ibus_impl_set_enable_by_default },
    };

    for (i = 0; i < G_N_ELEMENTS (entries); i++) {
        if (ibus->config != NULL &&
            ibus_config_get_value (ibus->config,
                                   entries[i].section,
                                   entries[i].key,
                                   &value)) {
            entries[i].func (ibus, &value);
            g_value_unset (&value);
        }
        else {
            entries[i].func (ibus, NULL);
        }
    }
}

static void
_config_value_changed_cb (IBusConfig  *config,
                          gchar       *section,
                          gchar       *key,
                          GValue      *value,
                          BusIBusImpl *ibus)
{
    g_assert (IBUS_IS_CONFIG (config));
    g_assert (section);
    g_assert (key);
    g_assert (value);
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    gint i;

    const static struct {
        gchar *section;
        gchar *key;
        void ( *func) (BusIBusImpl *, GValue *);
    } entries [] = {
        { "general/hotkey", "trigger", bus_ibus_impl_set_trigger },
        /* Only for backward compatibility, shall be removed later. */
        { "general/hotkey", "next_engine", bus_ibus_impl_set_next_engine_in_menu },
        { "general/hotkey", "next_engine_in_menu", bus_ibus_impl_set_next_engine_in_menu },
        /* Only for backward compatibility, shall be removed later. */
        { "general/hotkey", "prev_engine", bus_ibus_impl_set_previous_engine },
        { "general/hotkey", "previous_engine", bus_ibus_impl_set_previous_engine },
        { "general", "preload_engines",    bus_ibus_impl_set_preload_engines },
        { "general", "use_system_keyboard_layout", bus_ibus_impl_set_use_sys_layout },
        { "general", "use_global_engine", bus_ibus_impl_set_use_global_engine },
        { "general", "embed_preedit_text", bus_ibus_impl_set_embed_preedit_text },
        { "general", "enable_by_default", bus_ibus_impl_set_enable_by_default },
    };

    for (i = 0; i < G_N_ELEMENTS (entries); i++) {
        if (g_strcmp0 (entries[i].section, section) == 0 &&
            g_strcmp0 (entries[i].key, key) == 0) {
            entries[i].func (ibus, value);
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
                ibus_object_destroy (IBUS_OBJECT (ibus->panel));
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
                ibus_object_destroy (IBUS_OBJECT (ibus->config));
                /* config should be NULL */
                g_assert (ibus->config == NULL);
            }

            connection = bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS, new_name);
            g_return_if_fail (connection != NULL);

            ibus->config = g_object_new (IBUS_TYPE_CONFIG,
                                         "name", NULL,
                                         "path", IBUS_PATH_CONFIG,
                                         "connection", connection,
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

    bus_server_quit (BUS_DEFAULT_SERVER);
    ibus_object_destroy ((IBusObject *) BUS_DEFAULT_SERVER);
    IBUS_OBJECT_CLASS(bus_ibus_impl_parent_class)->destroy (IBUS_OBJECT (ibus));
}

/* introspectable interface */
static IBusMessage *
_ibus_introspect (BusIBusImpl     *ibus,
                  IBusMessage     *message,
                  BusConnection   *connection)
{
    static const gchar *introspect =
        DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE
        "<node>\n"
        "  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
        "    <method name=\"Introspect\">\n"
        "      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "  </interface>\n"
        "  <interface name=\"org.freedesktop.IBus\">\n"
        "    <method name=\"GetAddress\">\n"
        "      <arg name=\"address\" direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "    <method name=\"CreateInputContext\">\n"
        "      <arg name=\"name\" direction=\"in\" type=\"s\"/>\n"
        "      <arg name=\"context\" direction=\"out\" type=\"o\"/>\n"
        "    </method>\n"
        "    <method name=\"CurrentInputContext\">\n"
        "      <arg name=\"name\" direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "    <method name=\"RegisterComponent\">\n"
        "      <arg name=\"components\" direction=\"in\" type=\"v\"/>\n"
        "    </method>\n"
        "    <method name=\"ListEngines\">\n"
        "      <arg name=\"engines\" direction=\"out\" type=\"av\"/>\n"
        "    </method>\n"
        "    <method name=\"ListActiveEngines\">\n"
        "      <arg name=\"engines\" direction=\"out\" type=\"av\"/>\n"
        "    </method>\n"
        "    <method name=\"Exit\">\n"
        "      <arg name=\"restart\" direction=\"in\" type=\"b\"/>\n"
        "    </method>\n"
        "    <method name=\"Ping\">\n"
        "      <arg name=\"data\" direction=\"in\" type=\"v\"/>\n"
        "      <arg name=\"data\" direction=\"out\" type=\"v\"/>\n"
        "    </method>\n"
        "    <method name=\"GetUseSysLayout\">\n"
        "      <arg name=\"enable\" direction=\"out\" type=\"b\"/>\n"
        "    </method>\n"
        "    <method name=\"GetUseGlobalEngine\">\n"
        "      <arg name=\"enable\" direction=\"out\" type=\"b\"/>\n"
        "    </method>\n"
        "    <method name=\"GetGlobalEngine\">\n"
        "      <arg name=\"desc\" direction=\"out\" type=\"v\"/>\n"
        "    </method>\n"
        "    <method name=\"SetGlobalEngine\">\n"
        "      <arg name=\"name\" direction=\"in\" type=\"s\"/>\n"
        "    </method>\n"
        "    <method name=\"IsGlobalEngineEnabled\">\n"
        "      <arg name=\"enable\" direction=\"out\" type=\"b\"/>\n"
        "    </method>\n"
        "    <signal name=\"RegistryChanged\">\n"
        "    </signal>\n"
        "    <signal name=\"GlobalEngineChanged\">\n"
        "    </signal>\n"
        "  </interface>\n"
        "</node>\n";

    IBusMessage *reply_message;
    reply_message = ibus_message_new_method_return (message);
    ibus_message_append_args (reply_message,
                              G_TYPE_STRING, &introspect,
                              G_TYPE_INVALID);

    return reply_message;
}



static IBusMessage *
_ibus_get_address (BusIBusImpl     *ibus,
                   IBusMessage     *message,
                   BusConnection   *connection)
{
    const gchar *address;
    IBusMessage *reply;

    address = ibus_server_get_address (IBUS_SERVER (BUS_DEFAULT_SERVER));

    reply = ibus_message_new_method_return (message);
    ibus_message_append_args (reply,
                              G_TYPE_STRING, &address,
                              G_TYPE_INVALID);

    return reply;
}

static BusEngineProxy *
bus_ibus_impl_create_engine (IBusEngineDesc *engine_desc)
{
    IBusComponent *comp;
    BusFactoryProxy *factory;
    BusEngineProxy *engine;
    GTimeVal t1, t2;

    factory = bus_factory_proxy_get_from_engine (engine_desc);

    if (factory == NULL) {
        /* try to execute the engine */
        comp = ibus_component_get_from_engine (engine_desc);
        g_assert (comp);

        if (!ibus_component_is_running (comp)) {
            ibus_component_start (comp, g_verbose);
            g_get_current_time (&t1);
            while (1) {
                if (g_main_context_pending (NULL)) {
                    g_main_context_iteration (NULL, FALSE);
                    factory = bus_factory_proxy_get_from_engine (engine_desc);
                    if (factory != NULL) {
                        break;
                    }
                }
                g_get_current_time (&t2);
                if (t2.tv_sec - t1.tv_sec >= 5)
                    break;
            }
        }
        else {
            factory = bus_factory_proxy_get_from_engine (engine_desc);
        }
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
                          gchar *engine_name)
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
                            gchar           *engine_name,
                            BusIBusImpl     *ibus)
{
    IBusEngineDesc *engine_desc = NULL;
    BusEngineProxy *engine;

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
                engine_name = bus_ibus_impl_load_global_engine_name_from_config (ibus);
                if (engine_name) {
                    engine_desc = _find_engine_desc_by_name (ibus, engine_name);
                    g_free (engine_name);
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

    if (engine_desc != NULL) {
        engine = bus_ibus_impl_create_engine (engine_desc);
        if (engine != NULL) {
            bus_ibus_impl_set_context_engine (ibus, context, engine);
        }
    }
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

    if (next_desc != NULL) {
        engine = bus_ibus_impl_create_engine (next_desc);
        if (engine != NULL) {
            bus_ibus_impl_set_context_engine (ibus, context, engine);
        }
    }
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

        ibus_object_destroy ((IBusObject *)ibus->global_engine);
        /* global_engine should be NULL */
        g_assert (ibus->global_engine == NULL);
    }

    if (engine != NULL && !IBUS_OBJECT_DESTROYED (engine)) {
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

static IBusMessage *
_ibus_create_input_context (BusIBusImpl     *ibus,
                            IBusMessage     *message,
                            BusConnection   *connection)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    gint i;
    gchar *client;
    IBusError *error;
    IBusMessage *reply;
    BusInputContext *context;
    const gchar *path;

    if (!ibus_message_get_args (message,
                                &error,
                                G_TYPE_STRING, &client,
                                G_TYPE_INVALID)) {
        reply = ibus_message_new_error (message,
                                        DBUS_ERROR_INVALID_ARGS,
                                        "Argument 1 of CreateInputContext should be an string");
        ibus_error_free (error);
        return reply;
    }

    context = bus_input_context_new (connection, client);
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

    for (i = 0; i < G_N_ELEMENTS (signals); i++) {
        g_signal_connect (context,
                          signals[i].name,
                          signals[i].callback,
                          ibus);
    }

    if (ibus->enable_by_default) {
        bus_input_context_enable (context);
    }

    path = ibus_service_get_path ((IBusService *) context);
    reply = ibus_message_new_method_return (message);
    ibus_message_append_args (reply,
                              IBUS_TYPE_OBJECT_PATH, &path,
                              G_TYPE_INVALID);

    bus_dbus_impl_register_object (BUS_DEFAULT_DBUS,
                                   (IBusService *)context);
    return reply;
}

static IBusMessage *
_ibus_current_input_context (BusIBusImpl     *ibus,
                            IBusMessage     *message,
                            BusConnection   *connection)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    IBusMessage *reply;
    const gchar *path;

    if (!ibus->focused_context)
    {
        reply = ibus_message_new_error (message,
                                        DBUS_ERROR_FAILED,
                                        "No input context focused");
        return reply;
    }

    reply = ibus_message_new_method_return (message);
    path = ibus_service_get_path((IBusService *)ibus->focused_context);
    ibus_message_append_args (reply,
                              G_TYPE_STRING, &path,
                              G_TYPE_INVALID);

    return reply;
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


static IBusMessage *
_ibus_register_component (BusIBusImpl     *ibus,
                          IBusMessage     *message,
                          BusConnection   *connection)
{
    IBusMessage *reply;
    IBusError *error;
    gboolean retval;
    GList *engines;
    IBusComponent *component;
    BusFactoryProxy *factory;

    retval = ibus_message_get_args (message, &error,
                                    IBUS_TYPE_COMPONENT, &component,
                                    G_TYPE_INVALID);

    if (!retval) {
        reply = ibus_message_new_error_printf (message,
                                               DBUS_ERROR_INVALID_ARGS,
                                               "1st Argument must be IBusComponent: %s",
                                               error->message);
        ibus_error_free (error);
        return reply;
    }

    g_object_ref_sink (component);
    factory = bus_factory_proxy_new (component, connection);

    if (factory == NULL) {
        reply = ibus_message_new_error (message,
                                        DBUS_ERROR_FAILED,
                                        "Can not create factory");
        return reply;
    }

    bus_ibus_impl_add_factory (ibus, factory);

    engines = ibus_component_get_engines (component);

    g_list_foreach (engines, (GFunc) g_object_ref, NULL);
    ibus->register_engine_list = g_list_concat (ibus->register_engine_list, engines);
    g_object_unref (component);

    reply = ibus_message_new_method_return (message);
    return reply;
}

static IBusMessage *
_ibus_list_engines (BusIBusImpl   *ibus,
                    IBusMessage   *message,
                    BusConnection *connection)
{
    IBusMessage *reply;
    IBusMessageIter iter, sub_iter;
    GList *engines, *p;

    reply = ibus_message_new_method_return (message);

    ibus_message_iter_init_append (reply, &iter);
    ibus_message_iter_open_container (&iter, IBUS_TYPE_ARRAY, "v", &sub_iter);

    engines = bus_registry_get_engines (ibus->registry);
    for (p = engines; p != NULL; p = p->next) {
        ibus_message_iter_append (&sub_iter, IBUS_TYPE_ENGINE_DESC, &(p->data));
    }
    g_list_free (engines);
    ibus_message_iter_close_container (&iter, &sub_iter);

    return reply;
}

static IBusMessage *
_ibus_list_active_engines (BusIBusImpl   *ibus,
                           IBusMessage   *message,
                           BusConnection *connection)
{
    IBusMessage *reply;
    IBusMessageIter iter, sub_iter;
    GList *p;

    reply = ibus_message_new_method_return (message);

    ibus_message_iter_init_append (reply, &iter);
    ibus_message_iter_open_container (&iter, IBUS_TYPE_ARRAY, "v", &sub_iter);

    for (p = ibus->engine_list; p != NULL; p = p->next) {
        ibus_message_iter_append (&sub_iter, IBUS_TYPE_ENGINE_DESC, &(p->data));
    }

    for (p = ibus->register_engine_list; p != NULL; p = p->next) {
        ibus_message_iter_append (&sub_iter, IBUS_TYPE_ENGINE_DESC, &(p->data));
    }
    ibus_message_iter_close_container (&iter, &sub_iter);

    return reply;
}


static IBusMessage *
_ibus_exit (BusIBusImpl     *ibus,
            IBusMessage     *message,
            BusConnection   *connection)
{
    IBusMessage *reply;
    IBusError *error;
    gboolean restart;

    if (!ibus_message_get_args (message,
                                &error,
                                G_TYPE_BOOLEAN, &restart,
                                G_TYPE_INVALID)) {
        reply = ibus_message_new_error (message,
                                        DBUS_ERROR_INVALID_ARGS,
                                        "Argument 1 of Exit should be an boolean");
        ibus_error_free (error);
        return reply;
    }

    reply = ibus_message_new_method_return (message);
    ibus_connection_send ((IBusConnection *) connection, reply);
    ibus_connection_flush ((IBusConnection *) connection);
    ibus_message_unref (reply);

    ibus_object_destroy ((IBusObject *) BUS_DEFAULT_SERVER);

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

    return NULL;
}

static IBusMessage *
_ibus_ping (BusIBusImpl     *ibus,
            IBusMessage     *message,
            BusConnection   *connection)
{
    IBusMessage *reply;
    IBusMessageIter src, dst;

    reply = ibus_message_new_method_return (message);

    ibus_message_iter_init (message, &src);
    ibus_message_iter_init_append (reply, &dst);

    ibus_message_iter_copy_data (&dst, &src);

    return reply;
}

static IBusMessage *
_ibus_get_use_sys_layout (BusIBusImpl     *ibus,
                          IBusMessage     *message,
                          BusConnection   *connection)
{
    IBusMessage *reply;

    reply = ibus_message_new_method_return (message);
    ibus_message_append_args (reply,
            G_TYPE_BOOLEAN, &ibus->use_sys_layout,
            G_TYPE_INVALID);

    return reply;
}

static IBusMessage *
_ibus_get_use_global_engine (BusIBusImpl     *ibus,
                             IBusMessage     *message,
                             BusConnection   *connection)
{
    IBusMessage *reply;

    reply = ibus_message_new_method_return (message);
    ibus_message_append_args (reply,
            G_TYPE_BOOLEAN, &ibus->use_global_engine,
            G_TYPE_INVALID);

    return reply;
}

static IBusMessage *
_ibus_get_global_engine (BusIBusImpl     *ibus,
                         IBusMessage     *message,
                         BusConnection   *connection)
{
    IBusMessage *reply;

    if (ibus->use_global_engine && ibus->global_engine) {
        IBusEngineDesc *desc = bus_engine_proxy_get_desc (ibus->global_engine);
        if (desc != NULL) {
            reply = ibus_message_new_method_return (message);
            ibus_message_append_args (reply,
                                      IBUS_TYPE_ENGINE_DESC, &desc,
                                      G_TYPE_INVALID);
            return reply;
        }
    }

    reply = ibus_message_new_error (message, DBUS_ERROR_FAILED,
                                    "No global engine.");
    return reply;
}

static IBusMessage *
_ibus_set_global_engine (BusIBusImpl     *ibus,
                         IBusMessage     *message,
                         BusConnection   *connection)
{
    gboolean retval;
    IBusMessage *reply;
    IBusError *error;
    gchar *new_engine_name;
    gchar *old_engine_name;

    if (!ibus->use_global_engine) {
        reply = ibus_message_new_error (message, DBUS_ERROR_FAILED,
                                        "Global engine feature is disable.");
        return reply;
    }

    retval = ibus_message_get_args (message,
                                    &error,
                                    G_TYPE_STRING, &new_engine_name,
                                    G_TYPE_INVALID);
    if (!retval) {
        reply = ibus_message_new_error (message,
                                        error->name,
                                        error->message);
        ibus_error_free (error);
        return reply;
    }

    reply = ibus_message_new_method_return (message);
    old_engine_name = NULL;

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
        return reply;
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

    return reply;
}

static IBusMessage *
_ibus_is_global_engine_enabled (BusIBusImpl     *ibus,
                                IBusMessage     *message,
                                BusConnection   *connection)
{
    IBusMessage *reply;
    gboolean enabled = (ibus->use_global_engine && ibus->global_engine &&
                        bus_engine_proxy_is_enabled (ibus->global_engine));

    reply = ibus_message_new_method_return (message);
    ibus_message_append_args (reply,
                              G_TYPE_BOOLEAN, &enabled,
                              G_TYPE_INVALID);
    return reply;
}


static gboolean
bus_ibus_impl_ibus_message (BusIBusImpl     *ibus,
                            BusConnection   *connection,
                            IBusMessage     *message)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    gint i;
    IBusMessage *reply_message = NULL;

    static const struct {
        const gchar *interface;
        const gchar *name;
        IBusMessage *(* handler) (BusIBusImpl *, IBusMessage *, BusConnection *);
    } handlers[] =  {
        /* Introspectable interface */
        { DBUS_INTERFACE_INTROSPECTABLE,
                               "Introspect",            _ibus_introspect },
        /* IBus interface */
        { IBUS_INTERFACE_IBUS, "GetAddress",            _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "CreateInputContext",    _ibus_create_input_context },
        { IBUS_INTERFACE_IBUS, "CurrentInputContext",   _ibus_current_input_context },
        { IBUS_INTERFACE_IBUS, "RegisterComponent",     _ibus_register_component },
        { IBUS_INTERFACE_IBUS, "ListEngines",           _ibus_list_engines },
        { IBUS_INTERFACE_IBUS, "ListActiveEngines",     _ibus_list_active_engines },
        { IBUS_INTERFACE_IBUS, "Exit",                  _ibus_exit },
        { IBUS_INTERFACE_IBUS, "Ping",                  _ibus_ping },
        { IBUS_INTERFACE_IBUS, "GetUseSysLayout",       _ibus_get_use_sys_layout },
        { IBUS_INTERFACE_IBUS, "GetUseGlobalEngine",    _ibus_get_use_global_engine },
        { IBUS_INTERFACE_IBUS, "GetGlobalEngine",       _ibus_get_global_engine },
        { IBUS_INTERFACE_IBUS, "SetGlobalEngine",       _ibus_set_global_engine },
        { IBUS_INTERFACE_IBUS, "IsGlobalEngineEnabled", _ibus_is_global_engine_enabled },
    };

    ibus_message_set_sender (message, bus_connection_get_unique_name (connection));
    ibus_message_set_destination (message, DBUS_SERVICE_DBUS);

    if (ibus_message_get_type (message) == DBUS_MESSAGE_TYPE_METHOD_CALL) {
        for (i = 0; i < G_N_ELEMENTS (handlers); i++) {
            if (ibus_message_is_method_call (message,
                                             handlers[i].interface,
                                             handlers[i].name)) {

                reply_message = handlers[i].handler (ibus, message, connection);
                if (reply_message) {

                    ibus_message_set_sender (reply_message, DBUS_SERVICE_DBUS);
                    ibus_message_set_destination (reply_message, bus_connection_get_unique_name (connection));
                    ibus_message_set_no_reply (reply_message, TRUE);

                    ibus_connection_send ((IBusConnection *) connection, reply_message);
                    ibus_message_unref (reply_message);
                }

                g_signal_stop_emission_by_name (ibus, "ibus-message");
                return TRUE;
            }
        }
    }

    return IBUS_SERVICE_CLASS(bus_ibus_impl_parent_class)->ibus_message (
                                    (IBusService *) ibus,
                                    (IBusConnection *) connection,
                                    message);
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
bus_ibus_impl_registry_changed (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    IBusMessage *message;

    message = ibus_message_new_signal (IBUS_PATH_IBUS,
                                       IBUS_INTERFACE_IBUS,
                                       "RegistryChanged");
    ibus_message_append_args (message,
                              G_TYPE_INVALID);
    ibus_message_set_sender (message, IBUS_SERVICE_IBUS);

    bus_dbus_impl_dispatch_message_by_rule (BUS_DEFAULT_DBUS, message, NULL);

    ibus_message_unref (message);

}

static void
bus_ibus_impl_global_engine_changed (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    IBusMessage *message;

    message = ibus_message_new_signal (IBUS_PATH_IBUS,
                                       IBUS_INTERFACE_IBUS,
                                       "GlobalEngineChanged");
    ibus_message_append_args (message,
                              G_TYPE_INVALID);
    ibus_message_set_sender (message, IBUS_SERVICE_IBUS);

    bus_dbus_impl_dispatch_message_by_rule (BUS_DEFAULT_DBUS, message, NULL);

    ibus_message_unref (message);
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

    if (trigger == 0) {
        trigger = g_quark_from_static_string ("trigger");
        next = g_quark_from_static_string ("next-engine-in-menu");
        previous = g_quark_from_static_string ("previous-engine");
    }

    GQuark event = ibus_hotkey_profile_filter_key_event (ibus->hotkey_profile,
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
    return FALSE;
}

static gchar*
bus_ibus_impl_load_global_engine_name_from_config (BusIBusImpl *ibus)
{
    GValue value = { 0 };
    gchar *global_engine_name = NULL;

    g_assert (IBUS_IS_CONFIG (ibus->config));

    if (ibus_config_get_value (ibus->config, "general", "global_engine", &value) &&
        G_VALUE_TYPE (&value) == G_TYPE_STRING) {
        global_engine_name = g_value_dup_string (&value);
        g_value_unset (&value);
    }

    return global_engine_name;
}

static void
bus_ibus_impl_save_global_engine_name_to_config (BusIBusImpl *ibus)
{
    g_assert (IBUS_IS_CONFIG (ibus->config));

    if (ibus->use_global_engine && ibus->global_engine) {
        GValue value = { 0 };
        g_value_init (&value, G_TYPE_STRING);
        g_value_set_static_string (&value, bus_engine_proxy_get_desc (ibus->global_engine)->name);

        ibus_config_set_value (ibus->config, "general", "global_engine", &value);
        g_value_unset (&value);
    }
}

static gchar*
bus_ibus_impl_load_global_previous_engine_name_from_config (BusIBusImpl *ibus)
{
    GValue value = { 0 };
    gchar *global_previous_engine_name = NULL;

    g_assert (IBUS_IS_CONFIG (ibus->config));

    if (ibus_config_get_value (ibus->config, "general", "global_previous_engine", &value) &&
        G_VALUE_TYPE (&value) == G_TYPE_STRING) {
        global_previous_engine_name = g_value_dup_string (&value);
        g_value_unset (&value);
    }

    return global_previous_engine_name;
}

static void
bus_ibus_impl_save_global_previous_engine_name_to_config (BusIBusImpl *ibus)
{
    g_assert (IBUS_IS_CONFIG (ibus->config));

    if (ibus->use_global_engine && ibus->global_previous_engine_name) {
        GValue value = { 0 };
        g_value_init (&value, G_TYPE_STRING);
        g_value_set_static_string (&value, ibus->global_previous_engine_name);

        ibus_config_set_value (ibus->config, "general", "global_previous_engine", &value);
        g_value_unset (&value);
    }
}
