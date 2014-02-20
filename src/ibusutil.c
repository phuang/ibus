/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2008-2011 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2010-2011 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2008-2011 Red Hat, Inc.
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

const gchar *
ibus_get_language_name(const gchar *_locale) {
    const gchar *retval;
    gchar *p = NULL;
    gchar *lang = NULL;

    if (__languages_dict == NULL ) {
        _load_lang();
    }
    if ((p = strchr (_locale, '_')) !=  NULL) {
        p = g_strndup (_locale, p - _locale);
    } else {
        p = g_strdup (_locale);
    }
    lang = g_ascii_strdown (p, -1);
    g_free (p);
    retval = (const gchar *) g_hash_table_lookup (__languages_dict, lang);
    g_free (lang);
    if (retval != NULL) {
#ifdef ENABLE_NLS
        return dgettext("iso_639", retval);
#else
        return retval;
#endif
    }
    else {
#ifdef ENABLE_NLS
        return dgettext(GETTEXT_PACKAGE, N_("Other"));
#else
        return N_("Other");
#endif
    }
}
