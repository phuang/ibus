#include "ibus.h"

int main()
{
	g_type_init ();
	IBusLookupTable *table, *table1, table2;
	IBusMessage *message;
	IBusError *error;
	gboolean retval;

	table = ibus_lookup_table_new (9, 3, TRUE);
	ibus_lookup_table_append_candidate (table, "Hello", ibus_attr_list_new ());
	ibus_lookup_table_append_candidate (table, "Cool", ibus_attr_list_new ());

	message = ibus_message_new (DBUS_MESSAGE_TYPE_METHOD_CALL);
	
	retval = ibus_message_append_args (message,
							  		   IBUS_TYPE_LOOKUP_TABLE, &table,
							  		   IBUS_TYPE_LOOKUP_TABLE, &table,
							  		   G_TYPE_INVALID);
	g_assert (retval);

	ibus_lookup_table_unref (table);
	table = table1 = NULL;

	retval = ibus_message_get_args (message,
								    &error,
								    IBUS_TYPE_LOOKUP_TABLE, &table,
								    IBUS_TYPE_LOOKUP_TABLE, &table1,
									G_TYPE_INVALID);
	g_assert (retval);
	g_assert (table);
	g_assert (table1);

	ibus_lookup_table_unref (table);
	ibus_lookup_table_unref (table1);
	
	retval = ibus_message_get_args (message,
								    &error,
								    IBUS_TYPE_LOOKUP_TABLE, &table,
								    IBUS_TYPE_LOOKUP_TABLE, &table1,
								    IBUS_TYPE_LOOKUP_TABLE, &table2,
									G_TYPE_INVALID);
	g_assert (!retval);

	g_debug ("%s: %s", error->name, error->message);
	ibus_error_free (error);

	return 0;
}
