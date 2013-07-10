/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2013 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2013 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2013 Red Hat, Inc.
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
#include <gio/gio.h>
#include <glib/gstdio.h>
#include <string.h>

#include "ibusinternal.h"
#include "ibusmarshalers.h"
#include "ibusregistry.h"

enum {
    CHANGED,
    LAST_SIGNAL,
};

static guint             _signals[LAST_SIGNAL] = { 0 };

struct _IBusRegistryPrivate {
    /* a list of IBusObservedPath objects. */
    GList *observed_paths;

    /* a list of IBusComponent objects that are created from component XML
     * files (or from the cache of them). */
    GList *components;

    gboolean changed;

    /* a mapping from GFile to GFileMonitor. */
    GHashTable *monitor_table;

    guint monitor_timeout_id;
};

#define IBUS_REGISTRY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_REGISTRY, IBusRegistryPrivate))

/* functions prototype */
static void     ibus_registry_destroy        (IBusRegistry           *registry);
static void     ibus_registry_remove_all     (IBusRegistry           *registry);

G_DEFINE_TYPE (IBusRegistry, ibus_registry, IBUS_TYPE_OBJECT)

static void
ibus_registry_class_init (IBusRegistryClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (class);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_registry_destroy;

    g_type_class_add_private (class, sizeof (IBusRegistryPrivate));

    /* install signals */
    /**
     * IBusRegistry::changed:
     * @registry: An #IBusRegistry.
     *
     * Emitted when any observed paths are changed.
     * A method is not associated in this class. the "changed"
     * signal would be handled in other classes.
     *
     * See also: ibus_registry_start_monitor_changes().
     */
    _signals[CHANGED] =
        g_signal_new (I_("changed"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            _ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);
}

static void
ibus_registry_init (IBusRegistry *registry)
{
    registry->priv = IBUS_REGISTRY_GET_PRIVATE (registry);

    registry->priv->observed_paths = NULL;
    registry->priv->components = NULL;
    registry->priv->changed = FALSE;
    registry->priv->monitor_table =
        g_hash_table_new_full (g_file_hash,
                               (GEqualFunc) g_file_equal,
                               (GDestroyNotify) g_object_unref,
                               (GDestroyNotify) g_object_unref);
}

static void
ibus_registry_destroy (IBusRegistry *registry)
{
    ibus_registry_remove_all (registry);

    g_hash_table_destroy (registry->priv->monitor_table);
    registry->priv->monitor_table = NULL;

    if (registry->priv->monitor_timeout_id > 0) {
        g_source_remove (registry->priv->monitor_timeout_id);
        registry->priv->monitor_timeout_id = 0;
    }

    IBUS_OBJECT_CLASS (ibus_registry_parent_class)->
            destroy (IBUS_OBJECT (registry));
}

/**
 * ibus_registry_remove_all:
 *
 * Remove the loaded registry.
 */
static void
ibus_registry_remove_all (IBusRegistry *registry)
{
    g_assert (IBUS_IS_REGISTRY (registry));

    g_list_free_full (registry->priv->observed_paths, g_object_unref);
    registry->priv->observed_paths = NULL;

    g_list_free_full (registry->priv->components, g_object_unref);
    registry->priv->components = NULL;
}

void
ibus_registry_load (IBusRegistry *registry)
{
    const gchar *envstr;
    GPtrArray *path;
    gchar **d, **search_path;

    g_assert (IBUS_IS_REGISTRY (registry));

    path = g_ptr_array_new();

    envstr = g_getenv ("IBUS_COMPONENT_PATH");
    if (envstr) {
        gchar **dirs = g_strsplit (envstr, G_SEARCHPATH_SEPARATOR_S, 0);
        for (d = dirs; *d != NULL; d++)
            g_ptr_array_add (path, *d);
        g_free (dirs);
    } else {
        gchar *dirname;

        dirname = g_build_filename (IBUS_DATA_DIR, "component", NULL);
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
        ibus_registry_load_in_dir (registry, *d);
    }
    g_strfreev (search_path);
}

gboolean
ibus_registry_load_cache (IBusRegistry *registry, gboolean is_user)
{
    gchar *filename;
    gboolean retval;

    g_assert (IBUS_IS_REGISTRY (registry));

    if (is_user) {
        filename = g_build_filename (g_get_user_cache_dir (),
                                     "ibus", "bus", "registry.xml", NULL);
    } else {
        filename = g_build_filename (IBUS_CACHE_DIR,
                                     "bus", "registry.xml", NULL);
    }

    retval = ibus_registry_load_cache_file (registry, filename);
    g_free (filename);

    return retval;
}

gboolean
ibus_registry_load_cache_file (IBusRegistry *registry, const gchar *filename)
{
    XMLNode *node;
    GList *p;

    g_assert (IBUS_IS_REGISTRY (registry));
    g_assert (filename != NULL);

    node = ibus_xml_parse_file (filename);

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
                    registry->priv->observed_paths =
                            g_list_append (registry->priv->observed_paths,
                                           path);
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
                    g_object_ref_sink (component);
                    registry->priv->components =
                        g_list_append (registry->priv->components, component);
                }
            }

            continue;
        }
        g_warning ("Unknown element <%s>", sub_node->name);
    }

    ibus_xml_free (node);
    return TRUE;
}

gboolean
ibus_registry_check_modification (IBusRegistry *registry)
{
    GList *p;

    g_assert (IBUS_IS_REGISTRY (registry));

    for (p = registry->priv->observed_paths; p != NULL; p = p->next) {
        if (ibus_observed_path_check_modification (
                    (IBusObservedPath *) p->data))
            return TRUE;
    }

    for (p = registry->priv->components; p != NULL; p = p->next) {
        if (ibus_component_check_modification ((IBusComponent *) p->data))
            return TRUE;
    }

    return FALSE;
}

gboolean
ibus_registry_save_cache (IBusRegistry *registry, gboolean is_user)
{
    gchar *filename;
    gboolean retval;

    g_assert (IBUS_IS_REGISTRY (registry));

    if (is_user) {
        filename = g_build_filename (g_get_user_cache_dir (),
                                     "ibus", "bus", "registry.xml", NULL);
    } else {
        filename = g_build_filename (IBUS_CACHE_DIR,
                                     "bus", "registry.xml", NULL);
    }

    retval = ibus_registry_save_cache_file (registry, filename);
    g_free (filename);

    return retval;
}

gboolean
ibus_registry_save_cache_file (IBusRegistry *registry, const gchar *filename)
{
    gchar *cachedir;
    const gchar *user_cachedir;
    gboolean is_user = TRUE;
    GString *output;
    FILE *pf;
    size_t items = 0;

    g_assert (IBUS_IS_REGISTRY (registry));
    g_assert (filename != NULL);

    cachedir = g_path_get_dirname (filename);
    g_mkdir_with_parents (cachedir, 0775);
    g_free (cachedir);
    pf = g_fopen (filename, "w");

    if (pf == NULL) {
        g_warning ("create %s failed", filename);
        return FALSE;
    }

    output = g_string_new ("");

    ibus_registry_output (registry, output, 1);

    items = fwrite (output->str, output->len, 1, pf);
    g_string_free (output, TRUE);
    fclose (pf);

    user_cachedir = g_get_user_cache_dir ();
    is_user = (strncmp (user_cachedir, filename, strlen (user_cachedir)) == 0);

    if (!is_user) {
        g_chmod (filename, 0644);
    }

    return (items == 1 ? TRUE : FALSE);
}

#define g_string_append_indent(string, indent)  \
    {                                           \
        gint i;                                 \
        for (i = 0; i < (indent); i++) {        \
            g_string_append (string, "    ");   \
        }                                       \
    }

void
ibus_registry_output (IBusRegistry *registry, GString *output, int indent)
{
    GList *p;

    g_assert (IBUS_IS_REGISTRY (registry));
    g_return_if_fail (output != NULL);

    g_string_append (output, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    g_string_append (output, "<!-- \n"
                             "    This file was generated by ibus-daemon. "
                             "Please don't modify it.\n"
                             "    -->\n");
    g_string_append (output, "<ibus-registry>\n");

    if (registry->priv->observed_paths) {
        g_string_append_indent (output, indent);
        g_string_append (output, "<observed-paths>\n");
        for (p = registry->priv->observed_paths; p != NULL; p = p->next) {
            ibus_observed_path_output ((IBusObservedPath *) p->data,
                                      output, indent * 2);
        }
        g_string_append_indent (output, indent);
        g_string_append (output, "</observed-paths>\n");
    }

    if (registry->priv->components) {
        g_string_append_indent (output, indent);
        g_string_append (output, "<components>\n");
        for (p = registry->priv->components; p != NULL; p = p->next) {
            ibus_component_output ((IBusComponent *) p->data,
                                   output, indent * 2);
        }
        g_string_append_indent (output, indent);
        g_string_append (output, "</components>\n");
    }

    g_string_append (output, "</ibus-registry>\n");
}

void
ibus_registry_load_in_dir (IBusRegistry *registry,
                           const gchar  *dirname)
{
    GError *error = NULL;
    GDir *dir;
    IBusObservedPath *observed_path = NULL;
    const gchar *filename;

    g_assert (IBUS_IS_REGISTRY (registry));
    g_assert (dirname);

    dir = g_dir_open (dirname, 0, &error);

    if (dir == NULL) {
        g_warning ("Unable open directory %s : %s", dirname, error->message);
        g_error_free (error);
        return;
    }

    observed_path = ibus_observed_path_new (dirname, TRUE);

    registry->priv->observed_paths =
            g_list_append (registry->priv->observed_paths,
                           observed_path);

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
            g_object_ref_sink (component);
            registry->priv->components =
                g_list_append (registry->priv->components, component);
        }

        g_free (path);
    }

    g_dir_close (dir);
}


IBusRegistry *
ibus_registry_new (void)
{
    IBusRegistry *registry;
    registry = (IBusRegistry *) g_object_new (IBUS_TYPE_REGISTRY, NULL);
    return registry;
}

GList *
ibus_registry_get_components (IBusRegistry *registry)
{
    g_assert (IBUS_IS_REGISTRY (registry));

    return g_list_copy (registry->priv->components);
}

GList *
ibus_registry_get_observed_paths (IBusRegistry *registry)
{
    g_assert (IBUS_IS_REGISTRY (registry));

    return g_list_copy (registry->priv->observed_paths);
}

static gboolean
_monitor_timeout_cb (IBusRegistry *registry)
{
    g_hash_table_remove_all (registry->priv->monitor_table);
    registry->priv->changed = TRUE;
    g_signal_emit (registry, _signals[CHANGED], 0);
    registry->priv->monitor_timeout_id = 0;
    return FALSE;
}

static void
_monitor_changed_cb (GFileMonitor     *monitor,
                     GFile            *file,
                     GFile            *other_file,
                     GFileMonitorEvent event_type,
                     IBusRegistry     *registry)
{
    g_assert (IBUS_IS_REGISTRY (registry));

    if (event_type != G_FILE_MONITOR_EVENT_CHANGED &&
        event_type != G_FILE_MONITOR_EVENT_DELETED &&
        event_type != G_FILE_MONITOR_EVENT_CREATED &&
        event_type != G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED)
        return;

    /* Merge successive file changes into one, with a low priority
       timeout handler. */
    if (registry->priv->monitor_timeout_id > 0)
        return;

    registry->priv->monitor_timeout_id =
        g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,
                            5000,
                            (GSourceFunc) _monitor_timeout_cb,
                            g_object_ref (registry),
                            (GDestroyNotify) g_object_unref);
}

void
ibus_registry_start_monitor_changes (IBusRegistry *registry)
{
    GList *observed_paths, *p;

    g_assert (IBUS_IS_REGISTRY (registry));

    g_hash_table_remove_all (registry->priv->monitor_table);

    observed_paths = g_list_copy (registry->priv->observed_paths);
    for (p = registry->priv->components; p != NULL; p = p->next) {
        IBusComponent *component = (IBusComponent *) p->data;
        GList *component_observed_paths =
            ibus_component_get_observed_paths (component);
        observed_paths = g_list_concat (observed_paths,
                                        component_observed_paths);
    }

    for (p = observed_paths; p != NULL; p = p->next) {
        IBusObservedPath *path = (IBusObservedPath *) p->data;
        GFile *file = g_file_new_for_path (path->path);
        if (g_hash_table_lookup (registry->priv->monitor_table, file) == NULL) {
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

                g_hash_table_replace (registry->priv->monitor_table,
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
