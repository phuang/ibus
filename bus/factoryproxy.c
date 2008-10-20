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

#include <ibusinternal.h>
#include <ibusmarshalers.h>
#include "factoryproxy.h"

#define BUS_FACTORY_PROXY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_FACTORY_PROXY, BusFactoryProxyPrivate))

enum {
    COMMIT_STRING,
    FORWARD_KEY_EVENT,
    UPDATE_PREEDIT,
    SHOW_PREEDIT,
    HIDE_PREEDIT,
    UPDATE_AUX_STRING,
    SHOW_AUX_STRING,
    HIDE_AUX_STRING,
    UPDATE_LOOKUP_TABLE,
    SHOW_LOOKUP_TABLE,
    HIDE_LOOKUP_TABLE,
    PAGE_UP_LOOKUP_TABLE,
    PAGE_DOWN_LOOKUP_TABLE,
    CURSOR_UP_LOOKUP_TABLE,
    CURSOR_DOWN_LOOKUP_TABLE,
    REGISTER_PROPERTIES,
    UPDATE_PROPERTY,
    LAST_SIGNAL,
};


/* BusFactoryProxyPriv */
struct _BusFactoryProxyPrivate {
    void *pad;
};
typedef struct _BusFactoryProxyPrivate BusFactoryProxyPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_factory_proxy_class_init     (BusFactoryProxyClass    *klass);
static void     bus_factory_proxy_init           (BusFactoryProxy         *factory);
static void     _bus_factory_proxy_destroy       (BusFactoryProxy         *factory);


static IBusProxyClass  *_parent_class = NULL;

GType
bus_factory_proxy_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusFactoryProxyClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_factory_proxy_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusFactoryProxy),
        0,
        (GInstanceInitFunc) bus_factory_proxy_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_PROXY,
                    "BusFactoryProxy",
                    &type_info,
                    (GTypeFlags)0);
    }
    return type;
}

BusFactoryProxy *
bus_factory_proxy_new (const gchar       *path,
                      BusConnection     *connection)
{
    g_assert (path != NULL);
    g_assert (BUS_IS_CONNECTION (connection));
    GObject *obj;

    obj = g_object_new (BUS_TYPE_FACTORY_PROXY,
                        "name", "",
                        "path", path,
                        "connection", connection,
                        NULL);

    return BUS_FACTORY_PROXY (obj);
}

static void
bus_factory_proxy_class_init (BusFactoryProxyClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    _parent_class = (IBusProxyClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusFactoryProxyPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) _bus_factory_proxy_destroy;

    // proxy_class->dbus_signal = bus_factory_proxy_dbus_signal;
}

static void
bus_factory_proxy_init (BusFactoryProxy *factory_proxy)
{
    BusFactoryProxyPrivate *priv;
    priv = BUS_FACTORY_PROXY_GET_PRIVATE (factory_proxy);
}

static void
_bus_factory_proxy_destroy (BusFactoryProxy *factory_proxy)
{
    IBUS_OBJECT_CLASS(_parent_class)->destroy (IBUS_OBJECT (factory_proxy));
}

gboolean
bus_factory_proxy_get_info (BusFactoryProxy *factory,
                            gchar           **name,
                            gchar           **lang,
                            gchar           **icon,
                            gchar           **authors,
                            gchar           **credits)
{
    g_assert (BUS_IS_FACTORY_PROXY (factory));
    g_assert (name != NULL);
    g_assert (lang != NULL);
    g_assert (icon != NULL);
    g_assert (authors != NULL);
    g_assert (credits != NULL);

    DBusMessage *reply_message;
    IBusError *error;

    reply_message = ibus_proxy_call_with_reply_and_block (IBUS_PROXY (factory),
                                                  "GetInfo",
                                                  -1,
                                                  &error,
                                                  DBUS_TYPE_INVALID);
    if (reply_message == NULL) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        return FALSE;
    }

    if (dbus_message_get_type (reply_message) == DBUS_MESSAGE_TYPE_ERROR) {
        g_warning ("%s",
                 dbus_message_get_error_name (reply_message));
        dbus_message_unref (reply_message);
        return FALSE;
    }

    DBusError _error;
    dbus_error_init (&_error);
    gchar ** values;
    gint n;
    
    if (!dbus_message_get_args (reply_message, &_error,
                                DBUS_TYPE_ARRAY, DBUS_TYPE_STRING,
                                &values, n,
                                DBUS_TYPE_INVALID)) {
        g_warning ("%s: %s", _error.name, _error.message);
        dbus_error_free (&_error);
        dbus_message_unref (reply_message);
        return FALSE;
    }
    
    if(n != 5) {
        g_warning ("Factory.GetInfo should return 5 element");
        dbus_free_string_array (values);
        dbus_message_unref (reply_message);
        return FALSE;
    } else {
        *name =  g_strdup (values[0]);
        *lang =  g_strdup (values[1]);
        *icon =  g_strdup (values[2]);
        *authors =  g_strdup (values[3]);
        *credits =  g_strdup (values[4]);
        dbus_free_string_array (values);
        dbus_message_unref (reply_message);
        return TRUE;
    }
}

gboolean
bus_factory_proxy_create_engine (BusFactoryProxy *factory,
                                 gchar           **object_path)
{
    g_assert (BUS_IS_FACTORY_PROXY (factory));
    g_assert (object_path != NULL);

    DBusMessage *reply_message;
    IBusError *error;

    reply_message = ibus_proxy_call_with_reply_and_block (IBUS_PROXY (factory),
                                                  "CreateEngine",
                                                  -1,
                                                  &error,
                                                  DBUS_TYPE_INVALID);
    if (reply_message == NULL) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        return FALSE;
    }

    if (dbus_message_get_type (reply_message) == DBUS_MESSAGE_TYPE_ERROR) {
        g_warning ("%s",
                 dbus_message_get_error_name (reply_message));
        dbus_message_unref (reply_message);
        return FALSE;
    }

    DBusError _error;
    dbus_error_init (&_error);
    gchar *value;
    
    if (!dbus_message_get_args (reply_message, &_error,
                                   DBUS_TYPE_STRING, &value,
                                   DBUS_TYPE_INVALID)) {
        g_warning ("%s: %s", _error.name, _error.message);
        dbus_error_free (&_error);
        dbus_message_unref (reply_message);
        return FALSE;
    }
    
    *object_path = g_strdup (value);
    dbus_message_unref (reply_message);
    return TRUE;
}

