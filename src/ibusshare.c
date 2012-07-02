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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "ibusshare.h"
#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ibus.h>

static gchar *_display = NULL;

const gchar *
ibus_get_local_machine_id (void)
{
    static gchar *machine_id = NULL;

    if (machine_id == NULL) {
        if (!g_file_get_contents ("/var/lib/dbus/machine-id",
                                  &machine_id,
                                  NULL,
                                  NULL) &&
            /* /etc/machine-id is a fallback since dbus 1.6 */
            !g_file_get_contents ("/etc/machine-id",
                                  &machine_id,
                                  NULL,
                                  NULL) &&
            /* ultimately fallback on the kernel's boot-id */
            !g_file_get_contents ("/proc/sys/kernel/random/boot_id",
                                  &machine_id,
                                  NULL,
                                  NULL)) {
            g_warning ("Unable to find suitable machine-id");
        }
        else {
            g_strstrip (machine_id);
        }
    }

    return machine_id;
}

void
ibus_set_display (const gchar *display)
{
    if (_display != NULL)
        g_free (_display);
    _display = g_strdup (display);
}

const gchar *
ibus_get_user_name (void)
{
    return g_get_user_name ();
#if 0
    static gchar *username = NULL;
    if (username == NULL) {
        username = g_strdup (getlogin());
        if (username == NULL)
            username = g_strdup (g_getenv("SUDO_USER"));
        if (username == NULL) {
            const gchar *uid = g_getenv ("USERHELPER_UID");
            if (uid != NULL) {
                gchar *end;
                uid_t id = (uid_t)strtol(uid, &end, 10);
                if (uid != end) {
                    struct passwd *pw = getpwuid (id);
                    if (pw != NULL) {
                        username = g_strdup (pw->pw_name);
                    }
                }
            }
        }
        if (username == NULL)
            username = g_strdup (g_getenv("USERNAME"));
        if (username == NULL)
            username = g_strdup (g_getenv("LOGNAME"));
        if (username == NULL)
            username = g_strdup (g_getenv("USER"));
        if (username == NULL)
            username = g_strdup (g_getenv("LNAME"));

    }
    return username;
#endif
}

glong
ibus_get_daemon_uid (void)
{
    return getuid ();
#if 0
    struct passwd *pwd;
    uid_t uid;
    const gchar *username;

    uid = getuid ();

    if (uid != 0)
        return uid;

    username = ibus_get_user_name ();
    if (username == NULL)
        return 0;

    pwd = getpwnam (username);
    if (pwd == NULL)
        return 0;

    return pwd->pw_uid;
#endif
}

const gchar *
ibus_get_session_id (void)
{
    return g_getenv("IBUS_SESSION_ID");
}

const gchar *
ibus_get_socket_path (void)
{
    static gchar *path = NULL;

    if (path == NULL) {
        gchar *hostname = "unix";
        gchar *display;
        gchar *displaynumber = "0";
        /* gchar *screennumber = "0"; */
        gchar *p;

        path = g_strdup (g_getenv ("IBUS_ADDRESS_FILE"));
        if (path != NULL) {
            return path;
        }

        if (_display == NULL) {
            display = g_strdup (g_getenv ("DISPLAY"));
        }
        else {
            display = g_strdup (_display);
        }

        if (display == NULL) {
            g_warning ("DISPLAY is empty! We use default DISPLAY (:0.0)");
        }
        else {
            p = display;
            hostname = display;
            for (; *p != ':' && *p != '\0'; p++);

            if (*p == ':') {
                *p = '\0';
                p++;
                displaynumber = p;
            }

            for (; *p != '.' && *p != '\0'; p++);

            if (*p == '.') {
                *p = '\0';
                p++;
                /* Do not use screennumber
                 screennumber = p; */
            }
        }

        if (hostname[0] == '\0')
            hostname = "unix";

        p = g_strdup_printf ("%s-%s-%s",
                             ibus_get_local_machine_id (),
                             hostname,
                             displaynumber);
        path = g_build_filename (g_get_user_config_dir (),
                                 "ibus",
                                 "bus",
                                 p,
                                 NULL);
        g_free (p);
        g_free (display);
    }
    return path;
}

gint
ibus_get_timeout (void)
{
    /* 6000 ms is the default timeout on the ibus-daemon side (5 sec) plus 1. */
    static const gint default_timeout = 6000;

    static gint64 timeout = -2;
    if (timeout == -2) {
        const gchar *timeout_str = g_getenv ("IBUS_TIMEOUT");
        if (timeout_str == NULL) {
            timeout = default_timeout;
        } else {
            timeout = g_ascii_strtoll(timeout_str, NULL, 10);
            if (timeout < -1 || timeout == 0 || timeout > G_MAXINT) {
                timeout = default_timeout;
            }
        }
    }
    return timeout;
}

const gchar *
ibus_get_address (void)
{
    static gchar *address = NULL;
    pid_t pid = -1;
    static gchar buffer[1024];
    FILE *pf;

    /* free address */
    if (address != NULL) {
        g_free (address);
        address = NULL;
    }

    /* get address from env variable */
    address = g_strdup (g_getenv ("IBUS_ADDRESS"));
    if (address) {
        return address;
    }

    /* read address from ~/.config/ibus/bus/soketfile */
    pf = fopen (ibus_get_socket_path (), "r");
    if (pf == NULL) {
        return NULL;
    }

    while (!feof (pf)) {
        gchar *p = buffer;
        if (fgets (buffer, sizeof (buffer), pf) == NULL)
            break;

        /* skip comment line */
        if (p[0] == '#')
            continue;
        /* parse IBUS_ADDRESS */
        if (strncmp (p, "IBUS_ADDRESS=", sizeof ("IBUS_ADDRESS=") - 1) == 0) {
            address = p + sizeof ("IBUS_ADDRESS=") - 1;
            for (p = (gchar *)address; *p != '\n' && *p != '\0'; p++);
            if (*p == '\n')
                *p = '\0';
            address = g_strdup (address);
            continue;
        }

        /* parse IBUS_DAEMON_PID */
        if (strncmp (p, "IBUS_DAEMON_PID=", sizeof ("IBUS_DAEMON_PID=") - 1) == 0) {
            pid = atoi(p + sizeof ("IBUS_DAEMON_PID=") - 1);
            continue;
        }

    }
    fclose (pf);

    if (pid == -1 || kill (pid, 0) != 0) {
        return NULL;
    }

    return address;
}

void
ibus_write_address (const gchar *address)
{
    FILE *pf;
    gchar *path;
    g_return_if_fail (address != NULL);

    path = g_path_get_dirname (ibus_get_socket_path ());
    g_mkdir_with_parents (path, 0700);
    g_free (path);

    g_unlink (ibus_get_socket_path ());
    pf = fopen (ibus_get_socket_path (), "w");
    g_return_if_fail (pf != NULL);

    fprintf (pf,
        "# This file is created by ibus-daemon, please do not modify it\n"
        "IBUS_ADDRESS=%s\n"
        "IBUS_DAEMON_PID=%ld\n",
        address, (glong) getpid ());
    fclose (pf);
}

void
ibus_free_strv (gchar **strv)
{
    gchar **p;

    if (strv == NULL)
        return;

    for (p = strv; *p != NULL; p++) {
        g_free (*p);
    }

    g_free (strv);
}

void
ibus_init (void)
{
    g_type_init ();
    IBUS_ERROR;
    IBUS_TYPE_TEXT;
    IBUS_TYPE_ATTRIBUTE;
    IBUS_TYPE_ATTR_LIST;
    IBUS_TYPE_LOOKUP_TABLE;
    IBUS_TYPE_COMPONENT;
    IBUS_TYPE_ENGINE_DESC;
}

static GMainLoop *main_loop = NULL;

void
ibus_main (void)
{
    main_loop = g_main_loop_new (NULL, FALSE);

    g_main_loop_run (main_loop);

    g_main_loop_unref (main_loop);
    main_loop = NULL;
}

void
ibus_quit (void)
{
    if (main_loop) {
        g_main_loop_quit (main_loop);
    }
}

static gboolean ibus_log_handler_is_verbose = FALSE;
static guint ibus_log_handler_id = 0;

static void
ibus_log_handler (const gchar    *log_domain,
                  GLogLevelFlags  log_level,
                  const gchar    *message,
                  gpointer        user_data)
{
    // In the quiet mode (i.e. not verbose), we'll ignore DEBUG and
    // WARNING messages.
    if (!ibus_log_handler_is_verbose &&
        ((log_level & G_LOG_LEVEL_DEBUG) ||
         (log_level & G_LOG_LEVEL_WARNING))) {
        return;
    }
    // Add timing info like "17:34:57.680038" (hour, min, sec, microsecond).
    struct timeval time_val;
    gettimeofday (&time_val, NULL);
    struct tm local_time;
    localtime_r (&time_val.tv_sec, &local_time);
    char* new_message =
        g_strdup_printf ("%02d:%02d:%02d.%6d: %s",
                         local_time.tm_hour,
                         local_time.tm_min,
                         local_time.tm_sec,
                         (int)time_val.tv_usec,
                         message);
    g_log_default_handler (log_domain, log_level, new_message, user_data);
    g_free (new_message);
}

void
ibus_set_log_handler (gboolean verbose)
{
    if (ibus_log_handler_id != 0) {
        g_log_remove_handler (G_LOG_DOMAIN, ibus_log_handler_id);
    }

    ibus_log_handler_is_verbose = verbose;
    ibus_log_handler_id = g_log_set_handler (G_LOG_DOMAIN,
                                             G_LOG_LEVEL_MASK,
                                             ibus_log_handler,
                                             NULL);
}

void
ibus_unset_log_handler (void)
{
    if (ibus_log_handler_id != 0) {
        g_log_remove_handler (G_LOG_DOMAIN, ibus_log_handler_id);
        ibus_log_handler_id = 0;
    }
}
