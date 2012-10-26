/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
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
#include "registry.h"

#include <gio/gio.h>
#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbusimpl.h"
#include "global.h"
#include "marshalers.h"
#include "types.h"

enum {
    CHANGED,
    LAST_SIGNAL,
};

static guint             _signals[LAST_SIGNAL] = { 0 };

struct _BusRegistry {
    IBusObject parent;

    /* instance members */

    /* a list of IBusObservedPath objects. */
    GList *observed_paths;
    /* a list of BusComponent objects that are created from component XML files (or from the cache of them). */
    GList *components;
    /* a mapping from an engine name (e.g. 'pinyin') to the corresponding IBusEngineDesc object. */
    GHashTable *engine_table;
    gboolean changed;
    /* a mapping from GFile to GFileMonitor. */
    GHashTable *monitor_table;
    guint monitor_timeout_id;
};

struct _BusRegistryClass {
    IBusObjectClass parent;

    /* class members */
};

/* functions prototype */
static void              bus_registry_destroy           (BusRegistry        *registry);
static void              bus_registry_load              (BusRegistry        *registry);
static void              bus_registry_load_in_dir       (BusRegistry        *registry,
                                                         const gchar        *dirname);
static gboolean          bus_registry_save_cache        (BusRegistry        *registry);
static gboolean          bus_registry_load_cache        (BusRegistry        *registry);
static gboolean          bus_registry_check_modification(BusRegistry        *registry);
static void              bus_registry_remove_all        (BusRegistry        *registry);

G_DEFINE_TYPE (BusRegistry, bus_registry, IBUS_TYPE_OBJECT)

static void
bus_registry_class_init (BusRegistryClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (class);

    _signals[CHANGED] =
        g_signal_new (I_("changed"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            0, /* does not associate a method in this class. the "changed" signal would be handled in other classes. */
            NULL, NULL,
            bus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_registry_destroy;
}

static void
bus_registry_init (BusRegistry *registry)
{
    GList *p;
    registry->observed_paths = NULL;
    registry->components = NULL;
    registry->engine_table = g_hash_table_new (g_str_hash, g_str_equal);
    registry->changed = FALSE;
    registry->monitor_table =
        g_hash_table_new_full (g_file_hash,
                               (GEqualFunc) g_file_equal,
                               (GDestroyNotify) g_object_unref,
                               (GDestroyNotify) g_object_unref);

    if (g_strcmp0 (g_cache, "none") == 0) {
        /* Only load registry, but not read and write cache. */
        bus_registry_load (registry);
    }
    else if (g_strcmp0 (g_cache, "refresh") == 0) {
        /* Load registry and overwrite the cache. */
        bus_registry_load (registry);
        bus_registry_save_cache (registry);
    }
    else {
        /* Load registry from cache. If the cache does not exist or
         * it is outdated, then generate it.
         */
        if (bus_registry_load_cache (registry) == FALSE ||
            bus_registry_check_modification (registry)) {
            bus_registry_remove_all (registry);
            bus_registry_load (registry);
            bus_registry_save_cache (registry);
        }
    }

    for (p = registry->components; p != NULL; p = p->next) {
        BusComponent *comp = (BusComponent *) p->data;
        GList *engines = bus_component_get_engines (comp);
        GList *p1;
        for (p1 = engines; p1 != NULL; p1 = p1->next) {
            IBusEngineDesc *desc = (IBusEngineDesc *) p1->data;
            const gchar *name = ibus_engine_desc_get_name (desc);
            if (g_hash_table_lookup (registry->engine_table, name) == NULL) {
                g_hash_table_insert (registry->engine_table,
                                     (gpointer) name,
                                     desc);
            } else {
                g_message ("Engine %s is already registered by other component",
                           name);
            }
        }
        g_list_free (engines);
    }
}

static void
bus_registry_remove_all (BusRegistry *registry)
{
    g_list_free_full (registry->observed_paths, g_object_unref);
    registry->observed_paths = NULL;

    g_list_free_full (registry->components, g_object_unref);
    registry->components = NULL;

    g_hash_table_remove_all (registry->engine_table);
    g_hash_table_remove_all (registry->monitor_table);
}

static void
bus_registry_destroy (BusRegistry *registry)
{
    bus_registry_remove_all (registry);

    g_hash_table_destroy (registry->engine_table);
    registry->engine_table = NULL;

    g_hash_table_destroy (registry->monitor_table);
    registry->monitor_table = NULL;

    if (registry->monitor_timeout_id > 0) {
        g_source_remove (registry->monitor_timeout_id);
        registry->monitor_timeout_id = 0;
    }

    IBUS_OBJECT_CLASS (bus_registry_parent_class)->destroy (IBUS_OBJECT (registry));
}

/**
 * bus_registry_load:
 *
 * Read all XML files in the PKGDATADIR (typically /usr/share/ibus/components/ *.xml) and update the registry object.
 */
static void
bus_registry_load (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    const gchar *envstr;
    GPtrArray *path;
    gchar **d, **search_path;

    path = g_ptr_array_new();

    envstr = g_getenv ("IBUS_COMPONENT_PATH");
    if (envstr) {
        gchar **dirs = g_strsplit (envstr, G_SEARCHPATH_SEPARATOR_S, 0);
        for (d = dirs; *d != NULL; d++)
            g_ptr_array_add (path, *d);
        g_free (dirs);
    } else {
        gchar *dirname;

        dirname = g_build_filename (PKGDATADIR, "component", NULL);
        g_ptr_array_add (path, dirname);

#if 0
        /* FIXME Should we support install some IME in user dir? */
        dirname = g_build_filename (g_get_user_data_dir (),
                                    "ibus", "component",
                                    NULL);
        g_ptr_array_add (path, dirname);
#endif
    }

    g_ptr_array_add (path, NULL);
    search_path = (gchar **) g_ptr_array_free (path, FALSE);
    for (d = search_path; *d != NULL; d++) {
        IBusObservedPath *observed_path = ibus_observed_path_new (*d, TRUE);

        registry->observed_paths = g_list_append (registry->observed_paths,
                                                  observed_path);

        bus_registry_load_in_dir (registry, *d);
    }
    g_strfreev (search_path);
}

#define g_string_append_indent(string, indent)  \
    {                                           \
        gint i;                                 \
        for (i = 0; i < (indent); i++) {        \
            g_string_append (string, "    ");   \
        }                                       \
    }

static gboolean
bus_registry_load_cache (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    gchar *filename;
    XMLNode *node;
    GList *p;

    filename = g_build_filename (g_get_user_cache_dir (), "ibus", "bus", "registry.xml", NULL);
    node = ibus_xml_parse_file (filename);
    g_free (filename);

    if (node == NULL) {
        return FALSE;
    }

    if (g_strcmp0 (node->name, "ibus-registry") != 0) {
        ibus_xml_free (node);
        return FALSE;
    }

    for (p = node->sub_nodes; p != NULL; p = p->next) {
        XMLNode *sub_node = (XMLNode *) p->data;

        if (g_strcmp0 (sub_node->name, "observed-paths") == 0) {
            GList *pp;
            for (pp = sub_node->sub_nodes; pp != NULL; pp = pp->next) {
                IBusObservedPath *path;
                path = ibus_observed_path_new_from_xml_node (pp->data, FALSE);
                if (path) {
                    g_object_ref_sink (path);
                    registry->observed_paths = g_list_append (registry->observed_paths, path);
                }
            }
            continue;
        }
        if (g_strcmp0 (sub_node->name, "components") == 0) {
            GList *pp;
            for (pp = sub_node->sub_nodes; pp != NULL; pp = pp->next) {
                IBusComponent *component;
                component = ibus_component_new_from_xml_node (pp->data);
                if (component) {
                    BusComponent *buscomp = bus_component_new (component,
                                                               NULL /* factory */);
                    g_object_ref_sink (buscomp);
                    registry->components =
                        g_list_append (registry->components, buscomp);
                }
            }

            continue;
        }
        g_warning ("Unknown element <%s>", sub_node->name);
    }

    ibus_xml_free (node);
    return TRUE;
}

static gboolean
bus_registry_check_modification (BusRegistry *registry)
{
    GList *p;

    for (p = registry->observed_paths; p != NULL; p = p->next) {
        if (ibus_observed_path_check_modification ((IBusObservedPath *) p->data))
            return TRUE;
    }

    for (p = registry->components; p != NULL; p = p->next) {
        if (ibus_component_check_modification (bus_component_get_component ((BusComponent *) p->data)))
            return TRUE;
    }

    return FALSE;
}

static gboolean
bus_registry_save_cache (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    gchar *cachedir;
    gchar *filename;
    GString *output;
    GList *p;
    FILE *pf;
    size_t items = 0;

    cachedir = g_build_filename (g_get_user_cache_dir (), "ibus", "bus", NULL);
    filename = g_build_filename (cachedir, "registry.xml", NULL);
    g_mkdir_with_parents (cachedir, 0775);
    pf = g_fopen (filename, "w");
    g_free (filename);
    g_free (cachedir);

    if (pf == NULL) {
        g_warning ("create registry.xml failed");
        return FALSE;
    }

    output = g_string_new ("");
    g_string_append (output, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    g_string_append (output, "<!-- \n"
                             "    This file was generated by ibus-daemon. Please don't modify it.\n"
                             "    -->\n");
    g_string_append (output, "<ibus-registry>\n");

    if (registry->observed_paths) {
        g_string_append_indent (output, 1);
        g_string_append (output, "<observed-paths>\n");
        for (p = registry->observed_paths; p != NULL; p = p->next) {
            ibus_observed_path_output ((IBusObservedPath *) p->data,
                                      output, 2);
        }
        g_string_append_indent (output, 1);
        g_string_append (output, "</observed-paths>\n");
    }

    if (registry->components) {
        g_string_append_indent (output, 1);
        g_string_append (output, "<components>\n");
        for (p = registry->components; p != NULL; p = p->next) {
            ibus_component_output (bus_component_get_component ((BusComponent *) p->data),
                                   output, 2);
        }
        g_string_append_indent (output, 1);
        g_string_append (output, "</components>\n");
    }

    g_string_append (output, "</ibus-registry>\n");
    items = fwrite (output->str, output->len, 1, pf);
    g_string_free (output, TRUE);
    fclose (pf);
    return (items == 1 ? TRUE : FALSE);
}

/**
 * bus_registry_load_in_dir:
 *
 * Read all XML files in dirname, create a BusComponent object for each file, and add the component objects to the registry.
 */
static void
bus_registry_load_in_dir (BusRegistry *registry,
                          const gchar *dirname)
{
    g_assert (BUS_IS_REGISTRY (registry));
    g_assert (dirname);

    GError *error = NULL;
    GDir *dir;
    const gchar *filename;

    dir = g_dir_open (dirname, 0, &error);

    if (dir == NULL) {
        g_warning ("Unable open directory %s : %s", dirname, error->message);
        g_error_free (error);
        return;
    }

    while ((filename = g_dir_read_name (dir)) != NULL) {
        glong size;
        gchar *path;
        IBusComponent *component;

        size = g_utf8_strlen (filename, -1);
        if (g_strcmp0 (MAX (filename, filename + size - 4), ".xml") != 0)
            continue;

        path = g_build_filename (dirname, filename, NULL);
        component = ibus_component_new_from_file (path);
        if (component != NULL) {
            BusComponent *buscomp = bus_component_new (component,
                                                       NULL /* factory */);
            g_object_ref_sink (buscomp);
            registry->components =
                g_list_append (registry->components, buscomp);
        }

        g_free (path);
    }

    g_dir_close (dir);
}


BusRegistry *
bus_registry_new (void)
{
    BusRegistry *registry;
    registry = (BusRegistry *) g_object_new (BUS_TYPE_REGISTRY, NULL);
    return registry;
}

static gint
bus_register_component_is_name_cb (BusComponent *component,
                                   const gchar  *name)
{
    g_assert (BUS_IS_COMPONENT (component));
    g_assert (name);

    return g_strcmp0 (bus_component_get_name (component), name);
}

BusComponent *
bus_registry_lookup_component_by_name (BusRegistry *registry,
                                       const gchar *name)
{
    g_assert (BUS_IS_REGISTRY (registry));
    g_assert (name);

    GList *p;
    p = g_list_find_custom (registry->components,
                            name,
                            (GCompareFunc) bus_register_component_is_name_cb);
    if (p) {
        return (BusComponent *) p->data;
    }
    else {
        return NULL;
    }
}

GList *
bus_registry_get_components (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    return g_list_copy (registry->components);
}

GList *
bus_registry_get_engines (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    return g_hash_table_get_values (registry->engine_table);
}

GList *
bus_registry_get_engines_by_language (BusRegistry *registry,
                                      const gchar *language)
{
    g_assert (BUS_IS_REGISTRY (registry));
    g_assert (language);

    gint n;
    GList *p1, *p2, *engines;

    n = strlen (language);

    p1 = bus_registry_get_engines (registry);

    engines = NULL;

    for (p2 = p1; p2 != NULL; p2 = p2->next) {
        IBusEngineDesc *desc = (IBusEngineDesc *) p2->data;
        if (strncmp (ibus_engine_desc_get_language (desc), language, n) == 0) {
            engines = g_list_append (engines, desc);
        }
    }

    g_list_free (p1);
    return engines;
}

IBusEngineDesc *
bus_registry_find_engine_by_name (BusRegistry *registry,
                                  const gchar *name)
{
    g_assert (BUS_IS_REGISTRY (registry));
    g_assert (name);

    return (IBusEngineDesc *) g_hash_table_lookup (registry->engine_table, name);
}

void
bus_registry_stop_all_components (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    g_list_foreach (registry->components, (GFunc) bus_component_stop, NULL);

}

static gboolean
_monitor_timeout_cb (BusRegistry *registry)
{
    g_hash_table_remove_all (registry->monitor_table);
    registry->changed = TRUE;
    g_signal_emit (registry, _signals[CHANGED], 0);
    registry->monitor_timeout_id = 0;
    return FALSE;
}

static void
_monitor_changed_cb (GFileMonitor     *monitor,
                     GFile            *file,
                     GFile            *other_file,
                     GFileMonitorEvent event_type,
                     BusRegistry      *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    if (event_type != G_FILE_MONITOR_EVENT_CHANGED &&
        event_type != G_FILE_MONITOR_EVENT_DELETED &&
        event_type != G_FILE_MONITOR_EVENT_CREATED &&
        event_type != G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED)
        return;

    /* Merge successive file changes into one, with a low priority
       timeout handler. */
    if (registry->monitor_timeout_id > 0)
        return;

    registry->monitor_timeout_id =
        g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,
                            5000,
                            (GSourceFunc) _monitor_timeout_cb,
                            g_object_ref (registry),
                            (GDestroyNotify) g_object_unref);
}

/**
 * bus_registry_start_monitor_changes:
 *
 * Start the monitor thread.
 */
void
bus_registry_start_monitor_changes (BusRegistry *registry)
{
    GList *observed_paths, *p;

    g_assert (BUS_IS_REGISTRY (registry));

    g_hash_table_remove_all (registry->monitor_table);

    observed_paths = g_list_copy (registry->observed_paths);
    for (p = registry->components; p != NULL; p = p->next) {
        BusComponent *buscomp = (BusComponent *) p->data;
        IBusComponent *component = bus_component_get_component (buscomp);
        GList *component_observed_paths =
            ibus_component_get_observed_paths (component);
        observed_paths = g_list_concat (observed_paths,
                                        component_observed_paths);
    }

    for (p = observed_paths; p != NULL; p = p->next) {
        IBusObservedPath *path = (IBusObservedPath *) p->data;
        GFile *file = g_file_new_for_path (path->path);
        if (g_hash_table_lookup (registry->monitor_table,file) == NULL) {
            GFileMonitor *monitor;
            GError *error;

            error = NULL;
            monitor = g_file_monitor (file,
                                      G_FILE_MONITOR_NONE,
                                      NULL,
                                      &error);

            if (monitor != NULL) {
                g_signal_connect (monitor, "changed",
                                  G_CALLBACK (_monitor_changed_cb),
                                  registry);

                g_hash_table_replace (registry->monitor_table,
                                      g_object_ref (file),
                                      monitor);
            } else {
                g_warning ("Can't monitor directory %s: %s",
                           path->path,
                           error->message);
                g_error_free (error);
            }
        }
        g_object_unref (file);
    }
    g_list_free (observed_paths);
}

gboolean
bus_registry_is_changed (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));
    return (registry->changed != 0);
}

void
bus_registry_name_owner_changed (BusRegistry *registry,
                                 const gchar *name,
                                 const gchar *old_name,
                                 const gchar *new_name)
{
    g_assert (BUS_IS_REGISTRY (registry));
    g_assert (name);
    g_assert (old_name);
    g_assert (new_name);

    BusComponent *component;
    BusFactoryProxy *factory;

    component = bus_registry_lookup_component_by_name (registry, name);

    if (component == NULL) {
        /* name is a unique name, or a well-known name we don't know. */
        return;
    }

    if (g_strcmp0 (old_name, "") != 0) {
        /* the component is stopped. */
        factory = bus_component_get_factory (component);

        if (factory != NULL) {
            ibus_proxy_destroy ((IBusProxy *) factory);
        }
    }

    if (g_strcmp0 (new_name, "") != 0) {
        /* the component is started. */
        BusConnection *connection =
                bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS,
                                                      new_name);
        if (connection == NULL)
            return;

        factory = bus_factory_proxy_new (connection);
        if (factory == NULL)
            return;
        bus_component_set_factory (component, factory);
        g_object_unref (factory);
    }
}
