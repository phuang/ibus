/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* ibus - The Input Bus
 * Copyright (C) 2013-2014 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2013-2021 Takao Fujiwara <takao.fujiwara1@gmail.com>
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

#include <glib.h>
#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>

#include "ibuscomposetable.h"
#include "ibuserror.h"
#include "ibusenginesimple.h"
#include "ibuskeys.h"
#include "ibuskeysyms.h"
#include "ibustypes.h"

#include "ibusenginesimpleprivate.h"

/* This file contains the table of the compose sequences,
 * static const guint16 gtk_compose_seqs_compact[] = {}
 * It is generated from the compose-parse.py script.
 */
#include "gtkimcontextsimpleseqs.h"


#define IBUS_COMPOSE_TABLE_MAGIC "IBusComposeTable"
#define IBUS_COMPOSE_TABLE_VERSION (4)
#define X11_DATADIR X11_DATA_PREFIX "/share/X11/locale"

typedef struct {
  gunichar     *sequence;
  gunichar     *values;
  gchar        *comment;
} IBusComposeData;


extern const IBusComposeTableCompactEx ibus_compose_table_compact;
extern const IBusComposeTableCompactEx ibus_compose_table_compact_32bit;


static void
ibus_compose_data_free (IBusComposeData *compose_data)
{
    g_free (compose_data->sequence);
    g_free (compose_data->values);
    g_free (compose_data->comment);
    g_slice_free (IBusComposeData, compose_data);
}


static void
ibus_compose_list_element_free (IBusComposeData *compose_data,
                                gpointer         data)
{
    ibus_compose_data_free (compose_data);
}


static guint
unichar_length (gunichar *uni_array)
{
    guint i = 0;
    g_return_val_if_fail (uni_array, 0);
    while (uni_array[i])
        ++i;
    return i;
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
    gchar *head, *end, *p;
    gchar *ustr = NULL;
    gunichar *uchars = NULL, *up;
    GError *error = NULL;
    int n_uchars = 0;

    if (!(head = strchr (val, '\"'))) {
        g_warning ("Need to double-quote the value: %s: %s", val, line);
        goto fail;
    }
    ++head;
    p = head;
    end = NULL;
    while ((*p != '\0') && (end = strchr (p, '\"'))) {
        if (*(end - 1) == '\\' && *(end - 2) == '\\')
            break;
        if (*(end - 1) != '\\')
            break;
        p = end + 1;
    }
    if (end == NULL || *p == '\0') {
        g_warning ("Need to double-quote the value: %s: %s", val, line);
        goto fail;
    }
    ustr = g_strndup (head, end - head);
    p = ustr + 1;
    /* The escaped octal */
    if (*ustr == '\\' && *p >= '0' && *p <= '8') {
        compose_data->values = g_new (gunichar, 2);
        compose_data->values[0] = g_ascii_strtoll(p, NULL, 8);
        compose_data->values[1] = 0;
    } else {
        if (!(uchars = g_utf8_to_ucs4 (ustr, -1, NULL, NULL, &error))) {
            g_warning ("Invalid Unicode: %s: %s in %s:",
                       error->message, ustr, line);
            g_error_free (error);
            goto fail;
        } else if (!uchars[0]) {
            g_warning ("Invalid Unicode: \"\" in %s:", line);
            goto fail;
        }

        for (up = uchars; *up; up++) {
            if (*up == '\\') {
                ++up;
                if (*up != '"' && *up != '\\') {
                    g_warning ("Invalid backslash: %s: %s", val, line);
                    goto fail;
                }
            }
            if (!compose_data->values) {
                compose_data->values = g_new (gunichar, 2);
            } else {
                compose_data->values = g_renew (gunichar,
                                                compose_data->values,
                                                n_uchars + 2);
            }
            compose_data->values[n_uchars++] = *up;
        }
        compose_data->values[n_uchars] = 0;
    }

    g_free (ustr);
    g_free (uchars);
    compose_data->comment = g_strdup (g_strstrip (end + 1));

    return TRUE;

fail:
    g_free (ustr);
    g_free (uchars);
    return FALSE;
}


static int
parse_compose_sequence (IBusComposeData *compose_data,
                        const gchar     *seq,
                        const gchar     *line)
{
    gchar **words = g_strsplit (seq, "<", -1);
    int i;
    int n = 0;

    if (g_strv_length (words) < 2) {
        g_warning ("too few words; key sequence format is <a> <b>...: %s",
                   line);
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

        if (compose_data->sequence == NULL) {
            compose_data->sequence = g_new (gunichar, 2);
        } else {
            compose_data->sequence = g_renew (gunichar,
                                              compose_data->sequence,
                                              n + 2);
        }

        if (is_codepoint (match)) {
            codepoint = (gunichar) g_ascii_strtoll (match + 1, NULL, 16);
            compose_data->sequence[n] = codepoint;
        } else {
            codepoint = (gunichar) ibus_keyval_from_name (match);
            compose_data->sequence[n] = codepoint;
        }

        if (codepoint == IBUS_KEY_VoidSymbol) {
            g_warning ("Could not get code point of keysym %s: %s",
                       match, line);
            g_free (match);
            goto fail;
        }
        g_free (match);
        n++;
    }
    if (compose_data->sequence)
        compose_data->sequence[n] = 0;

    g_strfreev (words);
    if (0 == n || n >= IBUS_MAX_COMPOSE_LEN) {
        g_warning ("The max number of sequences is %d: %s",
                   IBUS_MAX_COMPOSE_LEN, line);
        return -1;
    }

    return n;

fail:
    g_strfreev (words);
    return -1;
}


static gchar *
expand_include_path (const gchar *include_path) {
    gchar *out = strdup ("");
    const gchar *head, *i;
    gchar *former, *o;

    for (head = i = include_path; *i; ++i) {
        /* expand sequence */
        if (*i == '%') {
            switch (*(i + 1)) {
            case 'H': { /* $HOME */
                const gchar *home = getenv ("HOME");
                if (!home) {
                    g_warning ("while parsing XCompose include target %s, "
                               "%%H replacement failed because HOME is not "
                               "defined; the include has been ignored",
                               include_path);
                    goto fail;
                }
                o = out;
                former = g_strndup (head, i - head);
                out = g_strdup_printf ("%s%s%s", o, former, home);
                head = i + 2;
                g_free (o);
                g_free (former);
                break;
            }
            case 'L': /* locale compose file */
                g_warning ("while handling XCompose include target %s, found "
                          "redundant %%L include; the include has been "
                          "ignored", include_path);
                goto fail;
            case 'S': /* system compose dir */
                o = out;
                former = g_strndup (head, i - head);
                out = g_strdup_printf ("%s%s%s", o, former, X11_DATADIR);
                head = i + 2;
                g_free (o);
                g_free (former);
                break;
            case '%': /* escaped % */
                o = out;
                former = g_strndup (head, i - head);
                out = g_strdup_printf ("%s%s%s", o, former, "%");
                head = i + 2;
                g_free (o);
                g_free (former);
                break;
            default:
                g_warning ("while parsing XCompose include target %s, found "
                           "unknown substitution character '%c'; the include "
                           "has been ignored", include_path, *(i+1));
                goto fail;
            }
            ++i;
        }
    }
    o = out;
    out = g_strdup_printf ("%s%s", o, head);
    g_free (o);
    return out;
fail:
    g_free (out);
    return NULL;
}


static void
parse_compose_line (GList       **compose_list,
                    const gchar  *line,
                    int          *compose_len,
                    gchar        **include)
{
    gchar **components = NULL;
    IBusComposeData *compose_data = NULL;
    int l;

    g_assert (compose_len);
    *compose_len = 0;

    /* eat spaces at the start of the line */
    while (*line && (*line == ' ' || *line == '\t')) line++;

    if (line[0] == '\0' || line[0] == '#')
        return;

    if (g_str_has_prefix (line, "include ")) {
        const char *rest = line + sizeof ("include ") - 1;
        while (*rest && *rest == ' ') rest++;

        /* grabbed the path part of the line */
        char *rest2;
        if (*rest == '"') {
            rest++;
            rest2 = g_strdup (rest);
            /* eat the closing quote */
            char *i = rest2;
            while (*i && *i != '"') i++;
            *i = '\0';
        } else {
            rest2 = g_strdup (rest);
        }
        *include = expand_include_path (rest2);
        g_free (rest2);
        return;
    }

    components = g_strsplit (line, ":", 2);

    if (components[1] == NULL) {
        g_warning ("No delimiter ':': %s", line);
        goto fail;
    }

    compose_data = g_slice_new0 (IBusComposeData);

    if ((l = parse_compose_sequence (compose_data,
                                     g_strstrip (components[0]),
                                     line)) < 1) {
        goto fail;
    }
    *compose_len = l;

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


static gchar *
get_en_compose_file (void)
{
    gchar * const sys_langs[] = { "en_US.UTF-8", "en_US", "en.UTF-8",
                                  "en", NULL };
    gchar * const *sys_lang = NULL;
    gchar *path = NULL;
    for (sys_lang = sys_langs; *sys_lang; sys_lang++) {
        path = g_build_filename (X11_DATADIR, *sys_lang, "Compose", NULL);
        if (g_file_test (path, G_FILE_TEST_EXISTS))
            break;
        g_free (path);
    }
    return path;
}


static GList *
ibus_compose_list_parse_file (const gchar *compose_file,
                              int         *max_compose_len)
{
    gchar *contents = NULL;
    gchar **lines = NULL;
    gsize length = 0;
    GError *error = NULL;
    GList *compose_list = NULL;
    int i;

    g_assert (max_compose_len);

    if (!g_file_get_contents (compose_file, &contents, &length, &error)) {
        g_error ("%s", error->message);
        g_error_free (error);
        return NULL;
    }

    lines = g_strsplit (contents, "\n", -1);
    g_free (contents);
    for (i = 0; lines[i] != NULL; i++) {
        int compose_len = 0;
        gchar *include = NULL;
        parse_compose_line (&compose_list, lines[i], &compose_len, &include);
        if (*max_compose_len < compose_len)
            *max_compose_len = compose_len;
        if (include && *include) {
            GStatBuf buf_include = { 0, };
            GStatBuf buf_parent = { 0, };
            gchar *en_compose;
            errno = 0;
            if (g_stat (include,  &buf_include)) {
                g_warning ("Cannot access %s: %s",
                           include,
                           g_strerror (errno));
                g_clear_pointer (&include, g_free);
                continue;
            }
            errno = 0;
            if (g_stat (compose_file,  &buf_parent)) {
                g_warning ("Cannot access %s: %s",
                           compose_file,
                           g_strerror (errno));
                g_clear_pointer (&include, g_free);
                continue;
            }
            if (buf_include.st_ino == buf_parent.st_ino) {
                g_warning ("Found recursive nest same file %s", include);
                g_clear_pointer (&include, g_free);
                continue;
            }
            en_compose = get_en_compose_file ();
            if (en_compose) {
                errno = 0;
                if (g_stat (en_compose,  &buf_parent)) {
                    g_warning ("Cannot access %s: %s",
                               compose_file,
                               g_strerror (errno));
                    g_clear_pointer (&include, g_free);
                    g_free (en_compose);
                    continue;
                }
            }
            g_free (en_compose);
            if (buf_include.st_ino == buf_parent.st_ino) {
                g_message ("System en_US Compose is already loaded %s\n",
                           include);
                g_clear_pointer (&include, g_free);
                continue;
            }
        }
        if (include && *include) {
            GList *rest = ibus_compose_list_parse_file (include,
                    max_compose_len);
            if (rest) {
                compose_list = g_list_concat (compose_list, rest);
            }
        }
        g_clear_pointer (&include, g_free);
    }
    g_strfreev (lines);

    return compose_list;
}


static GList *
ibus_compose_list_check_duplicated (GList *compose_list,
                                    int    max_compose_len)
{
    GList *list;
    static guint16 *keysyms;
    GList *removed_list = NULL;
    IBusComposeData *compose_data;
    gboolean is_32bit;

    keysyms = g_new (guint16, max_compose_len + 1);

    for (list = compose_list; list != NULL; list = list->next) {
        int i;
        int n_compose = 0;
        gboolean compose_finish;
        gunichar *output_chars = NULL;
        gunichar output_char = 0;
        guint n_outputs;

        compose_data = list->data;

        for (i = 0; i < max_compose_len + 1; i++)
            keysyms[i] = 0;

        for (i = 0; i < max_compose_len + 1; i++) {
            gunichar codepoint = compose_data->sequence[i];
            keysyms[i] = (guint16) codepoint;

            if (codepoint == 0)
                break;

            n_compose++;
        }

        n_outputs = unichar_length (compose_data->values);
        is_32bit = (n_outputs > 1) ? TRUE :
                (compose_data->values[0] >= 0xFFFF) ? TRUE : FALSE;
        if (!is_32bit &&
            ibus_compose_table_compact_check (&ibus_compose_table_compact,
                                              keysyms,
                                              n_compose,
                                              &compose_finish,
                                              &output_chars) &&
            compose_finish) {
            if (compose_data->values[0] == *output_chars)
                removed_list = g_list_append (removed_list, compose_data);
            g_free (output_chars);
        } else if (is_32bit &&
                   ibus_compose_table_compact_check (
                          &ibus_compose_table_compact_32bit,
                          keysyms,
                          n_compose,
                          &compose_finish,
                          &output_chars) && compose_finish) {
            
            if (n_outputs == unichar_length (output_chars)) {
                int j = 0;
                while (j < n_outputs && compose_data->values[j]) {
                    if (compose_data->values[j] != output_chars[j])
                        break;
                    ++j;
                }
                if (j == n_outputs)
                    removed_list = g_list_append (removed_list, compose_data);
            }
            g_free (output_chars);

        } else if (ibus_check_algorithmically (keysyms,
                                               n_compose,
                                               &output_char)) {
            if (compose_data->values[0] == output_char)
                removed_list = g_list_append (removed_list, compose_data);
        }
    }

    for (list = removed_list; list != NULL; list = list->next) {
        compose_data = list->data;
        compose_list = g_list_remove (compose_list, compose_data);
        ibus_compose_data_free (compose_data);
    }

    g_list_free (removed_list);
    g_free (keysyms);

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
    /* The allocation length of compose_data_a->sequence[] is different from
     * one of compose_data_b->sequence[] and max_compose_len indicates
     * the sequence length only but not include the compose value length.
     * So max_compose_len is greater than any allocation lengths of sequence[]
     * and this API should return if code_a or code_b is 0.
     */
    for (i = 0; i < max_compose_len; i++) {
        gunichar code_a = compose_data_a->sequence[i];
        gunichar code_b = compose_data_b->sequence[i];

        if (code_a != code_b)
            return code_a - code_b;
        if (code_a == 0 && code_b == 0)
            return 0;
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
        g_printf ("    ");
        for (i = 0; compose_data->values[i]; ++i)
            g_printf ("%#06X, ", compose_data->values[i]);
        g_printf (" /* %s */,\n", compose_data->comment);

        total_size += n_index_stride;
    }

    g_printerr ("TOTAL_SIZE: %d\nMAX_COMPOSE_LEN: %d\nN_INDEX_STRIDE: %d\n",
                total_size, max_compose_len, n_index_stride);
}


/* Implemented from g_str_hash() */
static guint32
ibus_compose_table_data_hash (gconstpointer v,
                              int           length)
{
    const guint16 *p, *head;
    unsigned char c;
    guint32 h = 5381;

    for (p = v, head = v; (p - head) < length; p++) {
        c = 0x00ff & (*p >> 8);
        h = (h << 5) + h + c;
        c = 0x00ff & *p;
        h = (h << 5) + h + c;
    }

    return h;
}


static gchar *
ibus_compose_hash_get_cache_path (guint32 hash)
{
    gchar *basename = NULL;
    const gchar *cache_dir;
    gchar *dir = NULL;
    gchar *path = NULL;

    basename = g_strdup_printf ("%08x.cache", hash);

    if ((cache_dir = g_getenv ("IBUS_COMPOSE_CACHE_DIR"))) {
        dir = g_strdup (cache_dir);
    } else {
        dir = g_build_filename (g_get_user_cache_dir (),
                                "ibus", "compose", NULL);
    }
    path = g_build_filename (dir, basename, NULL);
    errno = 0;
    if (g_mkdir_with_parents (dir, 0755)) {
        g_warning ("Failed to mkdir %s: %s", dir, g_strerror (errno));
        g_clear_pointer (&path, g_free);
    }

    g_free (dir);
    g_free (basename);

    return path;
}


static GVariant *
ibus_compose_table_serialize (IBusComposeTableEx *compose_table)
{
    const gchar *header = IBUS_COMPOSE_TABLE_MAGIC;
    const guint16 version = IBUS_COMPOSE_TABLE_VERSION;
    guint16 max_seq_len;
    guint16 index_stride;
    guint16 n_seqs;
    guint16 n_seqs_32bit = 0;
    guint16 second_size = 0;
    GVariant *variant_data = NULL;
    GVariant *variant_data_32bit_first = NULL;
    GVariant *variant_data_32bit_second = NULL;
    GVariant *variant_table;

    g_return_val_if_fail (compose_table != NULL, NULL);

    max_seq_len = compose_table->max_seq_len;
    index_stride = max_seq_len + 2;
    n_seqs = compose_table->n_seqs;

    g_return_val_if_fail (max_seq_len, NULL);

    if (n_seqs) {
        g_return_val_if_fail (compose_table->data, NULL);

        variant_data = g_variant_new_fixed_array (G_VARIANT_TYPE_UINT16,
                                                  compose_table->data,
                                                  (gsize)index_stride * n_seqs,
                                                  sizeof (guint16));
        if (!variant_data) {
            g_warning ("Could not change compose data to GVariant.");
            return NULL;
        }
    } else {
        variant_data = g_variant_new_fixed_array (
                G_VARIANT_TYPE_UINT16,
                NULL,
                0,
                sizeof (guint16));
        g_assert (variant_data);
    }
    if (compose_table->priv) {
        n_seqs_32bit = compose_table->priv->first_n_seqs;
        second_size = compose_table->priv->second_size;
    }
    if (!n_seqs && !n_seqs_32bit) {
        g_warning ("ComposeTable has not key sequences.");
        goto out_serialize;
    } else if (n_seqs_32bit && !second_size) {
        g_warning ("Compose key sequences are loaded but the values could " \
                   "not be loaded.");
        goto out_serialize;
    } else if (!n_seqs_32bit && second_size) {
        g_warning ("Compose values are loaded but the key sequences could " \
                   "not be loaded.");
        goto out_serialize;
    } else if (n_seqs_32bit && second_size) {
        if (!compose_table->priv->data_first) {
            g_warning ("data_first is NULL");
            goto out_serialize;
        }
        if (!compose_table->priv->data_second) {
            g_warning ("data_second is NULL");
            goto out_serialize;
        }
        variant_data_32bit_first = g_variant_new_fixed_array (
                G_VARIANT_TYPE_UINT16,
                compose_table->priv->data_first,
                (gsize)index_stride * n_seqs_32bit,
                sizeof (guint16));
        variant_data_32bit_second = g_variant_new_fixed_array (
                G_VARIANT_TYPE_UINT32,
                compose_table->priv->data_second,
                compose_table->priv->second_size,
                sizeof (guint32));
        if (!variant_data_32bit_first || !variant_data_32bit_second) {
            g_warning ("Could not change 32bit compose data to GVariant.");
            goto out_serialize;
        }
    } else {
        variant_data_32bit_first = g_variant_new_fixed_array (
                G_VARIANT_TYPE_UINT16,
                NULL,
                0,
                sizeof (guint16));
        variant_data_32bit_second = g_variant_new_fixed_array (
                G_VARIANT_TYPE_UINT32,
                NULL,
                0,
                sizeof (guint32));
        g_assert (variant_data_32bit_first && variant_data_32bit_second);
    }
    variant_table = g_variant_new ("(sqqqqqvvv)",
                                   header,
                                   version,
                                   max_seq_len,
                                   n_seqs,
                                   n_seqs_32bit,
                                   second_size,
                                   variant_data,
                                   variant_data_32bit_first,
                                   variant_data_32bit_second);
    return g_variant_ref_sink (variant_table);

out_serialize:
    g_clear_pointer (&variant_data, g_variant_unref);
    g_clear_pointer (&variant_data_32bit_first, g_variant_unref);
    g_clear_pointer (&variant_data_32bit_second, g_variant_unref);
    return NULL;
}


static gint
ibus_compose_table_find (gconstpointer data1,
                         gconstpointer data2)
{
    const IBusComposeTableEx *compose_table =
            (const IBusComposeTableEx *) data1;
    guint32 hash = (guint32) GPOINTER_TO_INT (data2);
    return compose_table->id != hash;
}


static IBusComposeTableEx *
ibus_compose_table_deserialize (const gchar *contents,
                                gsize        length)
{
    IBusComposeTableEx *retval = NULL;
    GVariantType *type;
    GVariant *variant_data = NULL;
    GVariant *variant_data_32bit_first = NULL;
    GVariant *variant_data_32bit_second = NULL;
    GVariant *variant_table = NULL;
    const gchar *header = NULL;
    guint16 version = 0;
    guint16 max_seq_len = 0;
    guint16 n_seqs = 0;
    guint16 n_seqs_32bit = 0;
    guint16 second_size = 0;
    guint16 index_stride;
    gconstpointer data = NULL;
    gconstpointer data_32bit_first = NULL;
    gconstpointer data_32bit_second = NULL;
    gsize data_length = 0;

    g_return_val_if_fail (contents != NULL, NULL);
    g_return_val_if_fail (length > 0, NULL);

    /* Check the cache version at first before load whole the file content. */
    type = g_variant_type_new ("(sq)");
    variant_table = g_variant_new_from_data (type,
                                             contents,
                                             length,
                                             FALSE,
                                             NULL,
                                             NULL);
    g_variant_type_free (type);

    if (!variant_table) {
        g_warning ("cache is broken.");
        goto out_load_cache;
    }

    g_variant_ref_sink (variant_table);
    g_variant_get (variant_table, "(&sq)", &header, &version);

    if (g_strcmp0 (header, IBUS_COMPOSE_TABLE_MAGIC) != 0) {
        g_warning ("cache is not IBusComposeTable.");
        goto out_load_cache;
    }

    if (version != IBUS_COMPOSE_TABLE_VERSION) {
        g_warning ("cache version is different: %u != %u",
                   version, IBUS_COMPOSE_TABLE_VERSION);
        goto out_load_cache;
    }

    version = 0;
    header = NULL;
    g_variant_unref (variant_table);
    variant_table = NULL;

    type = g_variant_type_new ("(sqqqqqvvv)");
    variant_table = g_variant_new_from_data (type,
                                             contents,
                                             length,
                                             FALSE,
                                             NULL,
                                             NULL);
    g_variant_type_free (type);

    if (!variant_table) {
        g_warning ("cache is broken.");
        goto out_load_cache;
    }

    g_variant_ref_sink (variant_table);
    g_variant_get (variant_table, "(&sqqqqqvvv)",
                   NULL,
                   NULL,
                   &max_seq_len,
                   &n_seqs,
                   &n_seqs_32bit,
                   &second_size,
                   &variant_data,
                   &variant_data_32bit_first,
                   &variant_data_32bit_second);

    if (max_seq_len == 0 || (n_seqs == 0 && n_seqs_32bit == 0)) {
        g_warning ("cache size is not correct %d %d %d",
                   max_seq_len, n_seqs, n_seqs_32bit);
        goto out_load_cache;
    }

    if (n_seqs && variant_data) {
        data = g_variant_get_fixed_array (variant_data,
                                          &data_length,
                                          sizeof (guint16));
    }
    index_stride = max_seq_len + 2;

    if (data_length != (gsize)index_stride * n_seqs) {
        g_warning ("cache size is not correct %d %d %lu",
                   max_seq_len, n_seqs, data_length);
        goto out_load_cache;
    }

    retval = g_new0 (IBusComposeTableEx, 1);
    if (data_length) {
        retval->data = g_new (guint16, data_length);
        memcpy (retval->data, data, data_length * sizeof (guint16));
    }
    retval->max_seq_len = max_seq_len;
    retval->n_seqs = n_seqs;

    if (n_seqs_32bit && !second_size) {
        g_warning ("32bit key sequences are loaded but the values " \
                   "could not be loaded.");
        goto out_load_cache;
    }
    if (!n_seqs_32bit && second_size) {
        g_warning ("32bit key sequences could not loaded but the values " \
                   "are loaded.");
        goto out_load_cache;
    }

    data_length = 0;
    if (n_seqs_32bit && variant_data_32bit_first) {
        data_32bit_first = g_variant_get_fixed_array (variant_data_32bit_first,
                                                      &data_length,
                                                      sizeof (guint16));
        if (data_length != (gsize) index_stride * n_seqs_32bit) {
            g_warning ("32bit cache size is not correct %d %d %lu",
                       max_seq_len, n_seqs_32bit, data_length);
            goto out_load_cache;
        }
    }
    if (!data && !data_32bit_first) {
        g_warning ("cache data is null.");
        goto out_load_cache;
    }
    if (data_length) {
        retval->priv = g_new0 (IBusComposeTablePrivate, 1);
        retval->priv->data_first = g_new (guint16, data_length);
        memcpy (retval->priv->data_first,
                data_32bit_first, data_length * sizeof (guint16));
        retval->priv->first_n_seqs = n_seqs_32bit;
    }

    data_length = 0;
    if (second_size && variant_data_32bit_second) {
        data_32bit_second = g_variant_get_fixed_array (
                variant_data_32bit_second,
                &data_length,
                sizeof (guint32));
        if (data_length != (gsize) second_size) {
            g_warning ("32bit cache size is not correct %d %d",
                       max_seq_len, second_size);
            goto out_load_cache;
        }
    }
    if (data_length) {
        if ((retval->priv->data_second = g_new (guint32, data_length))) {
            memcpy (retval->priv->data_second,
                    data_32bit_second, data_length * sizeof (guint32));
            retval->priv->second_size = second_size;
        } else {
            g_warning ("Failed g_new");
            retval->priv->second_size = 0;
        }
    }


out_load_cache:
    g_clear_pointer (&variant_data, g_variant_unref);
    g_clear_pointer (&variant_data_32bit_first, g_variant_unref);
    g_clear_pointer (&variant_data_32bit_second, g_variant_unref);
    g_clear_pointer (&variant_table, g_variant_unref);
    return retval;
}


IBusComposeTableEx *
ibus_compose_table_load_cache (const gchar *compose_file)
{
    IBusComposeTableEx *retval = NULL;
    guint32 hash;
    gchar *path = NULL;
    gchar *contents = NULL;
    GStatBuf original_buf;
    GStatBuf cache_buf;
    gsize length = 0;
    GError *error = NULL;

    do {
        hash = g_str_hash (compose_file);
        if ((path = ibus_compose_hash_get_cache_path (hash)) == NULL)
            return NULL;
        if (!g_file_test (path, G_FILE_TEST_EXISTS))
            break;

        if (g_stat (compose_file, &original_buf))
            break;
        if (g_stat (path, &cache_buf))
            break;
        if (original_buf.st_mtime > cache_buf.st_mtime)
            break;
        if (!g_file_get_contents (path, &contents, &length, &error)) {
            g_warning ("Failed to get cache content %s: %s",
                       path, error->message);
            g_error_free (error);
            break;
        }

        retval = ibus_compose_table_deserialize (contents, length);
        if (retval == NULL)
            g_warning ("Failed to load the cache file: %s", path);
        else
            retval->id = hash;
    } while (0);

    g_free (contents);
    g_free (path);
    return retval;
}


void
ibus_compose_table_save_cache (IBusComposeTableEx *compose_table)
{
    gchar *path = NULL;
    GVariant *variant_table = NULL;
    const gchar *contents = NULL;
    GError *error = NULL;
    gsize length = 0;

    if ((path = ibus_compose_hash_get_cache_path (compose_table->id)) == NULL)
      return;

    variant_table = ibus_compose_table_serialize (compose_table);
    if (variant_table == NULL) {
        g_warning ("Failed to serialize compose table %s", path);
        goto out_save_cache;
    }
    contents = g_variant_get_data (variant_table);
    length = g_variant_get_size (variant_table);
    if (!g_file_set_contents (path, contents, length, &error)) {
        g_warning ("Failed to save compose table %s: %s", path, error->message);
        g_error_free (error);
        goto out_save_cache;
    }

out_save_cache:
    g_variant_unref (variant_table);
    g_free (path);
}


static IBusComposeTableEx *
ibus_compose_table_new_with_list (GList   *compose_list,
                                  int      max_compose_len,
                                  int      n_index_stride,
                                  guint32  hash)
{
    /* @ibus_compose_seqs: Include both compose sequence and the value(compose
     *     output) as the tradition GTK. The value is one character only
     *     and within 16bit. I.e. All compose sequences and the values
     *     are 16bit.
     * @ibus_compose_seqs_32bit_second: Include the compose values only.
     *     The length of values by compose sequence is more than one characster
     *     or one of the values is outside 16bit but within 32bit.
     *     Some values could be more than one character and Emoji character
     *     could be outside 16bit.
     *     See also ibus/src/tests/ibus-compose.emoji file for e.g.
     * @ibus_compose_seqs_32bit_first: Include the compose sequence only in
     *     case the value is included in @ibus_compose_seqs_32bit_second.
     * @s_size_total: The number of compose sequences.
     * @s_size_16bit: The number of compose sequences whose value is one
     *     character only and within 16bit. I.e. the number of the compose
     *     sequence in @ibus_compose_seqs is @@s_size_16bit. And
     *     @s_size_total - @s_size_16bit is the number of the compose sequence
     *     in @ibus_compose_seqs_32bit_first.
     * @v_size_32bit: The total number of compose values. Each length of the
     *     values is more than one character or one of the value is
     *     outside 16bit but within 32bit. I.e. The size of
     *     @ibus_compose_seqs_32bit_second is @v_size_32bit.
     * @v_index_32bit: Each index of the compose values in
     *     @ibus_compose_seqs_32bit_second and this is not a fixed value in
     *     this API. If a compose sequence is found in
     *     @ibus_compose_seqs_32bit_first and the next value is 0, 0 is lined
     *     in @ibus_compose_seqs_32bit_first until @max_compose_len after
     *     the found compose sequence. And the next value is the length of
     *     the compose values and the next value is the @v_index_32bit, i.e.
     *     the index of @ibus_compose_seqs_32bit_second.
     *     E.g. the following line could be found in
     *     @ibus_compose_seqs_32bit_first:
     *         ..., "U17ff", "0", "0", "0", "0", 2, 100, ...
     *     @ibus_compose_seqs_32bit_second[100] is "ាំ"  and the character
     *     length is 2.
     *     @max_compose_len is 5 and @n_index_stride is 7.
     */
    gsize s_size_total, s_size_16bit, v_size_32bit, v_index_32bit;
    guint n = 0, m = 0;
    int i, j;
    guint16 *ibus_compose_seqs = NULL;
    guint16 *ibus_compose_seqs_32bit_first = NULL;
    guint32 *ibus_compose_seqs_32bit_second = NULL;
    GList *list;
    IBusComposeData *compose_data = NULL;
    IBusComposeTableEx *retval = NULL;

    g_return_val_if_fail (compose_list != NULL, NULL);

    s_size_total = g_list_length (compose_list);
    s_size_16bit = s_size_total;
    v_size_32bit = 0;

    for (list = compose_list; list != NULL; list = list->next) {
        compose_data = list->data;
        if (unichar_length (compose_data->values) > 1 ||
            compose_data->values[0] >= 0xFFFF) {
            --s_size_16bit;
            v_size_32bit += unichar_length (compose_data->values);
        }
    }

    if (s_size_16bit) {
        ibus_compose_seqs = g_new (guint16, s_size_16bit * n_index_stride);
        if (!ibus_compose_seqs) {
            g_warning ("Failed g_new");
            return NULL;
        }
    }
    if (s_size_total > s_size_16bit) {
        ibus_compose_seqs_32bit_first =
                g_new (guint16,
                       (s_size_total - s_size_16bit) * n_index_stride);
        ibus_compose_seqs_32bit_second = g_new (guint32, v_size_32bit);
        if (!ibus_compose_seqs_32bit_first || !ibus_compose_seqs_32bit_second) {
            g_warning ("Failed g_new");
            g_free (ibus_compose_seqs);
            g_free (ibus_compose_seqs_32bit_first);
            g_free (ibus_compose_seqs_32bit_second);
            return NULL;
        }
    }

    v_index_32bit = 0;
    for (list = compose_list; list != NULL; list = list->next) {
        gboolean is_32bit = FALSE;
        compose_data = list->data;

        is_32bit = unichar_length (compose_data->values) > 1 ? TRUE :
                compose_data->values[0] >= 0xFFFF ? TRUE : FALSE;
        if (is_32bit) {
            g_assert (ibus_compose_seqs_32bit_first);
            g_assert (ibus_compose_seqs_32bit_second);
        }
        for (i = 0; i < max_compose_len; i++) {
            if (compose_data->sequence[i] == 0) {
                for (j = i; j < max_compose_len; j++) {
                    if (is_32bit) {
                        g_assert (m < (s_size_total - s_size_16bit)
                                  * n_index_stride);
                        ibus_compose_seqs_32bit_first[m++] = 0;
                    } else {
                        g_assert (n < s_size_16bit * n_index_stride);
                        ibus_compose_seqs[n++] = 0;
                    }
                }
                break;
            }
            if (is_32bit) {
                g_assert (m < (s_size_total - s_size_16bit) * n_index_stride);
                ibus_compose_seqs_32bit_first[m++] =
                        (guint16) compose_data->sequence[i];
            } else {
                g_assert (n < s_size_16bit * n_index_stride);
                ibus_compose_seqs[n++] = (guint16) compose_data->sequence[i];
            }
        }
        if (is_32bit) {
            for (j = 0; compose_data->values[j]; j++) {
                g_assert (v_index_32bit + j <  v_size_32bit);
                ibus_compose_seqs_32bit_second[v_index_32bit + j] =
                        compose_data->values[j];
            }
            g_assert (m + 1 < (s_size_total - s_size_16bit) * n_index_stride);
            ibus_compose_seqs_32bit_first[m++] = j;
            ibus_compose_seqs_32bit_first[m++] = v_index_32bit;
            v_index_32bit += j;
        } else {
            g_assert (n + 1 < s_size_16bit * n_index_stride);
            ibus_compose_seqs[n++] = (guint16) compose_data->values[0];
            ibus_compose_seqs[n++] = 0;
        }
    }

    retval = g_new0 (IBusComposeTableEx, 1);
    retval->data = ibus_compose_seqs;
    retval->max_seq_len = max_compose_len;
    retval->n_seqs = s_size_16bit;
    retval->id = hash;
    if (s_size_total > s_size_16bit) {
        retval->priv = g_new0 (IBusComposeTablePrivate, 1);
        retval->priv->data_first = ibus_compose_seqs_32bit_first;
        retval->priv->data_second = ibus_compose_seqs_32bit_second;
        retval->priv->first_n_seqs = s_size_total - s_size_16bit;
        retval->priv->second_size = v_size_32bit;
    }

    return retval;
}


IBusComposeTableEx *
ibus_compose_table_new_with_file (const gchar *compose_file)
{
    GList *compose_list = NULL;
    IBusComposeTableEx *compose_table;
    int max_compose_len = 0;
    int n_index_stride = 0;

    g_assert (compose_file != NULL);

    compose_list = ibus_compose_list_parse_file (compose_file,
                                                 &max_compose_len);
    if (compose_list == NULL)
        return NULL;
    n_index_stride = max_compose_len + 2;
    compose_list = ibus_compose_list_check_duplicated (compose_list,
                                                       max_compose_len);
    compose_list = g_list_sort_with_data (
            compose_list,
            (GCompareDataFunc) ibus_compose_data_compare,
            GINT_TO_POINTER (max_compose_len));

    if (compose_list == NULL) {
        g_warning ("compose file %s does not include any keys besides keys "
                   "in en-us compose file", compose_file);
        return NULL;
    }

    if (g_getenv ("IBUS_COMPOSE_TABLE_PRINT") != NULL)
        ibus_compose_list_print (compose_list, max_compose_len, n_index_stride);

    compose_table = ibus_compose_table_new_with_list (
            compose_list,
            max_compose_len,
            n_index_stride,
            g_str_hash (compose_file));

    g_list_free_full (compose_list,
                      (GDestroyNotify) ibus_compose_list_element_free);

    return compose_table;
}


/* if ibus_compose_seqs[N - 1] is an outputed compose character,
 * ibus_compose_seqs[N * 2 - 1] is also an outputed compose character.
 * and ibus_compose_seqs[0] to ibus_compose_seqs[0 + N - 3] are the
 * sequences and call ibus_engine_simple_add_table:
 * ibus_engine_simple_add_table(engine, ibus_compose_seqs,
 *                              N - 2, G_N_ELEMENTS(ibus_compose_seqs) / N)
 * The compose sequences are allowed within G_MAXUINT16
 */
GSList *
ibus_compose_table_list_add_array (GSList        *compose_tables,
                                   const guint16 *data,
                                   gint           max_seq_len,
                                   gint           n_seqs)
{
    guint32 hash;
    IBusComposeTableEx *compose_table;
    int n_index_stride = max_seq_len + 2;
    int length = n_index_stride * n_seqs;
    int i;
    guint16 *ibus_compose_seqs = NULL;

    g_assert (length >= 0);
    g_return_val_if_fail (data != NULL, compose_tables);
    g_return_val_if_fail (max_seq_len <= IBUS_MAX_COMPOSE_LEN, compose_tables);

    hash = ibus_compose_table_data_hash (data, length);

    if (g_slist_find_custom (compose_tables,
                             GINT_TO_POINTER (hash),
                             ibus_compose_table_find) != NULL) {
        return compose_tables;
    }

    ibus_compose_seqs = g_new0 (guint16, length);
    for (i = 0; i < length; i++)
        ibus_compose_seqs[i] = data[i];

    compose_table = g_new0 (IBusComposeTableEx, 1);
    compose_table->data = ibus_compose_seqs;
    compose_table->max_seq_len = max_seq_len;
    compose_table->n_seqs = n_seqs;
    compose_table->id = hash;

    return g_slist_prepend (compose_tables, compose_table);
}


GSList *
ibus_compose_table_list_add_file (GSList      *compose_tables,
                                  const gchar *compose_file)
{
    guint32 hash;
    IBusComposeTableEx *compose_table;

    g_return_val_if_fail (compose_file != NULL, compose_tables);

    hash = g_str_hash (compose_file);
    if (g_slist_find_custom (compose_tables,
                             GINT_TO_POINTER (hash),
                             ibus_compose_table_find) != NULL) {
        return compose_tables;
    }

    compose_table = ibus_compose_table_load_cache (compose_file);
    if (compose_table != NULL)
        return g_slist_prepend (compose_tables, compose_table);

   if ((compose_table = ibus_compose_table_new_with_file (compose_file))
           == NULL) {
       return compose_tables;
   }

    ibus_compose_table_save_cache (compose_table);
    return g_slist_prepend (compose_tables, compose_table);
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


gboolean
ibus_compose_table_check (const IBusComposeTableEx *table,
                          guint16                  *compose_buffer,
                          gint                      n_compose,
                          gboolean                 *compose_finish,
                          gboolean                 *compose_match,
                          GString                  *output,
                          gboolean                  is_32bit)
{
    gint row_stride = table->max_seq_len + 2;
    guint16 *data_first;
    int n_seqs;
    guint16 *seq;

    if (compose_finish)
        *compose_finish = FALSE;
    if (compose_match)
        *compose_match = FALSE;
    if (output)
        g_string_set_size (output, 0);

    if (n_compose > table->max_seq_len)
        return FALSE;

    if (is_32bit) {
        if (!table->priv)
            return FALSE;
        data_first = table->priv->data_first;
        n_seqs = table->priv->first_n_seqs;
    } else {
        data_first = table->data;
        n_seqs = table->n_seqs;
    }
    seq = bsearch (compose_buffer,
                   data_first, n_seqs,
                   sizeof (guint16) * row_stride,
                   compare_seq);

    if (seq == NULL)
        return FALSE;

    guint16 *prev_seq;

    /* Back up to the first sequence that matches to make sure
     * we find the exact match if their is one.
     */
    while (seq > data_first) {
        prev_seq = seq - row_stride;
        if (compare_seq (compose_buffer, prev_seq) != 0)
            break;
        seq = prev_seq;
    }

    /* complete sequence */
    if (n_compose == table->max_seq_len || seq[n_compose] == 0) {
        guint16 *next_seq;
        gunichar value = 0;
        int num = 0;
        int index = 0;
        gchar *output_str = NULL;
        GError *error = NULL;

        if (is_32bit) {
            num = seq[table->max_seq_len];
            index = seq[table->max_seq_len + 1];
            value =  table->priv->data_second[index];
        } else {
            value = seq[table->max_seq_len];
        }

        if (is_32bit) {
            output_str = g_ucs4_to_utf8 (table->priv->data_second + index,
                                         num, NULL, NULL, &error);
            if (output_str) {
                if (output)
                    g_string_append (output, output_str);
                g_free (output_str);
                if (compose_match)
                    *compose_match = TRUE;
            } else {
                g_warning ("Failed to output multiple characters: %s",
                           error->message);
                g_error_free (error);
            }
        } else {
            if (output)
                g_string_append_unichar (output, value);
            if (compose_match)
                *compose_match = TRUE;
        }

        /* We found a tentative match. See if there are any longer
         * sequences containing this subsequence
         */
        next_seq = seq + row_stride;
        if (next_seq < data_first + row_stride * n_seqs) {
            if (compare_seq (compose_buffer, next_seq) == 0)
                return TRUE;
        }

        if (compose_finish)
            *compose_finish = TRUE;
        compose_buffer[0] = 0;
    }
    return TRUE;
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


/**
 * ibus_compose_table_compact_check:
 * @table: A const `IBusComposeTableCompactEx`
 * @compose_buffer: Typed compose sequence buffer
 * @n_compose: The length of `compose_buffer`
 * @compose_finish: If %TRUE, `output_chars` should be committed
 * @output_chars: An array of gunichar of output compose characters
 *
 * output_chars is better to use gunichar instead of GString because
 * IBusComposeData->values[] is the gunichar array.
 */
gboolean
ibus_compose_table_compact_check (const IBusComposeTableCompactEx *table,
                                  guint16
                                                               *compose_buffer,
                                  gint                             n_compose,
                                  gboolean
                                                               *compose_finish,
                                  gunichar                       **output_chars)
{
    gint row_stride;
    guint16 *seq_index;
    guint16 *seq;
    gint i;

    if (compose_finish)
        *compose_finish = FALSE;
    if (output_chars)
        *output_chars = NULL;

    /* Will never match, if the sequence in the compose buffer is longer
     * than the sequences in the table.  Further, compare_seq (key, val)
     * will overrun val if key is longer than val. */
    if (n_compose > table->max_seq_len)
        return FALSE;

    seq_index = bsearch (compose_buffer,
                         table->data,
                         table->n_index_size,
                         sizeof (guint16) *  table->n_index_stride,
                         compare_seq_index);

    if (seq_index == NULL)
        return FALSE;

    if (n_compose == 1)
        return TRUE;

    seq = NULL;

    if (table->priv) {
        for (i = n_compose - 1; i < table->max_seq_len; i++) {
            row_stride = i + 2;

            if (seq_index[i + 1] - seq_index[i] > 0) {
                seq = bsearch (compose_buffer + 1,
                               table->data + seq_index[i],
                               (seq_index[i + 1] - seq_index[i]) / row_stride,
                               sizeof (guint16) * row_stride,
                               compare_seq);
                if (seq) {
                    if (i == n_compose - 1)
                        break;
                    else
                        return TRUE;
                }
            }
        }
        if (!seq) {
            return FALSE;
        } else {
            int index = seq[row_stride - 2];
            int length = seq[row_stride - 1];
            int j;
            if (compose_finish)
                *compose_finish = TRUE;
            if (output_chars) {
                *output_chars = g_new (gunichar, length + 1);
                for (j = 0; j < length; j++) {
                    (*output_chars)[j] = table->priv->data2[index + j];
                }
                (*output_chars)[length] = 0;
            }

            return TRUE;
        }
    } else {
        for (i = n_compose - 1; i < table->max_seq_len; i++) {
            row_stride = i + 1;

            if (seq_index[i + 1] - seq_index[i] > 0) {
                seq = bsearch (compose_buffer + 1,
                               table->data + seq_index[i],
                               (seq_index[i + 1] - seq_index[i]) / row_stride,
                               sizeof (guint16) * row_stride,
                               compare_seq);

                if (seq) {
                    if (i == n_compose - 1)
                        break;
                    else
                        return TRUE;
                }
            }
        }
        if (!seq) {
            return FALSE;
        } else {
            if (compose_finish)
                *compose_finish = TRUE;
            if (output_chars) {
                *output_chars = g_new (gunichar, 2);
                (*output_chars)[0] = seq[row_stride - 1];
                (*output_chars)[1] = 0;
            }

            return TRUE;
        }
    }

    g_assert_not_reached ();
}


/* Checks if a keysym is a dead key. Dead key keysym values are defined in
 * ../gdk/gdkkeysyms.h and the first is GDK_KEY_dead_grave. As X.Org is updated,
 * more dead keys are added and we need to update the upper limit.
 * Currently, the upper limit is GDK_KEY_dead_dasia+1. The +1 has to do with
 * a temporary issue in the X.Org header files.
 * In future versions it will be just the keysym (no +1).
 */
#define IS_DEAD_KEY(k) \
      ((k) >= IBUS_KEY_dead_grave && (k) <= IBUS_KEY_dead_greek)

/* This function receives a sequence of Unicode characters and tries to
 * normalize it (NFC). We check for the case the the resulting string
 * has length 1 (single character).
 * NFC normalisation normally rearranges diacritic marks, unless these
 * belong to the same Canonical Combining Class.
 * If they belong to the same canonical combining class, we produce all
 * permutations of the diacritic marks, then attempt to normalize.
 */
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

    /* Xorg reuses dead_tilde for the perispomeni diacritic mark.
     * We check if base character belongs to Greek Unicode block,
     * and if so, we replace tilde with perispomeni. */
    if (combination_buffer[0] >= 0x390 && combination_buffer[0] <= 0x3FF) {
        for (i = 1; i < n_compose; i++ )
            if (combination_buffer[i] == 0x303)
                combination_buffer[i] = 0x342;
    }

    memcpy (combination_buffer_temp,
            combination_buffer,
            IBUS_MAX_COMPOSE_LEN * sizeof (gunichar) );

    for (i = 0; i < n_combinations; i++ ) {
        g_unicode_canonical_ordering (combination_buffer_temp, n_compose);
        combination_utf8_temp = g_ucs4_to_utf8 (combination_buffer_temp, -1,
                                                NULL, NULL, NULL);
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
            gint j = i % (n_compose - 1) + 1;
            gint k = (i+1) % (n_compose - 1) + 1;
            if (j >= IBUS_MAX_COMPOSE_LEN) {
                g_warning ("j >= IBUS_MAX_COMPOSE_LEN for " \
                           "combination_buffer_temp");
                break;
            }
            if (k >= IBUS_MAX_COMPOSE_LEN) {
                g_warning ("k >= IBUS_MAX_COMPOSE_LEN for " \
                           "combination_buffer_temp");
                break;
            }
            temp_swap = combination_buffer_temp[j];
            combination_buffer_temp[j] = combination_buffer_temp[k];
            combination_buffer_temp[k] = temp_swap;
        } else {
            break;
        }
    }

    return FALSE;
}


gboolean
ibus_check_algorithmically (const guint16 *compose_buffer,
                            gint           n_compose,
                            gunichar      *output_char)

{
    gint i;
    gunichar combination_buffer[IBUS_MAX_COMPOSE_LEN];
    gchar *combination_utf8, *nfc;

    if (output_char)
        *output_char = 0;

    if (n_compose >= IBUS_MAX_COMPOSE_LEN)
        return FALSE;

    for (i = 0; i < n_compose && IS_DEAD_KEY (compose_buffer[i]); i++)
        ;
    if (i == n_compose)
        return TRUE;

    if (i > 0 && i == n_compose - 1) {
        combination_buffer[0] = ibus_keyval_to_unicode (compose_buffer[i]);
        combination_buffer[n_compose] = 0;
        i--;
        while (i >= 0) {
            combination_buffer[i+1] = ibus_keysym_to_unicode (compose_buffer[i],
                                                              TRUE);
            if (!combination_buffer[i+1]) {
                combination_buffer[i+1] =
                        ibus_keyval_to_unicode (compose_buffer[i]);
            }
            i--;
        }

        /* If the buffer normalizes to a single character,
         * then modify the order of combination_buffer accordingly, if
         * necessary, and return TRUE.
         */
        if (check_normalize_nfc (combination_buffer, n_compose)) {
            combination_utf8 = g_ucs4_to_utf8 (combination_buffer, -1,
                                               NULL, NULL, NULL);
            nfc = g_utf8_normalize (combination_utf8, -1, G_NORMALIZE_NFC);

            if (output_char)
                *output_char = g_utf8_get_char (nfc);

            g_free (combination_utf8);
            g_free (nfc);

            return TRUE;
        }
    }

    return FALSE;
}


gunichar
ibus_keysym_to_unicode (guint16  keysym,
                        gboolean combining) {
#define CASE(keysym_suffix, unicode)                                    \
        case IBUS_KEY_dead_##keysym_suffix: return unicode
#define CASE_COMBINE(keysym_suffix, combined_unicode, isolated_unicode) \
        case IBUS_KEY_dead_##keysym_suffix:                             \
            if (combining)                                              \
                return combined_unicode;                                \
            else                                                        \
                return isolated_unicode
    switch (keysym) {
    CASE (a, 0x03041);
    CASE (A, 0x03042);
    CASE (i, 0x03043);
    CASE (I, 0x03044);
    CASE (u, 0x03045);
    CASE (U, 0x03046);
    CASE (e, 0x03047);
    CASE (E, 0x03048);
    CASE (o, 0x03049);
    CASE (O, 0x0304A);
    CASE         (abovecomma,                   0x0313);
    CASE_COMBINE (abovedot,                     0x0307, 0x02D9);
    CASE         (abovereversedcomma,           0x0314);
    CASE_COMBINE (abovering,                    0x030A, 0x02DA);
    CASE_COMBINE (acute,                        0x0301, 0x00B4);
    CASE         (belowbreve,                   0x032E);
    CASE_COMBINE (belowcircumflex,              0x032D, 0xA788);
    CASE_COMBINE (belowcomma,                   0x0326, 0x002C);
    CASE         (belowdiaeresis,               0x0324);
    CASE_COMBINE (belowdot,                     0x0323, 0x002E);
    CASE_COMBINE (belowmacron,                  0x0331, 0x02CD);
    CASE_COMBINE (belowring,                    0x030A, 0x02F3);
    CASE_COMBINE (belowtilde,                   0x0330, 0x02F7);
    CASE_COMBINE (breve,                        0x0306, 0x02D8);
    CASE_COMBINE (capital_schwa,                0x018F, 0x04D8);
    CASE_COMBINE (caron,                        0x030C, 0x02C7);
    CASE_COMBINE (cedilla,                      0x0327, 0x00B8);
    CASE_COMBINE (circumflex,                   0x0302, 0x005E);
    CASE         (currency,                     0x00A4);
    // IBUS_KEY_dead_dasia == IBUS_KEY_dead_abovereversedcomma
    CASE_COMBINE (diaeresis,                    0x0308, 0x00A8);
    CASE_COMBINE (doubleacute,                  0x030B, 0x02DD);
    CASE_COMBINE (doublegrave,                  0x030F, 0x02F5);
    CASE_COMBINE (grave,                        0x0300, 0x0060);
    CASE         (greek,                        0x03BC);
    CASE         (hook,                         0x0309);
    CASE         (horn,                         0x031B);
    CASE         (invertedbreve,                0x032F);
    CASE_COMBINE (iota,                         0x0345, 0x037A);
    CASE_COMBINE (macron,                       0x0304, 0x00AF);
    CASE_COMBINE (ogonek,                       0x0328, 0x02DB);
    // IBUS_KEY_dead_perispomeni == IBUS_KEY_dead_tilde
    // IBUS_KEY_dead_psili == IBUS_KEY_dead_abovecomma
    CASE_COMBINE (semivoiced_sound,             0x309A, 0x309C);
    CASE_COMBINE (small_schwa,                  0x1D4A, 0x04D9);
    CASE         (stroke,                       0x002F);
    CASE_COMBINE (tilde,                        0x0303, 0x007E);
    CASE_COMBINE (voiced_sound,                 0x3099, 0x309B);
    case IBUS_KEY_Multi_key:
        /* We only show the Compose key visibly when it is the
         * only glyph in the preedit, or when it occurs in the
         * middle of the sequence. Sadly, the official character,
         * U+2384, COMPOSITION SYMBOL, is bit too distracting, so
         * we use U+00B7, MIDDLE DOT.
         */
        return 0x00B7;
    default:;
    }
    return 0x0;
#undef CASE
#undef CASE_COMBINE
}
