/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#include "ibus.h"
#include "stdio.h"

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

	message = ibus_message_new_signal ("/org/freedesktop/IBus",
									   "org.freedesktop.IBus",
									   "Test");

	IBusSerializable *p = ibus_serializable_new ();
	retval = ibus_message_append_args (message,
									   IBUS_TYPE_SERIALIZABLE, &p,
									   IBUS_TYPE_SERIALIZABLE, &p,
									   IBUS_TYPE_SERIALIZABLE, &p,
									   IBUS_TYPE_SERIALIZABLE, &p,
									   IBUS_TYPE_SERIALIZABLE, &p,
									   G_TYPE_INVALID);
	g_assert (retval);

	retval = ibus_message_get_args (message,
									&error,
									IBUS_TYPE_SERIALIZABLE, &p,
									IBUS_TYPE_SERIALIZABLE, &p,
									IBUS_TYPE_SERIALIZABLE, &p,
									IBUS_TYPE_SERIALIZABLE, &p,
									IBUS_TYPE_SERIALIZABLE, &p,
									G_TYPE_INVALID);
	g_assert (retval);

	return 0;

}
