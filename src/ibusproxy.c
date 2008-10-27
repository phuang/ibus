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
#include "ibusproxy.h"
#include "ibusinternal.h"

#define IBUS_PROXY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_PROXY, IBusProxyPrivate))

enum {
    DBUS_SIGNAL,
    LAST_SIGNAL,
};

enum {
    PROP_0,
    PROP_NAME,
    PROP_PATH,
    PROP_INTERFACE,
    PROP_CONNECTION,
};


/* IBusProxyPriv */
struct _IBusProxyPrivate {
    gchar *name;
    gchar *path;
    gchar *interface;
    IBusConnection *connection;
};
typedef struct _IBusProxyPrivate IBusProxyPrivate;

static guint            proxy_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void      ibus_proxy_class_init  (IBusProxyClass *klass);
static void      ibus_proxy_init        (IBusProxy      *proxy);
static GObject  *ibus_proxy_constructor (GType           type,
                                         guint           n_construct_params,
                                         GObjectConstructParam *construct_params);
static void      ibus_proxy_destroy     (IBusProxy       *proxy);
static void      ibus_proxy_set_property(IBusProxy       *proxy,
                                         guint           prop_id,
                                         const GValue    *value,
                                         GParamSpec      *pspec);
static void      ibus_proxy_get_property(IBusProxy       *proxy,
                                         guint           prop_id,
                                         GValue          *value,
                                         GParamSpec      *pspec);

static gboolean  ibus_proxy_dbus_signal (IBusProxy       *proxy,
                                         DBusMessage     *message);

static IBusObjectClass  *_parent_class = NULL;

GType
ibus_proxy_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusProxyClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_proxy_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusProxy),
        0,
        (GInstanceInitFunc) ibus_proxy_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "IBusProxy",
                    &type_info,
                    (GTypeFlags)0);
    }
    return type;
}

IBusProxy *
ibus_proxy_new (const gchar     *name,
                const gchar     *path,
                IBusConnection  *connection)
{
    g_assert (name != NULL);
    g_assert (path != NULL);
    g_assert (IBUS_IS_CONNECTION (connection));

    IBusProxy *proxy;

    proxy = IBUS_PROXY (g_object_new (IBUS_TYPE_PROXY,
                            "name", name,
                            "path", path,
                            "connection", connection,
                            NULL));

    return proxy;
}

static void
ibus_proxy_class_init (IBusProxyClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    _parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusProxyPrivate));
    
    gobject_class->constructor = ibus_proxy_constructor;
    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_proxy_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_proxy_get_property;
    
    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_proxy_destroy;

    klass->dbus_signal = ibus_proxy_dbus_signal;
    
    /* install properties */
    g_object_class_install_property (gobject_class,
                    PROP_NAME,
                    g_param_spec_string ("name",
                        "service name",
                        "The service name of proxy object",
                        NULL,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    
    g_object_class_install_property (gobject_class,
                    PROP_PATH,
                    g_param_spec_string ("path",
                        "object path",
                        "The path of proxy object",
                        NULL,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
     
     g_object_class_install_property (gobject_class,
                    PROP_INTERFACE,
                    g_param_spec_string ("interface",
                        "interface",
                        "The interface of proxy object",
                        NULL,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
   
    g_object_class_install_property (gobject_class,
                    PROP_CONNECTION,
                    g_param_spec_object ("connection",
                        "object path",
                        "The path of proxy object",
                        IBUS_TYPE_CONNECTION,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    /* install signals */
    proxy_signals[DBUS_SIGNAL] =
        g_signal_new (I_("dbus-signal"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusProxyClass, dbus_signal),
            NULL, NULL,
            ibus_marshal_BOOLEAN__POINTER,
            G_TYPE_BOOLEAN,
            1, G_TYPE_POINTER);

}

static gboolean
_connection_dbus_signal_cb (IBusConnection *connection,
                             DBusMessage    *message,
                             IBusProxy      *proxy)
{
    if (ibus_proxy_handle_signal (proxy, message)) {
        g_signal_stop_emission_by_name (connection, "dbus-signal");
        return TRUE;
    }

    return FALSE;
}

static void
_connection_destroy_cb (IBusConnection  *connection,
                        IBusProxy       *proxy)
{
    ibus_object_destroy (IBUS_OBJECT (proxy));
}

static GObject *
ibus_proxy_constructor (GType           type,
                        guint           n_construct_params,
                        GObjectConstructParam *construct_params)
{
    GObject *obj;
    IBusProxy *proxy;
    IBusProxyPrivate *priv;

    obj = G_OBJECT_CLASS (_parent_class)->constructor (type, n_construct_params, construct_params);
    
    proxy = IBUS_PROXY (obj);
    priv = IBUS_PROXY_GET_PRIVATE (proxy);
    
    if (priv->connection != NULL) {
        g_signal_connect (priv->connection,
                          "dbus-signal",
                          (GCallback) _connection_dbus_signal_cb,
                          proxy);
        
        g_signal_connect (priv->connection,
                          "destroy",
                          (GCallback) _connection_destroy_cb,
                          proxy);
    }

    return obj;
}

static void
ibus_proxy_init (IBusProxy *proxy)
{
    IBusProxyPrivate *priv;
    priv = IBUS_PROXY_GET_PRIVATE (proxy);

    priv->name = NULL;
    priv->path = NULL;
    priv->interface = NULL;
    priv->connection = NULL;
}

static void
ibus_proxy_destroy (IBusProxy *proxy)
{
    IBusProxyPrivate *priv;
    priv = IBUS_PROXY_GET_PRIVATE (proxy);

    if (priv->connection) {
        g_signal_handlers_disconnect_by_func (priv->connection,
                                              (GCallback) _connection_dbus_signal_cb,
                                              proxy);
        g_signal_handlers_disconnect_by_func (priv->connection,
                                              (GCallback) _connection_destroy_cb,
                                              proxy);
    }
    
    g_free (priv->name);
    g_free (priv->path);
    g_free (priv->interface);
    
    priv->name = NULL;
    priv->path = NULL;
    priv->interface = NULL;
    
    if (priv->connection) {
        g_object_unref (priv->connection);
        priv->connection = NULL;
    }
    
    IBUS_OBJECT_CLASS(_parent_class)->destroy (IBUS_OBJECT (proxy));
}

static void
ibus_proxy_set_property (IBusProxy      *proxy,
                         guint           prop_id,
                         const GValue   *value,
                         GParamSpec     *pspec)
{
    IBusProxyPrivate *priv;
    priv = IBUS_PROXY_GET_PRIVATE (proxy);
    
    switch (prop_id) {
    case PROP_NAME:
        g_assert (priv->name == NULL);
        priv->name = g_strdup (g_value_get_string (value));
        break;
    case PROP_PATH:
        g_assert (priv->path == NULL);
        priv->path = g_strdup (g_value_get_string (value));
        break;
    case PROP_INTERFACE:
        g_assert (priv->interface == NULL);
        priv->interface = g_strdup (g_value_get_string (value));
        break;
    case PROP_CONNECTION:
        g_assert (priv->connection == NULL);
        priv->connection = IBUS_CONNECTION (g_value_get_object (value));
        g_object_ref (priv->connection);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (proxy, prop_id, pspec);
    }
}

static void
ibus_proxy_get_property (IBusProxy      *proxy,
                         guint           prop_id,
                         GValue         *value,
                         GParamSpec     *pspec)
{
    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string (value, ibus_proxy_get_name (proxy));
        break;
    case PROP_PATH:
        g_value_set_string (value, ibus_proxy_get_path (proxy));
        break;
    case PROP_INTERFACE:
        g_value_set_string (value, ibus_proxy_get_interface (proxy));
        break;
    case PROP_CONNECTION:
        g_value_set_object (value, ibus_proxy_get_connection (proxy));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (proxy, prop_id, pspec);
    }
}


gboolean
ibus_proxy_handle_signal (IBusProxy     *proxy,
                          DBusMessage   *message)
{
    g_assert (IBUS_IS_PROXY (proxy));
    g_assert (message != NULL);
    
    gboolean retval = FALSE;
    IBusProxyPrivate *priv;
    priv = IBUS_PROXY_GET_PRIVATE (proxy);

    if (g_strcmp0 (dbus_message_get_path (message), priv->path) == 0) {
        g_signal_emit (proxy, proxy_signals[DBUS_SIGNAL], 0, message, &retval);
    }
    
    return retval;
}

static gboolean
ibus_proxy_dbus_signal (IBusProxy   *proxy,
                        DBusMessage *message)
{
    return FALSE;
}


const gchar *
ibus_proxy_get_name (IBusProxy *proxy)
{
    IBusProxyPrivate *priv;
    priv = IBUS_PROXY_GET_PRIVATE (proxy);

    return priv->name;
}

const gchar *
ibus_proxy_get_path (IBusProxy *proxy)
{
    IBusProxyPrivate *priv;
    priv = IBUS_PROXY_GET_PRIVATE (proxy);

    return priv->path;
}

const gchar *
ibus_proxy_get_interface (IBusProxy *proxy)
{
    IBusProxyPrivate *priv;
    priv = IBUS_PROXY_GET_PRIVATE (proxy);

    return priv->interface;
}

IBusConnection *
ibus_proxy_get_connection (IBusProxy *proxy)
{
    IBusProxyPrivate *priv;
    priv = IBUS_PROXY_GET_PRIVATE (proxy);

    return priv->connection;
}

gboolean
ibus_proxy_send (IBusProxy      *proxy,
                 DBusMessage    *message)
{
    g_assert (IBUS_IS_PROXY (proxy));
    g_assert (message != NULL);
    
    IBusProxyPrivate *priv;
    priv = IBUS_PROXY_GET_PRIVATE (proxy);
    
    return ibus_connection_send (priv->connection, message);
}

gboolean
ibus_proxy_call (IBusProxy      *proxy,
                 const gchar    *method,
                 gint           first_arg_type,
                 ...)
{
    g_assert (IBUS_IS_PROXY (proxy));
    g_assert (method != NULL);
    
    va_list args;
    gboolean retval;
    DBusMessage *message;
    IBusProxyPrivate *priv;
    priv = IBUS_PROXY_GET_PRIVATE (proxy);

    message = dbus_message_new_method_call (priv->name,
                                            priv->path,
                                            priv->interface,
                                            method);
    va_start (args, first_arg_type);
    retval = dbus_message_append_args_valist (message,
                                              first_arg_type,
                                              args);
    va_end (args);

    retval = ibus_connection_send (priv->connection, message);

    dbus_message_unref (message);

    return retval;
}

DBusMessage *
ibus_proxy_call_with_reply_and_block (IBusProxy      *proxy,
                                      const gchar    *method,
                                      gint           timeout_milliseconds,
                                      IBusError      **error,
                                      gint           first_arg_type,
                                      ...)
{
    g_assert (IBUS_IS_PROXY (proxy));
    g_assert (method != NULL);
    
    va_list args;
    gboolean retval;
    DBusMessage *message;
    DBusMessage *reply_message;

    IBusProxyPrivate *priv;
    priv = IBUS_PROXY_GET_PRIVATE (proxy);

    message = dbus_message_new_method_call (priv->name,
                                            priv->path,
                                            priv->interface,
                                            method);
    va_start (args, first_arg_type);
    retval = dbus_message_append_args_valist (message,
                                              first_arg_type,
                                              args);
    va_end (args);

    reply_message = ibus_connection_send_with_reply_and_block (
                                            priv->connection,
                                            message,
                                            timeout_milliseconds,
                                            error);
    dbus_message_unref (message);

    return reply_message;
}
