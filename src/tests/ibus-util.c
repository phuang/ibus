/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <locale.h>

#include "ibus.h"

int main (int argc, char **argv)
{
    setlocale(LC_ALL, "C");

    g_assert_cmpstr (ibus_get_language_name ("eng"), ==, "English");

    return 0;
}
