/* vim:set et sts=4: */
/* IBus - The Input Bus
 * Copyright (C) 2008-2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2010 Red Hat, Inc.
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
#include <string.h>
#include <dbus/dbus.h>
#include "matchrule.h"

#define BUS_CONFIG_PROXY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_CONFIG_PROXY, BusMatchRulePrivate))


static void      bus_match_rule_destroy         (BusMatchRule       *rule);
static void      _connection_destroy_cb         (BusConnection      *connection,
                                                 BusMatchRule       *rule);

G_DEFINE_TYPE (BusMatchRule, bus_match_rule, IBUS_TYPE_OBJECT)

static void
bus_match_rule_class_init (BusMatchRuleClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_match_rule_destroy;
}

static void
bus_match_rule_init (BusMatchRule *rule)
{
    rule->flags = 0;
    rule->message_type = DBUS_MESSAGE_TYPE_INVALID;
    rule->interface = NULL;
    rule->member = NULL;
    rule->sender = NULL;
    rule->destination = NULL;
    rule->path = NULL;
    rule->args = g_array_new (TRUE, TRUE, sizeof (gchar *));
}

static void
bus_match_rule_destroy (BusMatchRule *rule)
{
    g_free (rule->interface);
    g_free (rule->member);
    g_free (rule->sender);
    g_free (rule->destination);
    g_free (rule->path);

    gint i;
    GList *link;

    for (i = 0; i < rule->args->len; i++) {
        g_free (g_array_index (rule->args, gchar *, i));
    }
    g_array_free (rule->args, TRUE);

    for (link = rule->recipients; link != NULL; link = link->next) {
        BusRecipient *recipient = (BusRecipient *) link->data;
        g_signal_handlers_disconnect_by_func (recipient->connection,
                                              G_CALLBACK (_connection_destroy_cb), rule);
        g_object_unref (recipient->connection);
        g_slice_free (BusRecipient, recipient);
    }
    g_list_free (rule->recipients);

    IBUS_OBJECT_CLASS(bus_match_rule_parent_class)->destroy (IBUS_OBJECT (rule));
}


typedef struct _Token {
    gchar *key;
    gchar *value;
} Token;

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

static Token *
tokenize_rule (const gchar *text)
{
    GArray *tokens;
    Token token;
    const gchar *p;
    gint i;

    tokens = g_array_new (TRUE, TRUE, sizeof (Token));

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

    return (Token *)g_array_free (tokens, FALSE);

failed:

    for (i = 0; i < tokens->len; i++) {
        Token *p = &g_array_index (tokens, Token, i);
        g_free (p->key);
        g_free (p->value);
    }

    g_array_free (tokens, TRUE);
    return NULL;
}

static void
tokens_free (Token *tokens)
{
    Token *p;
    p = tokens;

    while (p != NULL && p->key != NULL) {
        g_free (p->key);
        g_free (p->value);
        p ++;
    }
    g_free (tokens);
}

static gboolean
_atoi (const gchar *text, gint *i)
{
    const gchar *p = text;
    *i = 0;
    while (*p != '\0') {
        if (!IS_NUMBER(p))
            return FALSE;
        *i = (*i) * 10 - '0'  + *p;
        p ++;
    }
    return TRUE;
}

BusMatchRule *
bus_match_rule_new (const gchar *text)
{
    g_assert (text != NULL);

    Token *tokens, *p;
    BusMatchRule *rule;

    rule = BUS_MATCH_RULE (g_object_new (BUS_TYPE_MATCH_RULE, NULL));

    /* parse rule */
    tokens = tokenize_rule (text);

    for (p = tokens; p != NULL && p->key != 0; p++) {
        if (g_strcmp0 (p->key, "type") == 0) {
            if (g_strcmp0 (p->value, "signal") == 0) {
                bus_match_rule_set_message_type (rule, DBUS_MESSAGE_TYPE_SIGNAL);
            }
            else if (g_strcmp0 (p->value, "method_call") == 0) {
                bus_match_rule_set_message_type (rule, DBUS_MESSAGE_TYPE_METHOD_CALL);
            }
            else if (g_strcmp0 (p->value, "method_return") == 0) {
                bus_match_rule_set_message_type (rule, DBUS_MESSAGE_TYPE_METHOD_RETURN);
            }
            else if (g_strcmp0 (p->value, "error") == 0) {
                bus_match_rule_set_message_type (rule, DBUS_MESSAGE_TYPE_ERROR);
            }
            else
                goto failed;
        }
        else if (g_strcmp0 (p->key, "sender") == 0) {
            bus_match_rule_set_sender (rule, p->value);
        }
        else if (g_strcmp0 (p->key, "interface") == 0) {
            bus_match_rule_set_interface (rule, p->value);
        }
        else if (g_strcmp0 (p->key, "member") == 0) {
            bus_match_rule_set_member (rule, p->value);
        }
        else if (g_strcmp0 (p->key, "path") == 0) {
            bus_match_rule_set_path (rule, p->value);
        }
        else if (g_strcmp0 (p->key, "destination") == 0) {
            bus_match_rule_set_destination (rule, p->value);
        }
        else if (strncmp (p->key, "arg", 3) == 0) {
            gint i;
            if (! _atoi (p->key + 3, &i))
                goto failed;
            bus_match_rule_set_arg (rule, i, p->value);
        }
        else
            goto failed;
    }

    tokens_free (tokens);
    return rule;

failed:
    tokens_free (tokens);
    g_object_unref (rule);
    return NULL;
}

gboolean
bus_match_rule_set_message_type (BusMatchRule   *rule,
                                 gint            type)
{
    g_assert (rule != NULL);
    g_assert (type == DBUS_MESSAGE_TYPE_SIGNAL ||
              type == DBUS_MESSAGE_TYPE_METHOD_CALL ||
              type == DBUS_MESSAGE_TYPE_METHOD_RETURN ||
              type == DBUS_MESSAGE_TYPE_ERROR);

    rule->flags |= MATCH_TYPE;
    rule->message_type = type;

    return TRUE;
}

gboolean
bus_match_rule_set_sender  (BusMatchRule    *rule,
                            const gchar     *sender)
{
    g_assert (rule != NULL);
    g_assert (sender != NULL);

    rule->flags |= MATCH_SENDER;

    g_free (rule->sender);
    rule->sender = g_strdup (sender);

    return TRUE;
}

gboolean
bus_match_rule_set_interface (BusMatchRule   *rule,
                              const gchar    *interface)
{
    g_assert (rule != NULL);
    g_assert (interface != NULL);

    rule->flags |= MATCH_INTERFACE;

    g_free (rule->interface);
    rule->interface = g_strdup (interface);
    return TRUE;
}

gboolean
bus_match_rule_set_member (BusMatchRule   *rule,
                           const gchar    *member)
{
    g_assert (rule != NULL);
    g_assert (member != NULL);

    rule->flags |= MATCH_MEMBER;

    g_free (rule->member);
    rule->member = g_strdup (member);

    return TRUE;
}

gboolean
bus_match_rule_set_path (BusMatchRule   *rule,
                         const gchar    *path)
{
    g_assert (rule != NULL);
    g_assert (path != NULL);

    rule->flags |= MATCH_PATH;

    g_free (rule->path);
    rule->path = g_strdup (path);

    return TRUE;
}

gboolean
bus_match_rule_set_destination (BusMatchRule   *rule,
                                const gchar    *dest)
{
    g_assert (rule != NULL);
    g_assert (dest != NULL);

    rule->flags |= MATCH_DESTINATION;

    g_free (rule->destination);
    rule->destination = g_strdup (dest);

    return TRUE;
}

gboolean
bus_match_rule_set_arg (BusMatchRule   *rule,
                        guint           arg_i,
                        const gchar    *arg)
{
    g_assert (rule != NULL);
    g_assert (arg != NULL);

    rule->flags |= MATCH_ARGS;

    if (arg_i >= rule->args->len) {
        g_array_set_size (rule->args, arg_i + 1);
    }

    g_free (g_array_index (rule->args, gchar *, arg_i));
    g_array_index (rule->args, gchar *, arg_i) = g_strdup (arg);
    return TRUE;
}

gboolean
bus_match_rule_match (BusMatchRule   *rule,
                      DBusMessage    *message)
{
    g_assert (rule != NULL);
    g_assert (message != NULL);

    if (rule->flags & MATCH_TYPE) {
        if (ibus_message_get_type (message) != rule->message_type)
            return FALSE;
    }

    if (rule->flags & MATCH_INTERFACE) {
        if (g_strcmp0 (ibus_message_get_interface (message), rule->interface) != 0)
            return FALSE;
    }

    if (rule->flags & MATCH_MEMBER) {
        if (g_strcmp0 (ibus_message_get_member (message), rule->member) != 0)
            return FALSE;
    }

    if (rule->flags & MATCH_SENDER) {
        if (g_strcmp0 (ibus_message_get_sender (message), rule->sender) != 0)
            return FALSE;
    }

    if (rule->flags & MATCH_DESTINATION) {
        if (g_strcmp0 (ibus_message_get_destination (message), rule->destination) != 0)
            return FALSE;
    }

    if (rule->flags & MATCH_PATH) {
        if (g_strcmp0 (ibus_message_get_path (message), rule->path) != 0)
            return FALSE;
    }

    if (rule->flags & MATCH_ARGS) {
        guint i;
        DBusMessageIter iter;

        ibus_message_iter_init (message, &iter);

        for (i = 0; i < rule->args->len; i++) {
            gchar *arg = g_array_index (rule->args, gchar *, i);
            if (arg != NULL) {
                gint type;
                gchar *value;

                type = ibus_message_iter_get_arg_type (&iter);
                if (type != G_TYPE_STRING && type != IBUS_TYPE_OBJECT_PATH)
                    return FALSE;

                ibus_message_iter_get_basic (&iter, &value);

                if (g_strcmp0 (arg, value) != 0)
                    return FALSE;
            }
            ibus_message_iter_next (&iter);
        }
    }

    return TRUE;
}

gboolean
bus_match_rule_is_equal (BusMatchRule   *a,
                         BusMatchRule   *b)
{
    g_assert (a != NULL);
    g_assert (b != NULL);

    if (a->flags != b->flags)
        return FALSE;

    if (a->flags & MATCH_TYPE) {
        if (a->message_type != b->message_type)
            return FALSE;
    }

    if (a->flags & MATCH_INTERFACE) {
        if (g_strcmp0 (a->interface, b->interface) != 0)
            return FALSE;
    }

    if (a->flags & MATCH_MEMBER) {
        if (g_strcmp0 (a->member, b->member) != 0)
            return FALSE;
    }

    if (a->flags & MATCH_SENDER) {
        if (g_strcmp0 (a->sender, b->sender) != 0)
            return FALSE;
    }

    if (a->flags & MATCH_DESTINATION) {
        if (g_strcmp0 (a->destination, b->destination) != 0)
            return FALSE;
    }

    if (a->flags & MATCH_PATH) {
        if (g_strcmp0 (a->path, b->path) != 0)
            return FALSE;
    }

    if (a->flags & MATCH_ARGS) {
        if (a->args->len != b->args->len)
            return FALSE;

        gint i;

        for (i = 0; i < a->args->len; i++) {
            if (g_strcmp0 (g_array_index (a->args, gchar *, i),
                           g_array_index (b->args, gchar *, i)) != 0)
                return FALSE;
        }
    }

    return TRUE;
}

static void
_connection_destroy_cb (BusConnection   *connection,
                        BusMatchRule    *rule)
{
    g_assert (BUS_IS_MATCH_RULE (rule));
    g_assert (BUS_IS_CONNECTION (connection));

    GList *link;
    BusRecipient *recipient;

    for (link = rule->recipients; link != NULL; link = link->next) {
        recipient = (BusRecipient *)link->data;

        if (recipient->connection == connection) {
            rule->recipients = g_list_remove_link (rule->recipients, link);
            g_object_unref (connection);
            g_slice_free (BusRecipient, recipient);
            return;
        }

        if (rule->recipients == NULL) {
            ibus_object_destroy (IBUS_OBJECT (rule));
        }
    }
    g_assert_not_reached ();
}

void
bus_match_rule_add_recipient (BusMatchRule   *rule,
                              BusConnection  *connection)
{
    g_assert (BUS_IS_MATCH_RULE (rule));
    g_assert (BUS_IS_CONNECTION (connection));

    GList *link;
    BusRecipient *recipient;

    for (link = rule->recipients; link != NULL; link = link->next) {
        recipient = (BusRecipient *) link->data;
        if (connection == recipient->connection) {
            recipient->refcount ++;
            return;
        }
    }

    recipient = g_slice_new (BusRecipient);

    g_object_ref_sink (connection);
    recipient->connection = connection;
    recipient->refcount  = 1;

    rule->recipients = g_list_append (rule->recipients, recipient);
    g_signal_connect (connection,
                      "destroy",
                      G_CALLBACK (_connection_destroy_cb),
                      rule);
}

void
bus_match_rule_remove_recipient (BusMatchRule   *rule,
                                 BusConnection  *connection)
{
    g_assert (BUS_IS_MATCH_RULE (rule));
    g_assert (BUS_IS_CONNECTION (connection));

    GList *link;
    BusRecipient *recipient;

    for (link = rule->recipients; link != NULL; link = link->next) {
        recipient = (BusRecipient *) link->data;
        if (connection == recipient->connection) {
            recipient->refcount --;
            if (recipient->refcount == 0) {
                rule->recipients = g_list_remove_link (rule->recipients, link);
                g_slice_free (BusRecipient, recipient);
                g_signal_handlers_disconnect_by_func (connection,
                                                      G_CALLBACK (_connection_destroy_cb),
                                                      rule);
                g_object_unref (connection);
            }

            if (rule->recipients == NULL ) {
                ibus_object_destroy (IBUS_OBJECT(rule));
            }
            return;
        }
    }

    g_warning ("Remove recipient failed");
}

GList *
bus_match_rule_get_recipients (BusMatchRule   *rule,
                               DBusMessage    *message)
{
    g_assert (BUS_IS_MATCH_RULE (rule));
    g_assert (message != NULL);

    GList *link;
    GList *recipients = NULL;

    if (!bus_match_rule_match (rule, message))
        return FALSE;

    for (link = rule->recipients; link != NULL; link = link->next) {
        BusRecipient *recipient = (BusRecipient *) link->data;

        recipients = g_list_append (recipients, recipient->connection);
    }

    return recipients;
}

