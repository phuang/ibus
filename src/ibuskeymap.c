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
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ibusshare.h"
#include "ibuskeysyms.h"
#include "ibuskeymap.h"

/* functions prototype */
static void         ibus_keymap_class_init      (IBusKeymapClass        *klass);
static void         ibus_keymap_init            (IBusKeymap             *keymap);
static void         ibus_keymap_destroy         (IBusKeymap             *keymap);
static gboolean     ibus_keymap_load            (const gchar            *name,
                                                 KEYMAP                  keymap);
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

#define SKIP_SPACE(p)   \
    while (*p == ' ') p++;

static gboolean
ibus_keymap_parse_line (gchar  *str,
                        KEYMAP  keymap)
{
    gchar *p1, *p2;
    gint i;
    gint keycode, keysym;
    gchar keysym_name[256];

    const struct {
        const gchar *prefix;
        const gint len;
    } prefix [] = {
        { "keycode ", sizeof ("keycode ") - 1 },
        { "shift keycode ", sizeof ("shift keycode ") - 1 },
        { "numlock keycode ", sizeof ("numlock keycode ") - 1},
        { "altgr keycode ", sizeof ("altgr keycode ") - 1},
    };

    p1 = str;

    SKIP_SPACE(p1);

    if (*p1 == '#')
        return TRUE;

    if (strncmp (p1, "include ", sizeof ("include ") - 1) == 0) {
        p1 += sizeof ("include ") - 1;
        for (p2 = p1; *p2 != '\n'; p2++);
        *p2 = '\0';
        return ibus_keymap_load (p1, keymap);
    }

    for (i = 0; i < sizeof (prefix) / sizeof (prefix[0]); i++) {
        if (strncmp (p1, prefix[i].prefix, prefix[i].len) == 0) {
            p1 += prefix[i].len;
            break;
        }
    }

    if (i >= sizeof (prefix) / sizeof (prefix[0]))
        return FALSE;

    keycode = (guint) strtol (p1, &p2, 10);

    if (errno != 0)
        return FALSE;

    if (keycode < 0 || keycode > 255)
        return FALSE;

    p1 = p2;

    if (*p1++ != ' ')
        return FALSE;
    if (*p1++ != '=')
        return FALSE;
    if (*p1++ != ' ')
        return FALSE;

    for (p2 = p1; *p2 != '\n'; p2++);
    *p2 = '\0';

    keysym = ibus_keyval_from_name (p1);

    if (keysym == IBUS_VoidSymbol)
        return FALSE;

    keymap[keycode][i] = keysym;

    return TRUE;
}

static gboolean
ibus_keymap_load (const gchar *name,
                  KEYMAP       keymap)
{
    gchar *fname;
    FILE *pf;
    gchar buf[256];
    gint lineno;


    fname = g_build_filename (IBUS_DATA_DIR, "keymaps", name, NULL);

    if (fname == NULL) {
        return FALSE;
    }
    pf = g_fopen (fname, "r");
    g_free (fname);

    if (pf == NULL) {
        return FALSE;
    }

    lineno = 0;
    while (fgets (buf, sizeof (buf), pf) != NULL) {
        lineno ++;
        if (!ibus_keymap_parse_line (buf, keymap)) {
            g_warning ("parse %s failed on %d line", name, lineno);
            lineno = -1;
            break;
        }
    }

    fclose (pf);

    if (lineno == -1) {
        return FALSE;
    }

    return TRUE;
}

IBusKeymap *
ibus_keymap_new (const gchar *name)
{
    IBusKeymap *keymap;

    keymap = g_object_new (IBUS_TYPE_KEYMAP, NULL);

    if (!ibus_keymap_load (name, keymap->keymap)) {
        g_object_unref (keymap);
        keymap = NULL;
    }

    return keymap;
}

