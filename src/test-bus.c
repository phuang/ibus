#include "ibus.h"

int main()
{
	g_type_init ();
	
	GMainLoop *mainloop;
	IBusBus *bus;
	IBusInputContext *context;
	
	mainloop = g_main_loop_new (NULL, FALSE);
	bus = ibus_bus_new ();
	context = ibus_bus_create_input_context (bus, "test");
	ibus_input_context_set_capabilites (context, 0);
	ibus_input_context_destroy (context);
	g_object_unref (context);
	g_object_unref (bus);
	
	g_main_loop_run (mainloop);
	
	return 0;
}
