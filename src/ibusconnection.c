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
#include <stdarg.h>
#include "ibusmessage.h"
#include "ibusconnection.h"
#include "ibusinternal.h"

#define IBUS_CONNECTION_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_CONNECTION, IBusConnectionPrivate))

enum {
    IBUS_SIGNAL,
    IBUS_MESSAGE,
    IBUS_MESSAGE_SENT,
    DISCONNECTED,
    LAST_SIGNAL,
};


/* IBusConnectionPriv */
struct _IBusConnectionPrivate {
    DBusConnection *connection;
    gboolean shared;
};
typedef struct _IBusConnectionPrivate IBusConnectionPrivate;

static guint            connection_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_connection_class_init  (IBusConnectionClass    *klass);
static void     ibus_connection_init        (IBusConnection         *connection);
static void     ibus_connection_destroy     (IBusConnection         *connection);

static gboolean ibus_connection_ibus_message(IBusConnection         *connection,
                                             IBusMessage            *message);
static gboolean ibus_connection_ibus_signal (IBusConnection         *connection,
                                             IBusMessage            *message);
static void     ibus_connection_disconnected(IBusConnection         *connection);
static DBusHandlerResult
                _connection_handle_message_cb(DBusConnection        *dbus_connection,
                                              IBusMessage           *message,
                                              IBusConnection        *connection);

static IBusObjectClass  *parent_class = NULL;
static GHashTable       *_connections = NULL;

GType
ibus_connection_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusConnectionClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_connection_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusConnection),
        0,
        (GInstanceInitFunc) ibus_connection_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "IBusConnection",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}

IBusConnection *
ibus_connection_new (void)
{
    GObject *object;
    object = g_object_new (IBUS_TYPE_CONNECTION, NULL);
    return IBUS_CONNECTION (object);
}

static void
ibus_connection_class_init (IBusConnectionClass *klass)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusConnectionPrivate));

    object_class->destroy = (IBusObjectDestroyFunc) ibus_connection_destroy;

    klass->ibus_message = ibus_connection_ibus_message;
    klass->ibus_signal  = ibus_connection_ibus_signal;
    klass->disconnected = ibus_connection_disconnected;

    connection_signals[IBUS_MESSAGE] =
        g_signal_new (I_("ibus-message"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusConnectionClass, ibus_message),
            NULL, NULL,
            ibus_marshal_BOOLEAN__POINTER,
            G_TYPE_BOOLEAN, 1,
            G_TYPE_POINTER);

    connection_signals[IBUS_SIGNAL] =
        g_signal_new (I_("ibus-signal"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusConnectionClass, ibus_signal),
            NULL, NULL,
            ibus_marshal_BOOL__POINTER,
            G_TYPE_BOOLEAN, 1,
            G_TYPE_POINTER);

    connection_signals[IBUS_MESSAGE_SENT] =
        g_signal_new (I_("ibus-message-sent"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusConnectionClass, ibus_message_sent),
            NULL, NULL,
            ibus_marshal_VOID__POINTER,
            G_TYPE_NONE, 1,
            G_TYPE_POINTER);

    connection_signals[DISCONNECTED] =
        g_signal_new (I_("disconnected"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusConnectionClass, disconnected),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

}

static void
ibus_connection_init (IBusConnection *connection)
{
    IBusConnectionPrivate *priv;
    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    priv->connection = NULL;
    priv->shared = FALSE;
}

static void
ibus_connection_destroy (IBusConnection *connection)
{
    IBusConnectionPrivate *priv;
    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    if (priv->connection) {
        dbus_connection_remove_filter (priv->connection,
                    (DBusHandleMessageFunction) _connection_handle_message_cb,
                    connection);
    }

    if (!priv->shared && priv->connection) {
        dbus_connection_close (priv->connection);
        dbus_connection_unref (priv->connection);
        priv->connection = NULL;
        goto _out;
    }

    if (priv->shared && priv->connection) {
        g_warn_if_fail (_connections != NULL);
        if (_connections != NULL) {
            g_hash_table_remove (_connections, priv->connection);
        }
        dbus_connection_unref (priv->connection);
        priv->connection = NULL;
        goto _out;
    }
_out:
    parent_class->destroy (IBUS_OBJECT (connection));
}

static gboolean
ibus_connection_ibus_message (IBusConnection *connection,
                              IBusMessage    *message)
{
    gboolean retval = FALSE;

    if (ibus_message_get_type (message) == DBUS_MESSAGE_TYPE_SIGNAL)
        g_signal_emit (connection, connection_signals[IBUS_SIGNAL], 0, message, &retval);

    return retval;
}

static gboolean
ibus_connection_ibus_signal (IBusConnection *connection, IBusMessage *message)
{
    IBusConnectionPrivate *priv;
    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    if (ibus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
        g_signal_emit (connection, connection_signals[DISCONNECTED], 0);
        return FALSE;
    }
    return FALSE;
}

static void
ibus_connection_disconnected (IBusConnection         *connection)
{
    ibus_object_destroy (IBUS_OBJECT (connection));
}

static DBusHandlerResult
_connection_handle_message_cb (DBusConnection   *dbus_connection,
                               IBusMessage      *message,
                               IBusConnection   *connection)
{
    gboolean retval = FALSE;

    gint type;

    type = ibus_message_get_type (message);
    g_signal_emit (connection, connection_signals[IBUS_MESSAGE], 0, message, &retval);

    if (retval)
        return DBUS_HANDLER_RESULT_HANDLED;

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static gint
_get_slot ()
{
    static gint slot = -1;
    if (slot == -1) {
        dbus_connection_allocate_data_slot (&slot);
    }
    return slot;
}

void
ibus_connection_set_connection (IBusConnection *connection, DBusConnection *dbus_connection, gboolean shared)
{
    gboolean result;
    IBusConnectionPrivate *priv;

    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (dbus_connection != NULL);
    g_assert (dbus_connection_get_is_connected (dbus_connection));

    priv = IBUS_CONNECTION_GET_PRIVATE (connection);
    g_assert (priv->connection == NULL);

    priv->connection = dbus_connection_ref (dbus_connection);
    priv->shared = shared;

    dbus_connection_set_data (priv->connection, _get_slot(), connection, NULL);

    result = dbus_connection_add_filter (priv->connection,
                    (DBusHandleMessageFunction) _connection_handle_message_cb,
                    connection, NULL);

    dbus_connection_setup (priv->connection, NULL);
    g_warn_if_fail (result);
}

IBusConnection *
ibus_connection_open (const gchar *address)
{
    g_assert (address != NULL);

    DBusError error;
    DBusConnection *dbus_connection;
    IBusConnection *connection;

    if (_connections == NULL) {
        _connections = g_hash_table_new (g_direct_hash, g_direct_equal);
    }


    dbus_error_init (&error);
    dbus_connection = dbus_connection_open (address, &error);
    if (dbus_connection == NULL) {
        g_warning ("Connect to %s failed. %s.", address, error.message);
        dbus_error_free (&error);
        return NULL;
    }

    connection = g_hash_table_lookup (_connections, dbus_connection);

    if (connection) {
        dbus_connection_unref (dbus_connection);
        g_object_ref (connection);
        return connection;
    }

    connection = ibus_connection_new ();
    ibus_connection_set_connection (connection, dbus_connection, TRUE);
    g_hash_table_insert (_connections, dbus_connection, connection);

    return connection;
}

IBusConnection *
ibus_connection_open_private (const gchar *address)
{
    g_assert (address != NULL);

    DBusError error;
    DBusConnection *dbus_connection;
    IBusConnection *connection;

    dbus_error_init (&error);
    dbus_connection = dbus_connection_open_private (address, &error);
    if (dbus_connection == NULL) {
        g_warning ("Connect to %s failed. %s.", address, error.message);
        dbus_error_free (&error);
        return NULL;
    }

    connection = ibus_connection_new ();
    ibus_connection_set_connection (connection, dbus_connection, FALSE);

    return connection;
}

void ibus_connection_close (IBusConnection     *connection)
{
    g_assert (IBUS_IS_CONNECTION (connection));

    IBusConnectionPrivate *priv;
    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    dbus_connection_close (priv->connection);
}

gboolean
ibus_connection_is_connected (IBusConnection *connection)
{
    IBusConnectionPrivate *priv;
    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    if (priv->connection == NULL) {
        return FALSE;
    }
    return dbus_connection_get_is_connected (priv->connection);
}

DBusConnection *
ibus_connection_get_connection (IBusConnection *connection)
{
    IBusConnectionPrivate *priv;
    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    return priv->connection;
}

typedef struct _VTableCallData {
    IBusMessageFunc message_func;
    gpointer user_data;
}VTableCallData;

void
_unregister_function (DBusConnection *dbus_connection, VTableCallData *data)
{
    g_slice_free (VTableCallData, data);
}

DBusHandlerResult
_message_function (DBusConnection *dbus_connection,
                   DBusMessage    *message,
                   VTableCallData *data)
{
    gboolean retval;
    IBusConnection *connection;

    connection = IBUS_CONNECTION (dbus_connection_get_data (dbus_connection, _get_slot()));
    retval = data->message_func (connection, message, data->user_data);

    return retval ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

gboolean
ibus_connection_register_object_path (IBusConnection *connection,
        const gchar *path, IBusMessageFunc message_func, gpointer user_data)
{
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (path != NULL);
    g_assert (message_func != NULL);

    IBusConnectionPrivate *priv;
    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    gboolean retval;
    DBusObjectPathVTable vtable = {0};
    VTableCallData *data;

    vtable.unregister_function = (DBusObjectPathUnregisterFunction) _unregister_function;
    vtable.message_function = (DBusObjectPathMessageFunction) _message_function;

    data = g_slice_new (VTableCallData);
    data->message_func = message_func;
    data->user_data = user_data;

    retval = dbus_connection_register_object_path (priv->connection, path, &vtable, data);
    if (!retval) {
        g_warning ("Out of memory!");
        return FALSE;
    }
    return TRUE;
}

gboolean
ibus_connection_unregister_object_path (IBusConnection *connection, const gchar *path)
{
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (path != NULL);

    IBusConnectionPrivate *priv;
    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    gboolean retval;

    retval = dbus_connection_unregister_object_path (priv->connection, path);
    if (!retval) {
        g_warning ("Out of memory!");
        return FALSE;
    }

    return TRUE;
}


gboolean
ibus_connection_send (IBusConnection *connection,
                      IBusMessage    *message)
{
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    gboolean retval;
    IBusConnectionPrivate *priv;

    priv = IBUS_CONNECTION_GET_PRIVATE (connection);
    retval = dbus_connection_send (priv->connection, message, NULL);

    if (retval) {
        g_signal_emit (connection,
                       connection_signals[IBUS_MESSAGE_SENT],
                       0,
                       message);
    }

    return retval;
}


gboolean
ibus_connection_send_signal (IBusConnection *connection,
                             const gchar    *path,
                             const gchar    *interface,
                             const gchar    *name,
                             GType          first_arg_type,
                             ...)
{
    va_list args;
    gboolean retval;

    va_start (args, first_arg_type);
    retval = ibus_connection_send_signal_valist (connection,
                                                 path,
                                                 interface,
                                                 name,
                                                 first_arg_type,
                                                 args);
    va_end (args);
    return retval;
}

gboolean
ibus_connection_send_signal_valist (IBusConnection  *connection,
                                    const gchar     *path,
                                    const gchar     *interface,
                                    const gchar     *name,
                                    GType            first_arg_type,
                                    va_list          args)
{
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (interface != NULL);
    g_assert (name != NULL);

    gboolean retval;
    IBusMessage *message;

    message = ibus_message_new_signal (path, interface, name);

    ibus_message_append_args_valist (message, first_arg_type, args);
    retval = ibus_connection_send (connection, message);
    ibus_message_unref (message);

    return retval;
}

gboolean
ibus_connection_send_valist (IBusConnection  *connection,
                             gint             message_type,
                             const gchar     *path,
                             const gchar     *interface,
                             const gchar     *name,
                             GType            first_arg_type,
                             va_list          args)
{
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (interface != NULL);
    g_assert (name != NULL);

    gboolean retval;
    IBusMessage *message;

    message = ibus_message_new (message_type);
    ibus_message_set_path (message, path);
    ibus_message_set_interface (message, interface);
    ibus_message_set_member (message, name);

    ibus_message_append_args_valist (message, first_arg_type, args);
    retval = ibus_connection_send (connection, message);
    ibus_message_unref (message);

    return retval;
}

gboolean
ibus_connection_send_with_reply (IBusConnection   *connection,
                                 IBusMessage      *message,
                                 IBusPendingCall **pending_return,
                                 gint              timeout_milliseconds)
{
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (message != NULL);
    g_assert (pending_return != NULL);
    g_assert (timeout_milliseconds > 0 || timeout_milliseconds == -1);

    IBusConnectionPrivate *priv;
    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    return dbus_connection_send_with_reply (priv->connection,
                                            message,
                                            pending_return,
                                            timeout_milliseconds);
}

IBusMessage *
ibus_connection_send_with_reply_and_block (IBusConnection   *connection,
                                           IBusMessage      *message,
                                           gint              timeout_milliseconds,
                                           IBusError        **error)
{
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (message != NULL);
    g_assert (timeout_milliseconds > 0 || timeout_milliseconds == -1);

    IBusError *_error;
    IBusMessage *reply;
    IBusConnectionPrivate *priv;
    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    _error = ibus_error_new ();

    reply = dbus_connection_send_with_reply_and_block (priv->connection,
                                                       message,
                                                       timeout_milliseconds,
                                                       _error);

    if (reply != NULL) {
        g_signal_emit (connection,
                       connection_signals[IBUS_MESSAGE_SENT],
                       0,
                       message);
    }

    if (reply == NULL && error != NULL) {
            *error = _error;
    }
    else
        ibus_error_free (_error);

    return reply;
}

gboolean
ibus_connection_call (IBusConnection     *connection,
                      const gchar        *name,
                      const gchar        *path,
                      const gchar        *interface,
                      const gchar        *member,
                      IBusError          **error,
                      GType              first_arg_type,
                      ...)
{
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (name != NULL);
    g_assert (path != NULL);
    g_assert (interface != NULL);
    g_assert (member != NULL);

    IBusConnectionPrivate *priv;
    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    g_assert (dbus_connection_get_is_connected (priv->connection));

    IBusMessage *message, *reply;
    IBusError *tmp_error;
    va_list args;
    GType type;
    gboolean retval;

    message = ibus_message_new_method_call (name, path, interface, member);

    va_start (args, first_arg_type);

    ibus_message_append_args_valist (message, first_arg_type, args);

    va_end (args);

    reply = ibus_connection_send_with_reply_and_block (
                                        connection,
                                        message,
                                        -1,
                                        error);
    ibus_message_unref (message);

    if (reply == NULL) {
        return FALSE;
    }

    if ((tmp_error = ibus_error_new_from_message (reply)) != NULL) {
        if (error) {
            *error = tmp_error;
        }
        else
            ibus_error_free (tmp_error);
        ibus_message_unref (reply);
        return FALSE;
    }

    va_start (args, first_arg_type);

    type = first_arg_type;

    while (type != DBUS_TYPE_INVALID) {
        va_arg (args, gpointer);
        type = va_arg (args, GType);
    }
    type = va_arg (args, GType);

    if (type != G_TYPE_INVALID) {
        retval = ibus_message_get_args_valist (reply, error, type, args);
    }

    va_end (args);

    ibus_message_unref (reply);

    if (!retval) {
        return FALSE;
    }

    return TRUE;
}

void
ibus_connection_flush (IBusConnection *connection)
{
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (ibus_connection_is_connected (connection));

    IBusConnectionPrivate *priv;

    priv = IBUS_CONNECTION_GET_PRIVATE (connection);

    dbus_connection_flush (priv->connection);
}
