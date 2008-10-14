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

#include "dbusimpl.h"

#define BUS_DBUS_IMPL_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_DBUS_IMPL, IBusDBusImplPrivate))

enum {
    LAST_SIGNAL,
};

enum {
    PROP_0,
    PROP_CONNECTION,
};


/* IBusDBusImplPriv */
struct _IBusDBusImplPrivate {
    IBusConnection *connection;
};
typedef struct _IBusDBusImplPrivate IBusDBusImplPrivate;

static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_dbus_impl_class_init      (BusDBusImplClass    *klass);
static void     ibus_dbus_impl_init            (BusDBusImpl         *dbus_impl);
static void     ibus_dbus_impl_dispose         (BusDBusImpl         *dbus_impl);
static gboolean ibus_dbus_impl_dbus_message    (BusDBusImpl         *dbus_impl,
                                                BusConnection       *connection,
                                                DBusMessage         *message);

static IBusServiceClass  *_parent_class = NULL;

GType
ibus_dbus_impl_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusDBusImplClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_dbus_impl_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusDBusImpl),
        0,
        (GInstanceInitFunc) ibus_dbus_impl_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERVICE,
                    "BusDBusImpl",
                    &type_info,
                    (GTypeFlags) 0);
    }
    return type;
}

IBusDBusImpl *
ibus_dbus_impl_new (const gchar *path, IBusConnection *connection)
{
    g_assert (path);
    g_assert (IBUS_IS_CONNECTION (connection));

    IBusDBusImplPrivate *priv;
    IBusDBusImpl *dbus_impl;

    dbus_impl = IBUS_DBUS_IMPL (g_object_new (IBUS_TYPE_DBUS_IMPL,
                "path", path,
                NULL));

    priv = IBUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);
    priv->connection = g_object_ref (connection);

    ibus_service_add_to_connection (IBUS_SERVICE (dbus_impl), connection);

    return dbus_impl;
}

static void
ibus_dbus_impl_class_init (IBusDBusImplClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    _parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusDBusImplPrivate));

    gobject_class->dispose = (GObjectFinalizeFunc) ibus_dbus_impl_dispose;

    IBUS_SERVICE_CLASS (klass)->dbus_message = (ServiceDBusMessageFunc) ibus_dbus_impl_dbus_message;

}

static void
ibus_dbus_impl_init (IBusDBusImpl *dbus_impl)
{
    IBusDBusImplPrivate *priv;
    priv = IBUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    priv->connection = NULL;
}

static void
ibus_dbus_impl_dispose (IBusDBusImpl *dbus_impl)
{
    IBusDBusImplPrivate *priv;
    priv = IBUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);
    g_object_unref (priv->connection);
    G_OBJECT_CLASS(_parent_class)->dispose (G_OBJECT (dbus_impl));
}

static gboolean
ibus_dbus_impl_dbus_message (IBusDBusImpl *dbus_impl, IBusConnection *connection, DBusMessage *message)
{
    g_assert (IBUS_IS_DBUS_IMPL (dbus_impl));
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    IBusDBusImplPrivate *priv;
    priv = IBUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    g_assert (priv->connection == connection);

    DBusMessage *return_message = NULL;
    DBusMessage *error_message = NULL;

    static struct {
        gchar *member;
        guint  signal_id;
    } no_arg_methods[] = {
        {"FocusIn",     FOCUS_IN},
        {"FocusOut",    FOCUS_OUT},
        {"Reset",       RESET},
        {"Enable",      ENABLE},
        {"Disable",     DISABLE},
        {"PageUp",      PAGE_UP},
        {"PageDown",    PAGE_DOWN},
        {"CursorUp",    CURSOR_UP},
        {"CursorDown",  CURSOR_DOWN},
        {NULL, 0},
    };
    gint i;

    for (i = 0; no_arg_methods[i].member != NULL; i++) {
        if (!dbus_message_is_method_call (message, IBUS_INTERFACE_DBUS_IMPL, no_arg_methods[i].member))
            continue;

        DBusMessageIter iter;
        dbus_message_iter_init (message, &iter);
        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID) {
            error_message = dbus_message_new_error_printf (message,
                                "%s.%s: Method does not have arguments",
                                IBUS_INTERFACE_DBUS_IMPL, no_arg_methods[i].member);
            ibus_connection_send (connection, error_message);
            dbus_message_unref (error_message);
            return TRUE;
        }

        g_signal_emit (dbus_impl, _signals[no_arg_methods[i].signal_id], 0);
        return_message = dbus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;
    }


    if (dbus_message_is_method_call (message, IBUS_INTERFACE_DBUS_IMPL, "KeyPress")) {
        guint keyval, state;
        gboolean is_press, retval;
        DBusMessageIter iter;

        dbus_message_iter_init (message, &iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_UINT32)
            goto _keypress_fail;
        dbus_message_iter_get_basic (&iter, &keyval);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_BOOLEAN)
            goto _keypress_fail;
        dbus_message_iter_get_basic (&iter, &is_press);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_UINT32)
            goto _keypress_fail;
        dbus_message_iter_get_basic (&iter, &state);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID)
            goto _keypress_fail;

        g_signal_emit (dbus_impl, _signals[KEY_PRESS], 0, keyval, is_press, state, &retval);

        return_message = dbus_message_new_method_return (message);
        dbus_message_append_args (return_message, DBUS_TYPE_BOOLEAN, &retval, DBUS_TYPE_INVALID);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;

    _keypress_fail:
        error_message = dbus_message_new_error_printf (message,
                        "%s.%s: Can not match signature (ubu) of method",
                        IBUS_INTERFACE_DBUS_IMPL, "KeyPress");
        ibus_connection_send (connection, error_message);
        dbus_message_unref (error_message);
        return TRUE;
    }
    else if (dbus_message_is_method_call (message, IBUS_INTERFACE_DBUS_IMPL, "PropertyActivate")) {
        gchar *name;
        gint state;
        DBusMessageIter iter;

        dbus_message_iter_init (message, &iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_STRING)
            goto _property_activate_fail;
        dbus_message_iter_get_basic (&iter, &name);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INT32)
            goto _property_activate_fail;
        dbus_message_iter_get_basic (&iter, &state);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID)
            goto _property_activate_fail;

        g_signal_emit (dbus_impl, _signals[PROPERTY_ACTIVATE], 0, name, state);

        return_message = dbus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;

    _property_activate_fail:
        error_message = dbus_message_new_error_printf (message,
                        "%s.%s: Can not match signature (si) of method",
                        IBUS_INTERFACE_DBUS_IMPL, "PropertyActivate");
        ibus_connection_send (connection, error_message);
        dbus_message_unref (error_message);
        return TRUE;

    }
    else if (dbus_message_is_method_call (message, IBUS_INTERFACE_DBUS_IMPL, "PropertyShow")) {
        gchar *name;
        DBusMessageIter iter;

        dbus_message_iter_init (message, &iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_STRING)
            goto _property_show_fail;
        dbus_message_iter_get_basic (&iter, &name);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID)
            goto _property_show_fail;

        g_signal_emit (dbus_impl, _signals[PROPERTY_SHOW], 0, name);

        return_message = dbus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;

    _property_show_fail:
        error_message = dbus_message_new_error_printf (message,
                        "%s.%s: Can not match signature (s) of method",
                        IBUS_INTERFACE_DBUS_IMPL, "PropertyShow");
        ibus_connection_send (connection, error_message);
        dbus_message_unref (error_message);
        return TRUE;
    }
    else if (dbus_message_is_method_call (message, IBUS_INTERFACE_DBUS_IMPL, "PropertyHide")) {
        gchar *name;
        DBusMessageIter iter;

        dbus_message_iter_init (message, &iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_STRING)
            goto _property_hide_fail;
        dbus_message_iter_get_basic (&iter, &name);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID)
            goto _property_hide_fail;

        g_signal_emit (dbus_impl, _signals[PROPERTY_HIDE], 0, name);

        return_message = dbus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;

    _property_hide_fail:
        error_message = dbus_message_new_error_printf (message,
                        "%s.%s: Can not match signature (s) of method",
                        IBUS_INTERFACE_DBUS_IMPL, "PropertyHide");
        ibus_connection_send (connection, error_message);
        dbus_message_unref (error_message);
        return TRUE;
    }
    else if (dbus_message_is_method_call (message, IBUS_INTERFACE_DBUS_IMPL, "SetCursorLocation")) {
        gint args[4];
        DBusMessageIter iter;

        dbus_message_iter_init (message, &iter);
        for (i =0; i < 4; i++) {
            if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INT32)
                goto _set_cursor_location_fail;
            dbus_message_iter_get_basic (&iter, &args[i]);
            dbus_message_iter_next (&iter);
        }

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID)
            goto _set_cursor_location_fail;

        g_signal_emit (dbus_impl, _signals[SET_CURSOR_LOCATION], 0,
                    args[0], args[1], args[2], args[3]);

        return_message = dbus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;

    _set_cursor_location_fail:
        error_message = dbus_message_new_error_printf (message,
                        "%s.%s: Can not match signature (iiii) of method",
                        IBUS_INTERFACE_DBUS_IMPL, "SetCursorLocation");
        ibus_connection_send (connection, error_message);
        dbus_message_unref (error_message);
        return TRUE;
    }

    return FALSE;
}

