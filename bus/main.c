/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2010 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <config.h>
#include <fcntl.h>
#include <glib.h>
#include <gio/gio.h>
#include <ibus.h>
#include <locale.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "global.h"
#include "ibusimpl.h"
#include "server.h"

static gboolean daemonize = FALSE;
static gboolean single = FALSE;
static gboolean xim = FALSE;
static gboolean replace = FALSE;
static gboolean restart = FALSE;
static gchar *panel = "default";
static gchar *config = "default";
static gchar *desktop = "gnome";

static void
show_version_and_quit (void)
{
    g_print ("%s - Version %s\n", g_get_application_name (), VERSION);
    exit (EXIT_SUCCESS);
}

static const GOptionEntry entries[] =
{
    { "version",   'V', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, show_version_and_quit, "Show the application's version.", NULL },
    { "daemonize", 'd', 0, G_OPTION_ARG_NONE,   &daemonize, "run ibus as background process.", NULL },
    { "single",    's', 0, G_OPTION_ARG_NONE,   &single,    "do not execute panel and config module.", NULL },
    { "xim",       'x', 0, G_OPTION_ARG_NONE,   &xim,       "execute ibus XIM server.", NULL },
    { "desktop",   'n', 0, G_OPTION_ARG_STRING, &desktop,   "specify the name of desktop session. [default=gnome]", "name" },
    { "panel",     'p', 0, G_OPTION_ARG_STRING, &panel,     "specify the cmdline of panel program. pass 'disable' not to start a panel program.", "cmdline" },
    { "config",    'c', 0, G_OPTION_ARG_STRING, &config,    "specify the cmdline of config program. pass 'disable' not to start a config program.", "cmdline" },
    { "address",   'a', 0, G_OPTION_ARG_STRING, &g_address,   "specify the address of ibus daemon.", "address" },
    { "replace",   'r', 0, G_OPTION_ARG_NONE,   &replace,   "if there is an old ibus-daemon is running, it will be replaced.", NULL },
    { "cache",     't', 0, G_OPTION_ARG_STRING, &g_cache,   "specify the cache mode. [auto/refresh/none]", NULL },
    { "timeout",   'o', 0, G_OPTION_ARG_INT,    &g_gdbus_timeout, "gdbus reply timeout in milliseconds. pass -1 to use the default timeout of gdbus.", "timeout [default is 5000]" },
    { "mem-profile", 'm', 0, G_OPTION_ARG_NONE,   &g_mempro,   "enable memory profile, send SIGUSR2 to print out the memory profile.", NULL },
    { "restart",     'R', 0, G_OPTION_ARG_NONE,   &restart,    "restart panel and config processes when they die.", NULL },
    { "verbose",   'v', 0, G_OPTION_ARG_NONE,   &g_verbose,   "verbose.", NULL },
    { NULL },
};

/**
 * execute_cmdline:
 * @cmdline: An absolute path of the executable and its parameters, e.g.  "/usr/lib/ibus/ibus-x11 --kill-daemon".
 * @returns: TRUE if both parsing cmdline and executing the command succeed.
 *
 * Execute cmdline. Child process's stdin, stdout, and stderr are attached to /dev/null.
 * You don't have to handle SIGCHLD from the child process since glib will do.
 */
static gboolean
execute_cmdline (const gchar *cmdline)
{
    g_assert (cmdline);

    gint argc = 0;
    gchar **argv = NULL;
    GError *error = NULL;
    if (!g_shell_parse_argv (cmdline, &argc, &argv, &error)) {
        g_warning ("Can not parse cmdline `%s` exec: %s", cmdline, error->message);
        g_error_free (error);
        return FALSE;
    }

    error = NULL;
    gboolean retval = g_spawn_async (NULL, argv, NULL,
                            G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
                            NULL, NULL,
                            NULL, &error);
    g_strfreev (argv);

    if (!retval) {
        g_warning ("Can not execute cmdline `%s`: %s", cmdline, error->message);
        g_error_free (error);
        return FALSE;
    }

    return TRUE;
}

#ifndef HAVE_DAEMON
static void
closeall (gint fd)
{
    gint fdlimit = sysconf(_SC_OPEN_MAX);

    while (fd < fdlimit) {
      close(fd++);
    }
}

static gint
daemon (gint nochdir, gint noclose)
{
    switch (fork()) {
        case 0:  break;
        case -1: return -1;
        default: _exit(0);
    }

    if (setsid() < 0) {
      return -1;
    }

    switch (fork()) {
        case 0:  break;
        case -1: return -1;
        default: _exit(0);
    }

    if (!nochdir) {
      chdir("/");
    }

    if (!noclose) {
        closeall(0);
        open("/dev/null",O_RDWR);
        dup(0); dup(0);
    }
    return 0;
}
#endif

/*
 * _sig_usr2_handler:
 * @sig: the signal number, which is usually SIGUSR2.
 *
 * A signal handler for SIGUSR2 signal. Dump a summary of memory usage to stderr.
 */
static void
_sig_usr2_handler (int sig)
{
    g_mem_profile ();
}

gint
main (gint argc, gchar **argv)
{
    setlocale (LC_ALL, "");

    GOptionContext *context = g_option_context_new ("- ibus daemon");
    g_option_context_add_main_entries (context, entries, "ibus-daemon");

    g_argv = g_strdupv (argv);
    GError *error = NULL;
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_printerr ("Option parsing failed: %s\n", error->message);
	g_error_free (error);
        exit (-1);
    }
    if (g_gdbus_timeout < -1) {
        g_printerr ("Bad timeout (must be >= -1): %d\n", g_gdbus_timeout);
        exit (-1);
    }

    if (g_mempro) {
        g_mem_set_vtable (glib_mem_profiler_table);
        signal (SIGUSR2, _sig_usr2_handler);
    }

    /* check uid */
    {
        const gchar *username = ibus_get_user_name ();
        uid_t uid = getuid ();
        struct passwd *pwd = getpwuid (uid);

        if (pwd == NULL || g_strcmp0 (pwd->pw_name, username) != 0) {
            g_printerr ("Please run ibus-daemon with login user! Do not run ibus-daemon with sudo or su.\n");
            exit (-1);
        }
    }

    /* daemonize process */
    if (daemonize) {
        if (daemon (1, 0) != 0) {
            g_printerr ("Can not daemonize ibus.\n");
            exit (-1);
        }
    }

    /* create a new process group. this is important to kill all of its children by SIGTERM at a time in bus_ibus_impl_destroy. */
    setpgid (0, 0);

    ibus_init ();

    ibus_set_log_handler (g_verbose);

    /* check if ibus-daemon is running in this session */
    if (ibus_get_address () != NULL) {
        IBusBus *bus = ibus_bus_new ();

        if (ibus_bus_is_connected (bus)) {
            if (!replace) {
                g_printerr ("current session already has an ibus-daemon.\n");
                exit (-1);
            }
            ibus_bus_exit (bus, FALSE);
            while (ibus_bus_is_connected (bus)) {
                g_main_context_iteration (NULL, TRUE);
            }
        }
        g_object_unref (bus);
    }

    bus_server_init ();
    if (!single) {
        /* execute config component */
        if (g_strcmp0 (config, "default") == 0) {
            BusComponent *component;
            component = bus_registry_lookup_component_by_name (BUS_DEFAULT_REGISTRY, IBUS_SERVICE_CONFIG);
            if (component) {
                bus_component_set_restart (component, restart);
            }
            if (component == NULL || !bus_component_start (component, g_verbose)) {
                g_printerr ("Can not execute default config program\n");
                exit (-1);
            }
        } else if (g_strcmp0 (config, "disable") != 0 && g_strcmp0 (config, "") != 0) {
            if (!execute_cmdline (config))
                exit (-1);
        }

        /* execute panel component */
        if (g_strcmp0 (panel, "default") == 0) {
            BusComponent *component;
            component = bus_registry_lookup_component_by_name (BUS_DEFAULT_REGISTRY, IBUS_SERVICE_PANEL);
            if (component) {
                bus_component_set_restart (component, restart);
            }
            if (component == NULL || !bus_component_start (component, g_verbose)) {
                g_printerr ("Can not execute default panel program\n");
                exit (-1);
            }
        } else if (g_strcmp0 (panel, "disable") != 0 && g_strcmp0 (panel, "") != 0) {
            if (!execute_cmdline (panel))
                exit (-1);
        }
    }

    /* execute ibus xim server */
    if (xim) {
        if (!execute_cmdline (LIBEXECDIR "/ibus-x11 --kill-daemon"))
            exit (-1);
    }

    bus_server_run ();
    return 0;
}
