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
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include "ibusshare.h"

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

const gchar *
ibus_get_socket_path (void)
{
    static gchar *path = NULL;
    
    if (path == NULL) {
        gchar *display;
        gchar *hostname = "";
        gchar *displaynumber = "0";
        gchar *screennumber = "0";
        gchar *username = NULL;
        gchar *p;

        display = g_strdup (g_getenv ("DISPLAY"));
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

        username = ibus_get_user_name ();

        path = g_strdup_printf (
            "/tmp/ibus-%s/ibus-%s-%s.%s",
            username, hostname, displaynumber, screennumber);

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

