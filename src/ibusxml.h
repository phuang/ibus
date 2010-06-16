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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     See the GNU
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

/**
 * SECTION: ibusxml
 * @short_description: XML handling functions for IBus.
 * @stability: Stable
 *
 * IBusXML lists data structure and handling function for XML in IBus.
 */
#ifndef __IBUS_XML_H__
#define __IBUS_XML_H__

#include <glib.h>

/**
 * XMLNode:
 * @name: Name of XML tag.
 * @text: Text enclosed by XML start tag and end tag. i.e. <tag>text</tag>.
 * @attributes: Attributes of the XML node.
 * @sub_nodes: Children node of this XML node.
 *
 * A data type representing an XML nod.
 */
typedef struct {
    gchar  *name;
    gchar  *text;
    gchar  **attributes;
    GList *sub_nodes;
} XMLNode;

/**
 * ibus_xml_parse_file:
 * @name: File name to be parsed.
 * @returns: Root node of parsed XML tree.
 *
 * Parse an XML file and return a corresponding XML tree.
 */
XMLNode *ibus_xml_parse_file    (const gchar    *name);

/**
 * ibus_xml_parse_buffer:
 * @buffer: Buffer to be parsed.
 * @returns: Root node of parsed XML tree.
 *
 * Parse a string buffer which contains an XML-formatted string,
 * and return a corresponding XML tree.
 */
XMLNode *ibus_xml_parse_buffer  (const gchar    *buffer);

/**
 * ibus_xml_free:
 * @node: Root node of an XML tree.
 *
 * Free an XML tree.
 */
void     ibus_xml_free          (XMLNode        *node);

/**
 * ibus_xml_output:
 * @node: Root node of an XML tree.
 * @output: GString which stores the output.
 *
 * Output an XML tree to a GString.
 */
void     ibus_xml_output        (const XMLNode  *node,
                                 GString        *output);
#endif
