/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#include "ibus.h"

int main()
{
	g_assert_cmpstr (ibus_keyval_name (IBUS_Home), ==, "Home");
	g_assert (ibus_keyval_from_name ("Home") == IBUS_Home);
	return 0;
}
