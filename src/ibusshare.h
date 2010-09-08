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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/**
 * SECTION: ibusshare
 * @short_description: Shared utility functions and definition.
 * @stability: Stable
 *
 * This file defines some utility functions and definition
 * which are shared among ibus component and services.
 */

#ifndef __IBUS_SHARE_H_
#define __IBUS_SHARE_H_

#include <glib.h>

/**
 * IBUS_SERVICE_IBUS:
 *
 * Address of IBus service.
 */
#define IBUS_SERVICE_IBUS       "org.freedesktop.IBus"

/**
 * IBUS_SERVICE_PANEL:
 *
 * Address of IBus panel service.
 */
#define IBUS_SERVICE_PANEL      "org.freedesktop.IBus.Panel"

/**
 * IBUS_SERVICE_CONFIG:
 *
 * Address of IBus config service.
 */
#define IBUS_SERVICE_CONFIG     "org.freedesktop.IBus.Config"

/**
 * IBUS_SERVICE_NOTIFICATIONS:
 *
 * Address of IBus notification service.
 */
#define IBUS_SERVICE_NOTIFICATIONS    "org.freedesktop.IBus.Notifications"

/**
 * IBUS_PATH_IBUS:
 *
 * D-Bus path for IBus
 */
#define IBUS_PATH_IBUS          "/org/freedesktop/IBus"

/**
 * IBUS_PATH_FACTORY:
 *
 * D-Bus path for IBus factory.
 */
#define IBUS_PATH_FACTORY       "/org/freedesktop/IBus/Factory"

/**
 * IBUS_PATH_PANEL:
 *
 * D-Bus path for IBus panel.
 */
#define IBUS_PATH_PANEL         "/org/freedesktop/IBus/Panel"

/**
 * IBUS_PATH_CONFIG:
 *
 * D-Bus path for IBus config.
 */
#define IBUS_PATH_CONFIG        "/org/freedesktop/IBus/Config"

/**
 * IBUS_PATH_NOTIFICATIONS:
 *
 * D-Bus path for IBus notifications.
 */
#define IBUS_PATH_NOTIFICATIONS "/org/freedesktop/IBus/Notifications"

/**
 * IBUS_PATH_INPUT_CONTEXT:
 *
 * Template of D-Bus path for IBus input context.
 */
#define IBUS_PATH_INPUT_CONTEXT "/org/freedesktop/IBus/InputContext_%d"

/**
 * IBUS_INTERFACE_IBUS:
 *
 * D-Bus interface for IBus.
 */
#define IBUS_INTERFACE_IBUS     "org.freedesktop.IBus"

/**
 * IBUS_INTERFACE_INPUT_CONTEXT:
 *
 * D-Bus interface for IBus input context.
 */
#define IBUS_INTERFACE_INPUT_CONTEXT \
                                "org.freedesktop.IBus.InputContext"

/**
 * IBUS_INTERFACE_FACTORY:
 *
 * D-Bus interface for IBus factory.
 */
#define IBUS_INTERFACE_FACTORY  "org.freedesktop.IBus.Factory"

/**
 * IBUS_INTERFACE_ENGINE:
 *
 * D-Bus interface for IBus engine.
 */
#define IBUS_INTERFACE_ENGINE   "org.freedesktop.IBus.Engine"

/**
 * IBUS_INTERFACE_PANEL:
 *
 * D-Bus interface for IBus panel.
 */
#define IBUS_INTERFACE_PANEL    "org.freedesktop.IBus.Panel"

/**
 * IBUS_INTERFACE_CONFIG:
 *
 * D-Bus interface for IBus config.
 */
#define IBUS_INTERFACE_CONFIG   "org.freedesktop.IBus.Config"

/**
 * IBUS_INTERFACE_NOTIFICATIONS:
 *
 * D-Bus interface for IBus notifications.
 */
#define IBUS_INTERFACE_NOTIFICATIONS    "org.freedesktop.IBus.Notifications"

G_BEGIN_DECLS

/**
 * ibus_get_local_machine_id:
 * @returns: A newly allocated string that shows the UUID of the machine.
 *
 * Obtains the machine UUID of the machine this process is running on.
 */
const gchar     *ibus_get_local_machine_id
                                        (void);

/**
 * ibus_set_display:
 * @display: Display address, as in DISPLAY environment for X.
 *
 * Set the display address.
 */
void             ibus_set_display       (const gchar    *display);

/**
 * ibus_get_address:
 * @returns: D-Bus address of IBus. %NULL for not found.
 *
 * Return the D-Bus address of IBus.
 * It will find the address from following source:
 * <orderedlist>
 *    <listitem><para>Environment variable IBUS_ADDRESS</para></listitem>
 *    <listitem><para>Socket file under ~/.config/ibus/bus/</para></listitem>
 * </orderedlist>
 *
 * @see_also: ibus_write_address().
 */
const gchar     *ibus_get_address       (void);

/**
 * ibus_write_address:
 * @address: D-Bus address of IBus.
 *
 * Write D-Bus address to socket file.
 *
 * @see_also: ibus_get_address().
 */
void             ibus_write_address     (const gchar    *address);

/**
 * ibus_get_user_name:
 * @returns: A newly allocated string that stores current user name.
 *
 * Get the current user name.
 * It is determined by:
 * <orderedlist>
 *    <listitem><para>getlogin()</para></listitem>
 *    <listitem><para>Environment variable SUDO_USER</para></listitem>
 *    <listitem><para>Environment variable USERHELPER_UID</para></listitem>
 *    <listitem><para>Environment variable USERNAME</para></listitem>
 *    <listitem><para>Environment variable LOGNAME</para></listitem>
 *    <listitem><para>Environment variable USER</para></listitem>
 *    <listitem><para>Environment variable LNAME</para></listitem>
 * </orderedlist>
 */
const gchar     *ibus_get_user_name     (void);

/**
 * ibus_get_daemon_uid:
 * @returns: UID of ibus-daemon; or 0 if UID is not available.
 *
 * Get UID of ibus-daemon.
 *
 * Deprecated: This function has been deprecated and should
 * not be used in newly written code.
 */
glong            ibus_get_daemon_uid    (void) G_GNUC_DEPRECATED;

/**
 * ibus_get_socket_path:
 * @returns: A newly allocated string that stores the path of socket file.
 *
 * Get the path of socket file.
 */
const gchar     *ibus_get_socket_path   (void);

/**
 * ibus_keyval_name:
 * @keyval: Key symbol.
 * @returns: Corresponding key name. %NULL if no such key symbol.
 *
 * Return the name of a key symbol.
 *
 * Note that the returned string is used internally, so don't free it.
 */
const gchar     *ibus_keyval_name       (guint           keyval);

/**
 * ibus_keyval_from_name:
 * @keyval_name: Key name in #gdk_keys_by_name.
 * @returns: Corresponding key symbol.
 *
 * Return the key symbol that associate with the key name.
 */
guint            ibus_keyval_from_name  (const gchar    *keyval_name);

/**
 * ibus_free_strv:
 * @strv: List of strings.
 *
 * Free a list of strings.
 * Deprecated: This function has been deprecated and should
 * not be used in newly written code.
 */
void             ibus_free_strv         (gchar          **strv)
    G_GNUC_DEPRECATED;

/**
 * ibus_key_event_to_string:
 * @keyval: Key symbol.
 * @modifiers: Modifiers such as Ctrl or Shift.
 * @returns: The name of a key symbol and modifier.
 *
 * Return the name of a key symbol and modifiers.
 *
 * For example, if press ctrl, shift, and enter, then this function returns:
 * Shift+Control+enter.
 */
const gchar     *ibus_key_event_to_string
                                        (guint           keyval,
                                         guint           modifiers);

/**
 * ibus_key_event_from_string:
 * @string: Key event string.
 * @keyval: Variable that hold key symbol result.
 * @modifiers: Variable that hold modifiers result.
 * @returns: TRUE for succeed; FALSE if failed.
 *
 * Parse key event string and return key symbol and modifiers.
 */
gboolean         ibus_key_event_from_string
                                        (const gchar    *string,
                                         guint          *keyval,
                                         guint          *modifiers);

/**
 * ibus_init:
 *
 * Init the ibus types.
 *
 * It is actually a wrapper of g_type_init().
 */
void             ibus_init              (void);

/**
 * ibus_main:
 *
 * Runs an IBus main loop until ibus_quit() is called in the loop.
 *
 * @see_also: ibus_quit().
 */
void             ibus_main              (void);

/**
 * ibus_quit:
 *
 * Stops an IBus from running.
 *
 * Any calls to ibus_quit() for the loop will return.
 * @see_also: ibus_main().
 */
void             ibus_quit              (void);

G_END_DECLS
#endif
