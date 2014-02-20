/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */

#include <string.h>
#include "ibus.h"

static IBusBus *bus;

#define BEFORE_ENGINE "xkb:us::eng"
#define AFTER_ENGINE "xkb:jp::jpn"

static const gchar *engine_names[] = {
    BEFORE_ENGINE,
    AFTER_ENGINE
};

static void
change_global_engine (void)
{
    gint i;

    for (i = 0; i < G_N_ELEMENTS (engine_names); i++) {
        ibus_bus_set_global_engine (bus, engine_names[i]);
        IBusEngineDesc *engine_desc = ibus_bus_get_global_engine (bus);
        g_assert_cmpstr (ibus_engine_desc_get_name (engine_desc),
                         ==,
                         engine_names[i]);
    }
}

static void
change_context_engine (IBusInputContext *context)
{
    gint i;

    for (i = 0; i < G_N_ELEMENTS (engine_names); i++) {
        ibus_input_context_set_engine (context, engine_names[i]);
        IBusEngineDesc *engine_desc = ibus_input_context_get_engine (context);
        g_assert_cmpstr (ibus_engine_desc_get_name (engine_desc),
                         ==,
                         engine_names[i]);
    }
}

typedef struct {
    gint count;
} GlobalEngineChangedData;

static void
global_engine_changed_cb (IBusBus *bus, gchar *name, gpointer user_data)
{
    GlobalEngineChangedData *data = (GlobalEngineChangedData *) user_data;
    data->count++;
}

static gboolean
timeout_cb (gpointer user_data)
{
    ibus_quit ();
    return FALSE;
}

static gboolean
change_global_engine_cb (gpointer user_data)
{
    change_global_engine ();
    return FALSE;
}

static void
test_global_engine (void)
{
    GlobalEngineChangedData data;
    guint handler_id, timeout_id, idle_id;

    if (!ibus_bus_get_use_global_engine (bus))
        return;

    data.count = 0;

    handler_id = g_signal_connect (bus,
                                   "global-engine-changed",
                                   G_CALLBACK (global_engine_changed_cb),
                                   &data);
    timeout_id = g_timeout_add_seconds (1, timeout_cb, &data);
    idle_id = g_idle_add ((GSourceFunc) change_global_engine_cb, NULL);

    ibus_main ();

    g_assert_cmpint (data.count, ==, G_N_ELEMENTS (engine_names));

    g_source_remove (idle_id);
    g_source_remove (timeout_id);
    g_signal_handler_disconnect (bus, handler_id);
}

static void
test_context_engine (void)
{
    IBusEngineDesc *engine_desc;
    IBusInputContext *context;

    if (ibus_bus_get_use_global_engine (bus))
        return;

    context = ibus_bus_create_input_context (bus, "test");
    ibus_input_context_set_capabilities (context, IBUS_CAP_FOCUS);

    /* ibus_bus_set_global_engine() changes focused context engine. */
    ibus_input_context_focus_in (context);

    change_context_engine (context);
    engine_desc = ibus_input_context_get_engine (context);
    g_assert_cmpstr (ibus_engine_desc_get_name (engine_desc), ==, AFTER_ENGINE);

    g_object_unref (context);
}

static void
test_context_engine_set_by_global (void)
{
    IBusEngineDesc *engine_desc;
    IBusInputContext *context;

    if (!ibus_bus_get_use_global_engine (bus))
        return;

    context = ibus_bus_create_input_context (bus, "test");
    ibus_input_context_set_capabilities (context, IBUS_CAP_FOCUS);

    /* ibus_bus_set_global_engine() changes focused context engine. */
    ibus_input_context_focus_in (context);

    change_global_engine ();

    /* ibus_input_context_set_engine() does not take effect when
       global engine is used. */
    ibus_input_context_set_engine (context, BEFORE_ENGINE);

    engine_desc = ibus_input_context_get_engine (context);
    g_assert_cmpstr (ibus_engine_desc_get_name (engine_desc), ==, AFTER_ENGINE);

    g_object_unref (context);
}

static void
test_context_engine_set_by_focus (void)
{
    IBusEngineDesc *engine_desc;
    IBusInputContext *context, *another_context;

    if (!ibus_bus_get_use_global_engine (bus))
        return;

    context = ibus_bus_create_input_context (bus, "test");
    ibus_input_context_set_capabilities (context, IBUS_CAP_FOCUS);

    another_context = ibus_bus_create_input_context (bus, "another");
    ibus_input_context_set_capabilities (another_context, IBUS_CAP_FOCUS);

    ibus_input_context_focus_in (context);

    change_global_engine ();

    /* When focus is lost, context engine is set to "dummy". */
    ibus_input_context_focus_in (another_context);

    engine_desc = ibus_input_context_get_engine (context);
    g_assert_cmpstr (ibus_engine_desc_get_name (engine_desc), ==, "dummy");

    engine_desc = ibus_input_context_get_engine (another_context);
    g_assert_cmpstr (ibus_engine_desc_get_name (engine_desc), ==, AFTER_ENGINE);

    g_object_unref (context);
    g_object_unref (another_context);
}

gint
main (gint    argc,
      gchar **argv)
{
    gint result;
    ibus_init ();
    g_test_init (&argc, &argv, NULL);
    bus = ibus_bus_new ();
    g_object_unref (bus);
    bus = ibus_bus_new (); // crosbug.com/17293

    ibus_bus_set_watch_ibus_signal (bus, TRUE);

    g_test_add_func ("/ibus/engine-switch/global-engine",
                     test_global_engine);
    g_test_add_func ("/ibus/engine-switch/context-engine",
                     test_context_engine);
    g_test_add_func ("/ibus/engine-switch/context-engine-set-by-global",
                     test_context_engine_set_by_global);
    g_test_add_func ("/ibus/engine-switch/context-engine-set-by-focus",
                     test_context_engine_set_by_focus);

    result = g_test_run ();
    g_object_unref (bus);

    return result;
}
