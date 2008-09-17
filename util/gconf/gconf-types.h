/*
 * Copyright (C) 2005 Red Hat, Inc.
 *
 *   gconf-types.h: wrappers for some specialised GConf types.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 * Author:
 *     Mark McLoughlin <mark@skynet.ie>
 */

#ifndef __GCONF_TYPES_H__
#define __GCONF_TYPES_H__

#include <Python.h>
#include <gconf/gconf.h>

void         pygconf_register_engine_type (PyObject    *moddict);
PyObject    *pygconf_engine_new           (GConfEngine *engine);
GConfEngine *pygconf_engine_from_pyobject (PyObject    *object);
     
#endif /* __GCONF_TYPES_H__ */
