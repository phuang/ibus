/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#include <string.h>
#include "ibus.h"

static GList *engines = NULL;
static GList *current_engine = NULL;

void
global_engine_changed_cb (IBusBus *bus)
{
	IBusEngineDesc *global_engine = ibus_bus_get_global_engine (bus);
	const gchar *name = NULL;

	g_assert (global_engine);

	name = ibus_engine_desc_get_name (global_engine);
	g_debug ("%s (id:%s, icon:%s)",
	         ibus_engine_desc_get_longname (global_engine),
	         name,
	         ibus_engine_desc_get_icon (global_engine));
	IBusEngineDesc *engine_desc = IBUS_ENGINE_DESC (current_engine->data);

	g_assert (strcmp (name,
	                  ibus_engine_desc_get_name (engine_desc)) == 0);
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

	ibus_bus_set_global_engine (bus,
	                            ibus_engine_desc_get_name (engine_desc));

	return TRUE;
}

int main()
{
	g_type_init ();

	IBusBus *bus;

	g_type_init ();
	IBUS_TYPE_ENGINE_DESC;

	bus = ibus_bus_new ();
	engines = ibus_bus_list_active_engines (bus);
	g_assert (engines);

	g_debug ("===== Global engine:");
	if (ibus_bus_get_use_global_engine (bus) == FALSE)
        return 0;

	g_signal_connect (bus, "global-engine-changed",
                          G_CALLBACK (global_engine_changed_cb), bus);

	g_idle_add ((GSourceFunc)change_global_engine_cb, bus);

	ibus_main();

	g_debug ("Test ibusbus.c's global engine api: passed.");
	g_list_free (engines);
	g_object_unref (bus);

	return 0;
}
