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

#include <gio/gio.h>
#include "ibusbus.h"
#include "ibusinternal.h"
#include "ibusshare.h"
#include "ibusconnection.h"

#define IBUS_BUS_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_BUS, IBusBusPrivate))

enum {
    CONNECTED,
    DISCONNECT,
    LAST_SIGNAL,
};


/* IBusBusPriv */
struct _IBusBusPrivate {
    GFile *file;
    GFileMonitor *monitor;
    IBusConnection *connection;
};
typedef struct _IBusBusPrivate IBusBusPrivate;

static guint    bus_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_bus_class_init     (IBusBusClass   *klass);
static void     ibus_bus_init           (IBusBus        *bus);
static void     ibus_bus_finalize       (IBusBus        *bus);

static IBusObjectClass  *parent_class = NULL;

GType
ibus_bus_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusBusClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_bus_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusBus),
        0,
        (GInstanceInitFunc) ibus_bus_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "IBusBus",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}

IBusBus *
ibus_bus_new (void)
{
    IBusBus *bus = IBUS_BUS (g_object_new (IBUS_TYPE_BUS, NULL));

    return bus;
}

static void
ibus_bus_class_init (IBusBusClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusBusPrivate));
    
    // install signals
    bus_signals[CONNECTED] =
        g_signal_new (I_("connected"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    bus_signals[CONNECTED] =
        g_signal_new (I_("disconnected"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

}

void
_changed_cb (GFileMonitor       *monitor,
             GFile              *file,
             GFile              *other_file,
             GFileMonitorEvent   event_type,
             IBusBus            *bus)
{
    g_warning ("changed");
}

static void
ibus_bus_init (IBusBus *bus)
{
    gchar *path;
    IBusBusPrivate *priv;
    priv = IBUS_BUS_GET_PRIVATE (bus);

    path = g_strdup_printf ("/tmp/ibus-%s/", ibus_get_user_name ());
    priv->file = g_file_new_for_path ("/tmp/ibus-%s");
    priv->monitor = g_file_monitor_directory (priv->file, 0xff, NULL, NULL);

    g_signal_connect (priv->monitor, "changed", (GCallback) _changed_cb, NULL);

    g_free (path);
}
