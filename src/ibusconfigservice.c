/* vim:set et sts=4: */
/* ibus - The Input Bus
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

#include "ibusshare.h"
#include "ibusconfigservice.h"

enum {
    LAST_SIGNAL,
};

enum {
    PROP_0,
    PROP_CONNECTION,
};

// static guint            config_service_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_config_service_class_init      (IBusConfigServiceClass *klass);
static void     ibus_config_service_init            (IBusConfigService      *config);
static void     ibus_config_service_set_property    (IBusConfigService      *config,
                                                     guint                   prop_id,
                                                     const GValue           *value,
                                                     GParamSpec             *pspec);
static void     ibus_config_service_get_property    (IBusConfigService      *config,
                                                     guint                   prop_id,
                                                     GValue                 *value,
                                                     GParamSpec             *pspec);
static void     ibus_config_service_destroy         (IBusConfigService      *config);
static gboolean ibus_config_service_ibus_message    (IBusConfigService      *config,
                                                     IBusConnection         *connection,
                                                     IBusMessage            *message);
static gboolean ibus_config_service_set_value       (IBusConfigService      *config,
                                                     const gchar            *section,
                                                     const gchar            *name,
                                                     const GValue           *value,
                                                     IBusError             **error);
static gboolean ibus_config_service_get_value       (IBusConfigService      *config,
                                                     const gchar            *section,
                                                     const gchar            *name,
                                                     GValue                 *value,
                                                     IBusError             **error);

static IBusServiceClass  *parent_class = NULL;

GType
ibus_config_service_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusConfigServiceClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_config_service_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusConfigService),
        0,
        (GInstanceInitFunc) ibus_config_service_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERVICE,
                                       "IBusConfigService",
                                       &type_info,
                                       (GTypeFlags) 0);
    }
    return type;
}

IBusConfigService *
ibus_config_service_new (IBusConnection *connection)
{
    g_assert (IBUS_IS_CONNECTION (connection));

    IBusConfigService *config;

    config = (IBusConfigService *) g_object_new (IBUS_TYPE_CONFIG_SERVICE,
                                                 "path", IBUS_PATH_CONFIG,
                                                 "connection", connection,
                                                 NULL);

    return config;
}

static void
ibus_config_service_class_init (IBusConfigServiceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    parent_class = (IBusServiceClass *) g_type_class_peek_parent (klass);

    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_config_service_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_config_service_get_property;

    IBUS_OBJECT_CLASS (gobject_class)->destroy = (IBusObjectDestroyFunc) ibus_config_service_destroy;

    IBUS_SERVICE_CLASS (klass)->ibus_message = (ServiceIBusMessageFunc) ibus_config_service_ibus_message;

    klass->set_value = ibus_config_service_set_value;
    klass->get_value = ibus_config_service_get_value;

    /* install properties */
    /**
     * IBusConfigService:connection:
     *
     * Connection of this IBusConfigServifce.
     */
    g_object_class_install_property (gobject_class,
                    PROP_CONNECTION,
                    g_param_spec_object ("connection",
                        "connection",
                        "The connection of config object",
                        IBUS_TYPE_CONNECTION,
                        G_PARAM_READWRITE |  G_PARAM_CONSTRUCT_ONLY));
}

static void
ibus_config_service_init (IBusConfigService *config)
{
}

static void
ibus_config_service_set_property (IBusConfigService *config,
                                  guint              prop_id,
                                  const GValue      *value,
                                  GParamSpec        *pspec)
{
    switch (prop_id) {
    case PROP_CONNECTION:
        ibus_service_add_to_connection ((IBusService *) config,
                                        g_value_get_object (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (config, prop_id, pspec);
    }
}

static void
ibus_config_service_get_property (IBusConfigService *config,
                                  guint              prop_id,
                                  GValue            *value,
                                  GParamSpec        *pspec)
{
    switch (prop_id) {
    case PROP_CONNECTION:
        {
            GList *connections = ibus_service_get_connections ((IBusService *) config);
            if (connections) {
                g_value_set_object (value, connections->data);
            }
            else {
                g_value_set_object (value, NULL);
            }
            g_list_foreach (connections, (GFunc) g_object_unref, NULL);
            g_list_free (connections);
        }
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (config, prop_id, pspec);
    }
}

static void
ibus_config_service_destroy (IBusConfigService *config)
{
    IBUS_OBJECT_CLASS(parent_class)->destroy ((IBusObject *) config);
}

static gboolean
ibus_config_service_ibus_message (IBusConfigService     *config,
                                  IBusConnection        *connection,
                                  IBusMessage           *message)
{
    g_assert (IBUS_IS_CONFIG_SERVICE (config));
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    IBusMessage *reply = NULL;

    if (ibus_message_is_method_call (message, IBUS_INTERFACE_CONFIG, "SetValue")) {
        gchar *section;
        gchar *name;
        GValue value = { 0 };
        IBusError *error = NULL;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &section,
                                        G_TYPE_STRING, &name,
                                        G_TYPE_VALUE, &value,
                                        G_TYPE_INVALID);
        if (!retval) {
            reply = ibus_message_new_error_printf (message,
                                                   DBUS_ERROR_INVALID_ARGS,
                                                   "Can not parse arguments 1 of SetValue");
            ibus_error_free (error);
        }
        else if (!IBUS_CONFIG_SERVICE_GET_CLASS (config)->set_value (config, section, name, &value, &error)) {
            reply = ibus_message_new_error (message,
                                            error->name,
                                            error->message);
            ibus_error_free (error);
        }
        else {
            reply = ibus_message_new_method_return (message);
        }
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_CONFIG, "GetValue")) {
        gchar *section;
        gchar *name;
        GValue value = { 0 };
        IBusError *error = NULL;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &section,
                                        G_TYPE_STRING, &name,
                                        G_TYPE_INVALID);

        if (!retval) {
            reply = ibus_message_new_error (message,
                                            error->name,
                                            error->message);
            ibus_error_free (error);
        }
        else if (!IBUS_CONFIG_SERVICE_GET_CLASS (config)->get_value (config, section, name, &value, &error)) {
            reply = ibus_message_new_error (message,
                                            error->name,
                                            error->message);
            ibus_error_free (error);
        }
        else {
            reply = ibus_message_new_method_return (message);
            ibus_message_append_args (reply,
                                      G_TYPE_VALUE, &value,
                                      G_TYPE_INVALID);
            g_value_unset (&value);
        }
    }

    if (reply) {
        ibus_connection_send (connection, reply);
        ibus_message_unref (reply);
        return TRUE;
    }

    return parent_class->ibus_message ((IBusService *) config, connection, message);
}

static gboolean
ibus_config_service_set_value (IBusConfigService *config,
                               const gchar       *section,
                               const gchar       *name,
                               const GValue      *value,
                               IBusError        **error)
{
    if (error) {
        *error = ibus_error_new_from_printf (DBUS_ERROR_FAILED,
                                             "Can not set value [%s, %s]",
                                             section, name);
    }
    return FALSE;
}

static gboolean
ibus_config_service_get_value (IBusConfigService *config,
                               const gchar       *section,
                               const gchar       *name,
                               GValue            *value,
                               IBusError        **error)
{
    if (error) {
        *error = ibus_error_new_from_printf (DBUS_ERROR_FAILED,
                                             "Can not get value [%s, %s]",
                                             section, name);
    }
    return FALSE;
}

void
ibus_config_service_value_changed (IBusConfigService  *config,
                                   const gchar        *section,
                                   const gchar        *name,
                                   const GValue       *value)
{
    g_assert (IBUS_IS_CONFIG_SERVICE (config));
    g_assert (section);
    g_assert (name);
    g_assert (G_IS_VALUE (value));

    ibus_service_send_signal ((IBusService *) config,
                              IBUS_INTERFACE_CONFIG,
                              "ValueChanged",
                              G_TYPE_STRING, &section,
                              G_TYPE_STRING, &name,
                              G_TYPE_VALUE, value,
                              G_TYPE_INVALID);
}
