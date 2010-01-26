#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "ibus.h"

#ifndef __linux__
/* the test is supported only on linux */
int main (int argc, char **argv)
{
    return 3;
}
#else

#include <linux/input.h>

#define KEYBOARDPATH "/dev/input/event4"

int main (int argc, char **argv)
{
    gint fd;
    struct input_event e;

    g_type_init ();

    IBusKeymap *keymap = ibus_keymap_get (argc > 1 ? argv[1] : "us");

    g_object_unref (keymap);
	return 0;

    fd = open (KEYBOARDPATH, O_RDONLY);


    while (fd >= 0 && read (fd, &e, sizeof (e)) == sizeof (e)) {
        if (e.type != EV_KEY)
            continue;
        if (e.value != 0)
            continue;

        g_debug ("=========================================================================");
        g_debug ("keycode = %d, %s %s %s %s %s", e.code,
                    ibus_keyval_name (ibus_keymap_lookup_keysym (keymap, e.code, 0)),
                    ibus_keyval_name (ibus_keymap_lookup_keysym (keymap, e.code, IBUS_SHIFT_MASK)),
                    ibus_keyval_name (ibus_keymap_lookup_keysym (keymap, e.code, IBUS_MOD5_MASK)),
                    ibus_keyval_name (ibus_keymap_lookup_keysym (keymap, e.code, IBUS_MOD5_MASK | IBUS_SHIFT_MASK)),
                    ibus_keyval_name (ibus_keymap_lookup_keysym (keymap, e.code, IBUS_MOD2_MASK))
        );
    }

    g_object_unref (keymap);
	return 0;

}
#endif
