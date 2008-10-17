#include "ibus.h"

int main()
{
	g_type_init ();
	
	IBusConnection *connection;
	IBusProxy *proxy;

	connection = ibus_connection_new ();
	proxy = ibus_proxy_new ("a", "/a", connection);
	GValue value = {0};
	g_value_init (&value, G_TYPE_STRING);
	g_value_set_static_string (&value, "aaa");
	g_object_set_property (proxy, "name", &value);
	
	return 0;
}
