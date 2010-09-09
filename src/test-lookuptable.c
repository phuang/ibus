/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#include <dbus/dbus.h>
#include "ibus.h"

int main()
{
	g_type_init ();
	IBusLookupTable *table, *table1;
	IBusMessage *message;
	IBusError *error;
	gboolean retval;

	table = ibus_lookup_table_new (9, 0, TRUE, FALSE);
	ibus_lookup_table_append_candidate (table, ibus_text_new_from_static_string ("Hello"));
	ibus_lookup_table_append_candidate (table, ibus_text_new_from_static_string ("Cool"));

	message = ibus_message_new (DBUS_MESSAGE_TYPE_METHOD_CALL);

	retval = ibus_message_append_args (message,
									   IBUS_TYPE_LOOKUP_TABLE, &table,
									   IBUS_TYPE_LOOKUP_TABLE, &table,
									   G_TYPE_INVALID);
	g_assert (retval);

	g_object_unref (table);
	table = table1 = NULL;

	retval = ibus_message_get_args (message,
								    &error,
								    IBUS_TYPE_LOOKUP_TABLE, &table,
								    IBUS_TYPE_LOOKUP_TABLE, &table1,
									G_TYPE_INVALID);
	g_assert (retval);
	g_assert (table);
	g_assert (table1);

	g_object_unref (table);
	g_object_unref (table1);

	return 0;
}
