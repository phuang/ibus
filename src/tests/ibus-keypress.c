#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "ibus.h"
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#define GREEN "\033[0;32m"
#define RED   "\033[0;31m"
#define NC    "\033[0m"

typedef struct _KeyData {
    guint keyval;
    guint modifiers;
} KeyData;

static const KeyData test_cases[][30] = {
   { { IBUS_KEY_a, 0 }, { IBUS_KEY_comma, IBUS_SHIFT_MASK },
     { IBUS_KEY_b, 0 }, { IBUS_KEY_period, IBUS_SHIFT_MASK },
     { IBUS_KEY_c, 0 }, { IBUS_KEY_slash, IBUS_SHIFT_MASK },
     { IBUS_KEY_d, 0 }, { IBUS_KEY_semicolon, IBUS_SHIFT_MASK },
     { IBUS_KEY_e, 0 }, { IBUS_KEY_apostrophe, IBUS_SHIFT_MASK },
     { IBUS_KEY_f, 0 }, { IBUS_KEY_bracketleft, IBUS_SHIFT_MASK },
     { IBUS_KEY_g, 0 }, { IBUS_KEY_backslash, IBUS_SHIFT_MASK },
     { 0, 0 } },
   { { IBUS_KEY_grave, IBUS_SHIFT_MASK }, { IBUS_KEY_a, IBUS_SHIFT_MASK },
     { IBUS_KEY_1, IBUS_SHIFT_MASK }, { IBUS_KEY_b, IBUS_SHIFT_MASK  },
     { IBUS_KEY_2, IBUS_SHIFT_MASK }, { IBUS_KEY_c, IBUS_SHIFT_MASK  },
     { IBUS_KEY_3, IBUS_SHIFT_MASK }, { IBUS_KEY_d, IBUS_SHIFT_MASK },
     { IBUS_KEY_9, IBUS_SHIFT_MASK }, { IBUS_KEY_e, IBUS_SHIFT_MASK },
     { IBUS_KEY_0, IBUS_SHIFT_MASK }, { IBUS_KEY_f, IBUS_SHIFT_MASK },
     { IBUS_KEY_equal, IBUS_SHIFT_MASK }, { IBUS_KEY_g, IBUS_SHIFT_MASK },
     { 0, 0 } },
   { { 0, 0 } }
};

KeyData test_end_key = { IBUS_KEY_z, IBUS_SHIFT_MASK };

static const gunichar test_results[][60] = {
   { 'a', '<', 'b', '>', 'c', '?', 'd', ':', 'e', '"', 'f', '{', 'g', '|', 0 },
   { '~', 'A', '!', 'B', '@', 'C', '#', 'D', '(', 'E', ')', 'F', '+', 'G', 0 },
   { 0 }
};


IBusBus *m_bus;
IBusEngine *m_engine;

static gboolean window_focus_in_event_cb (GtkWidget     *entry,
                                          GdkEventFocus *event,
                                          gpointer       data);

static IBusEngine *
create_engine_cb (IBusFactory *factory, const gchar *name, gpointer data)
{
    static int i = 1;
    gchar *engine_path =
            g_strdup_printf ("/org/freedesktop/IBus/engine/simpletest/%d",
                             i++);

    m_engine = ibus_engine_new_with_type (IBUS_TYPE_ENGINE_SIMPLE,
                                          name,
                                          engine_path,
                                          ibus_bus_get_connection (m_bus));
    g_free (engine_path);
    return m_engine;
}

static gboolean
register_ibus_engine ()
{
    IBusFactory *factory;
    IBusComponent *component;
    IBusEngineDesc *desc;

    m_bus = ibus_bus_new ();
    if (!ibus_bus_is_connected (m_bus)) {
        g_critical ("ibus-daemon is not running.");
        return FALSE;
    }
    factory = ibus_factory_new (ibus_bus_get_connection (m_bus));
    g_signal_connect (factory, "create-engine",
                      G_CALLBACK (create_engine_cb), NULL);

    component = ibus_component_new (
            "org.freedesktop.IBus.SimpleTest",
            "Simple Engine Test",
            "0.0.1",
            "GPL",
            "Takao Fujiwara <takao.fujiwara1@gmail.com>",
            "https://github.com/ibus/ibus/wiki",
            "",
            "ibus");
    desc = ibus_engine_desc_new (
            "xkbtest:us::eng",
            "XKB Test",
            "XKB Test",
            "en",
            "GPL",
            "Takao Fujiwara <takao.fujiwara1@gmail.com>",
            "ibus-engine",
            "us");
    ibus_component_add_engine (component, desc);
    ibus_bus_register_component (m_bus, component);

    return TRUE;
}

static gboolean
finit (gpointer data)
{
    g_critical ("time out");
    gtk_main_quit ();
    return FALSE;
}

static void
send_key_event (Display *xdisplay,
                guint    keyval,
                guint    modifiers)
{
    static struct {
        guint   state;
        KeySym  keysym;
    } state2keysym[] = {
        { IBUS_CONTROL_MASK, XK_Control_L } ,
        { IBUS_MOD1_MASK,    XK_Alt_L },
        { IBUS_MOD4_MASK,    XK_Super_L },
        { IBUS_SHIFT_MASK,   XK_Shift_L },
        { IBUS_LOCK_MASK,    XK_Caps_Lock },
        { 0,           0L }
    };
    int i;
    guint keycode;
    guint state = modifiers;

    while (state) {
        for (i = 0; state2keysym[i].state; i++) {
            if ((state2keysym[i].state & state) != 0) {
                keycode = XKeysymToKeycode (xdisplay, state2keysym[i].keysym);
                XTestFakeKeyEvent (xdisplay, keycode, True, CurrentTime);
                XSync (xdisplay, False);
                state ^= state2keysym[i].state;
                break;
            }
        }
    }
    keycode = XKeysymToKeycode (xdisplay, keyval);
    XTestFakeKeyEvent (xdisplay, keycode, True, CurrentTime);
    XSync (xdisplay, False);
    XTestFakeKeyEvent (xdisplay, keycode, False, CurrentTime);
    XSync (xdisplay, False);

    state = modifiers;
    while (state) {
        for (i = G_N_ELEMENTS (state2keysym) - 1; i >= 0; i--) {
            if ((state2keysym[i].state & state) != 0) {
                keycode = XKeysymToKeycode (xdisplay, state2keysym[i].keysym);
                XTestFakeKeyEvent (xdisplay, keycode, False, CurrentTime);
                XSync (xdisplay, False);
                state ^= state2keysym[i].state;
                break;
            }
        }
    }
}

static void
set_engine_cb (GObject      *object,
               GAsyncResult *res,
               gpointer      data)
{
    IBusBus *bus = IBUS_BUS (object);
    GtkWidget *entry = GTK_WIDGET (data);
    GdkDisplay *display;
    Display *xdisplay;
    GError *error = NULL;
    int i, j;

    g_assert (GTK_IS_ENTRY (entry));

    if (!ibus_bus_set_global_engine_async_finish (bus, res, &error)) {
        g_critical ("set engine failed: %s", error->message);
        g_error_free (error);
        return;
    }

    display = gtk_widget_get_display (entry);
    if (GDK_IS_X11_DISPLAY (display)) {
        xdisplay = gdk_x11_display_get_xdisplay (display);
    } else {
#if 0
        xdisplay = XOpenDisplay (NULL);
#else
        g_critical ("No idea to simulate key events in Wayland\n");
#endif
    }
    g_return_if_fail (xdisplay);

    for (i = 0; test_cases[i][0].keyval; i++) {
        for (j = 0; test_cases[i][j].keyval; j++) {
            send_key_event (xdisplay,
                            test_cases[i][j].keyval,
                            test_cases[i][j].modifiers);
        }
        send_key_event (xdisplay, test_end_key.keyval, test_end_key.modifiers);
    }

    g_timeout_add_seconds (10, finit, NULL);
}

static gboolean
window_focus_in_event_cb (GtkWidget *entry, GdkEventFocus *event, gpointer data)
{
    g_assert (m_bus != NULL);
    ibus_bus_set_global_engine_async (m_bus,
                                      "xkbtest:us::eng",
                                      -1,
                                      NULL,
                                      set_engine_cb,
                                      entry);
    return FALSE;
}

static void
window_inserted_text_cb (GtkEntryBuffer *buffer,
                         guint           position,
                         const gchar    *chars,
                         guint           nchars,
                         gpointer        data)
{
    GtkWidget *entry = data;
    static int i = 0;
    static int j = 0;

    if (g_utf8_get_char (chars) == 'Z') {
        int k;
        g_print ("\n" GREEN "PASS" NC ": ");
        for (k = 0; k < j; k++)
            g_print ("%lc(%X) ", test_results[i][k], test_results[i][k]);
        g_print ("\n");
        i++;
        j = 0;
        if (test_results[i][0] == 0)
            gtk_main_quit ();
        else
            gtk_entry_set_text (GTK_ENTRY (entry), "");
        return;
    }
    g_assert (g_utf8_get_char (chars) == test_results[i][j]);
    j++;
}

static void
create_window ()
{
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    GtkWidget *entry = gtk_entry_new ();
    GtkEntryBuffer *buffer;

    g_signal_connect (window, "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (entry, "focus-in-event",
                      G_CALLBACK (window_focus_in_event_cb), NULL);
    buffer = gtk_entry_get_buffer (GTK_ENTRY (entry));
    g_signal_connect (buffer, "inserted-text",
                      G_CALLBACK (window_inserted_text_cb), entry);
    gtk_container_add (GTK_CONTAINER (window), entry);
    gtk_widget_show_all (window);
}

static void
test_keypress (void)
{
    int status = 0;
    GError *error = NULL;

    g_spawn_command_line_sync ("setxkbmap -layout us",
                               NULL, NULL,
                               &status, &error);
    g_assert (register_ibus_engine ());

    create_window ();
    gtk_main ();
}

int
main (int argc, char *argv[])
{
    ibus_init ();
    g_test_init (&argc, &argv, NULL);
    gtk_init (&argc, &argv);

    g_test_add_func ("/ibus/keyrepss", test_keypress);


    return g_test_run ();
}
