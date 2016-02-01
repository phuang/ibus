/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* ibus - The Input Bus
 * Copyright (C) 2013-2014 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2013-2016 Takao Fujiwara <takao.fujiwara1@gmail.com>
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

/* If you copy libX11/nls/??/Compose.pre in xorg git HEAD to
 * /usr/share/X11/locale/??/Compose , need to convert:
 * # sed -e 's/^XCOMM/#/' -e 's|X11_LOCALEDATADIR|/usr/share/X11/locale|'
 *   Compose.pre > /usr/share/X11/locale/foo/Compose
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "ibuscomposetable.h"
#include "ibuserror.h"
#include "ibusenginesimple.h"
#include "ibuskeys.h"
#include "ibuskeysyms.h"
#include "ibustypes.h"

#include "ibusenginesimpleprivate.h"

typedef struct {
  gunichar     *sequence;
  gunichar      value[2];
  gchar        *comment;
} IBusComposeData;

static void
ibus_compose_data_free (IBusComposeData *compose_data)
{
    g_free (compose_data->sequence);
    g_free (compose_data->comment);
    g_slice_free (IBusComposeData, compose_data);
}

static void
ibus_compose_list_element_free (IBusComposeData *compose_data,
                                gpointer         data)
{
    ibus_compose_data_free (compose_data);
}

static gboolean
is_codepoint (const gchar *str)
{
    int i;

    /* 'U' is not code point but 'U00C0' is code point */
    if (str[0] == '\0' || str[0] != 'U' || str[1] == '\0')
        return FALSE;

    for (i = 1; str[i] != '\0'; i++) {
        if (!g_ascii_isxdigit (str[i]))
            return FALSE;
    }

    return TRUE;
}

static gboolean
parse_compose_value (IBusComposeData  *compose_data,
                     const gchar      *val,
                     const gchar      *line)
{
    gchar **words = g_strsplit (val, "\"", 3);
    gunichar uch;

    if (g_strv_length (words) < 3) {
        g_warning ("Need to double-quote the value: %s: %s", val, line);
        goto fail;
    }

    uch = g_utf8_get_char (words[1]);

    if (uch == 0) {
        g_warning ("Invalid value: %s: %s", val, line);
        goto fail;
    }
    else if (uch == '\\') {
        uch = words[1][1];

        /* The escaped string "\"" is separated with '\\' and '"'. */
        if (uch == '\0' && words[2][0] == '"')
            uch = '"';
        /* The escaped octal */
        else if (uch >= '0' && uch <= '8')
            uch = g_ascii_strtoll(words[1] + 1, NULL, 8);
        /* If we need to handle other escape sequences. */
        else if (uch != '\\')
            g_assert_not_reached ();
    }

    if (g_utf8_get_char (g_utf8_next_char (words[1])) > 0) {
        g_warning ("GTK+ supports to output one char only: %s: %s", val, line);
        goto fail;
    }

    compose_data->value[1] = uch;

    if (uch == '"')
        compose_data->comment = g_strdup (g_strstrip (words[2] + 1));
    else
        compose_data->comment = g_strdup (g_strstrip (words[2]));

    g_strfreev (words);

    return TRUE;

fail:
    g_strfreev (words);
    return FALSE;
}

static gboolean
parse_compose_sequence (IBusComposeData *compose_data,
                        const gchar     *seq,
                        const gchar     *line)
{
    gchar **words = g_strsplit (seq, "<", -1);
    int i;
    int n = 0;

    if (g_strv_length (words) < 2) {
        g_warning ("key sequence format is <a> <b>...: %s", line);
        goto fail;
    }

    for (i = 1; words[i] != NULL; i++) {
        gchar *start = words[i];
        gchar *end = index (words[i], '>');
        gchar *match;
        gunichar codepoint;

        if (words[i][0] == '\0')
             continue;

        if (start == NULL || end == NULL || end <= start) {
            g_warning ("key sequence format is <a> <b>...: %s", line);
            goto fail;
        }

        match = g_strndup (start, end - start);

        if (compose_data->sequence == NULL)
            compose_data->sequence = g_malloc (sizeof (gunichar) * 2);
        else
            compose_data->sequence = g_realloc (compose_data->sequence,
                                                sizeof (gunichar) * (n + 2));

        if (is_codepoint (match)) {
            codepoint = (gunichar) g_ascii_strtoll (match + 1, NULL, 16);
            compose_data->sequence[n] = codepoint;
            compose_data->sequence[n + 1] = 0;
        } else {
            codepoint = (gunichar) ibus_keyval_from_name (match);
            compose_data->sequence[n] = codepoint;
            compose_data->sequence[n + 1] = 0;
        }

        if (codepoint == IBUS_KEY_VoidSymbol)
            g_warning ("Could not get code point of keysym %s", match);
        g_free (match);
        n++;
    }

    g_strfreev (words);
    if (0 == n || n >= IBUS_MAX_COMPOSE_LEN) {
        g_warning ("The max number of sequences is %d: %s",
                   IBUS_MAX_COMPOSE_LEN, line);
        return FALSE;
    }

    return TRUE;

fail:
    g_strfreev (words);
    return FALSE;
}

static void
parse_compose_line (GList       **compose_list,
                    const gchar  *line)
{
    gchar **components = NULL;
    IBusComposeData *compose_data = NULL;

    if (line[0] == '\0' || line[0] == '#')
        return;

    if (g_str_has_prefix (line, "include "))
        return;

    components = g_strsplit (line, ":", 2);

    if (components[1] == NULL) {
        g_warning ("No delimiter ':': %s", line);
        goto fail;
    }

    compose_data = g_slice_new0 (IBusComposeData);

    if (!parse_compose_sequence (compose_data,
                                 g_strstrip (components[0]),
                                 line)) {
        goto fail;
    }

    if (!parse_compose_value (compose_data, g_strstrip (components[1]), line))
        goto fail;

    g_strfreev (components);

    *compose_list = g_list_append (*compose_list, compose_data);

    return;

fail:
    g_strfreev (components);
    if (compose_data)
        ibus_compose_data_free (compose_data);
}

static GList *
ibus_compose_list_parse_file (const gchar *compose_file)
{
    gchar *contents = NULL;
    gchar **lines = NULL;
    gsize length = 0;
    GError *error = NULL;
    GList *compose_list = NULL;
    int i;

    if (!g_file_get_contents (compose_file, &contents, &length, &error)) {
        g_error ("%s", error->message);
        g_error_free (error);
        return NULL;
    }

    lines = g_strsplit (contents, "\n", -1);
    g_free (contents);
    for (i = 0; lines[i] != NULL; i++)
        parse_compose_line (&compose_list, lines[i]);
    g_strfreev (lines);

    return compose_list;
}

static GList *
ibus_compose_list_check_duplicated (GList *compose_list)
{
    GList *list;
    GList *removed_list = NULL;
    IBusComposeData *compose_data;

    for (list = compose_list; list != NULL; list = list->next) {
        static guint16 keysyms[IBUS_MAX_COMPOSE_LEN + 1];
        int i;
        int n_compose = 0;
        gboolean compose_finish;
        gunichar output_char;

        compose_data = list->data;

        for (i = 0; i < IBUS_MAX_COMPOSE_LEN + 1; i++)
            keysyms[i] = 0;

        for (i = 0; i < IBUS_MAX_COMPOSE_LEN + 1; i++) {
            gunichar codepoint = compose_data->sequence[i];
            keysyms[i] = (guint16) codepoint;

            if (codepoint == 0)
                break;

            n_compose++;
        }

        if (ibus_check_compact_table (&ibus_compose_table_compact,
                                      keysyms,
                                      n_compose,
                                      &compose_finish,
                                      &output_char) && compose_finish) {
            if (compose_data->value[1] == output_char)
                removed_list = g_list_append (removed_list, compose_data);

        } else if (ibus_check_algorithmically (keysyms,
                                               n_compose,
                                               &output_char)) {
            if (compose_data->value[1] == output_char)
                removed_list = g_list_append (removed_list, compose_data);
        }
    }

    for (list = removed_list; list != NULL; list = list->next) {
        compose_data = list->data;
        compose_list = g_list_remove (compose_list, compose_data);
        ibus_compose_data_free (compose_data);
    }

    g_list_free (removed_list);

    return compose_list;
}

static GList *
ibus_compose_list_check_uint16 (GList *compose_list)
{
    GList *list;
    GList *removed_list = NULL;
    IBusComposeData *compose_data;

    for (list = compose_list; list != NULL; list = list->next) {
        int i;

        compose_data = list->data;
        for (i = 0; i < IBUS_MAX_COMPOSE_LEN; i++) {
            gunichar codepoint = compose_data->sequence[i];

            if (codepoint == 0)
                break;

            if (codepoint > 0xffff) {
                removed_list = g_list_append (removed_list, compose_data);
                break;
            }
        }
    }

    for (list = removed_list; list != NULL; list = list->next) {
        compose_data = list->data;
        compose_list = g_list_remove (compose_list, compose_data);
        ibus_compose_data_free (compose_data);
    }

    g_list_free (removed_list);

    return compose_list;
}

static GList *
ibus_compose_list_format_for_gtk (GList *compose_list,
                                  int   *p_max_compose_len,
                                  int   *p_n_index_stride)
{
    GList *list;
    IBusComposeData *compose_data;
    int max_compose_len = 0;
    int i;
    gunichar codepoint;

    for (list = compose_list; list != NULL; list = list->next) {
        compose_data = list->data;

        for (i = 0; i < IBUS_MAX_COMPOSE_LEN + 1; i++) {
            codepoint = compose_data->sequence[i];
            if (codepoint == 0) {
                if (max_compose_len < i)
                    max_compose_len = i;
                break;
            }
        }
    }

    if (p_max_compose_len)
        *p_max_compose_len = max_compose_len;
    if (p_n_index_stride)
        *p_n_index_stride = max_compose_len + 2;

    for (list = compose_list; list != NULL; list = list->next) {
        compose_data = list->data;
        codepoint = compose_data->value[1];
        if (codepoint > 0xffff) {
            compose_data->value[0] = codepoint / 0x10000;
            compose_data->value[1] = codepoint - codepoint / 0x10000 * 0x10000;
        }
    }

    return compose_list;
}

static gint
ibus_compose_data_compare (gpointer a,
                           gpointer b,
                           gpointer data)
{
    IBusComposeData *compose_data_a = a;
    IBusComposeData *compose_data_b = b;
    int max_compose_len = GPOINTER_TO_INT (data);
    int i;
    for (i = 0; i < max_compose_len; i++) {
        gunichar code_a = compose_data_a->sequence[i];
        gunichar code_b = compose_data_b->sequence[i];

        if (code_a != code_b)
            return code_a - code_b;
    }
    return 0;
}

static void
ibus_compose_list_print (GList *compose_list,
                         int    max_compose_len,
                         int    n_index_stride)
{
    GList *list;
    int i, j;
    IBusComposeData *compose_data;
    int total_size = 0;
    gunichar upper;
    gunichar lower;
    const gchar *comment;
    const gchar *keyval;

    for (list = compose_list; list != NULL; list = list->next) {
        compose_data = list->data;
        g_printf ("  ");

        for (i = 0; i < max_compose_len; i++) {

            if (compose_data->sequence[i] == 0) {
                for (j = i; j < max_compose_len; j++) {
                    if (i == max_compose_len -1)
                        g_printf ("0,\n");
                    else
                        g_printf ("0, ");
                }
                break;
            }

            keyval = ibus_keyval_name (compose_data->sequence[i]);
            if (i == max_compose_len -1)
                g_printf ("%s,\n", keyval ? keyval : "(null)");
            else
                g_printf ("%s, ", keyval ? keyval : "(null)");
        }
        upper = compose_data->value[0];
        lower = compose_data->value[1];
        comment = compose_data->comment;

        if (list == g_list_last (compose_list))
            g_printf ("    %#06X, %#06X  /* %s */\n", upper, lower, comment);
        else
            g_printf ("    %#06X, %#06X, /* %s */\n", upper, lower, comment);

        total_size += n_index_stride;
    }

    g_printerr ("TOTAL_SIZE: %d\nMAX_COMPOSE_LEN: %d\nN_INDEX_STRIDE: %d\n",
                total_size, max_compose_len, n_index_stride);
}

static IBusComposeTable *
ibus_compose_table_new_with_list (GList   *compose_list,
                                  int      max_compose_len,
                                  int      n_index_stride)
{
    guint length;
    guint n = 0;
    int i, j;
    guint16 *ibus_compose_seqs = NULL;
    GList *list;
    IBusComposeData *compose_data;
    IBusComposeTable *retval = NULL;

    g_return_val_if_fail (compose_list != NULL, NULL);

    length = g_list_length (compose_list);

    ibus_compose_seqs = g_new0 (guint16, length * n_index_stride);

    for (list = compose_list; list != NULL; list = list->next) {
        compose_data = list->data;
        for (i = 0; i < max_compose_len; i++) {
            if (compose_data->sequence[i] == 0) {
                for (j = i; j < max_compose_len; j++)
                    ibus_compose_seqs[n++] = 0;
                break;
            }
            ibus_compose_seqs[n++] = (guint16) compose_data->sequence[i];
        }
        ibus_compose_seqs[n++] = (guint16) compose_data->value[0];
        ibus_compose_seqs[n++] = (guint16) compose_data->value[1];
    }

    retval = g_new0 (IBusComposeTable, 1);
    retval->data = ibus_compose_seqs;
    retval->max_seq_len = max_compose_len;
    retval->n_seqs = length;

    return retval;
}

IBusComposeTable *
ibus_compose_table_new_with_file (const gchar *compose_file)
{
    GList *compose_list = NULL;
    IBusComposeTable *compose_table;
    int max_compose_len = 0;
    int n_index_stride = 0;

    g_assert (compose_file != NULL);

    compose_list = ibus_compose_list_parse_file (compose_file);
    if (compose_list == NULL)
        return NULL;
    compose_list = ibus_compose_list_check_duplicated (compose_list);
    compose_list = ibus_compose_list_check_uint16 (compose_list);
    compose_list = ibus_compose_list_format_for_gtk (compose_list,
                                                     &max_compose_len,
                                                     &n_index_stride);
    compose_list = g_list_sort_with_data (
            compose_list,
            (GCompareDataFunc) ibus_compose_data_compare,
            GINT_TO_POINTER (max_compose_len));

    if (compose_list == NULL) {
        g_warning ("compose file %s does not include any keys besides keys "
                   "in en-us compose file", compose_file);
        return NULL;
    }

    if (g_getenv ("IBUS_COMPOSE_TABLE_PRINT") != NULL) {
        ibus_compose_list_print (compose_list, max_compose_len, n_index_stride);
    }

    compose_table = ibus_compose_table_new_with_list (
            compose_list,
            max_compose_len,
            n_index_stride);

    g_list_free_full (compose_list,
                      (GDestroyNotify) ibus_compose_list_element_free);

    return compose_table;
}
