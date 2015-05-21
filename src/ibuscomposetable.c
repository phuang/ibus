/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* ibus - The Input Bus
 * Copyright (C) 2013-2014 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2013-2015 Takao Fujiwara <takao.fujiwara1@gmail.com>
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

#include "ibuserror.h"
#include "ibuskeys.h"
#include "ibuskeysyms.h"
#include "ibuscomposetable.h"
#include "ibustypes.h"

#define IS_DEAD_KEY(k) \
    ((k) >= IBUS_KEY_dead_grave && (k) <= (IBUS_KEY_dead_dasia + 1))

int MAX_COMPOSE_LEN = 0;
int N_INDEX_STRIDE = 0;

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
parse_compose_file (const gchar *compose_file)
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
    }

    lines = g_strsplit (contents, "\n", -1);
    g_free (contents);
    for (i = 0; lines[i] != NULL; i++)
        parse_compose_line (&compose_list, lines[i]);
    g_strfreev (lines);

    return compose_list;
}

static int
compare_seq_index (const void *key, const void *value)
{
    const guint16 *keysyms = key;
    const guint16 *seq = value;

    if (keysyms[0] < seq[0])
        return -1;
    else if (keysyms[0] > seq[0])
        return 1;
    return 0;
}

static int
compare_seq (const void *key, const void *value)
{
    int i = 0;
    const guint16 *keysyms = key;
    const guint16 *seq = value;

    while (keysyms[i]) {
        if (keysyms[i] < seq[i])
            return -1;
        else if (keysyms[i] > seq[i])
            return 1;

        i++;
    }

    return 0;
}

/* Implement check_compact_table() in ibus/src/ibusenginesimple.c
 */
static gboolean
check_compact_table (const guint16                 *compose_buffer,
                     const IBusComposeTableCompact *table,
                     gint                           n_compose)
{
    gint row_stride;
    guint16 *seq_index;
    guint16 *seq;
    gint i;

    seq_index = bsearch (compose_buffer,
                         table->data,
                         table->n_index_size,
                         sizeof (guint16) *  table->n_index_stride,
                         compare_seq_index);

    if (seq_index == NULL) {
        // g_debug ("compact: no\n");
        return FALSE;
    }

    seq = NULL;
    i = n_compose - 1;

    if (i >= table->max_seq_len) {
        return FALSE;
    }

    row_stride = i + 1;

    if (seq_index[i + 1] <= seq_index[i]) {
        return FALSE;
    }

    seq = bsearch (compose_buffer + 1,
                   table->data + seq_index[i],
                   (seq_index[i + 1] - seq_index[i]) / row_stride,
                   sizeof (guint16) * row_stride,
                   compare_seq);
    // g_debug ("seq = %p", seq);

    if (!seq) {
        // g_debug ("no\n");
        return FALSE;
    }
    else {
        gunichar value = seq[row_stride - 1];
        // g_debug ("U+%04X\n", value);
        if (compose_buffer[n_compose + 1] == value)
            return TRUE;
        else
            return FALSE;
    }
}

static gboolean
check_normalize_nfc (gunichar* combination_buffer, gint n_compose)
{
    gunichar combination_buffer_temp[IBUS_MAX_COMPOSE_LEN];
    gchar *combination_utf8_temp = NULL;
    gchar *nfc_temp = NULL;
    gint n_combinations;
    gunichar temp_swap;
    gint i;

    n_combinations = 1;

    for (i = 1; i < n_compose; i++ )
        n_combinations *= i;

    if (combination_buffer[0] >= 0x390 && combination_buffer[0] <= 0x3FF) {
        for (i = 1; i < n_compose; i++ )
            if (combination_buffer[i] == 0x303)
                combination_buffer[i] = 0x342;
    }

    memcpy (combination_buffer_temp, combination_buffer,
            IBUS_MAX_COMPOSE_LEN * sizeof (gunichar) );

    for (i = 0; i < n_combinations; i++ ) {
        g_unicode_canonical_ordering (combination_buffer_temp, n_compose);
        combination_utf8_temp = g_ucs4_to_utf8 (combination_buffer_temp,
                                                -1, NULL, NULL, NULL);
        nfc_temp = g_utf8_normalize (combination_utf8_temp, -1,
                                     G_NORMALIZE_NFC);

        if (g_utf8_strlen (nfc_temp, -1) == 1) {
            memcpy (combination_buffer,
                    combination_buffer_temp,
                    IBUS_MAX_COMPOSE_LEN * sizeof (gunichar) );

            g_free (combination_utf8_temp);
            g_free (nfc_temp);

            return TRUE;
        }

        g_free (combination_utf8_temp);
        g_free (nfc_temp);

        if (n_compose > 2) {
            temp_swap = combination_buffer_temp[i % (n_compose - 1) + 1];
            combination_buffer_temp[i % (n_compose - 1) + 1] =
             combination_buffer_temp[(i+1) % (n_compose - 1) + 1];
            combination_buffer_temp[(i+1) % (n_compose - 1) + 1] = temp_swap;
        }
        else
            break;
    }

  return FALSE;
}

/* Implement check_algorithmically() in ibus/src/ibusenginesimple.c
 */
static gboolean
check_algorithmically (const guint16 *compose_buffer,
                       gint          n_compose)
{
    int i = 0;
    gunichar combination_buffer[IBUS_MAX_COMPOSE_LEN];
    gchar *combination_utf8, *nfc;

    if (n_compose >= IBUS_MAX_COMPOSE_LEN)
        return FALSE;

    for (i = 0; i < n_compose && IS_DEAD_KEY (compose_buffer[i]); i++)
        ;
    if (i == n_compose)
        return FALSE;

    if (i > 0 && i == n_compose - 1) {
        combination_buffer[0] =
                ibus_keyval_to_unicode ((guint) compose_buffer[i]);
        combination_buffer[n_compose] = 0;
        i--;

        while (i >= 0) {
            switch (compose_buffer[i]) {
#define CASE(keysym, unicode) \
            case IBUS_KEY_dead_##keysym: combination_buffer[i+1] = unicode; \
            break

            CASE (grave, 0x0300);
            CASE (acute, 0x0301);
            CASE (circumflex, 0x0302);
            CASE (tilde, 0x0303);
            CASE (macron, 0x0304);
            CASE (breve, 0x0306);
            CASE (abovedot, 0x0307);
            CASE (diaeresis, 0x0308);
            CASE (hook, 0x0309);
            CASE (abovering, 0x030A);
            CASE (doubleacute, 0x030B);
            CASE (caron, 0x030C);
            CASE (abovecomma, 0x0313);
            CASE (abovereversedcomma, 0x0314);
            CASE (horn, 0x031B);
            CASE (belowdot, 0x0323);
            CASE (cedilla, 0x0327);
            CASE (ogonek, 0x0328);
            CASE (iota, 0x0345);
            CASE (voiced_sound, 0x3099);
            CASE (semivoiced_sound, 0x309A);

            /* The following cases are to be removed once xkeyboard-config,
             * xorg are fully updated.
             */
            /* Workaround for typo in 1.4.x xserver-xorg */
            case 0xfe66: combination_buffer[i+1] = 0x314; break;
            /* CASE (dasia, 0x314); */
            /* CASE (perispomeni, 0x342); */
            /* CASE (psili, 0x343); */
#undef CASE
            default:
                combination_buffer[i+1] =
                        ibus_keyval_to_unicode ((guint) compose_buffer[i]);
            }
            i--;
        }

        if (check_normalize_nfc (combination_buffer, n_compose)) {
            gunichar value;

            combination_utf8 = g_ucs4_to_utf8 (combination_buffer,
                                               -1, NULL, NULL, NULL);
            nfc = g_utf8_normalize (combination_utf8, -1, G_NORMALIZE_NFC);

            value = g_utf8_get_char (nfc);
            g_free (combination_utf8);
            g_free (nfc);

            if (compose_buffer[n_compose + 1] == value)
                return TRUE;
        }
    }

    return FALSE;
}

static GList *
check_duplicated_compose (GList *compose_list)
{
    GList *list;
    GList *removed_list = NULL;

    for (list = compose_list; list != NULL; list = list->next) {
        GArray *array = (GArray *) list->data;
        static guint16 keysyms[IBUS_MAX_COMPOSE_LEN + 2];
        int i;
        int n_compose = 0;

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

        if (check_compact_table (keysyms,
                                 &ibus_compose_table_compact,
                                 n_compose))
            removed_list = g_list_append (removed_list, array);

        else if (check_algorithmically (keysyms, n_compose))
            removed_list = g_list_append (removed_list, array);
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
check_uint16 (GList *compose_list)
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
format_for_gtk (GList *compose_list)
{
    GList *list;
    GList *new_list = NULL;
    int i;
    int j;

    for (list = compose_list; list != NULL; list = list->next) {
        GArray *array = (GArray *) list->data;

        for (i = 0; i < array->len; i++) {
            const gchar *data = g_array_index (array, const gchar *, i);
            guint32 codepoint = get_codepoint (data);

            if (codepoint == 0) {
                if (MAX_COMPOSE_LEN < i)
                    MAX_COMPOSE_LEN = i;
                break;
            }
        }
    }

    N_INDEX_STRIDE = MAX_COMPOSE_LEN + 2;

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

                for (j = i; j < MAX_COMPOSE_LEN; j++) {
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
compare_array (gpointer a, gpointer b)
{
    GArray *array_a = (GArray *) a;
    GArray *array_b = (GArray *) b;
    int i;
    for (i = 0; i < MAX_COMPOSE_LEN; i++) {
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
print_compose_list (GList *compose_list)
{
    GList *list;
    int i;
    int TOTAL_SIZE = 0;

    for (list = compose_list; list != NULL; list = list->next) {
        GArray *array = (GArray *) list->data;
        const gchar *data;
        const gchar *upper;
        const gchar *lower;
        const gchar *comment;

        g_assert (array->len >= MAX_COMPOSE_LEN + 2);

        g_printf ("  ");
        for (i = 0; i < MAX_COMPOSE_LEN; i++) {
            data = g_array_index (array, const gchar *, i);

            if (i == MAX_COMPOSE_LEN -1)
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

        TOTAL_SIZE += N_INDEX_STRIDE;
    }

    g_printerr ("TOTAL_SIZE: %d\nMAX_COMPOSE_LEN: %d\nN_INDEX_STRIDE: %d\n",
                TOTAL_SIZE, MAX_COMPOSE_LEN, N_INDEX_STRIDE);
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
    IBusComposeTable *compose_table;

    g_assert (compose_file != NULL);

    MAX_COMPOSE_LEN = 0;
    N_INDEX_STRIDE = 0;

    compose_list = parse_compose_file (compose_file);
    if (compose_list == NULL) {
        g_list_free_full (compose_list, (GDestroyNotify) g_array_unref);
        return NULL;
    }
    compose_list = check_duplicated_compose (compose_list);
    compose_list = check_uint16 (compose_list);
    compose_list = format_for_gtk (compose_list);
    compose_list = g_list_sort (compose_list,
                                 (GCompareFunc) compare_array);
    if (compose_list == NULL) {
        g_list_free_full (compose_list, (GDestroyNotify) g_array_unref);
        return NULL;
    }

    if (g_getenv ("IBUS_PRINT_COMPOSE_TABLE") != NULL) {
        print_compose_list (compose_list);
    }

    compose_table = ibus_compose_table_new_with_list (compose_list,
                                                      MAX_COMPOSE_LEN,
                                                      N_INDEX_STRIDE);
    g_list_free_full (compose_list, (GDestroyNotify) g_array_unref);

    return compose_table;
}
