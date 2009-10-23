/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2008-2009 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2009 Red Hat, Inc.
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
#ifndef __IBUS_XML_H__
#define __IBUS_XML_H__

#include <glib.h>

typedef struct {
    gchar  *name;
    gchar  *text;
    gchar  **attributes;
    GList *sub_nodes;
} XMLNode;

XMLNode *ibus_xml_parse_file    (const gchar    *name);
XMLNode *ibus_xml_parse_buffer  (const gchar    *buffer);
void     ibus_xml_free          (XMLNode        *node);
void     ibus_xml_output        (const XMLNode  *node,
                                 GString        *output);
#endif
