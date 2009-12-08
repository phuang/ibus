#include <string.h>
#include "ibus.h"

static const char *
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
	g_debug ("%s (id:%s, icon:%s)", engine_desc->longname, engine_desc->name, engine_desc->icon);
	g_object_unref (engine_desc);
    }
}

int main()
{
	g_type_init ();

	IBusBus *bus;
	GList *engines;
	IBusInputContext *context;
	IBusEngineDesc *engine_desc;
	const char *active_engine_name;

	bus = ibus_bus_new ();

	/* Test ibusbus.c */
	g_debug ("===== Active engines:");
	engines = ibus_bus_list_active_engines (bus);
	g_assert (engines);
	active_engine_name = get_last_engine_id (engines);
	print_engines (engines);
	g_list_free (engines);

	g_debug ("===== All engines:");
	engines = ibus_bus_list_engines (bus);
	g_assert (engines);
	print_engines (engines);
	g_list_free (engines);
	g_debug ("Test ibusbus.c: passed.");

	/* Test ibusinputcontext.c */
	context = ibus_bus_create_input_context (bus, "test");
	ibus_input_context_set_capabilities (context, 0);
	ibus_input_context_disable (context);
	g_assert (ibus_input_context_is_enabled (context) == FALSE);
	ibus_input_context_enable (context);
	g_assert (ibus_input_context_is_enabled (context) == TRUE);
	ibus_input_context_set_engine (context, active_engine_name);
	engine_desc = ibus_input_context_get_engine (context);
	g_assert (engine_desc);
	g_assert (!strcmp (active_engine_name, engine_desc->name));
	g_debug ("Test ibusinputcontext.c: passed.");

	g_free (active_engine_name);
	g_object_unref (engine_desc);
	g_object_unref (context);
	g_object_unref (bus);

	return 0;
}
