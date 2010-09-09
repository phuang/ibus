/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
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

#include <unistd.h>
#include "connection.h"
#include "matchrule.h"

#define BUS_CONNECTION_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_CONNECTION, BusConnectionPrivate))

/* BusConnectionPriv */
struct _BusConnectionPrivate {
};
typedef struct _BusConnectionPrivate BusConnectionPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_connection_destroy      (BusConnection          *connection);
static gboolean bus_connection_authenticate_unix_user
                                            (IBusConnection         *connection,
                                             gulong                  uid);
static gboolean bus_connection_ibus_message (BusConnection          *connection,
                                             IBusMessage            *message);
#if 0
static gboolean bus_connection_dbus_signal  (BusConnection          *connection,
                                             DBusMessage            *message);
#endif

G_DEFINE_TYPE (BusConnection, bus_connection, IBUS_TYPE_CONNECTION)

BusConnection *
bus_connection_new (void)
{
    BusConnection *connection = BUS_CONNECTION (g_object_new (BUS_TYPE_CONNECTION, NULL));
    return connection;
}

static void
bus_connection_class_init (BusConnectionClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusConnectionClass *ibus_connection_class = IBUS_CONNECTION_CLASS (klass);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_connection_destroy;

    ibus_connection_class->authenticate_unix_user = bus_connection_authenticate_unix_user;
    ibus_connection_class->ibus_message =
            (IBusIBusMessageFunc) bus_connection_ibus_message;

}

static void
bus_connection_init (BusConnection *connection)
{
    connection->unique_name = NULL;
    connection->names = NULL;
}

static void
bus_connection_destroy (BusConnection *connection)
{
    GList *name;

    IBUS_OBJECT_CLASS(bus_connection_parent_class)->destroy (IBUS_OBJECT (connection));

    if (connection->unique_name) {
        g_free (connection->unique_name);
        connection->unique_name = NULL;
    }

    for (name = connection->names; name != NULL; name = name->next) {
        g_free (name->data);
    }
    g_list_free (connection->names);
    connection->names = NULL;
}

static gboolean
bus_connection_authenticate_unix_user (IBusConnection *connection,
                                       gulong          uid)
{
    /* just allow root or same user connect to ibus */
    if (uid == 0 || uid == getuid ())
        return TRUE;
    return FALSE;
}

static gboolean
bus_connection_ibus_message (BusConnection  *connection,
                             IBusMessage    *message)
{
    gboolean retval;

#if 0
    gchar *str = ibus_message_to_string (message);
    g_debug ("%s", str);
    g_free(str);
#endif

    retval = IBUS_CONNECTION_CLASS (bus_connection_parent_class)->ibus_message (
                    (IBusConnection *)connection,
                    message);
    return retval;
}

#if 0
static gboolean
bus_connection_dbus_signal  (BusConnection  *connection,
                             DBusMessage    *message)
{
    gboolean retval;
    retval = IBUS_CONNECTION_CLASS (bus_connection_parent_class)->dbus_signal (
            IBUS_CONNECTION (connection), message);
    return retval;
}
#endif

const gchar *
bus_connection_get_unique_name (BusConnection   *connection)
{
    return connection->unique_name;
}

void
bus_connection_set_unique_name (BusConnection   *connection,
                                const gchar     *name)
{
    g_assert (connection->unique_name == NULL);
    connection->unique_name = g_strdup (name);
}

const GList *
bus_connection_get_names (BusConnection   *connection)
{
    return connection->names;
}

const gchar *
bus_connection_add_name (BusConnection     *connection,
                          const gchar       *name)
{
    gchar *new_name;

    new_name = g_strdup (name);
    connection->names = g_list_append (connection->names, new_name);

    return new_name;
}

gboolean
bus_connection_remove_name (BusConnection     *connection,
                             const gchar       *name)
{
    GList *link;

    link = g_list_find_custom (connection->names, name, (GCompareFunc) g_strcmp0);

    if (link) {
        g_free (link->data);
        connection->names = g_list_delete_link (connection->names, link);
        return TRUE;
    }
    return FALSE;
}

gboolean
bus_connection_add_match (BusConnection  *connection,
                          const gchar    *rule)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (rule != NULL);

    BusMatchRule *p;
    GList *link;

    p = bus_match_rule_new (rule);
    if (p == NULL)
        return FALSE;

    for (link = connection->rules; link != NULL; link = link->next) {
        if (bus_match_rule_is_equal (p, (BusMatchRule *)link->data)) {
            g_object_unref (p);
            return TRUE;
        }
    }

    connection->rules = g_list_append (connection->rules, p);
    return TRUE;

}

gboolean
bus_connection_remove_match (BusConnection  *connection,
                             const gchar    *rule)
{
    return FALSE;
}

