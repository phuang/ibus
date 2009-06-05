/* vim:set et sts=4: */
/* bus - The Input Bus
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

#include <unistd.h>
#include "connection.h"
#include "matchrule.h"

#define BUS_CONNECTION_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_CONNECTION, BusConnectionPrivate))

/* BusConnectionPriv */
struct _BusConnectionPrivate {
    gchar *unique_name;
    /* list for well known names */
    GList  *names;
    GList  *rules;
};
typedef struct _BusConnectionPrivate BusConnectionPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_connection_class_init   (BusConnectionClass     *klass);
static void     bus_connection_init         (BusConnection          *connection);
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

static IBusObjectClass  *parent_class = NULL;

GType
bus_connection_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusConnectionClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_connection_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusConnection),
        0,
        (GInstanceInitFunc) bus_connection_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_CONNECTION,
                    "BusConnection",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}

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

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusConnectionPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_connection_destroy;

    ibus_connection_class->authenticate_unix_user = bus_connection_authenticate_unix_user;
    ibus_connection_class->ibus_message =
            (IBusIBusMessageFunc) bus_connection_ibus_message;

}

static void
bus_connection_init (BusConnection *connection)
{
    BusConnectionPrivate *priv;

    priv = BUS_CONNECTION_GET_PRIVATE (connection);

    priv->unique_name = NULL;
    priv->names = NULL;
}

static void
bus_connection_destroy (BusConnection *connection)
{
    GList *name;
    BusConnectionPrivate *priv;

    IBUS_OBJECT_CLASS(parent_class)->destroy (IBUS_OBJECT (connection));

    priv = BUS_CONNECTION_GET_PRIVATE (connection);

    if (priv->unique_name) {
        g_free (priv->unique_name);
        priv->unique_name = NULL;
    }

    for (name = priv->names; name != NULL; name = name->next) {
        g_free (name->data);
    }
    g_list_free (priv->names);
    priv->names = NULL;
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

    retval = IBUS_CONNECTION_CLASS (parent_class)->ibus_message (
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
    retval = IBUS_CONNECTION_CLASS (parent_class)->dbus_signal (
            IBUS_CONNECTION (connection), message);
    return retval;
}
#endif

const gchar *
bus_connection_get_unique_name (BusConnection   *connection)
{
    BusConnectionPrivate *priv;

    priv = BUS_CONNECTION_GET_PRIVATE (connection);
    return priv->unique_name;
}

void
bus_connection_set_unique_name (BusConnection   *connection,
                                const gchar     *name)
{
    BusConnectionPrivate *priv;
    priv = BUS_CONNECTION_GET_PRIVATE (connection);
    g_assert (priv->unique_name == NULL);
    priv->unique_name = g_strdup (name);
}

const GList *
bus_connection_get_names (BusConnection   *connection)
{
    BusConnectionPrivate *priv;

    priv = BUS_CONNECTION_GET_PRIVATE (connection);
    return priv->names;
}

const gchar *
bus_connection_add_name (BusConnection     *connection,
                          const gchar       *name)
{
    gchar *new_name;
    BusConnectionPrivate *priv;

    priv = BUS_CONNECTION_GET_PRIVATE (connection);
    new_name = g_strdup (name);
    priv->names = g_list_append (priv->names, new_name);

    return new_name;
}

gboolean
bus_connection_remove_name (BusConnection     *connection,
                             const gchar       *name)
{
    BusConnectionPrivate *priv;
    GList *link;

    priv = BUS_CONNECTION_GET_PRIVATE (connection);

    link = g_list_find_custom (priv->names, name, (GCompareFunc) g_strcmp0);

    if (link) {
        g_free (link->data);
        priv->names = g_list_delete_link (priv->names, link);
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
    BusConnectionPrivate *priv;

    priv = BUS_CONNECTION_GET_PRIVATE (connection);

    p = bus_match_rule_new (rule);
    if (p == NULL)
        return FALSE;

    for (link = priv->rules; link != NULL; link = link->next) {
        if (bus_match_rule_is_equal (p, (BusMatchRule *)link->data)) {
            g_object_unref (p);
            return TRUE;
        }
    }

    priv->rules = g_list_append (priv->rules, p);
    return TRUE;

}

gboolean
bus_connection_remove_match (BusConnection  *connection,
                             const gchar    *rule)
{
    return FALSE;
}

