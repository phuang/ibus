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

#include "ibusservice.h"
#include "ibusinternel.h"

#define IBUS_SERVICE_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_SERVICE, IBusServicePrivate))

enum {
    DBUS_MESSAGE,
    DBUS_SIGNAL,
    LAST_SIGNAL,
};


/* IBusServicePriv */
struct _IBusServicePrivate {
};
typedef struct _IBusServicePrivate IBusServicePrivate;

static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_service_class_init     (IBusServiceClass   *klass);
static void     ibus_service_init           (IBusService        *service);
static void     ibus_service_finalize       (IBusService        *service);
static gboolean ibus_service_dbus_message   (IBusService        *service,
                                             IBusConnection     *connection,
                                             DBusMessage        *message);
static gboolean ibus_service_dbus_signal    (IBusService        *service,
                                             IBusConnection     *connection,
                                             DBusMessage        *message);

static IBusObjectClass  *_parent_class = NULL;

GType
ibus_service_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusServiceClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_service_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusService),
        0,
        (GInstanceInitFunc) ibus_service_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "IBusService",
                    &type_info,
                    (GTypeFlags) 0);
    }
    return type;
}

IBusService *
ibus_service_new (void)
{
    return IBUS_SERVICE (g_object_new (IBUS_TYPE_SERVICE, NULL));
}

static void
ibus_service_class_init (IBusServiceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    _parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusServicePrivate));

    gobject_class->finalize = (GObjectFinalizeFunc) ibus_service_finalize;

    klass->dbus_message = ibus_service_dbus_message;
    klass->dbus_signal = ibus_service_dbus_signal;

    _signals[DBUS_MESSAGE] =
        g_signal_new (I_("dbus-message"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusServiceClass, dbus_message),
            NULL, NULL,
            ibus_marshal_BOOLEAN__POINTER_POINTER,
            G_TYPE_BOOLEAN, 2,
            G_TYPE_POINTER, G_TYPE_POINTER);

    _signals[DBUS_SIGNAL] =
        g_signal_new (I_("dbus-signal"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusServiceClass, dbus_signal),
            NULL, NULL,
            ibus_marshal_BOOLEAN__POINTER_POINTER,
            G_TYPE_BOOLEAN, 2,
            G_TYPE_POINTER, G_TYPE_POINTER);

}

static void
ibus_service_init (IBusService *service)
{
    // IBusServicePrivate *priv = IBUS_SERVICE_GET_PRIVATE (service);
}

static void
ibus_service_finalize (IBusService *service)
{
    G_OBJECT_CLASS(_parent_class)->finalize (G_OBJECT (service));
}

gboolean
ibus_service_handle_message (IBusService *service, IBusConnection *connection, DBusMessage *message)
{
    gboolean retval = FALSE;
    g_return_val_if_fail (message != NULL, FALSE);

    g_signal_emit (service, _signals[DBUS_MESSAGE], 0, connection, message, &retval);
    return retval ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static gboolean
ibus_service_dbus_message (IBusService *service, IBusConnection *connection, DBusMessage *message)
{
    return FALSE;
}

static gboolean
ibus_service_dbus_signal (IBusService *service, IBusConnection *connection, DBusMessage *message)
{
    return FALSE;
}
