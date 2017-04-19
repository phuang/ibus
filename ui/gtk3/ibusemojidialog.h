/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2017 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2017 Red Hat, Inc.
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

#ifndef __IBUS_EMOJI_DIALOG_H_
#define __IBUS_EMOJI_DIALOG_H_

#include <gtk/gtk.h>

/**
 * SECTION: ibusemojidialog
 * @short_description: emoji dialog utility.
 * @stability: Unstable
 *
 * miscellaneous emoji dialg APIs.
 */

G_BEGIN_DECLS

typedef struct _IBusEmojier IBusEmojier;
typedef struct _IBusEmojierPrivate IBusEmojierPrivate;
typedef struct _IBusEmojierClass IBusEmojierClass;

struct _IBusEmojier {
    /*< private >*/
    GtkWindow parent_instance;
    IBusEmojierPrivate * priv;
    /* instance members */
    /*< public >*/
};

struct _IBusEmojierClass {
    /*< private >*/
    GtkWindowClass parent_class;
    /* class members */
    /*< public >*/
    /* signals */
};

GType           ibus_emojier_get_type        (void);

/**
 * ibus_emojier_new:
 *
 * Creates a new #IBusEmojier.
 *
 * Returns: A newly allocated #IBusEmojiier.
 */
IBusEmojier * ibus_emojier_new                    (void);

/**
 * ibus_emojier_run:
 * @self: An #IBusEmojier
 * @input_context_path: An input context path of #IBusInputContext
 *                      of the focused application.
 *
 * Runs emoji dialog to select emoji.
 *
 * Returns: A selected emoji character.
 */
gchar *       ibus_emojier_run                    (IBusEmojier* self,
                                                   const gchar*
                                                           input_context_path);

/**
 * ibus_emojier_is_running:
 * @self: An #IBusEmojier
 *
 * Returns: boolean if the emoji dialog is running
 */
gboolean      ibus_emojier_is_running             (IBusEmojier* self);

/**
 * ibus_emojier_get_input_context_path:
 * @self: An #IBusEmojier
 *
 * Returns: an input context path of #IBusInputContext
 * which is saved in ibus_emojier_run().
 */
gchar *       ibus_emojier_get_input_context_path (IBusEmojier* self);

/**
 * ibus_emojier_get_selected_string:
 * @self: An #IBusEmojier
 *
 * Returns: an selected emoji character on the emoji dialog.
 */
gchar *       ibus_emojier_get_selected_string    (IBusEmojier* self);

/**
 * ibus_emojier_reset:
 * @self: An #IBusEmojier
 *
 * Reset the selected string and input context path.
 */
void          ibus_emojier_reset                  (IBusEmojier* self);

/**
 * ibus_emojier_present_centralize:
 * @self: An #IBusEmojier
 *
 * Move the window to the toplevel on the screen and centralize it.
 */
void          ibus_emojier_present_centralize     (IBusEmojier* self);

/**
 * ibus_emojier_has_loaded_emoji_dict:
 *
 * Returns: %TRUE if the emoji dict is loaded, otherwise %FALSE.
 */
gboolean      ibus_emojier_has_loaded_emoji_dict  (void);

/**
 * ibus_emojier_set_annotation_lang:
 * @lang: A langauge id for emoji annotations.
 *
 * Set a language id for emoji annotations. #IBusEmojier will load
 * $PKGDATADIR/dicts/emoji-@lang.dict. The default is "en".
 */
void          ibus_emojier_set_annotation_lang    (const gchar* lang);

/**
 * ibus_emojier_set_emoji_font:
 * @emoji_font: font name for emoji characters
 *
 * Set emoji font on the emoji dialog
 */
void          ibus_emojier_set_emoji_font         (const gchar* emoji_font);

#if 0
/* TODO: set customized annotations */
/**
 * ibus_emojier_set_favorites:
 * @favorites: (array length=favorites_length): A custom emoji list.
 * @favorites_length: A length of @favorites
 *
 * Set emoji font on the emoji dialog
 */
void          ibus_emojier_set_favorites          (gchar**      favorites,
                                                   int
                                                             favorites_length);
#endif
G_END_DECLS
#endif
