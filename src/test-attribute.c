#include "ibus.h"

int main()
{
	g_type_init ();
	IBusAttrList *list;
	IBusMessage *message;
	gboolean retval;
	IBusError *error;

	list = ibus_attr_list_new ();
	ibus_attr_list_append (list, ibus_attribute_new (1, 1, 1, 2));
	ibus_attr_list_append (list, ibus_attribute_new (2, 1, 1, 2));
	ibus_attr_list_append (list, ibus_attribute_new (3, 1, 1, 2));
	ibus_attr_list_append (list, ibus_attribute_new (3, 1, 1, 2));

	message = ibus_message_new (DBUS_MESSAGE_TYPE_METHOD_CALL);
	retval = ibus_message_append_args (message,
							  		   IBUS_TYPE_ATTR_LIST, &list,
							  		   G_TYPE_INVALID);
	ibus_attr_list_unref (list);
	g_assert (retval);
	
	list = NULL;
	ibus_message_get_args (message,
						   &error,
						   IBUS_TYPE_ATTR_LIST, &list,
						   G_TYPE_INVALID);
	g_assert (list != NULL);
	ibus_attr_list_unref (list);
	
	return 0;
}
