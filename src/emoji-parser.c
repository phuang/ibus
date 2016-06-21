/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2016 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2016 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

/* Convert http://www.unicode.org/emoji/charts/emoji-list.html
 * to the dictionary file which look up the Emoji from the annotation.
 */

#include <glib.h>
#include <libxml/HTMLparser.h>
#include <libgen.h>

#include "ibusutil.h"

typedef struct _EmojiData EmojiData;
struct _EmojiData {
    gchar      *class;
    gchar      *emoji;
    GSList     *annotates;
    GSList     *prev_annotates;
    GHashTable *dict;
};

const gchar *progname;

static gboolean         parse_node             (xmlNode     *node,
                                                gboolean     is_child,
                                                const gchar *prop_name,
                                                EmojiData   *data);

static void
usage (void)
{
    g_print ("%s emoji-list.html emoji.dict\n", progname);
}

static void
reset_emoji_element (EmojiData *data)
{
    g_clear_pointer (&data->class, g_free);
    g_clear_pointer (&data->emoji, g_free);
    if (data->annotates) {
        g_slist_free_full (data->prev_annotates, g_free);
        data->prev_annotates = data->annotates;
        data->annotates = NULL;
    }
}

static void
free_dict_words (gpointer list)
{
    g_slist_free_full (list, g_free);
}

static gboolean
parse_attr (xmlAttr   *attr,
            EmojiData *data)
{
    if (g_strcmp0 ((const gchar *) attr->name, "class") == 0 && attr->children)
        parse_node (attr->children, TRUE, (const gchar *) attr->name, data);
    if (g_strcmp0 ((const gchar *) attr->name, "target") == 0 && attr->children)
        parse_node (attr->children, TRUE, (const gchar *) attr->name, data);
    if (attr->next)
        parse_attr (attr->next, data);
    return TRUE;
}

static gboolean
parse_node (xmlNode     *node,
            gboolean     is_child,
            const gchar *prop_name,
            EmojiData   *data)
{
    if (g_strcmp0 ((const gchar *) node->name, "tr") == 0) {
        GSList *annotates = data->annotates;
        while (annotates) {
            GSList *emojis = g_hash_table_lookup (data->dict, annotates->data);
            if (emojis) {
                emojis = g_slist_copy_deep (emojis, (GCopyFunc) g_strdup, NULL);
            }
            emojis = g_slist_append (emojis, g_strdup (data->emoji));
            g_hash_table_replace (data->dict,
                                  g_strdup (annotates->data),
                                  emojis);
            annotates = annotates->next;
        }
        reset_emoji_element (data);
    }
    /* if node->name is "text" and is_child is FALSE,
     * it's '\n' or Space between <td> and <td>.
     */
    if (g_strcmp0 ((const gchar *) node->name, "text") == 0 && is_child) {
        /* Get "chars" in <td class="chars"> */
        if (g_strcmp0 (prop_name, "class") == 0) {
            if (g_strcmp0 (data->class, (const gchar *) node->content) != 0) {
                g_clear_pointer (&data->class, g_free);
                data->class = g_strdup ((const gchar *) node->content);
            }
        }
        /* Get "annotate" in <td class="name"><a target="annotate"> */
        if (g_strcmp0 (prop_name, "target") == 0 &&
            g_strcmp0 (data->class, "name") == 0) {
            g_clear_pointer (&data->class, g_free);
            data->class = g_strdup ((const gchar *) node->content);
        }
        /* Get "emoji" in <td class="chars">emoji</td> */
        if (g_strcmp0 (prop_name, "td") == 0 &&
            g_strcmp0 (data->class, "chars") == 0) {
            data->emoji = g_strdup ((const gchar *) node->content);
        }
        /* We ignore "NAME" for <td class="name">NAME</td> but
         * takes "ANNOTATE" for
         * <td class="name"><a target="annotate">ANNOTATE</a></td>
         */
        if (g_strcmp0 (prop_name, "td") == 0 &&
            g_strcmp0 (data->class, "name") == 0) {
            g_slist_free_full (data->annotates, g_free);
            data->annotates = NULL;
        }
        /* Get "ANNOTATE" in
         * <td class="name"><a target="annotate">ANNOTATE</a></td>
         */
        if (g_strcmp0 (prop_name, "a") == 0 &&
            g_strcmp0 (data->class, "annotate") == 0) {
            data->annotates =
                    g_slist_append (data->annotates,
                                    g_strdup ((const gchar *) node->content));
        }
    }
    /* Get "foo" in <td class="foo"> */
    if (g_strcmp0 ((const gchar *) node->name, "td") == 0 &&
        node->properties != NULL) {
        parse_attr (node->properties, data);
    }
    /* Get "foo" in <a target="foo"> */
    if (g_strcmp0 ((const gchar *) node->name, "a") == 0 &&
        node->properties != NULL) {
        parse_attr (node->properties, data);
    }
    if (node->children) {
        parse_node (node->children, TRUE, (const gchar *) node->name, data);
    } else {
        /* If annotate is NULL likes <td class="name"></td>,
         * the previous emoji cell has the same annotate.
         */
        if (g_strcmp0 ((const gchar *) node->name, "td") == 0 &&
            g_strcmp0 (data->class, "name") == 0) {
            data->annotates = g_slist_copy_deep (data->prev_annotates,
                                                 (GCopyFunc) g_strdup,
                                                 NULL);
        }
    }
    if (node->next)
        parse_node (node->next, FALSE, (const gchar *) node->name, data);

    return TRUE;
}

static GHashTable *
parse_html (const gchar *filename)
{
    xmlDoc *doc = htmlParseFile (filename, "utf-8");
    EmojiData data = { 0, };

    if (doc == NULL || doc->children == NULL) {
        g_warning ("Parse Error in document type: %x",
                   doc ? doc->type : 0);
        return FALSE;
    }

    data.dict = g_hash_table_new_full (g_str_hash,
                                       g_str_equal,
                                       g_free,
                                       free_dict_words);
    parse_node (doc->children, TRUE, (const gchar *) doc->name, &data);

    reset_emoji_element (&data);
    g_slist_free_full (data.prev_annotates, g_free);

    return data.dict;
}

int
main (int argc, char *argv[])
{
    GHashTable *dict;
    progname = basename (argv[0]);

    if (argc < 3) {
        usage ();
        return -1;
    }

    dict = parse_html (argv[1]);
    ibus_emoji_dict_save (argv[2], dict);
    g_hash_table_destroy (dict);

    return 0;
}
