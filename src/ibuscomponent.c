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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <dbus/dbus.h>
#include <glib/gstdio.h>
#include "ibuscomponent.h"

enum {
    LAST_SIGNAL,
};


/* IBusComponentPriv */
struct _IBusComponentPrivate {
    // TRUE if the component started in the verbose mode.
    gboolean verbose;
    // TRUE if the component needs to be restarted when it dies.
    gboolean restart;
};
typedef struct _IBusComponentPrivate IBusComponentPrivate;

#define IBUS_COMPONENT_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_COMPONENT, IBusComponentPrivate))

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void         ibus_component_destroy      (IBusComponent          *component);
static gboolean     ibus_component_serialize    (IBusComponent          *component,
                                                 IBusMessageIter        *iter);
static gboolean     ibus_component_deserialize  (IBusComponent          *component,
                                                 IBusMessageIter        *iter);
static gboolean     ibus_component_copy         (IBusComponent          *dest,
                                                 const IBusComponent    *src);
static gboolean     ibus_component_parse_xml_node
                                                (IBusComponent          *component,
                                                 XMLNode                *node,
                                                 gboolean                access_fs);

static void         ibus_component_parse_engines(IBusComponent          *component,
                                                 XMLNode                *node);
static void         ibus_component_parse_observed_paths
                                                (IBusComponent          *component,
                                                 XMLNode                *node,
                                                 gboolean                access_fs);

G_DEFINE_TYPE (IBusComponent, ibus_component, IBUS_TYPE_SERIALIZABLE)

static void
ibus_component_class_init (IBusComponentClass *klass)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (klass);

    g_type_class_add_private (klass, sizeof (IBusComponentPrivate));

    object_class->destroy = (IBusObjectDestroyFunc) ibus_component_destroy;

    serializable_class->serialize   = (IBusSerializableSerializeFunc) ibus_component_serialize;
    serializable_class->deserialize = (IBusSerializableDeserializeFunc) ibus_component_deserialize;
    serializable_class->copy        = (IBusSerializableCopyFunc) ibus_component_copy;

    g_string_append (serializable_class->signature, "ssssssssavav");
}


static void
ibus_component_init (IBusComponent *component)
{
    component->name = NULL;
    component->description = NULL;
    component->version = NULL;
    component->license = NULL;
    component->author = NULL;
    component->homepage = NULL;
    component->exec = NULL;
    component->textdomain = NULL;
    component->engines = NULL;
    component->observed_paths = NULL;
    component->pid = 0;
    component->child_source_id = 0;

    IBusComponentPrivate * priv = IBUS_COMPONENT_GET_PRIVATE (component);
    priv->verbose = FALSE;
    priv->restart = FALSE;
}

static void
ibus_component_destroy (IBusComponent *component)
{
    GList *p;

    g_free (component->name);
    g_free (component->description);
    g_free (component->version);
    g_free (component->license);
    g_free (component->author);
    g_free (component->homepage);
    g_free (component->exec);
    g_free (component->textdomain);

    component->name = NULL;
    component->description = NULL;
    component->version = NULL;
    component->license = NULL;
    component->author = NULL;
    component->homepage = NULL;
    component->exec = NULL;
    component->textdomain = NULL;

    g_list_foreach (component->observed_paths, (GFunc)g_object_unref, NULL);
    g_list_free (component->observed_paths);
    component->observed_paths = NULL;

    for (p = component->engines; p != NULL; p = p->next) {
        g_object_steal_data ((GObject *)p->data, "component");
        ibus_object_destroy ((IBusObject *)p->data);
        g_object_unref (p->data);
    }
    g_list_free (component->engines);
    component->engines = NULL;

    if (component->pid != 0) {
        ibus_component_stop (component);
        g_spawn_close_pid (component->pid);
        component->pid = 0;
    }

    if (component->child_source_id != 0) {
        g_source_remove (component->child_source_id);
        component->child_source_id = 0;
    }

    IBUS_OBJECT_CLASS (ibus_component_parent_class)->destroy (IBUS_OBJECT (component));
}

static gboolean
ibus_component_serialize (IBusComponent   *component,
                          IBusMessageIter *iter)
{
    gboolean retval;
    IBusMessageIter array_iter;
    GList *p;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_component_parent_class)->serialize ((IBusSerializable *)component, iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &component->name);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &component->description);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &component->version);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &component->license);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &component->author);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &component->homepage);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &component->exec);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &component->textdomain);
    g_return_val_if_fail (retval, FALSE);

    /* serialize observed paths */
    retval = ibus_message_iter_open_container (iter, IBUS_TYPE_ARRAY, "v", &array_iter);
    g_return_val_if_fail (retval, FALSE);

    for (p = component->observed_paths; p != NULL; p = p->next) {
        retval = ibus_message_iter_append (&array_iter, IBUS_TYPE_OBSERVED_PATH, &(p->data));
        g_return_val_if_fail (retval, FALSE);
    }
    retval = ibus_message_iter_close_container (iter, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    /* serialize engine desc */
    retval = ibus_message_iter_open_container (iter, IBUS_TYPE_ARRAY, "v", &array_iter);
    g_return_val_if_fail (retval, FALSE);

    for (p = component->engines; p != NULL; p = p->next) {
        retval = ibus_message_iter_append (&array_iter, IBUS_TYPE_ENGINE_DESC, &(p->data));
        g_return_val_if_fail (retval, FALSE);
    }
    retval = ibus_message_iter_close_container (iter, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    return TRUE;
}

static gboolean
ibus_component_deserialize (IBusComponent   *component,
                            IBusMessageIter *iter)
{
    gboolean retval;
    gchar *str;
    IBusMessageIter array_iter;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_component_parent_class)->deserialize ((IBusSerializable *)component, iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &str);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);
    component->name = g_strdup (str);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &str);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);
    component->description = g_strdup (str);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &str);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);
    component->version = g_strdup (str);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &str);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);
    component->license = g_strdup (str);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &str);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);
    component->author = g_strdup (str);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &str);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);
    component->homepage = g_strdup (str);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &str);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);
    component->exec = g_strdup (str);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &str);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);
    component->textdomain = g_strdup (str);

    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_ARRAY, &array_iter);
    g_return_val_if_fail (retval, FALSE);
    while (ibus_message_iter_get_arg_type (&array_iter) != G_TYPE_INVALID) {
        IBusObservedPath *path;

        retval = ibus_message_iter_get (&array_iter, IBUS_TYPE_OBSERVED_PATH, &path);
        g_return_val_if_fail (retval, FALSE);
        ibus_message_iter_next (&array_iter);

        g_object_ref_sink (path);
        component->observed_paths = g_list_append (component->observed_paths, path);
    }
    ibus_message_iter_next (iter);

    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_ARRAY, &array_iter);
    g_return_val_if_fail (retval, FALSE);

    while (ibus_message_iter_get_arg_type (&array_iter) != G_TYPE_INVALID) {
        IBusEngineDesc *engine;

        retval = ibus_message_iter_get (&array_iter, IBUS_TYPE_ENGINE_DESC, &engine);
        g_return_val_if_fail (retval, FALSE);
        ibus_message_iter_next (&array_iter);

        ibus_component_add_engine (component, engine);
    }
    ibus_message_iter_next (iter);

    return TRUE;
}

static gboolean
ibus_component_copy (IBusComponent       *dest,
                     const IBusComponent *src)
{
    gboolean retval;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_component_parent_class)->copy ((IBusSerializable *)dest,
                                 (IBusSerializable *)src);
    g_return_val_if_fail (retval, FALSE);


    dest->name          = g_strdup (src->name);
    dest->description   = g_strdup (src->description);
    dest->version       = g_strdup (src->version);
    dest->license       = g_strdup (src->license);
    dest->author        = g_strdup (src->author);
    dest->homepage      = g_strdup (src->homepage);
    dest->exec          = g_strdup (src->exec);
    dest->textdomain    = g_strdup (src->textdomain);

    dest->observed_paths = g_list_copy (src->observed_paths);
    g_list_foreach (dest->observed_paths, (GFunc) g_object_ref, NULL);

    dest->engines = g_list_copy (src->engines);
    g_list_foreach (dest->engines, (GFunc) g_object_ref, NULL);

    return TRUE;
}


#define g_string_append_indent(string, indent)  \
    {                                           \
        gint i;                                 \
        for (i = 0; i < (indent); i++) {        \
            g_string_append (string, "    ");   \
        }                                       \
    }

void
ibus_component_output (IBusComponent *component,
                      GString      *output,
                      gint          indent)
{
    g_assert (IBUS_IS_COMPONENT (component));
    GList *p;

    g_string_append_indent (output, indent);
    g_string_append (output, "<component>\n");

#define OUTPUT_ENTRY(field, element)                                        \
    {                                                                       \
        gchar *escape_text =                                                \
            g_markup_escape_text (component->field ?                        \
                                    component->field : "", -1);             \
        g_string_append_indent (output, indent + 1);                        \
        g_string_append_printf (output, "<"element">%s</"element">\n",      \
                                escape_text);                               \
        g_free (escape_text);                                               \
    }
#define OUTPUT_ENTRY_1(name) OUTPUT_ENTRY(name, #name)
    OUTPUT_ENTRY_1 (name);
    OUTPUT_ENTRY_1 (description);
    OUTPUT_ENTRY_1 (version);
    OUTPUT_ENTRY_1 (license);
    OUTPUT_ENTRY_1 (author);
    OUTPUT_ENTRY_1 (homepage);
    OUTPUT_ENTRY_1 (exec);
    OUTPUT_ENTRY_1 (textdomain);
#undef OUTPUT_ENTRY
#undef OUTPUT_ENTRY_1

    if (component->observed_paths) {
        g_string_append_indent (output, indent + 1);
        g_string_append (output, "<observed-paths>\n");

        for (p = component->observed_paths; p != NULL; p = p->next ) {
            IBusObservedPath *path = (IBusObservedPath *) p->data;

            g_string_append_indent (output, indent + 2);
            g_string_append_printf (output, "<path mtime=\"%ld\" >%s</path>\n",
                                    path->mtime,
                                    path->path);
        }

        g_string_append_indent (output, indent + 1);
        g_string_append (output, "</observed-paths>\n");
    }

    ibus_component_output_engines (component, output, indent + 1);

    g_string_append_indent (output, indent);
    g_string_append (output, "</component>\n");
}

void
ibus_component_output_engines (IBusComponent  *component,
                               GString        *output,
                               gint            indent)
{
    g_assert (IBUS_IS_COMPONENT (component));
    g_assert (output);

    GList *p;

    g_string_append_indent (output, indent);
    g_string_append (output, "<engines>\n");

    for (p = component->engines; p != NULL; p = p->next) {
        ibus_engine_desc_output ((IBusEngineDesc *)p->data, output, indent + 2);
    }

    g_string_append_indent (output, indent);
    g_string_append (output, "</engines>\n");
}

static gboolean
ibus_component_parse_xml_node (IBusComponent   *component,
                              XMLNode        *node,
                              gboolean        access_fs)
{
    g_assert (component);
    g_assert (node);

    if (G_UNLIKELY (g_strcmp0 (node->name, "component") != 0)) {
        return FALSE;
    }

    GList *p;
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        XMLNode *sub_node = (XMLNode *)p->data;

#define PARSE_ENTRY(field_name, element_name)                   \
        if (g_strcmp0 (sub_node->name, element_name) == 0) {    \
            if (component->field_name != NULL) {                \
                g_free (component->field_name);                 \
            }                                                   \
            component->field_name = g_strdup (sub_node->text);  \
            continue;                                           \
        }
#define PARSE_ENTRY_1(name) PARSE_ENTRY (name, #name)
        PARSE_ENTRY_1 (name);
        PARSE_ENTRY_1 (description);
        PARSE_ENTRY_1 (version);
        PARSE_ENTRY_1 (license);
        PARSE_ENTRY_1 (author);
        PARSE_ENTRY_1 (homepage);
        PARSE_ENTRY_1 (exec);
        PARSE_ENTRY_1 (textdomain);
#undef PARSE_ENTRY
#undef PARSE_ENTRY_1

        if (g_strcmp0 (sub_node->name, "engines") == 0) {
            ibus_component_parse_engines (component, sub_node);
            continue;
        }

        if (g_strcmp0 (sub_node->name, "observed-paths") == 0) {
            ibus_component_parse_observed_paths (component, sub_node, access_fs);
            continue;
        }

        g_warning ("<component> element contains invalidate element <%s>", sub_node->name);
    }

    return TRUE;
}


static void
ibus_component_parse_engines (IBusComponent *component,
                              XMLNode       *node)
{
    g_assert (IBUS_IS_COMPONENT (component));
    g_assert (node);

    gchar *exec = NULL;
    gchar **p;
    XMLNode *engines_node = NULL;

    if (g_strcmp0 (node->name, "engines") != 0) {
        return;
    }

    for (p = node->attributes; *p != NULL; p += 2) {
        if (g_strcmp0 (*p, "exec") == 0) {
            exec = *(p + 1);
            break;
        }
    }

    if (exec != NULL) {
        gchar *output = NULL;
        if (g_spawn_command_line_sync (exec, &output, NULL, NULL, NULL)) {
            engines_node = ibus_xml_parse_buffer (output);
            g_free (output);

            if (engines_node) {
                if (g_strcmp0 (engines_node->name, "engines") == 0) {
                    node = engines_node;
                }
            }
        }
    }

    GList *pl;
    for (pl = node->sub_nodes; pl != NULL; pl = pl->next) {
        IBusEngineDesc *engine;
        engine = ibus_engine_desc_new_from_xml_node ((XMLNode *)pl->data);

        if (G_UNLIKELY (engine == NULL))
            continue;
        ibus_component_add_engine (component, engine);
    }

    if (engines_node) {
        ibus_xml_free (engines_node);
    }
}

static void
ibus_component_parse_observed_paths (IBusComponent    *component,
                                    XMLNode         *node,
                                    gboolean         access_fs)
{
    g_assert (IBUS_IS_COMPONENT (component));
    g_assert (node);

    if (g_strcmp0 (node->name, "observed-paths") != 0) {
        return;
    }

    GList *p;
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        IBusObservedPath *path;

        path = ibus_observed_path_new_from_xml_node ((XMLNode *)p->data, access_fs);
        g_object_ref_sink (path);
        component->observed_paths = g_list_append (component->observed_paths, path);

        if (access_fs && path->is_dir && path->is_exist) {
            component->observed_paths = g_list_concat (component->observed_paths,
                                            ibus_observed_path_traverse (path));
        }
    }
}

IBusComponent *
ibus_component_new (const gchar *name,
                    const gchar *description,
                    const gchar *version,
                    const gchar *license,
                    const gchar *author,
                    const gchar *homepage,
                    const gchar *exec,
                    const gchar *textdomain)
{

    IBusComponent *component;
    component = (IBusComponent *)g_object_new (IBUS_TYPE_COMPONENT, NULL);

    component->name         = g_strdup (name);
    component->description  = g_strdup (description);
    component->version      = g_strdup (version);
    component->license      = g_strdup (license);
    component->author       = g_strdup (author);
    component->homepage     = g_strdup (homepage);
    component->exec         = g_strdup (exec);
    component->textdomain   = g_strdup (textdomain);

    return component;
}


IBusComponent *
ibus_component_new_from_xml_node (XMLNode  *node)
{
    g_assert (node);

    IBusComponent *component;

    component = (IBusComponent *)g_object_new (IBUS_TYPE_COMPONENT, NULL);
    if (!ibus_component_parse_xml_node (component, node, FALSE)) {
        g_object_unref (component);
        component = NULL;
    }

    return component;
}

IBusComponent *
ibus_component_new_from_file (const gchar *filename)
{
    g_assert (filename);

    XMLNode *node;
    struct stat buf;
    IBusComponent *component;
    gboolean retval;

    if (g_stat (filename, &buf) != 0) {
        g_warning ("Can not get stat of file %s", filename);
        return NULL;
    }

    node = ibus_xml_parse_file (filename);

    if (!node) {
        return NULL;
    }

    component = (IBusComponent *)g_object_new (IBUS_TYPE_COMPONENT, NULL);
    retval = ibus_component_parse_xml_node (component, node, TRUE);
    ibus_xml_free (node);

    if (!retval) {
        g_object_unref (component);
        component = NULL;
    }
    else {
        IBusObservedPath *path;
        path = ibus_observed_path_new (filename, TRUE);
        component->observed_paths = g_list_prepend (component->observed_paths, path);
    }

    return component;
}

void
ibus_component_add_observed_path (IBusComponent *component,
                                  const gchar   *path,
                                  gboolean       access_fs)
{
    IBusObservedPath *p;

    p = ibus_observed_path_new (path, access_fs);
    g_object_ref_sink (p);
    component->observed_paths = g_list_append (component->observed_paths, p);

    if (access_fs && p->is_dir && p->is_exist) {
        component->observed_paths = g_list_concat (component->observed_paths,
                                                   ibus_observed_path_traverse (p));
    }
}

void
ibus_component_add_engine (IBusComponent  *component,
                           IBusEngineDesc *engine)
{
    g_assert (IBUS_IS_COMPONENT (component));
    g_assert (IBUS_IS_ENGINE_DESC (engine));

    g_object_ref_sink (engine);
    component->engines = g_list_append (component->engines, engine);
    g_object_set_data ((GObject *)engine, "component", component);
}

GList *
ibus_component_get_engines (IBusComponent *component)
{
    return g_list_copy (component->engines);
}

static void
ibus_component_child_cb (GPid            pid,
                         gint            status,
                         IBusComponent  *component)
{
    g_assert (IBUS_IS_COMPONENT (component));
    g_assert (component->pid == pid);

    g_spawn_close_pid (pid);
    component->pid = 0;
    component->child_source_id = 0;

    IBusComponentPrivate *priv = IBUS_COMPONENT_GET_PRIVATE (component);
    if (priv->restart) {
        g_debug ("==== Restarting %s", component->exec);
        ibus_component_start (component, priv->verbose);
    }
}

gboolean
ibus_component_start (IBusComponent *component, gboolean verbose)
{
    g_assert (IBUS_IS_COMPONENT (component));

    if (component->pid != 0)
        return TRUE;

    IBusComponentPrivate *priv = IBUS_COMPONENT_GET_PRIVATE (component);
    priv->verbose = verbose;

    gint argc;
    gchar **argv;
    gboolean retval;
    GError *error;
    GSpawnFlags flags;

    error = NULL;
    if (!g_shell_parse_argv (component->exec, &argc, &argv, &error)) {
        g_warning ("Can not parse component %s exec: %s", component->name, error->message);
        g_error_free (error);
        return FALSE;
    }

    error = NULL;
    flags = G_SPAWN_DO_NOT_REAP_CHILD;
    if (!verbose) {
        flags |= G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL;
    }
    retval = g_spawn_async (NULL, argv, NULL,
                            flags,
                            NULL, NULL,
                            &(component->pid), &error);
    g_strfreev (argv);
    if (!retval) {
        g_warning ("Can not execute component %s: %s", component->name, error->message);
        g_error_free (error);
        return FALSE;
    }

    component->child_source_id =
        g_child_watch_add (component->pid, (GChildWatchFunc) ibus_component_child_cb, component);

    return TRUE;
}

gboolean
ibus_component_stop (IBusComponent *component)
{
    g_assert (IBUS_IS_COMPONENT (component));

    if (component->pid == 0)
        return TRUE;

    kill (component->pid, SIGTERM);
    return TRUE;
}

gboolean
ibus_component_is_running (IBusComponent *component)
{
    g_assert (IBUS_IS_COMPONENT (component));

    return (component->pid != 0);
}


gboolean
ibus_component_check_modification (IBusComponent *component)
{
    g_assert (IBUS_IS_COMPONENT (component));

    GList *p;

    for (p = component->observed_paths; p != NULL; p = p->next) {
        if (ibus_observed_path_check_modification ((IBusObservedPath *)p->data))
            return TRUE;
    }
    return FALSE;
}


IBusComponent *
ibus_component_get_from_engine (IBusEngineDesc *engine)
{
    g_assert (IBUS_IS_ENGINE_DESC (engine));

    IBusComponent *component;

    component = (IBusComponent *)g_object_get_data ((GObject *)engine, "component");
    return component;
}

void
ibus_component_set_restart (IBusComponent *component, gboolean restart)
{
    g_assert (IBUS_IS_COMPONENT (component));

    IBusComponentPrivate *priv = IBUS_COMPONENT_GET_PRIVATE (component);
    priv->restart = restart;
}
