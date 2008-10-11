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

#include "ibusengine.h"
#include "ibusinternel.h"

#define IBUS_ENGINE_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_ENGINE, IBusEnginePrivate))
#define DECLARE_PRIV IBusEnginePrivate *priv = IBUS_ENGINE_GET_PRIVATE(engine)

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


/* IBusEnginePriv */
struct _IBusEnginePrivate {
    gchar *name;
    gchar *lang;
    gchar *icon;
    gchar *authors;
    gchar *credits;
    gchar *engine_path;
};
typedef struct _IBusEnginePrivate IBusEnginePrivate;

static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_engine_class_init     (IBusEngineClass   *klass);
static void     ibus_engine_init           (IBusEngine        *engine);
static void     ibus_engine_finalize       (IBusEngine        *engine);
static void     ibus_engine_set_property   (IBusEngine        *engine,
                                             guint              prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void     ibus_engine_get_property   (IBusEngine        *engine,
                                             guint              prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);
static gboolean ibus_engine_dbus_message   (IBusEngine        *engine,
                                             IBusConnection     *connection,
                                             DBusMessage        *message);

static IBusObjectClass  *_parent_class = NULL;

GType
ibus_engine_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusEngineClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_engine_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusEngine),
        0,
        (GInstanceInitFunc) ibus_engine_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERVICE,
                    "IBusEngine",
                    &type_info,
                    (GTypeFlags) 0);
    }
    return type;
}

IBusEngine *
ibus_engine_new (const gchar *path)
{
    IBusEngine *engine;
    engine = IBUS_ENGINE (g_object_new (IBUS_TYPE_ENGINE,
                "path", path,
                NULL));
    return engine;
}

static void
ibus_engine_class_init (IBusEngineClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    _parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusEnginePrivate));

    gobject_class->finalize = (GObjectFinalizeFunc) ibus_engine_finalize;
    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_engine_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_engine_get_property;

    IBUS_SERVICE_CLASS (klass)->dbus_message = (ServiceDBusMessageFunc) ibus_engine_dbus_message;

    /* install properties */
    g_object_class_install_property (gobject_class,
                    PROP_NAME,
                    g_param_spec_string ("name",
                        "engine name",
                        "The name of engine object",
                        NULL,
                        G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                    PROP_LANG,
                    g_param_spec_string ("lang",
                        "engine language",
                        "The language of engine object",
                        NULL,
                        G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                    PROP_ICON,
                    g_param_spec_string ("icon",
                        "engine icon",
                        "The icon of engine object",
                        NULL,
                        G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                    PROP_AUTHORS,
                    g_param_spec_string ("authors",
                        "engine authors",
                        "The authors of engine object",
                        NULL,
                        G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                    PROP_CREDITS,
                    g_param_spec_string ("credits",
                        "engine credits",
                        "The credits of engine object",
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
ibus_engine_init (IBusEngine *engine)
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
ibus_engine_finalize (IBusEngine *engine)
{
    G_OBJECT_CLASS(_parent_class)->finalize (G_OBJECT (engine));
}

static void
ibus_engine_set_property (IBusEngine *engine,
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
        G_OBJECT_WARN_INVALID_PROPERTY_ID (engine, prop_id, pspec);
    }
}

static void
ibus_engine_get_property (IBusEngine *engine,
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
        G_OBJECT_WARN_INVALID_PROPERTY_ID (engine, prop_id, pspec);
    }
}

gboolean
ibus_engine_handle_message (IBusEngine *engine, IBusConnection *connection, DBusMessage *message)
{
    gboolean retval = FALSE;
    g_return_val_if_fail (message != NULL, FALSE);

    g_signal_emit (engine, _signals[DBUS_MESSAGE], 0, connection, message, &retval);
    return retval ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static gboolean
ibus_engine_dbus_message (IBusEngine *engine, IBusConnection *connection, DBusMessage *message)
{
    return FALSE;
}
