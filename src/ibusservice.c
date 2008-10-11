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

#include "ibusservice.h"
#include "ibusinternel.h"

#define IBUS_SERVICE_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_SERVICE, IBusServicePrivate))

#define DECLARE_PRIV IBusServicePrivate *priv = IBUS_SERVICE_GET_PRIVATE(service)

enum {
    DBUS_MESSAGE,
    DBUS_SIGNAL,
    LAST_SIGNAL,
};


/* IBusServicePriv */
struct _IBusServicePrivate {
    gchar *object_path;
    GList *connections;
};
typedef struct _IBusServicePrivate IBusServicePrivate;

static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_service_class_init     (IBusServiceClass   *klass);
static void     ibus_service_init           (IBusService        *service);
static void     ibus_service_finalize       (IBusService        *service);
static gboolean ibus_service_dbus_message   (IBusService        *service,
                                             IBusConnection     *connection,
                                             DBusMessage        *message);
static gboolean ibus_service_dbus_signal    (IBusService        *service,
                                             IBusConnection     *connection,
                                             DBusMessage        *message);

static IBusObjectClass  *_parent_class = NULL;

GType
ibus_service_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusServiceClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_service_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusService),
        0,
        (GInstanceInitFunc) ibus_service_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "IBusService",
                    &type_info,
                    (GTypeFlags) 0);
    }
    return type;
}

IBusService *
ibus_service_new (const gchar *object_path)
{
    IBusService *service;
    service = IBUS_SERVICE (g_object_new (IBUS_TYPE_SERVICE, NULL));

    DECLARE_PRIV;
    priv->object_path = g_strdup (object_path);

    return service;
}

static void
ibus_service_class_init (IBusServiceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    _parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusServicePrivate));

    gobject_class->finalize = (GObjectFinalizeFunc) ibus_service_finalize;

    klass->dbus_message = ibus_service_dbus_message;
    klass->dbus_signal = ibus_service_dbus_signal;

    _signals[DBUS_MESSAGE] =
        g_signal_new (I_("dbus-message"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusServiceClass, dbus_message),
            NULL, NULL,
            ibus_marshal_BOOLEAN__POINTER_POINTER,
            G_TYPE_BOOLEAN, 2,
            G_TYPE_POINTER, G_TYPE_POINTER);

    _signals[DBUS_SIGNAL] =
        g_signal_new (I_("dbus-signal"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusServiceClass, dbus_signal),
            NULL, NULL,
            ibus_marshal_BOOLEAN__POINTER_POINTER,
            G_TYPE_BOOLEAN, 2,
            G_TYPE_POINTER, G_TYPE_POINTER);

}

static void
ibus_service_init (IBusService *service)
{
    // IBusServicePrivate *priv = IBUS_SERVICE_GET_PRIVATE (service);
}

static void
ibus_service_finalize (IBusService *service)
{
    DECLARE_PRIV;
    g_free (priv->object_path);
    G_OBJECT_CLASS(_parent_class)->finalize (G_OBJECT (service));
}

gboolean
ibus_service_handle_message (IBusService *service, IBusConnection *connection, DBusMessage *message)
{
    gboolean retval = FALSE;
    g_return_val_if_fail (message != NULL, FALSE);

    g_signal_emit (service, _signals[DBUS_MESSAGE], 0, connection, message, &retval);
    return retval ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static gboolean
ibus_service_dbus_message (IBusService *service, IBusConnection *connection, DBusMessage *message)
{
    return FALSE;
}

static gboolean
ibus_service_dbus_signal (IBusService *service, IBusConnection *connection, DBusMessage *message)
{
    return FALSE;
}


gboolean
_service_message_function (IBusConnection *connection, DBusMessage *message, IBusService *service)
{
    return ibus_service_handle_message (service, connection, message);
}

gboolean
ibus_service_add_to_connection (IBusService *service, IBusConnection *connection)
{
    g_return_val_if_fail (IBUS_IS_SERVICE (service), FALSE);
    g_return_val_if_fail (IBUS_IS_CONNECTION (connection), FALSE);

    DECLARE_PRIV;

    g_return_val_if_fail (priv->object_path != NULL, FALSE);
    g_return_val_if_fail (g_list_find (priv->connections, connection) == NULL, FALSE);

    gboolean retval;
    retval = ibus_connection_register_object_path (connection, priv->object_path,
                    (IBusMessageFunc *) _service_message_function, service);
    if (!retval) {
        g_warning ("Out of memory!");
        return FALSE;
    }

    priv->connections = g_list_append (priv->connections, connection);

    return retval;
}

gboolean
ibus_service_remove_from_connection (IBusService *service, IBusConnection *connection)
{
    g_return_val_if_fail (IBUS_IS_SERVICE (service), FALSE);
    g_return_val_if_fail (IBUS_IS_CONNECTION (connection), FALSE);

    DECLARE_PRIV;

    g_return_val_if_fail (priv->object_path != NULL, FALSE);
    g_return_val_if_fail (g_list_find (priv->connections, connection) != NULL, FALSE);

    gboolean retval;
    retval = ibus_connection_unregister_object_path (connection, priv->object_path);

    if (!retval) {
        g_warning ("Out of memory!");
        return FALSE;
    }
    priv->connections = g_list_remove (priv->connections, connection);
    return TRUE;
}

gboolean
ibus_service_remove_from_all_connections (IBusService *service)
{
    g_return_val_if_fail (IBUS_IS_SERVICE (service), FALSE);

    DECLARE_PRIV;

    g_return_val_if_fail (priv->object_path != NULL, FALSE);

    GList *element = priv->connections;
    while (element != NULL) {
        IBusConnection *connection = IBUS_CONNECTION (element->data);

        gboolean retval;
        retval = ibus_connection_unregister_object_path (connection, priv->object_path);
        if (!retval) {
            g_warning ("Out of memory");
        }
        element = element->next;
    }

    g_list_free (priv->connections);
    priv->connections = NULL;
    return TRUE;
}

