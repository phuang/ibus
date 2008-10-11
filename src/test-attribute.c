#include "ibus.h"

int main()
{
	g_type_init ();
	IBusAttrList *list;
	DBusMessage *message;

	list = ibus_attr_list_new ();
	ibus_attr_list_append (list, ibus_attribute_new (1, 1, 1, 2));
	ibus_attr_list_append (list, ibus_attribute_new (2, 1, 1, 2));
	ibus_attr_list_append (list, ibus_attribute_new (3, 1, 1, 2));
	ibus_attr_list_append (list, ibus_attribute_new (3, 1, 1, 2));

	DBusMessageIter iter;
	
	message = dbus_message_new (DBUS_MESSAGE_TYPE_METHOD_CALL);
	dbus_message_iter_init_append (message, &iter);
	ibus_attr_list_to_dbus_message (list, &iter);
	ibus_attr_list_unref (list);
	
	dbus_message_iter_init (message, &iter);
	list = ibus_attr_list_from_dbus_message (&iter);
	ibus_attr_list_unref (list);
	
	return 0;
}
