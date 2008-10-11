#include "ibus.h"

int main()
{
	GMainLoop *mainloop;
	IBusConnection *connection;

	g_type_init ();
	mainloop = g_main_loop_new (NULL, FALSE);
	connection = ibus_connection_open ("unix:path=/tmp/ibus-phuang/ibus--0.0");
	IBusService *service = IBUS_SERVICE (ibus_factory_new ("/a/a", "Test", "zh_CN", "ibus", "Huang Peng", "GPL", "/e/e"));
	GValue value = {0};
	g_value_init (&value, G_TYPE_STRING);
	g_object_get_property (G_OBJECT (service), "authors", &value);
	g_debug ("authors=%s", g_value_get_string (&value));
	g_value_unset (&value);
	g_main_loop_run (mainloop);
	return 0;
}
