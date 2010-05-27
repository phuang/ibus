#include <string.h>
#include "ibus.h"

static GList *engines = NULL;
static GList *current_engine = NULL;

void
global_engine_changed_cb (IBusBus *bus)
{
	IBusEngineDesc *global_engine = ibus_bus_get_global_engine (bus);
	g_assert (global_engine);
	g_debug ("%s (id:%s, icon:%s)", global_engine->longname,
		 global_engine->name, global_engine->icon);
	IBusEngineDesc *engine_desc = IBUS_ENGINE_DESC (current_engine->data);
	g_assert (strcmp (engine_desc->name, global_engine->name) == 0);
	g_object_unref (global_engine);
}

gboolean
change_global_engine_cb (IBusBus *bus)
{
	if (!current_engine)
		current_engine = engines;
	else
		current_engine = g_list_next (current_engine);

	if (!current_engine) {
		ibus_quit();
		return FALSE;
	}

	IBusEngineDesc *engine_desc = IBUS_ENGINE_DESC (current_engine->data);
	ibus_bus_set_global_engine (bus, engine_desc->name);

	return TRUE;
}

int main()
{
	g_type_init ();

	IBusBus *bus;

	bus = ibus_bus_new ();
	engines = ibus_bus_list_active_engines (bus);
	g_assert (engines);

	g_debug ("===== Global engine:");
	g_assert (ibus_bus_get_use_global_engine (bus));

	g_signal_connect (bus, "global-engine-changed",
                          G_CALLBACK (global_engine_changed_cb), bus);

	g_idle_add ((GSourceFunc)change_global_engine_cb, bus);

	ibus_main();

	g_debug ("Test ibusbus.c's global engine api: passed.");
	g_list_free (engines);
	g_object_unref (bus);

	return 0;
}
