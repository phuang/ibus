/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#include "ibus.h"

int main()
{
    ibus_init ();

    GMainLoop *mainloop;
    IBusConnection *connection;
    IBusService *service;

    mainloop = g_main_loop_new (NULL, FALSE);
    connection = ibus_connection_open ("unix:path=/tmp/ibus-phuang/ibus--0.0");
    service = IBUS_SERVICE (ibus_engine_new ("/a/Engine", connection));
    g_main_loop_run (mainloop);
    return 0;
}
