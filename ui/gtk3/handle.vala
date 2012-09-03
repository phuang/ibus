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

class Handle : Gtk.EventBox {
    private bool m_move_begined;
    private Gdk.Rectangle m_workarea;
    private Gdk.Point m_press_pos;

    public signal void move_begin();
    public signal void move_end();

    public Handle() {
        set_size_request(6, -1);
        Gdk.EventMask mask = Gdk.EventMask.EXPOSURE_MASK |
                             Gdk.EventMask.BUTTON_PRESS_MASK |
                             Gdk.EventMask.BUTTON_RELEASE_MASK |
                             Gdk.EventMask.BUTTON1_MOTION_MASK;
        set_events(mask);
        m_move_begined = false;
    }

    public override void realize() {
        base.realize();
        // get_window().set_cursor(new Gdk.Cursor(Gdk.CursorType.FLEUR));
    }

    public override bool button_press_event(Gdk.EventButton event) {
        if (event.button != 1)
            return false;
        m_workarea = Gdk.Rectangle(){
            x = 0, y = 0, width = int.MAX, height = int.MAX};
        do {
            Gdk.Window root = Gdk.get_default_root_window();
            Gdk.Atom property = Gdk.Atom.intern("_NET_CURRENT_DESKTOP", false);
            Gdk.Atom type = Gdk.Atom.intern("CARDINAL", false);
            Gdk.Atom actual_type;
            int format;
            uchar[] data;
            bool result;
            result = Gdk.property_get(root,
                                      property,
                                      type,
                                      0, long.MAX,
                                      0,
                                      out actual_type,
                                      out format,
                                      out data);
            if (!result || actual_type != type || format != 32 || data.length != 4)
                break;
            int index = data[0] |
                        data[1] << 8 |
                        data[2] << 16 |
                        data[3] << 24;
            property = Gdk.Atom.intern("_NET_WORKAREA", false);
            type = Gdk.Atom.intern("CARDINAL", false);
            result = Gdk.property_get(root,
                                      property,
                                      type,
                                      0, long.MAX,
                                      0,
                                      out actual_type,
                                      out format,
                                      out data);
            if (!result || actual_type != type || format != 32 || data.length < (index + 1) * 16)
                break;
            int i = index * 4 * 4;
            m_workarea.x = data[i] |
                           data[i + 1] << 8 |
                           data[i + 2] << 16 |
                           data[i + 3] << 24;
            i += 4;
            m_workarea.y = data[i] |
                           data[i + 1] << 8 |
                           data[i + 2] << 16 |
                           data[i + 3] << 24;
            i += 4;
            m_workarea.width = data[i] |
                               data[i + 1] << 8 |
                               data[i + 2] << 16 |
                               data[i + 3] << 24;
            i += 4;
            m_workarea.height = data[i] |
                                data[i + 1] << 8 |
                                data[i + 2] << 16 |
                                data[i + 3] << 24;
        } while (false);
        m_move_begined = true;
        int x, y;
        Gtk.Window toplevel = (Gtk.Window)get_toplevel();
        toplevel.get_position(out x, out y);
        m_press_pos.x = (int)event.x_root - x;
        m_press_pos.y = (int)event.y_root - y;
        move_begin();
        return true;
    }

    public override bool button_release_event(Gdk.EventButton event) {
        if (event.button != 1)
            return false;
        m_move_begined = false;
        m_press_pos.x = 0;
        m_press_pos.y = 0;
        get_window().set_cursor(new Gdk.Cursor(Gdk.CursorType.LEFT_PTR));
        move_end();
        return true;
    }

    public override bool motion_notify_event(Gdk.EventMotion event) {
        if (!m_move_begined)
            return false;
        Gtk.Window toplevel = (Gtk.Window)get_toplevel();
        int x = (int)(event.x_root - m_press_pos.x);
        int y = (int)(event.y_root - m_press_pos.y);

        if (x < m_workarea.x && x > m_workarea.x - 16)
            x = m_workarea.x;
        if (y < m_workarea.y && y > m_workarea.y - 16)
            y = m_workarea.y;
        int w, h;
        toplevel.get_size(out w, out h);
        if (x + w > m_workarea.x + m_workarea.width &&
            x + w < m_workarea.x + m_workarea.width + 16)
            x = m_workarea.x + m_workarea.width - w;
        if (y + h > m_workarea.y + m_workarea.height &&
            y + h < m_workarea.y + m_workarea.height + 16)
            y = m_workarea.y + m_workarea.height - w;
        toplevel.move(x, y);
        return true;
    }

    public override bool draw(Cairo.Context cr) {
        if (Gtk.cairo_should_draw_window(cr, get_window())) {
            Gtk.StyleContext context = get_style_context();
            Gtk.Allocation allocation;
            get_allocation(out allocation);
            context.render_handle(cr,
                allocation.x, allocation.y + (allocation.height - 40) / 2, allocation.width, 40.0);
        }
        return false;
    }
}
