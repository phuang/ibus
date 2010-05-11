/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2008-2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2010 Red Hat, Inc.
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
/**
 * SECTION: ibusenginedesc
 * @short_description:  Input method engine description data.
 * @title: IBusEngineDesc
 * @stability: Stable
 *
 * An IBusEngineDesc stores description data of IBusEngine.
 * The description data can either be passed to ibus_engine_desc_new(),
 * or loaded from an XML node through ibus_engine_desc_new_from_xml_node()
 * to construct IBusEngineDesc.
 *
 * However, the recommended way to load engine description data is
 * using ibus_component_new_from_file() to load a component file,
 * which also includes engine description data.
 *
 * @see_also: #IBusComponent, #IBusEngine
 *
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

/**
 * IBusEngineDesc:
 * @name: Name of the engine.
 * @longname: Long name of the input method engine.
 * @description: Input method engine description.
 * @language: Language (e.g. zh, jp) supported by this input method engine.
 * @license: License of the input method engine.
 * @author: Author of the input method engine.
 * @icon: Icon file of this engine.
 * @layout: Keyboard layout
 * @hotkeys: One or more hotkeys for switching to this engine, separated by
 *  semi-colon.
 * @rank: Preference rank among engines, the highest ranked IME will put in
 * the front.
 *
 * Input method engine description data.
 */
struct _IBusEngineDesc {
    IBusSerializable parent;
    /* instance members */

    /*< public >*/
    gchar *name;
    gchar *longname;
    gchar *description;
    gchar *language;
    gchar *license;
    gchar *author;
    gchar *icon;
    gchar *layout;
    gchar *hotkeys;
    guint  rank;
};

struct _IBusEngineDescClass {
    IBusSerializableClass parent;
    /* class members */
};

GType            ibus_engine_desc_get_type      (void);

/**
 * ibus_engine_desc_new:
 * @name: Name of the engine.
 * @longname: Long name of the input method engine.
 * @description: Input method engine description.
 * @language: Language (e.g. zh, jp) supported by this input method engine.
 * @license: License of the input method engine.
 * @author: Author of the input method engine.
 * @icon: Icon file of this engine.
 * @layout: Keyboard layout
 * @returns: A newly allocated IBusEngineDesc.
 *
 * New a IBusEngineDesc.
 */
IBusEngineDesc  *ibus_engine_desc_new           (const gchar    *name,
                                                 const gchar    *longname,
                                                 const gchar    *description,
                                                 const gchar    *language,
                                                 const gchar    *license,
                                                 const gchar    *author,
                                                 const gchar    *icon,
                                                 const gchar    *layout);

/**
 * ibus_engine_desc_new2:
 * @name: Name of the engine.
 * @longname: Long name of the input method engine.
 * @description: Input method engine description.
 * @language: Language (e.g. zh, jp) supported by this input method engine.
 * @license: License of the input method engine.
 * @author: Author of the input method engine.
 * @icon: Icon file of this engine.
 * @layout: Keyboard layout
 * @hotkeys: Hotkeys for switching to this engine.
 * @returns: A newly allocated IBusEngineDesc.
 *
 * New a IBusEngineDesc.
 */
IBusEngineDesc  *ibus_engine_desc_new2          (const gchar    *name,
                                                 const gchar    *longname,
                                                 const gchar    *description,
                                                 const gchar    *language,
                                                 const gchar    *license,
                                                 const gchar    *author,
                                                 const gchar    *icon,
                                                 const gchar    *layout,
                                                 const gchar    *hotkeys);

/**
 * ibus_engine_desc_new_from_xml_node:
 * @node: An XML node
 * @returns: A newly allocated IBusEngineDesc that contains description from
 * @node.
 *
 * New a IBusEngineDesc from an XML node.
 * <note><para>This function is called by ibus_component_new_from_file(),
 *  so developers normally do not need to call it directly.
 * </para></note>
 */
IBusEngineDesc  *ibus_engine_desc_new_from_xml_node
                                                (XMLNode        *node);
/**
 * ibus_engine_desc_output:
 * @info: An IBusEngineDesc
 * @output: XML-formatted Input method engine description.
 * @indent: Number of indent (showed as 4 spaces).
 *
 * Output XML-formatted input method engine description.
 * The result will be append to GString specified in @output.
 */
void             ibus_engine_desc_output        (IBusEngineDesc *info,
                                                 GString        *output,
                                                 gint            indent);
G_END_DECLS
#endif
