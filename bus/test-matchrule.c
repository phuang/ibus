#include <dbus/dbus.h>
#include "matchrule.h"

int
main(gint argc, gchar **argv)
{
        BusMatchRule *rule, *rule1;
        g_type_init ();

        rule = bus_match_rule_new (" type='signal' , interface = 'org.freedesktop.IBus' ");
        g_assert (rule->message_type == DBUS_MESSAGE_TYPE_SIGNAL);
        g_assert (g_strcmp0 (rule->interface, "org.freedesktop.IBus") == 0 );
        g_object_unref (rule);

        rule = bus_match_rule_new ("type='method_call', interface='org.freedesktop.IBus' ");
        g_assert (rule->message_type == DBUS_MESSAGE_TYPE_METHOD_CALL);
        g_assert (g_strcmp0 (rule->interface, "org.freedesktop.IBus") == 0 );
        g_object_unref (rule);

        rule = bus_match_rule_new ("type='signal',"
                                                           "interface='org.freedesktop.DBus',"
                                                           "member='NameOwnerChanged',"
                                                           "arg0='ibus.freedesktop.IBus.config',"
                                                           "arg0='ibus.freedesktop.IBus.config',"
                                                           "arg2='ibus.freedesktop.IBus.config'");
        g_assert (rule->message_type == DBUS_MESSAGE_TYPE_SIGNAL);
        g_assert (g_strcmp0 (rule->interface, "org.freedesktop.DBus") == 0 );
        rule1 = bus_match_rule_new ("type='signal',"
                                                           "interface='org.freedesktop.DBus',"
                                                           "member='NameOwnerChanged',"
                                                           "arg0='ibus.freedesktop.IBus.config',"
                                                           "arg0='ibus.freedesktop.IBus.config',"
                                                           "arg2='ibus.freedesktop.IBus.config'");

        g_assert (bus_match_rule_is_equal (rule, rule1));

        g_object_unref (rule);
        g_object_unref (rule1);

        return 0;
}
