#include "ibus.h"

int main()
{
	g_type_init ();
	
	GMainLoop *mainloop;
	IBusBus *bus;
	
	mainloop = g_main_loop_new (NULL, FALSE);
	bus = ibus_bus_new ();
	g_main_loop_run (mainloop);
	
	return 0;
}
