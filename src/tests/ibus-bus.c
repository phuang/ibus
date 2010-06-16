#include <string.h>
#include "ibus.h"

#if 0
static gchar *
get_last_engine_id (const GList *engines)
{
    g_assert (engines);
    const char *result = NULL;
    for (; engines; engines = g_list_next (engines)) {
    IBusEngineDesc *engine_desc = IBUS_ENGINE_DESC (engines->data);
    g_assert (engine_desc);
    result = engine_desc->name;
    }
    g_assert (result);
    return g_strdup (result);
}

static void
print_engines (const GList *engines)
{
    g_assert (engines);
    for (; engines; engines = g_list_next (engines)) {
    IBusEngineDesc *engine_desc = IBUS_ENGINE_DESC (engines->data);
    g_assert (engine_desc);
    // g_debug ("%s (id:%s, icon:%s)", engine_desc->longname, engine_desc->name, engine_desc->icon);
    g_object_unref (engine_desc);
    }
}

static void
test_list_active_engines (void)
{
    GList *engines;
    IBusBus *bus;
    IBUS_TYPE_ENGINE_DESC;

    bus = ibus_bus_new ();

    engines = ibus_bus_list_active_engines (bus);

    g_assert (engines);
    g_list_foreach (engines, (GFunc) g_object_unref, NULL);
    g_list_free (engines);
    g_object_unref (bus);
}

static void
test_list_engines (void)
{
    GList *engines;
    IBusBus *bus;
    IBUS_TYPE_ENGINE_DESC;

    bus = ibus_bus_new ();

    engines = ibus_bus_list_engines (bus);

    g_assert (engines);
    g_list_foreach (engines, (GFunc) g_object_unref, NULL);
    g_list_free (engines);
    g_object_unref (bus);
}
#endif

gint
main (gint    argc,
      gchar **argv)
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);
#if 0
    g_test_add_func ("/ibus/list-engines", test_list_engines);
    g_test_add_func ("/ibus/list-active-engines", test_list_active_engines);
#endif
    return g_test_run ();
#if 0
    IBusBus *bus;
    GList *engines;
    gchar *active_engine_name;

    bus = ibus_bus_new ();

    if (ibus_bus_get_use_global_engine (bus)) {
        g_debug ("use_global_engine is true.");
        if (ibus_bus_is_global_engine_enabled (bus)) {
        g_debug ("Global engine is enabled.");
        IBusEngineDesc *global_engine = ibus_bus_get_global_engine (bus);
        g_assert (global_engine);
        g_debug ("%s (id:%s, icon:%s)", global_engine->longname,
             global_engine->name, global_engine->icon);
        g_object_unref (global_engine);
        }
    }

    g_debug ("===== Use system layout:%s", ibus_bus_get_use_sys_layout (bus) ? "true" : "false");

    g_debug ("Test ibusbus.c: passed.");

    /* Test ibusinputcontext.c */
#if 1
    {
        IBusInputContext *context;
        IBusEngineDesc *engine_desc;
        gchar *current_ic;
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
        g_assert (!strcmp (active_engine_name, engine_desc->name));
        g_debug ("Test ibusinputcontext.c: passed.");

        g_free (active_engine_name);
        g_free (current_ic);
        g_object_unref (engine_desc);
        g_object_unref (context);
    }
#endif
    g_object_unref (bus);
#endif
    return 0;
}
