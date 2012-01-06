/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "ibus.h"

int main (int argc, char **argv)
{
    g_debug ("%s=%s", "eng", ibus_get_language_name ("eng"));
    return 0;
}
