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

using Gdk;
using GLib;
using Gtk;

class IconWidget: Gtk.Image {
    public IconWidget(string icon, int size) {
        Gdk.Pixbuf pixbuf = null;
        try {
            if (icon[0] == '/') {
                pixbuf = new Gdk.Pixbuf.from_file(icon);
            } else {
                var theme = Gtk.IconTheme.get_default();
                pixbuf = theme.load_icon(icon, size, 0);
            }
        } catch (GLib.Error e) {
            try {
                var theme = Gtk.IconTheme.get_default();
                pixbuf = theme.load_icon(Gtk.Stock.MISSING_IMAGE, size, 0);
            } catch (GLib.Error e) {}
        }

        if (pixbuf == null)
            return;
        float width = (float)pixbuf.get_width();
        float height = (float)pixbuf.get_height();
        float scale = size / (width > height ? width : height);
        width *= scale;
        height *= scale;

        pixbuf = pixbuf.scale_simple((int)width, (int)height, Gdk.InterpType.BILINEAR);
        set_from_pixbuf(pixbuf);
        show();
    }
}
