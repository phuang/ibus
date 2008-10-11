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

#include "ibusfactory.h"
#include "ibusinternel.h"

#define IBUS_FACTORY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_FACTORY, IBusFactoryPrivate))
#define DECLARE_PRIV IBusFactoryPrivate *priv = IBUS_FACTORY_GET_PRIVATE(factory)

enum {
    DBUS_MESSAGE,
    LAST_SIGNAL,
};


/* IBusFactoryPriv */
struct _IBusFactoryPrivate {
    gchar *name;
    gchar *lang;
    gchar *icon;
    gchar *authors;
    gchar *credits;
};
typedef struct _IBusFactoryPrivate IBusFactoryPrivate;

static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_factory_class_init     (IBusFactoryClass   *klass);
static void     ibus_factory_init           (IBusFactory        *factory);
static void     ibus_factory_finalize       (IBusFactory        *factory);
static gboolean ibus_factory_dbus_message   (IBusFactory        *factory,
                                             IBusConnection     *connection,
                                             DBusMessage        *message);

static IBusObjectClass  *_parent_class = NULL;

GType
ibus_factory_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusFactoryClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_factory_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusFactory),
        0,
        (GInstanceInitFunc) ibus_factory_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERVICE,
                    "IBusFactory",
                    &type_info,
                    (GTypeFlags) 0);
    }
    return type;
}

IBusFactory *
ibus_factory_new (const gchar *path)
{
    IBusFactory *factory;
    factory = IBUS_FACTORY (g_object_new (IBUS_TYPE_FACTORY, "path", path, NULL));
    return factory;
}

static void
ibus_factory_class_init (IBusFactoryClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    _parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusFactoryPrivate));

    gobject_class->finalize = (GObjectFinalizeFunc) ibus_factory_finalize;

    IBUS_SERVICE_CLASS (klass)->dbus_message = (ServiceDBusMessageFunc) ibus_factory_dbus_message;
#if 0
    _signals[DBUS_MESSAGE] =
        g_signal_new (I_("dbus-message"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST,
            G_STRUCT_OFFSET (IBusFactoryClass, dbus_message),
            NULL, NULL,
            ibus_marshal_BOOLEAN__POINTER_POINTER,
            G_TYPE_BOOLEAN,
            2, G_TYPE_POINTER, G_TYPE_POINTER);
#endif
}

static void
ibus_factory_init (IBusFactory *factory)
{
    DECLARE_PRIV;
    priv->name = NULL;
    priv->lang = NULL;
    priv->icon = NULL;
    priv->authors = NULL;
    priv->credits = NULL;
}

static void
ibus_factory_finalize (IBusFactory *factory)
{
    G_OBJECT_CLASS(_parent_class)->finalize (G_OBJECT (factory));
}

gboolean
ibus_factory_handle_message (IBusFactory *factory, IBusConnection *connection, DBusMessage *message)
{
    gboolean retval = FALSE;
    g_return_val_if_fail (message != NULL, FALSE);
    
    g_signal_emit (factory, _signals[DBUS_MESSAGE], 0, connection, message, &retval);
    return retval ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static gboolean
ibus_factory_dbus_message (IBusFactory *factory, IBusConnection *connection, DBusMessage *message)
{
    return FALSE;
}
