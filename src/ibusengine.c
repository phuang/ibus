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
#include "ibusinternal.h"
#include "ibusshare.h"

#define IBUS_ENGINE_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_ENGINE, IBusEnginePrivate))

enum {
    KEY_PRESS,
    FOCUS_IN,
    FOCUS_OUT,
    RESET,
    ENABLE,
    DISABLE,
    SET_CURSOR_LOCATION,
    PAGE_UP,
    PAGE_DOWN,
    CURSOR_UP,
    CURSOR_DOWN,
    PROPERTY_ACTIVATE,
    PROPERTY_SHOW,
    PROPERTY_HIDE,
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

static guint            engine_signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     ibus_engine_class_init      (IBusEngineClass    *klass);
static void     ibus_engine_init            (IBusEngine         *engine);
static void     ibus_engine_destroy         (IBusEngine         *engine);
static void     ibus_engine_set_property    (IBusEngine         *engine,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void     ibus_engine_get_property    (IBusEngine         *engine,
                                             guint               prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);
static gboolean ibus_engine_dbus_message    (IBusEngine         *engine,
                                             IBusConnection     *connection,
                                             DBusMessage        *message);
static gboolean ibus_engine_key_press       (IBusEngine         *engine,
                                             guint               keyval,
                                             gboolean            is_press,
                                             guint               state);
static void     ibus_engine_focus_in        (IBusEngine         *engine);
static void     ibus_engine_focus_out       (IBusEngine         *engine);
static void     ibus_engine_reset           (IBusEngine         *engine);
static void     ibus_engine_enable          (IBusEngine         *engine);
static void     ibus_engine_disable         (IBusEngine         *engine);
static void     ibus_engine_set_cursor_location
                                            (IBusEngine         *engine,
                                             gint                x,
                                             gint                y,
                                             gint                w,
                                             gint                h);
static void     ibus_engine_page_up         (IBusEngine         *engine);
static void     ibus_engine_page_down       (IBusEngine         *engine);
static void     ibus_engine_cursor_up       (IBusEngine         *engine);
static void     ibus_engine_cursor_down     (IBusEngine         *engine);
static void     ibus_engine_property_activate
                                            (IBusEngine         *engine,
                                             const gchar        *prop_name,
                                             gint                prop_state);
static void     ibus_engine_property_show   (IBusEngine         *engine,
                                             const gchar        *prop_name);
static void     ibus_engine_property_hide   (IBusEngine         *engine,
                                             const gchar        *prop_name);


static IBusServiceClass  *_parent_class = NULL;

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
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    _parent_class = (IBusServiceClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusEnginePrivate));

    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_engine_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_engine_get_property;
    
    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_engine_destroy;

    IBUS_SERVICE_CLASS (klass)->dbus_message = (ServiceDBusMessageFunc) ibus_engine_dbus_message;

    klass->key_press    = ibus_engine_key_press;
    klass->focus_in     = ibus_engine_focus_in;
    klass->focus_out    = ibus_engine_focus_out;
    klass->reset        = ibus_engine_reset;
    klass->enable       = ibus_engine_enable;
    klass->disable      = ibus_engine_disable;
    klass->page_up      = ibus_engine_page_up;
    klass->page_down    = ibus_engine_page_down;
    klass->cursor_up    = ibus_engine_cursor_up;
    klass->cursor_down  = ibus_engine_cursor_down;
    klass->property_activate    = ibus_engine_property_activate;
    klass->property_show        = ibus_engine_property_show;
    klass->property_hide        = ibus_engine_property_hide;
    klass->set_cursor_location  = ibus_engine_set_cursor_location;


    /* install properties */
    g_object_class_install_property (gobject_class,
                    PROP_CONNECTION,
                    g_param_spec_object ("connection",
                        "connection",
                        "The connection of engine object",
                        IBUS_TYPE_CONNECTION,
                        G_PARAM_READABLE));

    /* install signals */
    engine_signals[KEY_PRESS] =
        g_signal_new (I_("key-press"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, key_press),
            NULL, NULL,
            ibus_marshal_BOOL__UINT_BOOL_UINT,
            G_TYPE_BOOLEAN, 3,
            G_TYPE_UINT, G_TYPE_BOOLEAN, G_TYPE_UINT);

    engine_signals[FOCUS_IN] =
        g_signal_new (I_("focus-in"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, focus_in),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    engine_signals[FOCUS_OUT] =
        g_signal_new (I_("focus-out"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, focus_out),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    engine_signals[RESET] =
        g_signal_new (I_("reset"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, reset),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    engine_signals[ENABLE] =
        g_signal_new (I_("enable"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, enable),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    engine_signals[DISABLE] =
        g_signal_new (I_("disable"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, disable),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    engine_signals[SET_CURSOR_LOCATION] =
        g_signal_new (I_("set-cursor-location"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, set_cursor_location),
            NULL, NULL,
            ibus_marshal_VOID__INT_INT_INT_INT,
            G_TYPE_NONE, 4,
            G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);

    engine_signals[PAGE_UP] =
        g_signal_new (I_("page-up"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, page_up),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    engine_signals[PAGE_DOWN] =
        g_signal_new (I_("page-down"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, page_down),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    engine_signals[CURSOR_UP] =
        g_signal_new (I_("cursor-up"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, cursor_up),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    engine_signals[CURSOR_DOWN] =
        g_signal_new (I_("cursor-down"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, cursor_down),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    engine_signals[PROPERTY_ACTIVATE] =
        g_signal_new (I_("property-activate"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, property_activate),
            NULL, NULL,
            ibus_marshal_VOID__STRING_INT,
            G_TYPE_NONE, 0);

    engine_signals[PROPERTY_SHOW] =
        g_signal_new (I_("property-show"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, property_show),
            NULL, NULL,
            ibus_marshal_VOID__STRING,
            G_TYPE_NONE, 0);

    engine_signals[PROPERTY_HIDE] =
        g_signal_new (I_("property-hide"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, property_hide),
            NULL, NULL,
            ibus_marshal_VOID__STRING,
            G_TYPE_NONE, 0);

}

static void
ibus_engine_init (IBusEngine *engine)
{
    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    priv->connection = NULL;
}

static void
ibus_engine_destroy (IBusEngine *engine)
{
    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    if (priv->connection) {
        g_object_unref (priv->connection);
        priv->connection = NULL;
    }

    IBUS_OBJECT_CLASS(_parent_class)->destroy (IBUS_OBJECT (engine));
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

static gboolean
ibus_engine_dbus_message (IBusEngine *engine, IBusConnection *connection, DBusMessage *message)
{
    g_assert (IBUS_IS_ENGINE (engine));
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    g_assert (priv->connection == connection);

    DBusMessage *return_message = NULL;
    DBusMessage *error_message = NULL;

    static struct {
        gchar *member;
        guint  signal_id;
    } no_arg_methods[] = {
        {"FocusIn",     FOCUS_IN},
        {"FocusOut",    FOCUS_OUT},
        {"Reset",       RESET},
        {"Enable",      ENABLE},
        {"Disable",     DISABLE},
        {"PageUp",      PAGE_UP},
        {"PageDown",    PAGE_DOWN},
        {"CursorUp",    CURSOR_UP},
        {"CursorDown",  CURSOR_DOWN},
        {NULL, 0},
    };
    gint i;

    for (i = 0; no_arg_methods[i].member != NULL; i++) {
        if (!dbus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, no_arg_methods[i].member))
            continue;

        DBusMessageIter iter;
        dbus_message_iter_init (message, &iter);
        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID) {
            error_message = dbus_message_new_error_printf (message,
                                "%s.%s: Method does not have arguments",
                                IBUS_INTERFACE_ENGINE, no_arg_methods[i].member);
            ibus_connection_send (connection, error_message);
            dbus_message_unref (error_message);
            return TRUE;
        }

        g_signal_emit (engine, engine_signals[no_arg_methods[i].signal_id], 0);
        return_message = dbus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;
    }


    if (dbus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "KeyPress")) {
        guint keyval, state;
        gboolean is_press, retval;
        DBusMessageIter iter;

        dbus_message_iter_init (message, &iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_UINT32)
            goto _keypress_fail;
        dbus_message_iter_get_basic (&iter, &keyval);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_BOOLEAN)
            goto _keypress_fail;
        dbus_message_iter_get_basic (&iter, &is_press);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_UINT32)
            goto _keypress_fail;
        dbus_message_iter_get_basic (&iter, &state);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID)
            goto _keypress_fail;

        g_signal_emit (engine, engine_signals[KEY_PRESS], 0, keyval, is_press, state, &retval);

        return_message = dbus_message_new_method_return (message);
        dbus_message_append_args (return_message, DBUS_TYPE_BOOLEAN, &retval, DBUS_TYPE_INVALID);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;

    _keypress_fail:
        error_message = dbus_message_new_error_printf (message,
                        "%s.%s: Can not match signature (ubu) of method",
                        IBUS_INTERFACE_ENGINE, "KeyPress");
        ibus_connection_send (connection, error_message);
        dbus_message_unref (error_message);
        return TRUE;
    }
    else if (dbus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "PropertyActivate")) {
        gchar *name;
        gint state;
        DBusMessageIter iter;

        dbus_message_iter_init (message, &iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_STRING)
            goto _property_activate_fail;
        dbus_message_iter_get_basic (&iter, &name);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INT32)
            goto _property_activate_fail;
        dbus_message_iter_get_basic (&iter, &state);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID)
            goto _property_activate_fail;

        g_signal_emit (engine, engine_signals[PROPERTY_ACTIVATE], 0, name, state);

        return_message = dbus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;

    _property_activate_fail:
        error_message = dbus_message_new_error_printf (message,
                        "%s.%s: Can not match signature (si) of method",
                        IBUS_INTERFACE_ENGINE, "PropertyActivate");
        ibus_connection_send (connection, error_message);
        dbus_message_unref (error_message);
        return TRUE;

    }
    else if (dbus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "PropertyShow")) {
        gchar *name;
        DBusMessageIter iter;

        dbus_message_iter_init (message, &iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_STRING)
            goto _property_show_fail;
        dbus_message_iter_get_basic (&iter, &name);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID)
            goto _property_show_fail;

        g_signal_emit (engine, engine_signals[PROPERTY_SHOW], 0, name);

        return_message = dbus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;

    _property_show_fail:
        error_message = dbus_message_new_error_printf (message,
                        "%s.%s: Can not match signature (s) of method",
                        IBUS_INTERFACE_ENGINE, "PropertyShow");
        ibus_connection_send (connection, error_message);
        dbus_message_unref (error_message);
        return TRUE;
    }
    else if (dbus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "PropertyHide")) {
        gchar *name;
        DBusMessageIter iter;

        dbus_message_iter_init (message, &iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_STRING)
            goto _property_hide_fail;
        dbus_message_iter_get_basic (&iter, &name);
        dbus_message_iter_next (&iter);

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID)
            goto _property_hide_fail;

        g_signal_emit (engine, engine_signals[PROPERTY_HIDE], 0, name);

        return_message = dbus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;

    _property_hide_fail:
        error_message = dbus_message_new_error_printf (message,
                        "%s.%s: Can not match signature (s) of method",
                        IBUS_INTERFACE_ENGINE, "PropertyHide");
        ibus_connection_send (connection, error_message);
        dbus_message_unref (error_message);
        return TRUE;
    }
    else if (dbus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "SetCursorLocation")) {
        gint args[4];
        DBusMessageIter iter;

        dbus_message_iter_init (message, &iter);
        for (i =0; i < 4; i++) {
            if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INT32)
                goto _set_cursor_location_fail;
            dbus_message_iter_get_basic (&iter, &args[i]);
            dbus_message_iter_next (&iter);
        }

        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INVALID)
            goto _set_cursor_location_fail;

        g_signal_emit (engine, engine_signals[SET_CURSOR_LOCATION], 0,
                    args[0], args[1], args[2], args[3]);

        return_message = dbus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        dbus_message_unref (return_message);
        return TRUE;

    _set_cursor_location_fail:
        error_message = dbus_message_new_error_printf (message,
                        "%s.%s: Can not match signature (iiii) of method",
                        IBUS_INTERFACE_ENGINE, "SetCursorLocation");
        ibus_connection_send (connection, error_message);
        dbus_message_unref (error_message);
        return TRUE;
    }

    return FALSE;
}

static gboolean
ibus_engine_key_press (IBusEngine *engine, guint keyval, gboolean is_press, guint state)
{
    g_debug ("key-press (%d, %d, %d)", keyval, is_press, state);
    return FALSE;
}

static void
ibus_engine_focus_in (IBusEngine *engine)
{
    g_debug ("focus-in");
}

static void
ibus_engine_focus_out (IBusEngine *engine)
{
    g_debug ("focus-out");
}

static void
ibus_engine_reset (IBusEngine *engine)
{
    g_debug ("reset");
}

static void
ibus_engine_enable (IBusEngine *engine)
{
    g_debug ("enable");
}

static void
ibus_engine_disable (IBusEngine *engine)
{
    g_debug ("disable");
}

static void
ibus_engine_set_cursor_location (IBusEngine *engine,
        gint x, gint y, gint w, gint h)
{
    g_debug ("set-cursor-location (%d, %d, %d, %d)", x, y, w, h);
}

static void
ibus_engine_page_up (IBusEngine *engine)
{
    g_debug ("page-up");
}

static void
ibus_engine_page_down (IBusEngine *engine)
{
    g_debug ("page-down");
}

static void
ibus_engine_cursor_up (IBusEngine *engine)
{
    g_debug ("cursor-up");
}

static void
ibus_engine_cursor_down (IBusEngine *engine)
{
    g_debug ("cursor-down");
}

static void
ibus_engine_property_activate (IBusEngine *engine,
    const gchar *prop_name, gint prop_state)
{
    g_debug ("property-activate ('%s', %d)", prop_name, prop_state);
}

static void
ibus_engine_property_show (IBusEngine *engine, const gchar *prop_name)
{
    g_debug ("property-show ('%s')", prop_name);
}

static void
ibus_engine_property_hide (IBusEngine *engine, const gchar *prop_name)
{
    g_debug ("property-hide ('%s')", prop_name);
}

static void
_send_signal (IBusEngine *engine, const gchar *name,
    gint first_arg_type, ...)
{
    g_assert (IBUS_IS_ENGINE (engine));
    g_assert (name != NULL);

    va_list args;
    const gchar *path;
    IBusEnginePrivate *priv;

    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    va_start (args, first_arg_type);
    ibus_connection_send_signal_valist (priv->connection,
            path, IBUS_INTERFACE_ENGINE, name,
            first_arg_type, args);
    va_end (args);
}

void
ibus_engine_commit_string (IBusEngine *engine, const gchar *text)
{
    _send_signal (engine, "CommitString",
            DBUS_TYPE_STRING, &text,
            DBUS_TYPE_INVALID);
}

void
ibus_engine_update_preedit (IBusEngine      *engine,
                            const gchar     *text,
                            IBusAttrList    *attr_list,
                            gint             cursor_pos,
                            gboolean         visible)
{
    g_assert (IBUS_IS_ENGINE (engine));
    g_assert (text != NULL);
    g_assert (attr_list != NULL);

    IBusEnginePrivate *priv;
    DBusMessage *message;
    DBusMessageIter iter;
    const gchar *path;

    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    path = ibus_service_get_path (IBUS_SERVICE (engine));
    message = dbus_message_new_signal (
                    path,
                    IBUS_INTERFACE_ENGINE, "UpdatePreedit");

    dbus_message_iter_init_append (message, &iter);

    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &text);
    ibus_attr_list_to_dbus_message (attr_list, &iter);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &cursor_pos);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &visible);

    ibus_connection_send (priv->connection, message);
    dbus_message_unref (message);
}

void
ibus_engine_show_preedit (IBusEngine *engine)
{
    _send_signal (engine, "ShowPreedit",
            DBUS_TYPE_INVALID);
}

void ibus_engine_hide_preedit (IBusEngine *engine)
{
    _send_signal (engine, "HidePreedit",
            DBUS_TYPE_INVALID);
}

void ibus_engine_update_aux_string (IBusEngine      *engine,
                                    const gchar     *text,
                                    IBusAttrList    *attr_list,
                                    gboolean         visible)
{
    g_assert (IBUS_IS_ENGINE (engine));
    g_assert (text != NULL);
    g_assert (attr_list != NULL);

    IBusEnginePrivate *priv;
    DBusMessage *message;
    DBusMessageIter iter;
    const gchar *path;

    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    path = ibus_service_get_path (IBUS_SERVICE (engine));
    message = dbus_message_new_signal (
                    path,
                    IBUS_INTERFACE_ENGINE, "UpdateAuxString");

    dbus_message_iter_init_append (message, &iter);

    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &text);
    ibus_attr_list_to_dbus_message (attr_list, &iter);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &visible);

    ibus_connection_send (priv->connection, message);
    dbus_message_unref (message);
}

void
ibus_engine_show_aux_string (IBusEngine *engine)
{
    _send_signal (engine, "ShowAuxString",
            DBUS_TYPE_INVALID);
}

void
ibus_engine_hide_aux_string (IBusEngine *engine)
{
    _send_signal (engine, "HideAuxString",
            DBUS_TYPE_INVALID);
}

void ibus_engine_update_lookup_table (IBusEngine        *engine,
                                      IBusLookupTable   *table,
                                      gboolean           visible)
{
    g_assert (IBUS_IS_ENGINE (engine));
    g_assert (table != NULL);

    IBusEnginePrivate *priv;
    DBusMessage *message;
    DBusMessageIter iter;
    const gchar *path;

    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    path = ibus_service_get_path (IBUS_SERVICE (engine));
    message = dbus_message_new_signal (
                    path,
                    IBUS_INTERFACE_ENGINE, "UpdateLookupTable");

    dbus_message_iter_init_append (message, &iter);

    ibus_lookup_table_to_dbus_message (table, &iter);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &visible);

    ibus_connection_send (priv->connection, message);
    dbus_message_unref (message);
}

void ibus_engine_show_lookup_table (IBusEngine *engine)
{
    _send_signal (engine, "ShowLookupTable",
            DBUS_TYPE_INVALID);
}

void ibus_engine_hide_lookup_table (IBusEngine *engine)
{
    _send_signal (engine, "HideLookupTable",
            DBUS_TYPE_INVALID);
}

void ibus_engine_forward_key_event (IBusEngine      *engine,
                                    guint            keyval,
                                    gboolean         is_press,
                                    guint            state)
{
    _send_signal (engine, "HideLookupTable",
            DBUS_TYPE_UINT32, &keyval,
            DBUS_TYPE_BOOLEAN, &is_press,
            DBUS_TYPE_UINT32, &state,
            DBUS_TYPE_INVALID);
}
