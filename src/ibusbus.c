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

#include "ibusbus.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include "ibusmarshalers.h"
#include "ibusinternal.h"
#include "ibusshare.h"
#include "ibusenginedesc.h"
#include "ibusserializable.h"
#include "ibusconfig.h"

#define DBUS_PATH_DBUS "/org/freedesktop/DBus"
#define DBUS_SERVICE_DBUS "org.freedesktop.DBus"
#define DBUS_INTERFACE_DBUS "org.freedesktop.DBus"

#define IBUS_BUS_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_BUS, IBusBusPrivate))

enum {
    CONNECTED,
    DISCONNECTED,
    GLOBAL_ENGINE_CHANGED,
    LAST_SIGNAL,
};


/* IBusBusPriv */
struct _IBusBusPrivate {
    GFileMonitor *monitor;
    GDBusConnection *connection;
    gboolean watch_dbus_signal;
    IBusConfig *config;
    gchar *unique_name;
};

static guint    bus_signals[LAST_SIGNAL] = { 0 };

static IBusBus *_bus = NULL;

/* functions prototype */
static GObject  *ibus_bus_constructor           (GType                   type,
                                                 guint                   n_params,
                                                 GObjectConstructParam  *params);
static void      ibus_bus_destroy               (IBusObject             *object);
static void      ibus_bus_watch_dbus_signal     (IBusBus                *bus);
static void      ibus_bus_unwatch_dbus_signal   (IBusBus                *bus);
static GVariant *ibus_bus_call                  (IBusBus                *bus,
                                                 const gchar            *service,
                                                 const gchar            *path,
                                                 const gchar            *interface,
                                                 const gchar            *member,
                                                 GVariant               *parameters,
                                                 const GVariantType     *reply_type);

G_DEFINE_TYPE (IBusBus, ibus_bus, IBUS_TYPE_OBJECT)

static void
ibus_bus_class_init (IBusBusClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (class);

    gobject_class->constructor = ibus_bus_constructor;
    ibus_object_class->destroy = ibus_bus_destroy;

    // install signals
    /**
     * IBusBus::connected:
     *
     * Emitted when IBusBus is connected.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
     */
    bus_signals[CONNECTED] =
        g_signal_new (I_("connected"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            _ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /**
     * IBusBus::disconnected:
     *
     * Emitted when IBusBus is disconnected.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
     */
    bus_signals[DISCONNECTED] =
        g_signal_new (I_("disconnected"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            _ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /**
     * IBusBus::global-engine-changed:
     *
     * Emitted when global engine is changed.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
     */
    bus_signals[GLOBAL_ENGINE_CHANGED] =
        g_signal_new (I_("global-engine-changed"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            _ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    g_type_class_add_private (class, sizeof (IBusBusPrivate));
}

#if 0
static gboolean
_connection_ibus_signal_cb (GDBusConnection *connection,
                            IBusMessage    *message,
                            IBusBus        *bus)
{
    if (ibus_message_is_signal (message, IBUS_INTERFACE_IBUS,
                                "GlobalEngineChanged")) {
        g_signal_emit (bus, bus_signals[GLOBAL_ENGINE_CHANGED], 0);
        return TRUE;
    }
    return FALSE;
}
#endif

static void
_connection_closed_cb (GDBusConnection  *connection,
                       gboolean          remote_peer_vanished,
                       GError           *error,
                       IBusBus          *bus)
{
    if (error) {
        g_warning ("%s", error->message);
    }

    g_assert (bus->priv->connection == connection);
    g_signal_handlers_disconnect_by_func (bus->priv->connection,
                                          G_CALLBACK (_connection_closed_cb),
                                          bus);
#if 0
    g_signal_handlers_disconnect_by_func (bus->priv->connection,
                                          G_CALLBACK (_connection_ibus_signal_cb),
                                          bus);
#endif
    g_object_unref (bus->priv->connection);
    bus->priv->connection = NULL;

    g_free (bus->priv->unique_name);
    bus->priv->unique_name = NULL;

    g_signal_emit (bus, bus_signals[DISCONNECTED], 0);
}

static void
ibus_bus_connect (IBusBus *bus)
{
    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    /* destry old connection at first */
    if (priv->connection != NULL) {
        ibus_object_destroy ((IBusObject *)priv->connection);
        g_assert (priv->connection == NULL);
    }

    if (ibus_get_address () != NULL) {
        bus->priv->connection =
            g_dbus_connection_new_for_address_sync (ibus_get_address (),
                                                    G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
                                                    G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
                                                    NULL, NULL, NULL);
    }

    if (bus->priv->connection) {
        /* FIXME */
        ibus_bus_hello (bus);

        g_signal_connect (bus->priv->connection,
                          "closed",
                          (GCallback) _connection_closed_cb,
                          bus);
        g_signal_emit (bus, bus_signals[CONNECTED], 0);

        /* FIXME */
        #if 0
        if (priv->watch_dbus_signal) {
            ibus_bus_watch_dbus_signal (bus);
        }

        /** Watch ibus signals. */
        const gchar *rule =
            "type='signal',"
            "path='" IBUS_PATH_IBUS "',"
            "interface='" IBUS_INTERFACE_IBUS "'";

        ibus_bus_add_match (bus, rule);
        g_signal_connect (priv->connection,
                          "ibus-signal",
                          (GCallback) _connection_ibus_signal_cb,
                          bus);
        #endif
    }
}

static void
_changed_cb (GFileMonitor       *monitor,
             GFile              *file,
             GFile              *other_file,
             GFileMonitorEvent   event_type,
             IBusBus            *bus)
{
    if (event_type != G_FILE_MONITOR_EVENT_CHANGED &&
        event_type != G_FILE_MONITOR_EVENT_CREATED &&
        event_type != G_FILE_MONITOR_EVENT_DELETED)
        return;

    if (ibus_bus_is_connected (bus))
        return;

    ibus_bus_connect (bus);
}

static void
ibus_bus_init (IBusBus *bus)
{
    struct stat buf;
    gchar *path;
    GFile *file;

    bus->priv = IBUS_BUS_GET_PRIVATE (bus);

    bus->priv->config = NULL;
    bus->priv->connection = NULL;
    bus->priv->watch_dbus_signal = FALSE;
    bus->priv->unique_name = NULL;

    path = g_path_get_dirname (ibus_get_socket_path ());

    g_mkdir_with_parents (path, 0700);
    g_chmod (path, 0700);

    if (stat (path, &buf) == 0) {
        if (buf.st_uid != getuid ()) {
            g_warning ("The owner of %s is not %s!", path, ibus_get_user_name ());
            return;
        }
    }

    ibus_bus_connect (bus);

    file = g_file_new_for_path (ibus_get_socket_path ());
    bus->priv->monitor = g_file_monitor_file (file, 0, NULL, NULL);

    g_signal_connect (bus->priv->monitor, "changed", (GCallback) _changed_cb, bus);

    g_object_unref (file);
    g_free (path);
}

static GObject*
ibus_bus_constructor (GType                  type,
                      guint                  n_params,
                      GObjectConstructParam *params)
{
    GObject *object;

    /* share one IBusBus instance in whole application */
    if (_bus == NULL) {
        object = G_OBJECT_CLASS (ibus_bus_parent_class)->constructor (type, n_params, params);
        /* make bus object sink */
        g_object_ref_sink (object);
        _bus = IBUS_BUS (object);
    }
    else {
        object = g_object_ref (_bus);
    }

    return object;
}

static void
ibus_bus_destroy (IBusObject *object)
{
    g_assert (_bus == (IBusBus *)object);

    IBusBus * bus = _bus;
    _bus = NULL;

    if (bus->priv->monitor) {
        g_object_unref (bus->priv->monitor);
        bus->priv->monitor = NULL;
    }

    if (bus->priv->config) {
        ibus_proxy_destroy ((IBusProxy *) bus->priv->config);
        bus->priv->config = NULL;
    }

    if (bus->priv->connection) {
        /* FIXME should use async close function */
        g_dbus_connection_close_sync (bus->priv->connection, NULL, NULL);
        bus->priv->connection = NULL;
    }

    g_free (bus->priv->unique_name);
    bus->priv->unique_name = NULL;

    IBUS_OBJECT_CLASS (ibus_bus_parent_class)->destroy (object);
}

IBusBus *
ibus_bus_new (void)
{
    IBusBus *bus = IBUS_BUS (g_object_new (IBUS_TYPE_BUS, NULL));

    return bus;
}


gboolean
ibus_bus_is_connected (IBusBus *bus)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), FALSE);

    if (bus->priv->connection == NULL || g_dbus_connection_is_closed (bus->priv->connection))
        return FALSE;

    return TRUE;
}


IBusInputContext *
ibus_bus_create_input_context (IBusBus      *bus,
                               const gchar  *client_name)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), NULL);
    g_return_val_if_fail (client_name != NULL, NULL);
    g_return_val_if_fail (ibus_bus_is_connected (bus), NULL);

    gchar *path;
    IBusInputContext *context = NULL;
    GVariant *result;
    result = ibus_bus_call (bus,
                            IBUS_SERVICE_IBUS,
                            IBUS_PATH_IBUS,
                            IBUS_INTERFACE_IBUS,
                            "CreateInputContext",
                            g_variant_new ("(s)", client_name),
                            G_VARIANT_TYPE ("(o)"));

    if (result != NULL) {
        GError *error = NULL;
        g_variant_get (result, "(&o)", &path);
        context = ibus_input_context_new (path, bus->priv->connection, NULL, &error);
        g_variant_unref (result);
        if (context == NULL) {
            g_warning ("%s", error->message);
            g_error_free (error);
        }
    }

    return context;
}

gchar *
ibus_bus_current_input_context (IBusBus      *bus)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), NULL);
    g_return_val_if_fail (ibus_bus_is_connected (bus), NULL);

    gchar *path = NULL;
    GVariant *result;
    result = ibus_bus_call (bus,
                            IBUS_SERVICE_IBUS,
                            IBUS_PATH_IBUS,
                            IBUS_INTERFACE_IBUS,
                            "CurrentInputContext",
                            NULL,
                            G_VARIANT_TYPE ("(o)"));

    if (result != NULL) {
        g_variant_get (result, "(o)", &path);
        g_variant_unref (result);
    }

    return path;
}

static void
ibus_bus_watch_dbus_signal (IBusBus *bus)
{
    const gchar *rule;

    rule = "type='signal'," \
           "path='" DBUS_PATH_DBUS "'," \
           "interface='" DBUS_INTERFACE_DBUS "'";

    ibus_bus_add_match (bus, rule);

}

static void
ibus_bus_unwatch_dbus_signal (IBusBus *bus)
{
    const gchar *rule;

    rule = "type='signal'," \
           "path='" DBUS_PATH_DBUS "'," \
           "interface='" DBUS_INTERFACE_DBUS "'";

    ibus_bus_remove_match (bus, rule);
}

void
ibus_bus_set_watch_dbus_signal (IBusBus        *bus,
                                gboolean        watch)
{
    g_return_if_fail (IBUS_IS_BUS (bus));

    if (bus->priv->watch_dbus_signal == watch)
        return;

    bus->priv->watch_dbus_signal = watch;

    if (ibus_bus_is_connected (bus)) {
        if (watch) {
            ibus_bus_watch_dbus_signal (bus);
        }
        else {
            ibus_bus_unwatch_dbus_signal (bus);
        }
    }
}

const gchar *
ibus_bus_hello (IBusBus *bus)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), NULL);
    /* FIXME gdbus connection will say hello by self. */
#if 1
    if (bus->priv->connection)
        return g_dbus_connection_get_unique_name (bus->priv->connection);
    return NULL;
#else
    g_assert (IBUS_IS_BUS (bus));

    GVariant *result;

    g_free (bus->priv->unique_name);
    bus->priv->unique_name = NULL;

    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "Hello",
                            NULL,
                            G_VARIANT_TYPE ("(s)"));

    if (result) {
        g_variant_get (result, "(s)", &bus->priv->unique_name);
        g_variant_unref (result);
    }

    return bus->priv->unique_name;
#endif
}

guint
ibus_bus_request_name (IBusBus      *bus,
                       const gchar  *name,
                       guint         flags)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), 0);
    g_return_val_if_fail (name != NULL, 0);

    guint retval = 0;
    GVariant *result;
    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "RequestName",
                            g_variant_new ("(su)", name, flags),
                            G_VARIANT_TYPE ("(u)"));

    if (result) {
        g_variant_get (result, "(u)", &retval);
        g_variant_unref (result);
    }

    return retval;
}

guint
ibus_bus_release_name (IBusBus      *bus,
                       const gchar  *name)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), 0);
    g_return_val_if_fail (name != NULL, 0);

    guint retval = 0;
    GVariant *result;
    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "ReleaseName",
                            g_variant_new ("(s)", name),
                            G_VARIANT_TYPE ("(u)"));

    if (result) {
        g_variant_get (result, "(u)", &retval);
        g_variant_unref (result);
    }

    return retval;
}

gboolean
ibus_bus_name_has_owner (IBusBus        *bus,
                         const gchar    *name)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);

    gboolean retval = FALSE;
    GVariant *result;
    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "NameHasOwner",
                            g_variant_new ("(s)", name),
                            G_VARIANT_TYPE ("(b)"));

    if (result) {
        g_variant_get (result, "(b)", &retval);
        g_variant_unref (result);
    }

    return retval;
}

GList *
ibus_bus_list_names (IBusBus    *bus)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), NULL);
    return NULL;
}

void
ibus_bus_add_match (IBusBus     *bus,
                    const gchar *rule)
{
    g_return_if_fail (IBUS_IS_BUS (bus));
    g_return_if_fail (rule != NULL);

    GVariant *result;
    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "AddMatch",
                            g_variant_new ("(s)", rule),
                            NULL);

    if (result) {
        g_variant_unref (result);
    }
}

void
ibus_bus_remove_match (IBusBus      *bus,
                       const gchar  *rule)
{
    g_return_if_fail (IBUS_IS_BUS (bus));
    g_return_if_fail (rule != NULL);

    GVariant *result;
    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "RemoveMatch",
                            g_variant_new ("(s)", rule),
                            NULL);

    if (result) {
        g_variant_unref (result);
    }
}

gchar *
ibus_bus_get_name_owner (IBusBus        *bus,
                         const gchar    *name)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), NULL);

    gchar *retval = NULL;
    GVariant *result;
    result = ibus_bus_call (bus,
                            DBUS_SERVICE_DBUS,
                            DBUS_PATH_DBUS,
                            DBUS_INTERFACE_DBUS,
                            "GetNameOwner",
                            g_variant_new ("(s)", name),
                            G_VARIANT_TYPE ("(s)"));

    if (result) {
        g_variant_get (result, "(s)", &retval);
        g_variant_unref (result);
    }

    return retval;
}

GDBusConnection *
ibus_bus_get_connection (IBusBus *bus)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), NULL);

    return bus->priv->connection;
}

void
ibus_bus_exit (IBusBus *bus,
               gboolean restart)
{
    g_return_if_fail (IBUS_IS_BUS (bus));

    GVariant *result;
    result = ibus_bus_call (bus,
                            IBUS_SERVICE_IBUS,
                            IBUS_PATH_IBUS,
                            IBUS_INTERFACE_IBUS,
                            "Exit",
                            g_variant_new ("(b)", restart),
                            NULL);

    if (result) {
        g_variant_unref (result);
    }
}

gboolean
ibus_bus_register_component (IBusBus       *bus,
                             IBusComponent *component)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), FALSE);
    g_return_val_if_fail (IBUS_IS_COMPONENT (component), FALSE);

    GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)component);
    GVariant *result = ibus_bus_call (bus,
                                      IBUS_SERVICE_IBUS,
                                      IBUS_PATH_IBUS,
                                      IBUS_INTERFACE_IBUS,
                                      "RegisterComponent",
                                      g_variant_new ("(v)", variant),
                                      NULL);
    if (result) {
        g_variant_unref (result);
        return FALSE;
    }
    return TRUE;
}

static GList *
ibus_bus_do_list_engines (IBusBus *bus, gboolean active_engines_only)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), NULL);

    GList *retval = NULL;
    GVariant *result;
    result = ibus_bus_call (bus,
                            IBUS_SERVICE_IBUS,
                            IBUS_PATH_IBUS,
                            IBUS_INTERFACE_IBUS,
                            active_engines_only ? "ListActiveEngines" : "ListEngines",
                            NULL,
                            G_VARIANT_TYPE ("(av)"));

    if (result) {
        GVariantIter *iter = NULL;
        g_variant_get (result, "(av)", &iter);
        GVariant *var;
        while (g_variant_iter_loop (iter, "v", &var)) {
            retval = g_list_append (retval, ibus_serializable_deserialize (var));
        }
        g_variant_iter_free (iter);
        g_variant_unref (result);
    }

    return retval;
}

GList *
ibus_bus_list_engines (IBusBus *bus)
{
    return ibus_bus_do_list_engines (bus, FALSE);
}

GList *
ibus_bus_list_active_engines (IBusBus *bus)
{
    return ibus_bus_do_list_engines (bus, TRUE);
}

static void
_config_destroy_cb (IBusConfig *config,
                    IBusBus    *bus)
{
    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    g_assert (priv->config == config);

    g_object_unref (config);
    priv->config = NULL;
}

IBusConfig *
ibus_bus_get_config (IBusBus *bus)
{
    g_assert (IBUS_IS_BUS (bus));
    g_return_val_if_fail (ibus_bus_is_connected (bus), NULL);

    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    if (priv->config == NULL && priv->connection) {
        priv->config = ibus_config_new (priv->connection, NULL, NULL);
        if (priv->config) {
            g_signal_connect (priv->config, "destroy", G_CALLBACK (_config_destroy_cb), bus);
        }
    }

    return priv->config;
}

gboolean
ibus_bus_get_use_sys_layout (IBusBus *bus)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), FALSE);

    gboolean retval = FALSE;
    GVariant *result;
    result = ibus_bus_call (bus,
                            IBUS_SERVICE_IBUS,
                            IBUS_PATH_IBUS,
                            IBUS_INTERFACE_IBUS,
                            "GetUseSysLayout",
                            NULL,
                            G_VARIANT_TYPE ("(b)"));

    if (result) {
        g_variant_get (result, "(b)", &retval);
        g_variant_unref (result);
    }

    return retval;
}

gboolean
ibus_bus_get_use_global_engine (IBusBus *bus)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), FALSE);

    gboolean retval = FALSE;
    GVariant *result;
    result = ibus_bus_call (bus,
                            IBUS_SERVICE_IBUS,
                            IBUS_PATH_IBUS,
                            IBUS_INTERFACE_IBUS,
                            "GetUseGlobalEngine",
                            NULL,
                            G_VARIANT_TYPE ("(b)"));

    if (result) {
        g_variant_get (result, "(b)", &retval);
        g_variant_unref (result);
    }

    return retval;
}

gboolean
ibus_bus_is_global_engine_enabled (IBusBus *bus)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), FALSE);

    gboolean retval = FALSE;
    GVariant *result;
    result = ibus_bus_call (bus,
                            IBUS_SERVICE_IBUS,
                            IBUS_PATH_IBUS,
                            IBUS_INTERFACE_IBUS,
                            "IsGlobalEngineEnabled",
                            NULL,
                            G_VARIANT_TYPE ("(b)"));

    if (result) {
        g_variant_get (result, "(b)", &retval);
        g_variant_unref (result);
    }

    return retval;
}

IBusEngineDesc *
ibus_bus_get_global_engine (IBusBus *bus)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), NULL);

    GVariant *result;
    IBusEngineDesc *engine = NULL;
    result = ibus_bus_call (bus,
                            IBUS_SERVICE_IBUS,
                            IBUS_PATH_IBUS,
                            IBUS_INTERFACE_IBUS,
                            "GetGlobalEngine",
                            NULL,
                            G_VARIANT_TYPE ("(v)"));

    if (result) {
        GVariant *variant = NULL;
        g_variant_get (result, "(v)", &variant);
        engine = IBUS_ENGINE_DESC (ibus_serializable_deserialize (variant));
        g_variant_unref (variant);
        g_variant_unref (result);
    }

    return engine;
}

gboolean
ibus_bus_set_global_engine (IBusBus     *bus,
                            const gchar *global_engine)
{
    g_return_val_if_fail (IBUS_IS_BUS (bus), FALSE);

    GVariant *result;
    result = ibus_bus_call (bus,
                            IBUS_SERVICE_IBUS,
                            IBUS_PATH_IBUS,
                            IBUS_INTERFACE_IBUS,
                            "SetGlobalEngine",
                            g_variant_new ("(s)", global_engine),
                            NULL);

    if (result) {
        g_variant_unref (result);
    }

    return TRUE;
}

static GVariant *
ibus_bus_call (IBusBus            *bus,
               const gchar        *bus_name,
               const gchar        *path,
               const gchar        *interface,
               const gchar        *member,
               GVariant           *parameters,
               const GVariantType *reply_type)
{
    g_assert (IBUS_IS_BUS (bus));
    g_assert (member != NULL);
    g_return_val_if_fail (ibus_bus_is_connected (bus), NULL);

    GError *error = NULL;
    GVariant *result;
    result = g_dbus_connection_call_sync (bus->priv->connection,
                                          bus_name,
                                          path,
                                          interface,
                                          member,
                                          parameters,
                                          reply_type,
                                          G_DBUS_CALL_FLAGS_NONE,
                                          -1,
                                          NULL,
                                          &error);

    if (result == NULL) {
        g_warning ("%s.%s: %s", interface, member, error->message);
        g_error_free (error);
        return NULL;
    }

    return result;
}
