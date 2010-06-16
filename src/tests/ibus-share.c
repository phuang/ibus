#include <ibus.h>
#include <dbus/dbus.h>

static void
test_machine_id (void)
{
    const gchar *s1 = ibus_get_local_machine_id ();
    gchar *s2 = dbus_get_local_machine_id ();

    g_assert_cmpstr (s1, ==, s2);
    dbus_free (s2);
}

gint
main (gint    argc,
      gchar **argv)
{
    g_mem_set_vtable (glib_mem_profiler_table);
	g_type_init ();
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/ibus/marchine-id", test_machine_id);

    return g_test_run ();
}
