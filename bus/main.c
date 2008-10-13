#include "server.h"

int main()
{
	g_type_init ();
	
	GMainLoop *mainloop;
	BusServer *server;
	
	mainloop = g_main_loop_new (NULL, FALSE);

	server = bus_server_new ();
	g_main_loop_run (mainloop);
	return 0;
}
