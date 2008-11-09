#include "ibus.h"

int main()
{
	g_type_init ();
	IBusAttrList *list;
	IBusMessage *message;
	IBusError *error;

	g_debug ("%d", IBUS_TYPE_OBJECT_PATH);

	list = ibus_attr_list_new ();
	ibus_attr_list_append (list, ibus_attribute_new (1, 1, 1, 2));
	ibus_attr_list_append (list, ibus_attribute_new (2, 1, 1, 2));
	ibus_attr_list_append (list, ibus_attribute_new (3, 1, 1, 2));
	ibus_attr_list_append (list, ibus_attribute_new (3, 1, 1, 2));

	message = ibus_message_new (DBUS_MESSAGE_TYPE_METHOD_CALL);
	ibus_message_append_args (message,
							  IBUS_TYPE_ATTR_LIST, &list,
							  G_TYPE_INVALID);
	ibus_attr_list_unref (list);
	
	return 0;
}
