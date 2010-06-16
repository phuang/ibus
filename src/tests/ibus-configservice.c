#include <ibus.h>

static gboolean
timeout_cb (gpointer data)
{
    g_main_loop_quit ((GMainLoop *)data);
    return FALSE;
}

static void
run_loop_with_timeout (gint interval)
{
    GMainLoop *loop = g_main_loop_new (NULL, FALSE);
    g_timeout_add (interval, timeout_cb, loop);
    g_main_loop_run (loop);
    g_main_loop_unref (loop);
}

static void
test_configservice (void)
{
    IBusBus *bus = ibus_bus_new ();
    IBusConfigService *config = ibus_config_service_new (ibus_bus_get_connection (bus));
    ibus_bus_request_name (bus, "test.config", 0);

    run_loop_with_timeout (1000);

    g_object_unref (config);
    g_object_unref (bus);
}

gint
main (gint    argc,
      gchar **argv)
{
    g_mem_set_vtable (glib_mem_profiler_table);
    g_type_init ();
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/ibus/configservice", test_configservice);

    return g_test_run ();
}
