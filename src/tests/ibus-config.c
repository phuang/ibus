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

typedef struct {
    gchar *section;
    gchar *name;
} WatchKey;

typedef struct {
    WatchKey *watched;          /* watched keys (null-terminated) */
    WatchKey *changed;          /* changed keys (null-terminated) */
    WatchKey *notified;         /* notified keys (same length as
                                   changed, not null-terminated) */
} WatchTestData;

static WatchKey default_watched[] = {
    { NULL }
};
static WatchKey default_changed[] = {
    { "test/s1", "n1" },
    { "test/s1", "n2" },
    { "test/s2", "n1" },
    { "test/s2", "n2" },
    { NULL }
};
static WatchKey default_notified[] = {
    { "test/s1", "n1" },
    { "test/s1", "n2" },
    { "test/s2", "n1" },
    { "test/s2", "n2" }
};
static WatchTestData default_data = {
    default_watched,
    default_changed,
    default_notified
};

static WatchKey section_watched[] = {
    { "test/s1", NULL },
    { NULL }
};
static WatchKey section_notified[] = {
    { "test/s1", "n1" },
    { "test/s1", "n2" },
    { NULL, NULL },
    { NULL, NULL },
};
static WatchTestData section_data = {
    section_watched,
    default_changed,
    section_notified
};

static WatchKey section_multiple_watched[] = {
    { "test/s1", NULL },
    { "test/s2", NULL },
    { NULL }
};
static WatchKey section_multiple_notified[] = {
    { "test/s1", "n1" },
    { "test/s1", "n2" },
    { "test/s2", "n1" },
    { "test/s2", "n2" },
};
static WatchTestData section_multiple_data = {
    section_multiple_watched,
    default_changed,
    section_multiple_notified
};

static WatchKey section_name_watched[] = {
    { "test/s1", "n1" },
    { NULL }
};
static WatchKey section_name_notified[] = {
    { "test/s1", "n1" },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
};
static WatchTestData section_name_data = {
    section_name_watched,
    default_changed,
    section_name_notified
};

static WatchKey section_name_multiple_watched[] = {
    { "test/s1", "n1" },
    { "test/s2", "n2" },
    { NULL }
};
static WatchKey section_name_multiple_notified[] = {
    { "test/s1", "n1" },
    { NULL, NULL },
    { NULL, NULL },
    { "test/s2", "n2" },
};
static WatchTestData section_name_multiple_data = {
    section_name_multiple_watched,
    default_changed,
    section_name_multiple_notified
};

typedef struct {
    IBusConfig *config;
    WatchData data;
} WatchFixture;

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
change_and_test (IBusConfig  *config,
                 const gchar *section,
                 const gchar *name,
                 const gchar *expected_section,
                 const gchar *expected_name,
                 WatchData   *data)
{
    gboolean retval;
    guint timeout_id;
    GVariant *var;

    data->section = NULL;
    data->name = NULL;

    /* Unset won't notify value-changed signal. */
    var = ibus_config_get_values (config, section);
    if (var != NULL) {
        GVariant *value = g_variant_lookup_value (var, name,
                                                  G_VARIANT_TYPE_VARIANT);
        if (value != NULL) {
            ibus_config_unset (config, section, name);
            g_variant_unref (value);
        }
        g_variant_unref (var);
    }

    timeout_id = g_timeout_add (1, timeout_cb, data);
    g_main_loop_run (data->loop);
    g_source_remove (timeout_id);

    retval = ibus_config_set_value (config, section, name,
                                    g_variant_new_int32 (1));
    g_assert (retval);

    timeout_id = g_timeout_add (1, timeout_cb, data);
    g_main_loop_run (data->loop);
    g_source_remove (timeout_id);

    g_assert_cmpstr (data->section, ==, expected_section);
    g_assert_cmpstr (data->name, ==, expected_name);

    g_free (data->section);
    g_free (data->name);
}

static void
watch_fixture_setup (WatchFixture *fixture, gconstpointer user_data)
{
    fixture->config = ibus_config_new (ibus_bus_get_connection (bus),
                                       NULL,
                                       NULL);
    g_assert (fixture->config);

    fixture->data.loop = g_main_loop_new (NULL, FALSE);
    g_signal_connect (fixture->config, "value-changed",
                      G_CALLBACK (value_changed_cb), &fixture->data);
}

static void
watch_fixture_teardown (WatchFixture *fixture, gconstpointer user_data)
{
    g_main_loop_unref (fixture->data.loop);

    ibus_proxy_destroy (IBUS_PROXY (fixture->config));
    g_object_unref (fixture->config);
}

static void
test_config_watch (WatchFixture *fixture, gconstpointer user_data)
{
    const WatchTestData *data = user_data;
    gint i;

    for (i = 0; data->watched[i].section != NULL; i++) {
        ibus_config_watch (fixture->config,
                           data->watched[i].section,
                           data->watched[i].name);
    }
    for (i = 0; data->changed[i].section != NULL; i++) {
        change_and_test (fixture->config,
                         data->changed[i].section,
                         data->changed[i].name,
                         data->notified[i].section,
                         data->notified[i].name,
                         &fixture->data);
    }
    for (i = 0; data->watched[i].section != NULL; i++) {
        ibus_config_unwatch (fixture->config,
                             data->watched[i].section,
                             data->watched[i].name);
    }
    if (i > 0) {
        /* Check if the above unwatch takes effect. */
        for (i = 0; data->changed[i].section != NULL; i++) {
            change_and_test (fixture->config,
                             data->changed[i].section,
                             data->changed[i].name,
                             NULL,
                             NULL,
                             &fixture->data);
        }
    } else {
        /* If ibus_config_unwatch has not been called, need to manually
           unwatch the default rule here, otherwise the recipient of
           the default match rule will be ref'd twice on the next
           ibus_config_new(), since we reuse single D-Bus
           connection. */
        ibus_config_unwatch (fixture->config, NULL, NULL);
    }
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

    g_test_add ("/ibus/config-watch/default",
                WatchFixture,
                &default_data,
                watch_fixture_setup,
                test_config_watch,
                watch_fixture_teardown);

    g_test_add ("/ibus/config-watch/section",
                WatchFixture,
                &section_data,
                watch_fixture_setup,
                test_config_watch,
                watch_fixture_teardown);

    g_test_add ("/ibus/config-watch/section-multiple",
                WatchFixture,
                &section_multiple_data,
                watch_fixture_setup,
                test_config_watch,
                watch_fixture_teardown);

    g_test_add ("/ibus/config-watch/section-name",
                WatchFixture,
                &section_name_data,
                watch_fixture_setup,
                test_config_watch,
                watch_fixture_teardown);

    g_test_add ("/ibus/config-watch/section-name-multiple",
                WatchFixture,
                &section_name_multiple_data,
                watch_fixture_setup,
                test_config_watch,
                watch_fixture_teardown);

    g_test_add_func ("/ibus/create-config-async", test_create_config_async);
    g_test_add_func ("/ibus/config-set-get", test_config_set_get);

    result = g_test_run ();
    g_object_unref (bus);

    return result;
}
