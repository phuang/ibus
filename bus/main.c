#include "server.h"

int main()
{
	g_type_init ();
	
	BusServer *server;
	server = bus_server_new ();
	bus_server_listen (server);
	bus_server_run (server);
	
	return 0;
}
