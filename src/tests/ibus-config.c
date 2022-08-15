/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */

#include <string.h>
#include "ibus.h"

static IBusBus *bus = NULL;
static int create_config_count = 0;

static void
finish_create_config_async_success (GObject      *source_object,
                                    GAsyncResult *res,
                                    gpointer      user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    GError *error = NULL;
    IBusConfig *config =
          ibus_config_new_async_finish (res, &error);

    if (error) {
        g_message ("Failed to generate IBusConfig: %s", error->message);
        g_error_free (error);
    }
    g_assert (IBUS_IS_CONFIG (config));

    /* Since we reuse single D-Bus connection, we need to remove the
       default match rule for the next ibus_config_new() call.  */
    ibus_config_unwatch (config, NULL, NULL);
    g_object_unref (config);
    if (--create_config_count == 0)
        g_main_loop_quit (loop);
}

static void
finish_create_config_async_failed (GObject      *source_object,
                                   GAsyncResult *res,
                                   gpointer      user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    GError *error = NULL;
    IBusConfig *config =
            ibus_config_new_async_finish (res, &error);

    g_assert (config == NULL);
    g_assert (error != NULL);
    g_error_free (error);
    if (--create_config_count <= 0)
        g_main_loop_quit (loop);
}

static void
test_create_config_async (void)
{
    GMainLoop *loop = NULL;
    GCancellable *cancellable = NULL;

    /* create an IC */
    create_config_count = 1;
    loop = g_main_loop_new (NULL, TRUE);
    ibus_config_new_async (ibus_bus_get_connection (bus),
                           NULL,
                           finish_create_config_async_success,
                           loop);
    g_main_loop_run (loop);
    g_main_loop_unref (loop);

    /* call create, and then cancel */
    create_config_count = 1;
    loop = g_main_loop_new (NULL, TRUE);
    cancellable = g_cancellable_new ();

    ibus_config_new_async (ibus_bus_get_connection (bus),
                           cancellable,
                           finish_create_config_async_failed,
                           loop);
    g_cancellable_cancel (cancellable);
    g_object_unref (cancellable);
    g_main_loop_run (loop);
    g_main_loop_unref (loop);
}

static void
test_config_set_get (void)
{
    IBusConfig *config = ibus_config_new (ibus_bus_get_connection (bus),
                                          NULL,
                                          NULL);
    g_assert (config);

    ibus_config_set_value (config, "test", "v1", g_variant_new_int32(1));
    ibus_config_set_value (config, "test", "v2", g_variant_new_string("2"));

    GVariant *var;
    var = ibus_config_get_value (config, "test", "v1");
    g_assert (var);
    g_assert_cmpint (g_variant_get_int32(var), ==, 1);
    g_variant_unref (var);

    var = ibus_config_get_value (config, "test", "v2");
    g_assert (var);
    g_assert_cmpstr (g_variant_get_string(var, NULL), ==, "2");
    g_variant_unref (var);

    var = ibus_config_get_values (config, "test");
    g_assert (var);

    GVariantIter iter;
    gchar *name;
    GVariant *value;
    g_variant_iter_init (&iter, var);
    gint value_bits = 0;
    while (g_variant_iter_next (&iter, "{&sv}", &name, &value)) {
        if (g_strcmp0 (name, "v1") == 0) {
            g_assert_cmpint (g_variant_get_int32(value), ==, 1);
            value_bits |= 1;
        }
        else if (g_strcmp0 (name, "v2") == 0) {
            g_assert_cmpstr (g_variant_get_string(value, NULL), ==, "2");
            value_bits |= (1 << 1);
        }
        else {
            g_warning ("Unknown value name=%s", name);
        }
        ibus_config_unset (config, "test", name);
        g_variant_unref (value);
    }
    g_assert_cmpint (value_bits, ==, 1 | (1 << 1));
    g_variant_unref (var);

    var = ibus_config_get_values (config, "test");
    g_assert (var);
    g_assert_cmpint (g_variant_n_children (var), ==, 0);
    g_variant_unref (var);

    /* Since we reuse single D-Bus connection, we need to remove the
       default match rule for the next ibus_config_new() call.  */
    ibus_config_unwatch (config, NULL, NULL);
    g_object_unref (config);
}

gint
main (gint    argc,
      gchar **argv)
{
    gint result;
    ibus_init ();
    g_test_init (&argc, &argv, NULL);
    bus = ibus_bus_new ();

    g_test_add_func ("/ibus/create-config-async", test_create_config_async);
    g_test_add_func ("/ibus/config-set-get", test_config_set_get);

    result = g_test_run ();
    g_object_unref (bus);

    return result;
}
