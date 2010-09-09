/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
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
#include <dbus/dbus.h>
#include "ibusfactory.h"
#include "ibusengine.h"
#include "ibusshare.h"
#include "ibusinternal.h"

#define IBUS_FACTORY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_FACTORY, IBusFactoryPrivate))

enum {
    LAST_SIGNAL,
};

enum {
    PROP_0,
    PROP_CONNECTION,
};

/* IBusFactoryPriv */
struct _IBusFactoryPrivate {
    guint id;
    IBusConnection *connection;
    GList          *engine_list;
    GHashTable     *engine_table;
};
typedef struct _IBusFactoryPrivate IBusFactoryPrivate;

/* functions prototype */
static void     ibus_factory_destroy        (IBusFactory        *factory);
static void     ibus_factory_set_property   (IBusFactory        *engine,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void     ibus_factory_get_property   (IBusFactory        *factory,
                                             guint               prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);

static gboolean ibus_factory_ibus_message   (IBusFactory        *factory,
                                             IBusConnection     *connection,
                                             IBusMessage        *message);

static void     _engine_destroy_cb          (IBusEngine         *engine,
                                             IBusFactory        *factory);

G_DEFINE_TYPE (IBusFactory, ibus_factory, IBUS_TYPE_SERVICE)

IBusFactory *
ibus_factory_new (IBusConnection *connection)
{
    g_assert (IBUS_IS_CONNECTION (connection));

    IBusFactory *factory;
    IBusFactoryPrivate *priv;

    factory = (IBusFactory *) g_object_new (IBUS_TYPE_FACTORY,
                                            "path", IBUS_PATH_FACTORY,
                                            "connection", connection,
                                            NULL);
    priv = IBUS_FACTORY_GET_PRIVATE (factory);

    return factory;
}

static void
ibus_factory_class_init (IBusFactoryClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (IBusFactoryPrivate));

    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_factory_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_factory_get_property;


    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_factory_destroy;

    IBUS_SERVICE_CLASS (klass)->ibus_message = (ServiceIBusMessageFunc) ibus_factory_ibus_message;

    /**
     * IBusFactory:connection:
     *
     * Connection of this IBusFactory.
     **/
    g_object_class_install_property (gobject_class,
                PROP_CONNECTION,
                g_param_spec_object ("connection",
                "connection",
                "The connection of factory object",
                IBUS_TYPE_CONNECTION,
                G_PARAM_READWRITE |  G_PARAM_CONSTRUCT_ONLY));


}

static void
ibus_factory_init (IBusFactory *factory)
{
    IBusFactoryPrivate *priv;
    priv = IBUS_FACTORY_GET_PRIVATE (factory);

    priv->id = 0;
    priv->connection = NULL;
    priv->engine_table = g_hash_table_new_full (g_str_hash,
                                                g_str_equal,
                                                g_free,
                                                NULL);
    priv->engine_list =  NULL;
}

static void
ibus_factory_destroy (IBusFactory *factory)
{
    GList *list;
    IBusFactoryPrivate *priv;
    priv = IBUS_FACTORY_GET_PRIVATE (factory);

    list = g_list_copy (priv->engine_list);
    g_list_foreach (list, (GFunc) ibus_object_destroy, NULL);
    g_list_free (priv->engine_list);
    g_list_free (list);
    priv->engine_list = NULL;

    if (priv->engine_table) {
        g_hash_table_destroy (priv->engine_table);
    }

    if (priv->connection) {
        g_object_unref (priv->connection);
        priv->connection = NULL;
    }

    IBUS_OBJECT_CLASS(ibus_factory_parent_class)->destroy (IBUS_OBJECT (factory));
}

static void
ibus_factory_set_property (IBusFactory  *factory,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    IBusFactoryPrivate *priv;
    priv = IBUS_FACTORY_GET_PRIVATE (factory);

    switch (prop_id) {
    case PROP_CONNECTION:
        priv->connection = g_value_get_object (value);
        g_object_ref_sink (priv->connection);
        ibus_service_add_to_connection ((IBusService *) factory,
                                        priv->connection);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (factory, prop_id, pspec);
    }
}

static void
ibus_factory_get_property (IBusFactory *factory,
                           guint        prop_id,
                           GValue      *value,
                           GParamSpec  *pspec)
{
    IBusFactoryPrivate *priv;
    priv = IBUS_FACTORY_GET_PRIVATE (factory);

    switch (prop_id) {
    case PROP_CONNECTION:
        g_value_set_object (value, priv->connection);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (factory, prop_id, pspec);
    }
}

static void
_engine_destroy_cb (IBusEngine  *engine,
                    IBusFactory *factory)
{
    IBusFactoryPrivate *priv;
    priv = IBUS_FACTORY_GET_PRIVATE (factory);

    priv->engine_list = g_list_remove (priv->engine_list, engine);
    g_object_unref (engine);
}

static gboolean
ibus_factory_ibus_message (IBusFactory    *factory,
                           IBusConnection *connection,
                           IBusMessage    *message)
{
    g_assert (IBUS_IS_FACTORY (factory));
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    IBusMessage *reply_message;
    IBusFactoryPrivate *priv;
    priv = IBUS_FACTORY_GET_PRIVATE (factory);

    g_assert (priv->connection == connection);

    if (ibus_message_is_method_call (message,
                                     IBUS_INTERFACE_FACTORY,
                                     "CreateEngine")) {
        gchar *engine_name;
        gchar *path;
        IBusError *error;
        IBusEngine *engine;
        gboolean retval;
        GType engine_type;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &engine_name,
                                        G_TYPE_INVALID);

        if (!retval) {
            reply_message = ibus_message_new_error_printf (message,
                                        DBUS_ERROR_INVALID_ARGS,
                                        "The 1st arg should be engine name");
            ibus_connection_send (connection, reply_message);
            ibus_message_unref (reply_message);
            return TRUE;
        }

        engine_type = (GType )g_hash_table_lookup (priv->engine_table, engine_name);

        if (engine_type == G_TYPE_INVALID) {
             reply_message = ibus_message_new_error_printf (message,
                                        DBUS_ERROR_FAILED,
                                        "Can not create engine %s", engine_name);
            ibus_connection_send (connection, reply_message);
            ibus_message_unref (reply_message);
            return TRUE;

        }

        path = g_strdup_printf ("/org/freedesktop/IBus/Engine/%d", ++priv->id);

        engine = g_object_new (engine_type,
                               "name", engine_name,
                               "path", path,
                               "connection", priv->connection,
                               NULL);

        priv->engine_list = g_list_append (priv->engine_list, engine);
        g_signal_connect (engine,
                          "destroy",
                          G_CALLBACK (_engine_destroy_cb),
                          factory);

        reply_message = ibus_message_new_method_return (message);
        ibus_message_append_args (reply_message,
                                  IBUS_TYPE_OBJECT_PATH, &path,
                                  G_TYPE_INVALID);
        g_free (path);
        ibus_connection_send (connection, reply_message);
        ibus_message_unref (reply_message);
        return TRUE;
    }

    return IBUS_SERVICE_CLASS (ibus_factory_parent_class)->ibus_message (
                                (IBusService *)factory,
                                connection,
                                message);
}

void
ibus_factory_add_engine (IBusFactory *factory,
                         const gchar *engine_name,
                         GType        engine_type)
{
    g_assert (IBUS_IS_FACTORY (factory));
    g_assert (engine_name);
    g_assert (g_type_is_a (engine_type, IBUS_TYPE_ENGINE));

    IBusFactoryPrivate *priv;
    priv = IBUS_FACTORY_GET_PRIVATE (factory);

    g_hash_table_insert (priv->engine_table, g_strdup (engine_name), (gpointer) engine_type);
}
