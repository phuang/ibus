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
    GHashTable *unique_names;
    GHashTable *names;
    GList *objects;
    GList *connections;
    GList *rules;
    gint id;

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

    IBUS_SERVICE_CLASS (class)->service_method_call =  bus_dbus_impl_service_method_call;
    IBUS_SERVICE_CLASS (class)->service_get_property = bus_dbus_impl_service_get_property;
    IBUS_SERVICE_CLASS (class)->service_set_property = bus_dbus_impl_service_set_property;

    ibus_service_class_add_interfaces (IBUS_SERVICE_CLASS (class), introspection_xml);

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
}

static void
bus_dbus_impl_destroy (BusDBusImpl *dbus)
{
    GList *p;

    for (p = dbus->objects; p != NULL; p = p->next) {
        IBusService *object = (IBusService *)p->data;
        g_signal_handlers_disconnect_by_func (object, G_CALLBACK (bus_dbus_impl_object_destroy_cb), dbus);
        ibus_object_destroy ((IBusObject *)object);
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

    IBUS_OBJECT_CLASS(bus_dbus_impl_parent_class)->destroy ((IBusObject *)dbus);
}

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
        gchar *name = g_strdup_printf (":1.%d", ++dbus->id);
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

    // append well-known names
    GList *names, *name;
    names = g_hash_table_get_keys (dbus->names);
    names = g_list_sort (names, (GCompareFunc) g_strcmp0);
    for (name = names; name != NULL; name = name->next) {
        g_variant_builder_add (&builder, "s", name->data);
    }
    g_list_free (names);

    // append unique names
    names = g_hash_table_get_keys (dbus->unique_names);
    names = g_list_sort (names, (GCompareFunc) g_strcmp0);
    for (name = names; name != NULL; name = name->next) {
        g_variant_builder_add (&builder, "s", name->data);
    }
    g_list_free (names);

    g_dbus_method_invocation_return_value (invocation,
                    g_variant_new ("(as)", &builder));
}

static void
bus_dbus_impl_name_has_owner (BusDBusImpl           *dbus,
                              BusConnection         *connection,
                              GVariant              *parameters,
                              GDBusMethodInvocation *invocation)
{
    const gchar *name = NULL;
    g_variant_get (parameters, "(&s)", &name);

    gboolean has_owner;
    if (name[0] == ':') {
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

static void
bus_dbus_impl_get_name_owner (BusDBusImpl           *dbus,
                              BusConnection         *connection,
                              GVariant              *parameters,
                              GDBusMethodInvocation *invocation)
{
    const gchar *name_owner = NULL;
    const gchar *name = NULL;
    g_variant_get (parameters, "(&s)", &name);

    if (g_strcmp0 (name, "org.freedesktop.IBus") == 0 ||
        g_strcmp0 (name, "org.freedesktop.DBus") == 0) {
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
                        "Can not get name owner of '%s': no suce name", name);
    }
    else {
        g_dbus_method_invocation_return_value (invocation,
                        g_variant_new ("(s)", name_owner));
    }
}


static void
bus_dbus_impl_get_id (BusDBusImpl           *dbus,
                      BusConnection         *connection,
                      GVariant              *parameters,
                      GDBusMethodInvocation *invocation)
{
    /* FXIME */
    const gchar *uuid = "FXIME";
    g_dbus_method_invocation_return_value (invocation,
                    g_variant_new ("(s)", uuid));
}

static void
bus_dbus_impl_rule_destroy_cb (BusMatchRule *rule,
                           BusDBusImpl  *dbus)
{
    dbus->rules = g_list_remove (dbus->rules, rule);
    g_object_unref (rule);
}

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
        if (bus_match_rule_is_equal (rule, (BusMatchRule *)p->data)) {
            bus_match_rule_add_recipient ((BusMatchRule *)p->data, connection);
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
        if (bus_match_rule_is_equal (rule, (BusMatchRule *)p->data)) {
            bus_match_rule_remove_recipient ((BusMatchRule *)p->data, connection);
            break;
        }
    }
    g_object_unref (rule);
}

static void
bus_dbus_impl_request_name (BusDBusImpl           *dbus,
                            BusConnection         *connection,
                            GVariant              *parameters,
                            GDBusMethodInvocation *invocation)
{
    /* FIXME need to handle flags */
    const gchar *name = NULL;
    guint flags = 0;
    g_variant_get (parameters, "(&su)", &name, &flags);

    if (name[0] == ':' || !g_dbus_is_name (name)) {
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

    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(u)", 1));
    g_hash_table_insert (dbus->names,
                         (gpointer )bus_connection_add_name (connection, name),
                         connection);

    g_signal_emit (dbus,
                   dbus_signals[NAME_OWNER_CHANGED],
                   0,
                   name,
                   "",
                   bus_connection_get_unique_name (connection));
}

static void
bus_dbus_impl_release_name (BusDBusImpl           *dbus,
                            BusConnection         *connection,
                            GVariant              *parameters,
                            GDBusMethodInvocation *invocation)
{
    const gchar *name= NULL;
    g_variant_get (parameters, "(&s)", &name);

    if (name[0] == ':' || !g_dbus_is_name (name)) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                        "'%s' is not a legal service name.", name);
        return;
    }

    if (g_strcmp0 (name, "org.freedesktop.DBus") == 0 ||
        g_strcmp0 (name, "org.freedesktop.IBus") == 0) {
        g_dbus_method_invocation_return_error (invocation,
                        G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                        "Service name '%s' is owned by bus.", name);
        return;
    }

    guint retval;
    if (g_hash_table_lookup (dbus->names, name) == NULL) {
        retval = 2;
    }
    else {
        if (bus_connection_remove_name (connection, name)) {
            retval = 1;
        }
        else {
            retval = 3;
        }
    }
    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(u)", retval));
}

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
    bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
    g_object_unref (message);
}

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

    /* unsupport methods */
    g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
                    "org.freedesktop.DBus does not support %s", method_name);
}

static GVariant *
bus_dbus_impl_service_get_property (IBusService        *service,
                                    GDBusConnection    *connection,
                                    const gchar        *sender,
                                    const gchar        *object_path,
                                    const gchar        *interface_name,
                                    const gchar        *property_name,
                                    GError            **error)
{
    return IBUS_SERVICE_CLASS (bus_dbus_impl_parent_class)->
                service_get_property (service,
                                      connection,
                                      sender,
                                      object_path,
                                      interface_name,
                                      property_name,
                                      error);
}

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

#if 1
static void
message_print(GDBusMessage *message)
{
    switch (g_dbus_message_get_message_type (message)) {
    case G_DBUS_MESSAGE_TYPE_METHOD_CALL:
        g_debug ("From %s to %s, CALL(%u) %s.%s (%s)",
            g_dbus_message_get_sender (message),
            g_dbus_message_get_destination (message),
            g_dbus_message_get_serial (message),
            g_dbus_message_get_interface (message),
            g_dbus_message_get_member (message),
            g_dbus_message_get_signature (message)
            );
        break;
    case G_DBUS_MESSAGE_TYPE_METHOD_RETURN:
        g_debug ("From %s to %s, RETURN(%u) (%s)",
            g_dbus_message_get_sender (message),
            g_dbus_message_get_destination (message),
            g_dbus_message_get_reply_serial (message),
            g_dbus_message_get_signature (message)
            );
        break;
    case G_DBUS_MESSAGE_TYPE_ERROR:
        g_debug ("From %s to %s, ERROR(%u) %s",
            g_dbus_message_get_sender (message),
            g_dbus_message_get_destination (message),
            g_dbus_message_get_reply_serial (message),
            g_dbus_message_get_error_name (message)
            );
        break;
    case G_DBUS_MESSAGE_TYPE_SIGNAL:
        g_debug ("From %s to %s, SIGNAL %s.%s (%s) @ %s",
            g_dbus_message_get_sender (message),
            g_dbus_message_get_destination (message),
            g_dbus_message_get_interface (message),
            g_dbus_message_get_member (message),
            g_dbus_message_get_signature (message),
            g_dbus_message_get_path (message)
            );
        break;
    default:
        break;
    }

}
#endif

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
            /* the message is sended to IBus service. */
            switch (message_type) {
            case G_DBUS_MESSAGE_TYPE_METHOD_CALL:
            case G_DBUS_MESSAGE_TYPE_METHOD_RETURN:
            case G_DBUS_MESSAGE_TYPE_ERROR:
                /* dispatch signal messages by match rule */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
                return message;
            case G_DBUS_MESSAGE_TYPE_SIGNAL:
            default:
                /* dispatch signal messages by match rule */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
                message_print (message);
                g_object_unref (message);
                g_return_val_if_reached (NULL);
            }
        }
        else if (g_strcmp0 (destination, "org.freedesktop.DBus") == 0) {
            /* The message is sended to DBus service. */
            switch (message_type) {
            case G_DBUS_MESSAGE_TYPE_METHOD_CALL:
            case G_DBUS_MESSAGE_TYPE_METHOD_RETURN:
            case G_DBUS_MESSAGE_TYPE_ERROR:
                /* dispatch signal messages by match rule */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
                return message;
            case G_DBUS_MESSAGE_TYPE_SIGNAL:
            default:
                /* dispatch signal messages by match rule */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
                message_print (message);
                g_object_unref (message);
                g_return_val_if_reached (NULL);
            }
        }
        else if (destination == NULL) {
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
                /* dispatch signal messages by match rule */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
                message_print (message);
                g_object_unref (message);
                g_return_val_if_reached (NULL);
            }
        }
        else {
            /* forward message */
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
    ibus_service_register ((IBusService *)dbus,
                    bus_connection_get_dbus_connection (connection), NULL);
    GList *p;
    for (p = dbus->objects; p != NULL; p = p->next) {
        ibus_service_register ((IBusService *)p->data,
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

    if (G_LIKELY (name[0] == ':')) {
        return (BusConnection *)g_hash_table_lookup (dbus->unique_names, name);
    }
    else {
        return (BusConnection *)g_hash_table_lookup (dbus->names, name);
    }
}

static gboolean
bus_dbus_impl_forward_message_idle_cb (BusDBusImpl   *dbus)
{
    g_return_val_if_fail (dbus->forward_queue != NULL, FALSE);

    g_mutex_lock (dbus->forward_lock);
    GDBusMessage *message = (GDBusMessage *)dbus->forward_queue->data;
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
            // message_print (message);
            g_error_free (error);
        }
    }
    else {
        /* FIXME can not get destination */
#if 0
        if (g_dbus_message_get_message_type (message) == G_DBUS_MESSAGE_TYPE_METHOD_CALL) {
            /* reply an error message, if the destination does not exist */
            GDBusMessage *reply_message = g_dbus_message_new_method_error (message,
                            "org.freedesktop.DBus.Error.ServiceUnknown",
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
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (G_IS_DBUS_MESSAGE (message));

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus)))
        return;

    g_mutex_lock (dbus->forward_lock);
    gboolean is_running = (dbus->forward_queue != NULL);
    dbus->forward_queue = g_list_append (dbus->forward_queue, g_object_ref (message));
    g_mutex_unlock (dbus->forward_lock);
    if (!is_running) {
        g_idle_add_full (0, (GSourceFunc) bus_dbus_impl_forward_message_idle_cb,
                        g_object_ref (dbus), (GDestroyNotify) g_object_unref);
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

static gboolean
bus_dbus_impl_dispatch_message_by_rule_idle_cb (BusDBusImpl *dbus)
{
    g_return_val_if_fail (dbus->dispatch_queue != NULL, FALSE);

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus))) {
        /* dbus was destryed */
        g_mutex_lock (dbus->dispatch_lock);
        g_list_foreach (dbus->dispatch_queue, (GFunc)bus_dispatch_data_free, NULL);
        g_list_free (dbus->dispatch_queue);
        dbus->dispatch_queue = NULL;
        g_mutex_unlock (dbus->dispatch_lock);
        return FALSE;
    }

    /* remove fist node */
    g_mutex_lock (dbus->dispatch_lock);
    BusDispatchData *data = (BusDispatchData *)dbus->dispatch_queue->data;
    dbus->dispatch_queue = g_list_delete_link (dbus->dispatch_queue, dbus->dispatch_queue);
    gboolean has_message = (dbus->dispatch_queue != NULL);
    g_mutex_unlock (dbus->dispatch_lock);

    GList *link = NULL;
    GList *recipients = NULL;

    /* check each match rules, and get recipients */
    for (link = dbus->rules; link != NULL; link = link->next) {
        GList *list = bus_match_rule_get_recipients ((BusMatchRule *)link->data,
                                                     data->message);
        recipients = g_list_concat (recipients, list);
    }

    /* send message to each recipients */
    for (link = recipients; link != NULL; link = link->next) {
        BusConnection *connection = (BusConnection *)link->data;
        if (G_LIKELY (connection != data->skip_connection)) {
            g_dbus_connection_send_message (bus_connection_get_dbus_connection (connection),
                                            data->message,
                                            G_DBUS_SEND_MESSAGE_FLAGS_PRESERVE_SERIAL,
                                            NULL, NULL);
        }
    }
    g_list_free (recipients);
    bus_dispatch_data_free (data);

    return has_message;
}

void
bus_dbus_impl_dispatch_message_by_rule (BusDBusImpl     *dbus,
                                        GDBusMessage    *message,
                                        BusConnection   *skip_connection)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (message != NULL);
    g_assert (skip_connection == NULL || BUS_IS_CONNECTION (skip_connection));

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus))) {
        return;
    }

    static GQuark dispatched_quark = 0;
    if (dispatched_quark == 0) {
        dispatched_quark = g_quark_from_static_string ("DISPATCHED");
    }

    /* If this message has been dispatched by rule. */
    if (g_object_get_qdata ((GObject *)message, dispatched_quark) != NULL)
        return;
    g_object_set_qdata ((GObject *)message, dispatched_quark, GINT_TO_POINTER (1));

    /* append dispatch data into the queue, and start idle task if necessary */
    g_mutex_lock (dbus->dispatch_lock);
    gboolean is_running = (dbus->dispatch_queue != NULL);
    dbus->dispatch_queue = g_list_append (dbus->dispatch_queue,
                    bus_dispatch_data_new (message, skip_connection));
    g_mutex_unlock (dbus->dispatch_lock);
    if (!is_running) {
        g_idle_add_full (0,
                         (GSourceFunc)bus_dbus_impl_dispatch_message_by_rule_idle_cb,
                         g_object_ref (dbus),
                         (GDestroyNotify)g_object_unref);
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
        GDBusConnection *connection = bus_connection_get_dbus_connection ((BusConnection *)p->data);
        if (connection != ibus_service_get_connection ((IBusService *)object))
            ibus_service_register ((IBusService *)object,
                            bus_connection_get_dbus_connection ((BusConnection *)p->data), NULL);
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
            ibus_service_unregister ((IBusService *)object,
                            bus_connection_get_dbus_connection ((BusConnection *)p->data));
        }
    }
    g_object_unref (object);

    return TRUE;
}

