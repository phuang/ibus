/* vim:set et sts=4 sw=4:
valac --pkg gtk+-2.0 --pkg x11 --pkg gdk-x11-2.0 --pkg gee-1.0 keybinding-manager.vala
*/

/**
 * This class is in charge to grab keybindings on the X11 display
 * and filter X11-events and passing on such events to the registed
 * handler methods.
 *
 * @author Oliver Sauder <os@esite.ch>
 */

using Gdk;
using GLib;
using Gtk;
using X;

class KeybindingManager : GLib.Object {
    /**
     * list of binded keybindings
     */
    private GLib.List<Keybinding> bindings = new GLib.List<Keybinding>();

    /**
     * locked modifiers used to grab all keys whatever lock key
     * is pressed.
     */
    private static uint[] lock_modifiers = {
        0,
        Gdk.ModifierType.MOD2_MASK, // NUM_LOCK
        Gdk.ModifierType.LOCK_MASK, // CAPS_LOCK
        Gdk.ModifierType.MOD5_MASK, // SCROLL_LOCK
        Gdk.ModifierType.MOD2_MASK|Gdk.ModifierType.LOCK_MASK,
        Gdk.ModifierType.MOD2_MASK|Gdk.ModifierType.MOD5_MASK,
        Gdk.ModifierType.LOCK_MASK|Gdk.ModifierType.MOD5_MASK,
        Gdk.ModifierType.MOD2_MASK|Gdk.ModifierType.LOCK_MASK|Gdk.ModifierType.MOD5_MASK
    };

    private uint get_primary_modifier (uint binding_mask) {
        const uint[] masks = {
            Gdk.ModifierType.MOD5_MASK,
            Gdk.ModifierType.MOD4_MASK,
            Gdk.ModifierType.MOD3_MASK,
            Gdk.ModifierType.MOD2_MASK,
            Gdk.ModifierType.MOD1_MASK,
            Gdk.ModifierType.CONTROL_MASK,
            Gdk.ModifierType.SHIFT_MASK,
            Gdk.ModifierType.LOCK_MASK
        };
        foreach (var mask in masks) {
            if ((binding_mask & mask) != 0)
                return mask;
        }
        return 0;
    }

    /**
     * Helper class to store keybinding
     */
    private class Keybinding {
        public Keybinding(string accelerator, uint keysym,
            Gdk.ModifierType modifiers, KeybindingHandlerFunc handler) {
            this.accelerator = accelerator;
            this.keysym = keysym;
            this.modifiers = modifiers;
            this.handler = handler;
        }

        public string accelerator { get; set; }
        public uint keysym { get; set; }
        public Gdk.ModifierType modifiers { get; set; }
        public unowned KeybindingHandlerFunc handler { get; set; }
    }

    /**
     * Keybinding func needed to bind key to handler
     *
     * @param event passing on gdk event
     */
    public delegate void KeybindingHandlerFunc(Gdk.Event event);

    public KeybindingManager() {
        // init filter to retrieve X.Events
        Gdk.Window rootwin = Gdk.get_default_root_window();
        if(rootwin != null) {
            // rootwin.add_filter(event_filter);
        }

        Gdk.Event.handler_set(event_handler);
    }

    /**
     * Bind accelerator to given handler
     *
     * @param accelerator accelerator parsable by Gtk.accelerator_parse
     * @param handler handler called when given accelerator is pressed
     */
    public bool bind(string accelerator,
                     KeybindingHandlerFunc handler) {
        debug("Binding key " + accelerator);

        // convert accelerator
        uint keysym;
        Gdk.ModifierType modifiers;
        Gtk.accelerator_parse(accelerator, out keysym, out modifiers);

        get_primary_modifier(modifiers);

        unowned X.Display display = Gdk.x11_get_default_xdisplay();

        int keycode = display.keysym_to_keycode(keysym);

        if (keycode == 0)
            return false;

        // trap XErrors to avoid closing of application
        // even when grabing of key fails
        Gdk.error_trap_push();

        // grab key finally
        // also grab all keys which are combined with a lock key such NumLock
        foreach(uint lock_modifier in lock_modifiers) {
            display.grab_key(keycode, modifiers | lock_modifier,
                             Gdk.x11_get_default_root_xwindow(), false,
                             X.GrabMode.Async, X.GrabMode.Async);
        }

        if (Gdk.error_trap_pop() != 0) {
            debug("grab key failed");
        }
        // wait until all X request have been processed
        Gdk.flush();

        // store binding
        Keybinding binding = new Keybinding(accelerator, keysym, modifiers, handler);
        bindings.append(binding);

        debug("Successfully binded key " + accelerator);
        return true;
    }

    /**
     * Unbind given accelerator.
     *
     * @param accelerator accelerator parsable by Gtk.accelerator_parse
     */
    public void unbind(string accelerator) {
        debug("Unbinding key " + accelerator);

        unowned X.Display display = Gdk.x11_get_default_xdisplay();

        // unbind all keys with given accelerator
        GLib.List<Keybinding> remove_bindings = new GLib.List<Keybinding>();
        foreach(Keybinding binding in bindings) {
            if(str_equal(accelerator, binding.accelerator)) {
                int keycode = display.keysym_to_keycode(binding.keysym);
                foreach(uint lock_modifier in lock_modifiers) {
                    display.ungrab_key(keycode,
                                       binding.modifiers | lock_modifier,
                                       Gdk.x11_get_default_root_xwindow());
                }
                remove_bindings.append(binding);
            }
        }

        // remove unbinded keys
        foreach(Keybinding binding in remove_bindings)
            bindings.remove(binding);
    }

    public void event_handler(Gdk.Event event) {
        debug("event_handler");
        do {
            if (event.any.window != Gdk.get_default_root_window()) {
                debug("is not root window");
                break;
            }

            debug("is root window");
            if (event.type == Gdk.EventType.KEY_PRESS) {
                uint modifiers = event.key.state & ~(lock_modifiers[7]);
                foreach (var binding in bindings) {
                    if (event.key.keyval != binding.keysym ||
                        modifiers != binding.modifiers)
                        continue;
                    binding.handler(event);
                    return;
                }
            }
        } while (false);
        Gtk.main_do_event(event);
    }

    /**
     * Event filter method needed to fetch X.Events
     */
    /* 
    public Gdk.FilterReturn event_filter(Gdk.XEvent gdk_xevent, Gdk.Event gdk_event) {
        Gdk.FilterReturn filter_return = Gdk.FilterReturn.CONTINUE;

        void* pointer = &gdk_xevent;
        X.Event* xevent = (X.Event*) pointer;

         if(xevent->type == X.EventType.KeyPress) {
            foreach(Keybinding binding in bindings) {
                // remove NumLock, CapsLock and ScrollLock from key state
                uint event_mods = xevent.xkey.state & ~ (lock_modifiers[7]);
                if(xevent->xkey.keycode == binding.keycode && event_mods == binding.modifiers) {
                    // call all handlers with pressed key and modifiers
                    binding.handler(gdk_event);
                }
            }
         }

        return filter_return;
    }
    */
}

/*
public static int main (string[] args)
{
    Gtk.init (ref args);

    KeybindingManager manager = new KeybindingManager();
    manager.bind("<Ctrl><Alt>V", test);

    Gtk.main ();
    return 0;
}

private static void test()
{
    debug("hotkey pressed");
}
*/
