/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2016-2017 Takao Fujiwara <takao.fujiwara1@gmail.com>
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

/* Convert /usr/share/unicode/cldr/common/annotations/*.xml and
 * /usr/lib/node_modules/emojione/emoji.json
 * to the dictionary file which look up the Emoji from the annotation.
 * Get *.xml from https://github.com/fujiwarat/cldr-emoji-annotation
 * or http://www.unicode.org/repos/cldr/trunk/common/annotations .
 * Get emoji.json with 'npm install -g emojione'.
 * en.xml is used for the Unicode annotations and emoji.json is used
 * for the aliases_ascii, e.g. ":)", and category, e.g. "people".
 */

#include <glib.h>
#include <json-glib/json-glib.h>

#include <string.h>

#include "ibusemoji.h"

typedef struct _EmojiData EmojiData;
struct _EmojiData {
    gchar      *emoji;
    GSList     *annotations;
    gboolean    is_annotation;
    gchar      *description;
    gboolean    is_tts;
    gchar      *category;
    GSList     *list;
};

static void
reset_emoji_element (EmojiData *data)
{
    g_assert (data != NULL);

    g_clear_pointer (&data->emoji, g_free);
    g_slist_free_full (data->annotations, g_free);
    data->annotations = NULL;
    g_clear_pointer (&data->description, g_free);
    g_clear_pointer (&data->category, g_free);
}

gint
find_emoji_data_list (IBusEmojiData *a,
                      const gchar   *b)
{
   g_return_val_if_fail (IBUS_IS_EMOJI_DATA (a), 0);
   return g_strcmp0 (ibus_emoji_data_get_emoji (a), b);
}

static void
update_emoji_list (EmojiData *data)
{
    GSList *list = g_slist_find_custom (
            data->list,
            data->emoji,
            (GCompareFunc) find_emoji_data_list);
    if (list) {
        IBusEmojiData *emoji = list->data;
        GSList *src_annotations = data->annotations;
        GSList *dest_annotations = ibus_emoji_data_get_annotations (emoji);
        GSList *l;
        gboolean updated_annotations = FALSE;
        for (l = src_annotations; l; l = l->next) {
            GSList *duplicated = g_slist_find_custom (dest_annotations,
                                                      l->data,
                                                      (GCompareFunc) g_strcmp0);
            if (duplicated == NULL) {
                dest_annotations = g_slist_append (dest_annotations,
                                                   g_strdup (l->data));
                updated_annotations = TRUE;
            }
        }
        if (updated_annotations) {
            ibus_emoji_data_set_annotations (
                    emoji,
                    g_slist_copy_deep (dest_annotations,
                                       (GCopyFunc) g_strdup,
                                       NULL));
        }
        if (data->description)
            ibus_emoji_data_set_description (emoji, data->description);
    } else {
        IBusEmojiData *emoji =
                ibus_emoji_data_new ("emoji",
                                     data->emoji,
                                     "annotations",
                                     data->annotations,
                                     "description",
                                     data->description ? data->description
                                             : g_strdup (""),
                                     "category",
                                     data->category ? data->category
                                             : g_strdup (""),
                                     NULL);
        data->list = g_slist_append (data->list, emoji);
    }
}

static void
unicode_annotations_start_element_cb (GMarkupParseContext *context,
                                      const gchar         *element_name,
                                      const gchar        **attribute_names,
                                      const gchar        **attribute_values,
                                      gpointer             user_data,
                                      GError             **error)
{
    EmojiData *data = (EmojiData *) user_data;
    int i;
    const gchar *attribute;
    const gchar *value;

    g_assert (data != NULL);

    if (g_strcmp0 (element_name, "annotation") != 0)
        return;

    reset_emoji_element (data);

    for (i = 0; (attribute = attribute_names[i]) != NULL; i++) {
        value = attribute_values[i];

        if (g_strcmp0 (attribute, "cp") == 0) {
            if (value == NULL || *value == '\0') {
                g_warning ("cp='' in unicode.org annotations file");
                return;
            } else if (value[0] == '[' && value[strlen(value) - 1] == ']') {
                g_warning ("cp!='[emoji]' is an old format in unicode.org"
                           " annotations file");
                data->emoji = g_strndup (value + 1, strlen(value) - 2);
            } else {
                data->emoji = g_strdup (value);
            }
        }
        /* tts seems 'text to speach' and it would be a description
         * instead of annotation.
         */
        else if (g_strcmp0 (attribute, "type") == 0) {
            if (g_strcmp0 (value, "tts") == 0) {
                data->is_tts = TRUE;
            }
        }
    }

    data->is_annotation = TRUE;
}

static void
unicode_annotations_end_element_cb (GMarkupParseContext *context,
                                    const gchar         *element_name,
                                    gpointer             user_data,
                                    GError             **error)
{
    EmojiData *data = (EmojiData *) user_data;

    g_assert (data != NULL);
    if (!data->is_annotation)
        return;

    update_emoji_list (data);
    data->is_annotation = FALSE;
    data->is_tts = FALSE;
}

void
unicode_annotations_text_cb (GMarkupParseContext *context,
                             const gchar         *text,
                             gsize                text_len,
                             gpointer             user_data,
                             GError             **error)
{
    EmojiData *data = (EmojiData *) user_data;
    gchar **annotations = NULL;
    const gchar *annotation;
    int i;

    g_assert (data != NULL);
    if (!data->is_annotation)
        return;
    if (data->is_tts) {
        if (data->description) {
            g_warning ("Duplicated 'tts' is found: %s: %s",
                       data->description, text);
            g_clear_pointer (&data->description, g_free);
        }
        data->description = g_strdup (text);
        return;
    }
    annotations = g_strsplit (text, " | ", -1);
    for (i = 0; (annotation = annotations[i]) != NULL; i++) {
        GSList *duplicated = g_slist_find_custom (data->annotations,
                                                  annotation,
                                                  (GCompareFunc) g_strcmp0);
        if (duplicated == NULL) {
            data->annotations = g_slist_prepend (data->annotations,
                                                 g_strdup (annotation));
        }
    }
    g_strfreev (annotations);
}

static gboolean
unicode_annotations_parse_xml_file (const gchar  *filename,
                                    GSList      **list)
{
    gchar *content = NULL;
    gsize length = 0;
    GError *error = NULL;
    const static GMarkupParser parser = {
        unicode_annotations_start_element_cb,
        unicode_annotations_end_element_cb,
        unicode_annotations_text_cb,
        NULL,
        NULL
    };
    GMarkupParseContext *context = NULL;
    EmojiData data = { 0, };

    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (list != NULL, FALSE);

    if (!g_file_get_contents (filename, &content, &length, &error)) {
        g_warning ("Failed to load %s: %s", filename, error->message);
        goto failed_to_parse_unicode_annotations;
    }

    data.list = *list;

    context = g_markup_parse_context_new (&parser, 0, &data, NULL);
    if (!g_markup_parse_context_parse (context, content, length, &error)) {
        g_warning ("Failed to parse %s: %s", filename, error->message);
        goto failed_to_parse_unicode_annotations;
    }

    reset_emoji_element (&data);
    g_markup_parse_context_free (context);
    g_free (content);
    *list = data.list;
    return TRUE;

failed_to_parse_unicode_annotations:
    if (error)
        g_error_free (error);
    if (data.list)
        g_slist_free (data.list);
    if (context)
        g_markup_parse_context_free (context);
    g_free (content);
    return FALSE;
}

static gboolean
parse_emojione_unicode (JsonNode  *node,
                        EmojiData *data)
{
    const gchar *str, *unicode;
    gchar *endptr = NULL;
    guint32 uch;
    static gchar outbuf[8] = { 0, };
    GString *emoji;

    if (json_node_get_node_type (node) != JSON_NODE_VALUE) {
        g_warning ("'unicode' element is not string");
        return FALSE;
    }

    emoji = g_string_new (NULL);
    str = unicode = json_node_get_string (node);
    while (str && *str) {
        uch = g_ascii_strtoull (str, &endptr, 16);
        outbuf[g_unichar_to_utf8 (uch, outbuf)] = '\0';
        g_string_append (emoji, outbuf);
        if (*endptr == '\0') {
            break;
        } else {
            switch (*endptr) {
            case '-':
                endptr++;
                break;
            default:
                g_warning ("Failed to parse unicode %s", unicode);
            }
        }
        str = endptr;
        endptr = NULL;
    }

    data->emoji = g_string_free (emoji, FALSE);

    return TRUE;
}

static gboolean
parse_emojione_shortname (JsonNode  *node,
                          EmojiData *data)
{
#if 0
    const gchar *shortname;
    gchar *head, *s;
    int length;
    GSList *duplicated;

    if (json_node_get_node_type (node) != JSON_NODE_VALUE) {
        g_warning ("'shortname' element is not string");
        return FALSE;
    }

    /* The format is ':short_name:' */
    shortname = json_node_get_string (node);
    if (shortname == 0 || *shortname == '\0')
        return TRUE;
    if (*shortname != ':') {
        g_warning ("'shortname' format is different: %s", shortname);
        return FALSE;
    }

    length = strlen (shortname);
    head  = g_new0 (gchar, length);
    strcpy (head, shortname + 1);
    for (s = head; *s; s++) {
        if (*s == ':') {
            *s = '\0';
            break;
        } else if (*s == '_') {
            *s = ' ';
        }
    }

    if (head == NULL || *head == '\0') {
        g_warning ("'shortname' format is different: %s", shortname);
        g_free (head);
        return FALSE;
    }

    duplicated = g_slist_find_custom (data->annotations,
                                      head,
                                      (GCompareFunc) g_strcmp0);
    if (duplicated == NULL) {
        data->annotations = g_slist_prepend (data->annotations,
                                             head);
    } else {
       g_free (head);
    }

#endif
    return TRUE;
}

static gboolean
parse_emojione_name (JsonNode  *node,
                     EmojiData *data)
{
    const gchar *name;

    if (json_node_get_node_type (node) != JSON_NODE_VALUE) {
        g_warning ("'name' element is not string");
        return FALSE;
    }

    name = json_node_get_string (node);

    if (name == NULL || *name == '\0')
        return TRUE;

    data->description = g_strdup (name);

    return TRUE;
}

static gboolean
parse_emojione_category (JsonNode  *node,
                         EmojiData *data)
{
    const gchar *category;
    GSList *duplicated;

    if (json_node_get_node_type (node) != JSON_NODE_VALUE) {
        g_warning ("'category' element is not string");
        return FALSE;
    }

    category = json_node_get_string (node);

    if (category == NULL || *category == '\0')
        return TRUE;

    data->category = g_strdup (category);
    duplicated = g_slist_find_custom (data->annotations,
                                      category,
                                      (GCompareFunc) g_strcmp0);
    if (duplicated == NULL) {
        data->annotations = g_slist_prepend (data->annotations,
                                             g_strdup (category));
    }

    return TRUE;
}

static gboolean
parse_emojione_aliases_ascii (JsonNode  *node,
                              EmojiData *data)
{
    JsonArray *aliases_ascii;
    guint i, length;

    if (json_node_get_node_type (node) != JSON_NODE_ARRAY) {
        g_warning ("'aliases_ascii' element is not array");
        return FALSE;
    }

    aliases_ascii = json_node_get_array (node);
    length = json_array_get_length (aliases_ascii);
    for (i = 0; i < length; i++) {
        const gchar *alias = json_array_get_string_element (aliases_ascii, i);
        GSList *duplicated = g_slist_find_custom (data->annotations,
                                                  alias,
                                                  (GCompareFunc) g_strcmp0);
        if (duplicated == NULL) {
            data->annotations = g_slist_prepend (data->annotations,
                                                 g_strdup (alias));
        }
    }

    return TRUE;
}

static gboolean
parse_emojione_keywords (JsonNode  *node,
                         EmojiData *data)
{
#if 0
    JsonArray *keywords;
    guint i, length;

    if (json_node_get_node_type (node) != JSON_NODE_ARRAY) {
        g_warning ("'keywords' element is not array");
        return FALSE;
    }

    keywords = json_node_get_array (node);
    length = json_array_get_length (keywords);
    for (i = 0; i < length; i++) {
        const gchar *keyword = json_array_get_string_element (keywords, i);
        GSList *duplicated = g_slist_find_custom (data->annotations,
                                                  keyword,
                                                  (GCompareFunc) g_strcmp0);
        if (duplicated == NULL) {
            data->annotations = g_slist_prepend (data->annotations,
                                                 g_strdup (keyword));
        }
    }

#endif
    return TRUE;
}

static gboolean
parse_emojione_emoji_data (JsonNode    *node,
                           const gchar *member,
                           EmojiData   *data)
{
    if (g_strcmp0 (member, "unicode") == 0)
        return parse_emojione_unicode (node, data);
    else if (g_strcmp0 (member, "shortname") == 0)
        return parse_emojione_shortname (node, data);
    else if (g_strcmp0 (member, "name") == 0)
        return parse_emojione_name (node, data);
    else if (g_strcmp0 (member, "category") == 0)
        return parse_emojione_category (node, data);
    else if (g_strcmp0 (member, "aliases_ascii") == 0)
        return parse_emojione_aliases_ascii (node, data);
    else if (g_strcmp0 (member, "keywords") == 0)
        return parse_emojione_keywords (node, data);
    return TRUE;
}

static gboolean
parse_emojione_element (JsonNode  *node,
                        EmojiData *data)
{
    JsonObject *object;
    GList *members, *m;

    if (json_node_get_node_type (node) != JSON_NODE_OBJECT) {
            return FALSE;
    }

    reset_emoji_element (data);

    object = json_node_get_object (node);
    m = members = json_object_get_members (object);
    while (m) {
       const gchar *member = (const gchar *) m->data;
       if (!parse_emojione_emoji_data (json_object_get_member (object, member),
                                       member,
                                       data)) {
           g_list_free (members);
           return FALSE;
       }
       m = m->next;
    }
    g_list_free (members);

    update_emoji_list (data);

    return TRUE;
}

static gboolean
emojione_parse_json_file (const gchar  *filename,
                          GSList      **list)
{
    JsonParser *parser = json_parser_new ();
    JsonNode *node;
    JsonObject *object;
    GList *members, *m;
    GError *error = NULL;
    EmojiData data = { 0, };

    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (list != NULL, FALSE);

    if (!json_parser_load_from_file (parser, filename, &error)) {
        g_error ("%s", error->message);
        g_error_free (error);
        goto fail_to_json_file;
    }

    node = json_parser_get_root (parser);
    if (json_node_get_node_type (node) != JSON_NODE_OBJECT) {
        g_warning ("Json file does not have Json object %s", filename);
        goto fail_to_json_file;
    }

    object = json_node_get_object (node);
    members = json_object_get_members (object);
    data.list = *list;

    m = members;
    while (m) {
       const gchar *member = (const gchar *) m->data;
       if (!parse_emojione_element (json_object_get_member (object, member),
                                    &data)) {
           g_warning ("Failed to parse member '%s' in %s", member, filename);
       }
       m = m->next;
    }

    g_list_free (members);
    reset_emoji_element (&data);
    g_object_unref (parser);
    *list = data.list;

    return TRUE;

fail_to_json_file:
    g_object_unref (parser);
    return FALSE;
}

int
main (int argc, char *argv[])
{
    gchar *prgname;
    gchar *json_file = NULL;
    gchar *xml_file = NULL;
    gchar *output = NULL;
    GOptionEntry     entries[] = {
        { "json", 'j', 0, G_OPTION_ARG_STRING, &json_file,
          "Parse Emoji One JSON file",
          "JSON"
        },
        { "out", 'o', 0, G_OPTION_ARG_STRING, &output,
          "Save the emoji data as FILE",
          "FILE"
        },
        { "xml", 'x', 0, G_OPTION_ARG_STRING, &xml_file,
          "Parse Unocode.org ANNOTATIONS file",
          "ANNOTATIONS"
        },
        { NULL }
    };
    GOptionContext *context;
    GError *error = NULL;
    GSList *list = NULL;

    prgname = g_path_get_basename (argv[0]);
    g_set_prgname (prgname);
    g_free (prgname);

    context = g_option_context_new (NULL);
    g_option_context_add_main_entries (context, entries, NULL);

    if (argc < 3) {
        g_print ("%s", g_option_context_get_help (context, TRUE, NULL));
        g_option_context_free (context);
        return -1;
    }

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_warning ("Failed options: %s", error->message);
        g_error_free (error);
        return -1;
    }
    g_option_context_free (context);

    if (json_file)
        emojione_parse_json_file (json_file, &list);
    if (xml_file)
        unicode_annotations_parse_xml_file (xml_file, &list);
    if (list != NULL && output)
        ibus_emoji_data_save (output, list);
    if (list)
        g_slist_free (list);

    return 0;
}
