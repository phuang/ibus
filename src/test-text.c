/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#include "ibus.h"

int main()
{
	g_type_init ();

	IBusText *text1;
	IBusText *text2;
	IBusMessage *message;
	IBusError *error;
	gboolean retval;

	text1 = ibus_text_new_from_string ("Hello");
	text2 = ibus_text_new_from_static_string ("Hello");

	message = ibus_message_new_signal ("/org/freedesktop/IBus",
									   "org.freedesktop.IBus",
									   "Test");

	retval = ibus_message_append_args (message,
									   IBUS_TYPE_SERIALIZABLE, &text1,
									   IBUS_TYPE_SERIALIZABLE, &text2,
									   G_TYPE_INVALID);
	g_assert (retval);
	g_object_unref (text1);
	g_object_unref (text2);

	retval = ibus_message_get_args (message,
									&error,
									IBUS_TYPE_SERIALIZABLE, &text1,
									IBUS_TYPE_SERIALIZABLE, &text2,
									G_TYPE_INVALID);
	g_assert (retval);
	g_assert_cmpstr (text1->text, ==, "Hello");
	g_assert_cmpstr (text2->text, ==, "Hello");

	g_object_unref (text1);
	g_object_unref (text2);

	return 0;

}
