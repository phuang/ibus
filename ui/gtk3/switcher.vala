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

    public Switcher() {
        GLib.Object(type : Gtk.WindowType.POPUP);
        set_can_focus(true);
        set_decorated(false);
        set_position(Gtk.WindowPosition.CENTER);
        add_events(Gdk.EventMask.KEY_PRESS_MASK);
        add_events(Gdk.EventMask.KEY_RELEASE_MASK);

        key_press_event.connect((e) => {
            debug ("press");
            if (e.keyval == 0xff1b /* Escape */)
                hide();
            return true;
        });

        key_release_event.connect((e) => {
            debug ("release");
            return true;
        });

        m_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
        add(m_box);

        grab_focus();
    }

    public void update_engines(IBus.EngineDesc[] engines) {
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
            button.show();
            m_box.pack_start(button, true, true);
            m_buttons += button;
        }
    }

    public void start_switch(Gdk.Event event) {
        show_all();
        Gdk.Device device = event.get_device();
        device.grab(get_window(),
                    Gdk.GrabOwnership.NONE,
                    true,
                    Gdk.EventMask.KEY_PRESS_MASK |
                    Gdk.EventMask.KEY_RELEASE_MASK,
                    null,
                    Gdk.CURRENT_TIME);
    }

    public override void show() {
        base.show();
        get_window().focus(Gdk.CURRENT_TIME);
    }
}
