/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */

#include <ibus.h>
#include <stdlib.h>
#include <locale.h>
#include "config.h"

static IBusBus *bus = NULL;
static IBusConfigGConf *config = NULL;

/* options */
static gboolean ibus = FALSE;
static gboolean verbose = FALSE;

static const GOptionEntry entries[] =
{
    { "ibus", 'i', 0, G_OPTION_ARG_NONE, &ibus, "component is executed by ibus", NULL },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL },
    { NULL },
};


static void
ibus_disconnected_cb (IBusBus  *bus,
                      gpointer  user_data)
{
    ibus_quit ();
}

static void
ibus_gconf_start (void)
{
    ibus_init ();
    bus = ibus_bus_new ();
    if (!ibus_bus_is_connected (bus)) {
        exit (-1);
    }
    g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);
    config = ibus_config_gconf_new (ibus_bus_get_connection (bus));
    ibus_bus_request_name (bus, IBUS_SERVICE_CONFIG, 0);
    ibus_main ();
}

gint
main (gint argc, gchar **argv)
{
    GError *error = NULL;
    GOptionContext *context;

    setlocale (LC_ALL, "");

    context = g_option_context_new ("- ibus gconf component");

    g_option_context_add_main_entries (context, entries, "ibus-gconf");

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("Option parsing failed: %s\n", error->message);
        g_error_free (error);
        exit (-1);
    }

    ibus_gconf_start ();

    return 0;
}
