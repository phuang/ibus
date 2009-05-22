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
/**
 * SECTION: ibuskeymap
 * @short_description: Keymap with decorating information.
 *
 */

#ifndef __IBUS_KEYMAP_H_
#define __IBUS_KEYMAP_H_

#include "ibusobject.h"

/*
 * Type macros.
 */
/* define IBusKeymap macros */
#define IBUS_TYPE_KEYMAP             \
    (ibus_keymap_get_type ())
#define IBUS_KEYMAP(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_KEYMAP, IBusKeymap))
#define IBUS_KEYMAP_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_KEYMAP, IBusKeymapClass))
#define IBUS_IS_KEYMAP(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_KEYMAP))
#define IBUS_IS_KEYMAP_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_KEYMAP))
#define IBUS_KEYMAP_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_KEYMAP, IBusKeymapClass))

G_BEGIN_DECLS

typedef struct _IBusKeymap IBusKeymap;
typedef struct _IBusKeymapClass IBusKeymapClass;
typedef guint KEYMAP[256][5];

/**
 * IBusKeymap:
 * A keymap object in IBus.
 */
struct _IBusKeymap {
    IBusObject parent;

    /* members */
    /*< public >*/
    KEYMAP keymap;
};

struct _IBusKeymapClass {
    IBusObjectClass parent;
};

GType            ibus_keymap_get_type               (void);

/**
 * ibus_keymap_new:
 * @name: keymap name.
 * @returns: A newly allocated IBusKeymap.
 *
 * New an IBusKeymap.
 */
IBusKeymap        *ibus_keymap_new                  (const gchar        *name);
guint              ibus_keymap_get_keysym_for_keycode
                                                    (IBusKeymap         *keymap,
                                                     guint16             keycode,
                                                     guint32             state);

G_END_DECLS
#endif

