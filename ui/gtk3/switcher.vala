/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright(c) 2011 Peng Huang <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or(at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

using IBus;
using GLib;
using Gtk;

class Switcher : Gtk.Window {
    private Gtk.Box m_box;
    private Gtk.Button[] m_buttons = {};
    private IBus.EngineDesc[] m_engines;
    private uint m_selected_engine;
    private uint m_primary_modifier;
    private GLib.MainLoop m_loop;
    private int m_result;

    public Switcher() {
        GLib.Object(
            type : Gtk.WindowType.POPUP,
            events : Gdk.EventMask.KEY_PRESS_MASK | Gdk.EventMask.KEY_RELEASE_MASK,
            window_position : Gtk.WindowPosition.CENTER,
            accept_focus : true,
            decorated : false,
            modal : true,
            focus_visible : true
        );
        m_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
        add(m_box);

        grab_focus();
    }

    public int run(Gdk.Event event, IBus.EngineDesc[] engines, int index) {
        assert (m_loop == null);
        assert (index < engines.length);

        m_primary_modifier =
            KeybindingManager.get_primary_modifier(
                event.key.state & KeybindingManager.MODIFIER_FILTER);

        update_engines(engines);
        m_selected_engine = index;
        m_buttons[index].grab_focus();

        Gdk.Device device = event.get_device();
        if (device == null) {
            var display = get_display();
            var device_manager = display.get_device_manager();
/* The macro VALA_X_Y supports even numbers.
 * http://git.gnome.org/browse/vala/commit/?id=294b374af6
 */
#if VALA_0_16
            device = device_manager.list_devices(Gdk.DeviceType.MASTER).data;
#else
            unowned GLib.List<Gdk.Device> devices = device_manager.list_devices(Gdk.DeviceType.MASTER);
            device = devices.data;
#endif
        }

        Gdk.Device keyboard;
        Gdk.Device pointer;
        if (device.get_source() == Gdk.InputSource.KEYBOARD) {
            keyboard = device;
            pointer = device.get_associated_device();
        } else {
            pointer = device;
            keyboard = device.get_associated_device();
        }

        show_all();

        if (is_composited()) {
            // Hide the window by set the opactiy to 0.0, because real hiden
            // window can not grab keyboard and pointer.
            get_window().set_opacity(0.0);

            // Show window after 1/10 secound
            GLib.Timeout.add(100, ()=> {
                get_window().set_opacity(1.0);
                return false;
            });
        }

        Gdk.GrabStatus status;
        // Grab all keyboard events
        status = keyboard.grab(get_window(),
                               Gdk.GrabOwnership.NONE,
                               true,
                               Gdk.EventMask.KEY_PRESS_MASK |
                               Gdk.EventMask.KEY_RELEASE_MASK,
                               null,
                               Gdk.CURRENT_TIME);
        if (status != Gdk.GrabStatus.SUCCESS)
            warning("Grab keyboard failed! status = %d", status);
        // Grab all pointer events
        status = pointer.grab(get_window(),
                              Gdk.GrabOwnership.NONE,
                              true,
                              Gdk.EventMask.BUTTON_PRESS_MASK |
                              Gdk.EventMask.BUTTON_RELEASE_MASK,
                              null,
                              Gdk.CURRENT_TIME);
        if (status != Gdk.GrabStatus.SUCCESS)
            warning("Grab pointer failed! status = %d", status);


        m_loop = new GLib.MainLoop();
        m_loop.run();
        m_loop = null;

        keyboard.ungrab(Gdk.CURRENT_TIME);
        pointer.ungrab(Gdk.CURRENT_TIME);

        hide();
        // Make sure the switcher is hidden before returning from this function.
        while (Gtk.events_pending())
            Gtk.main_iteration ();

        return m_result;
    }

    private void update_engines(IBus.EngineDesc[] engines) {
        foreach (var button in m_buttons) {
            button.destroy();
        }
        m_buttons = {};

        if (engines == null) {
            m_engines = {};
            return;
        }

        int width, height;
        Gtk.icon_size_lookup(Gtk.IconSize.MENU, out width, out height);
        m_engines = engines;
        for (int i = 0; i < m_engines.length; i++) {
            var index = i;
            var engine = m_engines[i];
            var button = new Gtk.Button.with_label(engine.get_longname());
            button.set_image(new IconWidget(engine.get_icon(), width));
            button.set_relief(Gtk.ReliefStyle.NONE);
            button.show();

            button.enter_notify_event.connect((e) => {
                button.grab_focus();
                m_selected_engine = index;
                return true;
            });

            button.button_press_event.connect((e) => {
                m_selected_engine = index;
                m_result = (int)m_selected_engine;
                m_loop.quit();
                return true;
            });

            m_box.pack_start(button, true, true);
            m_buttons += button;
        }

        // Cause window recalculate the perferred size.
        resize(1, 1);
    }

    private void next_engine() {
        if (m_selected_engine == m_engines.length - 1)
            m_selected_engine = 0;
        else
            m_selected_engine ++;
        set_focus(m_buttons[m_selected_engine]);
    }

    private void previous_engine() {
        if (m_selected_engine == 0)
            m_selected_engine = m_engines.length - 1;
        else
            m_selected_engine --;
        set_focus(m_buttons[m_selected_engine]);
    }

    /* override virtual functions */
    public override void show() {
        base.show();
        set_focus_visible(true);
    }

    public override bool key_press_event(Gdk.EventKey e) {
        Gdk.EventKey *pe = &e;
        switch (pe->keyval) {
            case 0x0020: /* space */
            case 0xff80: /* KP_Space */
                if ((pe->state & Gdk.ModifierType.SHIFT_MASK) == 0)
                    next_engine();
                else
                    previous_engine();
                break;
            case 0x08fb: /* leftarrow */
            case 0xff51: /* Down */
                break;
            case 0x08fc: /* uparrow */
            case 0xff52: /* Up */
                previous_engine();
                break;
            case 0x08fd: /* rightarrow */
            case 0xff53: /* Right */
                break;
            case 0x08fe: /* downarrow */
            case 0xff54: /* Down */
                next_engine();
                break;
            default:
                debug("0x%04x", pe->keyval);
                break;
        }
        return true;
    }

    public override bool key_release_event(Gdk.EventKey e) {
        Gdk.EventKey *pe = &e;

        if (KeybindingManager.primary_modifier_still_pressed((Gdk.Event *)pe,
            m_primary_modifier)) {
            return true;
        }

        // if e.type == Gdk.EventType.KEY_RELEASE, m_loop is already null.
        if (m_loop == null) {
            return false;
        }

        m_loop.quit();
        m_result = (int)m_selected_engine;
        return true;
    }
}
