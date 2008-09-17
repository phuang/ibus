/* -*- Mode: C; c-basic-offset: 4 -*- */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* include this first, before NO_IMPORT_PYGOBJECT is defined */
#include <pygobject.h>

#include "gconf-types.h"

void pygconf_register_classes (PyObject *d);
void pygconf_add_constants(PyObject *module, const gchar *strip_prefix);
		
extern PyMethodDef pygconf_functions[];
extern PyTypeObject PyGConfEngine_Type;

DL_EXPORT(void)
initgconf (void)
{
	PyObject *m, *d;

	init_pygobject ();

	m = Py_InitModule ("gconf", pygconf_functions);
	d = PyModule_GetDict (m);

	pygconf_register_classes (d);
	pygconf_add_constants (m, "GCONF_");
	pygconf_register_engine_type (m);

        PyModule_AddObject(m, "Engine", (PyObject *) &PyGConfEngine_Type);
}
