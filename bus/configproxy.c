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
#include "configproxy.h"

#define BUS_CONFIG_PROXY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_CONFIG_PROXY, BusConfigProxyPrivate))

enum {
    VALUE_CHANGED,
    LAST_SIGNAL,
};


/* BusConfigProxyPriv */
struct _BusConfigProxyPrivate {
    gpointer pad;
};
typedef struct _BusConfigProxyPrivate BusConfigProxyPrivate;

struct _BusPair {
    GValue car;
    GValue cdr;
};
typedef struct _BusPair BusPair;

static guint    config_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static BusPair  *bus_pair_new                   (GType                  car_type,
                                                 GType                  cdr_type,
                                                 gpointer               car,
                                                 gpointer               cdr);
static BusPair  *bus_pair_copy                  (BusPair                *pair);
static void      bus_pair_free                  (BusPair                *pair);
static void      bus_config_proxy_class_init    (BusConfigProxyClass    *klass);
static void      bus_config_proxy_init          (BusConfigProxy         *config);
static void      bus_config_proxy_real_destroy  (BusConfigProxy         *config);

static gboolean bus_config_proxy_dbus_signal    (IBusProxy             *proxy,
                                                 DBusMessage            *message);

static IBusProxyClass  *parent_class = NULL;

static BusPair *
bus_pair_new (GType     car_type,
              GType     cdr_type,
              gpointer  car,
              gpointer  cdr)
{

    g_assert (car_type == G_TYPE_STRING ||
              car_type == G_TYPE_INT ||
              car_type == G_TYPE_BOOLEAN ||
              car_type == G_TYPE_DOUBLE);
    g_assert (cdr_type == G_TYPE_STRING ||
              cdr_type == G_TYPE_INT ||
              cdr_type == G_TYPE_BOOLEAN ||
              cdr_type == G_TYPE_DOUBLE);
    g_assert (car != NULL);
    g_assert (cdr != NULL);

    BusPair *pair;

    pair = g_slice_new0 (BusPair);

    g_value_init (&(pair->car), car_type);
    g_value_init (&(pair->cdr), cdr_type);

    switch (car_type) {
    case G_TYPE_STRING:
        g_value_set_string (&(pair->car), *(gchar **)car);
        break;
    case G_TYPE_INT:
        g_value_set_int (&(pair->car), *(gint32 *)car);
        break;
    case G_TYPE_BOOLEAN:
        g_value_set_boolean (&(pair->car), *(gboolean *)car);
        break;
    case G_TYPE_DOUBLE:
        g_value_set_double (&(pair->car), *(gdouble *)car);
        break;
    }
    
    switch (cdr_type) {
    case G_TYPE_STRING:
        g_value_set_string (&(pair->cdr), *(gchar **)cdr);
        break;
    case G_TYPE_INT:
        g_value_set_int (&(pair->cdr), *(gint32 *)car);
        break;
    case G_TYPE_BOOLEAN:
        g_value_set_boolean (&(pair->cdr), *(gboolean *)cdr);
        break;
    case G_TYPE_DOUBLE:
        g_value_set_double (&(pair->cdr), *(gdouble *)cdr);
        break;
    }
    return pair;
}

static BusPair *
bus_pair_copy (BusPair *pair)
{
    g_assert (pair != NULL);
    
    BusPair *new_pair;

    new_pair = g_slice_new0 (BusPair);
    g_value_copy (&(pair->car), &(new_pair->car));
    g_value_copy (&(pair->cdr), &(new_pair->cdr));
    return new_pair;
}

static void
bus_pair_free (BusPair *pair)
{
    g_assert (pair == NULL);
    
    g_value_unset (&(pair->car));
    g_value_unset (&(pair->cdr));

    g_slice_free (BusPair, pair);
}

GType
bus_config_proxy_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusConfigProxyClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_config_proxy_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusConfigProxy),
        0,
        (GInstanceInitFunc) bus_config_proxy_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_PROXY,
                    "BusConfigProxy",
                    &type_info,
                    (GTypeFlags)0);
    }
    return type;
}

BusConfigProxy *
bus_config_proxy_new (BusConnection *connection)
{
    g_assert (BUS_IS_CONNECTION (connection));
    
    GObject *obj;
    obj = g_object_new (BUS_TYPE_CONFIG_PROXY,
                        "name", NULL,
                        "path", IBUS_PATH_CONFIG,
                        "connection", connection,
                        NULL);

    return BUS_CONFIG_PROXY (obj);
}

static void
bus_config_proxy_class_init (BusConfigProxyClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusProxyClass *proxy_class = IBUS_PROXY_CLASS (klass);


    parent_class = (IBusProxyClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusConfigProxyPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_config_proxy_real_destroy;

    proxy_class->dbus_signal = bus_config_proxy_dbus_signal;
    
    /* install signals */
    config_signals[VALUE_CHANGED] =
        g_signal_new (I_("value-changed"),
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL, NULL,
            ibus_marshal_VOID__STRING_STRING_BOXED,
            G_TYPE_NONE, 3,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_VALUE);
}

static void
bus_config_proxy_init (BusConfigProxy *config)
{
    BusConfigProxyPrivate *priv;
    priv = BUS_CONFIG_PROXY_GET_PRIVATE (config);
}

static void
bus_config_proxy_real_destroy (BusConfigProxy *config)
{
    BusConfigProxyPrivate *priv;
    priv = BUS_CONFIG_PROXY_GET_PRIVATE (config);
    
    ibus_proxy_call (IBUS_PROXY (config),
                     "Destroy",
                     DBUS_TYPE_INVALID);

    IBUS_OBJECT_CLASS(parent_class)->destroy (IBUS_OBJECT (config));
}

static void
_from_dbus_value (DBusMessageIter *iter, GValue *value)
{
    gint type;

    type = dbus_message_iter_get_arg_type (iter);
    
    switch (type) {
    case DBUS_TYPE_STRING:
        {
            gchar *v;
            g_value_init (value, G_TYPE_STRING);
            dbus_message_iter_get_basic (iter, &v);
            g_value_set_string (value, v);
        }
        break;
    case DBUS_TYPE_INT32:
        {
            gint v;
            g_value_init (value, G_TYPE_INT);
            dbus_message_iter_get_basic (iter, &v);
            g_value_set_int (value, v);
        }
        break;
    case DBUS_TYPE_BOOLEAN:
        {
            gboolean v;
            g_value_init (value, G_TYPE_BOOLEAN);
            dbus_message_iter_get_basic (iter, &v);
            g_value_set_boolean (value, v);
        }
        break;
    case DBUS_TYPE_DOUBLE:
        {
            gdouble v;
            g_value_init (value, G_TYPE_DOUBLE);
            dbus_message_iter_get_basic (iter, &v);
            g_value_set_double (value, v);
        }
        break;
    case DBUS_TYPE_ARRAY:
        {
            GValue v = { 0 };
            DBusMessageIter sub_iter;
            gint sub_type;
            GValueArray *array;

            
            sub_type = dbus_message_iter_get_element_type (iter);
            g_assert (sub_type == DBUS_TYPE_STRING ||
                      sub_type == DBUS_TYPE_INT32 ||
                      sub_type == DBUS_TYPE_BOOLEAN ||
                      sub_type == DBUS_TYPE_DOUBLE);
            
            g_value_init (value, G_TYPE_VALUE_ARRAY);
            array = g_value_array_new (0);            
            dbus_message_iter_recurse (iter, &sub_iter);
            while (dbus_message_iter_get_arg_type (&sub_iter) != DBUS_TYPE_INVALID) {
                _from_dbus_value (&sub_iter, &v);
                g_value_array_append (array, &v);
                dbus_message_iter_next (&sub_iter);
            }
            g_value_take_boxed (value, array);
            break;
        }
    default:
        g_assert_not_reached();
    }
}

static gboolean
bus_config_proxy_dbus_signal (IBusProxy     *proxy,
                              DBusMessage   *message)
{
    g_assert (BUS_IS_CONFIG_PROXY (proxy));
    g_assert (message != NULL);
    
    BusConfigProxy *config;
    config = BUS_CONFIG_PROXY (proxy);
    
    dbus_error_init (&error);
    if (dbus_message_is_signal (message, IBUS_INTERFACE_CONFIG, "ValueChanged")) {
        DBusMessageIter iter;
        gchar *section;
        gchar *name;
        GValue value = { 0 };
        gint type;

        dbus_message_iter_init (message, &iter);
        
        type = dbus_message_iter_get_arg_type (&iter);
        if (type != DBUS_TYPE_STRING) {
            g_warning ("Argument 1 of ValueChanged should be a string");
            return FALSE;
        }
        dbus_message_iter_get_basic (&iter, &section);
        
        if (type != DBUS_TYPE_STRING) {
            g_warning ("Argument 2 of ValueChanged should be a string");
            return FALSE;
        }
        dbus_message_iter_get_basic (&iter, &name);

        _from_dbus_value (&iter, &value);

        g_signal_emit (panel, config_signals[VALUE_CHANGED], 0, section, name, value);
    }
    
    return FALSE;

handled:
    g_signal_stop_emission_by_name (panel, "dbus-signal");
    return TRUE;
}


void
bus_config_proxy_focus_in (BusConfigProxy     *panel,
                          BusInputContext   *context)
{
    g_assert (BUS_IS_CONFIG_PROXY (panel));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    BusConfigProxyPrivate *priv;
    priv = BUS_CONFIG_PROXY_GET_PRIVATE (panel);

    if (priv->focused_context == context)
        return;

    if (priv->focused_context != NULL)
        bus_config_proxy_focus_out (panel, context);

    g_object_ref (context);
    priv->focused_context = context;
}

void
bus_config_proxy_focus_out (BusConfigProxy    *panel,
                           BusInputContext  *context)
{
    g_assert (BUS_IS_CONFIG_PROXY (panel));
    g_assert (BUS_IS_INPUT_CONTEXT (context));
    
    BusConfigProxyPrivate *priv;
    priv = BUS_CONFIG_PROXY_GET_PRIVATE (panel);

    g_assert (priv->focused_context == context);
    
    g_object_unref (priv->focused_context);
    priv->focused_context = NULL;
}

void
bus_config_proxy_set_cursor_location (BusConfigProxy *panel,
                                     gint32         x,
                                     gint32         y,
                                     gint32         w,
                                     gint32         h)
{
    g_assert (BUS_IS_CONFIG_PROXY (panel));

    ibus_proxy_call (IBUS_PROXY (panel),
                     "SetCursorLocation",
                     DBUS_TYPE_INT32, &x,
                     DBUS_TYPE_INT32, &y,
                     DBUS_TYPE_INT32, &w,
                     DBUS_TYPE_INT32, &h,
                     DBUS_TYPE_INVALID);
}

void
bus_config_proxy_update_preedit (BusConfigProxy   *panel,
                                const gchar     *text,
                                IBusAttrList    *attr_list,
                                gint             cursor_pos,
                                gboolean         visible)
{
    g_assert (BUS_IS_CONFIG_PROXY (panel));
    g_assert (text != NULL);
    g_assert (attr_list != NULL);

    DBusMessage *message;
    DBusMessageIter iter;

    message = dbus_message_new_method_call (
                                ibus_proxy_get_name (IBUS_PROXY (panel)),
                                ibus_proxy_get_path (IBUS_PROXY (panel)),
                                ibus_proxy_get_interface (IBUS_PROXY (panel)),
                                "UpdatePreedit");
    
    dbus_message_iter_init_append (message, &iter);

    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &text);
    ibus_attr_list_to_dbus_message (attr_list, &iter);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &cursor_pos);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &visible);

    ibus_proxy_send (IBUS_PROXY (panel), message);

    dbus_message_unref (message);
}

void
bus_config_proxy_update_aux_string (BusConfigProxy *panel,
                                   const gchar   *text,
                                   IBusAttrList  *attr_list,
                                   gboolean       visible)
{
    g_assert (BUS_IS_CONFIG_PROXY (panel));
    g_assert (text != NULL);
    g_assert (attr_list != NULL);

    DBusMessage *message;
    DBusMessageIter iter;

    message = dbus_message_new_method_call (
                                ibus_proxy_get_name (IBUS_PROXY (panel)),
                                ibus_proxy_get_path (IBUS_PROXY (panel)),
                                ibus_proxy_get_interface (IBUS_PROXY (panel)),
                                "UpdateAuxString");
    
    dbus_message_iter_init_append (message, &iter);

    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &text);
    ibus_attr_list_to_dbus_message (attr_list, &iter);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &visible);

    ibus_proxy_send (IBUS_PROXY (panel), message);

    dbus_message_unref (message);

}

void
bus_config_proxy_update_lookup_table (BusConfigProxy   *panel,
                                     IBusLookupTable *table,
                                     gboolean         visible)
{
    g_assert (BUS_IS_CONFIG_PROXY (panel));
    g_assert (table != NULL);

    DBusMessage *message;
    DBusMessageIter iter;

    message = dbus_message_new_method_call (
                                ibus_proxy_get_name (IBUS_PROXY (panel)),
                                ibus_proxy_get_path (IBUS_PROXY (panel)),
                                ibus_proxy_get_interface (IBUS_PROXY (panel)),
                                "UpdateLookupTable");
    
    dbus_message_iter_init_append (message, &iter);

    ibus_lookup_table_to_dbus_message (table, &iter);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &visible);

    ibus_proxy_send (IBUS_PROXY (panel), message);

    dbus_message_unref (message);
}

void
bus_config_proxy_register_properties (BusConfigProxy  *panel,
                                     IBusPropList   *prop_list)
{
    g_assert (BUS_IS_CONFIG_PROXY (panel));
    g_assert (prop_list != NULL);

    DBusMessage *message;
    DBusMessageIter iter;

    message = dbus_message_new_method_call (
                                ibus_proxy_get_name (IBUS_PROXY (panel)),
                                ibus_proxy_get_path (IBUS_PROXY (panel)),
                                ibus_proxy_get_interface (IBUS_PROXY (panel)),
                                "RegisterProperties");
    
    dbus_message_iter_init_append (message, &iter);

    ibus_prop_list_to_dbus_message (prop_list, &iter);

    ibus_proxy_send (IBUS_PROXY (panel), message);

    dbus_message_unref (message);
}

void
bus_config_proxy_update_property (BusConfigProxy  *panel,
                                 IBusProperty   *prop)
{
    g_assert (BUS_IS_CONFIG_PROXY (panel));
    g_assert (prop != NULL);

    DBusMessage *message;
    DBusMessageIter iter;

    message = dbus_message_new_method_call (
                                ibus_proxy_get_name (IBUS_PROXY (panel)),
                                ibus_proxy_get_path (IBUS_PROXY (panel)),
                                ibus_proxy_get_interface (IBUS_PROXY (panel)),
                                "UpdateProperty");
    
    dbus_message_iter_init_append (message, &iter);

    ibus_property_to_dbus_message (prop, &iter);

    ibus_proxy_send (IBUS_PROXY (panel), message);

    dbus_message_unref (message);
}

#define DEFINE_FUNCTION(Name, name)                     \
    void bus_config_proxy_##name (BusConfigProxy *panel)  \
    {                                                   \
        g_assert (BUS_IS_CONFIG_PROXY (panel));          \
        ibus_proxy_call (IBUS_PROXY (panel),            \
                     #Name,                             \
                     DBUS_TYPE_INVALID);                \
    }

DEFINE_FUNCTION (ShowPreedit, show_preedit)
DEFINE_FUNCTION (HidePreedit, hide_preedit)
DEFINE_FUNCTION (ShowAuxString, show_aux_string)
DEFINE_FUNCTION (HideAuxString, hide_aux_string)
DEFINE_FUNCTION (ShowLookupTable, show_lookup_table)
DEFINE_FUNCTION (HideLookupTable, hide_lookup_table)
DEFINE_FUNCTION (PageUpLookupTable, page_up_lookup_table)
DEFINE_FUNCTION (PageDownLookupTable, page_down_lookup_table)
DEFINE_FUNCTION (CursorUpLookupTable, cursor_up_lookup_table)
DEFINE_FUNCTION (CursorDownLookupTable, cursor_down_lookup_table)

#undef DEFINE_FUNCTION
