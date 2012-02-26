#include "ibus.h"

static void
test_keyname (void)
{
    g_assert_cmpstr (ibus_keyval_name (IBUS_KEY_Home), ==, "Home");
    g_assert (ibus_keyval_from_name ("Home") == IBUS_KEY_Home);
}

gint
main (gint    argc,
      gchar **argv)
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/ibus/keyname", test_keyname);

    return g_test_run ();
}
