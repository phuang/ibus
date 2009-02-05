/* vim:set et sts=4: */
/* IBus - The Input Bus
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

struct _IBusHotkeyProfile {
    IBusSerializable parent;

    /* members */
};

struct _IBusHotkeyProfileClass {
    IBusSerializableClass parent;

    void (* trigger) (IBusHotkeyProfile *profile,
                      GQuark             event);
};

/* hotkey profile functions */
GType            ibus_hotkey_profile_get_type   (void);
IBusHotkeyProfile
                *ibus_hotkey_profile_new        (void);
gboolean         ibus_hotkey_profile_add_hotkey (IBusHotkeyProfile  *profile,
                                                 guint               keyval,
                                                 guint               modifiers,
                                                 GQuark              event);
gboolean         ibus_hotkey_profile_add_hotkey_from_string
                                                (IBusHotkeyProfile  *profile,
                                                 const gchar        *str,
                                                 GQuark              event);
gboolean         ibus_hotkey_profile_remove_hotkey
                                                (IBusHotkeyProfile  *profile,
                                                 guint               keyval,
                                                 guint               modifiers);
gboolean         ibus_hotkey_profile_remove_hotkey_by_event
                                                (IBusHotkeyProfile  *profile,
                                                 GQuark              event);
GQuark           ibus_hotkey_profile_filter_key_event
                                                (IBusHotkeyProfile  *profile,
                                                 guint               keyval,
                                                 guint               modifiers,
                                                 gpointer            user_data);

G_END_DECLS
#endif

