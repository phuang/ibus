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
        GLib.Object(type : Gtk.WindowType.POPUP);
        set_accept_focus(true);
        set_decorated(false);
        set_position(Gtk.WindowPosition.CENTER);
        add_events(Gdk.EventMask.KEY_PRESS_MASK);
        add_events(Gdk.EventMask.KEY_RELEASE_MASK);

        m_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
        add(m_box);

        grab_focus();
    }

    public int run(Gdk.Event event, IBus.EngineDesc[] engines, int index) {
        assert (m_loop == null);
        assert (index < engines.length);

        m_selected_engine = index;
        update_engines(engines);

        show_all();
        Gdk.Device device = event.get_device();
        device.grab(get_window(),
                    Gdk.GrabOwnership.NONE,
                    true,
                    Gdk.EventMask.KEY_PRESS_MASK |
                    Gdk.EventMask.KEY_RELEASE_MASK,
                    null,
                    Gdk.CURRENT_TIME);
        m_primary_modifier =
            KeybindingManager.get_primary_modifier(event.key.state);

        m_loop = new GLib.MainLoop();
        m_loop.run();
        m_loop = null;
        hide();
        debug("run over");
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
        foreach (var engine in m_engines) {
            var button = new Gtk.Button.with_label(engine.get_longname());
            button.set_image(new IconWidget(engine.get_icon(), width));
            button.set_relief(Gtk.ReliefStyle.NONE);
            button.show();
            m_box.pack_start(button, true, true);
            m_buttons += button;
        }
    }

    private void next_engine() {
        if (m_selected_engine == m_engines.length - 1)
            m_selected_engine = 0;
        else
            m_selected_engine ++;
        set_focus(m_buttons[m_selected_engine]);
        m_buttons[m_selected_engine].set_state_flags(Gtk.StateFlags.FOCUSED, true);
        debug("next engine");
    }

    private void previous_engine() {
        if (m_selected_engine == 0)
            m_selected_engine = m_engines.length - 1;
        else
            m_selected_engine --;
        set_focus(m_buttons[m_selected_engine]);
        debug("previous engine");
    }

    /* override virtual functions */
    public override void show() {
        base.show();
        debug("is_active = %d", (int)this.is_active);
    }

    public override void grab_focus() {
        base.grab_focus();
        debug("grab_focus");
        set_focus(m_buttons[m_selected_engine]);
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
        if (m_primary_modifier != KeybindingManager.keyval_to_modifier(pe->keyval))
            return true;

        if (KeybindingManager.primary_modifier_still_pressed((Gdk.Event *)pe))
            return true;

        m_loop.quit();
        m_result = (int)m_selected_engine;
        return true;
    }
}
