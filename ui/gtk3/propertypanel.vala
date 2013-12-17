/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright(c) 2013 Red Hat, Inc.
 * Copyright(c) 2013 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright(c) 2013 Takao Fujiwara <takao.fujiwara1@gmail.com>
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

enum PanelShow {
    DO_NOT_SHOW,
    AUTO_HIDE,
    ALWAYS
}

public class PropertyPanel : Gtk.Box {
    private Gtk.Window m_toplevel;
    private IBus.PropList m_props;
    private IPropToolItem[] m_items;
    private Gdk.Rectangle m_cursor_location;
    private int m_show = PanelShow.AUTO_HIDE;
    private uint m_timeout = 3000;
    private uint m_timeout_id = 0;

    public PropertyPanel() {
        /* Chain up base class constructor */
        GLib.Object(orientation: Gtk.Orientation.HORIZONTAL,
                    spacing: 0);

        set_visible(true);

        m_toplevel = new Gtk.Window(Gtk.WindowType.POPUP);
        m_toplevel.add_events(Gdk.EventMask.BUTTON_PRESS_MASK);

        Handle handle = new Handle();
        handle.set_visible(true);
        pack_start(handle, false, false, 0);

        m_toplevel.add(this);
    }

    public void set_properties(IBus.PropList props) {
        foreach (var item in m_items)
            (item as Gtk.Widget).destroy();
        m_items = {};

        m_props = props;

        create_menu_items();
    }

    public void update_property(IBus.Property prop) {
        GLib.assert(prop != null);
        if (m_props != null)
            m_props.update_property(prop);

        /* Need to update GUI since panel buttons are not redrawn. */
        foreach (var item in m_items)
            item.update_property(prop);

        set_show_timer();
    }

    public void set_cursor_location(int x, int y, int width, int height) {
        Gdk.Rectangle location = Gdk.Rectangle(){
            x = x, y = y, width = width, height = height };
        if (m_cursor_location == location)
            return;
        m_cursor_location = location;
        adjust_window_position();
    }

    public new void show() {
        if (m_show == PanelShow.DO_NOT_SHOW)
            return;

        m_toplevel.show_all();
    }

    public new void hide() {
        m_toplevel.hide();
    }

    public void focus_in() {
        set_show_timer();
    }

    public void set_show(int _show) {
        m_show = _show;
        show();
    }

    public void set_timeout(uint timeout) {
        m_timeout = timeout;
    }

    private int create_menu_items() {
        int i = 0;
        while (true) {
            IBus.Property prop = m_props.get(i);
            if (prop == null)
                break;

            i++;
            IPropToolItem item = null;
            switch(prop.get_prop_type()) {
                case IBus.PropType.NORMAL:
                    item = new PropToolButton(prop);
                    break;
                case IBus.PropType.TOGGLE:
                    item = new PropToggleToolButton(prop);
                    break;
                case IBus.PropType.MENU:
                    item = new PropMenuToolButton(prop);
                    break;
                case IBus.PropType.SEPARATOR:
                    item = new PropSeparatorToolItem(prop);
                    break;
                default:
                    warning("unknown property type %d",
                            (int) prop.get_prop_type());
                    break;
            }
            if (item != null) {
                pack_start(item as Gtk.Widget, false, false, 0);
                m_items += item;
                item.property_activate.connect((w, k, s) =>
                                               property_activate(k, s));
            }
        }
        return i;
    }

    private void move(int x, int y) {
        m_toplevel.move(x, y);
    }

    private void adjust_window_position() {
        Gdk.Point cursor_right_bottom = {
                m_cursor_location.x + m_cursor_location.width,
                m_cursor_location.y + m_cursor_location.height
        };

        Gtk.Allocation allocation;
        m_toplevel.get_allocation(out allocation);
        Gdk.Point window_right_bottom = {
            cursor_right_bottom.x + allocation.width,
            cursor_right_bottom.y + allocation.height
        };

        Gdk.Window root = Gdk.get_default_root_window();
        int root_width = root.get_width();
        int root_height = root.get_height();

        int x, y;
        if (window_right_bottom.x > root_width)
            x = root_width - allocation.width;
        else
            x = cursor_right_bottom.x;

        if (window_right_bottom.y > root_height)
            y = m_cursor_location.y - allocation.height;
        else
            y = cursor_right_bottom.y;

        move(x, y);
    }

    private void set_show_timer() {
        if (m_show != PanelShow.AUTO_HIDE)
            return;

        if (m_timeout_id != 0)
            GLib.Source.remove(m_timeout_id);

        m_toplevel.show_all();

        /* Change the priority because IME typing sometimes freezes. */
        m_timeout_id = GLib.Timeout.add(m_timeout, () => {
            m_toplevel.hide();
            m_timeout_id = 0;
            return false;
        },
        GLib.Priority.DEFAULT_IDLE);
    }

    public signal void property_activate(string key, int state);
}

public interface IPropToolItem : GLib.Object {
    public abstract void update_property(IBus.Property prop);
    public signal void property_activate(string key, int state);
}

public class PropMenu : Gtk.Menu, IPropToolItem {
    private Gtk.Widget m_parent_button;
    private IPropItem[] m_items;

    public PropMenu(IBus.Property prop) {
        /* Chain up base class constructor */
        GLib.Object();

        set_take_focus(false);
        create_items(prop.get_sub_props());
        show_all();
        set_sensitive(prop.get_sensitive());
    }

    public void update_property(IBus.Property prop) {
        foreach (var item in m_items)
            item.update_property(prop);
    }

    public new void popup(uint       button,
                          uint32     activate_time,
                          Gtk.Widget widget) {
        m_parent_button = widget;
        base.popup(null, null, menu_position, button, activate_time);
    }

    public override void destroy() {
        m_parent_button = null;
        m_items = {};
        base.destroy();
    }

    private void create_items(IBus.PropList props) {
        int i = 0;
        PropRadioMenuItem last_radio = null;

        while (true) {
            IBus.Property prop = props.get(i);
            if (prop == null)
                break;

            i++;
            IPropItem item = null;
            switch(prop.get_prop_type()) {
                case IBus.PropType.NORMAL:
                    item = new PropImageMenuItem(prop);
                    break;
                case IBus.PropType.TOGGLE:
                    item = new PropCheckMenuItem(prop);
                    break;
                case IBus.PropType.RADIO:
                    last_radio = new PropRadioMenuItem(prop, last_radio);
                    item = last_radio;
                    break;
                case IBus.PropType.MENU:
                    {
                        var menuitem = new PropImageMenuItem(prop);
                        menuitem.set_submenu(new PropMenu(prop));
                        item = menuitem;
                    }
                    break;
                case IBus.PropType.SEPARATOR:
                    item = new PropSeparatorMenuItem(prop);
                    break;
                default:
                    warning("Unknown property type: %d",
                            (int) prop.get_prop_type());
                    break;
            }
            if (prop.get_prop_type() != IBus.PropType.RADIO)
                last_radio = null;
            if (item != null) {
                append(item as Gtk.MenuItem);
                item.property_activate.connect((w, k, s) =>
                                               property_activate(k, s));
                m_items += item;
            }
        }
    }

    private void menu_position(Gtk.Menu menu,
                               out int  x,
                               out int  y,
                               out bool push_in) {
        var button = m_parent_button;
        var screen = button.get_screen();
        var monitor = screen.get_monitor_at_window(button.get_window());

        Gdk.Rectangle monitor_location;
        screen.get_monitor_geometry(monitor, out monitor_location);

        button.get_window().get_origin(out x, out y);

        Gtk.Allocation button_allocation;
        button.get_allocation(out button_allocation);

        x += button_allocation.x;
        y += button_allocation.y;

        int menu_width;
        int menu_height;
        menu.get_size_request(out menu_width, out menu_height);

        if (x + menu_width >= monitor_location.width)
            x -= menu_width - button_allocation.width;
        else if (x - menu_width <= 0)
            ;
        else {
            if (x <= monitor_location.width * 3 / 4)
                ;
            else
                x -= menu_width - button_allocation.width;
        }

        if (y + button_allocation.height + menu_width
                >= monitor_location.height)
            y -= menu_height;
        else if (y - menu_height <= 0)
            y += button_allocation.height;
        else {
            if (y <= monitor_location.height * 3 / 4)
                y += button_allocation.height;
            else
                y -= menu_height;
        }

        push_in = false;
    }
}

public class PropToolButton : Gtk.ToolButton, IPropToolItem {
    private IBus.Property m_prop = null;

    public PropToolButton(IBus.Property prop) {
        string label = prop.get_symbol().get_text();

        /* Chain up base class constructor */
        GLib.Object(label: label);

        m_prop = prop;

        set_homogeneous(false);
        sync();
    }

    public void update_property(IBus.Property prop) {
        if (m_prop.get_key() != prop.get_key())
            return;
        m_prop.set_symbol(prop.get_symbol());
        m_prop.set_tooltip(prop.get_tooltip());
        m_prop.set_sensitive(prop.get_sensitive());
        m_prop.set_icon(prop.get_icon());
        m_prop.set_state(prop.get_state());
        m_prop.set_visible(prop.get_visible());
        sync();
    }

    private void sync() {
        set_label(m_prop.get_symbol().get_text());
        set_tooltip_text(m_prop.get_tooltip().get_text());
        set_sensitive(m_prop.get_sensitive());
        set_icon_name(m_prop.get_icon());

        if (m_prop.get_visible())
            show();
        else
            hide();
    }

    public override void clicked() {
        property_activate(m_prop.get_key(), m_prop.get_state());
    }

    public new void set_icon_name(string icon_name) {
        string label = m_prop.get_symbol().get_text();
        IconWidget icon_widget = null;

        if (label == "") {
            label = null;
            icon_widget = new IconWidget(icon_name, Gtk.IconSize.BUTTON);
            set_is_important(false);
        } else {
            set_is_important(true);
        }

        set_icon_widget(icon_widget);
    }
}

public class PropToggleToolButton : Gtk.ToggleToolButton, IPropToolItem {
    private IBus.Property m_prop = null;

    public PropToggleToolButton(IBus.Property prop) {
        /* Chain up base class constructor */
        GLib.Object();

        m_prop = prop;

        set_homogeneous(false);
        sync();
    }

    public new void set_property(IBus.Property prop) {
        m_prop = prop;
        sync();
    }

    public void update_property(IBus.Property prop) {
        if (m_prop.get_key() != prop.get_key())
            return;
        m_prop.set_symbol(prop.get_symbol());
        m_prop.set_tooltip(prop.get_tooltip());
        m_prop.set_sensitive(prop.get_sensitive());
        m_prop.set_icon(prop.get_icon());
        m_prop.set_state(prop.get_state());
        m_prop.set_visible(prop.get_visible());
        sync();
    }

    private void sync() {
        set_label(m_prop.get_symbol().get_text());
        set_tooltip_text(m_prop.get_tooltip().get_text());
        set_sensitive(m_prop.get_sensitive());
        set_icon_name(m_prop.get_icon());
        set_active(m_prop.get_state() == IBus.PropState.CHECKED);

        if (m_prop.get_visible())
            show();
        else
            hide();
    }

    public override void toggled() {
        /* Do not send property-activate to engine in case the event is
         * sent from engine. */

        bool do_emit = false;

        if (get_active()) {
            if (m_prop.get_state() != IBus.PropState.CHECKED)
                do_emit = true;
            m_prop.set_state(IBus.PropState.CHECKED);
        } else {
            if (m_prop.get_state() != IBus.PropState.UNCHECKED)
                do_emit = true;
            m_prop.set_state(IBus.PropState.UNCHECKED);
        }

        if (do_emit)
            property_activate(m_prop.get_key(), m_prop.get_state());
    }

    public new void set_icon_name(string icon_name) {
        string label = m_prop.get_symbol().get_text();
        IconWidget icon_widget = null;

        if (label == "") {
            label = null;
            icon_widget = new IconWidget(icon_name, Gtk.IconSize.BUTTON);
            set_is_important(false);
        } else {
            set_is_important(true);
        }

        set_icon_widget(icon_widget);
    }
}

public class PropMenuToolButton : PropToggleToolButton, IPropToolItem {
    private PropMenu m_menu = null;

    public PropMenuToolButton(IBus.Property prop) {
        /* Chain up base class constructor */
        GLib.Object();

        m_menu = new PropMenu(prop);
        m_menu.deactivate.connect((m) =>
                                  set_active(false));
        m_menu.property_activate.connect((w, k, s) =>
                                         property_activate(k, s));

        base.set_property(prop);
    }

    public new void update_property(IBus.Property prop) {
        base.update_property(prop);
        m_menu.update_property(prop);
    }

    public override void toggled() {
        if (get_active())
            m_menu.popup(0, Gtk.get_current_event_time(), this);
    }

    public override void destroy() {
        m_menu = null;
        base.destroy();
    }
}

public class PropSeparatorToolItem : Gtk.SeparatorToolItem, IPropToolItem {
    public PropSeparatorToolItem(IBus.Property prop) {
        /* Chain up base class constructor */
        GLib.Object();

        set_homogeneous(false);
    }

    public void update_property(IBus.Property prop) {
    }
}
