/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2008-2009 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2009 Red Hat, Inc.
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
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include <ibusinternal.h>
#include "registry.h"
#include "option.h"

enum {
    CHANGED,
    LAST_SIGNAL,
};

static guint             _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void              bus_registry_class_init        (BusRegistryClass   *klass);
static void              bus_registry_init              (BusRegistry        *registry);
static void              bus_registry_destroy           (BusRegistry        *registry);
static void              bus_registry_load              (BusRegistry        *registry);
static void              bus_registry_load_in_dir       (BusRegistry        *registry,
                                                         const gchar        *dirname);
static gboolean          bus_registry_save_cache        (BusRegistry        *registry);
static gboolean          bus_registry_load_cache        (BusRegistry        *registry);
static gboolean          bus_registry_check_modification(BusRegistry        *registry);
static void              bus_registry_remove_all        (BusRegistry        *registry);

static IBusObjectClass  *parent_class = NULL;

GType
bus_registry_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusRegistryClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_registry_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusRegistry),
        0,
        (GInstanceInitFunc) bus_registry_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "BusRegistry",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}


static void
bus_registry_class_init (BusRegistryClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    _signals[CHANGED] =
        g_signal_new (I_("changed"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
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
    registry->timeout_id = 0;
    registry->changed = FALSE;

    if (g_rescan ||
        bus_registry_load_cache (registry) == FALSE ||
        bus_registry_check_modification (registry)) {
        bus_registry_remove_all (registry);
        bus_registry_load (registry);
        bus_registry_save_cache (registry);
    }

    for (p = registry->components; p != NULL; p = p->next) {
        IBusComponent *comp = (IBusComponent *)p->data;
        GList *p1;

        for (p1 = comp->engines; p1 != NULL; p1 = p1->next) {
            IBusEngineDesc *desc = (IBusEngineDesc *)p1->data;
            g_hash_table_insert (registry->engine_table, desc->name, desc);
            g_object_set_data ((GObject *)desc, "component", comp);
        }
    }
}

static void
bus_registry_remove_all (BusRegistry *registry)
{
    g_list_foreach (registry->observed_paths, (GFunc) g_object_unref, NULL);
    g_list_free (registry->observed_paths);
    registry->observed_paths = NULL;

    g_list_foreach (registry->components, (GFunc) g_object_unref, NULL);
    g_list_free (registry->components);
    registry->components = NULL;

    g_hash_table_remove_all (registry->engine_table);
}

static void
bus_registry_destroy (BusRegistry *registry)
{
    if (bus_registry_is_monitor_changes (registry))
        bus_registry_set_monitor_changes (registry, FALSE);

    bus_registry_remove_all (registry);

    g_hash_table_destroy (registry->engine_table);
    registry->engine_table = NULL;

    IBUS_OBJECT_CLASS (parent_class)->destroy (IBUS_OBJECT (registry));
}


static void
bus_registry_load (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    gchar *dirname;
    gchar *homedir;
    IBusObservedPath *path;

    dirname = g_build_filename (PKGDATADIR, "component", NULL);

    path = ibus_observed_path_new (dirname, TRUE);
    registry->observed_paths = g_list_append (registry->observed_paths, path);

    bus_registry_load_in_dir (registry, dirname);

    g_free (dirname);

    dirname = g_build_filename (g_get_user_data_dir (), "ibus", "component", NULL);

    path = ibus_observed_path_new (dirname, TRUE);
    registry->observed_paths = g_list_append (registry->observed_paths, path);

    if (g_file_test(dirname, G_FILE_TEST_EXISTS)) {
        bus_registry_load_in_dir (registry, dirname);
    }

    g_free (dirname);
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
                    registry->components = g_list_append (registry->components, component);
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
        if (ibus_observed_path_check_modification ((IBusObservedPath *)p->data))
            return TRUE;
    }

    for (p = registry->components; p != NULL; p = p->next) {
        if (ibus_component_check_modification ((IBusComponent *)p->data))
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
            ibus_observed_path_output ((IBusObservedPath *)p->data,
                                      output, 2);
        }
        g_string_append_indent (output, 1);
        g_string_append (output, "</observed-paths>\n");
    }

    if (registry->components) {
        g_string_append_indent (output, 1);
        g_string_append (output, "<components>\n");
        for (p = registry->components; p != NULL; p = p->next) {
            ibus_component_output ((IBusComponent *)p->data,
                                      output, 2);
        }
        g_string_append_indent (output, 1);
        g_string_append (output, "</components>\n");
    }

    g_string_append (output, "</ibus-registry>\n");
    fwrite (output->str, output->len, 1, pf);
    g_string_free (output, TRUE);
    fclose (pf);
    return TRUE;
}

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
        if (g_strcmp0 (MAX (filename, filename + size -4), ".xml" ) != 0)
            continue;

        path = g_build_filename (dirname, filename, NULL);
        component = ibus_component_new_from_file (path);
        registry->components = g_list_append (registry->components, component);

        g_free (path);
    }

    g_dir_close (dir);
}


BusRegistry *
bus_registry_new (void)
{
    BusRegistry *registry;
    registry = (BusRegistry *)g_object_new (BUS_TYPE_REGISTRY, NULL);
    return registry;
}

static gint
_component_is_name (IBusComponent *component,
                    const gchar   *name)
{
    g_assert (IBUS_IS_COMPONENT (component));
    g_assert (name);

    return g_strcmp0 (component->name, name);
}

IBusComponent *
bus_registry_lookup_component_by_name (BusRegistry *registry,
                                       const gchar *name)
{
    g_assert (BUS_IS_REGISTRY (registry));
    g_assert (name);

    GList *p;
    p = g_list_find_custom (registry->components,
                            name,
                            (GCompareFunc)_component_is_name);
    if (p) {
        return (IBusComponent *)p->data;
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
        if (strncmp (desc->language, language, n) == 0) {
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

    g_list_foreach (registry->components, (GFunc) ibus_component_stop, NULL);

}

static gboolean
_check_changes_cb (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    if (registry->changed)
        return TRUE;

    if (bus_registry_check_modification (registry)) {
        registry->changed = TRUE;
        g_signal_emit (registry, _signals[CHANGED], 0);
        return TRUE;
    }
    return TRUE;
}

void
bus_registry_set_monitor_changes (BusRegistry *registry,
                                  gboolean     enable)
{
    g_assert (BUS_IS_REGISTRY (registry));

    if (enable) {
        extern gint g_monitor_timeout;
        g_return_if_fail (!bus_registry_is_monitor_changes (registry));
        registry->timeout_id = g_timeout_add_seconds (g_monitor_timeout, (GSourceFunc) _check_changes_cb, registry);
    }
    else {
        g_return_if_fail (bus_registry_is_monitor_changes (registry));
        g_warn_if_fail (g_source_remove (registry->timeout_id));
        registry->timeout_id = 0;
    }
}

gboolean
bus_registry_is_monitor_changes (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));
    return (registry->timeout_id != 0);
}

gboolean
bus_registry_is_changed (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));
    return (registry->changed != 0);
}

BusFactoryProxy *
bus_registry_name_owner_changed (BusRegistry *registry,
                                 const gchar *name,
                                 const gchar *old_name,
                                 const gchar *new_name)
{
    g_assert (BUS_IS_REGISTRY (registry));
    g_assert (name);
    g_assert (old_name);
    g_assert (new_name);

    IBusComponent *component;
    BusFactoryProxy *factory;

    component = bus_registry_lookup_component_by_name (registry, name);

    if (component == NULL) {
        return NULL;
    }

    if (g_strcmp0 (old_name, "") != 0) {
        factory = bus_factory_proxy_get_from_component (component);

        if (factory != NULL) {
            ibus_object_destroy ((IBusObject *)factory);
        }
    }

    if (g_strcmp0 (new_name, "") != 0) {
        factory = bus_factory_proxy_new (component, NULL);
        return factory;
    }

    return NULL;
}
