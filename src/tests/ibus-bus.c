/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */

#include <string.h>
#include "ibus.h"

static IBusBus *bus;

static void
print_engines (const GList *engines)
{
    for (; engines; engines = g_list_next (engines)) {
        IBusEngineDesc *engine_desc = IBUS_ENGINE_DESC (engines->data);
        g_assert (engine_desc);
#if 0
        g_debug ("%s (id:%s, icon:%s)",
                 ibus_engine_desc_get_longname (engine_desc),
                 ibus_engine_desc_get_name (engine_desc),
                 ibus_engine_desc_get_icon (engine_desc));
#endif
    }
}

static void
test_list_active_engines (void)
{
    GList *engines;
    IBUS_TYPE_ENGINE_DESC;

    engines = ibus_bus_list_active_engines (bus);
    print_engines (engines);

    g_list_foreach (engines, (GFunc) g_object_unref, NULL);
    g_list_free (engines);
}

static void
test_list_engines (void)
{
    GList *engines;
    IBUS_TYPE_ENGINE_DESC;

    engines = ibus_bus_list_engines (bus);
    print_engines (engines);

    g_list_foreach (engines, (GFunc) g_object_unref, NULL);
    g_list_free (engines);
}

static void call_next_async_function (void);

static void
finish_request_name_async (GObject *source_object,
                           GAsyncResult *res,
                           gpointer user_data)
{
    GError *error = NULL;
    guint id = ibus_bus_request_name_async_finish (bus,
                                                   res,
                                                   &error);
    g_assert (id != 0);
    g_debug ("ibus_bus_request_name_async_finish: OK");
    call_next_async_function ();
}

static void
start_request_name_async (void)
{
    ibus_bus_request_name_async (bus,
                                 "org.freedesktop.IBus.IBusBusTest",
                                 0,
                                 -1, /* timeout */
                                 NULL, /* cancellable */
                                 finish_request_name_async,
                                 NULL); /* user_data */
}


static void
finish_name_has_owner_async (GObject *source_object,
                             GAsyncResult *res,
                             gpointer user_data)
{
    GError *error = NULL;
    gboolean has_owner = ibus_bus_name_has_owner_async_finish (bus,
                                                               res,
                                                               &error);
    g_assert (has_owner);
    g_debug ("ibus_bus_name_has_owner_async_finish: OK");
    call_next_async_function ();
}

static void
start_name_has_owner_async (void)
{
    ibus_bus_name_has_owner_async (bus,
                                   "org.freedesktop.IBus.IBusBusTest",
                                   -1, /* timeout */
                                   NULL, /* cancellable */
                                   finish_name_has_owner_async,
                                   NULL); /* user_data */
}

static void
finish_get_name_owner_async (GObject *source_object,
                             GAsyncResult *res,
                             gpointer user_data)
{
    GError *error = NULL;
    gchar *owner = ibus_bus_get_name_owner_async_finish (bus,
                                                         res,
                                                         &error);
    g_assert (owner);
    g_free (owner);
    g_debug ("ibus_bus_name_get_name_owner_async_finish: OK");
    call_next_async_function ();
}

static void
start_get_name_owner_async (void)
{
    ibus_bus_get_name_owner_async (bus,
                                   "org.freedesktop.IBus.IBusBusTest",
                                   -1, /* timeout */
                                   NULL, /* cancellable */
                                   finish_get_name_owner_async,
                                   NULL); /* user_data */
}

static void
finish_release_name_async (GObject *source_object,
                           GAsyncResult *res,
                           gpointer user_data)
{
    GError *error = NULL;
    guint id = ibus_bus_release_name_async_finish (bus,
                                                   res,
                                                   &error);
    g_assert (id != 0);
    g_debug ("ibus_bus_release_name_async_finish: OK");
    call_next_async_function ();
}

static void
start_release_name_async (void)
{
    ibus_bus_release_name_async (bus,
                                 "org.freedesktop.IBus.IBusBusTest",
                                 -1, /* timeout */
                                 NULL, /* cancellable */
                                 finish_release_name_async,
                                 NULL); /* user_data */
}

static void
finish_add_match_async (GObject *source_object,
                        GAsyncResult *res,
                        gpointer user_data)
{
    GError *error = NULL;
    gboolean result = ibus_bus_add_match_async_finish (bus,
                                                       res,
                                                       &error);
    g_assert (result);
    g_debug ("ibus_bus_add_match_finish: OK");
    call_next_async_function ();
}

static void
start_add_match_async (void)
{
    ibus_bus_add_match_async (bus,
                              "type='signal'",
                              -1, /* timeout */
                              NULL, /* cancellable */
                              finish_add_match_async,
                              NULL); /* user_data */
}

static void
finish_remove_match_async (GObject *source_object,
                           GAsyncResult *res,
                           gpointer user_data)
{
    GError *error = NULL;
    gboolean result = ibus_bus_remove_match_async_finish (bus,
                                                          res,
                                                          &error);
    g_assert (result);
    g_debug ("ibus_bus_remove_match_finish: OK");
    call_next_async_function ();
}

static void
start_remove_match_async (void)
{
    ibus_bus_remove_match_async (bus,
                                 "type='signal'",
                                 -1, /* timeout */
                                 NULL, /* cancellable */
                                 finish_remove_match_async,
                                 NULL); /* user_data */
}

static int create_input_context_count = 0;
static void
finish_create_input_context_async_sucess (GObject      *source_object,
                                          GAsyncResult *res,
                                          gpointer      user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    GError *error = NULL;
    IBusInputContext *context =
          ibus_bus_create_input_context_async_finish (bus, res, &error);

    g_assert (IBUS_IS_INPUT_CONTEXT (context));
    g_object_unref (context);
    if (--create_input_context_count == 0)
        g_main_loop_quit (loop);
}

static void
finish_create_input_context_async_failed (GObject      *source_object,
                                          GAsyncResult *res,
                                          gpointer      user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    GError *error = NULL;
    IBusInputContext *context =
            ibus_bus_create_input_context_async_finish (bus, res, &error);

    g_assert (context == NULL);
    g_assert (error != NULL);
    g_error_free (error);
    if (--create_input_context_count <= 0)
        g_main_loop_quit (loop);
}

static void
test_create_input_context_async (void)
{
    GMainLoop *loop = NULL;
    GCancellable *cancellable = NULL;

    /* create an IC */
    create_input_context_count = 1;
    loop = g_main_loop_new (NULL, TRUE);
    ibus_bus_create_input_context_async (bus,
            "test-async",
            -1, /* timeout */
            NULL, /* cancellable */
            finish_create_input_context_async_sucess,
            loop); /* user_data */
    g_main_loop_run (loop);
    g_main_loop_unref (loop);

    /* call create, and then cancel */
    create_input_context_count = 1;
    loop = g_main_loop_new (NULL, TRUE);
    cancellable = g_cancellable_new ();
    ibus_bus_create_input_context_async (bus,
            "test-async",
            -1, /* timeout */
            cancellable, /* cancellable */
            finish_create_input_context_async_failed,
            loop); /* user_data */
    g_cancellable_cancel (cancellable);
    g_object_unref (cancellable);
    g_main_loop_run (loop);
    g_main_loop_unref (loop);

    /* ceate four IC, and cancel two */
    create_input_context_count = 4;
    loop = g_main_loop_new (NULL, TRUE);
    cancellable = g_cancellable_new ();
    ibus_bus_create_input_context_async (bus,
            "test-async",
            -1, /* timeout */
            cancellable, /* cancellable */
            finish_create_input_context_async_failed,
            loop); /* user_data */
    ibus_bus_create_input_context_async (bus,
            "test-async",
            -1, /* timeout */
            NULL, /* cancellable */
            finish_create_input_context_async_sucess,
            loop); /* user_data */
    ibus_bus_create_input_context_async (bus,
            "test-async",
            -1, /* timeout */
            NULL, /* cancellable */
            finish_create_input_context_async_sucess,
            loop); /* user_data */
    ibus_bus_create_input_context_async (bus,
            "test-async",
            -1, /* timeout */
            cancellable, /* cancellable */
            finish_create_input_context_async_failed,
            loop); /* user_data */
    g_cancellable_cancel (cancellable);
    g_object_unref (cancellable);

    g_main_loop_run (loop);
    g_main_loop_unref (loop);
}

static void
finish_current_input_context_async (GObject *source_object,
                                    GAsyncResult *res,
                                    gpointer user_data)
{
    GError *error = NULL;
    g_free (ibus_bus_current_input_context_async_finish (bus,
                                                         res,
                                                         &error));
    // no null check.
    g_debug ("ibus_bus_current_input_context_finish: OK");
    call_next_async_function ();
}

static void
start_current_input_context_async (void)
{
    ibus_bus_current_input_context_async (bus,
                                          -1, /* timeout */
                                          NULL, /* cancellable */
                                          finish_current_input_context_async,
                                          NULL); /* user_data */
}

static void
finish_list_engines_async (GObject *source_object,
                           GAsyncResult *res,
                           gpointer user_data)
{
    GError *error = NULL;
    GList *engines = ibus_bus_list_engines_async_finish (bus,
                                                         res,
                                                         &error);
    // no null check.
    g_list_foreach (engines, (GFunc) g_object_unref, NULL);
    g_list_free (engines);
    g_debug ("ibus_bus_list_engines_finish: OK");
    call_next_async_function ();
}

static void
start_list_engines_async (void)
{
    ibus_bus_list_engines_async (bus,
                                 -1, /* timeout */
                                 NULL, /* cancellable */
                                 finish_list_engines_async,
                                 NULL); /* user_data */
}

static void
finish_list_active_engines_async (GObject *source_object,
                                  GAsyncResult *res,
                                  gpointer user_data)
{
    GError *error = NULL;
    GList *engines = ibus_bus_list_active_engines_async_finish (bus,
                                                                res,
                                                                &error);
    // no null check.
    g_list_foreach (engines, (GFunc) g_object_unref, NULL);
    g_list_free (engines);
    g_debug ("ibus_bus_list_active_engines_finish: OK");
    call_next_async_function ();
}

static void
start_list_active_engines_async (void)
{
    ibus_bus_list_active_engines_async (bus,
                                        -1, /* timeout */
                                        NULL, /* cancellable */
                                        finish_list_active_engines_async,
                                        NULL); /* user_data */
}

static void
finish_get_use_sys_layout_async (GObject *source_object,
                                 GAsyncResult *res,
                                 gpointer user_data)
{
    GError *error = NULL;
    ibus_bus_get_use_sys_layout_async_finish (bus,
                                              res,
                                              &error);
    g_debug ("ibus_bus_get_use_sys_layout_finish: OK");
    call_next_async_function ();
}

static void
start_get_use_sys_layout_async (void)
{
    ibus_bus_get_use_sys_layout_async (bus,
                                       -1, /* timeout */
                                       NULL, /* cancellable */
                                       finish_get_use_sys_layout_async,
                                       NULL); /* user_data */
}

static void
finish_get_use_global_engine_async (GObject *source_object,
                                    GAsyncResult *res,
                                    gpointer user_data)
{
    GError *error = NULL;
    ibus_bus_get_use_global_engine_async_finish (bus,
                                                 res,
                                                 &error);
    g_debug ("ibus_bus_get_use_global_engine_finish: OK");
    call_next_async_function ();
}

static void
start_get_use_global_engine_async (void)
{
    ibus_bus_get_use_global_engine_async (bus,
                                          -1, /* timeout */
                                          NULL, /* cancellable */
                                          finish_get_use_global_engine_async,
                                          NULL); /* user_data */
}

static void
finish_is_global_engine_enabled_async (GObject *source_object,
                                       GAsyncResult *res,
                                       gpointer user_data)
{
    GError *error = NULL;
    ibus_bus_is_global_engine_enabled_async_finish (bus,
                                                    res,
                                                    &error);
    g_debug ("ibus_bus_is_global_engine_enabled_finish: OK");
    call_next_async_function ();
}

static void
start_is_global_engine_enabled_async (void)
{
    ibus_bus_is_global_engine_enabled_async (bus,
                                             -1, /* timeout */
                                             NULL, /* cancellable */
                                             finish_is_global_engine_enabled_async,
                                             NULL); /* user_data */
}

static void
finish_get_global_engine_async (GObject *source_object,
                                GAsyncResult *res,
                                gpointer user_data)
{
    GError *error = NULL;
    IBusEngineDesc *desc = ibus_bus_get_global_engine_async_finish (bus,
                                                                    res,
                                                                    &error);
    if (desc)
        g_object_unref (desc);
    g_debug ("ibus_bus_get_global_engine_finish: OK");
    call_next_async_function ();
}

static void
start_get_global_engine_async (void)
{
    ibus_bus_get_global_engine_async (bus,
                                      -1, /* timeout */
                                      NULL, /* cancellable */
                                      finish_get_global_engine_async,
                                      NULL); /* user_data */
}

static void
finish_set_global_engine_async (GObject *source_object,
                                GAsyncResult *res,
                                gpointer user_data)
{
    GError *error = NULL;
    ibus_bus_set_global_engine_async_finish (bus,
                                             res,
                                             &error);
    g_debug ("ibus_bus_set_global_engine_finish: OK");
    call_next_async_function ();
}

static void
start_set_global_engine_async (void)
{
    ibus_bus_set_global_engine_async (bus,
                                      "anthy",
                                      -1, /* timeout */
                                      NULL, /* cancellable */
                                      finish_set_global_engine_async,
                                      NULL); /* user_data */
}

static void
finish_preload_engines_async (GObject      *source_object,
                              GAsyncResult *res,
                              gpointer      user_data)
{
    GError *error = NULL;
    ibus_bus_preload_engines_async_finish (bus, res, &error);
    g_debug ("ibus_bus_preload_engines_async_finish: OK");
    call_next_async_function ();
}

static void
start_preload_engines_async (void)
{
    const gchar *preload_engines[] = { "xkb:us::eng", NULL };

    ibus_bus_preload_engines_async (
            bus,
            preload_engines,
            -1, /* timeout */
            NULL, /* cancellable */
            finish_preload_engines_async,
            NULL); /* user_data */
}

static void
finish_exit_async (GObject *source_object,
                   GAsyncResult *res,
                   gpointer user_data)
{
    GError *error = NULL;
    gboolean result = ibus_bus_exit_async_finish (bus,
                                                  res,
                                                  &error);
    g_assert (result);
    g_debug ("ibus_bus_exit_finish: OK");
    g_usleep (G_USEC_PER_SEC);
    call_next_async_function ();
}

static void
start_exit_async (void)
{
    ibus_bus_exit_async (bus,
                         TRUE, /* restart */
                         -1, /* timeout */
                         NULL, /* cancellable */
                         finish_exit_async,
                         NULL); /* user_data */
}

static gboolean
test_async_apis_finish (gpointer user_data)
{
    ibus_quit ();
    return FALSE;
}

static void
test_get_engines_by_names (void)
{
    IBusEngineDesc **engines = NULL;
    const gchar *names[] = {
        "xkb:us::eng",
        "xkb:ca:eng:eng",
        "xkb:fr::fra",
        "xkb:jp::jpn",
        "invalid_engine_name",
        NULL,
    };

    engines = ibus_bus_get_engines_by_names (bus, names);

    g_assert(engines != NULL);
    IBusEngineDesc **p;

    gint i = 0;
    for (p = engines; *p != NULL; p++) {
        g_assert (IBUS_IS_ENGINE_DESC (*p));
        g_assert_cmpstr (names[i], ==, ibus_engine_desc_get_name (*p));
        i++;
        g_object_unref (*p);
        // The ref should be zero, *p is released.
        g_assert (!IBUS_IS_ENGINE_DESC (*p));
    }

    // The last engine does not exist.
    g_assert_cmpint (i, ==, G_N_ELEMENTS(names) - 2);

    g_free (engines);

    engines = NULL;
}

static void
test_async_apis (void)
{
    g_debug ("start");
    call_next_async_function ();
    ibus_main ();
}

static void
call_next_async_function (void)
{
    static void (*async_functions[])(void) = {
        start_request_name_async,
        start_name_has_owner_async,
        start_get_name_owner_async,
        start_release_name_async,
        start_add_match_async,
        start_remove_match_async,
        start_current_input_context_async,
        // FIXME test ibus_bus_register_component_async.
        start_list_engines_async,
        start_list_active_engines_async,
        start_get_use_sys_layout_async,
        start_get_use_global_engine_async,
        start_is_global_engine_enabled_async,
        start_set_global_engine_async,
        start_get_global_engine_async,
        start_preload_engines_async,
        start_exit_async,
    };
    static guint index = 0;

    // Use g_timeout_add to make sure test_async_apis finishes even if async_functions is empty.
    if (index >= G_N_ELEMENTS (async_functions))
        g_timeout_add (1, test_async_apis_finish, NULL);
    else
        (*async_functions[index++])();
}

static void
_bus_connected_cb (IBusBus *bus,
                   gpointer user_data)
{
    g_assert (ibus_bus_is_connected (bus));
    ibus_quit ();
}

static void
test_bus_new_async (void)
{
    g_object_unref (bus);
    bus = ibus_bus_new_async ();
    g_signal_connect (bus, "connected", G_CALLBACK (_bus_connected_cb), NULL);
    ibus_main ();
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
    g_object_unref (bus);
    bus = ibus_bus_new (); // crosbug.com/17293

    g_test_add_func ("/ibus/list-engines", test_list_engines);
    g_test_add_func ("/ibus/list-active-engines", test_list_active_engines);
    g_test_add_func ("/ibus/create-input-context-async",
                     test_create_input_context_async);
    g_test_add_func ("/ibus/get-engines-by-names", test_get_engines_by_names);
    g_test_add_func ("/ibus/async-apis", test_async_apis);
    g_test_add_func ("/ibus/bus-new-async", test_bus_new_async);
    g_test_add_func ("/ibus/bus-new-async/list-engines", test_list_engines);
    g_test_add_func ("/ibus/bus-new-async/list-active-engines", test_list_active_engines);
    g_test_add_func ("/ibus/bus-new-async/create-input-context-async",
                     test_create_input_context_async);
    g_test_add_func ("/ibus/bus-new-async/get-engines-by-names", test_get_engines_by_names);
    g_test_add_func ("/ibus/bus-new-async/async-apis", test_async_apis);

    result = g_test_run ();
    g_object_unref (bus);

    return result;
}
