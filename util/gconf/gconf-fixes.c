/* GConf-python
 * Copyright (C) 2002 Johan Dahlin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * Author: Johan Dahlin <jdahlin@telia.com>
 */

#include <gconf/gconf-client.h>
#include <gconf/gconf-value.h>

#include "gconf-fixes.h"

GConfMetaInfo*
gconf_meta_info_copy (const GConfMetaInfo *src)
{
	GConfMetaInfo *info;
	
	info = gconf_meta_info_new ();
	 
	info->schema = g_strdup (src->schema);
	info->mod_user = g_strdup (src->mod_user);
	info->mod_time = src->mod_time;
	
	return info;
}

#define BOILERPLATE_TYPE_BOXED(func,name)                                         \
GType                                                                             \
py##func##_get_type (void)                                                        \
{                                                                                 \
	static GType type = 0;                                                    \
	if (type == 0) {                                                          \
		type = g_boxed_type_register_static(name,                         \
                                                    (GBoxedCopyFunc)func##_copy,  \
						    (GBoxedFreeFunc)func##_free); \
	}                                                                         \
	return type;                                                              \
}

BOILERPLATE_TYPE_BOXED(gconf_value,     "GConfValue")
BOILERPLATE_TYPE_BOXED(gconf_entry,     "GConfEntry")
BOILERPLATE_TYPE_BOXED(gconf_schema,    "GConfSchema")
BOILERPLATE_TYPE_BOXED(gconf_meta_info, "GConfMetaInfo")
