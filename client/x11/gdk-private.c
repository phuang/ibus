/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* ibus
 * Copyright (C) 2008 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008 Red Hat, Inc.
 *
 * gdk-private.c: Copied some code from gtk2
 *
 * This tool is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#ifdef HAVE_X11_XKBLIB_H
#  define HAVE_XKB
#  include <X11/XKBlib.h>
#endif

void
translate_key_event (GdkDisplay *display,
		     GdkEvent   *event,
		     XEvent     *xevent)
{
  GdkKeymap *keymap = gdk_keymap_get_for_display (display);
#if 0
  gunichar c = 0;
  gchar buf[7];
#endif

  event->key.type = xevent->xany.type == KeyPress ? GDK_KEY_PRESS : GDK_KEY_RELEASE;
  event->key.time = xevent->xkey.time;

  event->key.state = (GdkModifierType) xevent->xkey.state;

#ifdef HAVE_XKB
  event->key.group = XkbGroupForCoreState (xevent->xkey.state);
#else
  event->key.group = 0;
#endif

  event->key.hardware_keycode = xevent->xkey.keycode;

  event->key.keyval = GDK_VoidSymbol;

  gdk_keymap_translate_keyboard_state (keymap,
				       event->key.hardware_keycode,
				       event->key.state,
				       event->key.group,
				       &event->key.keyval,
				       NULL, NULL, NULL);
#if 0
  _gdk_keymap_add_virtual_modifiers (keymap, &event->key.state);
  event->key.is_modifier = _gdk_keymap_key_is_modifier (keymap, event->key.hardware_keycode);
#endif
  event->key.is_modifier = 0;

  /* Fill in event->string crudely, since various programs
   * depend on it.
   */
  event->key.string = NULL;
  event->key.length = 0;

  /* Don't need event->string.
   */
#if 0
  if (event->key.keyval != GDK_VoidSymbol)
    c = gdk_keyval_to_unicode (event->key.keyval);

  if (c)
    {
      gsize bytes_written;
      gint len;

      /* Apply the control key - Taken from Xlib
       */
      if (event->key.state & GDK_CONTROL_MASK)
	{
	  if ((c >= '@' && c < '\177') || c == ' ') c &= 0x1F;
	  else if (c == '2')
	    {
	      event->key.string = g_memdup ("\0\0", 2);
	      event->key.length = 1;
	      buf[0] = '\0';
	      goto out;
	    }
	  else if (c >= '3' && c <= '7') c -= ('3' - '\033');
	  else if (c == '8') c = '\177';
	  else if (c == '/') c = '_' & 0x1F;
	}

      len = g_unichar_to_utf8 (c, buf);
      buf[len] = '\0';

      event->key.string = g_locale_from_utf8 (buf, len,
					      NULL, &bytes_written,
					      NULL);
      if (event->key.string)
	event->key.length = bytes_written;
    }
  else if (event->key.keyval == GDK_Escape)
    {
      event->key.length = 1;
      event->key.string = g_strdup ("\033");
    }
  else if (event->key.keyval == GDK_Return ||
	  event->key.keyval == GDK_KP_Enter)
    {
      event->key.length = 1;
      event->key.string = g_strdup ("\r");
    }

  if (!event->key.string)
    {
      event->key.length = 0;
      event->key.string = g_strdup ("");
    }
 out:
#endif
  return;
}

