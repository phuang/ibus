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

#include <string.h>
#include <dbus/dbus.h>
#include <ibusinternal.h>
#include <ibusmarshalers.h>
#include "dbusimpl.h"
#include "connection.h"
#include "matchrule.h"

enum {
    NAME_ACQUIRED,
    NAME_LOST,
    NAME_OWNER_CHANGED,
    LAST_SIGNAL,
};

enum {
    PROP_0,
};

static guint dbus_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_dbus_impl_destroy           (BusDBusImpl        *dbus);
static gboolean bus_dbus_impl_ibus_message      (BusDBusImpl        *dbus,
                                                 BusConnection      *connection,
                                                 IBusMessage        *message);
static void     bus_dbus_impl_name_owner_changed(BusDBusImpl        *dbus,
                                                 gchar              *name,
                                                 gchar              *old_name,
                                                 gchar              *new_name);


static void     _connection_destroy_cb          (BusConnection      *connection,
                                                 BusDBusImpl        *dbus);
static void     _rule_destroy_cb                (BusMatchRule       *rule,
                                                 BusDBusImpl        *dbus);

G_DEFINE_TYPE(BusDBusImpl, bus_dbus_impl, IBUS_TYPE_SERVICE)

BusDBusImpl *
bus_dbus_impl_get_default (void)
{
    static BusDBusImpl *dbus = NULL;

    if (dbus == NULL) {
        dbus = (BusDBusImpl *) g_object_new (BUS_TYPE_DBUS_IMPL,
                                             "path", DBUS_PATH_DBUS,
                                             NULL);
    }

    return dbus;
}

static void
bus_dbus_impl_class_init (BusDBusImplClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    IBusServiceClass *service_class = IBUS_SERVICE_CLASS (klass);

    IBUS_OBJECT_CLASS (gobject_class)->destroy = (IBusObjectDestroyFunc) bus_dbus_impl_destroy;

    service_class->ibus_message = (ServiceIBusMessageFunc) bus_dbus_impl_ibus_message;

    klass->name_owner_changed = bus_dbus_impl_name_owner_changed;

    /* install signals */
    dbus_signals[NAME_OWNER_CHANGED] =
        g_signal_new (I_("name-owner-changed"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST,
            G_STRUCT_OFFSET (BusDBusImplClass, name_owner_changed),
            NULL, NULL,
            ibus_marshal_VOID__STRING_STRING_STRING,
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
    dbus->objects = g_hash_table_new (g_str_hash, g_str_equal);
    dbus->connections = NULL;
    dbus->rules = NULL;
    dbus->id = 1;

    g_object_ref (dbus);
    g_hash_table_insert (dbus->objects, DBUS_PATH_DBUS, dbus);
}

static void
bus_dbus_impl_destroy (BusDBusImpl *dbus)
{
    GList *p;

    for (p = dbus->rules; p != NULL; p = p->next) {
        BusMatchRule *rule = BUS_MATCH_RULE (p->data);
        g_signal_handlers_disconnect_by_func (rule, _rule_destroy_cb, dbus);
        ibus_object_destroy ((IBusObject *) rule);
        g_object_unref (rule);
    }
    g_list_free (dbus->rules);
    dbus->rules = NULL;

    for (p = dbus->connections; p != NULL; p = p->next) {
        BusConnection *connection = BUS_CONNECTION (p->data);
        g_signal_handlers_disconnect_by_func (connection, _connection_destroy_cb, dbus);
        ibus_connection_close ((IBusConnection *) connection);
        ibus_object_destroy ((IBusObject *) connection);
        g_object_unref (connection);
    }
    g_list_free (dbus->connections);
    dbus->connections = NULL;

    g_hash_table_remove_all (dbus->unique_names);
    g_hash_table_remove_all (dbus->names);
    g_hash_table_remove_all (dbus->objects);

    dbus->unique_names = NULL;
    dbus->names = NULL;
    dbus->objects = NULL;

    G_OBJECT_CLASS(bus_dbus_impl_parent_class)->dispose (G_OBJECT (dbus));
}


/* introspectable interface */
static IBusMessage *
_dbus_introspect (BusDBusImpl     *dbus,
                  IBusMessage     *message,
                  BusConnection   *connection)
{
    static const gchar *introspect =
        DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE
        "<node>\n"
        "  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
        "    <method name=\"Introspect\">\n"
        "      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "  </interface>\n"
        "  <interface name=\"org.freedesktop.DBus\">\n"
        "    <method name=\"Hello\">\n"
        "      <arg direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "    <method name=\"RequestName\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"in\" type=\"u\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <method name=\"ReleaseName\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
#if 0
        "    <method name=\"StartServiceByName\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"in\" type=\"u\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <method name=\"UpdateActivationEnvironment\">\n"
        "      <arg direction=\"in\" type=\"a{ss}\"/>\n"
        "    </method>\n"
#endif
        "    <method name=\"NameHasOwner\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"b\"/>\n"
        "    </method>\n"
        "    <method name=\"ListNames\">\n"
        "      <arg direction=\"out\" type=\"as\"/>\n"
        "    </method>\n"
#if 0
        "    <method name=\"ListActivatableNames\">\n"
        "      <arg direction=\"out\" type=\"as\"/>\n"
        "    </method>\n"
#endif
        "    <method name=\"AddMatch\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "    </method>\n"
        "    <method name=\"RemoveMatch\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "    </method>\n"
        "    <method name=\"GetNameOwner\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
#if 0
        "    <method name=\"ListQueuedOwners\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"as\"/>\n"
        "    </method>\n"
        "    <method name=\"GetConnectionUnixUser\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <method name=\"GetConnectionUnixProcessID\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <method name=\"GetAdtAuditSessionData\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"ay\"/>\n"
        "    </method>\n"
        "    <method name=\"GetConnectionSELinuxSecurityContext\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"ay\"/>\n"
        "    </method>\n"
        "    <method name=\"ReloadConfig\">\n"
        "    </method>\n"
#endif
        "    <method name=\"GetId\">\n"
        "      <arg direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "    <signal name=\"NameOwnerChanged\">\n"
        "      <arg type=\"s\"/>\n"
        "      <arg type=\"s\"/>\n"
        "      <arg type=\"s\"/>\n"
        "    </signal>\n"
#if 0
        "    <signal name=\"NameLost\">\n"
        "      <arg type=\"s\"/>\n"
        "    </signal>\n"
        "    <signal name=\"NameAcquired\">\n"
        "      <arg type=\"s\"/>\n"
        "    </signal>\n"
#endif
        "  </interface>\n"
        "</node>\n";

    IBusMessage *reply_message;
    reply_message = ibus_message_new_method_return (message);
    ibus_message_append_args (reply_message,
                              G_TYPE_STRING, &introspect,
                              G_TYPE_INVALID);

    return reply_message;
}


/* dbus interface */
static IBusMessage *
_dbus_no_implement (BusDBusImpl     *dbus,
                    IBusMessage     *message,
                    BusConnection   *connection)
{
    IBusMessage *reply_message;
    reply_message = ibus_message_new_error_printf (message,
                                                   DBUS_ERROR_UNKNOWN_METHOD,
                                                   "IBus does not support %s.",
                                                   ibus_message_get_member (message));
    return reply_message;
}


static IBusMessage *
_dbus_hello (BusDBusImpl    *dbus,
             IBusMessage    *message,
             BusConnection  *connection)
{
    IBusMessage *reply_message;

    if (bus_connection_get_unique_name (connection) != NULL) {
        reply_message = ibus_message_new_error (message,
                                                DBUS_ERROR_FAILED,
                                                "Already handled an Hello message");
    }
    else {
        gchar *name;

        name = g_strdup_printf (":1.%d", dbus->id ++);
        bus_connection_set_unique_name (connection, name);
        g_free (name);

        name = (gchar *) bus_connection_get_unique_name (connection);
        g_hash_table_insert (dbus->unique_names, name, connection);

        reply_message = ibus_message_new_method_return (message);
        ibus_message_append_args (reply_message,
                                  G_TYPE_STRING, &name,
                                  G_TYPE_INVALID);

        ibus_connection_send ((IBusConnection *) connection, reply_message);
        ibus_message_unref (reply_message);
        ibus_connection_flush ((IBusConnection *) connection);
        reply_message = NULL;

        g_signal_emit (dbus,
                       dbus_signals[NAME_OWNER_CHANGED],
                       0,
                       name,
                       "",
                       name);

    }

    return reply_message;
}

static IBusMessage *
_dbus_list_names (BusDBusImpl       *dbus,
                  IBusMessage       *message,
                  BusConnection     *connection)
{
    IBusMessage *reply_message;
    IBusMessageIter iter, sub_iter;
    GList *name, *names;
    gchar *v;

    reply_message = ibus_message_new_method_return (message);

    ibus_message_iter_init_append (reply_message, &iter);
    ibus_message_iter_open_container (&iter, IBUS_TYPE_ARRAY, "s", &sub_iter);

    v = DBUS_SERVICE_DBUS;
    ibus_message_iter_append (&sub_iter, G_TYPE_STRING, &v);

    v = IBUS_SERVICE_IBUS;
    ibus_message_iter_append (&sub_iter, G_TYPE_STRING, &v);

    // append well-known names
    names = g_hash_table_get_keys (dbus->names);
    names = g_list_sort (names, (GCompareFunc) g_strcmp0);
    for (name = names; name != NULL; name = name->next) {
        ibus_message_iter_append (&sub_iter, G_TYPE_STRING, &(name->data));
    }
    g_list_free (names);

    // append unique names
    names = g_hash_table_get_keys (dbus->unique_names);

    names = g_list_sort (names, (GCompareFunc) g_strcmp0);
    for (name = names; name != NULL; name = name->next) {
        ibus_message_iter_append (&sub_iter, G_TYPE_STRING, &(name->data));
    }
    g_list_free (names);

    ibus_message_iter_close_container (&iter, &sub_iter);

    return reply_message;
}

static IBusMessage *
_dbus_name_has_owner (BusDBusImpl   *dbus,
                      IBusMessage   *message,
                      BusConnection *connection)
{
    gchar *name;
    gboolean retval;
    gboolean has_owner;
    IBusMessage *reply_message;
    IBusError *error;

    retval = ibus_message_get_args (message,
                                    &error,
                                    G_TYPE_STRING, &name,
                                    G_TYPE_INVALID);

    if (! retval) {
        reply_message = ibus_message_new_error (message,
                                                error->name,
                                                error->message);
        ibus_error_free (error);
        return reply_message;
    }

    if (name[0] == ':') {
        has_owner = g_hash_table_lookup (dbus->unique_names, name) != NULL;
    }
    else {
        has_owner = g_hash_table_lookup (dbus->names, name) != NULL;
    }

    reply_message = ibus_message_new_method_return (message);
    ibus_message_append_args (reply_message,
                              G_TYPE_BOOLEAN, &has_owner,
                              G_TYPE_INVALID);

    return reply_message;
}


static IBusMessage *
_dbus_get_name_owner (BusDBusImpl   *dbus,
                      IBusMessage   *message,
                      BusConnection *connection)
{
    gchar *name;
    BusConnection *owner;
    gboolean retval;
    const gchar *owner_name = NULL;
    IBusMessage *reply_message;
    IBusError *error;

    retval = ibus_message_get_args (message,
                                    &error,
                                    G_TYPE_STRING, &name,
                                    G_TYPE_INVALID);

    if (! retval) {
        reply_message = ibus_message_new_error (message,
                                                error->name,
                                                error->message);
        ibus_error_free (error);
        return reply_message;
    }

    if (g_strcmp0 (name, DBUS_SERVICE_DBUS) == 0 ||
        g_strcmp0 (name, IBUS_SERVICE_IBUS) == 0) {
        owner_name = name;
    }
    else {
        owner = bus_dbus_impl_get_connection_by_name (dbus, name);
        if (owner != NULL) {
            owner_name = bus_connection_get_unique_name (owner);
        }
    }

    if (owner_name != NULL) {
        reply_message = ibus_message_new_method_return (message);
        ibus_message_append_args (reply_message,
                                  G_TYPE_STRING, &owner_name,
                                  G_TYPE_INVALID);
    }
    else {
        reply_message = ibus_message_new_error_printf (message,
                                                       DBUS_ERROR_NAME_HAS_NO_OWNER,
                                                       "Name '%s' does have owner",
                                                       name);
    }

    return reply_message;
}

static IBusMessage *
_dbus_get_id (BusDBusImpl   *dbus,
              IBusMessage   *message,
              BusConnection *connection)
{
    IBusMessage *reply_message;
    const gchar *name;

    name = bus_connection_get_unique_name (connection);

    if (name == NULL) {
        reply_message = ibus_message_new_error (message,
                                                DBUS_ERROR_FAILED,
                                                "Can not GetId before Hello");
        return reply_message;
    }

    reply_message = ibus_message_new_method_return (message);
    ibus_message_append_args (reply_message,
                              G_TYPE_STRING, &name,
                              G_TYPE_INVALID);
    return reply_message;
}

static void
_rule_destroy_cb (BusMatchRule *rule,
                  BusDBusImpl  *dbus)
{
    g_assert (BUS_IS_MATCH_RULE (rule));
    g_assert (BUS_IS_DBUS_IMPL (dbus));

    dbus->rules = g_list_remove (dbus->rules, rule);
    g_object_unref (rule);
}

static IBusMessage *
_dbus_add_match (BusDBusImpl    *dbus,
                 IBusMessage    *message,
                 BusConnection  *connection)
{
    IBusMessage *reply_message;
    IBusError *error;
    gboolean retval;
    gchar *rule_text;
    BusMatchRule *rule;
    GList *link;

    retval = ibus_message_get_args (message,
                                    &error,
                                    G_TYPE_STRING, &rule_text,
                                    G_TYPE_INVALID);

    if (!retval) {
        reply_message = ibus_message_new_error (message,
                                                error->name,
                                                error->message);
        ibus_error_free (error);
        return reply_message;
    }

    rule = bus_match_rule_new (rule_text);

    if (rule == NULL) {
         reply_message = ibus_message_new_error_printf (message,
                                                        DBUS_ERROR_MATCH_RULE_INVALID,
                                                        "Parse rule [%s] failed",
                                                        rule_text);
        return reply_message;
    }

    for (link = dbus->rules; link != NULL; link = link->next) {
        if (bus_match_rule_is_equal (rule, BUS_MATCH_RULE (link->data))) {
            bus_match_rule_add_recipient (BUS_MATCH_RULE (link->data), connection);
            g_object_unref (rule);
            rule = NULL;
            break;
        }
    }

    if (rule) {
        bus_match_rule_add_recipient (rule, connection);
        dbus->rules = g_list_append (dbus->rules, rule);
        g_signal_connect (rule, "destroy", G_CALLBACK (_rule_destroy_cb), dbus);
    }

    reply_message = ibus_message_new_method_return (message);
    return reply_message;
}

static IBusMessage *
_dbus_remove_match (BusDBusImpl     *dbus,
                    IBusMessage     *message,
                    BusConnection   *connection)
{
    IBusMessage *reply_message;
    IBusError *error;
    gchar *rule_text;
    BusMatchRule *rule;
    GList *link;

    if (!ibus_message_get_args (message,
                                &error,
                                G_TYPE_STRING, &rule_text,
                                G_TYPE_INVALID)) {
        reply_message = ibus_message_new_error (message,
                                                error->name,
                                                error->message);
        ibus_error_free (error);
        return reply_message;
    }

    rule = bus_match_rule_new (rule_text);

    if (rule == NULL ) {
         reply_message = ibus_message_new_error_printf (message,
                                                        DBUS_ERROR_MATCH_RULE_INVALID,
                                                        "Parse rule [%s] failed",
                                                        rule_text);
        return reply_message;
    }

    for (link = dbus->rules; link != NULL; link = link->next) {
        if (bus_match_rule_is_equal (rule, BUS_MATCH_RULE (link->data))) {
            bus_match_rule_remove_recipient (BUS_MATCH_RULE (link->data), connection);
            break;
        }
    }

    g_object_unref (rule);

    reply_message = ibus_message_new_method_return (message);
    return reply_message;
}

static IBusMessage *
_dbus_request_name (BusDBusImpl     *dbus,
                    IBusMessage     *message,
                    BusConnection   *connection)
{
    IBusMessage *reply_message;
    IBusError *error;
    gchar *name;
    guint flags;
    guint retval;

    if (!ibus_message_get_args (message,
                                &error,
                                G_TYPE_STRING, &name,
                                G_TYPE_UINT, &flags,
                                G_TYPE_INVALID)) {
        reply_message = ibus_message_new_error (message,
                                                error->name,
                                                error->message);
        ibus_error_free (error);
        return reply_message;
    }

    if (g_strcmp0 (name, DBUS_SERVICE_DBUS) == 0 ||
        g_strcmp0 (name, IBUS_SERVICE_IBUS) == 0 ||
        g_hash_table_lookup (dbus->names, name) != NULL) {
        reply_message = ibus_message_new_error_printf (message,
                                                       DBUS_ERROR_FAILED,
                                                       "Name %s has owner",
                                                       name);
        return reply_message;
    }

    retval = 1;
    g_hash_table_insert (dbus->names,
                         (gpointer )bus_connection_add_name (connection, name),
                         connection);
    reply_message = ibus_message_new_method_return (message);
    ibus_message_append_args (reply_message,
                              G_TYPE_UINT, &retval,
                              G_TYPE_INVALID);

    ibus_connection_send ((IBusConnection *) connection, reply_message);
    ibus_message_unref (reply_message);
    ibus_connection_flush ((IBusConnection *) connection);

    g_signal_emit (dbus,
                   dbus_signals[NAME_OWNER_CHANGED],
                   0,
                   name,
                   "",
                   bus_connection_get_unique_name (connection));

    return NULL;
}

static IBusMessage *
_dbus_release_name (BusDBusImpl     *dbus,
                    IBusMessage     *message,
                    BusConnection   *connection)
{
    IBusMessage *reply_message;
    IBusError *error;
    gchar *name;
    guint retval;

    if (!ibus_message_get_args (message,
                                &error,
                                G_TYPE_STRING, &name,
                                G_TYPE_INVALID)) {
        reply_message = ibus_message_new_error (message,
                                                error->name,
                                                error->message);
        ibus_error_free (error);
        return reply_message;
    }

    reply_message = ibus_message_new_method_return (message);
    if (bus_connection_remove_name (connection, name)) {
        retval = 1;
    }
    else {
        retval = 2;
    }

    ibus_message_append_args (message,
                              G_TYPE_UINT, &retval,
                              G_TYPE_INVALID);

    return reply_message;
}


static gboolean
bus_dbus_impl_ibus_message (BusDBusImpl  *dbus,
                            BusConnection   *connection,
                            IBusMessage     *message)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    gint i;
    IBusMessage *reply_message = NULL;

    static const struct {
        const gchar *interface;
        const gchar *name;
        IBusMessage *(* handler) (BusDBusImpl *, IBusMessage *, BusConnection *);
    } handlers[] =  {
        /* Introspectable interface */
        { DBUS_INTERFACE_INTROSPECTABLE,
                               "Introspect", _dbus_introspect },
        /* DBus interface */
        { DBUS_INTERFACE_DBUS, "Hello",     _dbus_hello },
        { DBUS_INTERFACE_DBUS, "ListNames", _dbus_list_names },
        { DBUS_INTERFACE_DBUS, "ListActivatableNames",
                                            _dbus_no_implement },
        { DBUS_INTERFACE_DBUS, "NameHasOwner",
                                            _dbus_name_has_owner },
        { DBUS_INTERFACE_DBUS, "StartServiceByName",
                                            _dbus_no_implement },
        { DBUS_INTERFACE_DBUS, "GetNameOwner",
                                            _dbus_get_name_owner },
        { DBUS_INTERFACE_DBUS, "GetConnectionUnixUser",
                                            _dbus_no_implement },
        { DBUS_INTERFACE_DBUS, "AddMatch",  _dbus_add_match },
        { DBUS_INTERFACE_DBUS, "RemoveMatch",
                                            _dbus_remove_match },
        { DBUS_INTERFACE_DBUS, "GetId",     _dbus_get_id },
        { DBUS_INTERFACE_DBUS, "RequestName", _dbus_request_name },
        { DBUS_INTERFACE_DBUS, "ReleaseName", _dbus_release_name },
        { NULL, NULL, NULL }
    };

    ibus_message_set_destination (message, DBUS_SERVICE_DBUS);

    for (i = 0; handlers[i].interface != NULL; i++) {
        if (ibus_message_is_method_call (message,
                                         handlers[i].interface,
                                         handlers[i].name)) {

            reply_message = handlers[i].handler (dbus, message, connection);
            if (reply_message) {

                ibus_message_set_sender (reply_message, DBUS_SERVICE_DBUS);
                ibus_message_set_destination (reply_message,
                                              bus_connection_get_unique_name (connection));
                ibus_message_set_no_reply (reply_message, TRUE);

                ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
                ibus_message_unref (reply_message);
            }

            g_signal_stop_emission_by_name (dbus, "ibus-message");
            return TRUE;
        }
    }

    return IBUS_SERVICE_CLASS (bus_dbus_impl_parent_class)->ibus_message (
                                (IBusService *) dbus,
                                (IBusConnection *) connection,
                                message);
}

static void
bus_dbus_impl_name_owner_changed (BusDBusImpl   *dbus,
                                  gchar         *name,
                                  gchar         *old_name,
                                  gchar         *new_name)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (name != NULL);
    g_assert (old_name != NULL);
    g_assert (new_name != NULL);

    IBusMessage *message;

    message = ibus_message_new_signal (DBUS_PATH_DBUS,
                                       DBUS_INTERFACE_DBUS,
                                       "NameOwnerChanged");
    ibus_message_append_args (message,
                              G_TYPE_STRING, &name,
                              G_TYPE_STRING, &old_name,
                              G_TYPE_STRING, &new_name,
                              G_TYPE_INVALID);
    ibus_message_set_sender (message, DBUS_SERVICE_DBUS);

    bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);

    ibus_message_unref (message);

}

static gboolean
_connection_ibus_message_cb (BusConnection  *connection,
                             IBusMessage    *message,
                             BusDBusImpl    *dbus)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (message != NULL);
    g_assert (BUS_IS_DBUS_IMPL (dbus));

    const gchar *dest;

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus))) {
        return FALSE;
    }

    if (ibus_message_is_signal (message,
                                DBUS_INTERFACE_LOCAL,
                                "Disconnected")) {
        /* ignore signal from local interface */
        return FALSE;
    }

    ibus_message_set_sender (message, bus_connection_get_unique_name (connection));

    switch (ibus_message_get_type (message)) {
#if 1
    case DBUS_MESSAGE_TYPE_ERROR:
        g_debug ("From :%s to %s, Error: %s : %s",
                 ibus_message_get_sender (message),
                 ibus_message_get_destination (message),
                 ibus_message_get_error_name (message),
                 ibus_message_get_error_message (message));
        break;
#endif
#if 0
    case DBUS_MESSAGE_TYPE_SIGNAL:
        g_debug ("From :%s to %s, Signal: %s @ %s",
                 ibus_message_get_sender (message),
                 ibus_message_get_destination (message),
                 ibus_message_get_member (message),
                 ibus_message_get_path (message)
                 );
        break;
#endif
#if 0
    case DBUS_MESSAGE_TYPE_METHOD_CALL:
        g_debug("From %s to %s, Method %s on %s",
                ibus_message_get_sender (message),
                ibus_message_get_destination (message),
                ibus_message_get_path (message),
                ibus_message_get_member (message));
        break;
#endif
    }

    dest = ibus_message_get_destination (message);

    if (dest == NULL ||
        strcmp ((gchar *)dest, IBUS_SERVICE_IBUS) == 0 ||
        strcmp ((gchar *)dest, DBUS_SERVICE_DBUS) == 0) {
        /* this message is sent to ibus-daemon */

        switch (ibus_message_get_type (message)) {
        case DBUS_MESSAGE_TYPE_SIGNAL:
        case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        case DBUS_MESSAGE_TYPE_ERROR:
            bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
            return FALSE;
        case DBUS_MESSAGE_TYPE_METHOD_CALL:
            {
                const gchar *path;
                IBusService *object;

                path = ibus_message_get_path (message);

                object = g_hash_table_lookup (dbus->objects, path);

                if (object == NULL ||
                    ibus_service_handle_message (object,
                                                 (IBusConnection *) connection,
                                                 message) == FALSE) {
                    IBusMessage *error;
                    error = ibus_message_new_error_printf (message,
                                                           DBUS_ERROR_UNKNOWN_METHOD,
                                                           "Unknown method %s on %s",
                                                           ibus_message_get_member (message),
                                                           path);
                    ibus_connection_send ((IBusConnection *) connection, error);
                    ibus_message_unref (error);
                }

                /* dispatch message */
                bus_dbus_impl_dispatch_message_by_rule (dbus, message, NULL);
            }
            break;
        default:
            g_assert (FALSE);
        }
    }
    else {
        /* If the destination is not IBus or DBus, the message will be forwanded. */
        bus_dbus_impl_dispatch_message (dbus, message);
    }

    g_signal_stop_emission_by_name (connection, "ibus-message");
    return TRUE;
}

static void
_connection_ibus_message_sent_cb (BusConnection  *connection,
                                  IBusMessage    *message,
                                  BusDBusImpl    *dbus)
{
    bus_dbus_impl_dispatch_message_by_rule (dbus, message, connection);
}

static void
_connection_destroy_cb (BusConnection   *connection,
                        BusDBusImpl     *dbus)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (BUS_IS_DBUS_IMPL (dbus));

    /*
    ibus_service_remove_from_connection (
                    IBUS_SERVICE (dbus),
                    IBUS_CONNECTION (connection));
    */

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
bus_dbus_impl_new_connection (BusDBusImpl    *dbus,
                              BusConnection  *connection)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (BUS_IS_CONNECTION (connection));

    g_assert (g_list_find (dbus->connections, connection) == NULL);

    g_object_ref_sink (connection);
    dbus->connections = g_list_append (dbus->connections, connection);

    g_signal_connect (connection,
                      "ibus-message",
                      G_CALLBACK (_connection_ibus_message_cb),
                      dbus);

    g_signal_connect (connection,
                      "ibus-message-sent",
                      G_CALLBACK (_connection_ibus_message_sent_cb),
                      dbus);


    g_signal_connect (connection,
                      "destroy",
                      G_CALLBACK (_connection_destroy_cb),
                      dbus);
    return TRUE;
}


BusConnection *
bus_dbus_impl_get_connection_by_name (BusDBusImpl    *dbus,
                                      const gchar    *name)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (name != NULL);

    BusConnection *connection = NULL;

    if (name[0] == ':') {
        connection = BUS_CONNECTION (g_hash_table_lookup (
                                        dbus->unique_names,
                                        name));
    }
    else {
        connection = BUS_CONNECTION (g_hash_table_lookup (
                                        dbus->names,
                                        name));
    }

    return connection;
}


void
bus_dbus_impl_dispatch_message (BusDBusImpl  *dbus,
                                IBusMessage  *message)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (message != NULL);

    const gchar *destination;
    BusConnection *dest_connection = NULL;

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus))) {
        return;
    }

    destination = ibus_message_get_destination (message);

    if (destination != NULL) {
        dest_connection = bus_dbus_impl_get_connection_by_name (dbus, destination);

        if (dest_connection != NULL) {
            ibus_connection_send (IBUS_CONNECTION (dest_connection), message);
        }
        else {
            IBusMessage *reply_message;
            reply_message = ibus_message_new_error_printf (message,
                                                     DBUS_ERROR_SERVICE_UNKNOWN,
                                                     "Can not find service %s",
                                                     destination);
            bus_dbus_impl_dispatch_message (dbus, reply_message);
            ibus_message_unref (reply_message);
        }
    }

    bus_dbus_impl_dispatch_message_by_rule (dbus, message, dest_connection);
}

void
bus_dbus_impl_dispatch_message_by_rule (BusDBusImpl     *dbus,
                                        IBusMessage     *message,
                                        BusConnection   *skip_connection)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (skip_connection) || skip_connection == NULL);

    GList *recipients = NULL;
    GList *link = NULL;

    static gint32 data_slot = -1;

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus))) {
        return;
    }

    if (data_slot == -1) {
        dbus_message_allocate_data_slot (&data_slot);
    }

    /* If this message has been dispatched by rule, it will be ignored. */
    if (dbus_message_get_data (message, data_slot) != NULL)
        return;

    dbus_message_set_data (message, data_slot, (gpointer) TRUE, NULL);

    for (link = dbus->rules; link != NULL; link = link->next) {
        GList *list = bus_match_rule_get_recipients (BUS_MATCH_RULE (link->data),
                                                     message);
        recipients = g_list_concat (recipients, list);
    }

    for (link = recipients; link != NULL; link = link->next) {
        BusConnection *connection = BUS_CONNECTION (link->data);
        if (connection != skip_connection) {
            ibus_connection_send (IBUS_CONNECTION (connection), message);
        }
    }
    g_list_free (recipients);
}


static void
_object_destroy_cb (IBusService *object,
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

    const gchar *path;

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus))) {
        return FALSE;
    }

    path = ibus_service_get_path (object);

    g_return_val_if_fail (path, FALSE);

    g_return_val_if_fail  (g_hash_table_lookup (dbus->objects, path) == NULL, FALSE);

    g_object_ref_sink (object);
    g_hash_table_insert (dbus->objects, (gpointer)path, object);

    g_signal_connect (object, "destroy", G_CALLBACK (_object_destroy_cb), dbus);

    return TRUE;
}

gboolean
bus_dbus_impl_unregister_object (BusDBusImpl *dbus,
                                 IBusService *object)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (IBUS_IS_SERVICE (object));

    const gchar *path;

    if (G_UNLIKELY (IBUS_OBJECT_DESTROYED (dbus))) {
        return FALSE;
    }

    path = ibus_service_get_path (object);
    g_return_val_if_fail (path, FALSE);

    g_return_val_if_fail  (g_hash_table_lookup (dbus->objects, path) == object, FALSE);

    g_signal_handlers_disconnect_by_func (object, G_CALLBACK (_object_destroy_cb), dbus);

    g_hash_table_remove (dbus->objects, path);
    g_object_unref (object);

    return TRUE;
}

