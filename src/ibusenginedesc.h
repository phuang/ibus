/* vim:set et sts=4: */
/* bus - The Input Bus
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
#ifndef __ENGINE_DESC_H_
#define __ENGINE_DESC_H_

#include "ibusserializable.h"
#include "ibusxml.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_ENGINE_DESC             \
    (ibus_engine_desc_get_type ())
#define IBUS_ENGINE_DESC(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_ENGINE_DESC, IBusEngineDesc))
#define IBUS_ENGINE_DESC_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_ENGINE_DESC, IBusEngineDescClass))
#define IBUS_IS_ENGINE_DESC(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_ENGINE_DESC))
#define IBUS_IS_ENGINE_DESC_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_ENGINE_DESC))
#define IBUS_ENGINE_DESC_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_ENGINE_DESC, IBusEngineDescClass))

G_BEGIN_DECLS

typedef struct _IBusEngineDesc IBusEngineDesc;
typedef struct _IBusEngineDescClass IBusEngineDescClass;
typedef struct _BusComponent BusComponent;

struct _IBusEngineDesc {
    IBusSerializable parent;
    /* instance members */

    gchar *name;
    gchar *longname;
    gchar *description;
    gchar *language;
    gchar *license;
    gchar *author;
    gchar *icon;
    gchar *layout;
    guint   priority;
};

struct _IBusEngineDescClass {
    IBusSerializableClass parent;

    /* class members */
};

GType            ibus_engine_desc_get_type      (void);
IBusEngineDesc  *ibus_engine_desc_new           (const gchar    *name,
                                                 const gchar    *longname,
                                                 const gchar    *description,
                                                 const gchar    *language,
                                                 const gchar    *license,
                                                 const gchar    *author,
                                                 const gchar    *icon,
                                                 const gchar    *layout);
IBusEngineDesc  *ibus_engine_desc_new_from_xml_node
                                                (XMLNode        *node);
void             ibus_engine_desc_output        (IBusEngineDesc  *info,
                                                 GString        *output,
                                                 gint            indent);
G_END_DECLS
#endif

