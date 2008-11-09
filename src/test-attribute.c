#include "ibus.h"

int main()
{
	g_type_init ();
	IBusAttrList *list, *list1, *list2, *list3, *list4;
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
							  		   IBUS_TYPE_ATTR_LIST, &list,
							  		   IBUS_TYPE_ATTR_LIST, &list,
							  		   IBUS_TYPE_ATTR_LIST, &list,
							  		   G_TYPE_INVALID);
	ibus_attr_list_unref (list);
	g_assert (retval);
	
	list = list1 = list2 = list3 = NULL;
	retval = ibus_message_get_args (message,
						   			&error,
						   			IBUS_TYPE_ATTR_LIST, &list,
						   			IBUS_TYPE_ATTR_LIST, &list1,
						   			IBUS_TYPE_ATTR_LIST, &list2,
						   			IBUS_TYPE_ATTR_LIST, &list3,
						   			G_TYPE_INVALID);

	g_assert (retval);
	g_assert (list != NULL);
	g_assert (list1 != NULL);
	g_assert (list2 != NULL);
	g_assert (list3 != NULL);

	ibus_attr_list_unref (list);
	ibus_attr_list_unref (list1);
	ibus_attr_list_unref (list2);
	ibus_attr_list_unref (list3);
	
	retval = ibus_message_get_args (message,
						   			&error,
						   			IBUS_TYPE_ATTR_LIST, &list,
						   			IBUS_TYPE_ATTR_LIST, &list1,
						   			IBUS_TYPE_ATTR_LIST, &list2,
						   			IBUS_TYPE_ATTR_LIST, &list3,
						   			G_TYPE_INVALID);
	
	g_assert (retval);
	g_assert (list != NULL);
	g_assert (list1 != NULL);
	g_assert (list2 != NULL);
	g_assert (list3 != NULL);

	ibus_attr_list_unref (list);
	ibus_attr_list_unref (list1);
	ibus_attr_list_unref (list2);
	ibus_attr_list_unref (list3);

	return 0;
}
