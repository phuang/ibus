/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#include "ibus.h"

static
_value_changed_cb (IBusConfig *config, gchar *section, gchar *name, GValue *value, gpointer data)
{
    g_debug ("value-changed %s %s", section, name);
}

int main()
{
    IBusBus *bus;
    IBusConfig *config;

    ibus_init ();

    bus = ibus_bus_new ();
    config = ibus_bus_get_config (bus);

    g_signal_connect (config,
                      "value-changed",
                      G_CALLBACK (_value_changed_cb),
                      NULL);
    g_main_loop_run (g_main_loop_new (NULL, FALSE));

    return 0;
}
