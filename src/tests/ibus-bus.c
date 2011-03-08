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

#if 0
static gchar *
get_last_engine_id (const GList *engines)
{
    const char *result = NULL;
    for (; engines; engines = g_list_next (engines)) {
        IBusEngineDesc *engine_desc = IBUS_ENGINE_DESC (engines->data);
        g_assert (engine_desc);
        result = ibus_engine_desc_get_name (engine_desc);
    }
    return g_strdup (result);
}

static void
test_input_context (void)
{
    GList *engines;
    gchar *active_engine_name = NULL;
    IBusInputContext *context;
    IBusEngineDesc *engine_desc;
    gchar *current_ic;

    engines = ibus_bus_list_active_engines (bus);
    if (engines == NULL)
        return;
    active_engine_name = get_last_engine_id (engines);
    g_assert (active_engine_name);

    context = ibus_bus_create_input_context (bus, "test");
    ibus_input_context_set_capabilities (context, IBUS_CAP_FOCUS);
    ibus_input_context_disable (context);
    g_assert (ibus_input_context_is_enabled (context) == FALSE);
    ibus_input_context_enable (context);
    g_assert (ibus_input_context_is_enabled (context) == TRUE);
    ibus_input_context_focus_in (context);
    ibus_input_context_set_engine (context, active_engine_name);
    current_ic = ibus_bus_current_input_context (bus);
    g_assert (!strcmp (current_ic, g_dbus_proxy_get_object_path ((GDBusProxy *)context)));
    engine_desc = ibus_input_context_get_engine (context);
    g_assert (engine_desc);
    g_assert (!strcmp (active_engine_name, ibus_engine_desc_get_name(engine_desc)));
    g_free (current_ic);
    g_object_unref (engine_desc);
    g_object_unref (context);

    g_free (active_engine_name);
    g_list_foreach (engines, (GFunc) g_object_unref, NULL);
    g_list_free (engines);
}
#endif

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

static void
finish_create_input_context_async (GObject *source_object,
                                   GAsyncResult *res,
                                   gpointer user_data)
{
    GError *error = NULL;
    IBusInputContext *context = ibus_bus_create_input_context_async_finish (bus,
                                                                            res,
                                                                            &error);
    g_assert (context != NULL);
    g_object_unref (context);
    g_debug ("ibus_bus_create_input_context_finish: OK");
    call_next_async_function ();
}

static void
start_create_input_context_async (void)
{
    ibus_bus_create_input_context_async (bus,
                                         "test-async",
                                         -1, /* timeout */
                                         NULL, /* cancellable */
                                         finish_create_input_context_async,
                                         NULL); /* user_data */
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
        start_create_input_context_async,
        start_current_input_context_async,
        // FIXME test ibus_bus_register_component_async.
        start_list_engines_async,
        start_list_active_engines_async,
        start_get_use_sys_layout_async,
        start_get_use_global_engine_async,
        start_is_global_engine_enabled_async,
        start_set_global_engine_async,
        start_get_global_engine_async,
        start_exit_async,
    };
    static guint index = 0;

    // Use g_timeout_add to make sure test_async_apis finishes even if async_functions is empty.
    if (index >= G_N_ELEMENTS (async_functions))
        g_timeout_add (1, test_async_apis_finish, NULL);
    else
        (*async_functions[index++])();
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

    g_test_add_func ("/ibus/list-engines", test_list_engines);
    g_test_add_func ("/ibus/list-active-engines", test_list_active_engines);
    g_test_add_func ("/ibus/async-apis", test_async_apis);

    // FIXME This test does not pass if global engine is not available. Disabling it for now.
    // g_test_add_func ("/ibus/input_context", test_input_context);

    result = g_test_run ();
    g_object_unref (bus);

    return result;
}
