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
    NAME_OWNER_CHANGED,
    LAST_SIGNAL,
};


/* IBusBusPriv */
struct _IBusBusPrivate {
    GFileMonitor *monitor;
    IBusConnection *connection;
    gboolean watch_dbus_signal;

    IBusConfig *config;
};
typedef struct _IBusBusPrivate IBusBusPrivate;

static guint    bus_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_bus_class_init     (IBusBusClass   *klass);
static void     ibus_bus_init           (IBusBus        *bus);
static void     ibus_bus_destroy        (IBusObject     *object);
static void     ibus_bus_watch_dbus_signal
                                        (IBusBus        *bus);
static void     ibus_bus_unwatch_dbus_signal
                                        (IBusBus        *bus);
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
            G_TYPE_NONE,
            0);

    bus_signals[DISCONNECTED] =
        g_signal_new (I_("disconnected"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);
    
    bus_signals[NAME_OWNER_CHANGED] =
        g_signal_new (I_("name-owner-changed"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING_STRING_STRING,
            G_TYPE_NONE,
            3,
            G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
            G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
            G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE
            );

}

static gboolean
_connection_dbus_signal_cb (IBusConnection *connection,
                            DBusMessage    *message,
                            IBusBus        *bus)
{
    g_assert (IBUS_IS_BUS (bus));
    g_assert (message != NULL);
    g_assert (IBUS_IS_CONNECTION (connection));
    
    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    if (dbus_message_is_signal (message, DBUS_INTERFACE_DBUS, "NameOwnerChanged")) {
        DBusError error;
        const gchar *name;
        const gchar *old_name;
        const gchar *new_name;
        gboolean retval;

        dbus_error_init (&error);
        retval = dbus_message_get_args (message, &error,
                                        DBUS_TYPE_STRING, &name,
                                        DBUS_TYPE_STRING, &old_name,
                                        DBUS_TYPE_STRING, &new_name,
                                        DBUS_TYPE_INVALID);
        if (!retval) {
            g_warning ("%s: %s", error.name, error.message);
            dbus_error_free (&error);
        }
        else {
            g_signal_emit (bus,
                           bus_signals[NAME_OWNER_CHANGED],
                           0,
                           name,
                           old_name,
                           new_name);
        }
    }

    return FALSE;
}

static void
_connection_destroy_cb (IBusConnection  *connection,
                        IBusBus         *bus)
{
    g_assert (IBUS_IS_BUS (bus));
    g_assert (IBUS_IS_CONNECTION (connection));
    
    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    g_assert (priv->connection == connection);
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

    if (priv->connection) {
        g_signal_connect (priv->connection,
                          "dbus-signal",
                          (GCallback) _connection_dbus_signal_cb,
                          bus);

        g_signal_connect (priv->connection,
                          "destroy",
                          (GCallback) _connection_destroy_cb,
                          bus);
        g_signal_emit (bus, bus_signals[CONNECTED], 0);

        if (priv->watch_dbus_signal) {
            ibus_bus_watch_dbus_signal (bus);
        }
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
    priv->watch_dbus_signal = FALSE;

    priv->config = NULL;

    ibus_bus_connect (bus);

    path = g_strdup_printf ("/tmp/ibus-%s/", ibus_get_user_name ());
    file = g_file_new_for_path (path);
    priv->monitor = g_file_monitor_directory (file, 0, NULL, NULL);
   
    g_signal_connect (priv->monitor, "changed", (GCallback) _changed_cb, bus);
    
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
        ibus_object_destroy (IBUS_OBJECT (priv->connection));
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
            DBusError _error;
            dbus_error_init (&_error);
            if (!dbus_message_get_args (reply,
                                        &_error,
                                        DBUS_TYPE_OBJECT_PATH, &path,
                                        DBUS_TYPE_INVALID)) {
                g_warning ("%s: %s", _error.name, _error.message);
                dbus_error_free (&_error);
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

static void
ibus_bus_watch_dbus_signal (IBusBus *bus)
{
    g_assert (IBUS_IS_BUS (bus));
    
    const gchar *rule;
    
    rule = "type='signal'," \
           "path='" DBUS_PATH_DBUS "'," \
           "interface='" DBUS_INTERFACE_DBUS "'";

    ibus_bus_dbus_add_match (bus, rule);

}

static void
ibus_bus_unwatch_dbus_signal (IBusBus *bus)
{
    g_assert (IBUS_IS_BUS (bus));
    g_assert (ibus_bus_is_connected (bus));
    
    const gchar *rule;
    
    rule = "type='signal'," \
           "path='" DBUS_PATH_DBUS "'," \
           "interface='" DBUS_INTERFACE_DBUS "'";

    ibus_bus_dbus_remove_match (bus, rule);
}

void
ibus_bus_set_watch_dbus_signal (IBusBus        *bus,
                                gboolean        watch)
{
    g_assert (IBUS_IS_BUS (bus));
    
    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    if (priv->watch_dbus_signal == watch)
        return;

    priv->watch_dbus_signal = watch;

    if (ibus_bus_is_connected (bus)) {
        if (watch) {
            ibus_bus_watch_dbus_signal (bus);
        }
        else {
            ibus_bus_unwatch_dbus_signal (bus);
        }
    }
}


IBusConfig *
ibus_bus_get_config (IBusBus *bus)
{
    
}

static gboolean
ibus_bus_call (IBusBus      *bus,
               const gchar  *name,
               const gchar  *path,
               const gchar  *interface,
               const gchar  *member,
               gint          first_arg,
               ...)
{
    g_assert (IBUS_IS_BUS (bus));
    g_assert (ibus_bus_is_connected (bus));
    g_assert (name != NULL);
    g_assert (path != NULL);
    g_assert (interface != NULL);
    g_assert (member);
    
    DBusMessage *message, *reply;
    IBusError *error;
    va_list args;
    gint type;
    gboolean retval;
    IBusBusPrivate *priv;

    priv = IBUS_BUS_GET_PRIVATE (bus);

    message = dbus_message_new_method_call (name, path, interface, member);

    va_start (args, first_arg);
    dbus_message_append_args_valist (message, first_arg, args);
    va_end (args);
    
    reply = ibus_connection_send_with_reply_and_block (
                                        priv->connection,
                                        message,
                                        -1,
                                        &error);
    dbus_message_unref (message);

    if (reply == NULL) {
        g_warning ("%s : %s", error->name, error->message);
        ibus_error_free (error);
        return FALSE;
    }

    if (dbus_message_get_type (reply) == DBUS_MESSAGE_TYPE_ERROR) {
        g_warning ("%s", dbus_message_get_error_name (reply));
        dbus_message_unref (reply);
        return FALSE;
    }

    va_start (args, first_arg);
    
    type = first_arg;
    
    while (type != DBUS_TYPE_INVALID) {
        va_arg (args, void *);
        type = va_arg (args, gint);
    }
    
    type = va_arg (args, gint);
    DBusError _error;
    dbus_error_init (&_error);
    retval = dbus_message_get_args_valist (message, &_error, type, args);
    va_end (args);

    dbus_message_unref (reply);

    if (!retval) {
        g_warning ("%s: %s", _error.name, _error.message);
        dbus_error_free (&_error);
        return FALSE;
    }

    return TRUE;
}

const gchar *
ibus_bus_hello (IBusBus *bus)
{
    g_assert (IBUS_IS_BUS (bus));

    gchar *unique_name = NULL;
    gboolean result;

    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "Hello",
                            DBUS_TYPE_INVALID,
                            DBUS_TYPE_STRING, &unique_name,
                            DBUS_TYPE_INVALID);

    if (result)
        return unique_name;

    return NULL;
}

guint
ibus_bus_request_name (IBusBus      *bus,
                       const gchar  *name,
                       guint         flags)
{
    g_assert (IBUS_IS_BUS (bus));

    guint retval;
    gboolean result;

    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "RequestName",
                            DBUS_TYPE_STRING, &name,
                            DBUS_TYPE_UINT32, &flags,
                            DBUS_TYPE_INVALID,
                            DBUS_TYPE_UINT32, &retval,
                            DBUS_TYPE_INVALID);

    if (result)
        return retval;

    return 0;
}

guint
ibus_bus_release_name (IBusBus      *bus,
                       const gchar  *name)
{
    g_assert (IBUS_IS_BUS (bus));

    guint retval;
    gboolean result;

    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "ReleaseName",
                            DBUS_TYPE_STRING, &name,
                            DBUS_TYPE_INVALID,
                            DBUS_TYPE_UINT32, &retval,
                            DBUS_TYPE_INVALID);

    if (result)
        return retval;

    return 0;
}

gboolean
ibus_bus_name_has_owner (IBusBus        *bus,
                         const gchar    *name)
{
    g_assert (IBUS_IS_BUS (bus));

    gboolean retval;
    gboolean result;

    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "NameHasOwner",
                            DBUS_TYPE_STRING, &name,
                            DBUS_TYPE_INVALID,
                            DBUS_TYPE_BOOLEAN, &retval,
                            DBUS_TYPE_INVALID);

    if (result)
        return retval;

    return FALSE;
}

GList *
ibus_bus_list_names (IBusBus    *bus)
{

}

void
ibus_bus_add_match (IBusBus     *bus,
                    const gchar *rule)
{
    g_assert (IBUS_IS_BUS (bus));

    gboolean result;

    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "AddMatch",
                            DBUS_TYPE_STRING, &rule,
                            DBUS_TYPE_INVALID,
                            DBUS_TYPE_INVALID);
}

void
ibus_bus_remove_match (IBusBus      *bus,
                       const gchar  *rule)
{
    g_assert (IBUS_IS_BUS (bus));

    gboolean result;

    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "RemoveMatch",
                            DBUS_TYPE_STRING, &rule,
                            DBUS_TYPE_INVALID,
                            DBUS_TYPE_INVALID);
}

const gchar *
ibus_bus_get_name_owner (IBusBus        *bus,
                         const gchar    *name)
{
    g_assert (IBUS_IS_BUS (bus));

    gchar *owner = NULL;
    gboolean result;

    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "RemoveMatch",
                            DBUS_TYPE_STRING, &name,
                            DBUS_TYPE_INVALID,
                            DBUS_TYPE_STRING, &owner,
                            DBUS_TYPE_INVALID);

    if (result)
        return owner;

    return NULL;
}

