/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2008-2015 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2010-2016 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2008-2016 Red Hat, Inc.
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <string.h>
#include "ibusxml.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#endif

#define IBUS_DICT_MAGIC "IBusDict"
#define IBUS_DICT_VERSION (1)

/* gettext macro */
#define N_(t) t

static GHashTable *__languages_dict;

static gboolean
_iso_codes_parse_xml_node (XMLNode          *node)
{
    GList *p;
    g_assert (node);

    if (G_UNLIKELY (g_strcmp0 (node->name, "iso_639_entries") != 0)) {
        return FALSE;
    }

    for (p = node->sub_nodes; p != NULL; p = p->next) {
        XMLNode *sub_node = (XMLNode *)p->data;
        gchar **attributes = NULL;
        int i, j;
        struct {
            const gchar *key;
            gchar *value;
        } entries[] = {
            { "iso_639_2B_code", NULL },
            { "iso_639_2T_code", NULL },
            { "iso_639_1_code", NULL },
        };

        if (sub_node->attributes == NULL) {
            continue;
        }

        attributes = sub_node->attributes;
        for (i = 0; attributes[i]; i += 2) {
            if (g_strcmp0 (attributes[i], "name") == 0) {
                for (j = 0; j < G_N_ELEMENTS (entries); j++) {
                    if (entries[j].value == NULL)
                        continue;
                    g_hash_table_insert (__languages_dict,
                                         (gpointer) g_strdup (entries[j].value),
                                         (gpointer) g_strdup (attributes[i + 1]));
                    entries[j].value = NULL;
                }
            } else {
                for (j = 0; j < G_N_ELEMENTS (entries); j++) {
                    if (g_strcmp0 (attributes[i], entries[j].key) == 0 &&
                        attributes[i + 1] != NULL) {
                        entries[j].value = attributes[i + 1];
                    }
                }
            }
        }
    }

    return TRUE;
}

void
_load_lang()
{
    gchar *filename;
    XMLNode *node;
    struct stat buf;

#ifdef ENABLE_NLS
    bindtextdomain ("iso_639", GLIB_LOCALE_DIR);
    bind_textdomain_codeset ("iso_639", "UTF-8");
#endif

    __languages_dict = g_hash_table_new_full (g_str_hash,
            g_str_equal, g_free, g_free);
    filename = g_build_filename (ISOCODES_PREFIX,
                                 "share/xml/iso-codes/iso_639.xml",
                                 NULL);
    if (g_stat (filename, &buf) != 0) {
        g_warning ("Can not get stat of file %s", filename);
        g_free (filename);
        return;
    }

    node = ibus_xml_parse_file (filename);
    g_free (filename);

    if (!node) {
        return;
    }

    _iso_codes_parse_xml_node (node);
    ibus_xml_free (node);
}

static void
free_dict_words (gpointer list)
{
    g_slist_free_full (list, g_free);
}

static void
variant_foreach_add_emoji (gchar           *annotation,
                           GSList          *emojis,
                           GVariantBuilder *builder)
{
    int i;
    int length = (int) g_slist_length (emojis);
    gchar **buff = g_new0 (gchar *, length);
    GSList *l = emojis;

    for (i = 0; i < length; i++, l = l->next)
        buff[i] = (gchar *) l->data;

    g_variant_builder_add (builder,
                           "{sv}",
                           annotation,
                           g_variant_new_strv ((const gchar * const  *) buff,
                                               length));
    g_free (buff);
}

static GVariant *
ibus_emoji_dict_serialize (GHashTable *dict)
{
    GVariantBuilder builder;

    g_variant_builder_init (&builder, G_VARIANT_TYPE ("a{sv}"));
    g_hash_table_foreach (dict,  (GHFunc) variant_foreach_add_emoji, &builder);
    return g_variant_builder_end (&builder);
}

static GHashTable *
ibus_emoji_dict_deserialize (GVariant *variant)
{
    GHashTable *dict = NULL;
    GVariantIter iter;
    gchar *annotate = NULL;
    GVariant *emojis_variant = NULL;

    dict = g_hash_table_new_full (g_str_hash,
                                  g_str_equal,
                                  g_free,
                                  free_dict_words);

    g_variant_iter_init (&iter, variant);
    while (g_variant_iter_loop (&iter, "{sv}", &annotate, &emojis_variant)) {
        gsize i;
        gsize length = 0;
        const gchar **array = g_variant_get_strv (emojis_variant, &length);
        GSList *emojis = NULL;

        for (i = 0; i < length; i++) {
            emojis = g_slist_append (emojis, g_strdup (array[i]));
        }
        g_hash_table_insert (dict, annotate, emojis);
        annotate = NULL;
        g_clear_pointer (&emojis_variant, g_variant_unref);
    }

    return dict;
}

const gchar *
ibus_get_untranslated_language_name (const gchar *_locale)
{
    const gchar *retval;
    gchar *p = NULL;
    gchar *lang = NULL;

    if (__languages_dict == NULL )
        _load_lang();
    if ((p = strchr (_locale, '_')) !=  NULL)
        p = g_strndup (_locale, p - _locale);
    else
        p = g_strdup (_locale);
    lang = g_ascii_strdown (p, -1);
    g_free (p);
    retval = (const gchar *) g_hash_table_lookup (__languages_dict, lang);
    g_free (lang);
    if (retval != NULL)
        return retval;
    else
        return "Other";
}

const gchar *
ibus_get_language_name (const gchar *_locale)
{
    const gchar *retval = ibus_get_untranslated_language_name (_locale);

#ifdef ENABLE_NLS
    if (g_strcmp0 (retval, "Other") == 0)
        return dgettext (GETTEXT_PACKAGE, N_("Other"));
    else
        return dgettext ("iso_639", retval);
#else
    return retval;
#endif
}

void
ibus_g_variant_get_child_string (GVariant *variant, gsize index, char **str)
{
    g_return_if_fail (str != NULL);

    g_free (*str);
    g_variant_get_child (variant, index, "s", str);
}

void
ibus_emoji_dict_save (const gchar *path, GHashTable *dict)
{
    GVariant *variant;
    const gchar *header = IBUS_DICT_MAGIC;
    const guint16 version = IBUS_DICT_VERSION;
    const gchar *contents;
    gsize length;
    GError *error = NULL;

    variant = g_variant_new ("(sqv)",
                             header,
                             version,
                             ibus_emoji_dict_serialize (dict));

    contents =  g_variant_get_data (variant);
    length =  g_variant_get_size (variant);

    if (!g_file_set_contents (path, contents, length, &error)) {
        g_warning ("Failed to save emoji dict %s: %s", path, error->message);
        g_error_free (error);
    }

    g_variant_unref (variant);
}

GHashTable *
ibus_emoji_dict_load (const gchar *path)
{
    gchar *contents = NULL;
    gsize length = 0;
    GError *error = NULL;
    GVariant *variant_table = NULL;
    GVariant *variant = NULL;
    const gchar *header = NULL;
    guint16 version = 0;
    GHashTable *retval = NULL;

    if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
        g_warning ("Emoji dict does not exist: %s", path);
        goto out_load_cache;
    }

    if (!g_file_get_contents (path, &contents, &length, &error)) {
        g_warning ("Failed to get dict content %s: %s", path, error->message);
        g_error_free (error);
        goto out_load_cache;
    }

    variant_table = g_variant_new_from_data (G_VARIANT_TYPE ("(sq)"),
                                             contents,
                                             length,
                                             FALSE,
                                             NULL,
                                             NULL);

    if (variant_table == NULL) {
        g_warning ("cache table is broken.");
        goto out_load_cache;
    }

    g_variant_get (variant_table, "(&sq)", &header, &version);

    if (g_strcmp0 (header, IBUS_DICT_MAGIC) != 0) {
        g_warning ("cache is not IBusDict.");
        goto out_load_cache;
    }

    if (version != IBUS_DICT_VERSION) {
        g_warning ("cache version is different: %u != %u",
                   version, IBUS_DICT_VERSION);
        goto out_load_cache;
    }

    version = 0;
    header = NULL;
    g_variant_unref (variant_table);

    variant_table = g_variant_new_from_data (G_VARIANT_TYPE ("(sqv)"),
                                             contents,
                                             length,
                                             FALSE,
                                             NULL,
                                             NULL);

    if (variant_table == NULL) {
        g_warning ("cache table is broken.");
        goto out_load_cache;
    }

    g_variant_get (variant_table, "(&sqv)",
                   NULL,
                   NULL,
                   &variant);

    if (variant == NULL) {
        g_warning ("cache dict is broken.");
        goto out_load_cache;
    }

    retval = ibus_emoji_dict_deserialize (variant);

out_load_cache:
    if (variant)
        g_variant_unref (variant);
    if (variant_table)
        g_variant_unref (variant_table);

    return retval;
}

GSList *
ibus_emoji_dict_lookup (GHashTable  *dict,
                        const gchar *annotation)
{
    return (GSList *) g_hash_table_lookup (dict, annotation);
}
