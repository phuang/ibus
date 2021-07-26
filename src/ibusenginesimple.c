/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2014 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2015-2021 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2014-2017 Red Hat, Inc.
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
#  include "config.h"
#endif

#include "ibuscomposetable.h"
#include "ibusemoji.h"
#include "ibusenginesimple.h"
#include "ibusenginesimpleprivate.h"

#include "ibuskeys.h"
#include "ibuskeysyms.h"
#include "ibusutil.h"

/* This file contains the table of the compose sequences,
 * static const guint16 gtk_compose_seqs_compact[] = {}
 * It is generated from the compose-parse.py script.
 */
#include "gtkimcontextsimpleseqs.h"

#include <memory.h>
#include <stdlib.h>

#define X11_DATADIR X11_DATA_PREFIX "/share/X11/locale"
#define IBUS_ENGINE_SIMPLE_GET_PRIVATE(o)  \
   ((IBusEngineSimplePrivate *)ibus_engine_simple_get_instance_private (o))

#define SET_COMPOSE_BUFFER_ELEMENT_NEXT(buffer, index, value) {         \
    if ((index) >= COMPOSE_BUFFER_SIZE &&                               \
        COMPOSE_BUFFER_SIZE < IBUS_MAX_COMPOSE_LEN) {                   \
        COMPOSE_BUFFER_SIZE = ((index) + 10) < IBUS_MAX_COMPOSE_LEN     \
                              ? ((index) + 10) : IBUS_MAX_COMPOSE_LEN;  \
        (buffer) = g_renew (guint16, (buffer), COMPOSE_BUFFER_SIZE + 1);\
    }                                                                   \
    if ((index) < COMPOSE_BUFFER_SIZE) {                                \
        (buffer)[(index)] = (value);                                    \
        (index) += 1;                                                   \
    }                                                                   \
}

#define SET_COMPOSE_BUFFER_ELEMENT_END(buffer, index, value) {          \
    if ((index) > COMPOSE_BUFFER_SIZE)                                  \
        (index) = COMPOSE_BUFFER_SIZE;                                  \
    (buffer)[(index)] = (value);                                        \
}

#define CHECK_COMPOSE_BUFFER_LENGTH(index) {                            \
    if ((index) > COMPOSE_BUFFER_SIZE)                                  \
        (index) = COMPOSE_BUFFER_SIZE;                                  \
}

typedef struct {
    GHashTable *dict;
    int         max_seq_len;
} IBusEngineDict;

struct _IBusEngineSimplePrivate {
    guint16            *compose_buffer;
    gunichar            tentative_match;
    char               *tentative_emoji;
    int                 tentative_match_len;

    guint               hex_mode_enabled : 1;
    guint               in_hex_sequence : 1;
    guint               in_emoji_sequence : 1;
    guint               modifiers_dropped : 1;
    IBusEngineDict     *emoji_dict;
    IBusLookupTable    *lookup_table;
    gboolean            lookup_table_visible;
};

/* From the values below, the value 30 means the number of different first keysyms
 * that exist in the Compose file (from Xorg). When running compose-parse.py without
 * parameters, you get the count that you can put here. Needed when updating the
 * gtkimcontextsimpleseqs.h header file (contains the compose sequences).
 * Assign the value of "Number of different first items" of compose-parse.py
 * to n_seqs in IBusComposeTableCompact
 */
IBusComposeTableCompactPrivate ibus_compose_table_compact_32bit_priv = {
    gtk_compose_seqs_compact_32bit_second
};

const IBusComposeTableCompactEx ibus_compose_table_compact = {
    NULL,
    gtk_compose_seqs_compact,
    5,
    30,
    6
};

const IBusComposeTableCompactEx ibus_compose_table_compact_32bit = {
    &ibus_compose_table_compact_32bit_priv,
    gtk_compose_seqs_compact_32bit_first,
    5,
    9,
    6
};

guint COMPOSE_BUFFER_SIZE = 20;
G_LOCK_DEFINE_STATIC (global_tables);
static GSList *global_tables;

/* functions prototype */
static void     ibus_engine_simple_destroy      (IBusEngineSimple   *simple);
static void     ibus_engine_simple_focus_in     (IBusEngine         *engine);
static void     ibus_engine_simple_focus_out    (IBusEngine         *engine);
static void     ibus_engine_simple_reset        (IBusEngine         *engine);
static gboolean ibus_engine_simple_process_key_event
                                                (IBusEngine         *engine,
                                                 guint               keyval,
                                                 guint               keycode,
                                                 guint               modifiers);
static void     ibus_engine_simple_page_down   (IBusEngine          *engine);
static void     ibus_engine_simple_page_up     (IBusEngine          *engine);
static void     ibus_engine_simple_candidate_clicked
                                               (IBusEngine          *engine,
                                                guint                index,
                                                guint                button,
                                                guint                state);
static void     ibus_engine_simple_commit_char (IBusEngineSimple    *simple,
                                                gunichar             ch);
static void     ibus_engine_simple_commit_str  (IBusEngineSimple    *simple,
                                                const char          *str);
static void     ibus_engine_simple_update_preedit_text
                                               (IBusEngineSimple    *simple);

G_DEFINE_TYPE_WITH_PRIVATE (IBusEngineSimple,
                            ibus_engine_simple,
                            IBUS_TYPE_ENGINE)

static void
ibus_engine_simple_class_init (IBusEngineSimpleClass *class)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (class);
    IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (class);

    ibus_object_class->destroy =
        (IBusObjectDestroyFunc) ibus_engine_simple_destroy;

    engine_class->focus_in  = ibus_engine_simple_focus_in;
    engine_class->focus_out = ibus_engine_simple_focus_out;
    engine_class->reset     = ibus_engine_simple_reset;
    engine_class->process_key_event
                            = ibus_engine_simple_process_key_event;
    engine_class->page_down = ibus_engine_simple_page_down;
    engine_class->page_up   = ibus_engine_simple_page_up;
    engine_class->candidate_clicked
                            = ibus_engine_simple_candidate_clicked;
}

static void
ibus_engine_simple_init (IBusEngineSimple *simple)
{
    simple->priv = IBUS_ENGINE_SIMPLE_GET_PRIVATE (simple);
    simple->priv->compose_buffer = g_new0(guint16, COMPOSE_BUFFER_SIZE + 1);
    simple->priv->hex_mode_enabled =
        g_getenv("IBUS_ENABLE_CTRL_SHIFT_U") != NULL ||
        g_getenv("IBUS_ENABLE_CONTROL_SHIFT_U") != NULL;
}


static void
ibus_engine_simple_destroy (IBusEngineSimple *simple)
{
    IBusEngineSimplePrivate *priv = simple->priv;

    if (priv->emoji_dict) {
        if (priv->emoji_dict->dict)
            g_clear_pointer (&priv->emoji_dict->dict, g_hash_table_destroy);
        g_slice_free (IBusEngineDict, priv->emoji_dict);
        priv->emoji_dict = NULL;
    }

    g_clear_object (&priv->lookup_table);
    g_clear_pointer (&priv->compose_buffer, g_free);
    g_clear_pointer (&priv->tentative_emoji, g_free);

    IBUS_OBJECT_CLASS(ibus_engine_simple_parent_class)->destroy (
        IBUS_OBJECT (simple));
}

static void
ibus_engine_simple_focus_in (IBusEngine *engine)
{
    IBUS_ENGINE_CLASS (ibus_engine_simple_parent_class)->focus_in (engine);
}

static void
ibus_engine_simple_focus_out (IBusEngine *engine)
{
    ibus_engine_simple_reset (engine);
    IBUS_ENGINE_CLASS (ibus_engine_simple_parent_class)->focus_out (engine);
}

static void
ibus_engine_simple_reset (IBusEngine *engine)
{
    IBusEngineSimple *simple = (IBusEngineSimple *)engine;
    IBusEngineSimplePrivate *priv = simple->priv;

    priv->compose_buffer[0] = 0;

    if (priv->tentative_match || priv->in_hex_sequence) {
        priv->in_hex_sequence = FALSE;
        priv->tentative_match = 0;
        priv->tentative_match_len = 0;
    } else if (priv->tentative_emoji || priv->in_emoji_sequence) {
        priv->in_emoji_sequence = FALSE;
        g_clear_pointer (&priv->tentative_emoji, g_free);
    } else if (!priv->in_hex_sequence && !priv->in_emoji_sequence) {
        priv->tentative_match = 0;
        priv->tentative_match_len = 0;
    }
    ibus_engine_hide_preedit_text ((IBusEngine *)simple);
}

static void
ibus_engine_simple_commit_char (IBusEngineSimple *simple,
                                gunichar          ch)
{
    g_return_if_fail (g_unichar_validate (ch));

    IBusEngineSimplePrivate *priv = simple->priv;

    if (priv->tentative_match || priv->in_hex_sequence) {
        priv->in_hex_sequence = FALSE;
        priv->tentative_match = 0;
        priv->tentative_match_len = 0;
    }
    if (priv->tentative_emoji || priv->in_emoji_sequence) {
        priv->in_emoji_sequence = FALSE;
        g_clear_pointer (&priv->tentative_emoji, g_free);
    }
    ibus_engine_commit_text ((IBusEngine *)simple,
            ibus_text_new_from_unichar (ch));
}


static void
ibus_engine_simple_commit_str (IBusEngineSimple *simple,
                               const char       *str)
{
    IBusEngineSimplePrivate *priv = simple->priv;
    char *backup_str;

    g_return_if_fail (str && *str);

    backup_str = g_strdup (str);

    if (priv->tentative_match || priv->in_hex_sequence) {
        priv->in_hex_sequence = FALSE;
        priv->tentative_match = 0;
        priv->tentative_match_len = 0;
    }
    if (priv->tentative_emoji || priv->in_emoji_sequence) {
        priv->in_emoji_sequence = FALSE;
        g_clear_pointer (&priv->tentative_emoji, g_free);
    }

    ibus_engine_commit_text ((IBusEngine *)simple,
            ibus_text_new_from_string (backup_str));
    g_free (backup_str);
}

static void
ibus_engine_simple_update_preedit_text (IBusEngineSimple *simple)
{
    IBusEngineSimplePrivate *priv = simple->priv;

    GString *s = g_string_new ("");

    if (priv->in_hex_sequence || priv->in_emoji_sequence) {
        int hexchars = 0;

        if (priv->in_hex_sequence)
            g_string_append_c (s, 'u');
        else
            g_string_append_c (s, '@');

        while (priv->compose_buffer[hexchars] != 0) {
            g_string_append_unichar(
                    s,
                    ibus_keyval_to_unicode (priv->compose_buffer[hexchars++])
            );
        }
    } else if (priv->tentative_match) {
        g_string_append_unichar(s, priv->tentative_match);
    } else if (priv->tentative_emoji && *priv->tentative_emoji) {
        IBusText *text = ibus_text_new_from_string (priv->tentative_emoji);
        int len = strlen (priv->tentative_emoji);
        ibus_text_append_attribute (text,
                IBUS_ATTR_TYPE_UNDERLINE, IBUS_ATTR_UNDERLINE_SINGLE, 0, len);
        ibus_engine_update_preedit_text ((IBusEngine *)simple, text, len, TRUE);
        return;
    } else {
        int hexchars = 0;
        while (priv->compose_buffer[hexchars] != 0) {
            guint16 keysym = priv->compose_buffer[hexchars];
            gunichar unichar = ibus_keysym_to_unicode (keysym, FALSE);
            if (unichar > 0) {
                g_string_append_unichar(s, unichar);
            } else {
                unichar = ibus_keyval_to_unicode (keysym);
                g_string_append_unichar(s, unichar);
            }
            if (!unichar) {
                g_warning (
                        "Not found alternative character of compose key 0x%X",
                        priv->compose_buffer[hexchars]);
            }
            ++hexchars;
        }
    }

    if (s->len == 0) {
        ibus_engine_hide_preedit_text ((IBusEngine *)simple);
    } else if (s->len >= G_MAXINT) {
        g_warning ("%s is too long compose length: %lu", s->str, s->len);
    } else {
        int len = (int)s->len;
        IBusText *text = ibus_text_new_from_string (s->str);
        ibus_text_append_attribute (text,
                IBUS_ATTR_TYPE_UNDERLINE, IBUS_ATTR_UNDERLINE_SINGLE, 0, len);
        ibus_engine_update_preedit_text ((IBusEngine *)simple, text, len, TRUE);
    }
    g_string_free (s, TRUE);
}


/* In addition to the table-driven sequences, we allow Unicode hex
 * codes to be entered. The method chosen here is similar to the
 * one recommended in ISO 14755, but not exactly the same, since we
 * don't want to steal 16 valuable key combinations.
 *
 * A hex Unicode sequence must be started with Ctrl-Shift-U, followed
 * by a sequence of hex digits entered with Ctrl-Shift still held.
 * Releasing one of the modifiers or pressing space while the modifiers
 * are still held commits the character. It is possible to erase
 * digits using backspace.
 *
 * As an extension to the above, we also allow to start the sequence
 * with Ctrl-Shift-U, then release the modifiers before typing any
 * digits, and enter the digits without modifiers.
 */
#define HEX_MOD_MASK (IBUS_CONTROL_MASK | IBUS_SHIFT_MASK)

static gboolean
check_hex (IBusEngineSimple *simple,
           int               n_compose)
{
    IBusEngineSimplePrivate *priv = simple->priv;

    int i;
    GString *str;
    gulong n;
    char *nptr = NULL;
    char buf[7];

    CHECK_COMPOSE_BUFFER_LENGTH (n_compose);

    priv->tentative_match = 0;
    priv->tentative_match_len = 0;

    str = g_string_new (NULL);

    i = 0;
    while (i < n_compose) {
        gunichar ch;

        ch = ibus_keyval_to_unicode (priv->compose_buffer[i]);

        if (ch == 0) {
            g_string_free (str, TRUE);
            return FALSE;
        }

        if (!g_unichar_isxdigit (ch)) {
            g_string_free (str, TRUE);
            return FALSE;
        }

        buf[g_unichar_to_utf8 (ch, buf)] = '\0';

        g_string_append (str, buf);

        ++i;
    }

    n = strtoul (str->str, &nptr, 16);

    /* if strtoul fails it probably means non-latin digits were used;
     * we should in principle handle that, but we probably don't.
     */
    if (nptr - str->str < str->len) {
        g_string_free (str, TRUE);
        return FALSE;
    } else {
        g_string_free (str, TRUE);
    }

    if (g_unichar_validate (n)) {
        priv->tentative_match = n;
        priv->tentative_match_len = n_compose;
    }

    return TRUE;
}

static IBusEngineDict *
load_emoji_dict ()
{
    IBusEngineDict *emoji_dict;
    GList *keys;
    int max_length = 0;

    emoji_dict = g_slice_new0 (IBusEngineDict);
    emoji_dict->dict = ibus_emoji_dict_load (IBUS_DATA_DIR "/dicts/emoji.dict");
    if (!emoji_dict->dict)
        return emoji_dict;

    keys = g_hash_table_get_keys (emoji_dict->dict);
    for (; keys; keys = keys->next) {
        int length = strlen (keys->data);
        if (max_length < length)
            max_length = length;
    }
    emoji_dict->max_seq_len = max_length;

    return emoji_dict;
}

static gboolean
check_emoji_table (IBusEngineSimple       *simple,
                   int                     n_compose,
                   int                     index)
{
    IBusEngineSimplePrivate *priv = simple->priv;
    IBusEngineDict *emoji_dict = priv->emoji_dict;
    GString *str = NULL;
    int i;
    char buf[7];
    GSList *words = NULL;

    g_assert (IBUS_IS_ENGINE_SIMPLE (simple));

    CHECK_COMPOSE_BUFFER_LENGTH (n_compose);

    if (priv->lookup_table == NULL) {
        priv->lookup_table = ibus_lookup_table_new (10, 0, TRUE, TRUE);
        g_object_ref_sink (priv->lookup_table);
    }
    if (emoji_dict == NULL)
        emoji_dict = priv->emoji_dict = load_emoji_dict (simple);

    if (emoji_dict == NULL || emoji_dict->dict == NULL)
        return FALSE;

    if (n_compose > emoji_dict->max_seq_len)
        return FALSE;

    str = g_string_new (NULL);
    priv->lookup_table_visible = FALSE;

    i = 0;
    while (i < n_compose) {
        gunichar ch;

        ch = ibus_keyval_to_unicode (priv->compose_buffer[i]);

        if (ch == 0) {
            g_string_free (str, TRUE);
            return FALSE;
        }

        if (!g_unichar_isprint (ch)) {
            g_string_free (str, TRUE);
            return FALSE;
        }

        buf[g_unichar_to_utf8 (ch, buf)] = '\0';

        g_string_append (str, buf);

        ++i;
    }

    if (str->str) {
        words = g_hash_table_lookup (emoji_dict->dict, str->str);
    }
    g_string_free (str, TRUE);

    if (words != NULL) {
        int i = 0;
        ibus_lookup_table_clear (priv->lookup_table);
        priv->lookup_table_visible = TRUE;

        while (words) {
            if (i == index) {
                g_clear_pointer (&priv->tentative_emoji, g_free);
                priv->tentative_emoji = g_strdup (words->data);
            }
            IBusText *text = ibus_text_new_from_string (words->data);
            ibus_lookup_table_append_candidate (priv->lookup_table, text);
            words = words->next;
            i++;
        }
        return TRUE;
    }

    return FALSE;
}


static gboolean
no_sequence_matches (IBusEngineSimple *simple,
                     int               n_compose,
                     guint             keyval,
                     guint             keycode,
                     guint             modifiers)
{
    IBusEngineSimplePrivate *priv = simple->priv;

    gunichar ch;

    CHECK_COMPOSE_BUFFER_LENGTH (n_compose);

    /* No compose sequences found, check first if we have a partial
     * match pending.
     */
    if (priv->tentative_match) {
        int len = priv->tentative_match_len;
        int i;

        ibus_engine_simple_commit_char (simple, priv->tentative_match);
        priv->compose_buffer[0] = 0;
        ibus_engine_simple_update_preedit_text (simple);

        for (i=0; i < n_compose - len - 1; i++) {
            ibus_engine_simple_process_key_event (
                    (IBusEngine *)simple,
                    priv->compose_buffer[len + i],
                    0, 0);
        }

        return ibus_engine_simple_process_key_event (
                (IBusEngine *)simple, keyval, keycode, modifiers);
    } else if (priv->tentative_emoji && *priv->tentative_emoji) {
        ibus_engine_simple_commit_str (simple, priv->tentative_emoji);
        priv->compose_buffer[0] = 0;
        ibus_engine_simple_update_preedit_text (simple);
    } else {
        priv->compose_buffer[0] = 0;
        if (n_compose > 1) {
            /* Invalid sequence */
            // FIXME beep_window (event->window);
            ibus_engine_simple_update_preedit_text (simple);
            return TRUE;
        }

        ibus_engine_simple_update_preedit_text (simple);
        ch = ibus_keyval_to_unicode (keyval);
        /* IBUS_CHANGE: RH#769133
         * Since we use ibus xkb engines as the disable state,
         * do not commit the characters locally without in_hex_sequence. */
        if (ch != 0 && !g_unichar_iscntrl (ch) &&
            priv->in_hex_sequence) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
    return FALSE;
}

static gboolean
is_hex_keyval (guint keyval)
{
  gunichar ch = ibus_keyval_to_unicode (keyval);

  return g_unichar_isxdigit (ch);
}

static gboolean
is_graph_keyval (guint keyval)
{
  gunichar ch = ibus_keyval_to_unicode (keyval);

  return g_unichar_isgraph (ch);
}

static void
ibus_engine_simple_update_lookup_and_aux_table (IBusEngineSimple *simple)
{
    IBusEngineSimplePrivate *priv;
    guint index, candidates;
    char *aux_label = NULL;
    IBusText *text = NULL;

    g_return_if_fail (IBUS_IS_ENGINE_SIMPLE (simple));

    priv = simple->priv;
    index = ibus_lookup_table_get_cursor_pos (priv->lookup_table) + 1;
    candidates = ibus_lookup_table_get_number_of_candidates(priv->lookup_table);
    aux_label = g_strdup_printf ("(%u / %u)", index, candidates);
    text = ibus_text_new_from_string (aux_label);
    g_free (aux_label);

    ibus_engine_update_auxiliary_text (IBUS_ENGINE (simple),
                                       text,
                                       priv->lookup_table_visible);
    ibus_engine_update_lookup_table (IBUS_ENGINE (simple),
                                     priv->lookup_table,
                                     priv->lookup_table_visible);
}

static gboolean
ibus_engine_simple_if_in_range_of_lookup_table (IBusEngineSimple *simple,
                                                guint             keyval)
{
    IBusEngineSimplePrivate *priv;
    int index, candidates, cursor_pos, cursor_in_page, page_size;

    priv = simple->priv;

    if (priv->lookup_table == NULL || !priv->lookup_table_visible)
        return FALSE;
    if (keyval < IBUS_KEY_0 || keyval > IBUS_KEY_9)
        return FALSE;
    if (keyval == IBUS_KEY_0)
        keyval = IBUS_KEY_9 + 1;
    index = keyval - IBUS_KEY_1;
    candidates =
            ibus_lookup_table_get_number_of_candidates (priv->lookup_table);
    cursor_pos = ibus_lookup_table_get_cursor_pos (priv->lookup_table);
    cursor_in_page = ibus_lookup_table_get_cursor_in_page (priv->lookup_table);
    page_size = ibus_lookup_table_get_page_size (priv->lookup_table);
    if (index > ((candidates - (cursor_pos - cursor_in_page)) % page_size))
        return FALSE;
    return TRUE;
}

static void
ibus_engine_simple_set_number_on_lookup_table (IBusEngineSimple *simple,
                                               guint             keyval,
                                               int               n_compose)
{
    IBusEngineSimplePrivate *priv;
    int index, cursor_pos, cursor_in_page, real_index;

    priv = simple->priv;

    if (keyval == IBUS_KEY_0)
        keyval = IBUS_KEY_9 + 1;
    index = keyval - IBUS_KEY_1;
    cursor_pos = ibus_lookup_table_get_cursor_pos (priv->lookup_table);
    cursor_in_page = ibus_lookup_table_get_cursor_in_page (priv->lookup_table);
    real_index = cursor_pos - cursor_in_page + index;

    ibus_lookup_table_set_cursor_pos (priv->lookup_table, real_index);
    check_emoji_table (simple, n_compose, real_index);
    priv->lookup_table_visible = FALSE;
    ibus_engine_simple_update_lookup_and_aux_table (simple);

    if (priv->tentative_emoji && *priv->tentative_emoji) {
        ibus_engine_simple_commit_str (simple, priv->tentative_emoji);
        priv->compose_buffer[0] = 0;
    } else {
        g_clear_pointer (&priv->tentative_emoji, g_free);
        priv->in_emoji_sequence = FALSE;
        priv->compose_buffer[0] = 0;
    }

    ibus_engine_simple_update_preedit_text (simple);
}

static gboolean
ibus_engine_simple_check_all_compose_table (IBusEngineSimple *simple,
                                            int               n_compose)
{
    IBusEngineSimplePrivate *priv = simple->priv;
    gboolean compose_finish = FALSE;
    gboolean compose_match = FALSE;
    GString *output = g_string_new ("");
    gboolean does_hit = FALSE;
    gboolean is_32bit = FALSE;
    GSList *tmp_list;
    gunichar *output_chars = NULL;
    gunichar output_char;

    G_LOCK (global_tables);
    tmp_list = global_tables;
    while (tmp_list) {
        is_32bit = FALSE;
        if (ibus_compose_table_check (
            (IBusComposeTableEx *)tmp_list->data,
            priv->compose_buffer,
            n_compose,
            &compose_finish,
            &compose_match,
            output,
            is_32bit)) {
            does_hit = TRUE;
            break;
        }
        is_32bit = TRUE;
        if (ibus_compose_table_check (
            (IBusComposeTableEx *)tmp_list->data,
            priv->compose_buffer,
            n_compose,
            &compose_finish,
            &compose_match,
            output,
            is_32bit)) {
            does_hit = TRUE;
            break;
        }
        tmp_list = tmp_list->next;
    }
    G_UNLOCK (global_tables);

    if (does_hit) {
        if (compose_finish) {
            if (compose_match) {
                if (is_32bit) {
                    ibus_engine_simple_commit_str (simple, output->str);
                } else {
                    ibus_engine_simple_commit_char (simple,
                                                    g_utf8_get_char (output->str));
                }
            }
        } else if (compose_match) {
            priv->tentative_match = g_utf8_get_char (output->str);
            priv->tentative_match_len = n_compose;
        }
        ibus_engine_simple_update_preedit_text (simple);
        g_string_free (output, TRUE);
        return TRUE;
    }
    g_string_free (output, TRUE);

    if (ibus_compose_table_compact_check (&ibus_compose_table_compact,
                                          priv->compose_buffer,
                                          n_compose,
                                          &compose_finish,
                                          &output_chars)) {
        if (compose_finish) {
            ibus_engine_simple_commit_char (simple, *output_chars);
            g_free (output_chars);
            priv->compose_buffer[0] = 0;
        }
        ibus_engine_simple_update_preedit_text (simple);
        return TRUE;
    }
    if (ibus_compose_table_compact_check (&ibus_compose_table_compact_32bit,
                                          priv->compose_buffer,
                                          n_compose,
                                          &compose_finish,
                                          &output_chars)) {
        if (compose_finish) {
            GError *error = NULL;
            char *str = g_ucs4_to_utf8 (output_chars, -1, NULL, NULL, &error);
            if (str) {
                ibus_engine_simple_commit_str (simple, str);
                g_free (str);
            } else {
                g_warning ("Failed to output multiple characters: %s",
                           error->message);
                g_error_free (error);
            }
            priv->compose_buffer[0] = 0;
        }
        g_free (output_chars);
        ibus_engine_simple_update_preedit_text (simple);
        return TRUE;
    }
    g_assert (!output_chars);
    if (ibus_check_algorithmically (priv->compose_buffer,
                                    n_compose,
                                    &output_char)) {
        if (output_char) {
            ibus_engine_simple_commit_char (simple, output_char);
            priv->compose_buffer[0] = 0;
        }
        ibus_engine_simple_update_preedit_text (simple);
        return TRUE;
    }

    return FALSE;
}

static gboolean
ibus_engine_simple_process_key_event (IBusEngine *engine,
                                      guint       keyval,
                                      guint       keycode,
                                      guint       modifiers)
{
    IBusEngineSimple *simple = (IBusEngineSimple *)engine;
    IBusEngineSimplePrivate *priv = simple->priv;
    int n_compose = 0;
    gboolean have_hex_mods;
    gboolean is_hex_start = FALSE;
    gboolean is_emoji_start = FALSE;
    gboolean is_hex_end;
    gboolean is_space;
    gboolean is_backspace;
    gboolean is_escape;
    guint hex_keyval;
    guint printable_keyval;
    int i;

    while (n_compose <= COMPOSE_BUFFER_SIZE && priv->compose_buffer[n_compose] != 0)
        n_compose++;
    if (n_compose > COMPOSE_BUFFER_SIZE) {
        g_warning ("copmose table buffer is full.");
        n_compose = COMPOSE_BUFFER_SIZE;
    }

    if (modifiers & IBUS_RELEASE_MASK) {
        if (priv->in_hex_sequence &&
            (keyval == IBUS_KEY_Control_L || keyval == IBUS_KEY_Control_R ||
             keyval == IBUS_KEY_Shift_L || keyval == IBUS_KEY_Shift_R)) {
            if (priv->tentative_match &&
                g_unichar_validate (priv->tentative_match)) {
                ibus_engine_simple_commit_char (simple, priv->tentative_match);
                ibus_engine_simple_update_preedit_text (simple);
            } else if (n_compose == 0) {
                priv->modifiers_dropped = TRUE;
            } else {
                /* invalid hex sequence */
                /* FIXME beep_window (event->window); */
                priv->tentative_match = 0;
                g_clear_pointer (&priv->tentative_emoji, g_free);
                priv->in_hex_sequence = FALSE;
                priv->in_emoji_sequence = FALSE;
                priv->compose_buffer[0] = 0;

                ibus_engine_simple_update_preedit_text (simple);
            }

            return TRUE;
        }
        /* Handle Shift + Space */
        else if (priv->in_emoji_sequence &&
            (keyval == IBUS_KEY_Control_L || keyval == IBUS_KEY_Control_R)) {
            if (priv->tentative_emoji && *priv->tentative_emoji) {
                ibus_engine_simple_commit_str (simple, priv->tentative_emoji);
                priv->compose_buffer[0] = 0;
                ibus_engine_simple_update_preedit_text (simple);
            } else if (n_compose == 0) {
                priv->modifiers_dropped = TRUE;
            } else {
                /* invalid hex sequence */
                /* FIXME beep_window (event->window); */
                priv->tentative_match = 0;
                g_clear_pointer (&priv->tentative_emoji, g_free);
                priv->in_hex_sequence = FALSE;
                priv->in_emoji_sequence = FALSE;
                priv->compose_buffer[0] = 0;

                ibus_engine_simple_update_preedit_text (simple);
            }
        }
        else
            return FALSE;
    }

    /* Ignore modifier key presses */
    for (i = 0; i < G_N_ELEMENTS (IBUS_COMPOSE_IGNORE_KEYLIST); i++)
        if (keyval == IBUS_COMPOSE_IGNORE_KEYLIST[i])
            return FALSE;

    if ((priv->in_hex_sequence || priv->in_emoji_sequence)
        && priv->modifiers_dropped) {
        have_hex_mods = TRUE;
    } else {
        have_hex_mods = (modifiers & (HEX_MOD_MASK)) == HEX_MOD_MASK;
    }

    is_hex_start = (keyval == IBUS_KEY_U) && priv->hex_mode_enabled;
    is_hex_end = (keyval == IBUS_KEY_space ||
                  keyval == IBUS_KEY_KP_Space ||
                  keyval == IBUS_KEY_Return ||
                  keyval == IBUS_KEY_ISO_Enter ||
                  keyval == IBUS_KEY_KP_Enter);
    is_space = (keyval == IBUS_KEY_space || keyval == IBUS_KEY_KP_Space);
    is_backspace = keyval == IBUS_KEY_BackSpace;
    is_escape = keyval == IBUS_KEY_Escape;
    hex_keyval = is_hex_keyval (keyval) ? keyval : 0;
    printable_keyval = is_graph_keyval (keyval) ? keyval : 0;

    /* gtkimcontextsimple causes a buffer overflow in priv->compose_buffer.
     * Add the check code here.
     */
    if (n_compose > COMPOSE_BUFFER_SIZE &&
        (priv->in_hex_sequence || priv->in_emoji_sequence)) {
        if (is_backspace) {
            priv->compose_buffer[--n_compose] = 0;
        }
        else if (is_hex_end) {
            /* invalid hex sequence */
            // beep_window (event->window);
            priv->tentative_match = 0;
            g_clear_pointer (&priv->tentative_emoji, g_free);
            priv->in_hex_sequence = FALSE;
            priv->in_emoji_sequence = FALSE;
            priv->compose_buffer[0] = 0;
        }
        else if (is_escape) {
            ibus_engine_simple_reset (engine);
            if (priv->lookup_table != NULL && priv->lookup_table_visible) {
                priv->lookup_table_visible = FALSE;
                ibus_engine_simple_update_lookup_and_aux_table (simple);
            }
            return TRUE;
        }

        if (have_hex_mods)
            ibus_engine_simple_update_preedit_text (simple);

        return TRUE;
    }

    /* If we are already in a non-hex sequence, or
     * this keystroke is not hex modifiers + hex digit, don't filter
     * key events with accelerator modifiers held down. We only treat
     * Control and Alt as accel modifiers here, since Super, Hyper and
     * Meta are often co-located with Mode_Switch, Multi_Key or
     * ISO_Level3_Switch.
     */
    if (!have_hex_mods ||
        (n_compose > 0 && !priv->in_hex_sequence && !priv->in_emoji_sequence) ||
        (n_compose == 0 && !priv->in_hex_sequence && !is_hex_start &&
         !priv->in_emoji_sequence && !is_emoji_start) ||
        (priv->in_hex_sequence && !hex_keyval &&
         !is_hex_start && !is_hex_end && !is_escape && !is_backspace) ||
        (priv->in_emoji_sequence && !printable_keyval &&
         !is_emoji_start && !is_hex_end && !is_escape && !is_backspace)) {
        if (modifiers & (IBUS_MOD1_MASK | IBUS_CONTROL_MASK) ||
            ((priv->in_hex_sequence || priv->in_emoji_sequence) &&
             priv->modifiers_dropped &&
             (keyval == IBUS_KEY_Return ||
              keyval == IBUS_KEY_ISO_Enter ||
              keyval == IBUS_KEY_KP_Enter))) {
              return FALSE;
        }
    }

    /* Handle backspace */
    if (priv->in_hex_sequence && have_hex_mods && is_backspace) {
        if (n_compose > 0) {
            n_compose--;
            priv->compose_buffer[n_compose] = 0;
            check_hex (simple, n_compose);
        } else {
            priv->in_hex_sequence = FALSE;
        }

        ibus_engine_simple_update_preedit_text (simple);

        return TRUE;
    }
    if (priv->in_emoji_sequence && have_hex_mods && is_backspace) {
        if (n_compose > 0) {
            n_compose--;
            priv->compose_buffer[n_compose] = 0;
            check_emoji_table (simple, n_compose, -1);
            ibus_engine_simple_update_lookup_and_aux_table (simple);
        } else {
            priv->in_emoji_sequence = FALSE;
        }

        ibus_engine_simple_update_preedit_text (simple);

        return TRUE;
    }
    if (!priv->in_hex_sequence && !priv->in_emoji_sequence && is_backspace) {
        if (n_compose > 0) {
            n_compose--;
            priv->compose_buffer[n_compose] = 0;
            priv->tentative_match = 0;
            ibus_engine_simple_check_all_compose_table (simple, n_compose);
            return TRUE;
        }
    }

    /* Check for hex sequence restart */
    if (priv->in_hex_sequence && have_hex_mods && is_hex_start) {
        if (priv->tentative_match &&
            g_unichar_validate (priv->tentative_match)) {
            ibus_engine_simple_commit_char (simple, priv->tentative_match);
            ibus_engine_simple_update_preedit_text (simple);
        }
        else {
            /* invalid hex sequence */
            if (n_compose > 0) {
                // FIXME beep_window (event->window);
                priv->tentative_match = 0;
                priv->in_hex_sequence = FALSE;
                priv->compose_buffer[0] = 0;
            }
        }
    }
    if (priv->in_emoji_sequence && have_hex_mods && is_emoji_start) {
        if (priv->tentative_emoji && *priv->tentative_emoji) {
            ibus_engine_simple_commit_str (simple, priv->tentative_emoji);
            priv->compose_buffer[0] = 0;
            ibus_engine_simple_update_preedit_text (simple);
        }
        else {
            if (n_compose > 0) {
                g_clear_pointer (&priv->tentative_emoji, g_free);
                priv->in_emoji_sequence = FALSE;
                priv->compose_buffer[0] = 0;
            }
        }
    }

    /* Check for hex sequence start */
    if (!priv->in_hex_sequence && !priv->in_emoji_sequence &&
        have_hex_mods && is_hex_start) {
        priv->compose_buffer[0] = 0;
        priv->in_hex_sequence = TRUE;
        priv->in_emoji_sequence = FALSE;
        priv->modifiers_dropped = FALSE;
        priv->tentative_match = 0;
        g_clear_pointer (&priv->tentative_emoji, g_free);

        // g_debug ("Start HEX MODE");

        ibus_engine_simple_update_preedit_text (simple);

        return TRUE;
    } else if (!priv->in_hex_sequence && !priv->in_emoji_sequence &&
               have_hex_mods && is_emoji_start) {
        priv->compose_buffer[0] = 0;
        priv->in_hex_sequence = FALSE;
        priv->in_emoji_sequence = TRUE;
        priv->modifiers_dropped = FALSE;
        priv->tentative_match = 0;
        g_clear_pointer (&priv->tentative_emoji, g_free);

        // g_debug ("Start HEX MODE");

        ibus_engine_simple_update_preedit_text (simple);

        return TRUE;
    }

    /* Then, check for compose sequences */
    if (priv->in_hex_sequence) {
        if (hex_keyval) {
            SET_COMPOSE_BUFFER_ELEMENT_NEXT (priv->compose_buffer,
                                             n_compose,
                                             hex_keyval);
        } else if (is_escape) {
            // FIXME
            ibus_engine_simple_reset (engine);

            return TRUE;
        } else if (!is_hex_end) {
            // FIXME
            /* non-hex character in hex sequence */
            // beep_window (event->window);
            return TRUE;
        }
    } else if (priv->in_emoji_sequence) {
        if (printable_keyval) {
            if (!ibus_engine_simple_if_in_range_of_lookup_table (simple,
                        printable_keyval)) {
                /* digit keyval can be an index on the current lookup table
                 * but it also can be a part of an emoji annotation.
                 * E.g. "1" and "2" are  indexes of emoji "1".
                 * "100" is an annotation of the emoji "100".
                 */
                SET_COMPOSE_BUFFER_ELEMENT_NEXT (priv->compose_buffer,
                                                 n_compose,
                                                 printable_keyval);
            }
        }
        else if (is_space && (modifiers & IBUS_SHIFT_MASK)) {
            SET_COMPOSE_BUFFER_ELEMENT_NEXT (priv->compose_buffer,
                                             n_compose,
                                             IBUS_KEY_space);
        }
        else if (is_escape) {
            ibus_engine_simple_reset (engine);
            if (priv->lookup_table != NULL && priv->lookup_table_visible) {
                priv->lookup_table_visible = FALSE;
                ibus_engine_simple_update_lookup_and_aux_table (simple);
            }
            return TRUE;
        }
    } else {
        if (is_escape) {
            if (n_compose > 0) {
                ibus_engine_simple_reset (engine);
                return TRUE;
            }
        }
        SET_COMPOSE_BUFFER_ELEMENT_NEXT (priv->compose_buffer,
                                         n_compose,
                                         keyval);
    }

    SET_COMPOSE_BUFFER_ELEMENT_END (priv->compose_buffer,
                                    n_compose,
                                    0);

    if (priv->in_hex_sequence) {
        /* If the modifiers are still held down, consider the sequence again */
        if (have_hex_mods) {
            /* space or return ends the sequence, and we eat the key */
            if (n_compose > 0 && is_hex_end) {
                if (priv->tentative_match &&
                    g_unichar_validate (priv->tentative_match)) {
                    ibus_engine_simple_commit_char (simple,
                            priv->tentative_match);
                    priv->compose_buffer[0] = 0;
                    ibus_engine_simple_update_preedit_text (simple);
                } else {
                    // FIXME
                    /* invalid hex sequence */
                    // beep_window (event->window);
                    priv->tentative_match = 0;
                    priv->in_hex_sequence = FALSE;
                    priv->compose_buffer[0] = 0;
                }
            }
            else if (!check_hex (simple, n_compose))
                // FIXME
                // beep_window (event->window);
                ;
            ibus_engine_simple_update_preedit_text (simple);

            return TRUE;
        }
    }
    else if (priv->in_emoji_sequence) {
        if (have_hex_mods && n_compose > 0) {
            gboolean update_lookup_table = FALSE;

            if (priv->lookup_table_visible) {
                switch (keyval) {
                case IBUS_KEY_space:
                case IBUS_KEY_KP_Space:
                    if ((modifiers & IBUS_SHIFT_MASK) == 0) {
                        ibus_lookup_table_cursor_down (priv->lookup_table);
                        update_lookup_table = TRUE;
                    }
                    break;
                case IBUS_KEY_Down:
                    ibus_lookup_table_cursor_down (priv->lookup_table);
                    update_lookup_table = TRUE;
                    break;
                case IBUS_KEY_Up:
                    ibus_lookup_table_cursor_up (priv->lookup_table);
                    update_lookup_table = TRUE;
                    break;
                case IBUS_KEY_Page_Down:
                    ibus_lookup_table_page_down (priv->lookup_table);
                    update_lookup_table = TRUE;
                    break;
                case IBUS_KEY_Page_Up:
                    ibus_lookup_table_page_up (priv->lookup_table);
                    update_lookup_table = TRUE;
                    break;
                default:;
                }
            }

            if (!update_lookup_table) {
                if (ibus_engine_simple_if_in_range_of_lookup_table (simple,
                            keyval)) {
                        ibus_engine_simple_set_number_on_lookup_table (
                                simple,
                                keyval,
                                n_compose);
                        return TRUE;
                }
                else if (is_hex_end && !is_space) {
                    if (priv->lookup_table) {
                        int index = (int) ibus_lookup_table_get_cursor_pos (
                                priv->lookup_table);
                        check_emoji_table (simple, n_compose, index);
                        priv->lookup_table_visible = FALSE;
                        update_lookup_table = TRUE;
                    }
                } else if (check_emoji_table (simple, n_compose, -1)) {
                    update_lookup_table = TRUE;
                } else {
                    priv->lookup_table_visible = FALSE;
                    update_lookup_table = TRUE;
                }
            }

            if (update_lookup_table)
                ibus_engine_simple_update_lookup_and_aux_table (simple);
            if (is_hex_end && !is_space) {
                if (priv->tentative_emoji && *priv->tentative_emoji) {
                    ibus_engine_simple_commit_str (simple,
                            priv->tentative_emoji);
                    priv->compose_buffer[0] = 0;
                    ibus_engine_simple_update_preedit_text (simple);
                } else {
                    g_clear_pointer (&priv->tentative_emoji, g_free);
                    priv->in_emoji_sequence = FALSE;
                    priv->compose_buffer[0] = 0;
                }
            }

            ibus_engine_simple_update_preedit_text (simple);

            return TRUE;
        }
    } else {
        if (ibus_engine_simple_check_all_compose_table (simple, n_compose))
            return TRUE;
    }

    /* The current compose_buffer doesn't match anything */
    return no_sequence_matches (simple, n_compose, keyval, keycode, modifiers);
}

static void
ibus_engine_simple_page_down (IBusEngine *engine)
{
    IBusEngineSimple *simple = (IBusEngineSimple *)engine;
    IBusEngineSimplePrivate *priv = simple->priv;
    if (priv->lookup_table == NULL)
        return;
    ibus_lookup_table_page_down (priv->lookup_table);
    ibus_engine_simple_update_lookup_and_aux_table (simple);
}

static void
ibus_engine_simple_page_up (IBusEngine *engine)
{
    IBusEngineSimple *simple = (IBusEngineSimple *)engine;
    IBusEngineSimplePrivate *priv = simple->priv;
    if (priv->lookup_table == NULL)
        return;
    ibus_lookup_table_page_up (priv->lookup_table);
    ibus_engine_simple_update_lookup_and_aux_table (simple);
}

static void
ibus_engine_simple_candidate_clicked (IBusEngine *engine,
                                      guint       index,
                                      guint       button,
                                      guint       state)
{
    IBusEngineSimple *simple = (IBusEngineSimple *)engine;
    IBusEngineSimplePrivate *priv = simple->priv;
    guint keyval;
    int n_compose = 0;

    if (priv->lookup_table == NULL || !priv->lookup_table_visible)
        return;
    if (index == 9)
        keyval = IBUS_KEY_0;
    else
        keyval = IBUS_KEY_1 + index;
    while (priv->compose_buffer[n_compose] != 0)
        n_compose++;
    CHECK_COMPOSE_BUFFER_LENGTH (n_compose);
    ibus_engine_simple_set_number_on_lookup_table (simple, keyval, n_compose);
}

void
ibus_engine_simple_add_table (IBusEngineSimple *simple,
                              const guint16    *data,
                              gint              max_seq_len,
                              gint              n_seqs)
{
    g_return_if_fail (IBUS_IS_ENGINE_SIMPLE (simple));

    global_tables = ibus_compose_table_list_add_array (global_tables,
                                                       data,
                                                       max_seq_len,
                                                       n_seqs);
}

gboolean
ibus_engine_simple_add_table_by_locale (IBusEngineSimple *simple,
                                        const gchar      *locale)
{
    /* Now ibus_engine_simple_add_compose_file() always returns TRUE. */
    gboolean retval = TRUE;
    char *path = NULL;
    const char *home;
#if GLIB_CHECK_VERSION (2, 58, 0)
    const char * const *langs;
    const char * const *lang = NULL;
#else
    const char *_locale;
    char **langs = NULL;
    char **lang = NULL;
#endif
    char * const sys_langs[] = { "el_gr", "fi_fi", "pt_br", NULL };
    char * const *sys_lang = NULL;

    if (locale == NULL) {
        path = g_build_filename (g_get_user_config_dir (),
                                 "ibus", "Compose", NULL);
        if (g_file_test (path, G_FILE_TEST_EXISTS)) {
            ibus_engine_simple_add_compose_file (simple, path);
            g_free (path);
            return retval;
        }
        g_free (path);
        path = NULL;

        path = g_build_filename (g_get_user_config_dir (),
                                 "gtk-3.0", "Compose", NULL);
        if (g_file_test (path, G_FILE_TEST_EXISTS)) {
            ibus_engine_simple_add_compose_file (simple, path);
            g_free (path);
            return retval;
        }
        g_free (path);
        path = NULL;

        home = g_get_home_dir ();
        if (home == NULL)
            return retval;

        path = g_build_filename (home, ".XCompose", NULL);
        if (g_file_test (path, G_FILE_TEST_EXISTS)) {
            ibus_engine_simple_add_compose_file (simple, path);
            g_free (path);
            return retval;
        }
        g_free (path);
        path = NULL;

#if GLIB_CHECK_VERSION (2, 58, 0)
        langs = g_get_language_names_with_category ("LC_CTYPE");
#else
        _locale = g_getenv ("LC_CTYPE");
        if (_locale == NULL)
            _locale = g_getenv ("LANG");
        if (_locale == NULL)
            _locale = "C";

        /* FIXME: https://bugzilla.gnome.org/show_bug.cgi?id=751826 */
        langs = g_get_locale_variants (_locale);
#endif

        for (lang = langs; *lang; lang++) {
            if (g_str_has_prefix (*lang, "en_US"))
                break;
            if (**lang == 'C')
                break;

            /* Other languages just include en_us compose table. */
            for (sys_lang = sys_langs; *sys_lang; sys_lang++) {
                if (g_ascii_strncasecmp (*lang, *sys_lang,
                                         strlen (*sys_lang)) == 0) {
                    path = g_build_filename (X11_DATADIR,
                                             *lang, "Compose", NULL);
                    break;
                }
            }

            if (path == NULL)
                continue;

            if (g_file_test (path, G_FILE_TEST_EXISTS))
                break;
            g_free (path);
            path = NULL;
        }

#if !GLIB_CHECK_VERSION (2, 58, 0)
        g_strfreev (langs);
#endif

        if (path != NULL)
            ibus_engine_simple_add_compose_file (simple, path);
        g_free (path);
        path = NULL;
    } else {
        path = g_build_filename (X11_DATADIR, locale, "Compose", NULL);
        do {
            if (g_file_test (path, G_FILE_TEST_EXISTS))
                break;
            g_free (path);
            path = NULL;
        } while (0);
        if (path == NULL)
            return retval;
        ibus_engine_simple_add_compose_file (simple, path);
    }

    return retval;
}

gboolean
ibus_engine_simple_add_compose_file (IBusEngineSimple *simple,
                                     const gchar      *compose_file)
{
    g_return_val_if_fail (IBUS_IS_ENGINE_SIMPLE (simple), TRUE);

    global_tables = ibus_compose_table_list_add_file (global_tables,
                                                      compose_file);
    return TRUE;
}

