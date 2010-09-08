/* vim:set et sts=4: */
/* IBus - The Input Bus
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
 * SECTION: ibushotkey
 * @short_description: Hotkeys and associated events.
 * @stability: Stable
 *
 * An IBusHotkeyProfile associates a hotkey and an event.
 */
#ifndef __IBUS_HOTKEY_H_
#define __IBUS_HOTKEY_H_

#include "ibusserializable.h"

/*
 * Type macros.
 */
/* define IBusHotkeyProfile macros */
#define IBUS_TYPE_HOTKEY_PROFILE             \
    (ibus_hotkey_profile_get_type ())
#define IBUS_HOTKEY_PROFILE(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_HOTKEY_PROFILE, IBusHotkeyProfile))
#define IBUS_HOTKEY_PROFILE_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_HOTKEY_PROFILE, IBusHotkeyProfileClass))
#define IBUS_IS_HOTKEY_PROFILE(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_HOTKEY_PROFILE))
#define IBUS_IS_HOTKEY_PROFILE_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_HOTKEY_PROFILE))
#define IBUS_HOTKEY_PROFILE_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_HOTKEY_PROFILE, IBusHotkeyProfileClass))

G_BEGIN_DECLS

typedef struct _IBusHotkeyProfile IBusHotkeyProfile;
typedef struct _IBusHotkeyProfileClass IBusHotkeyProfileClass;

/**
 * IBusHotkeyProfile:
 *
 * An opaque data type representing an IBusHotkeyProfile.
 */
struct _IBusHotkeyProfile {
    IBusSerializable parent;

    /* members */
};

struct _IBusHotkeyProfileClass {
    IBusSerializableClass parent;

    void (* trigger) (IBusHotkeyProfile *profile,
                      GQuark             event,
                      gpointer           user_data);
};

/* hotkey profile functions */
GType            ibus_hotkey_profile_get_type   (void);
/**
 * ibus_hotkey_profile_new:
 * @returns: A newly allocated IBusHotkeyProfile.
 *
 * New an IBusHotkeyProfile.
 */
IBusHotkeyProfile
                *ibus_hotkey_profile_new        (void);

/**
 * ibus_hotkey_profile_add_hotkey :
 * @profile: An IBusHotkeyProfile.
 * @keyval: Keycode of the hotkey.
 * @modifiers: Modifiers of the hotkey.
 * @event: The event to be associated.
 * @returns: Always TRUE.
 *
 * Add a hotkey and its associated event to an IBusHotkeyProfile.
 */
gboolean         ibus_hotkey_profile_add_hotkey (IBusHotkeyProfile  *profile,
                                                 guint               keyval,
                                                 guint               modifiers,
                                                 GQuark              event);

/**
 * ibus_hotkey_profile_add_hotkey_from_string:
 * @profile: An IBusHotkeyProfile.
 * @str: Key in string representation.  '+' is the separator.
 * @event: The event to be associated.
 * @returns: FALSE if @str contains invalid symbol; TRUE otherwise.
 *
 * Add a hotkey and its associated event to an IBusHotkeyProfile.
 * The hotkey is in string format, such like <constant>Control+Shift+A</constant>.
 */
gboolean         ibus_hotkey_profile_add_hotkey_from_string
                                                (IBusHotkeyProfile  *profile,
                                                 const gchar        *str,
                                                 GQuark              event);

/**
 * ibus_hotkey_profile_remove_hotkey:
 * @profile: An IBusHotkeyProfile.
 * @keyval: Keycode of the hotkey.
 * @modifiers: Modifiers of the hotkey.
 * @returns: FALSE if the key is not in @profile, TRUE otherwise.
 *
 * Remove the hotkey for an IBusHotkeyProfile.
 */
gboolean         ibus_hotkey_profile_remove_hotkey
                                                (IBusHotkeyProfile  *profile,
                                                 guint               keyval,
                                                 guint               modifiers);

/**
 * ibus_hotkey_profile_remove_hotkey_by_event:
 * @profile: An IBusHotkeyProfile.
 * @event: The associated event.
 * @returns: FALSE if no such event in @profile, TRUE otherwise.
 *
 * Remove the hotkey for an IBusHotkeyProfile by event.
 */
gboolean         ibus_hotkey_profile_remove_hotkey_by_event
                                                (IBusHotkeyProfile  *profile,
                                                 GQuark              event);

/**
 * ibus_hotkey_profile_filter_key_event:
 * @profile: An IBusHotkeyProfile.
 * @keyval: Keycode of the hotkey.
 * @modifiers: Modifiers of the hotkey.
 * @prev_keyval: Keycode of the hotkey.
 * @prev_modifiers: Modifiers of the hotkey.
 * @user_data: user data for signal "trigger".
 * @returns: 0 if releasing a hotkey and the hotkey is not in the profile ; an associated event otherwise.
 *
 * Emit a <constant>::trigger</constant> signal when a hotkey is in a profile.
 *
 * @see_also: ::trigger
 */
GQuark           ibus_hotkey_profile_filter_key_event
                                                (IBusHotkeyProfile  *profile,
                                                 guint               keyval,
                                                 guint               modifiers,
                                                 guint               prev_keyval,
                                                 guint               prev_modifiers,
                                                 gpointer            user_data);

/**
 * ibus_hotkey_profile_lookup_hotkey:
 * @profile: An IBusHotkeyProfile.
 * @keyval: Keycode of the hotkey.
 * @modifiers: Modifiers of the hotkey.
 * @returns: The event associated to the hotkey or 0 if the hotkey is not in the
 * profile.
 */
GQuark           ibus_hotkey_profile_lookup_hotkey
                                                (IBusHotkeyProfile  *profile,
                                                 guint               keyval,
                                                 guint               modifiers);

G_END_DECLS
#endif
