/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */

#include <string.h>
#include "ibus.h"

static IBusBus *bus = NULL;
static int create_config_count = 0;

static void
finish_create_config_async_sucess (GObject      *source_object,
                                   GAsyncResult *res,
                                   gpointer      user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    GError *error = NULL;
    IBusConfig *config =
          ibus_config_new_async_finish (res, &error);

    g_assert (IBUS_IS_CONFIG (config));
    g_object_unref (config);
    if (--create_config_count == 0)
        g_main_loop_quit (loop);
}

static void
finish_create_config_async_failed (GObject      *source_object,
                                   GAsyncResult *res,
                                   gpointer      user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    GError *error = NULL;
    IBusConfig *config =
            ibus_config_new_async_finish (res, &error);

    g_assert (config == NULL);
    g_assert (error != NULL);
    g_error_free (error);
    if (--create_config_count <= 0)
        g_main_loop_quit (loop);
}

static void
test_create_config_async (void)
{
    GMainLoop *loop = NULL;
    GCancellable *cancellable = NULL;

    /* create an IC */
    create_config_count = 1;
    loop = g_main_loop_new (NULL, TRUE);
    ibus_config_new_async (ibus_bus_get_connection (bus),
                           NULL,
                           finish_create_config_async_sucess,
                           loop);
    g_main_loop_run (loop);
    g_main_loop_unref (loop);

    /* call create, and then cancel */
    create_config_count = 1;
    loop = g_main_loop_new (NULL, TRUE);
    cancellable = g_cancellable_new ();

    ibus_config_new_async (ibus_bus_get_connection (bus),
                           cancellable,
                           finish_create_config_async_failed,
                           loop);
    g_cancellable_cancel (cancellable);
    g_object_unref (cancellable);
    g_main_loop_run (loop);
    g_main_loop_unref (loop);
}

gint
main (gint    argc,
      gchar **argv)
{
    gint result;
    g_type_init ();
    g_test_init (&argc, &argv, NULL);
    ibus_init ();
    bus = ibus_bus_new ();

    g_test_add_func ("/ibus/create-config-async", test_create_config_async);

    result = g_test_run ();
    g_object_unref (bus);

    return result;
}
