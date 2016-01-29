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
#include <glib/gprintf.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "ibuscomposetable.h"
#include "ibuserror.h"
#include "ibusenginesimple.h"
#include "ibuskeys.h"
#include "ibuskeysyms.h"
#include "ibustypes.h"

#include "ibusenginesimpleprivate.h"

static gboolean
is_codepoint (const gchar *str)
{
    gboolean retval = (strlen (str) > 1 && *str == 'U');
    int i;

    if (!retval)
        return FALSE;

    for (i = 1; str[i] != '\0'; i++) {
        if (g_ascii_isxdigit (str[i]))
            continue;
        else
            return FALSE;
    }

    return TRUE;
}

static guint32
get_codepoint (const gchar *str)
{
    if (g_str_has_prefix (str, "IBUS_KEY_"))
        return (guint32) ibus_keyval_from_name (str + 9);
    if (*str == '0' && *(str + 1) == '\0')
        return 0;
    if (*str == '0' && *(str + 1) == 'x')
        return (guint32) g_ascii_strtoll (str, NULL, 16);

    g_assert_not_reached ();
    return 0;
}

static gboolean
parse_compose_value (GArray *array, const gchar *val, GError **error)
{
    gchar **words = g_strsplit (val, "\"", 3);
    gchar *result;
    gunichar uch;

    if (g_strv_length (words) < 3) {
        g_set_error (error,
                     IBUS_ERROR,
                     IBUS_ERROR_FAILED,
                     "Need to double-quote the value: %s", val);
        g_strfreev (words);
        return FALSE;
    }

    uch = g_utf8_get_char (words[1]);

    if (uch == 0) {
        g_set_error (error,
                     IBUS_ERROR,
                     IBUS_ERROR_FAILED,
                     "Invalid value: %s", val);
        g_strfreev (words);
        return FALSE;
    }
    else if (uch == '\\') {
        uch = words[1][1];

        /* The escaped string "\"" is separated with '\\' and '"'. */
        if (uch == '\0' && words[2][0] == '"')
            uch = '"';
        /* The escaped octal */
        else if (uch >= '0' && uch <= '8')
            uch = g_ascii_strtoll(words[1] + 1, 0, 8);
        /* If we need to handle other escape sequences. */
        else if (uch != '\\')
            g_assert_not_reached ();
    }
    else if (g_utf8_get_char (g_utf8_next_char (words[1])) > 0) {
        g_set_error (error,
                     IBUS_ERROR,
                     IBUS_ERROR_FAILED,
                     "GTK+ supports to output one char only: %s", val);
        g_strfreev (words);
        return FALSE;
    }


    result = g_strdup_printf ("0x%08X", uch);
    g_array_append_val (array, result);

    if (uch == '"')
        result = g_strdup (g_strstrip (words[2] + 1));
    else
        result = g_strdup (g_strstrip (words[2]));

    g_array_append_val (array, result);
    g_strfreev (words);

    return TRUE;
}

static gboolean
parse_compose_sequence (GArray *array, const gchar *seq, GError **error)
{
    gchar **words = g_strsplit (seq, "<", -1);
    gchar *result;
    int i;
    int n = 0;

    if (g_strv_length (words) < 2) {
        g_set_error (error,
                     IBUS_ERROR,
                     IBUS_ERROR_FAILED,
                     "key sequence format is <a> <b>...");
        g_strfreev (words);
        return FALSE;
    }

    for (i = 1; words[i] != NULL; i++) {
        gchar *start = words[i];
        gchar *end = index (words[i], '>');
        gchar *match;

        if (words[i][0] == '\0')
             continue;

        if (start == NULL || end == NULL || end <= start) {
            g_set_error (error,
                         IBUS_ERROR,
                         IBUS_ERROR_FAILED,
                         "key sequence format is <a> <b>...");
            g_strfreev (words);
            return FALSE;
        }

        match = g_strndup (start, end - start);

        if (is_codepoint (match)) {
            gint64 code = g_ascii_strtoll (match + 1, NULL, 16);
            result = g_strdup_printf ("0x%04X", (unsigned int) code);
        } else
            result = g_strdup_printf ("IBUS_KEY_%s", match);

        g_free (match);
        g_array_append_val (array, result);
        n++;
    }

    g_strfreev (words);
    if (0 == n || n >= IBUS_MAX_COMPOSE_LEN) {
        g_set_error (error,
                     IBUS_ERROR,
                     IBUS_ERROR_FAILED,
                     "The max number of sequences is %d", IBUS_MAX_COMPOSE_LEN);
        return FALSE;
    }

    result = g_strdup ("0");
    g_array_append_val (array, result);
    return TRUE;
}

static void
clear_char_array (gpointer data)
{
    gchar **array = data;
    g_free (*array);
}

static void
parse_compose_line (GList **compose_list, const gchar *line)
{
    gchar **components = NULL;
    GArray *array;
    GError *error = NULL;

    if (line[0] == '\0' || line[0] == '#')
        return;

    if (g_str_has_prefix (line, "include "))
        return;

    components = g_strsplit (line, ":", 2);

    if (components[1] == NULL) {
        g_warning ("No delimiter ':': %s", line);
        g_strfreev (components);
        return;
    }

    array = g_array_new (TRUE, TRUE, sizeof (gchar *));
    g_array_set_clear_func (array, clear_char_array);

    if (!parse_compose_sequence (array, g_strstrip (components[0]), &error)) {
        g_warning ("%s: %s", error->message, line);
        g_clear_error (&error);
        g_strfreev (components);
        g_array_unref (array);
        return;
    }

    if (!parse_compose_value (array, g_strstrip (components[1]), &error)) {
        g_warning ("%s: %s", error->message, line);
        g_clear_error (&error);
        g_strfreev (components);
        g_array_unref (array);
        return;
    }

    g_strfreev (components);

    *compose_list = g_list_append (*compose_list, array);
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

    for (list = compose_list; list != NULL; list = list->next) {
        GArray *array = (GArray *) list->data;
        static guint16 keysyms[IBUS_MAX_COMPOSE_LEN + 2];
        int i;
        int n_compose = 0;
        gboolean compose_finish;
        gunichar output_char;

        for (i = 0; i < IBUS_MAX_COMPOSE_LEN + 2; i++) {
            keysyms[i] = 0;
        }

        for (i = 0; i < array->len; i++) {
            const gchar *data = g_array_index (array, const gchar *, i);
            guint32 codepoint = get_codepoint (data);
            keysyms[i] = (guint16) codepoint;
            if (codepoint == IBUS_KEY_VoidSymbol)
                g_warning ("Could not get code point of keysym %s", data);

            if (codepoint == 0) {
                data = g_array_index (array, const gchar *, i + 1);
                codepoint = get_codepoint (data);
                keysyms[i + 1] = (guint16) codepoint;
                if (codepoint == IBUS_KEY_VoidSymbol)
                    g_warning ("Could not get code point of keysym %s", data);
                break;
            }

            n_compose++;
        }

        if (ibus_check_compact_table (&ibus_compose_table_compact,
                                      keysyms,
                                      n_compose,
                                      &compose_finish,
                                      &output_char) && compose_finish) {
            if (keysyms[n_compose + 1] == output_char)
                removed_list = g_list_append (removed_list, array);

        } else if (ibus_check_algorithmically (keysyms,
                                               n_compose,
                                               &output_char)) {
            if (keysyms[n_compose + 1] == output_char)
                removed_list = g_list_append (removed_list, array);
        }
    }

    for (list = removed_list; list != NULL; list = list->next) {
        GArray *array = (GArray *) list->data;
        compose_list = g_list_remove (compose_list, array);
        g_array_unref (array);
    }

    g_list_free (removed_list);

    return compose_list;
}

static GList *
ibus_compose_list_check_uint16 (GList *compose_list)
{
    GList *list;
    GList *removed_list = NULL;

    for (list = compose_list; list != NULL; list = list->next) {
        GArray *array = (GArray *) list->data;
        int i;

        for (i = 0; i < array->len; i++) {
            const gchar *data = g_array_index (array, const gchar *, i);
            guint32 codepoint = get_codepoint (data);

            if (codepoint == 0)
                break;

            if (codepoint > 0xffff) {
                removed_list = g_list_append (removed_list, array);
                break;
            }
        }
    }

    for (list = removed_list; list != NULL; list = list->next) {
        GArray *array = (GArray *) list->data;
        compose_list = g_list_remove (compose_list, array);
        g_array_unref (array);
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
    GList *new_list = NULL;
    int i;
    int j;
    int max_compose_len = 0;

    for (list = compose_list; list != NULL; list = list->next) {
        GArray *array = (GArray *) list->data;

        for (i = 0; i < array->len; i++) {
            const gchar *data = g_array_index (array, const gchar *, i);
            guint32 codepoint = get_codepoint (data);

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
        GArray *array = (GArray *) list->data;
        for (i = 0; i < array->len; i++) {
            const gchar *data = g_array_index (array, const gchar *, i);
            guint32 codepoint = get_codepoint (data);

            if (codepoint == 0) {
                gchar *value = g_strdup (g_array_index (array, const gchar *,
                                                        i + 1));
                gchar *comment = g_strdup (g_array_index (array, const gchar *,
                                                          i + 2));
                gchar *result;

                g_array_remove_range (array, i, array->len - i);

                for (j = i; j < max_compose_len; j++) {
                    result = g_strdup ("0");
                    g_array_append_val (array, result);
                }

                codepoint = get_codepoint (value);
                g_free (value);

                if (codepoint > 0xffff) {
                    result = g_strdup_printf ("0x%04X", codepoint / 0x10000);
                    g_array_append_val (array, result);
                    result = g_strdup_printf ("0x%04X",
                            codepoint - codepoint / 0x10000 * 0x10000);
                    g_array_append_val (array, result);
                } else {
                    result = g_strdup ("0");
                    g_array_append_val (array, result);
                    result = g_strdup_printf ("0x%04X", codepoint);
                    g_array_append_val (array, result);
                }

                g_array_append_val (array, comment);
                new_list = g_list_append (new_list, array);
                break;
            }
        }
    }

    g_list_free (compose_list);
    return new_list;
}

static gint
ibus_compose_data_compare (gpointer a,
                           gpointer b,
                           gpointer data)
{
    GArray *array_a = (GArray *) a;
    GArray *array_b = (GArray *) b;
    int max_compose_len = GPOINTER_TO_INT (data);
    int i;
    for (i = 0; i < max_compose_len; i++) {
        const gchar *data_a = g_array_index (array_a, const gchar *, i);
        const gchar *data_b = g_array_index (array_b, const gchar *, i);
        guint32 code_a = get_codepoint (data_a);
        guint32 code_b = get_codepoint (data_b);

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
    int i;
    int total_size = 0;

    for (list = compose_list; list != NULL; list = list->next) {
        GArray *array = (GArray *) list->data;
        const gchar *data;
        const gchar *upper;
        const gchar *lower;
        const gchar *comment;

        g_assert (array->len >= max_compose_len + 2);

        g_printf ("  ");
        for (i = 0; i < max_compose_len; i++) {
            data = g_array_index (array, const gchar *, i);

            if (i == max_compose_len - 1)
                g_printf ("%s,\n", data);
            else
                g_printf ("%s, ", data);
        }
        upper = g_array_index (array, const gchar *, i);
        lower = g_array_index (array, const gchar *, i + 1);
        comment = g_array_index (array, const gchar *, i + 2);

        if (list == g_list_last (compose_list))
            g_printf ("    %s, %s  /* %s */\n", upper, lower, comment);
        else
            g_printf ("    %s, %s, /* %s */\n", upper, lower, comment);

        total_size += n_index_stride;
    }

    g_printerr ("TOTAL_SIZE: %d\nMAX_COMPOSE_LEN: %d\nN_INDEX_STRIDE: %d\n",
                total_size, max_compose_len, n_index_stride);
}

static IBusComposeTable *
ibus_compose_table_new_with_list (GList *compose_list,
                                  int    max_compose_len,
                                  int    n_index_stride)
{
    guint length;
    guint n = 0;
    int i;
    static guint16 *ibus_compose_seqs = NULL;
    GList *list;
    IBusComposeTable *retval = NULL;

    g_return_val_if_fail (compose_list != NULL, NULL);

    length = g_list_length (compose_list);

    ibus_compose_seqs = g_new0 (guint16, length * n_index_stride);

    for (list = compose_list; list != NULL; list = list->next) {
        GArray *array = (GArray *) list->data;
        const gchar *data;
        for (i = 0; i < n_index_stride; i++) {
            data = g_array_index (array, const gchar *, i);
            ibus_compose_seqs[n++] = (guint16) get_codepoint (data);
        }
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
    int max_compose_len = 0;
    int n_index_stride = 0;
    IBusComposeTable *compose_table;

    g_assert (compose_file != NULL);

    compose_list = ibus_compose_list_parse_file (compose_file);
    if (compose_list == NULL) {
        g_list_free_full (compose_list, (GDestroyNotify) g_array_unref);
        return NULL;
    }
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
        g_list_free_full (compose_list, (GDestroyNotify) g_array_unref);
        return NULL;
    }

    if (g_getenv ("IBUS_COMPOSE_TABLE_PRINT") != NULL) {
        ibus_compose_list_print (compose_list, max_compose_len, n_index_stride);
    }

    compose_table = ibus_compose_table_new_with_list (
            compose_list,
            max_compose_len,
            n_index_stride);
    g_list_free_full (compose_list, (GDestroyNotify) g_array_unref);

    return compose_table;
}
