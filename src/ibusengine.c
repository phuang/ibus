/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
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
#include "ibusengine.h"
#include <stdarg.h>
#include <string.h>
#include "ibusmarshalers.h"
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
    SET_SURROUNDING_TEXT,
    PROCESS_HAND_WRITING_EVENT,
    CANCEL_HAND_WRITING,
    LAST_SIGNAL,
};

enum {
    PROP_0,
    PROP_ENGINE_NAME,
};


/* IBusEnginePriv */
struct _IBusEnginePrivate {
    gchar *engine_name;
    GDBusConnection *connection;

    /* cached surrounding text (see also IBusInputContextPrivate and
       BusEngineProxy) */
    IBusText *surrounding_text;
    guint surrounding_cursor_pos;
    guint selection_anchor_pos;
};

static guint            engine_signals[LAST_SIGNAL] = { 0 };

static IBusText *text_empty = NULL;

/* functions prototype */
static void      ibus_engine_destroy         (IBusEngine         *engine);
static void      ibus_engine_set_property    (IBusEngine         *engine,
                                              guint               prop_id,
                                              const GValue       *value,
                                              GParamSpec         *pspec);
static void      ibus_engine_get_property    (IBusEngine         *engine,
                                              guint               prop_id,
                                              GValue             *value,
                                              GParamSpec         *pspec);
static void      ibus_engine_service_method_call
                                              (IBusService        *service,
                                               GDBusConnection    *connection,
                                               const gchar        *sender,
                                               const gchar        *object_path,
                                               const gchar        *interface_name,
                                               const gchar        *method_name,
                                               GVariant           *parameters,
                                               GDBusMethodInvocation
                                                                  *invocation);
static GVariant *ibus_engine_service_get_property
                                             (IBusService        *service,
                                              GDBusConnection    *connection,
                                              const gchar        *sender,
                                              const gchar        *object_path,
                                              const gchar        *interface_name,
                                              const gchar        *property_name,
                                              GError            **error);
static gboolean  ibus_engine_service_set_property
                                             (IBusService        *service,
                                              GDBusConnection    *connection,
                                              const gchar        *sender,
                                              const gchar        *object_path,
                                              const gchar        *interface_name,
                                              const gchar        *property_name,
                                              GVariant           *value,
                                              GError            **error);
static gboolean  ibus_engine_process_key_event
                                             (IBusEngine         *engine,
                                              guint               keyval,
                                              guint               keycode,
                                              guint               state);
static void      ibus_engine_focus_in        (IBusEngine         *engine);
static void      ibus_engine_focus_out       (IBusEngine         *engine);
static void      ibus_engine_reset           (IBusEngine         *engine);
static void      ibus_engine_enable          (IBusEngine         *engine);
static void      ibus_engine_disable         (IBusEngine         *engine);
static void      ibus_engine_set_cursor_location
                                             (IBusEngine         *engine,
                                              gint                x,
                                              gint                y,
                                              gint                w,
                                              gint                h);
static void      ibus_engine_set_capabilities
                                             (IBusEngine         *engine,
                                              guint               caps);
static void      ibus_engine_page_up         (IBusEngine         *engine);
static void      ibus_engine_page_down       (IBusEngine         *engine);
static void      ibus_engine_cursor_up       (IBusEngine         *engine);
static void      ibus_engine_cursor_down     (IBusEngine         *engine);
static void      ibus_engine_candidate_clicked
                                             (IBusEngine         *engine,
                                              guint               index,
                                              guint               button,
                                              guint               state);
static void      ibus_engine_property_activate
                                             (IBusEngine         *engine,
                                              const gchar        *prop_name,
                                              guint               prop_state);
static void      ibus_engine_property_show   (IBusEngine         *engine,
                                              const gchar        *prop_name);
static void      ibus_engine_property_hide   (IBusEngine         *engine,
                                              const gchar        *prop_name);
static void      ibus_engine_set_surrounding_text
                                            (IBusEngine         *engine,
                                             IBusText           *text,
                                             guint               cursor_pos,
                                             guint               anchor_pos);
static void      ibus_engine_process_hand_writing_event
                                             (IBusEngine         *engine,
                                              const gdouble      *coordinates,
                                              guint               coordinates_len);
static void      ibus_engine_cancel_hand_writing
                                             (IBusEngine         *engine,
                                              guint               n_strokes);
static void      ibus_engine_emit_signal     (IBusEngine         *engine,
                                              const gchar        *signal_name,
                                              GVariant           *parameters);


G_DEFINE_TYPE (IBusEngine, ibus_engine, IBUS_TYPE_SERVICE)

static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='org.freedesktop.IBus.Engine'>"
    /* FIXME methods */
    "    <method name='ProcessKeyEvent'>"
    "      <arg direction='in'  type='u' name='keyval' />"
    "      <arg direction='in'  type='u' name='keycode' />"
    "      <arg direction='in'  type='u' name='state' />"
    "      <arg direction='out' type='b' />"
    "    </method>"
    "    <method name='SetCursorLocation'>"
    "      <arg direction='in'  type='i' name='x' />"
    "      <arg direction='in'  type='i' name='y' />"
    "      <arg direction='in'  type='i' name='w' />"
    "      <arg direction='in'  type='i' name='h' />"
    "    </method>"
    "    <method name='ProcessHandWritingEvent'>"
    "      <arg direction='in'  type='ad' name='coordinates' />"
    "    </method>"
    "    <method name='CancelHandWriting'>"
    "      <arg direction='in'  type='u' name='n_strokes' />"
    "    </method>"
    "    <method name='SetCapabilities'>"
    "      <arg direction='in'  type='u' name='caps' />"
    "    </method>"
    "    <method name='PropertyActivate'>"
    "      <arg direction='in'  type='s' name='name' />"
    "      <arg direction='in'  type='u' name='state' />"
    "    </method>"
    "    <method name='PropertyShow'>"
    "      <arg direction='in'  type='s' name='name' />"
    "    </method>"
    "    <method name='PropertyHide'>"
    "      <arg direction='in'  type='s' name='name' />"
    "    </method>"
    "    <method name='CandidateClicked'>"
    "      <arg direction='in'  type='u' name='index' />"
    "      <arg direction='in'  type='u' name='button' />"
    "      <arg direction='in'  type='u' name='state' />"
    "    </method>"
    "    <method name='FocusIn' />"
    "    <method name='FocusOut' />"
    "    <method name='Reset' />"
    "    <method name='Enable' />"
    "    <method name='Disable' />"
    "    <method name='PageUp' />"
    "    <method name='PageDown' />"
    "    <method name='CursorUp' />"
    "    <method name='CursorDown' />"
    "    <method name='SetSurroundingText'>"
    "      <arg direction='in'  type='v' name='text' />"
    "      <arg direction='in'  type='u' name='cursor_pos' />"
    "      <arg direction='in'  type='u' name='anchor_pos' />"
    "    </method>"
    /* FIXME signals */
    "    <signal name='CommitText'>"
    "      <arg type='v' name='text' />"
    "    </signal>"
    "    <signal name='UpdatePreeditText'>"
    "      <arg type='v' name='text' />"
    "      <arg type='u' name='cursor_pos' />"
    "      <arg type='b' name='visible' />"
    "      <arg type='u' name='mode' />"
    "    </signal>"
    "    <signal name='UpdateAuxiliaryText'>"
    "      <arg type='v' name='text' />"
    "      <arg type='b' name='visible' />"
    "    </signal>"
    "    <signal name='UpdateLookupTable'>"
    "      <arg type='v' name='table' />"
    "      <arg type='b' name='visible' />"
    "    </signal>"
    "    <signal name='RegisterProperties'>"
    "      <arg type='v' name='props' />"
    "    </signal>"
    "    <signal name='UpdateProperty'>"
    "      <arg type='v' name='prop' />"
    "    </signal>"
    "    <signal name='ForwardKeyEvent'>"
    "      <arg type='u' name='keyval' />"
    "      <arg type='u' name='keycode' />"
    "      <arg type='u' name='state' />"
    "    </signal>"
    "  </interface>"
    "</node>";

static void
ibus_engine_class_init (IBusEngineClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (class);

    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_engine_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_engine_get_property;

    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_engine_destroy;

    IBUS_SERVICE_CLASS (class)->service_method_call  = ibus_engine_service_method_call;
    IBUS_SERVICE_CLASS (class)->service_get_property = ibus_engine_service_get_property;
    IBUS_SERVICE_CLASS (class)->service_set_property = ibus_engine_service_set_property;

    ibus_service_class_add_interfaces (IBUS_SERVICE_CLASS (class), introspection_xml);

    class->process_key_event = ibus_engine_process_key_event;
    class->focus_in     = ibus_engine_focus_in;
    class->focus_out    = ibus_engine_focus_out;
    class->reset        = ibus_engine_reset;
    class->enable       = ibus_engine_enable;
    class->disable      = ibus_engine_disable;
    class->page_up      = ibus_engine_page_up;
    class->page_down    = ibus_engine_page_down;
    class->cursor_up    = ibus_engine_cursor_up;
    class->cursor_down  = ibus_engine_cursor_down;
    class->candidate_clicked    = ibus_engine_candidate_clicked;
    class->property_activate    = ibus_engine_property_activate;
    class->property_show        = ibus_engine_property_show;
    class->property_hide        = ibus_engine_property_hide;
    class->set_cursor_location  = ibus_engine_set_cursor_location;
    class->set_capabilities     = ibus_engine_set_capabilities;
    class->set_surrounding_text = ibus_engine_set_surrounding_text;
    class->process_hand_writing_event
                                = ibus_engine_process_hand_writing_event;
    class->cancel_hand_writing  = ibus_engine_cancel_hand_writing;

    /* install properties */
    /**
     * IBusEngine:name:
     *
     * Name of this IBusEngine.
     */
    g_object_class_install_property (gobject_class,
                    PROP_ENGINE_NAME,
                    g_param_spec_string ("engine-name",
                        "engine name",
                        "engine name",
                        "noname",
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT_ONLY |
                        G_PARAM_STATIC_STRINGS));

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
            g_signal_accumulator_true_handled, NULL,
            _ibus_marshal_BOOL__UINT_UINT_UINT,
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
            _ibus_marshal_VOID__VOID,
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
            _ibus_marshal_VOID__VOID,
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
            _ibus_marshal_VOID__VOID,
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
            _ibus_marshal_VOID__VOID,
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
            _ibus_marshal_VOID__VOID,
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
            _ibus_marshal_VOID__INT_INT_INT_INT,
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
            _ibus_marshal_VOID__UINT,
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
            _ibus_marshal_VOID__VOID,
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
            _ibus_marshal_VOID__VOID,
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
            _ibus_marshal_VOID__VOID,
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
            _ibus_marshal_VOID__VOID,
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
            _ibus_marshal_VOID__UINT_UINT_UINT,
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
            _ibus_marshal_VOID__STRING_UINT,
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
            _ibus_marshal_VOID__STRING,
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
            _ibus_marshal_VOID__STRING,
            G_TYPE_NONE,
            1,
            G_TYPE_STRING);

    /**
     * IBusEngine::process-hand-writing-event:
     * @engine: An IBusEngine.
     * @coordinates: An array of double (0.0 to 1.0) which represents a stroke (i.e. [x1, y1, x2, y2, x3, y3, ...]).
     * @coordinates_len: The number of elements in the array.
     *
     * Emitted when a hand writing operation is cancelled.
     * Implement the member function cancel_hand_writing() in extended class to receive this signal.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
     */
    engine_signals[PROCESS_HAND_WRITING_EVENT] =
        g_signal_new (I_("process-hand-writing-event"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, process_hand_writing_event),
            NULL, NULL,
            _ibus_marshal_VOID__POINTER_UINT,
            G_TYPE_NONE,
            2,
            G_TYPE_POINTER,
            G_TYPE_UINT);

    /**
     * IBusEngine::cancel-hand-writing:
     * @engine: An IBusEngine.
     * @n_strokes: The number of strokes to be removed. 0 means "remove all".
     *
     * Emitted when a hand writing operation is cancelled.
     * Implement the member function cancel_hand_writing() in extended class to receive this signal.
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
     */
    engine_signals[CANCEL_HAND_WRITING] =
        g_signal_new (I_("cancel-hand-writing"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, cancel_hand_writing),
            NULL, NULL,
            _ibus_marshal_VOID__UINT,
            G_TYPE_NONE,
            1,
            G_TYPE_UINT);

    g_type_class_add_private (class, sizeof (IBusEnginePrivate));

    /**
     * IBusEngine::set-surrounding-text:
     * @engine: An IBusEngine.
     * @text: The surrounding text.
     * @cursor_pos: The cursor position on surrounding text.
     * @anchor_pos: The anchor position on selection area.
     *
     * Emitted when a surrounding text is set.
     * Implement the member function set_surrounding_text() in extended class to receive this signal.
     * If anchor_pos equals to cursor_pos, it means "there are no selection" or "does not support
     * selection retrival".
     *
     * <note><para>Argument @user_data is ignored in this function.</para></note>
     */
    engine_signals[SET_SURROUNDING_TEXT] =
        g_signal_new (I_("set-surrounding-text"),
            G_TYPE_FROM_CLASS (gobject_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (IBusEngineClass, set_surrounding_text),
            NULL, NULL,
            _ibus_marshal_VOID__OBJECT_UINT_UINT,
            G_TYPE_NONE,
            3,
            G_TYPE_OBJECT,
            G_TYPE_UINT,
            G_TYPE_UINT);

    text_empty = ibus_text_new_from_static_string ("");
    g_object_ref_sink (text_empty);
}

static void
ibus_engine_init (IBusEngine *engine)
{
    engine->priv = IBUS_ENGINE_GET_PRIVATE (engine);

    engine->priv->surrounding_text = g_object_ref_sink (text_empty);
}

static void
ibus_engine_destroy (IBusEngine *engine)
{
    g_free (engine->priv->engine_name);
    engine->priv->engine_name = NULL;

    if (engine->priv->surrounding_text) {
        g_object_unref (engine->priv->surrounding_text);
        engine->priv->surrounding_text = NULL;
    }

    IBUS_OBJECT_CLASS(ibus_engine_parent_class)->destroy (IBUS_OBJECT (engine));
}

static void
ibus_engine_set_property (IBusEngine   *engine,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    switch (prop_id) {
    case PROP_ENGINE_NAME:
        engine->priv->engine_name = g_value_dup_string (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (engine, prop_id, pspec);
    }
}

static void
ibus_engine_get_property (IBusEngine *engine,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    switch (prop_id) {
    case PROP_ENGINE_NAME:
        g_value_set_string (value, engine->priv->engine_name);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (engine, prop_id, pspec);
    }
}

static void
ibus_engine_service_method_call (IBusService           *service,
                                 GDBusConnection       *connection,
                                 const gchar           *sender,
                                 const gchar           *object_path,
                                 const gchar           *interface_name,
                                 const gchar           *method_name,
                                 GVariant              *parameters,
                                 GDBusMethodInvocation *invocation)
{
    IBusEngine *engine = IBUS_ENGINE (service);

    if (g_strcmp0 (interface_name, IBUS_INTERFACE_ENGINE) != 0) {
        IBUS_SERVICE_CLASS (ibus_engine_parent_class)->
                service_method_call (service,
                                     connection,
                                     sender,
                                     object_path,
                                     interface_name,
                                     method_name,
                                     parameters,
                                     invocation);
        return;
    }

    if (g_strcmp0 (method_name, "ProcessKeyEvent") == 0) {
        guint keyval, keycode, state;
        gboolean retval = FALSE;
        g_variant_get (parameters, "(uuu)", &keyval, &keycode, &state);
        g_signal_emit (engine,
                       engine_signals[PROCESS_KEY_EVENT],
                       0,
                       keyval,
                       keycode,
                       state,
                       &retval);
        g_dbus_method_invocation_return_value (invocation, g_variant_new ("(b)", retval));
        return;
    }

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

    gint i;
    for (i = 0; i < G_N_ELEMENTS (no_arg_methods); i++) {
        if (g_strcmp0 (method_name, no_arg_methods[i].member) == 0) {
            g_signal_emit (engine, engine_signals[no_arg_methods[i].signal_id], 0);
            g_dbus_method_invocation_return_value (invocation, NULL);
            return;
        }
    }

    if (g_strcmp0 (method_name, "CandidateClicked") == 0) {
        guint index, button, state;
        g_variant_get (parameters, "(uuu)", &index, &button, &state);
        g_signal_emit (engine,
                       engine_signals[CANDIDATE_CLICKED],
                       0,
                       index,
                       button,
                       state);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "PropertyActivate") == 0) {
        gchar *name;
        guint state;
        g_variant_get (parameters, "(&su)", &name, &state);
        g_signal_emit (engine,
                       engine_signals[PROPERTY_ACTIVATE],
                       0,
                       name,
                       state);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "PropertyShow") == 0) {
        gchar *name;
        g_variant_get (parameters, "(&s)", &name);
        g_signal_emit (engine,
                       engine_signals[PROPERTY_SHOW],
                       0,
                       name);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "PropertyHide") == 0) {
        gchar *name;
        g_variant_get (parameters, "(&s)", &name);
        g_signal_emit (engine,
                       engine_signals[PROPERTY_HIDE],
                       0,
                       name);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "SetCursorLocation") == 0) {
        gint x, y, w, h;
        g_variant_get (parameters, "(iiii)", &x, &y, &w, &h);
        engine->cursor_area.x = x;
        engine->cursor_area.y = y;
        engine->cursor_area.width = w;
        engine->cursor_area.height = h;

        g_signal_emit (engine,
                       engine_signals[SET_CURSOR_LOCATION],
                       0,
                       x, y, w, h);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "SetCapabilities") == 0) {
        guint caps;
        g_variant_get (parameters, "(u)", &caps);
        engine->client_capabilities = caps;
        g_signal_emit (engine, engine_signals[SET_CAPABILITIES], 0, caps);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "SetSurroundingText") == 0) {
        GVariant *variant = NULL;
        IBusText *text;
        guint cursor_pos;
        guint anchor_pos;

        g_variant_get (parameters,
                       "(vuu)",
                       &variant,
                       &cursor_pos,
                       &anchor_pos);
        text = IBUS_TEXT (ibus_serializable_deserialize (variant));
        g_variant_unref (variant);

        g_signal_emit (engine, engine_signals[SET_SURROUNDING_TEXT],
                       0,
                       text,
                       cursor_pos,
                       anchor_pos);
        if (g_object_is_floating (text)) {
            g_object_unref (text);
        }
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "ProcessHandWritingEvent") == 0) {
        const gdouble *coordinates;
        gsize coordinates_len = 0;

        coordinates = g_variant_get_fixed_array (g_variant_get_child_value (parameters, 0), &coordinates_len, sizeof (gdouble));
        g_return_if_fail (coordinates != NULL);
        g_return_if_fail (coordinates_len >= 4); /* The array should contain at least one line. */
        g_return_if_fail (coordinates_len <= G_MAXUINT); /* to prevent overflow in the cast in g_signal_emit */
        g_return_if_fail ((coordinates_len & 1) == 0);

        g_signal_emit (engine, engine_signals[PROCESS_HAND_WRITING_EVENT], 0,
                       coordinates, (guint) coordinates_len);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "CancelHandWriting") == 0) {
        guint n_strokes = 0;
        g_variant_get (parameters, "(u)", &n_strokes);
        g_signal_emit (engine, engine_signals[CANCEL_HAND_WRITING], 0, n_strokes);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    /* should not be reached */
    g_return_if_reached ();
}

static GVariant *
ibus_engine_service_get_property (IBusService        *service,
                                  GDBusConnection    *connection,
                                  const gchar        *sender,
                                  const gchar        *object_path,
                                  const gchar        *interface_name,
                                  const gchar        *property_name,
                                  GError            **error)
{
    return IBUS_SERVICE_CLASS (ibus_engine_parent_class)->
                service_get_property (service,
                                      connection,
                                      sender,
                                      object_path,
                                      interface_name,
                                      property_name,
                                      error);
}

static gboolean
ibus_engine_service_set_property (IBusService        *service,
                                  GDBusConnection    *connection,
                                  const gchar        *sender,
                                  const gchar        *object_path,
                                  const gchar        *interface_name,
                                  const gchar        *property_name,
                                  GVariant           *value,
                                  GError            **error)
{
    return IBUS_SERVICE_CLASS (ibus_engine_parent_class)->
                service_set_property (service,
                                      connection,
                                      sender,
                                      object_path,
                                      interface_name,
                                      property_name,
                                      value,
                                      error);
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
ibus_engine_set_surrounding_text (IBusEngine *engine,
                                  IBusText   *text,
                                  guint       cursor_pos,
                                  guint       anchor_pos)
{
    g_assert (IBUS_IS_ENGINE (engine));

    if (engine->priv->surrounding_text) {
        g_object_unref (engine->priv->surrounding_text);
    }

    engine->priv->surrounding_text = (IBusText *) g_object_ref_sink (text ? text : text_empty);
    engine->priv->surrounding_cursor_pos = cursor_pos;
    engine->priv->selection_anchor_pos = anchor_pos;
    // g_debug ("set-surrounding-text ('%s', %d, %d)", text->text, cursor_pos, anchor_pos);
}

static void
ibus_engine_process_hand_writing_event (IBusEngine         *engine,
                                        const gdouble      *coordinates,
                                        guint               coordinates_len)
{
    // guint i;
    // g_debug ("process-hand-writing-event (%u)", coordinates_len);
    // for (i = 0; i < coordinates_len; i++)
    //     g_debug (" %lf", coordinates[i]);
}

static void
ibus_engine_cancel_hand_writing (IBusEngine         *engine,
                                 guint               n_strokes)
{
    // g_debug ("cancel-hand-writing (%u)", n_strokes);
}

static void
ibus_engine_emit_signal (IBusEngine  *engine,
                         const gchar *signal_name,
                         GVariant    *parameters)
{
    ibus_service_emit_signal ((IBusService *)engine,
                              NULL,
                              IBUS_INTERFACE_ENGINE,
                              signal_name,
                              parameters,
                              NULL);
}

IBusEngine *
ibus_engine_new (const gchar     *engine_name,
                 const gchar     *object_path,
                 GDBusConnection *connection)
{
    return ibus_engine_new_with_type (IBUS_TYPE_ENGINE,
                                      engine_name,
                                      object_path,
                                      connection);
}

IBusEngine  *
ibus_engine_new_with_type (GType            engine_type,
                           const gchar     *engine_name,
                           const gchar     *object_path,
                           GDBusConnection *connection)
{
    g_return_val_if_fail (g_type_is_a (engine_type, IBUS_TYPE_ENGINE), NULL);
    g_return_val_if_fail (engine_name != NULL, NULL);
    g_return_val_if_fail (object_path != NULL, NULL);
    g_return_val_if_fail (G_IS_DBUS_CONNECTION (connection), NULL);

    GObject *object = g_object_new (engine_type,
                                    "engine-name", engine_name,
                                    "object-path", object_path,
                                    "connection", connection,
                                    NULL);
    return IBUS_ENGINE (object);
}


void
ibus_engine_commit_text (IBusEngine *engine,
                         IBusText   *text)
{
    g_return_if_fail (IBUS_IS_ENGINE (engine));
    g_return_if_fail (IBUS_IS_TEXT (text));

    GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)text);
    ibus_engine_emit_signal (engine,
                             "CommitText",
                             g_variant_new ("(v)", variant));

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
    g_return_if_fail (IBUS_IS_ENGINE (engine));
    g_return_if_fail (IBUS_IS_TEXT (text));

    GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)text);
    ibus_engine_emit_signal (engine,
                             "UpdatePreeditText",
                             g_variant_new ("(vubu)", variant, cursor_pos, visible, mode));

    if (g_object_is_floating (text)) {
        g_object_unref (text);
    }
}

void ibus_engine_update_auxiliary_text (IBusEngine      *engine,
                                        IBusText        *text,
                                        gboolean         visible)
{
    g_return_if_fail (IBUS_IS_ENGINE (engine));
    g_return_if_fail (IBUS_IS_TEXT (text));

    GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)text);
    ibus_engine_emit_signal (engine,
                             "UpdateAuxiliaryText",
                             g_variant_new ("(vb)", variant, visible));

    if (g_object_is_floating (text)) {
        g_object_unref (text);
    }
}


void
ibus_engine_update_lookup_table (IBusEngine        *engine,
                                 IBusLookupTable   *table,
                                 gboolean           visible)
{
    g_return_if_fail (IBUS_IS_ENGINE (engine));
    g_return_if_fail (IBUS_IS_LOOKUP_TABLE (table));

    GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)table);
    ibus_engine_emit_signal (engine,
                             "UpdateLookupTable",
                             g_variant_new ("(vb)", variant, visible));

    if (g_object_is_floating (table)) {
        g_object_unref (table);
    }
}

void
ibus_engine_update_lookup_table_fast (IBusEngine        *engine,
                                      IBusLookupTable   *table,
                                      gboolean           visible)
{
    g_return_if_fail (IBUS_IS_ENGINE (engine));
    g_return_if_fail (IBUS_IS_LOOKUP_TABLE (table));

    IBusLookupTable *new_table;
    IBusText *text;
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

    for (i = 0; (text = ibus_lookup_table_get_label (table, i)) != NULL; i++) {
        ibus_lookup_table_append_label (new_table, text);
    }

    ibus_lookup_table_set_cursor_pos (new_table, ibus_lookup_table_get_cursor_in_page (table));
    ibus_lookup_table_set_orientation (new_table, ibus_lookup_table_get_orientation (table));

    ibus_engine_update_lookup_table (engine, new_table, visible);

    if (g_object_is_floating (table)) {
        g_object_unref (table);
    }
}

void
ibus_engine_forward_key_event (IBusEngine      *engine,
                               guint            keyval,
                               guint            keycode,
                               guint            state)
{
    g_return_if_fail (IBUS_IS_ENGINE (engine));

    ibus_engine_emit_signal (engine,
                             "ForwardKeyEvent",
                             g_variant_new ("(uuu)", keyval, keycode, state));
}

void ibus_engine_delete_surrounding_text (IBusEngine      *engine,
                                          gint             offset_from_cursor,
                                          guint            nchars)
{
    IBusEnginePrivate *priv;

    g_return_if_fail (IBUS_IS_ENGINE (engine));

    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    /* Update surrounding-text cache.  This is necessary since some
       engines call ibus_engine_get_surrounding_text() immediately
       after ibus_engine_delete_surrounding_text(). */
    if (priv->surrounding_text) {
        IBusText *text;
        glong cursor_pos, len;

        cursor_pos = priv->surrounding_cursor_pos + offset_from_cursor;
        len = ibus_text_get_length (priv->surrounding_text);
        if (cursor_pos >= 0 && len - cursor_pos >= nchars) {
            gunichar *ucs;

            ucs = g_utf8_to_ucs4_fast (priv->surrounding_text->text,
                                       -1,
                                       NULL);
            memmove (&ucs[cursor_pos],
                     &ucs[cursor_pos + nchars],
                     sizeof(gunichar) * (len - cursor_pos - nchars));
            ucs[len - nchars] = 0;
            text = ibus_text_new_from_ucs4 (ucs);
            g_free (ucs);
            priv->surrounding_cursor_pos = cursor_pos;
        } else {
            text = text_empty;
            priv->surrounding_cursor_pos = 0;
        }

        g_object_unref (priv->surrounding_text);
        priv->surrounding_text = g_object_ref_sink (text);
    }

    ibus_engine_emit_signal (engine,
                             "DeleteSurroundingText",
                             g_variant_new ("(iu)", offset_from_cursor, nchars));
}

void
ibus_engine_get_surrounding_text (IBusEngine   *engine,
                                  IBusText    **text,
                                  guint        *cursor_pos,
                                  guint        *anchor_pos)
{
    IBusEnginePrivate *priv;

    g_return_if_fail (IBUS_IS_ENGINE (engine));
    const gboolean signal_only = (text == NULL);

    g_return_if_fail (( signal_only && (cursor_pos == NULL)) ||
                      (!signal_only && (cursor_pos != NULL)));

    g_return_if_fail (( signal_only && (anchor_pos == NULL)) ||
                      (!signal_only && (anchor_pos != NULL)));

    priv = IBUS_ENGINE_GET_PRIVATE (engine);

    if (!signal_only) {
        *text = g_object_ref (priv->surrounding_text);
        *cursor_pos = priv->surrounding_cursor_pos;
        *anchor_pos = priv->selection_anchor_pos;
    }

    /* tell the client that this engine will utilize surrounding-text
     * feature, which causes periodical update.  Note that the client
     * should request the initial surrounding-text when the engine is
     * enabled (see ibus_im_context_focus_in() and
     * _ibus_context_enabled_cb() in client/gtk2/ibusimcontext.c). */
    ibus_engine_emit_signal (engine,
                             "RequireSurroundingText",
                             NULL);

    // g_debug ("get-surrounding-text ('%s', %d, %d)", (*text)->text, *cursor_pos, *anchor_pos);
}

void
ibus_engine_register_properties (IBusEngine   *engine,
                                 IBusPropList *prop_list)
{
    g_return_if_fail (IBUS_IS_ENGINE (engine));
    g_return_if_fail (IBUS_IS_PROP_LIST (prop_list));

    GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)prop_list);
    ibus_engine_emit_signal (engine,
                             "RegisterProperties",
                             g_variant_new ("(v)", variant));

    if (g_object_is_floating (prop_list)) {
        g_object_unref (prop_list);
    }
}

void
ibus_engine_update_property (IBusEngine   *engine,
                             IBusProperty *prop)
{
    g_return_if_fail (IBUS_IS_ENGINE (engine));
    g_return_if_fail (IBUS_IS_PROPERTY (prop));

    GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)prop);
    ibus_engine_emit_signal (engine,
                             "UpdateProperty",
                             g_variant_new ("(v)", variant));

    if (g_object_is_floating (prop)) {
        g_object_unref (prop);
    }
}

#define DEFINE_FUNC(name, Name)                             \
    void                                                    \
    ibus_engine_##name (IBusEngine *engine)                 \
    {                                                       \
        g_return_if_fail (IBUS_IS_ENGINE (engine));         \
        ibus_engine_emit_signal (engine,                    \
                              #Name,                        \
                              NULL);                        \
    }
DEFINE_FUNC (show_preedit_text, ShowPreeditText)
DEFINE_FUNC (hide_preedit_text, HidePreeditText)
DEFINE_FUNC (show_auxiliary_text, ShowAuxiliaryText)
DEFINE_FUNC (hide_auxiliary_text, HideAuxiliaryText)
DEFINE_FUNC (show_lookup_table, ShowLookupTable)
DEFINE_FUNC (hide_lookup_table, HideLookupTable)
#undef DEFINE_FUNC

const gchar *
ibus_engine_get_name (IBusEngine *engine)
{
    g_return_val_if_fail (IBUS_IS_ENGINE (engine), NULL);
    return engine->priv->engine_name;
}
