#include "matchrule.c"

int main()
{
	BusMatchRule *rule;

	rule = bus_match_rule_new ("type='signal', interface='org.freedesktop.IBus' ");
	g_assert (rule->message_type == DBUS_MESSAGE_TYPE_SIGNAL);
	g_assert (g_strcmp0 (rule->interface, "org.freedesktop.IBus") == 0 );
	bus_match_rule_unref (rule);
	
	rule = bus_match_rule_new ("type='method_call', interface='org.freedesktop.IBus' ");
	g_assert (rule->message_type == DBUS_MESSAGE_TYPE_METHOD_CALL);
	g_assert (g_strcmp0 (rule->interface, "org.freedesktop.IBus") == 0 );
	bus_match_rule_unref (rule);

	return 0;
}
