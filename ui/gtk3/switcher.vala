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
        GLib.Object(
            type : Gtk.WindowType.TOPLEVEL, // TODO POPUP
            window_position: Gtk.WindowPosition.CENTER
        );
        init();
    }

    private void init() {
        m_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
        add(m_box);
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
}



