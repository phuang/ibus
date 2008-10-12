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
#include "ibusinternal.h"

#define IBUS_FACTORY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_FACTORY, IBusFactoryPrivate))
#define DECLARE_PRIV IBusFactoryPrivate *priv = IBUS_FACTORY_GET_PRIVATE(factory)

enum {
    DBUS_MESSAGE,
    LAST_SIGNAL,
};

enum {
    PROP_0,
    PROP_NAME,
    PROP_LANG,
    PROP_ICON,
    PROP_AUTHORS,
    PROP_CREDITS,
    PROP_ENGINE_PATH,
};


/* IBusFactoryPriv */
struct _IBusFactoryPrivate {
    gchar *name;
    gchar *lang;
    gchar *icon;
    gchar *authors;
    gchar *credits;
    gchar *engine_path;
};
typedef struct _IBusFactoryPrivate IBusFactoryPrivate;

static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_factory_class_init     (IBusFactoryClass   *klass);
static void     ibus_factory_init           (IBusFactory        *factory);
static void     ibus_factory_dispose        (IBusFactory        *factory);
static void     ibus_factory_set_property   (IBusFactory        *factory,
                                             guint              prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void     ibus_factory_get_property   (IBusFactory        *factory,
                                             guint              prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);
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
ibus_factory_new (const gchar *path, const gchar *name, const gchar *lang, const gchar *icon,
    const gchar *authors, const gchar *credits, const gchar *engine_path)
{
    IBusFactory *factory;
    factory = IBUS_FACTORY (g_object_new (IBUS_TYPE_FACTORY,
                "path", path,
                "name", name,
                "lang", lang,
                "icon", icon,
                "authors", authors,
                "credits", credits,
                "engine-path", engine_path,
                NULL));
    return factory;
}

static void
ibus_factory_class_init (IBusFactoryClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    _parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusFactoryPrivate));

    gobject_class->dispose = (GObjectFinalizeFunc) ibus_factory_dispose;
    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_factory_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_factory_get_property;

    IBUS_SERVICE_CLASS (klass)->dbus_message = (ServiceDBusMessageFunc) ibus_factory_dbus_message;

    /* install properties */
    g_object_class_install_property (gobject_class,
                    PROP_NAME,
                    g_param_spec_string ("name",
                        "factory name",
                        "The name of factory object",
                        NULL,
                        G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                    PROP_LANG,
                    g_param_spec_string ("lang",
                        "factory language",
                        "The language of factory object",
                        NULL,
                        G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                    PROP_ICON,
                    g_param_spec_string ("icon",
                        "factory icon",
                        "The icon of factory object",
                        NULL,
                        G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                    PROP_AUTHORS,
                    g_param_spec_string ("authors",
                        "factory authors",
                        "The authors of factory object",
                        NULL,
                        G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                    PROP_CREDITS,
                    g_param_spec_string ("credits",
                        "factory credits",
                        "The credits of factory object",
                        NULL,
                        G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                    PROP_ENGINE_PATH,
                    g_param_spec_string ("engine-path",
                        "engine path",
                        "The path of engine object",
                        NULL,
                        G_PARAM_READWRITE));
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
    priv->engine_path = NULL;
}

static void
ibus_factory_dispose (IBusFactory *factory)
{
    G_OBJECT_CLASS(_parent_class)->dispose (G_OBJECT (factory));
}

static void
ibus_factory_set_property (IBusFactory *factory,
    guint prop_id, const GValue *value, GParamSpec *pspec)
{
    DECLARE_PRIV;

    switch (prop_id) {
    case PROP_NAME:
        if (priv->name == NULL)
            priv->name = g_strdup (g_value_get_string (value));
        break;
    case PROP_LANG:
        if (priv->lang == NULL)
            priv->lang = g_strdup (g_value_get_string (value));
        break;
    case PROP_ICON:
        if (priv->icon == NULL)
            priv->icon = g_strdup (g_value_get_string (value));
        break;
    case PROP_AUTHORS:
        if (priv->authors == NULL)
            priv->authors = g_strdup (g_value_get_string (value));
        break;
    case PROP_CREDITS:
        if (priv->credits == NULL)
            priv->credits = g_strdup (g_value_get_string (value));
        break;
    case PROP_ENGINE_PATH:
        if (priv->engine_path == NULL)
            priv->engine_path = g_strdup (g_value_get_string (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (factory, prop_id, pspec);
    }
}

static void
ibus_factory_get_property (IBusFactory *factory,
    guint prop_id, GValue *value, GParamSpec *pspec)
{
    DECLARE_PRIV;

    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string (value, priv->name ? priv->name : "");
        break;
    case PROP_LANG:
        g_value_set_string (value, priv->lang ? priv->lang : "");
        break;
    case PROP_ICON:
        g_value_set_string (value, priv->icon ? priv->icon : "");
        break;
    case PROP_AUTHORS:
        g_value_set_string (value, priv->authors ? priv->authors : "");
        break;
    case PROP_CREDITS:
        g_value_set_string (value, priv->credits ? priv->credits : "");
        break;
    case PROP_ENGINE_PATH:
        g_value_set_string (value, priv->engine_path ? priv->engine_path : "");
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (factory, prop_id, pspec);
    }
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
