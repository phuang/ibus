/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2008-2015 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2010-2016 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2008-2016 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#if !defined (__IBUS_H_INSIDE__) && !defined (IBUS_COMPILATION)
#error "Only <ibus.h> can be included directly"
#endif

#ifndef __IBUS_UTIL_H_
#define __IBUS_UTIL_H_

/**
 * SECTION: ibusutil
 * @short_description: Utilities with C-Language.
 * @stability: Unstable
 *
 * Utilized functions are available for miscellaneous purposes.
 */

/**
 * ibus_get_untranslated_language_name:
 * @_locale: A const locale name.
 *
 * Returns: untranslated language name
 */
const gchar *    ibus_get_untranslated_language_name
                                                (const gchar    *_locale);

/**
 * ibus_get_language_name:
 * @_locale: A const locale name.
 *
 * Returns: translated language name
 */
const gchar *    ibus_get_language_name         (const gchar    *_locale);

/**
 * ibus_emoji_dict_save:
 * @path: A path of the saved dictionary file.
 * @dict: (element-type utf8 gpointer) (transfer none): An Emoji dictionary
 *
 * Save the Emoji dictionary to the cache file.
 */
void             ibus_emoji_dict_save           (const gchar    *path,
                                                 GHashTable     *dict);
/**
 * ibus_emoji_dict_load:
 * @path: A path of the saved dictionary file.
 *
 * Returns: (element-type utf8 gpointer) (transfer none): An Emoji dictionary file loaded from the saved cache file.
 */
GHashTable *     ibus_emoji_dict_load           (const gchar    *path);

/**
 * ibus_emoji_dict_lookup:
 * @dict: (element-type utf8 gpointer) (transfer none): An Emoji dictionary
 * @annotation: Annotation for Emoji characters
 *
 * Returns: (element-type utf8) (transfer none): List of Emoji characters
 * This API is for gobject-introspection.
 */
GSList *         ibus_emoji_dict_lookup         (GHashTable     *dict,
                                                 const gchar    *annotation);
#endif
