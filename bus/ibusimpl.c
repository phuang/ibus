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

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <locale.h>
#include <strings.h>
#include "ibusimpl.h"
#include "dbusimpl.h"
#include "server.h"
#include "connection.h"
#include "registry.h"
#include "factoryproxy.h"
#include "panelproxy.h"
#include "inputcontext.h"


enum {
    LAST_SIGNAL,
};

enum {
    PROP_0,
};

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_ibus_impl_class_init        (BusIBusImplClass   *klass);
static void     bus_ibus_impl_init              (BusIBusImpl        *ibus);
static void     bus_ibus_impl_destroy           (BusIBusImpl        *ibus);
static gboolean bus_ibus_impl_ibus_message      (BusIBusImpl        *ibus,
                                                 BusConnection      *connection,
                                                 IBusMessage        *message);
static void     bus_ibus_impl_add_factory       (BusIBusImpl        *ibus,
                                                 BusFactoryProxy    *factory);
static void     bus_ibus_impl_set_trigger       (BusIBusImpl        *ibus,
                                                 GValue             *value);
static void     bus_ibus_impl_set_preload_engines
                                                (BusIBusImpl        *ibus,
                                                 GValue             *value);
static void     _factory_destroy_cb             (BusFactoryProxy      *factory,
                                                 BusIBusImpl          *ibus);

static IBusServiceClass  *parent_class = NULL;

extern gboolean g_verbose;

GType
bus_ibus_impl_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusIBusImplClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_ibus_impl_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusIBusImpl),
        0,
        (GInstanceInitFunc) bus_ibus_impl_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERVICE,
                    "BusIBusImpl",
                    &type_info,
                    (GTypeFlags) 0);
    }
    return type;
}

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

    parent_class = (IBusServiceClass *) g_type_class_peek_parent (klass);

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
bus_ibus_impl_set_next_engine (BusIBusImpl *ibus,
                               GValue      *value)
{
    GQuark hotkey = g_quark_from_static_string ("next-engine");
    bus_ibus_impl_set_hotkey (ibus, hotkey, value);
}

static void
bus_ibus_impl_set_prev_engine (BusIBusImpl *ibus,
                               GValue      *value)
{
    GQuark hotkey = g_quark_from_static_string ("prev-engine");
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
    GList *engines, *l;
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

    engines = g_list_sort (engines, (GCompareFunc) _engine_desc_cmp);
    g_free (lang);

    g_value_init (&value, G_TYPE_VALUE_ARRAY);
    array = g_value_array_new (5);
    for (l = engines; l != NULL; l = l->next) {
        IBusEngineDesc *desc;
        GValue name = { 0 };
        desc = (IBusEngineDesc *)l->data;
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
        { "general/hotkey", "next_engine", bus_ibus_impl_set_next_engine },
        { "general/hotkey", "prev_engine", bus_ibus_impl_set_prev_engine },
        { "general", "preload_engines", bus_ibus_impl_set_preload_engines },
        { NULL, NULL, NULL },
    };

    for (i = 0; entries[i].section != NULL; i++) {
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
        { "general/hotkey", "trigger",     bus_ibus_impl_set_trigger },
        { "general/hotkey", "next_engine", bus_ibus_impl_set_next_engine },
        { "general/hotkey", "prev_engine", bus_ibus_impl_set_prev_engine },
        { "general", "preload_engines",    bus_ibus_impl_set_preload_engines },
        { NULL, NULL, NULL },
    };

    for (i = 0; entries[i].section != NULL; i++) {
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

    ibus->config = NULL;
    g_object_unref (config);
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
                ibus->panel = NULL;
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
        }
    }
    else if (g_strcmp0 (name, IBUS_SERVICE_CONFIG) == 0) {
        if (g_strcmp0 (new_name, "") != 0) {
            BusConnection *connection;

            if (ibus->config != NULL) {
                ibus_object_destroy (IBUS_OBJECT (ibus->config));
                ibus->config = NULL;
            }

            connection = bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS, new_name);
            g_return_if_fail (connection != NULL);

            ibus->config = g_object_new (IBUS_TYPE_CONFIG,
                                         "name", NULL,
                                         "path", IBUS_PATH_CONFIG,
                                         "connection", connection,
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

    factory = bus_registry_name_owner_changed (ibus->registry, name, old_name, new_name);

    if (factory) {
        bus_ibus_impl_add_factory (ibus, factory);
        g_object_unref (factory);
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

    ibus->registry = bus_registry_new ();
    ibus->engine_list = NULL;
    ibus->register_engine_list = NULL;
    ibus->contexts = NULL;
    ibus->focused_context = NULL;
    ibus->panel = NULL;
    ibus->config = NULL;

    ibus->hotkey_profile = ibus_hotkey_profile_new ();
    ibus->keymap = ibus_keymap_new ("us");

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

    bus_server_quit (BUS_DEFAULT_SERVER);
    ibus_object_destroy ((IBusObject *) BUS_DEFAULT_SERVER);
    IBUS_OBJECT_CLASS(parent_class)->destroy (IBUS_OBJECT (ibus));
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
        "  <interface name=\"org.freedesktop.DBus\">\n"
        "    <method name=\"RequestName\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"in\" type=\"u\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <signal name=\"NameOwnerChanged\">\n"
        "      <arg type=\"s\"/>\n"
        "      <arg type=\"s\"/>\n"
        "      <arg type=\"s\"/>\n"
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
    ibus_message_append_args (message,
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

static void
_context_request_engine_cb (BusInputContext *context,
                            gchar           *engine_name,
                            BusIBusImpl     *ibus)
{
    IBusEngineDesc *engine_desc = NULL;
    BusEngineProxy *engine;

    if (engine_name == NULL || engine_name[0] == '\0') {
        /* request default engine */
        if (ibus->register_engine_list) {
            engine_desc = (IBusEngineDesc *)ibus->register_engine_list->data;
        }
        else if (ibus->engine_list) {
            engine_desc = (IBusEngineDesc *)ibus->engine_list->data;
        }
    }
    else {
        /* request engine by name */
        GList *p;
        gboolean found = FALSE;

        /* find engine in registered engine list */
        for (p = ibus->register_engine_list; p != NULL; p = p->next) {
            engine_desc = (IBusEngineDesc *)p->data;
            if (g_strcmp0 (engine_desc->name, engine_name) == 0) {
                found = TRUE;
                break;
            }
        }

        if (!found) {
            /* find engine in preload engine list */
            for (p = ibus->engine_list; p != NULL; p = p->next) {
                 engine_desc = (IBusEngineDesc *)p->data;
                if (g_strcmp0 (engine_desc->name, engine_name) == 0) {
                    found = TRUE;
                    break;
                }
            }
        }

        if (!found) {
            engine_desc = NULL;
        }
    }

    if (engine_desc == NULL) {
        return;
    }

    engine = bus_ibus_impl_create_engine (engine_desc);

    if (engine == NULL) {
        return;
    }

    bus_input_context_set_engine (context, engine);
    g_object_unref (engine);
}

static void
_context_request_next_engine_cb (BusInputContext *context,
                                 BusIBusImpl     *ibus)
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
        bus_input_context_set_engine (context, engine);
        g_object_unref (engine);
    }
}

static void
_context_request_prev_engine_cb (BusInputContext *context,
                                 BusIBusImpl     *ibus)
{

}

static void
_context_focus_out_cb (BusInputContext    *context,
                       BusIBusImpl        *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (ibus->focused_context != context)
        return;

    if (ibus->panel != NULL) {
        bus_panel_proxy_focus_out (ibus->panel, context);
    }

    if (context) {
        g_object_unref (context);
        ibus->focused_context = NULL;
    }
}

static void
_context_focus_in_cb (BusInputContext *context,
                      BusIBusImpl     *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (ibus->focused_context) {
        /* focus out context */
        bus_input_context_focus_out (ibus->focused_context);
        g_assert (ibus->focused_context == NULL);
    }

    g_object_ref (context);
    ibus->focused_context = context;

    if (ibus->panel != NULL) {
        bus_panel_proxy_focus_in (ibus->panel, ibus->focused_context);
    }

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
    ibus->contexts = g_list_append (ibus->contexts, context);

    static const struct {
        gchar *name;
        GCallback callback;
    } signals [] = {
        { "request-engine",      G_CALLBACK (_context_request_engine_cb) },
        { "request-next-engine", G_CALLBACK (_context_request_next_engine_cb) },
        { "request-prev-engine", G_CALLBACK (_context_request_prev_engine_cb) },
        { "focus-in",       G_CALLBACK (_context_focus_in_cb) },
        { "focus-out",      G_CALLBACK (_context_focus_out_cb) },
        { "destroy",        G_CALLBACK (_context_destroy_cb) },
        { NULL, NULL }
    };

    for (i = 0; signals[i].name != NULL; i++) {
        g_signal_connect (context,
                          signals[i].name,
                          signals[i].callback,
                          ibus);
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

    g_object_ref (factory);
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

    factory = bus_factory_proxy_new (component, connection);

    if (factory == NULL) {
        reply = ibus_message_new_error (message,
                                        DBUS_ERROR_FAILED,
                                        "Can not create factory");
        return reply;
    }

    bus_ibus_impl_add_factory (ibus, factory);
    g_object_unref (factory);

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
        if (!g_file_test (exe, G_FILE_TEST_EXISTS)) {
            g_free (exe);
            exe = g_argv[0];
        }

        /* close all fds except stdin, stdout, stderr */
        for (fd = 3; fd <= sysconf (_SC_OPEN_MAX); fd ++) {
            close (fd);
        }

        execv (exe, g_argv);
        g_warning ("execv %s failed!", g_argv[0]);
        exit (-1);
    }

    /* should not reach here */
    g_assert_not_reached ();

    return NULL;
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
                               "Introspect", _ibus_introspect },
        /* IBus interface */
        { IBUS_INTERFACE_IBUS, "GetAddress",            _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "CreateInputContext",    _ibus_create_input_context },
        { IBUS_INTERFACE_IBUS, "RegisterComponent",     _ibus_register_component },
        { IBUS_INTERFACE_IBUS, "ListEngines",           _ibus_list_engines },
        { IBUS_INTERFACE_IBUS, "ListActiveEngines",     _ibus_list_active_engines },
        { IBUS_INTERFACE_IBUS, "Exit",                  _ibus_exit },
        { NULL, NULL, NULL }
    };

    ibus_message_set_sender (message, bus_connection_get_unique_name (connection));
    ibus_message_set_destination (message, DBUS_SERVICE_DBUS);

    for (i = 0; handlers[i].interface != NULL; i++) {
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

    return parent_class->ibus_message ((IBusService *) ibus,
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

