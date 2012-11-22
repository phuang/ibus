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

class IconWidget: Gtk.Image {
    /**
     * IconWidget:
     * @icon_name_or_path: Can be a name or path but not stock id
     *     because gtk_icon_theme_load_icon() cannot fallback the
     *     stock id to a real file name against
     *     gtk_image_new_from_stock().
     * @size: #Gtk.IconSize
     */
    public IconWidget(string icon_name_or_path, Gtk.IconSize size) {
        Gdk.Pixbuf pixbuf = null;
        int fixed_width, fixed_height;
        Gtk.icon_size_lookup(size, out fixed_width, out fixed_height);

        try {
            if (icon_name_or_path[0] == '/') {
                pixbuf = new Gdk.Pixbuf.from_file(icon_name_or_path);
            } else {
                var theme = Gtk.IconTheme.get_default();
                pixbuf = theme.load_icon(icon_name_or_path, fixed_width, 0);
            }
        } catch (GLib.Error e) {
            try {
                var theme = Gtk.IconTheme.get_default();
                pixbuf = theme.load_icon("ibus-engine", fixed_width, 0);
            } catch (GLib.Error e) {
                /* "gtk-missing-image.png" is the symlink of
                 * "image-missing.png" and included in
                 * gnome-icon-theme-legacy package in fedora.
                 * gtk_image_set_from_stock() can fallback the stock name
                 * to the real name instead of gtk_image_set_from_icon_name()
                 * or gtk_icon_theme_load_icon() and
                 * could remove gnome-icon-theme-legacy.
                 */
                set_from_stock(Gtk.Stock.MISSING_IMAGE, size);
                return;
            }
        }

        if (pixbuf == null)
            return;
        float width = (float)pixbuf.get_width();
        float height = (float)pixbuf.get_height();
        float scale = fixed_width / (width > height ? width : height);
        width *= scale;
        height *= scale;

        pixbuf = pixbuf.scale_simple((int)width, (int)height, Gdk.InterpType.BILINEAR);
        set_from_pixbuf(pixbuf);
        show();
    }
}
