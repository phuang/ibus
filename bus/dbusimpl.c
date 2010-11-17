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
#include "dbusimpl.h"
#include <string.h>
#include "types.h"
#include "marshalers.h"
#include "matchrule.h"

enum {
    NAME_OWNER_CHANGED,
    LAST_SIGNAL,
};

enum {
    PROP_0,
};

static guint dbus_signals[LAST_SIGNAL] = { 0 };

struct _BusDBusImpl {
    IBusService parent;

    /* instance members */
    /* a map from a unique bus name (e.g. ":1.0") to a BusConnection. */
    GHashTable *unique_names;
    /* a map from a requested well-known name (e.g. "org.freedesktop.IBus.Panel") to a BusConnection. */
    GHashTable *names;
    /* a list of IBusService objects. */
    GList *objects;
    /* a list of active BusConnections. */
    GList *connections;
    /* a list of BusMatchRules requested by the connections above. */
    GList *rules;
    /* a serial number used to generate a unique name of a bus. */
    guint id;

    GMutex *dispatch_lock;
    GList *dispatch_queue;

    GMutex *forward_lock;
    GList *forward_queue;
};

struct _BusDBusImplClass {
    IBusServiceClass parent;

    /* class members */
    void    (* name_owner_changed) (BusDBusImpl     *dbus,
                                    gchar           *name,
                                    gchar           *old_name,
                                    gchar           *new_name);
};

typedef struct _BusDispatchData BusDispatchData;
struct _BusDispatchData {
    GDBusMessage *message;
    BusConnection *skip_connection;
};


/* functions prototype */
static void     bus_dbus_impl_destroy           (BusDBusImpl        *dbus);
static void     bus_dbus_impl_service_method_call
                                                (IBusService        *service,
                                                 GDBusConnection    *dbus_connection,
                                                 const gchar        *sender,
                                                 const gchar        *object_path,
                                                 const gchar        *interface_name,
                                                 const gchar        *method_name,
                                                 GVariant           *parameters,
                                                 GDBusMethodInvocation
                                                                    *invocation);
static GVariant *bus_dbus_impl_service_get_property
                                                (IBusService        *service,
                                                 GDBusConnection    *connection,
                                                 const gchar        *sender,
                                                 const gchar        *object_path,
                                                 const gchar        *interface_name,
                                                 const gchar        *property_name,
                                                 GError            **error);
static gboolean  bus_dbus_impl_service_set_property
                                                (IBusService        *service,
                                                 GDBusConnection    *connection,
                                                 const gchar        *sender,
                                                 const gchar        *object_path,
                                                 const gchar        *interface_name,
                                                 const gchar        *property_name,
                                                 GVariant           *value,
                                                 GError            **error);
static void      bus_dbus_impl_name_owner_changed
                                                (BusDBusImpl        *dbus,
                                                 gchar              *name,
                                                 gchar              *old_name,
                                                 gchar              *new_name);
static void      bus_dbus_impl_connection_destroy_cb
                                                (BusConnection      *connection,
                                                 BusDBusImpl        *dbus);
static void      bus_dbus_impl_rule_destroy_cb  (BusMatchRule       *rule,
                                                 BusDBusImpl        *dbus);
static void      bus_dbus_impl_object_destroy_cb(IBusService        *object,
                                                 BusDBusImpl        *dbus);

G_DEFINE_TYPE(BusDBusImpl, bus_dbus_impl, IBUS_TYPE_SERVICE)

/* The D-Bus interfaces available in this class, which consists of a list of methods this class implements and
 * a list of signals this class may emit. See bus_dbus_impl_new_connection and ibusservice.c for more details. */
static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='org.freedesktop.DBus'>"
    "    <method name='Hello'>"
    "      <arg direction='out' type='s' name='unique_name' />"
    "    </method>"
    "    <method name='RequestName'>"
    "      <arg direction='in'  type='s' name='name' />"
    "      <arg direction='in'  type='u' name='flags' />"
    "      <arg direction='out' type='u' />"
    "    </method>"
    "    <method name='ReleaseName'>"
    "      <arg direction='in'  type='s' name='name' />"
    "      <arg direction='out' type='u' />"
    "    </method>"
    "    <method name='StartServiceByName'>"
    "      <arg direction='in'  type='s' />"
    "      <arg direction='in'  type='u' />"
    "      <arg direction='out' type='u' />"
    "    </method>"
    "    <method name='UpdateActivationEnvironment'>"
    "      <arg direction='in' type='a{ss}'/>"
    "    </method>"
    "    <method name='NameHasOwner'>"
    "      <arg direction='in'  type='s' name='name' />"
    "      <arg direction='out' type='b' />"
    "    </method>"
    "    <method name='ListNames'>"
    "      <arg direction='out' type='as' />"
    "    </method>"
    "    <method name='ListActivatableNames'>"
    "      <arg direction='out' type='as' />"
    "    </method>"
    "    <method name='AddMatch'>"
    "      <arg direction='in'  type='s' name='match_rule' />"
    "    </method>"
    "    <method name='RemoveMatch'>"
    "      <arg direction='in'  type='s' name='match_rule' />"
    "    </method>"
    "    <method name='GetNameOwner'>"
    "      <arg direction='in'  type='s' name='name' />"
    "      <arg direction='out' type='s' name='unique_name' />"
    "    </method>"
    "    <method name='ListQueuedOwners'>"
    "      <arg direction='in'  type='s' />"
    "      <arg direction='out' type='as' />"
    "    </method>"
    "    <method name='GetConnectionUnixUser'>"
    "      <arg direction='in'  type='s' />"
    "      <arg direction='out' type='u' />"
    "    </method>"
    "    <method name='GetConnectionUnixProcessID'>"
    "      <arg direction='in'  type='s' />"
    "      <arg direction='out' type='u' />"
    "    </method>"
    "    <method name='GetAdtAuditSessionData'>"
    "      <arg direction='in'  type='s' />"
    "      <arg direction='out' type='ay' />"
    "    </method>"
    "    <method name='GetConnectionSELinuxSecurityContext'>"
    "      <arg direction='in'  type='s' />"
    "      <arg direction='out' type='ay' />"
    "    </method>"
    "    <method name='ReloadConfig' />"
    "    <method name='GetId'>"
    "      <arg direction='out' type='s' />"
    "    </method>"
    "    <signal name='NameOwnerChanged'>"
    "      <arg type='s' name='name' />"
    "      <arg type='s' name='old_owner' />"
    "      <arg type='s' name='new_owner' />"
    "    </signal>"
    "    <signal name='NameLost'>"
    "      <arg type='s' name='name' />"
    "    </signal>"
    "    <signal name='NameAcquired'>"
    "      <arg type='s' name='name' />"
    "    </signal>"
    "  </interface>"
    "</node>";

static void
bus_dbus_impl_class_init (BusDBusImplClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);

    IBUS_OBJECT_CLASS (gobject_class)->destroy = (IBusObjectDestroyFunc) bus_dbus_impl_destroy;

    /* override the default implementations in the parent class. */
    IBUS_SERVICE_CLASS (class)->service_method_call =  bus_dbus_impl_service_method_call;
    IBUS_SERVICE_CLASS (class)->service_get_property = bus_dbus_impl_service_get_property;
    IBUS_SERVICE_CLASS (class)->service_set_property = bus_dbus_impl_service_set_property;

    ibus_service_class_add_interfaces (IBUS_SERVICE_CLASS (class), introspection_xml);

    /* register a handler of the name-owner-changed signal below. */
    class->name_owner_changed = bus_dbus_impl_name_owner_changed;

    /* install signals */
    dbus_signals[NAME_OWNER_CHANGED] =
        g_signal_new (I_("name-owner-changed"),
            G_TYPE_FROM_CLASS (class),
            G_SIGNAL_RUN_FIRST,
            G_STRUCT_OFFSET (BusDBusImplClass, name_owner_changed),
            NULL, NULL,
            bus_marshal_VOID__STRING_STRING_STRING,
            G_TYPE_NONE,
            3,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_STRING);
}

static void
bus_dbus_impl_init (BusDBusImpl *dbus)
{
    dbus->unique_names = g_hash_table_new (g_str_hash, g_str_equal);
    dbus->names = g_hash_table_new (g_str_hash, g_str_equal);

    dbus->dispatch_lock = g_mutex_new ();
    dbus->forward_lock = g_mutex_new ();

    /* other members are automatically zero-initialized. */
}

static void
bus_dbus_impl_destroy (BusDBusImpl *dbus)
{
    GList *p;

    for (p = dbus->objects; p != NULL; p = p->next) {
        IBusService *object = (IBusService *) p->data;
        g_signal_handlers_disconnect_by_func (object, G_CALLBACK (bus_dbus_impl_object_destroy_cb), dbus);
        ibus_object_destroy ((IBusObject *) object);
        g_object_unref (object);
    }
    g_list_free (dbus->objects);
    dbus->objects = NULL;

    for (p = dbus->rules; p != NULL; p = p->next) {
        BusMatchRule *rule = BUS_MATCH_RULE (p->data);
        g_signal_handlers_disconnect_by_func (rule,
                        G_CALLBACK (bus_dbus_impl_rule_destroy_cb), dbus);
        ibus_object_destroy ((IBusObject *) rule);
        g_object_unref (rule);
    }
    g_list_free (dbus->rules);
    dbus->rules = NULL;

    for (p = dbus->connections; p != NULL; p = p->next) {
        GDBusConnection *connection = G_DBUS_CONNECTION (p->data);
        g_signal_handlers_disconnect_by_func (connection, bus_dbus_impl_connection_destroy_cb, dbus);
        /* FIXME should handle result? */
        g_dbus_connection_close (connection, NULL, NULL, NULL);
        g_object_unref (connection);
    }
    g_list_free (dbus->connections);
    dbus->connections = NULL;

    g_hash_table_remove_all (dbus->unique_names);
    g_hash_table_remove_all (dbus->names);

    dbus->unique_names = NULL;
    dbus->names = NULL;

    /* FIXME destruct _lock and _queue members. */
    IBUS_OBJECT_CLASS(bus_dbus_impl_parent_class)->destroy ((IBusObject *) dbus);
}

/**
 * bus_dbus_impl_hello:
 *
 * Implement the "Hello" method call of the org.freedesktop.DBus interface.
 * Assign a unique bus name like ":1.0" for the connection and return the name (as a D-Bus reply.)
 */
static void
bus_dbus_impl_hello (BusDBusImpl           *dbus,
                     BusConnection         *connection,
                     GVariant              *parameters,
                     GDBusMethodInvocation *invocation)
{
    if (bus_connection_get_unique_name (connection) != NULL) {
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "Already handled an Hello message");
    }
    else {
        gchar *name = g_strdup_printf (":1.%u", ++dbus->id);
        bus_connection_set_unique_name (connection, name);
        g_free (name);

        name = (gchar *) bus_connection_get_unique_name (connection);
        g_hash_table_insert (dbus->unique_names, name, connection);
        g_dbus_method_invocation_return_value (invocation, g_variant_new ("(s)", name));

        g_signal_emit (dbus,
                       dbus_signals[NAME_OWNER_CHANGED],
                       0,
                       name,
                       "",
                       name);
    }
}

/**
 * bus_dbus_impl_list_names:
 *
 * Implement the "ListNames" method call of the org.freedesktop.DBus interface.
 * Return all bus names (e.g. ":1.0", "org.freedesktop.IBus.Panel") as a D-Bus reply.
 */
static void
bus_dbus_impl_list_names (BusDBusImpl           *dbus,
                          BusConnection         *connection,
                          GVariant              *parameters,
                          GDBusMethodInvocation *invocation)
{
    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

    /* FIXME should add them? */
    g_variant_builder_add (&builder, "s", "org.freedesktop.DBus");
    g_variant_builder_add (&builder, "s", "org.freedesktop.IBus");

    /* append well-known names */
    GList *names, *name;
    names = g_hash_table_get_keys (dbus->names);
    for (name = names; name != NULL; name = name->next) {
        g_variant_builder_add (&builder, "s", name->data);
    }
    g_list_free (names);

    /* append unique names */
    names = g_hash_table_get_keys (dbus->unique_names);
    for (name = names; name != NULL; name = name->next) {
        g_variant_builder_add (&builder, "s", name->data);
    }
    g_list_free (names);

    g_dbus_method_invocation_return_value (invocation,
                    g_variant_new ("(as)", &builder));
}

/**
 * bus_dbus_impl_list_names:
 *
 * Implement the "NameHasOwner" method call of the org.freedesktop.DBus interface.
 * Return TRUE (as a D-Bus reply) if the name is available in dbus->unique_names or is a well-known name.
 */
static void
bus_dbus_impl_name_has_owner (BusDBusImpl           *dbus,
                              BusConnection         *connection,
                              GVariant              *parameters,
                              GDBusMethodInvocation *invocation)
{
    const gchar *name = NULL;
    g_variant_get (parameters, "(&s)", &name);

    gboolean has_owner;
    if (!g_dbus_is_name (name)) {
        g_dbus_method_invocation_return_error (invocation,
                                               G_DBUS_ERROR,
                                               G_DBUS_ERROR_FAILED,
                                               "'%s' is not a legal bus name");
        return;
    }

    if (g_dbus_is_unique_name (name)) {
        has_owner = g_hash_table_lookup (dbus->unique_names, name) != NULL;
    }
    else {
        if (g_strcmp0 (name, "org.freedesktop.DBus") == 0 ||
            g_strcmp0 (name, "org.freedesktop.IBus") == 0)
            has_owner = TRUE;
        else
            has_owner = g_hash_table_lookup (dbus->names, name) != NULL;
    }
    g_dbus_method_invocation_return_value (invocation,
                    g_variant_new ("(b)", has_owner));
}

/**
 * bus_dbus_impl_get_name_owner:
 *
 * Implement the "GetNameOwner" method call of the org.freedesktop.DBus interface.
 */
static void
bus_dbus_impl_get_name_owner (BusDBusImpl           *dbus,
                              BusConnection         *connection,
                              GVariant              *parameters,
                              GDBusMethodInvocation *invocation)
{
    const gchar *name_owner = NULL;
    const gchar *name = NULL;
    g_variant_get (parameters, "(&s)", &name);

    if (g_strcmp0 (name, "org.freedesktop.DBus") == 0 ||
        g_strcmp0 (name, "org.freedesktop.IBus") == 0) {
        name_owner = "org.freedesktop.DBus";
    }
    else {
        BusConnection *owner = bus_dbus_impl_get_connection_by_name (dbus, name);
        if (owner != NULL) {
            name_owner = bus_connection_get_unique_name (owner);
        }
    }

    if (name_owner == NULL) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_NAME_HAS_NO_OWNER,
                        "Can not get name owner of '%s': no such name", name);
    }
    else {
        g_dbus_method_invocation_return_value (invocation,
                        g_variant_new ("(s)", name_owner));
    }
}


/**
 * bus_dbus_impl_get_id:
 *
 * Implement the "GetId" method call of the org.freedesktop.DBus interface.
 * This function is not implemented yet and always returns a dummy string - "FIXME".
 */
static void
bus_dbus_impl_get_id (BusDBusImpl           *dbus,
                      BusConnection         *connection,
                      GVariant              *parameters,
                      GDBusMethodInvocation *invocation)
{
    /* FIXME */
    const gchar *uuid = "FIXME";
    g_dbus_method_invocation_return_value (invocation,
                    g_variant_new ("(s)", uuid));
}

/**
 * bus_dbus_impl_rule_destroy_cb:
 *
 * A function to be called when one of the dbus->rules is destroyed.
 */
static void
bus_dbus_impl_rule_destroy_cb (BusMatchRule *rule,
                               BusDBusImpl  *dbus)
{
    dbus->rules = g_list_remove (dbus->rules, rule);
    g_object_unref (rule);
}

/**
 * bus_dbus_impl_get_id:
 *
 * Implement the "AddMatch" method call of the org.freedesktop.DBus interface.
 */
static void
bus_dbus_impl_add_match (BusDBusImpl           *dbus,
                         BusConnection         *connection,
                         GVariant              *parameters,
                         GDBusMethodInvocation *invocation)
{
    const gchar *rule_text = NULL;
    g_variant_get (parameters, "(&s)", &rule_text);

    BusMatchRule *rule = bus_match_rule_new (rule_text);
    if (rule == NULL) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_MATCH_RULE_INVALID,
                        "Parse match rule [%s] failed", rule_text);
        return;
    }

    g_dbus_method_invocation_return_value (invocation, NULL);
    GList *p;
    for (p = dbus->rules; p != NULL; p = p->next) {
        if (bus_match_rule_is_equal (rule, (BusMatchRule *) p->data)) {
            /* The same rule is already registered. Just reuse it. */
            bus_match_rule_add_recipient ((BusMatchRule *) p->data, connection);
            g_object_unref (rule);
            return;
        }
    }

    if (rule) {
        bus_match_rule_add_recipient (rule, connection);
        dbus->rules = g_list_append (dbus->rules, rule);
        g_signal_connect (rule, "destroy", G_CALLBACK (bus_dbus_impl_rule_destroy_cb), dbus);
    }
}

/**
 * bus_dbus_impl_get_id:
 *
 * Implement the "RemoveMatch" method call of the org.freedesktop.DBus interface.
 */
static void
bus_dbus_impl_remove_match (BusDBusImpl           *dbus,
                            BusConnection         *connection,
                            GVariant              *parameters,
                            GDBusMethodInvocation *invocation)
{
    const gchar *rule_text = NULL;
    g_variant_get (parameters, "(&s)", &rule_text);

    BusMatchRule *rule = bus_match_rule_new (rule_text);
    if (rule == NULL) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_MATCH_RULE_INVALID,
                        "Parse match rule [%s] failed", rule_text);
        return;
    }

    g_dbus_method_invocation_return_value (invocation, NULL);
    GList *p;
    for (p = dbus->rules; p != NULL; p = p->next) {
        if (bus_match_rule_is_equal (rule, (BusMatchRule *) p->data)) {
            /* p->data will be destroyed when the final recipient is removed.  */
            bus_match_rule_remove_recipient ((BusMatchRule *) p->data, connection);
            break;
        }
        /* FIXME should we return G_DBUS_ERROR if rule is not found in dbus->rules */
    }
    g_object_unref (rule);
}

/**
 * bus_dbus_impl_request_name:
 *
 * Implement the "RequestName" method call of the org.freedesktop.DBus interface.
 */
static void
bus_dbus_impl_request_name (BusDBusImpl           *dbus,
                            BusConnection         *connection,
                            GVariant              *parameters,
                            GDBusMethodInvocation *invocation)
{
    /* FIXME need to handle flags defined in:
     * http://dbus.freedesktop.org/doc/dbus-specification.html#message-bus-names */
    const gchar *name = NULL;  // e.g. "org.freedesktop.IBus.Panel"
    guint flags = 0;
    g_variant_get (parameters, "(&su)", &name, &flags);

    if (name == NULL ||
        !g_dbus_is_name (name) ||
        g_dbus_is_unique_name (name)) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                        "'%s' is not a legal service name.", name);
        return;
    }

    if (g_strcmp0 (name, "org.freedesktop.DBus") == 0 ||
        g_strcmp0 (name, "org.freedesktop.IBus") == 0) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                        "Can not acquire the service name '%s', it is reserved by IBus", name);
        return;
    }

    if (g_hash_table_lookup (dbus->names, name) != NULL) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "Service name '%s' already has an owner.", name);
        return;
    }

    const guint retval = 1; /* DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER */
    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(u)", retval));
    g_hash_table_insert (dbus->names,
                         (gpointer) bus_connection_add_name (connection, name),
                         connection);

    g_signal_emit (dbus,
                   dbus_signals[NAME_OWNER_CHANGED],
                   0,
                   name,
                   "",
                   bus_connection_get_unique_name (connection));
}

/**
 * bus_dbus_impl_release_name:
 *
 * Implement the "ReleaseName" method call of the org.freedesktop.DBus interface.
 */
static void
bus_dbus_impl_release_name (BusDBusImpl           *dbus,
                            BusConnection         *connection,
                            GVariant              *parameters,
                            GDBusMethodInvocation *invocation)
{
    const gchar *name= NULL;
    g_variant_get (parameters, "(&s)", &name);

    if (name == NULL ||
        !g_dbus_is_name (name) ||
        g_dbus_is_unique_name (name)) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                        "'%s' is not a legal service name.", name);
        return;
    }

    if (g_strcmp0 (name, "org.freedesktop.DBus") == 0 ||
        g_strcmp0 (name, "org.freedesktop.IBus") == 0) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                        "Service name '%s' is owned by IBus.", name);
        return;
    }

    guint retval;
    if (g_hash_table_lookup (dbus->names, name) == NULL) {
        retval = 2; /* DBUS_RELEASE_NAME_REPLY_NON_EXISTENT */
    }
    else {
        if (bus_connection_remove_name (connection, name)) {
            retval = 1; /* DBUS_RELEASE_NAME_REPLY_RELEASED */
        }
        else {
            retval = 3; /* DBUS_RELEASE_NAME_REPLY_NOT_OWNER */
        }
    }
    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(u)", retval));
}

/**
 * bus_dbus_impl_name_owner_changed:
 *
 * The function is called on name-owner-changed signal, typically when g_signal_emit (dbus, NAME_OWNER_CHANGED)
 * is called, and broadcasts the signal to clients.
 */
static void
bus_dbus_impl_name_owner_changed (BusDBusImpl   *dbus,
                                  gchar         *name,
                                  gchar         *old_owner,
                                  gchar         *new_owner)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (name != NULL);
    g_assert (old_owner != NULL);
    g_assert (new_owner != NULL);

    static guint32 serial = 0;

    GDBusMessage *message = g_dbus_message_new_signal ("/org/freedesktop/DBus",
                                                       "org.freedesktop.DBus",
                                                       "NameOwnerChanged");
    g_dbus_message_set_sender (message, "org.freedesktop.DBus");

    /* set a non-zero serial to make libdbus happy */
    g_dbus_message_set_serial (message, ++serial);
    g_dbus_message_set_body (message,
                             g_variant_new ("(sss)", name, old_owner, new_owner));

    /* broadcast the message to clients that listen to the signal. */
    bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
    g_object_unref (message);
}

/**
 * bus_dbus_impl_service_method_call:
 *
 * Handle a D-Bus method call from a client. This function overrides an implementation in src/ibusservice.c.
 */
static void
bus_dbus_impl_service_method_call (IBusService           *service,
                                   GDBusConnection       *dbus_connection,
                                   const gchar           *sender,
                                   const gchar           *object_path,
                                   const gchar           *interface_name,
                                   const gchar           *method_name,
                                   GVariant              *parameters,
                                   GDBusMethodInvocation *invocation)
{
    BusDBusImpl *dbus = BUS_DBUS_IMPL (service);

    if (g_strcmp0 (interface_name, "org.freedesktop.DBus") != 0) {
        IBUS_SERVICE_CLASS (bus_dbus_impl_parent_class)->service_method_call (
                                        (IBusService *) dbus,
                                        dbus_connection,
                                        sender,
                                        object_path,
                                        interface_name,
                                        method_name,
                                        parameters,
                                        invocation);
        return;
    }

    static const struct {
        const gchar *method_name;
        void (* method) (BusDBusImpl *, BusConnection *, GVariant *, GDBusMethodInvocation *);
    } methods[] =  {
        /* DBus interface */
        { "Hello",          bus_dbus_impl_hello },
        { "ListNames",      bus_dbus_impl_list_names },
        { "NameHasOwner",   bus_dbus_impl_name_has_owner },
        { "GetNameOwner",   bus_dbus_impl_get_name_owner },
        { "GetId",          bus_dbus_impl_get_id },
        { "AddMatch",       bus_dbus_impl_add_match },
        { "RemoveMatch",    bus_dbus_impl_remove_match },
        { "RequestName",    bus_dbus_impl_request_name },
        { "ReleaseName",    bus_dbus_impl_release_name },
    };

    gint i;
    for (i = 0; i < G_N_ELEMENTS (methods); i++) {
        if (g_strcmp0 (method_name, methods[i].method_name) == 0) {
            BusConnection *connection = bus_connection_lookup (dbus_connection);
            g_assert (BUS_IS_CONNECTION (connection));
            methods[i].method (dbus, connection, parameters, invocation);
            return;
        }
    }

    /* unsupported methods */
    g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
                                           "org.freedesktop.DBus does not support %s", method_name);
}

/**
 * bus_dbus_impl_service_get_property:
 *
 * Handle a D-Bus method call from a client. This function overrides an implementation in src/ibusservice.c.
 */
static GVariant *
bus_dbus_impl_service_get_property (IBusService        *service,
                                    GDBusConnection    *connection,
                                    const gchar        *sender,
                                    const gchar        *object_path,
                                    const gchar        *interface_name,
                                    const gchar        *property_name,
                                    GError            **error)
{
    /* FIXME implement the function. */
    return IBUS_SERVICE_CLASS (bus_dbus_impl_parent_class)->
                service_get_property (service,
                                      connection,
                                      sender,
                                      object_path,
                                      interface_name,
                                      property_name,
                                      error);
}

/**
 * bus_dbus_impl_service_set_property:
 *
 * Handle a D-Bus method call from a client. This function overrides an implementation in src/ibusservice.c.
 */
static gboolean
bus_dbus_impl_service_set_property (IBusService        *service,
                                    GDBusConnection    *connection,
                                    const gchar        *sender,
                                    const gchar        *object_path,
                                    const gchar        *interface_name,
                                    const gchar        *property_name,
                                    GVariant           *value,
                                    GError            **error)
{
    /* FIXME implement the function. */
    return IBUS_SERVICE_CLASS (bus_dbus_impl_parent_class)->
                service_set_property (service,
                                      connection,
                                      sender,
                                      object_path,
                                      interface_name,
                                      property_name,
                                      value,
                                      error);

}

/**
 * bus_dbus_impl_connection_filter_cb:
 * @returns: A GDBusMessage that will be processed by bus_dbus_impl_service_method_call. NULL when dropping the message.
 *
 * A filter function that is called for all incoming and outgoing messages.
 * WARNING - this function could be called by the GDBus's worker thread. So you should not call thread unsafe IBus functions.
 */
static GDBusMessage *
bus_dbus_impl_connection_filter_cb (GDBusConnection *dbus_connection,
                                    GDBusMessage    *message,
                                    gboolean         incoming,
                                    gpointer         user_data)
{
    g_assert (G_IS_DBUS_CONNECTION (dbus_connection));
    g_assert (G_IS_DBUS_MESSAGE (message));
    g_assert (BUS_IS_DBUS_IMPL (user_data));

    BusDBusImpl *dbus = (BusDBusImpl *) user_data;
    BusConnection *connection = bus_connection_lookup (dbus_connection);
    g_assert (connection != NULL);

    if (incoming) {
        /* is incoming message */

        /* get the destination aka bus name of the message. the destination is set by g_dbus_connection_call_sync (for DBus and IBus messages
         * in the IBusBus class) or g_initable_new (for config and context messages in the IBusProxy sub classes.) */
        const gchar *destination = g_dbus_message_get_destination (message);
        GDBusMessageType message_type = g_dbus_message_get_message_type (message);

        if (g_dbus_message_get_locked (message)) {
            /* If the message is locked, we need make a copy of it. */
            GDBusMessage *new_message = g_dbus_message_copy (message, NULL);
            g_assert (new_message != NULL);
            g_object_unref (message);
            message = new_message;
        }

        /* connection unique name as sender of the message*/
        g_dbus_message_set_sender (message, bus_connection_get_unique_name (connection));

        if (g_strcmp0 (destination, "org.freedesktop.IBus") == 0) {
            /* the message is sent to IBus service. messages from ibusbus and ibuscontext may fall into this category. */
            switch (message_type) {
            case G_DBUS_MESSAGE_TYPE_METHOD_CALL:
            case G_DBUS_MESSAGE_TYPE_METHOD_RETURN:
            case G_DBUS_MESSAGE_TYPE_ERROR:
                /* dispatch messages by match rule */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
                return message;
            case G_DBUS_MESSAGE_TYPE_SIGNAL:
            default:
                /* notreached - signals should not be sent to IBus service. dispatch signal messages by match rule, just in case. */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
                g_object_unref (message);
                g_return_val_if_reached (NULL);  /* return NULL since the service does not handle signals. */
            }
        }
        else if (g_strcmp0 (destination, "org.freedesktop.DBus") == 0) {
            /* the message is sent to DBus service. messages from ibusbus may fall into this category. */
            switch (message_type) {
            case G_DBUS_MESSAGE_TYPE_METHOD_CALL:
            case G_DBUS_MESSAGE_TYPE_METHOD_RETURN:
            case G_DBUS_MESSAGE_TYPE_ERROR:
                /* dispatch messages by match rule */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
                return message;
            case G_DBUS_MESSAGE_TYPE_SIGNAL:
            default:
                /* notreached - signals should not be sent to IBus service. dispatch signal messages by match rule, just in case. */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
                g_object_unref (message);
                g_return_val_if_reached (NULL);  /* return NULL since the service does not handle signals. */
            }
        }
        else if (destination == NULL) {
            /* the message is sent to the current connection. communications between ibus-daemon and panel/engines may fall into this
             * category since the panel/engine proxies created by ibus-daemon does not set bus name. */
            switch (message_type) {
            case G_DBUS_MESSAGE_TYPE_SIGNAL:
                /* dispatch signal messages by match rule */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
                return message;
            case G_DBUS_MESSAGE_TYPE_METHOD_RETURN:
            case G_DBUS_MESSAGE_TYPE_ERROR:
                return message;
            case G_DBUS_MESSAGE_TYPE_METHOD_CALL:
            default:
                /* notreached. dispatch messages by match rule just in case. */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
                g_object_unref (message);
                g_return_val_if_reached (NULL);  /* return NULL since the service does not handle the message. */
            }
        }
        else {
            /* The message is sent to an other service. Forward it.
             * For example, the config proxy class in src/ibusconfig.c sets its "g-name" property (i.e. destination) to IBUS_SERVICE_CONFIG. */
            bus_dbus_impl_forward_message (dbus, connection, message);
            g_object_unref (message);
            return NULL;
        }
    }
    else {
        /* is outgoing message */
        if (g_dbus_message_get_sender (message) == NULL) {
            if (g_dbus_message_get_locked (message)) {
                GDBusMessage *new_message = g_dbus_message_copy (message, NULL);
                g_assert (new_message != NULL);
                g_object_unref (message);
                message = new_message;
            }
            /* If the message is sending from ibus-daemon directly,
             * we set the sender to org.freedesktop.DBus */
            g_dbus_message_set_sender (message, "org.freedesktop.DBus");
        }

        /* dispatch the outgoing message by rules. */
        bus_dbus_impl_dispatch_message_by_rule (dbus, message, connection);
        return message;
    }
}

BusDBusImpl *
bus_dbus_impl_get_default (void)
{
    static BusDBusImpl *dbus = NULL;

    if (dbus == NULL) {
        dbus = (BusDBusImpl *) g_object_new (BUS_TYPE_DBUS_IMPL,
                                             "object-path", "/org/freedesktop/DBus",
                                             NULL);
    }

    return dbus;
}

static void
bus_dbus_impl_connection_destroy_cb (BusConnection *connection,
                                     BusDBusImpl   *dbus)
{
    const gchar *unique_name = bus_connection_get_unique_name (connection);
    if (unique_name != NULL) {
        g_hash_table_remove (dbus->unique_names, unique_name);
        g_signal_emit (dbus,
                       dbus_signals[NAME_OWNER_CHANGED],
                       0,
                       unique_name,
                       unique_name,
                       "");
    }

    const GList *name = bus_connection_get_names (connection);
    while (name != NULL) {
        g_hash_table_remove (dbus->names, name->data);
        g_signal_emit (dbus,
                       dbus_signals[NAME_OWNER_CHANGED],
                       0,
                       name->data,
                       unique_name,
                       "");
        name = name->next;
    }

    dbus->connections = g_list_remove (dbus->connections, connection);
    g_object_unref (connection);
}


gboolean
bus_dbus_impl_new_connection (BusDBusImpl   *dbus,
                              BusConnection *connection)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (g_list_find (dbus->connections, connection) == NULL);

    g_object_ref_sink (connection);
    dbus->connections = g_list_append (dbus->connections, connection);

    bus_connection_set_filter (connection,
                    bus_dbus_impl_connection_filter_cb, g_object_ref (dbus), g_object_unref);

    g_signal_connect (connection,
                      "destroy",
                      G_CALLBACK (bus_dbus_impl_connection_destroy_cb),
                      dbus);

    /* add introspection_xml[] (see above) to the connection. */
    ibus_service_register ((IBusService *) dbus,
                    bus_connection_get_dbus_connection (connection), NULL);
    GList *p;
    for (p = dbus->objects; p != NULL; p = p->next) {
        /* add all introspection xmls in dbus->objects to the connection. */
        ibus_service_register ((IBusService *) p->data,
                        bus_connection_get_dbus_connection (connection), NULL);
    }
    return TRUE;
}

BusConnection *
bus_dbus_impl_get_connection_by_name (BusDBusImpl    *dbus,
                                      const gchar    *name)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (name != NULL);

    if (G_LIKELY (g_dbus_is_unique_name (name))) {
        return (BusConnection *) g_hash_table_lookup (dbus->unique_names, name);
    }
    else {
        return (BusConnection *) g_hash_table_lookup (dbus->names, name);
    }
}

/**
 * bus_dbus_impl_forward_message_ible_cb:
 *
 * Process the first element of the dbus->forward_queue. The first element is forwarded by g_dbus_connection_send_message.
 */
static gboolean
bus_dbus_impl_forward_message_idle_cb (BusDBusImpl   *dbus)
{
    g_return_val_if_fail (dbus->forward_queue != NULL, FALSE);

    g_mutex_lock (dbus->forward_lock);
    GDBusMessage *message = (GDBusMessage *) dbus->forward_queue->data;
    dbus->forward_queue = g_list_delete_link (dbus->forward_queue, dbus->forward_queue);
    gboolean has_message = (dbus->forward_queue != NULL);
    g_mutex_unlock (dbus->forward_lock);

    const gchar *destination = g_dbus_message_get_destination (message);
    BusConnection *dest_connection = NULL;
    if (destination != NULL)
            dest_connection = bus_dbus_impl_get_connection_by_name (dbus, destination);
    if (dest_connection != NULL) {
        /* FIXME workaround for gdbus. gdbus can not set an empty body message with signature '()' */
        if (g_dbus_message_get_body (message) == NULL)
            g_dbus_message_set_signature (message, NULL);
        GError *error = NULL;
        gboolean retval = g_dbus_connection_send_message (
                                        bus_connection_get_dbus_connection (dest_connection),
                                        message,
                                        G_DBUS_SEND_MESSAGE_FLAGS_PRESERVE_SERIAL,
                                        NULL, &error);
        if (!retval) {
            g_warning ("send error failed:  %s.", error->message);
            g_error_free (error);
        }
    }
    else {
        /* FIXME What should we do, if can not get destination.
         * It should not happen */
#if 0
        if (g_dbus_message_get_message_type (message) == G_DBUS_MESSAGE_TYPE_METHOD_CALL) {
            /* reply an error message, if the destination does not exist */
            GDBusMessage *reply_message = g_dbus_message_new_method_error (message,
                            "org.freedesktop.DBus.Error.ServiceUnknown ",
                            "No service name is '%s'.", destination);
            g_dbus_message_set_sender (reply_message, "org.freedesktop.DBus");
            g_dbus_message_set_destination (reply_message, bus_connection_get_unique_name (connection));
            g_dbus_connection_send_message (bus_connection_get_dbus_connection (connection),
                            reply_message, NULL, NULL);
            g_object_unref (reply_message);
        }
        else {
            /* ignore other messages */
        }
#endif
    }
    g_object_unref (message);
    return has_message;
}

void
bus_dbus_impl_forward_message (BusDBusImpl   *dbus,
                               BusConnection *connection,
                               GDBusMessage  *message)
{
    /* WARNING - this function could be called by the GDBus's worker thread. So you should not call thread unsafe IBus functions. */
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (G_IS_DBUS_MESSAGE (message));

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus)))
        return;
    /* FIXME the check above might not be sufficient. dbus object could be destroyed in the main thread right after the check, though the
     * dbus structure itself would not be freed (since the dbus object is ref'ed in bus_dbus_impl_new_connection.)
     * Anyway, it'd be better to investigate whether the thread safety issue could cause any real problems. */

    g_mutex_lock (dbus->forward_lock);
    gboolean is_running = (dbus->forward_queue != NULL);
    dbus->forward_queue = g_list_append (dbus->forward_queue, g_object_ref (message));
    g_mutex_unlock (dbus->forward_lock);
    if (!is_running) {
        g_idle_add_full (G_PRIORITY_DEFAULT, (GSourceFunc) bus_dbus_impl_forward_message_idle_cb,
                        g_object_ref (dbus), (GDestroyNotify) g_object_unref);
        /* the idle callback function will be called from the ibus's main thread. */
    }
}

static BusDispatchData *
bus_dispatch_data_new (GDBusMessage  *message,
                       BusConnection *skip_connection)
{
    BusDispatchData *data = g_slice_new (BusDispatchData);

    data->message = (GDBusMessage *) g_object_ref (message);
    if (skip_connection) {
        data->skip_connection = (BusConnection *) g_object_ref (skip_connection);
    }
    else {
        data->skip_connection = NULL;
    }
    return data;
}

static void
bus_dispatch_data_free (BusDispatchData *data)
{
    g_object_unref (data->message);
    if (data->skip_connection)
        g_object_unref (data->skip_connection);
    g_slice_free (BusDispatchData, data);
}

/**
 * bus_dbus_impl_dispatch_message_by_rule_idle_cb:
 *
 * Process the first element of the dbus->dispatch_queue.
 */
static gboolean
bus_dbus_impl_dispatch_message_by_rule_idle_cb (BusDBusImpl *dbus)
{
    g_return_val_if_fail (dbus->dispatch_queue != NULL, FALSE);

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus))) {
        /* dbus was destryed */
        g_mutex_lock (dbus->dispatch_lock);
        g_list_foreach (dbus->dispatch_queue, (GFunc) bus_dispatch_data_free, NULL);
        g_list_free (dbus->dispatch_queue);
        dbus->dispatch_queue = NULL;
        g_mutex_unlock (dbus->dispatch_lock);
        return FALSE; /* return FALSE to prevent this callback to be called again. */
    }

    /* remove fist node */
    g_mutex_lock (dbus->dispatch_lock);
    BusDispatchData *data = (BusDispatchData *) dbus->dispatch_queue->data;
    dbus->dispatch_queue = g_list_delete_link (dbus->dispatch_queue, dbus->dispatch_queue);
    gboolean has_message = (dbus->dispatch_queue != NULL);
    g_mutex_unlock (dbus->dispatch_lock);

    GList *link = NULL;
    GList *recipients = NULL;

    /* check each match rules, and get recipients */
    for (link = dbus->rules; link != NULL; link = link->next) {
        GList *list = bus_match_rule_get_recipients ((BusMatchRule *) link->data,
                                                     data->message);
        recipients = g_list_concat (recipients, list);
    }

    /* send message to each recipients */
    for (link = recipients; link != NULL; link = link->next) {
        BusConnection *connection = (BusConnection *) link->data;
        if (G_LIKELY (connection != data->skip_connection)) {
            g_dbus_connection_send_message (bus_connection_get_dbus_connection (connection),
                                            data->message,
                                            G_DBUS_SEND_MESSAGE_FLAGS_PRESERVE_SERIAL,
                                            NULL, NULL);
        }
    }
    g_list_free (recipients);
    bus_dispatch_data_free (data);

    return has_message;  /* remove this idle callback if no message is left by returning FALSE. */
}

void
bus_dbus_impl_dispatch_message_by_rule (BusDBusImpl     *dbus,
                                        GDBusMessage    *message,
                                        BusConnection   *skip_connection)
{
    /* WARNING - this function could be called by the GDBus's worker thread. So you should not call thread unsafe IBus functions. */
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (message != NULL);
    g_assert (skip_connection == NULL || BUS_IS_CONNECTION (skip_connection));

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus)))
        return;
    /* FIXME - see the FIXME comment in bus_dbus_impl_forward_message. */

    static GQuark dispatched_quark = 0;
    if (dispatched_quark == 0) {
        dispatched_quark = g_quark_from_static_string ("DISPATCHED");
    }

    /* A message sent or forwarded by bus_dbus_impl_dispatch_message_by_rule_idle_cb is also processed by the filter callback.
     * If this message has been dispatched by rule, do nothing. */
    if (g_object_get_qdata ((GObject *) message, dispatched_quark) != NULL)
        return;
    g_object_set_qdata ((GObject *) message, dispatched_quark, GINT_TO_POINTER (1));

    /* append dispatch data into the queue, and start idle task if necessary */
    g_mutex_lock (dbus->dispatch_lock);
    gboolean is_running = (dbus->dispatch_queue != NULL);
    dbus->dispatch_queue = g_list_append (dbus->dispatch_queue,
                    bus_dispatch_data_new (message, skip_connection));
    g_mutex_unlock (dbus->dispatch_lock);
    if (!is_running) {
        g_idle_add_full (G_PRIORITY_DEFAULT,
                         (GSourceFunc) bus_dbus_impl_dispatch_message_by_rule_idle_cb,
                         g_object_ref (dbus),
                         (GDestroyNotify) g_object_unref);
        /* the idle callback function will be called from the ibus's main thread. */
    }
}

static void
bus_dbus_impl_object_destroy_cb (IBusService *object,
                                 BusDBusImpl *dbus)
{
    bus_dbus_impl_unregister_object (dbus, object);
}


gboolean
bus_dbus_impl_register_object (BusDBusImpl *dbus,
                               IBusService *object)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (IBUS_IS_SERVICE (object));

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus))) {
        return FALSE;
    }

    dbus->objects = g_list_prepend (dbus->objects, g_object_ref (object));
    g_signal_connect (object, "destroy",
                      G_CALLBACK (bus_dbus_impl_object_destroy_cb), dbus);

    GList *p;
    for (p = dbus->connections; p != NULL; p = p->next) {
        GDBusConnection *connection = bus_connection_get_dbus_connection ((BusConnection *) p->data);
        if (connection != ibus_service_get_connection ((IBusService *) object))
            ibus_service_register ((IBusService *) object,
                            bus_connection_get_dbus_connection ((BusConnection *) p->data), NULL);
    }
    return TRUE;
}

gboolean
bus_dbus_impl_unregister_object (BusDBusImpl *dbus,
                                 IBusService *object)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (IBUS_IS_SERVICE (object));

    GList *p = g_list_find (dbus->objects, object);
    if (p == NULL)
        return FALSE;

    g_signal_handlers_disconnect_by_func (object,
                    G_CALLBACK (bus_dbus_impl_object_destroy_cb), dbus);
    dbus->objects = g_list_delete_link (dbus->objects, p);
    if (!IBUS_OBJECT_DESTROYED (object)) {
        GList *p;
        for (p = dbus->connections; p != NULL; p = p->next) {
            ibus_service_unregister ((IBusService *) object,
                            bus_connection_get_dbus_connection ((BusConnection *) p->data));
        }
    }
    g_object_unref (object);

    return TRUE;
}

