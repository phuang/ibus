/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* GDK - The GIMP Drawing Kit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

/* Key handling not part of the keymap */
#include <glib.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>
#include "ibuskeysyms.h"
#include "keyname-table.h"

#define IBUS_NUM_KEYS G_N_ELEMENTS (gdk_keys_by_keyval)

static int
gdk_keys_keyval_compare (const void *pkey, const void *pbase)
{
  return (*(int *) pkey) - ((gdk_key *) pbase)->keyval;
}

const gchar*
ibus_keyval_name (guint keyval)
{
  static gchar buf[100];
  gdk_key *found;

  /* Check for directly encoded 24-bit UCS characters: */
  if ((keyval & 0xff000000) == 0x01000000)
    {
      g_sprintf (buf, "U+%.04X", (keyval & 0x00ffffff));
      return buf;
    }

  found = bsearch (&keyval, gdk_keys_by_keyval,
                   IBUS_NUM_KEYS, sizeof (gdk_key),
           gdk_keys_keyval_compare);

  if (found != NULL)
    {
      while ((found > gdk_keys_by_keyval) &&
             ((found - 1)->keyval == keyval))
        found--;

      return (gchar *) (keynames + found->offset);
    }
  else if (keyval != 0)
    {
      g_sprintf (buf, "%#x", keyval);
      return buf;
    }

  return NULL;
}

static int
gdk_keys_name_compare (const void *pkey, const void *pbase)
{
  return strcmp ((const char *) pkey,
         (const char *) (keynames + ((const gdk_key *) pbase)->offset));
}

guint
ibus_keyval_from_name (const gchar *keyval_name)
{
  gdk_key *found;

  g_return_val_if_fail (keyval_name != NULL, 0);

  found = bsearch (keyval_name, gdk_keys_by_name,
           IBUS_NUM_KEYS, sizeof (gdk_key),
           gdk_keys_name_compare);
  if (found != NULL)
    return found->keyval;
  else
    return IBUS_KEY_VoidSymbol;
}

static const gchar *
modifier_name[] = {
    "Shift",       // 0
    "Lock",        // 1
    "Control",    // 2
    "Alt",        // 3
    "Mod2",        // 4
    "Mod3",        // 5
    "Mod4",        // 6
    "Mod5",        // 7
    "Button1",    // 8
    "Button2",    // 9
    "Button3",    // 10
    "Button4",    // 11
    "Button5",    // 12
    NULL, NULL, NULL, NULL, NULL, // 13 - 17
    NULL, NULL, NULL, NULL, NULL, // 18 - 22
    NULL, NULL, NULL, // 23 - 25
    "Super",    // 26
    "Hyper",    // 27
    "Meta",        // 28
    NULL,        // 29
    "Release",    // 30
    NULL,        // 31
};

const gchar *
ibus_key_event_to_string (guint keyval,
                          guint modifiers)
{
    guint i;
    GString *str;
    const gchar *keyval_name;

    g_return_val_if_fail (keyval != IBUS_KEY_VoidSymbol, NULL);

    keyval_name = ibus_keyval_name (keyval);
    g_return_val_if_fail (keyval_name != NULL, NULL);

    str = g_string_new ("");

    for (i = 0; i < 32; i++) {
        guint mask = 1 << i;

        if ((modifiers & mask) == 0)
            continue;
        if (modifier_name[i] == NULL)
            continue;

        g_string_append (str, modifier_name[i]);
        g_string_append_c (str, '+');
    }

    g_string_append (str, keyval_name);

    return g_string_free (str, FALSE);
}

gboolean
ibus_key_event_from_string (const gchar *string,
                            guint       *keyval,
                            guint       *modifiers)
{
    g_return_val_if_fail (string != NULL, FALSE);
    g_return_val_if_fail (keyval != NULL, FALSE);
    g_return_val_if_fail (modifiers != NULL, FALSE);

    gchar **tokens = NULL;
    gchar **p;
    gboolean retval = FALSE;

    tokens = g_strsplit (string, "+", 0);
    g_return_val_if_fail (tokens != NULL, FALSE);

    *keyval = 0;
    *modifiers = 0;

    for (p = tokens; *(p + 1) != NULL; p++) {
        gint i;
        for (i = 0; i < 32; i++) {
            if (g_strcmp0 (modifier_name[i], *p) != 0)
                continue;
            *modifiers |= (1 << i);
            break;
        }
        if (i == 32) {
            goto _out;
        }
    }

    *keyval = ibus_keyval_from_name (*p);
    if (*keyval != IBUS_KEY_VoidSymbol)
        retval = TRUE;
_out:
    if (tokens)
        g_strfreev (tokens);
    return retval;
}

