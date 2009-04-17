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

#include "ibusinternal.h"
#include "ibusmarshalers.h"
#include "ibusshare.h"
#include "ibusconfig.h"

#define IBUS_CONFIG_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_CONFIG, IBusConfigPrivate))

enum {
    VALUE_CHANGED,
    LAST_SIGNAL,
};


/* IBusConfigPriv */
struct _IBusConfigPrivate {
    gpointer pad;
};
typedef struct _IBusConfigPrivate IBusConfigPrivate;

#if 0
struct _BusPair {
    GValue car;
    GValue cdr;
};
typedef struct _BusPair BusPair;
#endif

static guint    config_signals[LAST_SIGNAL] = { 0 };

#if 0
/* functions prototype */
static BusPair  *bus_pair_new                   (GType                  car_type,
                                                 GType                  cdr_type,
                                                 gpointer               car,
                                                 gpointer               cdr);
static BusPair  *bus_pair_copy                  (BusPair                *pair);
static void      bus_pair_free                  (BusPair                *pair);
#endif
static void      ibus_config_class_init    (IBusConfigClass    *klass);
static void      ibus_config_init          (IBusConfig         *config);
static void      ibus_config_real_destroy  (IBusConfig         *config);

static gboolean ibus_config_ibus_signal    (IBusProxy           *proxy,
                                            IBusMessage         *message);

static IBusProxyClass  *parent_class = NULL;

GType
ibus_config_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusConfigClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_config_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusConfig),
        0,
        (GInstanceInitFunc) ibus_config_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_PROXY,
                    "IBusConfig",
                    &type_info,
                    (GTypeFlags)0);
    }
    return type;
}

IBusConfig *
ibus_config_new (IBusConnection *connection)
{
    g_assert (IBUS_IS_CONNECTION (connection));

    GObject *obj;
    obj = g_object_new (IBUS_TYPE_CONFIG,
                        "name", IBUS_SERVICE_CONFIG,
                        "path", IBUS_PATH_CONFIG,
                        "connection", connection,
                        NULL);

    return IBUS_CONFIG (obj);
}

static void
ibus_config_class_init (IBusConfigClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusProxyClass *proxy_class = IBUS_PROXY_CLASS (klass);


    parent_class = (IBusProxyClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusConfigPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_config_real_destroy;

    proxy_class->ibus_signal = ibus_config_ibus_signal;

    /* install signals */
    /**
     * IBusConfig:value-changed:
     *
     * Emitted when configuration value is changed.
     */
    config_signals[VALUE_CHANGED] =
        g_signal_new (I_("value-changed"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING_STRING_BOXED,
            G_TYPE_NONE,
            3,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_VALUE | G_SIGNAL_TYPE_STATIC_SCOPE);
}

static void
ibus_config_init (IBusConfig *config)
{
    IBusConfigPrivate *priv;
    priv = IBUS_CONFIG_GET_PRIVATE (config);
}

static void
ibus_config_real_destroy (IBusConfig *config)
{
    if (ibus_proxy_get_connection ((IBusProxy *) config) != NULL) {
        ibus_proxy_call ((IBusProxy *) config,
                         "Destroy",
                         G_TYPE_INVALID);
    }

    IBUS_OBJECT_CLASS(parent_class)->destroy (IBUS_OBJECT (config));
}


static gboolean
ibus_config_ibus_signal (IBusProxy     *proxy,
                         IBusMessage   *message)
{
    g_assert (IBUS_IS_CONFIG (proxy));
    g_assert (message != NULL);

    IBusConfig *config;
    config = IBUS_CONFIG (proxy);

    if (ibus_message_is_signal (message, IBUS_INTERFACE_CONFIG, "ValueChanged")) {
        gchar *section;
        gchar *name;
        GValue value = { 0 };
        IBusError *error = NULL;
        gboolean retval;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &section,
                                        G_TYPE_STRING, &name,
                                        G_TYPE_VALUE, &value,
                                        G_TYPE_INVALID);
        if (!retval) {
            g_warning ("%s: Can not parse arguments of ValueChanges.", DBUS_ERROR_INVALID_ARGS);
            return FALSE;
        }

        g_signal_emit (config,
                       config_signals[VALUE_CHANGED],
                       0,
                       section,
                       name,
                       &value);
        g_value_unset (&value);

        g_signal_stop_emission_by_name (config, "ibus-signal");

        return TRUE;
    }

    return FALSE;
}

gboolean
ibus_config_get_value (IBusConfig  *config,
                       const gchar *section,
                       const gchar *name,
                       GValue      *value)
{
    g_assert (IBUS_IS_CONFIG (config));
    g_assert (section != NULL);
    g_assert (name != NULL);
    g_assert (value != NULL);

    IBusMessage *reply;
    IBusError *error;
    gboolean retval;

    reply = ibus_proxy_call_with_reply_and_block ((IBusProxy *) config,
                                                  "GetValue",
                                                  -1,
                                                  &error,
                                                  G_TYPE_STRING, &section,
                                                  G_TYPE_STRING, &name,
                                                  G_TYPE_INVALID);
    if (reply == NULL) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        return FALSE;
    }

    if ((error = ibus_error_new_from_message (reply)) != NULL) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        ibus_message_unref (reply);
        return FALSE;
    }

    retval = ibus_message_get_args (reply,
                                    &error,
                                    G_TYPE_VALUE, value,
                                    G_TYPE_INVALID);
    ibus_message_unref (reply);
    if (!retval) {
        g_warning ("%s: %s", error->name, error->message);
        return FALSE;
    }

    return TRUE;
}

gboolean
ibus_config_set_value (IBusConfig   *config,
                       const gchar  *section,
                       const gchar  *name,
                       const GValue *value)
{
    g_assert (IBUS_IS_CONFIG (config));
    g_assert (section != NULL);
    g_assert (name != NULL);
    g_assert (value != NULL);

    gboolean retval;

    retval = ibus_proxy_call ((IBusProxy *) config,
                              "SetValue",
                              G_TYPE_STRING, &section,
                              G_TYPE_STRING, &name,
                              G_TYPE_VALUE, value,
                              G_TYPE_INVALID);
    g_assert (retval);
    return TRUE;
}
