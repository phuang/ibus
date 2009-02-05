/* vim:set et sts=4: */
/* ibus - The Input IBus
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
#ifndef __IBUS_OBSERVED_PATH_H_
#define __IBUS_OBSERVED_PATH_H_

#include "ibusserializable.h"
#include "ibusxml.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_OBSERVED_PATH             \
    (ibus_observed_path_get_type ())
#define IBUS_OBSERVED_PATH(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_OBSERVED_PATH, IBusObservedPath))
#define IBUS_OBSERVED_PATH_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_OBSERVED_PATH, IBusObservedPathClass))
#define IBUS_IS_OBSERVED_PATH(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_OBSERVED_PATH))
#define IBUS_IS_OBSERVED_PATH_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_OBSERVED_PATH))
#define IBUS_OBSERVED_PATH_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_OBSERVED_PATH, IBusObservedPathClass))

G_BEGIN_DECLS

typedef struct _IBusObservedPath IBusObservedPath;
typedef struct _IBusObservedPathClass IBusObservedPathClass;

struct _IBusObservedPath {
    IBusSerializable parent;
    /* instance members */

    gchar *path;
    glong mtime;
    gboolean is_dir;
    gboolean is_exist;

};

struct _IBusObservedPathClass {
    IBusSerializableClass parent;

    /* class members */
};

GType                ibus_observed_path_get_type            (void);
IBusObservedPath    *ibus_observed_path_new_from_xml_node   (XMLNode            *node,
                                                             gboolean            fill_stat);
IBusObservedPath    *ibus_observed_path_new                 (const gchar        *path,
                                                             gboolean            fill_stat);
GList               *ibus_observed_path_traverse            (IBusObservedPath   *path);
gboolean             ibus_observed_path_check_modification  (IBusObservedPath   *path);
void                 ibus_observed_path_output              (IBusObservedPath   *path,
                                                             GString            *output,
                                                             gint                indent);

G_END_DECLS
#endif

