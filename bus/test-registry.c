/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#include "registry.h"

int main()
{
	g_type_init ();
	BusRegistry *registry = bus_registry_new ();
	g_object_unref (registry);
	return 0;
}
