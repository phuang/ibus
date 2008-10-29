/* vim:set et sts=4: */
/* IBus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <glib.h>
#include <dbus/dbus.h>
#include "matchrule.h"

typedef struct _BusArg BusArg;
struct _BusArg {
    guint len;
    gchar *value;
};

typedef struct _Token Token;
struct _Token {
    gchar *key;
    gchar *value;
};


#define SKIP_WHITE(a)   \
    while (*(a) == ' ' || *(a) == '\t') { (a)++; }
#define IS_ALPHA(a) \
    ((*(a) >= 'a' && *(a) <= 'z') || (*(a) >= 'A' && *(a) <= 'Z'))
#define IS_NUMBER(a) \
    (*(a) >= '0' && *(a) <= '9')

static gchar *
find_key (const gchar **p)
{
    GString *text;

    text = g_string_new ("");
    
    SKIP_WHITE(*p)
    if (!IS_ALPHA (*p))
        goto failed;
    
    g_string_append_c (text, **p);
    (*p) ++;

    while (IS_ALPHA (*p) || IS_NUMBER (*p)) {
        g_string_append_c (text, **p);
        (*p) ++;
    }

    return g_string_free (text, FALSE);

failed:
    g_string_free (text, TRUE);
    return NULL;

}

static gchar *
find_value (const gchar **p)
{
    GString *text;

    text = g_string_new ("");
    
    SKIP_WHITE (*p);

    if (**p != '\'')
        goto failed;
    (*p) ++;

    while (**p != '\'') {
        if (**p == '\0')
            goto failed;
        if (**p == '\\')
            (*p) ++;
        g_string_append_c (text, **p);
        (*p) ++;
    }
    (*p) ++;

    return g_string_free (text, FALSE);

failed:
    g_string_free (text, TRUE);
    return NULL;
}

static GArray *
tokenize_rule (const gchar *text)
{
    GArray *tokens;
    Token token;
    const gchar *p;
    gint i;

    tokens = g_array_new (FALSE, TRUE, sizeof (Token));

    p = text;

    while (*p != '\0') {
        gchar *key;
        gchar *value;

        SKIP_WHITE (p);
        key = find_key (&p);
        if (key == NULL)
            goto failed;
        SKIP_WHITE (p);
        if (*p != '=')
            goto failed;
        p ++;
        SKIP_WHITE (p);
        value = find_value (&p);
        if (value == NULL) {
            g_free (key);
            goto failed;
        }
        SKIP_WHITE (p);
        if (*p != ',' && *p != '\0') {
            g_free (key);
            g_free (value);
            goto failed;
        }

        if (*p == ',')
         p ++;
        token.key = key;
        token.value = value;
        g_array_append_val (tokens, token);
    }

    return tokens;

failed:
    
    for (i = 0; i < tokens->len; i++) {
        Token *p = &g_array_index (tokens, Token, i);
        g_free (p->key);
        g_free (p->value);
    }

    g_array_free (tokens, TRUE);    
    return NULL;
}

BusMatchRule *
bus_match_rule_new (const gchar *text)
{
    g_assert (text != NULL);

    GArray *tokens;
    gint i;
    BusMatchRule *rule;

    rule = g_slice_new0 (BusMatchRule);

    rule->refcount = 1;
    rule->message_type = DBUS_MESSAGE_TYPE_INVALID;
    rule->args = g_array_new (FALSE, TRUE, sizeof (BusArg));

    /* parse rule */
    tokens = tokenize_rule (text);

    for (i = 0; i < tokens->len; i++) {
        Token *token = &g_array_index (tokens, Token, i);
        g_debug ("key=%s, value=%s", token->key, token->value);
    }



    return rule;

failed:
    bus_match_rule_unref (rule);
    return NULL;
}

void
bus_match_rule_unref (BusMatchRule *rule)
{
    g_assert (rule != NULL);

    gint i;

    rule->refcount --;

    if (rule->refcount > 0)
        return;

    g_free (rule->interface);
    g_free (rule->member);
    g_free (rule->sender);
    g_free (rule->destination);
    g_free (rule->path);

    for (i = 0; i < rule->args->len; i++ ) {
        BusArg *arg = &g_array_index (rule->args, BusArg, i);
        g_free (arg->value);
    }

    g_array_free (rule->args, TRUE);
}


