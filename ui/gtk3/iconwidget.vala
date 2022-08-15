/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright(c) 2011-2014 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright(c) 2018 Takao Fujiwara <takao.fujiwara1@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

class ThemedRGBA {
    public Gdk.RGBA *normal_fg { get; set; }
    public Gdk.RGBA *normal_bg { get; set; }
    public Gdk.RGBA *selected_fg { get; set; }
    public Gdk.RGBA *selected_bg { get; set; }

    private Gtk.StyleContext m_style_context;

    public ThemedRGBA(Gtk.Widget widget) {
        this.normal_fg = null;
        this.normal_bg = null;
        this.selected_fg = null;
        this.selected_bg = null;

        /* Use the color of Gtk.TextView instead of Gtk.Label
         * because the selected label "color" is not configured
         * in "Adwaita" theme and the selected label "background-color"
         * is not configured in "Maia" theme.
         * https://github.com/ibus/ibus/issues/1871
         */
        Gtk.WidgetPath widget_path = new Gtk.WidgetPath();
        widget_path.append_type(typeof(Gtk.TextView));
        m_style_context = new Gtk.StyleContext();
        m_style_context.set_path(widget_path);
        m_style_context.add_class(Gtk.STYLE_CLASS_VIEW);

        /* "-gtk-secondary-caret-color" value is different
         * if the parent widget is set in "Menta" theme.
         */
        m_style_context.set_parent(widget.get_style_context());

        get_rgba();

        m_style_context.changed.connect(() => { get_rgba(); });
    }

    ~ThemedRGBA() {
        reset_rgba();
    }

    private void reset_rgba() {
        if (this.normal_fg != null) {
            this.normal_fg.free();
            this.normal_fg = null;
        }
        if (this.normal_bg != null) {
            this.normal_bg.free();
            this.normal_bg = null;
        }
        if (this.selected_fg != null) {
            this.selected_fg.free();
            this.selected_fg = null;
        }
        if (this.selected_bg != null) {
            this.selected_bg.free();
            this.selected_bg = null;
        }
    }

    private void get_rgba() {
        reset_rgba();
        Gdk.RGBA *normal_fg = null;
        Gdk.RGBA *normal_bg = null;
        Gdk.RGBA *selected_fg = null;
        Gdk.RGBA *selected_bg = null;
        m_style_context.get(Gtk.StateFlags.NORMAL,
                            "color",
                            out normal_fg);
        m_style_context.get(Gtk.StateFlags.SELECTED,
                            "color",
                            out selected_fg);

        string bg_prop = "background-color";
        m_style_context.get(Gtk.StateFlags.NORMAL,
                            bg_prop,
                            out normal_bg);
        m_style_context.get(Gtk.StateFlags.SELECTED,
                            bg_prop,
                            out selected_bg);
        if (normal_bg.red   == selected_bg.red &&
            normal_bg.green == selected_bg.green &&
            normal_bg.blue  == selected_bg.blue &&
            normal_bg.alpha == selected_bg.alpha) {
            normal_bg.free();
            normal_bg = null;
            normal_bg.free();
            normal_bg = null;
            bg_prop = "-gtk-secondary-caret-color";
            m_style_context.get(Gtk.StateFlags.NORMAL,
                                bg_prop,
                                out normal_bg);
            m_style_context.get(Gtk.StateFlags.SELECTED,
                                bg_prop,
                                out selected_bg);
        }
        this.normal_fg   = normal_fg;
        this.normal_bg   = normal_bg;
        this.selected_fg = selected_fg;
        this.selected_bg = selected_bg;
    }
}

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
                set_from_icon_name("image-missing", size);
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
