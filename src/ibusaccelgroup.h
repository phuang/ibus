/* GTK - The GIMP Toolkit
 * Copyright (C) 1998, 2001 Tim Janik
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __IBUS_ACCEL_GROUP_H_
#define __IBUS_ACCEL_GROUP_H_


#if !defined (__IBUS_H_INSIDE__) && !defined (IBUS_COMPILATION)
#error "Only <ibus.h> can be included directly"
#endif

#include <glib.h>
#include <ibustypes.h>

G_BEGIN_DECLS


/* --- Accelerators--- */
gboolean ibus_accelerator_valid               (guint            keyval,
                                               IBusModifierType modifiers)
                                               G_GNUC_CONST;
void     ibus_accelerator_parse               (const gchar      *accelerator,
                                               guint            *accelerator_key,
                                               IBusModifierType *accelerator_mods);
gchar*   ibus_accelerator_name                (guint            accelerator_key,
                                               IBusModifierType accelerator_mods);

G_END_DECLS

#endif /* __IBUS_ACCEL_GROUP_H_ */
