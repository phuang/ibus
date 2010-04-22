/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2010 Red Hat, Inc.
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
    CANDIDATE_CLICKED,
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
                                             guint               keycode,
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
static void     ibus_engine_candidate_clicked
                                            (IBusEngine         *engine,
                                             guint               index,
                                             guint               button,
                                             guint               state);
static void     ibus_engine_property_activate
                                            (IBusEngine         *engine,
                                             const gchar        *prop_name,
                                             guint               prop_state);
static void     ibus_engine_property_show   (IBusEngine         *engine,
                                             const gchar        *prop_name);
static void     ibus_engine_property_hide   (IBusEngine         *engine,
                                             const gchar        *prop_name);


G_DEFINE_TYPE (IBusEngine, ibus_engine, IBUS_TYPE_SERVICE)

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
    klass->candidate_clicked    = ibus_engine_candidate_clicked;
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
     * @keyval: Key symbol of the key press.
     * @keycode: KeyCode of the key press.
     * @state: Key modifier flags.
     *
     * Emitted when a key event is received.
     * Implement the member function process_key_event() in extended class to receive this signal.
     * Both the key symbol and keycode are passed to the member function.
     * See ibus_input_context_process_key_event() for further explanation of
     * key symbol, keycode and which to use.
     *
     * Returns: TRUE for successfully process the key; FALSE otherwise.
     * See also:  ibus_input_context_process_key_event().
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
     */
    engine_signals[PROCESS_KEY_EVENT] =
        g_signal_new (I_("process-key-event"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, process_key_event),
            NULL, NULL,
            ibus_marshal_BOOL__UINT_UINT_UINT,
            G_TYPE_BOOLEAN,
            3,
            G_TYPE_UINT,
            G_TYPE_UINT,
            G_TYPE_UINT);

    /**
     * IBusEngine::focus-in:
     * @engine: An IBusEngine.
     *
     * Emitted when the client application get the focus.
     * Implement the member function focus_in() in extended class to receive this signal.
     *
     * See also: ibus_input_context_focus_in()
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * Implement the member function focus_out() in extended class to receive this signal.
     *
     * See also: ibus_input_context_focus_out()
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * Implement the member function reset() in extended class to receive this signal.
     *
     * See also:  ibus_input_context_reset().
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * Implement the member function set_enable() in extended class to receive this signal.
     *
     * See also:  ibus_input_context_enable().
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * Implement the member function set_disable() in extended class to receive this signal.
     *
     * See also:  ibus_input_context_disable().
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * @x: X coordinate of the cursor.
     * @y: Y coordinate of the cursor.
     * @w: Width of the cursor.
     * @h: Height of the cursor.
     *
     * Emitted when the location of IME is set.
     * Implement the member function set_cursor_location() in extended class to receive this signal.
     *
     * See also:  ibus_input_context_set_cursor_location().
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * @caps: Capabilities flags of IBusEngine, see #IBusCapabilite
     *
     * Emitted when the client application capabilities is set.
     * Implement the member function set_capabilities() in extended class to receive this signal.
     *
     * See also:  ibus_input_context_set_capabilities().
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * Emitted when the page-up button is pressed.
     * Implement the member function page_up() in extended class to receive this signal.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * Emitted when the page-down button is pressed.
     * Implement the member function page_down() in extended class to receive this signal.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * Emitted when the up cursor button is pressed.
     * Implement the member function cursor_up() in extended class to receive this signal.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * Emitted when the down cursor button is pressed.
     * Implement the member function cursor_down() in extended class to receive this signal.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * IBusEngine::candidate-clicked:
     * @engine: An IBusEngine.
     * @index:  Index of candidate be clicked.
     * @button: Mouse button.
     * @state:  Keyboard state.
     *
     * Emitted when candidate on lookup table is clicked.
     * Implement the member function candidate_clicked() in extended class to receive this signal.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
     */
    engine_signals[CANDIDATE_CLICKED] =
        g_signal_new (I_("candidate-clicked"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, candidate_clicked),
            NULL, NULL,
            ibus_marshal_VOID__UINT_UINT_UINT,
            G_TYPE_NONE,
            3,
            G_TYPE_UINT,
            G_TYPE_UINT,
            G_TYPE_UINT);

    /**
     * IBusEngine::property-activate:
     * @engine: An IBusEngine.
     * @name:   Property name.
     * @state:  Property state.
     *
     * Emitted when a property is activated or change changed.
     * Implement the member function property_activate() in extended class to receive this signal.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * @name:   Property name.
     *
     * Emitted when a property is shown.
     * Implement the member function property_side() in extended class to receive this signal.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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
     * @name:   Property name.
     *
     * Emitted when a property is hidden.
     * Implement the member function property_hide() in extended class to receive this signal.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
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

    IBUS_OBJECT_CLASS(ibus_engine_parent_class)->destroy (IBUS_OBJECT (engine));
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
        g_object_ref_sink (priv->connection);
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
    g_assert (ibus_message_get_type (message) == DBUS_MESSAGE_TYPE_METHOD_CALL);

    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    g_assert (priv->connection == connection);

    IBusMessage *reply = NULL;
    IBusError *error = NULL;
    gboolean retval;

    gint i;
    const gchar *interface;
    const gchar *name;

    static const struct {
        gchar *member;
        guint  signal_id;
    } no_arg_methods[] = {
        { "FocusIn",     FOCUS_IN },
        { "FocusOut",    FOCUS_OUT },
        { "Reset",       RESET },
        { "Enable",      ENABLE },
        { "Disable",     DISABLE },
        { "PageUp",      PAGE_UP },
        { "PageDown",    PAGE_DOWN },
        { "CursorUp",    CURSOR_UP },
        { "CursorDown",  CURSOR_DOWN },
    };

    interface = ibus_message_get_interface (message);
    name = ibus_message_get_member (message);

    if (interface != NULL && g_strcmp0 (interface, IBUS_INTERFACE_ENGINE) != 0)
        return IBUS_SERVICE_CLASS (ibus_engine_parent_class)->ibus_message (
                        (IBusService *) engine, connection, message);

    do {
        if (g_strcmp0 (name, "ProcessKeyEvent") == 0) {
            guint keyval, keycode, state;

            retval = ibus_message_get_args (message,
                                            &error,
                                            G_TYPE_UINT, &keyval,
                                            G_TYPE_UINT, &keycode,
                                            G_TYPE_UINT, &state,
                                            G_TYPE_INVALID);

            if (!retval) {
                reply = ibus_message_new_error_printf (message,
                                DBUS_ERROR_INVALID_ARGS,
                                "%s.%s: Can not match signature (uuu) of method",
                                IBUS_INTERFACE_ENGINE, "ProcessKeyEvent");
                ibus_error_free (error);
            }
            else {
                retval = FALSE;
                g_signal_emit (engine,
                               engine_signals[PROCESS_KEY_EVENT],
                               0,
                               keyval,
                               keycode,
                               state,
                               &retval);

                reply = ibus_message_new_method_return (message);
                ibus_message_append_args (reply,
                                          G_TYPE_BOOLEAN, &retval,
                                          G_TYPE_INVALID);
            }
            break;
        }

        for (i = 0;
             i < G_N_ELEMENTS (no_arg_methods) && g_strcmp0 (name, no_arg_methods[i].member) != 0;
             i++);

        if (i < G_N_ELEMENTS (no_arg_methods)) {
            IBusMessageIter iter;
            ibus_message_iter_init (message, &iter);
            if (ibus_message_iter_has_next (&iter)) {
                reply = ibus_message_new_error_printf (message,
                                    DBUS_ERROR_INVALID_ARGS,
                                    "%s.%s: Method does not have arguments",
                                    IBUS_INTERFACE_ENGINE, no_arg_methods[i].member);
            }
            else {
                g_signal_emit (engine, engine_signals[no_arg_methods[i].signal_id], 0);
                reply = ibus_message_new_method_return (message);
            }
            break;
        }

        if (g_strcmp0 (name, "CandidateClicked") == 0) {
            guint index, button, state;

            retval = ibus_message_get_args (message,
                                            &error,
                                            G_TYPE_UINT, &index,
                                            G_TYPE_UINT, &button,
                                            G_TYPE_UINT, &state,
                                            G_TYPE_INVALID);

            if (!retval) {
                reply = ibus_message_new_error_printf (message,
                                DBUS_ERROR_INVALID_ARGS,
                                "%s.%s: Can not match signature (uuu) of method",
                                IBUS_INTERFACE_ENGINE, "CandidateClicked");
                ibus_error_free (error);
            }
            else {
                g_signal_emit (engine,
                               engine_signals[CANDIDATE_CLICKED],
                               0,
                               index,
                               button,
                               state);
                reply = ibus_message_new_method_return (message);
            }
        }
        else if (g_strcmp0 (name, "PropertyActivate") == 0) {
            gchar *name;
            guint state;

            retval = ibus_message_get_args (message,
                                            &error,
                                            G_TYPE_STRING, &name,
                                            G_TYPE_UINT, &state,
                                            G_TYPE_INVALID);

            if (!retval) {
                reply = ibus_message_new_error_printf (message,
                                DBUS_ERROR_INVALID_ARGS,
                                "%s.%s: Can not match signature (si) of method",
                                IBUS_INTERFACE_ENGINE,
                                "PropertyActivate");
                ibus_error_free (error);
            }
            else {
                g_signal_emit (engine,
                               engine_signals[PROPERTY_ACTIVATE],
                               0,
                               name,
                               state);

                reply = ibus_message_new_method_return (message);
            }
        }
        else if (g_strcmp0 (name, "PropertyShow") == 0) {
            gchar *name;

            retval = ibus_message_get_args (message,
                                            &error,
                                            G_TYPE_STRING, &name,
                                            G_TYPE_INVALID);

            if (!retval) {
                reply = ibus_message_new_error_printf (message,
                            DBUS_ERROR_INVALID_ARGS,
                            "%s.%s: Can not match signature (s) of method",
                            IBUS_INTERFACE_ENGINE,
                            "PropertyShow");
                ibus_error_free (error);
            }
            else {
                g_signal_emit (engine,
                               engine_signals[PROPERTY_SHOW],
                               0,
                               name);

                reply = ibus_message_new_method_return (message);
            }
        }
        else if (g_strcmp0 (name, "PropertyHide") == 0) {
            gchar *name;

            retval = ibus_message_get_args (message,
                                            &error,
                                            G_TYPE_STRING, &name,
                                            G_TYPE_INVALID);
            if (!retval) {
                reply = ibus_message_new_error_printf (message,
                            DBUS_ERROR_INVALID_ARGS,
                            "%s.%s: Can not match signature (s) of method",
                            IBUS_INTERFACE_ENGINE, "PropertyHide");
                ibus_error_free (error);
            }
            else {
                g_signal_emit (engine, engine_signals[PROPERTY_HIDE], 0, name);
                reply = ibus_message_new_method_return (message);
            }
        }
        else if (g_strcmp0 (name, "SetCursorLocation") == 0) {
            gint x, y, w, h;

            retval = ibus_message_get_args (message,
                                            &error,
                                            G_TYPE_INT, &x,
                                            G_TYPE_INT, &y,
                                            G_TYPE_INT, &w,
                                            G_TYPE_INT, &h,
                                            G_TYPE_INVALID);
            if (!retval) {
                reply = ibus_message_new_error_printf (message,
                            DBUS_ERROR_INVALID_ARGS,
                            "%s.%s: Can not match signature (iiii) of method",
                            IBUS_INTERFACE_ENGINE,
                            "SetCursorLocation");
                ibus_error_free (error);
            }
            else {
                engine->cursor_area.x = x;
                engine->cursor_area.y = y;
                engine->cursor_area.width = w;
                engine->cursor_area.height = h;

                g_signal_emit (engine,
                               engine_signals[SET_CURSOR_LOCATION],
                               0,
                               x, y, w, h);

                reply = ibus_message_new_method_return (message);
            }
        }
        else if (g_strcmp0 (name, "SetCapabilities") == 0) {
            guint caps;

            retval = ibus_message_get_args (message,
                                            &error,
                                            G_TYPE_UINT, &caps,
                                            G_TYPE_INVALID);

            if (!retval) {
                reply = ibus_message_new_error_printf (message,
                            DBUS_ERROR_INVALID_ARGS,
                            "%s.%s: Can not match signature (u) of method",
                            IBUS_INTERFACE_ENGINE, "SetCapabilities");
                ibus_error_free (error);
            }
            else {
                engine->client_capabilities = caps;
                g_signal_emit (engine, engine_signals[SET_CAPABILITIES], 0, caps);
                reply = ibus_message_new_method_return (message);
            }
        }
        else if (g_strcmp0 (name, "Destroy") == 0) {
            reply = ibus_message_new_method_return (message);
            ibus_connection_send (connection, reply);
            ibus_message_unref (reply);
            ibus_object_destroy ((IBusObject *) engine);
            return TRUE;
        }
        else {
            reply = ibus_message_new_error_printf (message,
                        DBUS_ERROR_UNKNOWN_METHOD,
                        "%s.%s",
                        IBUS_INTERFACE_ENGINE, name);
            g_warn_if_reached ();
        }
    } while (0);

    ibus_connection_send (connection, reply);
    ibus_message_unref (reply);
    return TRUE;
}

static gboolean
ibus_engine_process_key_event (IBusEngine *engine,
                               guint       keyval,
                               guint       keycode,
                               guint       state)
{
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
ibus_engine_candidate_clicked (IBusEngine *engine,
                               guint       index,
                               guint       button,
                               guint       state)
{
    // g_debug ("candidate-clicked");
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

    if (g_object_is_floating (text)) {
        g_object_unref (text);
    }
}

void
ibus_engine_update_preedit_text (IBusEngine      *engine,
                                 IBusText        *text,
                                 guint            cursor_pos,
                                 gboolean         visible)
{
    ibus_engine_update_preedit_text_with_mode (engine,
            text, cursor_pos, visible, IBUS_ENGINE_PREEDIT_CLEAR);
}

void
ibus_engine_update_preedit_text_with_mode (IBusEngine            *engine,
                                           IBusText              *text,
                                           guint                  cursor_pos,
                                           gboolean               visible,
                                           IBusPreeditFocusMode   mode)
{
    _send_signal (engine,
                  "UpdatePreeditText",
                  IBUS_TYPE_TEXT, &text,
                  G_TYPE_UINT, &cursor_pos,
                  G_TYPE_BOOLEAN, &visible,
                  G_TYPE_UINT, &mode,
                  G_TYPE_INVALID);

    if (g_object_is_floating (text)) {
        g_object_unref (text);
    }
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

    if (g_object_is_floating (text)) {
        g_object_unref (text);
    }
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

    if (g_object_is_floating (table)) {
        g_object_unref (table);
    }
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
    ibus_lookup_table_set_orientation (new_table, ibus_lookup_table_get_orientation (table));

    ibus_engine_update_lookup_table (engine, new_table, visible);

    if (g_object_is_floating (table)) {
        g_object_unref (table);
    }
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
                                    guint            keycode,
                                    guint            state)
{
    _send_signal (engine,
                  "ForwardKeyEvent",
                  G_TYPE_UINT, &keyval,
                  G_TYPE_UINT, &keycode,
                  G_TYPE_UINT, &state,
                  G_TYPE_INVALID);
}

void ibus_engine_delete_surrounding_text (IBusEngine      *engine,
                                          gint             offset_from_cursor,
                                          guint            nchars)
{
    _send_signal (engine,
                  "DeleteSurroundingText",
                  G_TYPE_INT,  &offset_from_cursor,
                  G_TYPE_UINT, &nchars,
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

    if (g_object_is_floating (prop_list)) {
        g_object_unref (prop_list);
    }
}

void
ibus_engine_update_property (IBusEngine   *engine,
                             IBusProperty *prop)
{
    _send_signal (engine,
                  "UpdateProperty",
                  IBUS_TYPE_PROPERTY, &prop,
                  G_TYPE_INVALID);

    if (g_object_is_floating (prop)) {
        g_object_unref (prop);
    }
}

const gchar *
ibus_engine_get_name (IBusEngine *engine)
{
    g_assert (IBUS_IS_ENGINE (engine));

    IBusEnginePrivate *priv;
    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    return priv->name;
}

