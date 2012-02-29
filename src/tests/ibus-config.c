/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */

#include <string.h>
#include "ibus.h"

static IBusBus *bus = NULL;
static int create_config_count = 0;

static void
finish_create_config_async_sucess (GObject      *source_object,
                                   GAsyncResult *res,
                                   gpointer      user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    GError *error = NULL;
    IBusConfig *config =
          ibus_config_new_async_finish (res, &error);

    g_assert (IBUS_IS_CONFIG (config));
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
                           finish_create_config_async_sucess,
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
            g_warning ("unknow value name=%s", name);
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

    g_object_unref (config);
}

typedef struct {
    GMainLoop *loop;
    gchar *section;
    gchar *name;
} WatchData;

static void
value_changed_cb (IBusConfig  *config,
                  const gchar *section,
                  const gchar *name,
                  GVariant    *value,
                  gpointer     user_data)
{
    WatchData *data = (WatchData *) user_data;

    data->section = g_strdup (section);
    data->name = g_strdup (name);

    g_main_loop_quit (data->loop);
}

static gboolean
timeout_cb (gpointer user_data)
{
    WatchData *data = (WatchData *) user_data;
    g_main_loop_quit (data->loop);
    return FALSE;
}

static void
change_value (IBusConfig  *config,
              const gchar *section,
              const gchar *name,
              WatchData   *data)
{
    gboolean retval;
    guint timeout_id;

    /* Unset won't notify value-changed signal. */
    retval = ibus_config_unset (config, section, name);
    g_assert (retval);

    timeout_id = g_timeout_add (1, timeout_cb, data);
    g_main_loop_run (data->loop);
    g_source_remove (timeout_id);

    retval = ibus_config_set_value (config, section, name,
                                    g_variant_new_int32 (1));
    g_assert (retval);

    timeout_id = g_timeout_add (1, timeout_cb, data);
    g_main_loop_run (data->loop);
    g_source_remove (timeout_id);
}

static void
test_config_watch (void)
{
    IBusConfig *config;
    WatchData data;

    config = ibus_config_new (ibus_bus_get_connection (bus),
                              NULL,
                              NULL);
    g_assert (config);

    data.loop = g_main_loop_new (NULL, FALSE);
    g_signal_connect (config, "value-changed",
                      G_CALLBACK (value_changed_cb), &data);

    /* By default, no watch is set and every change will be notified. */
    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s1", "n1", &data);

    g_assert_cmpstr (data.section, ==, "test/s1");
    g_assert_cmpstr (data.name, ==, "n1");

    g_free (data.section);
    g_free (data.name);

    /* Watch only section. */
    ibus_config_watch (config, "test/s1", NULL);

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s1", "n1", &data);

    g_assert_cmpstr (data.section, ==, "test/s1");
    g_assert_cmpstr (data.name, ==, "n1");

    g_free (data.section);
    g_free (data.name);

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s2", "n1", &data);

    g_assert_cmpstr (data.section, ==, NULL);
    g_assert_cmpstr (data.name, ==, NULL);

    g_free (data.section);
    g_free (data.name);

    ibus_config_unwatch (config, "test/s1", NULL);

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s1", "n1", &data);

    g_assert_cmpstr (data.section, ==, NULL);
    g_assert_cmpstr (data.name, ==, NULL);

    g_free (data.section);
    g_free (data.name);

    ibus_config_watch (config, NULL, NULL);

    /* Watch only section; multiple watches. */
    ibus_config_watch (config, "test/s1", NULL);
    ibus_config_watch (config, "test/s2", NULL);

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s1", "n1", &data);

    g_assert_cmpstr (data.section, ==, "test/s1");
    g_assert_cmpstr (data.name, ==, "n1");

    g_free (data.section);
    g_free (data.name);

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s2", "n1", &data);

    g_assert_cmpstr (data.section, ==, "test/s2");
    g_assert_cmpstr (data.name, ==, "n1");

    g_free (data.section);
    g_free (data.name);

    ibus_config_unwatch (config, "test/s1", NULL);
    ibus_config_unwatch (config, "test/s2", NULL);
    ibus_config_watch (config, NULL, NULL);

    /* Watch both section and name. */
    ibus_config_watch (config, "test/s1", "n1");

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s1", "n1", &data);

    g_assert_cmpstr (data.section, ==, "test/s1");
    g_assert_cmpstr (data.name, ==, "n1");

    g_free (data.section);
    g_free (data.name);

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s1", "n2", &data);

    g_assert_cmpstr (data.section, ==, NULL);
    g_assert_cmpstr (data.name, ==, NULL);

    g_free (data.section);
    g_free (data.name);

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s2", "n1", &data);

    g_assert_cmpstr (data.section, ==, NULL);
    g_assert_cmpstr (data.name, ==, NULL);

    g_free (data.section);
    g_free (data.name);

    ibus_config_unwatch (config, "test/s1", "n1");
    ibus_config_watch (config, NULL, NULL);

    /* Watch both section and name; multiple watches. */
    ibus_config_watch (config, "test/s1", "n1");
    ibus_config_watch (config, "test/s2", "n2");

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s1", "n1", &data);

    g_assert_cmpstr (data.section, ==, "test/s1");
    g_assert_cmpstr (data.name, ==, "n1");

    g_free (data.section);
    g_free (data.name);

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s1", "n2", &data);

    g_assert_cmpstr (data.section, ==, NULL);
    g_assert_cmpstr (data.name, ==, NULL);

    g_free (data.section);
    g_free (data.name);

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s2", "n1", &data);

    g_assert_cmpstr (data.section, ==, NULL);
    g_assert_cmpstr (data.name, ==, NULL);

    g_free (data.section);
    g_free (data.name);

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s2", "n2", &data);

    g_assert_cmpstr (data.section, ==, "test/s2");
    g_assert_cmpstr (data.name, ==, "n2");

    g_free (data.section);
    g_free (data.name);

    ibus_config_unwatch (config, "test/s1", "n1");

    data.section = NULL;
    data.name = NULL;

    change_value (config, "test/s1", "n1", &data);

    g_assert_cmpstr (data.section, ==, NULL);
    g_assert_cmpstr (data.name, ==, NULL);

    g_free (data.section);
    g_free (data.name);

    ibus_config_unwatch (config, "test/s2", "n2");
    ibus_config_watch (config, NULL, NULL);

    g_main_loop_unref (data.loop);
    g_object_unref (config);
}

gint
main (gint    argc,
      gchar **argv)
{
    gint result;
    g_type_init ();
    g_test_init (&argc, &argv, NULL);
    ibus_init ();
    bus = ibus_bus_new ();

    g_test_add_func ("/ibus/config-watch", test_config_watch);
    g_test_add_func ("/ibus/create-config-async", test_create_config_async);
    g_test_add_func ("/ibus/config-set-get", test_config_set_get);

    result = g_test_run ();
    g_object_unref (bus);

    return result;
}
