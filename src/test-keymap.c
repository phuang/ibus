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

    while (read (fd, &e, sizeof (e)) == sizeof (e)) {
        if (e.type != EV_KEY)
            continue;
        if (e.value != 0)
            continue;

        g_debug ("=========================== %d =============================", e.code);
        keyval = ibus_keymap_get_keysym_for_keycode (keymap, e.code, 0);
        g_debug ("keycode = %d, keysym = %s (%d)", e.code, ibus_keyval_name (keyval), keyval);

        keyval = ibus_keymap_get_keysym_for_keycode (keymap, e.code, IBUS_SHIFT_MASK);
        g_debug ("shift keycode = %d, keysym = %s (%d)", e.code, ibus_keyval_name (keyval), keyval);

        keyval = ibus_keymap_get_keysym_for_keycode (keymap, e.code, IBUS_MOD2_MASK);
        g_debug ("numlock keycode = %d, keysym = %s (%d)", e.code, ibus_keyval_name (keyval), keyval);

        keyval = ibus_keymap_get_keysym_for_keycode (keymap, e.code, IBUS_SHIFT_MASK | IBUS_LOCK_MASK);
        g_debug ("lock & shift keycode = %d, keysym = %s (%d)", e.code, ibus_keyval_name (keyval), keyval);

        keyval = ibus_keymap_get_keysym_for_keycode (keymap, e.code, IBUS_LOCK_MASK);
        g_debug ("lock keycode = %d, keysym = %s (%d)", e.code, ibus_keyval_name (keyval), keyval);

    }

    g_object_unref (keymap);
	return 0;

}
