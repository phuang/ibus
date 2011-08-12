/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2011 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2011 Google, Inc.
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

#if !defined (__IBUS_H_INSIDE__) && !defined (IBUS_COMPILATION)
#error "Only <ibus.h> can be included directly"
#endif

#ifndef __IBUS_KEYS_H_
#define __IBUS_KEYS_H_

#include <glib.h>

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
 * ibus_unicode_to_keyval:
 * @wc: a ISO10646 encoded character
 * 
 * Convert from a ISO10646 character to a key symbol.
 * 
 * Return value: the corresponding IBus key symbol, if one exists.
 *               or, if there is no corresponding symbol,
 *               wc | 0x01000000
 **/
guint            ibus_unicode_to_keyval (gunichar        wc);

/**
 * ibus_keyval_to_unicode:
 * @keyval: an IBus key symbol
 * 
 * Convert from an IBus key symbol to the corresponding ISO10646 (Unicode)
 * character.
 * 
 * Return value: the corresponding unicode character, or 0 if there
 *               is no corresponding character.
 **/
gunichar         ibus_keyval_to_unicode (guint           keyval);

#endif // __IBUS_KEYS_H_
