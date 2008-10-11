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

enum {
    DBUS_MESSAGE,
    LAST_SIGNAL,
};

enum {
    PROP_0,
    PROP_CONNECTION,
};


/* IBusEnginePriv */
struct _IBusEnginePrivate {
    IBusConnection *connection;
};
typedef struct _IBusEnginePrivate IBusEnginePrivate;

static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_engine_class_init      (IBusEngineClass   *klass);
static void     ibus_engine_init            (IBusEngine        *engine);
static void     ibus_engine_finalize        (IBusEngine        *engine);
static void     ibus_engine_set_property    (IBusEngine        *engine,
                                             guint              prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void     ibus_engine_get_property    (IBusEngine        *engine,
                                             guint              prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);
static gboolean ibus_engine_dbus_message    (IBusEngine        *engine,
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
ibus_engine_new (const gchar *path, IBusConnection *connection)
{
    g_assert (path);
    g_assert (IBUS_IS_CONNECTION (connection));
    
    IBusEnginePrivate *priv;
    IBusEngine *engine;

    engine = IBUS_ENGINE (g_object_new (IBUS_TYPE_ENGINE,
                "path", path,
                NULL));
    
    priv = IBUS_ENGINE_GET_PRIVATE (engine);
    priv->connection = g_object_ref (connection);
    
    ibus_service_add_to_connection (IBUS_SERVICE (engine), connection);

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
                    PROP_CONNECTION,
                    g_param_spec_object ("connection",
                        "connection",
                        "The connection of engine object",
                        IBUS_TYPE_CONNECTION,
                        G_PARAM_READABLE));
}

static void
ibus_engine_init (IBusEngine *engine)
{
    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);
    
    priv->connection = NULL;
}

static void
ibus_engine_finalize (IBusEngine *engine)
{
    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);
    g_object_unref (priv->connection);
    G_OBJECT_CLASS(_parent_class)->finalize (G_OBJECT (engine));
}

static void
ibus_engine_set_property (IBusEngine *engine,
    guint prop_id, const GValue *value, GParamSpec *pspec)
{
    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (engine, prop_id, pspec);
    }
}

static void
ibus_engine_get_property (IBusEngine *engine,
    guint prop_id, GValue *value, GParamSpec *pspec)
{
    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    switch (prop_id) {
    case PROP_CONNECTION:
        g_value_set_object (value, priv->connection);
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
