#include <gtk/gtk.h>
#include "ibus.h"
#include "ibuscomposetable.h"

IBusBus *m_bus;
IBusComposeTable *m_compose_table;
IBusEngine *m_engine;
int m_retval;

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
    gchar *compose_path = NULL;
    const gchar * const *langs;
    const gchar * const *l;

    m_engine = ibus_engine_new_with_type (IBUS_TYPE_ENGINE_SIMPLE,
                                          name,
                                          engine_path,
                                          ibus_bus_get_connection (m_bus));
    g_free (engine_path);
    langs = g_get_language_names ();
    for (l = langs; *l; l++) {
        if (g_str_has_prefix (*l, "en_US"))
            break;
        if (g_strcmp0 (*l, "C") == 0)
            break;
        compose_path = g_build_filename ("/usr/share/X11/locale",
                                         *l,
                                         "Compose",
                                         NULL);
        if (g_file_test (compose_path, G_FILE_TEST_EXISTS))
            break;
        g_free (compose_path);
        compose_path = NULL;
    }
    if (compose_path != NULL) {
        m_compose_table = ibus_compose_table_new_with_file (compose_path);
        if (m_compose_table == NULL)
            g_warning ("Your locale uses en_US compose table.");
        else
            ibus_engine_simple_add_table (IBUS_ENGINE_SIMPLE (m_engine),
                                          m_compose_table->data,
                                          m_compose_table->max_seq_len,
                                          m_compose_table->n_seqs);
    } else {
        g_warning ("Your locale uses en_US compose file.");
    }
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
    m_retval = -1;
    g_warning ("time out");
    gtk_main_quit ();
    return FALSE;
}

static void
set_engine_cb (GObject *object, GAsyncResult *res, gpointer data)
{
    IBusBus *bus = IBUS_BUS (object);
    GtkWidget *entry = GTK_WIDGET (data);
    GError *error = NULL;
    int i, j;
    int index_stride;

    if (!ibus_bus_set_global_engine_async_finish (bus, res, &error)) {
        g_warning ("set engine failed: %s", error->message);
        g_error_free (error);
        return;
    }

    if (m_compose_table == NULL) {
        gtk_main_quit ();
        return;
    }

    index_stride = m_compose_table->max_seq_len + 2;
    for (i = 0;
         i < (m_compose_table->n_seqs * index_stride);
         i += index_stride) {
        for (j = i; j < i + (index_stride - 1); j++) {
            guint keyval = m_compose_table->data[j];
            guint keycode = 0;
            guint modifiers = 0;
            gboolean retval;

            if (keyval == 0)
                break;
            g_signal_emit_by_name (m_engine, "process-key-event",
                                   keyval, keycode, modifiers, &retval);
            modifiers |= IBUS_RELEASE_MASK;
            g_signal_emit_by_name (m_engine, "process-key-event",
                                   keyval, keycode, modifiers, &retval);
        }
    }

    g_signal_handlers_disconnect_by_func (entry, 
                                          G_CALLBACK (window_focus_in_event_cb),
                                          NULL);
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
    static int n_loop = 0;
    static guint stride = 0;
    guint i;
    int seq;
    gunichar code = g_utf8_get_char (chars);
    const gchar *test;
    GtkEntry *entry = GTK_ENTRY (data);

    g_assert (m_compose_table != NULL);

    if (n_loop % 2 == 1) {
        n_loop = 0;
        return;
    }
    i = stride + (m_compose_table->max_seq_len + 2) - 1;
    seq = (i + 1) / (m_compose_table->max_seq_len + 2);
    if (m_compose_table->data[i] == code) {
        test = "OK";
    } else {
        test = "NG";
        m_retval = -1;
    }
    g_print ("%05d/%05d %s expected: %04X typed: %04X\n",
             seq,
             m_compose_table->n_seqs,
             test,
             m_compose_table->data[i],
             code);

    if (seq == m_compose_table->n_seqs) {
        gtk_main_quit ();
        return;
    }

    stride += m_compose_table->max_seq_len + 2;
    n_loop++;
    gtk_entry_set_text (entry, "");
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

int
main (int argc, char *argv[])
{
    ibus_init ();
    gtk_init (&argc, &argv);

    if (!register_ibus_engine ())
        return -1;

    create_window ();
    gtk_main ();

    return m_retval;
}
