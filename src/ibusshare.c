/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang@gmail.com>
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

#include <glib-object.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include "ibusshare.h"

static gchar *_display = NULL;

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
}

glong
ibus_get_daemon_uid (void)
{
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
}

const gchar *
ibus_get_session_id (void)
{
    return g_getenv("IBUS_SESSION_ID");
}

const gchar *
ibus_get_socket_folder (void)
{
    static gchar *folder = NULL;

    if (folder == NULL) {
        const gchar *session = ibus_get_session_id ();
        if (session && session[0] != '\0') {
            folder = g_strdup_printf ("/tmp/ibus-%s-%s",
                ibus_get_user_name (), session);
        }
        else {
            folder = g_strdup_printf ("/tmp/ibus-%s",
                ibus_get_user_name ());
        }
    }
    return folder;
}

const gchar *
ibus_get_socket_path (void)
{
    static gchar *path = NULL;

    if (path == NULL) {
        gchar *hostname = "unix";
        gchar *display;
        gchar *displaynumber = "0";
        gchar *screennumber = "0";
        const gchar *folder= NULL;
        gchar *p;

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
                screennumber = p;
            }
        }

        folder = ibus_get_socket_folder ();

        if (hostname[0] == '\0')
            hostname = "unix";

        path = g_strdup_printf (
            "%s/ibus-%s-%s",
            folder, hostname, displaynumber);
        g_free (display);
    }
    return path;
}

const gchar *
ibus_get_address (void)
{
    static gchar *address = NULL;

    if (address == NULL) {
        address = g_strdup_printf (
            "unix:path=%s",
            ibus_get_socket_path ());
    }
    return address;
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
