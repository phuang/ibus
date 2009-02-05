#include "ibus.h"


void connection_destroy_cb (IBusConnection *connection, gpointer user_data)
{
	g_debug ("connnection %p destroyed", connection );
}

void new_connection_cb (IBusServer *server, IBusConnection *connection, gpointer user_data)
{
	g_debug ("new-connection %p", connection);
	g_signal_connect (connection, "destroy", (GCallback) connection_destroy_cb, 0);
}

int main()
{
	g_type_init ();

	GMainLoop *mainloop;
	IBusServer *server;

	mainloop = g_main_loop_new (NULL, FALSE);
	server = ibus_server_new ();
	ibus_server_listen (server, "unix:abstract=/tmp/1234567");
	g_signal_connect (server, "new-connection", (GCallback) new_connection_cb, 0);

	g_main_loop_run (mainloop);

	return 0;
}
