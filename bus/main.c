#include "server.h"

int main()
{
	g_type_init ();
	
	BusServer *server;
	server = bus_server_get_default ();
	bus_server_listen (server);
	bus_server_run (server);
	
	return 0;
}
