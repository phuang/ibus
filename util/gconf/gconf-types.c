/*
 * Copyright (C) 2005 Red Hat, Inc.
 *
 *   gconf-types.c: wrappers for some specialised GConf types.
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
 */

#include "gconf-types.h"
#define NO_IMPORT_PYGOBJECT
#include <pygobject.h>

typedef struct {
  PyObject_HEAD
  GConfEngine *engine;
} PyGConfEngine;
extern PyTypeObject PyGConfEngine_Type;

static void
pygconf_engine_dealloc (PyGConfEngine *self)
{
  pyg_begin_allow_threads;
  gconf_engine_unref (self->engine);
  pyg_end_allow_threads;
  PyObject_DEL (self);
}


static PyObject *
pygconf_engine_associate_schema(PyGConfEngine *self, PyObject *args, PyObject *kwargs)
{
	gchar *key, *schema_key;
	gboolean result;
        GError *err = NULL;
	char *kwlist[] = {"key", "schema_key", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                         "ss:gconf.Engine.associate_schema",
                                         kwlist, &key, &schema_key))
            return NULL;

        result = gconf_engine_associate_schema(self->engine, key, schema_key, &err);

        if (pyg_error_check(&err))
            return NULL;
       
        if (result) {
            Py_INCREF(Py_True);
            return Py_True;
        } else {
            Py_INCREF(Py_False);
            return Py_False;
        }
}

static PyMethodDef pygconf_engine_methods[] = {
        {"associate_schema", (PyCFunction)pygconf_engine_associate_schema,
         METH_KEYWORDS, NULL},

        {NULL, NULL, 0, NULL}
};


PyTypeObject PyGConfEngine_Type = {
  PyObject_HEAD_INIT (NULL)
  0,
  "gconf.GConfEngine",
  sizeof (PyGConfEngine),
  0,
  (destructor) pygconf_engine_dealloc,
  (printfunc) 0,
  (getattrfunc) 0,
  (setattrfunc) 0,
  (cmpfunc) 0,
  (reprfunc) 0,
  0,
  0,
  0,
  (hashfunc) 0,
  (ternaryfunc) 0,
  (reprfunc) 0,
  (getattrofunc) 0,
  (setattrofunc) 0,
  0,
  Py_TPFLAGS_DEFAULT,
  0,                           /* tp_doc */
  0,		               /* tp_traverse */
  0,		               /* tp_clear */
  0,		               /* tp_richcompare */
  0,		               /* tp_weaklistoffset */
  0,		               /* tp_iter */
  0,		               /* tp_iternext */
  pygconf_engine_methods,      /* tp_methods */
};

PyObject *
pygconf_engine_new (GConfEngine *engine)
{
  PyGConfEngine *self;

  if (engine == NULL)
    {
      Py_INCREF (Py_None);
      return Py_None;
    }

  self = (PyGConfEngine *) PyObject_NEW (PyGConfEngine,	&PyGConfEngine_Type);
  if (self == NULL)
    return NULL;

  pyg_begin_allow_threads;
  self->engine = engine;
  gconf_engine_ref (engine);
  pyg_end_allow_threads;

  return (PyObject *) self;
}

GConfEngine *
pygconf_engine_from_pyobject (PyObject *object)
{
  PyGConfEngine *self;

  if (object == NULL)
    return NULL;

  if (!PyObject_TypeCheck (object, &PyGConfEngine_Type))
    {
      PyErr_SetString (PyExc_TypeError, "unable to convert argument to GConfEngine*");
      return NULL;
    }

  self = (PyGConfEngine *) object;

  return self->engine;
}

void
pygconf_register_engine_type (PyObject *moddict)
{
  PyGConfEngine_Type.ob_type = &PyType_Type;

  PyType_Ready(&PyGConfEngine_Type);
}
