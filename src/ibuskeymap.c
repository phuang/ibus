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
#include "ibuskeymap.h"

/* functions prototype */
static void         ibus_keymap_class_init      (IBusKeymapClass        *klass);
static void         ibus_keymap_init            (IBusKeymap             *keymap);
static void         ibus_keymap_destroy         (IBusKeymap             *keymap);

static IBusObjectClass *parent_class = NULL;

GType
ibus_keymap_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusKeymapClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_keymap_class_init,
        NULL,               /* class finialize */
        NULL,               /* class data */
        sizeof (IBusKeymap),
        0,
        (GInstanceInitFunc) ibus_keymap_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                                       "IBusKeymap",
                                       &type_info,
                                       0);
    }

    return type;
}

static void
ibus_keymap_class_init (IBusKeymapClass *klass)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    object_class->destroy = (IBusObjectDestroyFunc) ibus_keymap_destroy;

}

static void
ibus_keymap_init (IBusKeymap *keymap)
{
    memset (keymap->keymap, 0, sizeof (keymap->keymap));
}

static void
ibus_keymap_destroy (IBusKeymap *text)
{
    IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *)text);
}

IBusKeymap *
ibus_keymap_new (const gchar *name)
{
    IBusKeymap *keymap;

    keymap = g_object_new (IBUS_TYPE_KEYMAP, NULL);

    return keymap;
}

