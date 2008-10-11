#include "ibus.h"

int main()
{
	g_type_init ();
	IBusLookupTable *table;
	DBusMessage *message;

	table = ibus_lookup_table_new (9, 3, TRUE);
	ibus_lookup_table_append_candidate (table, "Hello", ibus_attr_list_new ());
	ibus_lookup_table_append_candidate (table, "Cool", ibus_attr_list_new ());

	DBusMessageIter iter;
	
	message = dbus_message_new (DBUS_MESSAGE_TYPE_METHOD_CALL);
	dbus_message_iter_init_append (message, &iter);
	ibus_lookup_table_to_dbus_message (table, &iter);
	
	dbus_message_iter_init (message, &iter);
	table = ibus_lookup_table_from_dbus_message (&iter);
	return 0;
}
