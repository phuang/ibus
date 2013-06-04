/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#include "registry.h"

int main()
{
#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init ();
#endif
	BusRegistry *registry = bus_registry_new ();
	g_object_unref (registry);
	return 0;
}
