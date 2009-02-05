#include <Python.h>
#include <pygobject.h>

void pyibus_register_classes (PyObject *d);
void pyibus_add_constants (PyObject *module, const gchar *strip_prefix);
extern PyMethodDef pyibus_functions[];


DL_EXPORT(void)
initibus (void)
{
        PyObject *m, *d;

	init_pygobject ();

	m = Py_InitModule ("ibus", pyibus_functions);
        d = PyModule_GetDict (m);

        pyibus_register_classes (d);

        if (PyErr_Occurred ()) {
                Py_FatalError ("unable to initialise ibus module");
        }
	pyibus_add_constants (m, "GIK_");
        if (PyErr_Occurred ()) {
                Py_FatalError ("unable to initialise ibus module");
        }
}

