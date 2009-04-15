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

#include <stdarg.h>
#include "ibusengine.h"
#include "ibusinternal.h"
#include "ibusshare.h"

#define IBUS_ENGINE_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_ENGINE, IBusEnginePrivate))

enum {
    PROCESS_KEY_EVENT,
    FOCUS_IN,
    FOCUS_OUT,
    RESET,
    ENABLE,
    DISABLE,
    SET_CURSOR_LOCATION,
    SET_CAPABILITIES,
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
    PROP_NAME,
    PROP_CONNECTION,
};


/* IBusEnginePriv */
struct _IBusEnginePrivate {
    gchar *name;
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
static gboolean ibus_engine_ibus_message    (IBusEngine         *engine,
                                             IBusConnection     *connection,
                                             IBusMessage        *message);
static gboolean ibus_engine_process_key_event
                                            (IBusEngine         *engine,
                                             guint               keyval,
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
static void     ibus_engine_set_capabilities
                                            (IBusEngine         *engine,
                                             guint               caps);
static void     ibus_engine_page_up         (IBusEngine         *engine);
static void     ibus_engine_page_down       (IBusEngine         *engine);
static void     ibus_engine_cursor_up       (IBusEngine         *engine);
static void     ibus_engine_cursor_down     (IBusEngine         *engine);
static void     ibus_engine_property_activate
                                            (IBusEngine         *engine,
                                             const gchar        *prop_name,
                                             guint               prop_state);
static void     ibus_engine_property_show   (IBusEngine         *engine,
                                             const gchar        *prop_name);
static void     ibus_engine_property_hide   (IBusEngine         *engine,
                                             const gchar        *prop_name);


static IBusServiceClass  *parent_class = NULL;

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
ibus_engine_new (const gchar    *name,
                 const gchar    *path,
                 IBusConnection *connection)
{
    g_assert (path);
    g_assert (IBUS_IS_CONNECTION (connection));

    IBusEngine *engine;

    engine = (IBusEngine *) g_object_new (IBUS_TYPE_ENGINE,
                                          "name", name,
                                          "path", path,
                                          "connection", connection,
                                          NULL);

    return engine;
}

static void
ibus_engine_class_init (IBusEngineClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusServiceClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusEnginePrivate));

    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_engine_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_engine_get_property;

    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_engine_destroy;

    IBUS_SERVICE_CLASS (klass)->ibus_message = (ServiceIBusMessageFunc) ibus_engine_ibus_message;

    klass->process_key_event = ibus_engine_process_key_event;
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
    klass->set_capabilities     = ibus_engine_set_capabilities;


    /* install properties */
    /**
     * IBusEngine:name:
     *
     * Name of this IBusEngine.
     */
    g_object_class_install_property (gobject_class,
                    PROP_NAME,
                    g_param_spec_string ("name",
                        "name",
                        "engine name",
                        "noname",
                        G_PARAM_READWRITE |  G_PARAM_CONSTRUCT_ONLY));

    /**
     * IBusEngine:connection:
     *
     * Connection of this IBusEngine.
     */
    g_object_class_install_property (gobject_class,
                    PROP_CONNECTION,
                    g_param_spec_object ("connection",
                        "connection",
                        "The connection of engine object",
                        IBUS_TYPE_CONNECTION,
                        G_PARAM_READWRITE |  G_PARAM_CONSTRUCT_ONLY));

    /* install signals */
    /**
     * IBusEngine::process-key-event:
     * @engine: An IBusEngine.
     * @keyval: KeySym of the key press.
     * @state: Key modifier flags
     *
     * Emitted when a key event is received.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[PROCESS_KEY_EVENT] =
        g_signal_new (I_("process-key-event"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, process_key_event),
            NULL, NULL,
            ibus_marshal_BOOL__UINT_UINT,
            G_TYPE_BOOLEAN,
            2,
            G_TYPE_UINT,
            G_TYPE_UINT);

    /**
     * IBusEngine::focus-in:
     * @engine: An IBusEngine.
     *
     * Emitted when the client application get the focus.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[FOCUS_IN] =
        g_signal_new (I_("focus-in"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, focus_in),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /**
     * IBusEngine::focus-out:
     * @engine: An IBusEngine.
     *
     * Emitted when the client application  lost the focus.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[FOCUS_OUT] =
        g_signal_new (I_("focus-out"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, focus_out),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /**
     * IBusEngine::reset:
     * @engine: An IBusEngine.
     *
     * Emitted when the IME is reset.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[RESET] =
        g_signal_new (I_("reset"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, reset),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /**
     * IBusEngine::enable:
     * @engine: An IBusEngine.
     *
     * Emitted when the IME is enabled.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[ENABLE] =
        g_signal_new (I_("enable"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, enable),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /**
     * IBusEngine::disable:
     * @engine: An IBusEngine.
     *
     * Emitted when the IME is disabled.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[DISABLE] =
        g_signal_new (I_("disable"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, disable),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /**
     * IBusEngine::set-cursor-location:
     * @engine: An IBusEngine.
     *
     * Emitted when the location of IME is set.
     * Implement this functionin extended class to receive this signal.
     */
    engine_signals[SET_CURSOR_LOCATION] =
        g_signal_new (I_("set-cursor-location"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, set_cursor_location),
            NULL, NULL,
            ibus_marshal_VOID__INT_INT_INT_INT,
            G_TYPE_NONE,
            4,
            G_TYPE_INT,
            G_TYPE_INT,
            G_TYPE_INT,
            G_TYPE_INT);

    /**
     * IBusEngine::set-capabilities:
     * @engine: An IBusEngine.
     *
     * Emitted when the client application capabilities is set.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[SET_CAPABILITIES] =
        g_signal_new (I_("set-capabilities"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, set_capabilities),
            NULL, NULL,
            ibus_marshal_VOID__UINT,
            G_TYPE_NONE,
            1,
            G_TYPE_UINT);

    /**
     * IBusEngine::page-up:
     * @engine: An IBusEngine.
     *
     * Emitted when the page-up key is pressed.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[PAGE_UP] =
        g_signal_new (I_("page-up"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, page_up),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /**
     * IBusEngine::page-down:
     * @engine: An IBusEngine.
     *
     * Emitted when the page-down key is pressed.
     * Implement this function extend class to receive this signal.
     */
    engine_signals[PAGE_DOWN] =
        g_signal_new (I_("page-down"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, page_down),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /**
     * IBusEngine::cursor-up:
     * @engine: An IBusEngine.
     *
     * Emitted when the up cursor key is pressed.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[CURSOR_UP] =
        g_signal_new (I_("cursor-up"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, cursor_up),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /**
     * IBusEngine::cursor-down:
     * @engine: An IBusEngine.
     *
     * Emitted when the down cursor key is pressed.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[CURSOR_DOWN] =
        g_signal_new (I_("cursor-down"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, cursor_down),
            NULL, NULL,
            ibus_marshal_VOID__VOID,
            G_TYPE_NONE,
            0);

    /**
     * IBusEngine::property-activate:
     * @engine: An IBusEngine.
     *
     * Emitted when a property is activated or change changed.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[PROPERTY_ACTIVATE] =
        g_signal_new (I_("property-activate"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, property_activate),
            NULL, NULL,
            ibus_marshal_VOID__STRING_UINT,
            G_TYPE_NONE,
            2,
            G_TYPE_STRING,
            G_TYPE_UINT);

    /**
     * IBusEngine::property-show:
     * @engine: An IBusEngine.
     *
     * Emitted when a property is shown.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[PROPERTY_SHOW] =
        g_signal_new (I_("property-show"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, property_show),
            NULL, NULL,
            ibus_marshal_VOID__STRING,
            G_TYPE_NONE,
            1,
            G_TYPE_STRING);

    /**
     * IBusEngine::property-hide:
     * @engine: An IBusEngine.
     *
     * Emitted when a property is hidden.
     * Implement this function in extended class to receive this signal.
     */
    engine_signals[PROPERTY_HIDE] =
        g_signal_new (I_("property-hide"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, property_hide),
            NULL, NULL,
            ibus_marshal_VOID__STRING,
            G_TYPE_NONE,
            1,
            G_TYPE_STRING);

}

static void
ibus_engine_init (IBusEngine *engine)
{
    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    priv->name = NULL;
    priv->connection = NULL;
}

static void
ibus_engine_destroy (IBusEngine *engine)
{
    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    g_free (priv->name);

    if (priv->connection) {
        g_object_unref (priv->connection);
        priv->connection = NULL;
    }

    IBUS_OBJECT_CLASS(parent_class)->destroy (IBUS_OBJECT (engine));
}

static void
ibus_engine_set_property (IBusEngine   *engine,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    switch (prop_id) {
    case PROP_NAME:
        priv->name = g_strdup (g_value_dup_string (value));
        break;

    case PROP_CONNECTION:
        priv->connection = g_value_get_object (value);
        g_object_ref (priv->connection);
        ibus_service_add_to_connection ((IBusService *) engine,
                                        priv->connection);
        break;

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
    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;

    case PROP_CONNECTION:
        g_value_set_object (value, priv->connection);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (engine, prop_id, pspec);
    }
}

static gboolean
ibus_engine_ibus_message (IBusEngine     *engine,
                          IBusConnection *connection,
                          IBusMessage    *message)
{
    g_assert (IBUS_IS_ENGINE (engine));
    g_assert (IBUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    g_assert (priv->connection == connection);

    IBusMessage *return_message = NULL;
    IBusMessage *error_message = NULL;

    static const struct {
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
        if (!ibus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, no_arg_methods[i].member))
            continue;

        IBusMessageIter iter;
        ibus_message_iter_init (message, &iter);
        if (ibus_message_iter_has_next (&iter)) {
            error_message = ibus_message_new_error_printf (message,
                                DBUS_ERROR_INVALID_ARGS,
                                "%s.%s: Method does not have arguments",
                                IBUS_INTERFACE_ENGINE, no_arg_methods[i].member);
            ibus_connection_send (connection, error_message);
            ibus_message_unref (error_message);
            return TRUE;
        }

        g_signal_emit (engine, engine_signals[no_arg_methods[i].signal_id], 0);
        return_message = ibus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        ibus_message_unref (return_message);
        return TRUE;
    }


    if (ibus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "ProcessKeyEvent")) {
        guint keyval, state;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_UINT, &keyval,
                                        G_TYPE_UINT, &state,
                                        G_TYPE_INVALID);

        if (!retval)
            goto _keypress_fail;

        retval = FALSE;
        g_signal_emit (engine,
                       engine_signals[PROCESS_KEY_EVENT],
                       0,
                       keyval,
                       state,
                       &retval);

        return_message = ibus_message_new_method_return (message);
        ibus_message_append_args (return_message,
                                  G_TYPE_BOOLEAN, &retval,
                                  G_TYPE_INVALID);
        ibus_connection_send (connection, return_message);
        ibus_message_unref (return_message);
        return TRUE;

    _keypress_fail:
        error_message = ibus_message_new_error_printf (message,
                        DBUS_ERROR_INVALID_ARGS,
                        "%s.%s: Can not match signature (ubu) of method",
                        IBUS_INTERFACE_ENGINE, "ProcessKeyEvent");
        ibus_connection_send (connection, error_message);
        ibus_message_unref (error_message);
        return TRUE;
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "PropertyActivate")) {
        gchar *name;
        guint state;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &name,
                                        G_TYPE_UINT, &state,
                                        G_TYPE_INVALID);

        if (!retval)
            goto _property_activate_fail;

        g_signal_emit (engine,
                       engine_signals[PROPERTY_ACTIVATE],
                       0,
                       name,
                       state);

        return_message = ibus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        ibus_message_unref (return_message);
        return TRUE;

    _property_activate_fail:
        error_message = ibus_message_new_error_printf (message,
                        DBUS_ERROR_INVALID_ARGS,
                        "%s.%s: Can not match signature (si) of method",
                        IBUS_INTERFACE_ENGINE,
                        "PropertyActivate");
        ibus_connection_send (connection, error_message);
        ibus_message_unref (error_message);
        return TRUE;

    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "PropertyShow")) {
        gchar *name;
        gboolean retval;
        IBusError *error;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &name,
                                        G_TYPE_INVALID);

        if (!retval)
            goto _property_show_fail;

        g_signal_emit (engine,
                       engine_signals[PROPERTY_SHOW],
                       0,
                       name);

        return_message = ibus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        ibus_message_unref (return_message);
        return TRUE;

    _property_show_fail:
        error_message = ibus_message_new_error_printf (message,
                        DBUS_ERROR_INVALID_ARGS,
                        "%s.%s: Can not match signature (s) of method",
                        IBUS_INTERFACE_ENGINE,
                        "PropertyShow");
        ibus_connection_send (connection, error_message);
        ibus_message_unref (error_message);
        return TRUE;
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "PropertyHide")) {
        gchar *name;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_STRING, &name,
                                        G_TYPE_INVALID);
        if (!retval)
            goto _property_hide_fail;

        g_signal_emit (engine, engine_signals[PROPERTY_HIDE], 0, name);

        return_message = ibus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        ibus_message_unref (return_message);
        return TRUE;

    _property_hide_fail:
        error_message = ibus_message_new_error_printf (message,
                        DBUS_ERROR_INVALID_ARGS,
                        "%s.%s: Can not match signature (s) of method",
                        IBUS_INTERFACE_ENGINE, "PropertyHide");
        ibus_connection_send (connection, error_message);
        ibus_message_unref (error_message);
        return TRUE;
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "SetCursorLocation")) {
        gint x, y, w, h;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_INT, &x,
                                        G_TYPE_INT, &y,
                                        G_TYPE_INT, &w,
                                        G_TYPE_INT, &h,
                                        G_TYPE_INVALID);
        if (!retval)
            goto _set_cursor_location_fail;

        engine->cursor_area.x = x;
        engine->cursor_area.y = y;
        engine->cursor_area.width = w;
        engine->cursor_area.height = h;

        g_signal_emit (engine,
                       engine_signals[SET_CURSOR_LOCATION],
                       0,
                       x, y, w, h);

        return_message = ibus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        ibus_message_unref (return_message);
        return TRUE;

    _set_cursor_location_fail:
        error_message = ibus_message_new_error_printf (message,
                        DBUS_ERROR_INVALID_ARGS,
                        "%s.%s: Can not match signature (iiii) of method",
                        IBUS_INTERFACE_ENGINE,
                        "SetCursorLocation");
        ibus_connection_send (connection, error_message);
        ibus_message_unref (error_message);
        return TRUE;
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "SetCapabilities")) {
        guint caps;
        gboolean retval;
        IBusError *error = NULL;

        retval = ibus_message_get_args (message,
                                        &error,
                                        G_TYPE_UINT, &caps,
                                        G_TYPE_INVALID);

        if (!retval)
            goto _set_capabilities_fail;

        engine->client_capabilities = caps;

        g_signal_emit (engine, engine_signals[SET_CAPABILITIES], 0, caps);

        return_message = ibus_message_new_method_return (message);
        ibus_connection_send (connection, return_message);
        ibus_message_unref (return_message);
        return TRUE;

    _set_capabilities_fail:
        error_message = ibus_message_new_error_printf (message,
                        DBUS_ERROR_INVALID_ARGS,
                        "%s.%s: Can not match signature (u) of method",
                        IBUS_INTERFACE_ENGINE, "SetCapabilities");
        ibus_connection_send (connection, error_message);
        ibus_message_unref (error_message);
        return TRUE;
    }
    else if (ibus_message_is_method_call (message, IBUS_INTERFACE_ENGINE, "Destroy")) {
        return_message = ibus_message_new_method_return (message);

        ibus_connection_send (connection, return_message);
        ibus_message_unref (return_message);

        ibus_object_destroy ((IBusObject *) engine);
    }

    return parent_class->ibus_message ((IBusService *) engine, connection, message);
}

static gboolean
ibus_engine_process_key_event (IBusEngine *engine,
                               guint       keyval,
                               guint       state)
{
    // g_debug ("process-key-event (%d, %d)", keyval, state);
    return FALSE;
}

static void
ibus_engine_focus_in (IBusEngine *engine)
{
    // g_debug ("focus-in");
}

static void
ibus_engine_focus_out (IBusEngine *engine)
{
    // g_debug ("focus-out");
}

static void
ibus_engine_reset (IBusEngine *engine)
{
    // g_debug ("reset");
}

static void
ibus_engine_enable (IBusEngine *engine)
{
    // g_debug ("enable");
}

static void
ibus_engine_disable (IBusEngine *engine)
{
    // g_debug ("disable");
}

static void
ibus_engine_set_cursor_location (IBusEngine *engine,
                                 gint        x,
                                 gint        y,
                                 gint        w,
                                 gint        h)
{
    // g_debug ("set-cursor-location (%d, %d, %d, %d)", x, y, w, h);
}

static void
ibus_engine_set_capabilities (IBusEngine *engine,
                              guint       caps)
{
    // g_debug ("set-capabilities (0x%04x)", caps);
}

static void
ibus_engine_page_up (IBusEngine *engine)
{
    // g_debug ("page-up");
}

static void
ibus_engine_page_down (IBusEngine *engine)
{
    // g_debug ("page-down");
}

static void
ibus_engine_cursor_up (IBusEngine *engine)
{
    // g_debug ("cursor-up");
}

static void
ibus_engine_cursor_down (IBusEngine *engine)
{
    // g_debug ("cursor-down");
}

static void
ibus_engine_property_activate (IBusEngine  *engine,
                               const gchar *prop_name,
                               guint        prop_state)
{
    // g_debug ("property-activate ('%s', %d)", prop_name, prop_state);
}

static void
ibus_engine_property_show (IBusEngine *engine, const gchar *prop_name)
{
    // g_debug ("property-show ('%s')", prop_name);
}

static void
ibus_engine_property_hide (IBusEngine *engine, const gchar *prop_name)
{
    // g_debug ("property-hide ('%s')", prop_name);
}

static void
_send_signal (IBusEngine  *engine,
              const gchar *name,
              GType        first_arg_type,
              ...)
{
    g_assert (IBUS_IS_ENGINE (engine));
    g_assert (name != NULL);

    va_list args;
    const gchar *path;
    IBusEnginePrivate *priv;

    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    path = ibus_service_get_path ((IBusService *)engine);

    va_start (args, first_arg_type);
    ibus_connection_send_signal_valist (priv->connection,
                                        path,
                                        IBUS_INTERFACE_ENGINE,
                                        name,
                                        first_arg_type,
                                        args);
    va_end (args);
}

void
ibus_engine_commit_text (IBusEngine *engine,
                         IBusText   *text)
{
    _send_signal (engine,
                  "CommitText",
                  IBUS_TYPE_TEXT, &text,
                  G_TYPE_INVALID);
}

void
ibus_engine_update_preedit_text (IBusEngine      *engine,
                                 IBusText        *text,
                                 guint            cursor_pos,
                                 gboolean         visible)
{
    _send_signal (engine,
                  "UpdatePreeditText",
                  IBUS_TYPE_TEXT, &text,
                  G_TYPE_UINT, &cursor_pos,
                  G_TYPE_BOOLEAN, &visible,
                  G_TYPE_INVALID);
}

void
ibus_engine_show_preedit_text (IBusEngine *engine)
{
    _send_signal (engine,
                  "ShowPreeditText",
                  G_TYPE_INVALID);
}

void ibus_engine_hide_preedit_text (IBusEngine *engine)
{
    _send_signal (engine,
                  "HidePreeditText",
                  G_TYPE_INVALID);
}

void ibus_engine_update_auxiliary_text (IBusEngine      *engine,
                                        IBusText        *text,
                                        gboolean         visible)
{
    _send_signal (engine,
                  "UpdateAuxiliaryText",
                  IBUS_TYPE_TEXT, &text,
                  G_TYPE_BOOLEAN, &visible,
                  G_TYPE_INVALID);
}

void
ibus_engine_show_auxiliary_text (IBusEngine *engine)
{
    _send_signal (engine,
                  "ShowAuxiliaryText",
                  G_TYPE_INVALID);
}

void
ibus_engine_hide_auxiliary_text (IBusEngine *engine)
{
    _send_signal (engine,
                  "HideAuxiliaryText",
                  G_TYPE_INVALID);
}

void
ibus_engine_update_lookup_table (IBusEngine        *engine,
                                 IBusLookupTable   *table,
                                 gboolean           visible)
{
    _send_signal (engine,
                  "UpdateLookupTable",
                  IBUS_TYPE_LOOKUP_TABLE, &table,
                  G_TYPE_BOOLEAN, &visible,
                  G_TYPE_INVALID);
}

void
ibus_engine_update_lookup_table_fast (IBusEngine        *engine,
                                      IBusLookupTable   *table,
                                      gboolean           visible)
{
    IBusLookupTable *new_table;
    gint page_begin;
    gint i;

    if (table->candidates->len < table->page_size << 2) {
        ibus_engine_update_lookup_table (engine, table, visible);
        return;
    }

    page_begin = (table->cursor_pos / table->page_size) * table->page_size;

    new_table = ibus_lookup_table_new (table->page_size, 0, table->cursor_visible, table->round);

    for (i = page_begin; i < page_begin + table->page_size && i < table->candidates->len; i++) {
        ibus_lookup_table_append_candidate (new_table, ibus_lookup_table_get_candidate (table, i));
    }

    ibus_lookup_table_set_cursor_pos (new_table, ibus_lookup_table_get_cursor_in_page (table));

    ibus_engine_update_lookup_table (engine, new_table, visible);

    g_object_unref (new_table);
}

void ibus_engine_show_lookup_table (IBusEngine *engine)
{
    _send_signal (engine,
                  "ShowLookupTable",
                  G_TYPE_INVALID);
}

void ibus_engine_hide_lookup_table (IBusEngine *engine)
{
    _send_signal (engine,
                  "HideLookupTable",
                  G_TYPE_INVALID);
}

void ibus_engine_forward_key_event (IBusEngine      *engine,
                                    guint            keyval,
                                    guint            state)
{
    _send_signal (engine,
                  "ForwardKeyEvent",
                  G_TYPE_UINT, &keyval,
                  G_TYPE_UINT, &state,
                  G_TYPE_INVALID);
}

void
ibus_engine_register_properties (IBusEngine   *engine,
                                 IBusPropList *prop_list)
{
    _send_signal (engine,
                  "RegisterProperties",
                  IBUS_TYPE_PROP_LIST, &prop_list,
                  G_TYPE_INVALID);
}

void
ibus_engine_update_property (IBusEngine   *engine,
                             IBusProperty *prop)
{
    _send_signal (engine,
                  "UpdateProperty",
                  IBUS_TYPE_PROPERTY, &prop,
                  G_TYPE_INVALID);
}

const gchar *
ibus_engine_get_name (IBusEngine *engine)
{
    g_assert (IBUS_IS_ENGINE (engine));

    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    return priv->name;
}

