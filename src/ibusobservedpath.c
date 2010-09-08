/* vim:set et sts=4: */
/* ibus - The Input IBus
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
#include <glib/gstdio.h>
#include <stdlib.h>
#include "ibusobservedpath.h"


enum {
    LAST_SIGNAL,
};


/* IBusObservedPathPriv */
struct _IBusObservedPathPrivate {
    gpointer pad;
};
typedef struct _IBusObservedPathPrivate IBusObservedPathPrivate;

#define IBUS_OBSERVED_PATH_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_OBSERVED_PATH, IBusObservedPathPrivate))

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void      ibus_observed_path_destroy         (IBusObservedPath       *path);
static gboolean  ibus_observed_path_serialize       (IBusObservedPath       *path,
                                                     IBusMessageIter        *iter);
static gboolean  ibus_observed_path_deserialize     (IBusObservedPath       *path,
                                                     IBusMessageIter        *iter);
static gboolean  ibus_observed_path_copy            (IBusObservedPath       *dest,
                                                     const IBusObservedPath *src);
static gboolean  ibus_observed_path_parse_xml_node  (IBusObservedPath       *path,
                                                     XMLNode                *node);

G_DEFINE_TYPE (IBusObservedPath, ibus_observed_path, IBUS_TYPE_SERIALIZABLE)

static void
ibus_observed_path_class_init (IBusObservedPathClass *klass)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (klass);

    // g_type_class_add_private (klass, sizeof (IBusObservedPathPrivate));

    object_class->destroy = (IBusObjectDestroyFunc) ibus_observed_path_destroy;

    serializable_class->serialize   = (IBusSerializableSerializeFunc) ibus_observed_path_serialize;
    serializable_class->deserialize = (IBusSerializableDeserializeFunc) ibus_observed_path_deserialize;
    serializable_class->copy        = (IBusSerializableCopyFunc) ibus_observed_path_copy;

    g_string_append (serializable_class->signature, "sx");
}


static void
ibus_observed_path_init (IBusObservedPath *path)
{
    path->path = NULL;
}

static void
ibus_observed_path_destroy (IBusObservedPath *path)
{
    g_free (path->path);
    IBUS_OBJECT_CLASS (ibus_observed_path_parent_class)->destroy (IBUS_OBJECT (path));
}

static gboolean
ibus_observed_path_serialize (IBusObservedPath *path,
                              IBusMessageIter  *iter)
{
    gboolean retval;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_observed_path_parent_class)->serialize ((IBusSerializable *)path, iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_STRING, &(path->path));
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_append (iter, G_TYPE_LONG, &(path->mtime));
    g_return_val_if_fail (retval, FALSE);

    return TRUE;
}

static gboolean
ibus_observed_path_deserialize (IBusObservedPath *path,
                                IBusMessageIter  *iter)
{
    gboolean retval;
    gchar *str;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_observed_path_parent_class)->deserialize ((IBusSerializable *)path, iter);
    g_return_val_if_fail (retval, FALSE);

    retval = ibus_message_iter_get (iter, G_TYPE_STRING, &str);
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);
    path->path = g_strdup (str);

    retval = ibus_message_iter_get (iter, G_TYPE_LONG, &(path->mtime));
    g_return_val_if_fail (retval, FALSE);
    ibus_message_iter_next (iter);

    return TRUE;
}

static gboolean
ibus_observed_path_copy (IBusObservedPath       *dest,
                         const IBusObservedPath *src)
{
    gboolean retval;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_observed_path_parent_class)->copy ((IBusSerializable *)dest, (IBusSerializable *)src);
    g_return_val_if_fail (retval, FALSE);

    dest->path = g_strdup (src->path);
    dest->mtime = src->mtime;

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
ibus_observed_path_output (IBusObservedPath *path,
                          GString         *output,
                          gint             indent)
{
    g_assert (IBUS_IS_OBSERVED_PATH (path));
    g_assert (output);

    g_string_append_indent (output, indent);
    g_string_append_printf (output, "<path mtime=\"%ld\" >%s</path>\n",
                                    path->mtime,
                                    path->path);
}

gboolean
ibus_observed_path_check_modification (IBusObservedPath *path)
{
    g_assert (IBUS_IS_OBSERVED_PATH (path));
    struct stat buf;

    if (g_stat (path->path, &buf) != 0) {
        buf.st_mtime = 0;
    }

    if (path->mtime == buf.st_mtime)
        return FALSE;
    return TRUE;
}

static void
ibus_observed_path_fill_stat (IBusObservedPath *path)
{
    g_assert (IBUS_IS_OBSERVED_PATH (path));

    struct stat buf;

    if (g_stat (path->path, &buf) == 0) {
        path->is_exist = 1;
        if (S_ISDIR (buf.st_mode)) {
            path->is_dir = 1;
        }
        path->mtime = buf.st_mtime;
    }
    else {
        path->is_dir = 0;
        path->is_exist = 0;
        path->mtime = 0;
    }
}

GList *
ibus_observed_path_traverse (IBusObservedPath *path)
{
    g_assert (IBUS_IS_OBSERVED_PATH (path));

    GDir *dir;
    const gchar *name;
    GList *paths = NULL;

    dir = g_dir_open (path->path, 0, NULL);

    if (dir == NULL)
        return NULL;

    while ((name = g_dir_read_name (dir)) != NULL) {
        IBusObservedPath *sub;

        sub = g_object_new (IBUS_TYPE_OBSERVED_PATH, NULL);
        g_object_ref_sink (sub);
        sub->path = g_build_filename (path->path, name, NULL);

        ibus_observed_path_fill_stat (sub);
        paths = g_list_append (paths, sub);

        if (sub->is_exist && sub->is_dir)
            paths = g_list_concat (paths, ibus_observed_path_traverse (sub));
    }
    g_dir_close (dir);

    return paths;
}

static gboolean
ibus_observed_path_parse_xml_node (IBusObservedPath *path,
                                   XMLNode          *node)
{
    g_assert (IBUS_IS_OBSERVED_PATH (path));
    g_assert (node);

    if (G_UNLIKELY (g_strcmp0 (node->name, "path") != 0)) {
        return FALSE;
    }

    if (node->text[0] == '~' && node->text[1] != G_DIR_SEPARATOR) {
        g_warning ("invalide path \"%s\"", node->text);
        return FALSE;
    }

    if (node->text[0] == '~') {
        const gchar *homedir = g_getenv ("HOME");
        if (homedir == NULL)
            homedir = g_get_home_dir ();
        path->path = g_build_filename (homedir, node->text + 2, NULL);
    }
    else {
        path->path = g_strdup (node->text);
    }

    gchar **attr;
    for (attr = node->attributes; attr[0]; attr += 2) {
        if (g_strcmp0 (*attr, "mtime") == 0) {
            path->mtime = atol (attr[1]);
            continue;
        }
        g_warning ("Unkonwn attribute %s", attr[0]);
    }

    return TRUE;
}

IBusObservedPath *
ibus_observed_path_new_from_xml_node (XMLNode *node,
                                     gboolean fill_stat)
{
    g_assert (node);

    IBusObservedPath *path;

    path = (IBusObservedPath *) g_object_new (IBUS_TYPE_OBSERVED_PATH, NULL);

    if (!ibus_observed_path_parse_xml_node (path, node)) {
        g_object_unref (path);
        path = NULL;
    }
    else if (fill_stat) {
        ibus_observed_path_fill_stat (path);
    }

    return path;
}

IBusObservedPath *
ibus_observed_path_new (const gchar *path,
                        gboolean     fill_stat)
{
    g_assert (path);

    IBusObservedPath *op;

    op = (IBusObservedPath *) g_object_new (IBUS_TYPE_OBSERVED_PATH, NULL);
    op->path = g_strdup (path);

    if (fill_stat) {
        ibus_observed_path_fill_stat (op);
    }

    return op;
}

