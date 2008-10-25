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

#include <gio/gio.h>
#include "ibusbus.h"
#include "ibusinternal.h"
#include "ibusshare.h"
#include "ibusconnection.h"

#define IBUS_BUS_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_BUS, IBusBusPrivate))

enum {
    CONNECTED,
    DISCONNECTED,
    LAST_SIGNAL,
};


/* IBusBusPriv */
struct _IBusBusPrivate {
    GFileMonitor *monitor;
    IBusConnection *connection;
};
typedef struct _IBusBusPrivate IBusBusPrivate;

static guint    bus_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_bus_class_init     (IBusBusClass   *klass);
static void     ibus_bus_init           (IBusBus        *bus);
static void     ibus_bus_destroy        (IBusObject     *object);

static IBusObjectClass  *parent_class = NULL;

GType
ibus_bus_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusBusClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_bus_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusBus),
        0,
        (GInstanceInitFunc) ibus_bus_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "IBusBus",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}

IBusBus *
ibus_bus_new (void)
{
    IBusBus *bus = IBUS_BUS (g_object_new (IBUS_TYPE_BUS, NULL));

    return bus;
}

static void
ibus_bus_class_init (IBusBusClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusBusPrivate));

    ibus_object_class->destroy = ibus_bus_destroy;
    
    // install signals
    bus_signals[CONNECTED] =
        g_signal_new (I_("connected"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    bus_signals[DISCONNECTED] =
        g_signal_new (I_("disconnected"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

}

static void
_connection_destroy_cb (IBusConnection  *connection,
                        IBusBus         *bus)
{
    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    g_assert (priv->connection == connection);

    g_signal_handlers_disconnect_by_func (priv->connection,
                                          (GCallback) _connection_destroy_cb,
                                          bus);
    g_object_unref (priv->connection);    
    priv->connection = NULL;

    g_signal_emit (bus, bus_signals[DISCONNECTED], 0);
}

static void
ibus_bus_connect (IBusBus *bus)
{
    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    g_assert (priv->connection == NULL);

    priv->connection = ibus_connection_open (ibus_get_address ());

    if (!ibus_connection_is_connected (priv->connection)) {
        g_object_unref (priv->connection);
        priv->connection = NULL;
    }
    else {
        g_signal_connect (priv->connection,
                          "destroy",
                          (GCallback) _connection_destroy_cb,
                          bus);
        g_signal_emit (bus, bus_signals[CONNECTED], 0);
    }
}

static void
_changed_cb (GFileMonitor       *monitor,
             GFile              *file,
             GFile              *other_file,
             GFileMonitorEvent   event_type,
             IBusBus            *bus)
{
    static GFile *socket_file = NULL;

    if (socket_file == NULL) {
        socket_file = g_file_new_for_path (ibus_get_socket_path ());
    }

    if (event_type == G_FILE_MONITOR_EVENT_CREATED) {
        if (g_file_equal (file, socket_file)) {
            ibus_bus_connect (bus);
        }
    }
}

static void
ibus_bus_init (IBusBus *bus)
{
    gchar *path;
    GFile *file;
    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    priv->connection = NULL;

    ibus_bus_connect (bus);

    path = g_strdup_printf ("/tmp/ibus-%s/", ibus_get_user_name ());
    file = g_file_new_for_path (path);
    priv->monitor = g_file_monitor_directory (file, 0, NULL, NULL);
   
    g_signal_connect (priv->monitor, "changed", (GCallback) _changed_cb, NULL);
    
    g_object_unref (file);
    g_free (path);
}

static void
ibus_bus_destroy (IBusObject *object)
{
    IBusBus *bus;
    IBusBusPrivate *priv;
    
    bus = IBUS_BUS (object);
    priv = IBUS_BUS_GET_PRIVATE (bus);

    if (priv->monitor) {
        g_object_unref (priv->monitor);
        priv->monitor = NULL;
    }

    if (priv->connection) {
        ibus_object_destroy (priv->connection);
        priv->connection = NULL;
    }

    IBUS_OBJECT_CLASS (parent_class)->destroy (object);
}

gboolean
ibus_bus_is_connected (IBusBus *bus)
{
    g_assert (IBUS_IS_BUS (bus));

    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    if (priv->connection) {
        return ibus_connection_is_connected (priv->connection);
    }

    return FALSE;
}


IBusInputContext *
ibus_bus_create_input_context (IBusBus      *bus,
                               const gchar  *client_name)
{
    g_assert (IBUS_IS_BUS (bus));
    g_assert (client_name != NULL);
    g_assert (ibus_bus_is_connected (bus));

    DBusMessage *call = NULL;
    DBusMessage *reply = NULL;
    IBusError *error;
    IBusInputContext *context = NULL;
    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    call = dbus_message_new_method_call (IBUS_SERVICE_IBUS,
                                         IBUS_PATH_IBUS,
                                         IBUS_INTERFACE_IBUS,
                                         "CreateInputContext");
    dbus_message_append_args (call,
                              DBUS_TYPE_STRING, &client_name,
                              DBUS_TYPE_INVALID);

    reply = ibus_connection_send_with_reply_and_block (priv->connection,
                                                       call,
                                                       -1,
                                                       &error);
    dbus_message_unref (call);

    if (reply == NULL) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        return NULL;
    }
    else {
        if (dbus_message_get_type (reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN) {
            gchar *path;
            DBusError error;
            dbus_error_init (&error);
            if (!dbus_message_get_args (reply,
                                        &error,
                                        DBUS_TYPE_OBJECT_PATH, &path,
                                        DBUS_TYPE_INVALID)) {
                g_warning ("%s: %s", error.name, error.message);
                dbus_error_free (&error);
            }
            else 
                context = ibus_input_context_new (path,
                                                  priv->connection);
        }
        else if (dbus_message_get_type (reply) == DBUS_MESSAGE_TYPE_ERROR) {
            g_warning ("%s:", dbus_message_get_error_name (reply));
        }
        dbus_message_unref (reply);
    }
    
    return context;
}
