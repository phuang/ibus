#include "ibus.h"

int main()
{
	GMainLoop *mainloop;
	IBusConnection *connection;

	g_type_init ();
	mainloop = g_main_loop_new (NULL, FALSE);
	connection = ibus_connection_open ("unix:path=/tmp/ibus-phuang/ibus--0.0");
	g_main_loop_run (mainloop);
}
