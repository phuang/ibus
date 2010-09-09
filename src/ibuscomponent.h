/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
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
 * SECTION: ibuscomponent
 * @short_description:  Component (executable) specification.
 * @title: IBusComponent
 * @stability: Stable
 *
 * An IBusComponent is an executable program.
 * It provides services such as user interface, configuration,
 * and input method engine (IME).
 *
 * It is recommended that IME developers provide
 * a component XML file and
 * load the XML file by ibus_component_new_from_file().
 *
 * The format of a component XML file is described  at
 * <ulink url="http://code.google.com/p/ibus/wiki/DevXML">http://code.google.com/p/ibus/wiki/DevXML</ulink>
 */
#ifndef __IBUS_COMPONENT_H_
#define __IBUS_COMPONENT_H_

#include "ibusserializable.h"
#include "ibusobservedpath.h"
#include "ibusenginedesc.h"
#include "ibusxml.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_COMPONENT             \
    (ibus_component_get_type ())
#define IBUS_COMPONENT(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_COMPONENT, IBusComponent))
#define IBUS_COMPONENT_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_COMPONENT, IBusComponentClass))
#define IBUS_IS_COMPONENT(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_COMPONENT))
#define IBUS_IS_COMPONENT_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_COMPONENT))
#define IBUS_COMPONENT_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_COMPONENT, IBusComponentClass))

G_BEGIN_DECLS

typedef struct _IBusComponent IBusComponent;
typedef struct _IBusComponentClass IBusComponentClass;

/**
 * IBusComponent:
 * @name: Name of the component.
 * @description: Detailed description of component.
 * @version: Component version.
 * @license: Distribution license of this component.
 * @author: Author(s) of the component.
 * @homepage: Homepage of the component.
 * @exec: path to component executable.
 * @textdomain: Domain name for dgettext()
 *
 * An IBusComponent stores component information.
 */
struct _IBusComponent {
    IBusSerializable parent;
    /* instance members */

    /*< public >*/
    gchar *name;
    gchar *description;
    gchar *version;
    gchar *license;
    gchar *author;
    gchar *homepage;
    gchar *exec;

    /* text domain for dgettext */
    gchar *textdomain;

    /*< private >*/
    /* engines */
    GList *engines;

    /* observed paths */
    GList *observed_paths;

    GPid     pid;
    guint    child_source_id;

    /* padding */
    gpointer pdummy[5];  // We can add 5 pointers without breaking the ABI.
};

struct _IBusComponentClass {
  IBusSerializableClass parent;

  /* class members */
};

GType            ibus_component_get_type        (void);

/**
 * ibus_component_new:
 * @name: Name of the component.
 * @description: Detailed description of component.
 * @version: Component version.
 * @license: Distribution license of this component.
 * @author: Author(s) of the component.
 * @homepage: Homepage of the component.
 * @exec: path to component executable.
 * @textdomain: Domain name for dgettext()
 * @returns: A newly allocated IBusComponent.
 *
 * New an IBusComponent.
 */
IBusComponent   *ibus_component_new             (const gchar    *name,
                                                 const gchar    *description,
                                                 const gchar    *version,
                                                 const gchar    *license,
                                                 const gchar    *author,
                                                 const gchar    *homepage,
                                                 const gchar    *exec,
                                                 const gchar    *textdomain);

/**
 * ibus_component_new_from_xml_node:
 * @node: Root node of component XML tree.
 * @returns: A newly allocated IBusComponent.
 *
 * New an IBusComponent from an XML tree.
 */
IBusComponent   *ibus_component_new_from_xml_node
                                                (XMLNode        *node);

/**
 * ibus_component_new_from_file:
 * @filename: An XML file that contains component information.
 * @returns: A newly allocated IBusComponent.
 *
 * New an IBusComponent from an XML file.
 * Note that a component file usually contains engine descriptions,
 * if it does, ibus_engine_desc_new_from_xml_node() will be called
 * to load the engine descriptions.
 */
IBusComponent   *ibus_component_new_from_file   (const gchar    *filename);

/**
 * ibus_component_add_observed_path:
 * @component: An IBusComponent
 * @path: Observed path to be added.
 * @access_fs: TRUE for filling the file status; FALSE otherwise.
 *
 * Add an observed path to IBusComponent.
 */
void             ibus_component_add_observed_path
                                                (IBusComponent  *component,
                                                 const gchar    *path,
                                                 gboolean        access_fs);

/**
 * ibus_component_add_engine:
 * @component: An IBusComponent
 * @engine: A description of an engine.
 *
 * Add an engine to IBusComponent according to the description in @engine.
 */
void             ibus_component_add_engine      (IBusComponent  *component,
                                                 IBusEngineDesc *engine);

/**
 * ibus_component_get_engines:
 * @component: An IBusComponent.
 * @returns: (transfer none) (element-type IBusEngineDesc): A newly allocated GList that contains engines.
 *
 * Get the engines of this component.
 */
GList           *ibus_component_get_engines     (IBusComponent  *component);

/**
 * ibus_component_output:
 * @component: An IBusComponent.
 * @output: GString that holds the result.
 * @indent: level of indent.
 *
 * Output IBusComponent as an XML-formatted string.
 * The output string can be then shown on the screen or written to file.
 */
void             ibus_component_output          (IBusComponent  *component,
                                                 GString        *output,
                                                 gint            indent);

/**
 * ibus_component_output_engines:
 * @component: An IBusComponent.
 * @output: GString that holds the result.
 * @indent: level of indent.
 *
 * Output engine description  as an XML-formatted string.
 * The output string can be then shown on the screen or written to file.
 */
void             ibus_component_output_engines  (IBusComponent  *component,
                                                 GString        *output,
                                                 gint            indent);

/**
 * ibus_component_check_modification:
 * @component: An IBusComponent.
 * @returns: TRUE if at least one of the observed paths is modified; FALSE otherwise.
 *
 * Check whether the observed paths of component is modified.
 */
gboolean         ibus_component_check_modification
                                                (IBusComponent  *component);

/**
 * ibus_component_start:
 * @component: An IBusComponent.
 * @verbose: if FALSE, redirect the child output to /dev/null
 * @returns: TRUE if the component is started; FALSE otherwise.
 *
 * Whether the IBusComponent is started.
 */
gboolean         ibus_component_start           (IBusComponent  *component,
                                                 gboolean        verbose);

/**
 * ibus_component_stop:
 * @component: An IBusComponent.
 * @returns: TRUE if the component is stopped; FALSE otherwise.
 *
 * Whether the IBusComponent is stopped.
 */
gboolean         ibus_component_stop            (IBusComponent  *component);

/**
 * ibus_component_is_running:
 * @component: An IBusComponent.
 * @returns: TRUE if the component is running; FALSE otherwise.
 *
 * Whether the IBusComponent is running.
 */
gboolean         ibus_component_is_running      (IBusComponent  *component);

/**
 * ibus_component_get_from_engine:
 * @engine: A description of an engine.
 * @returns: (transfer none): An IBusComponent of the engine.
 *
 * Get the IBusComponent from an engine description.
 */
IBusComponent   *ibus_component_get_from_engine (IBusEngineDesc *engine);

/**
 * ibus_component_set_restart:
 * @component: An IBusComponent.
 * @restart: if TRUE, the component will be restartd when it dies.
 *
 * Set whether the component needs to be restarted when it dies.
 */
void             ibus_component_set_restart     (IBusComponent  *component,
                                                 gboolean        restart);


G_END_DECLS
#endif

