#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ibus.h"

#define KEYBOARDPATH "/dev/input/event3"

int main()
{
    gint fd;
    struct input_event e;
    guint keyval;

    g_type_init ();

    IBusKeymap *keymap = ibus_keymap_new ("ja");

    fd = open (KEYBOARDPATH, O_RDONLY);


    while (fd >= 0 && read (fd, &e, sizeof (e)) == sizeof (e)) {
        if (e.type != EV_KEY)
            continue;
        if (e.value != 0)
            continue;

        g_debug ("=========================================================================");
        g_debug ("keycode = %d, %s %s %s %s %s", e.code,
                    ibus_keyval_name (ibus_keymap_get_keysym_for_keycode (keymap, e.code, 0)),
                    ibus_keyval_name (ibus_keymap_get_keysym_for_keycode (keymap, e.code, IBUS_SHIFT_MASK)),
                    ibus_keyval_name (ibus_keymap_get_keysym_for_keycode (keymap, e.code, IBUS_MOD5_MASK)),
                    ibus_keyval_name (ibus_keymap_get_keysym_for_keycode (keymap, e.code, IBUS_MOD5_MASK | IBUS_SHIFT_MASK)),
                    ibus_keyval_name (ibus_keymap_get_keysym_for_keycode (keymap, e.code, IBUS_MOD2_MASK))
        );
    }

    g_object_unref (keymap);
	return 0;

}
