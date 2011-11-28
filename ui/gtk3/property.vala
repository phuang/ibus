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


public class PropertyManager {
    private IBus.PropList m_props;
    private GLib.HashTable<string, IPropItem> m_prop_map =
        new GLib.HashTable<string, IPropItem>(GLib.str_hash, null);
    private Gtk.Menu m_menu;

    public void ProperyManager() {
    }

    public void set_properties(IBus.PropList props) {
        m_props = props;
        m_prop_map.remove_all();
        if (m_menu != null)
            m_menu.destroy();
        m_menu = create_menu(props);
    }

    public Gtk.Menu? get_menu() {
        return m_menu;
    }

    public Gtk.Menu? create_menu(IBus.PropList props) {
        Gtk.Menu menu = new Gtk.Menu();
        int i = 0;
        GLib.SList<PropRadioMenuItem> group = 
            new GLib.SList<PropRadioMenuItem>();
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
                    {
                        PropRadioMenuItem radio = new PropRadioMenuItem(prop, group);
                        group.append(radio);
                        item = radio;
                    }
                    break;
                case IBus.PropType.MENU:
                    {
                        var menuitem = new PropImageMenuItem(prop);
                        item = menuitem;
                        Gtk.Menu submenu = create_menu(prop.get_sub_props());
                        menuitem.set_submenu(submenu);
                    }
                    break;
                case IBus.PropType.SEPARATOR:
                    item = new PropSeparatorMenuItem(prop);
                    break;
                default:
                    warning("unknown property type %d", (int)prop.get_prop_type());
                    break;
            }
            if (item != null) {
                m_prop_map.insert(prop.get_key(), item);
                menu.append(item as Gtk.MenuItem);
                item.property_activate.connect((k, s) => property_activate(k, s));
            }
        }

        if (i == 0)
            return null;
        return menu;
    }

    public void update_property(IBus.Property prop) {
        assert(prop != null);
        
        IPropItem item = m_prop_map.lookup(prop.get_key());
        return_if_fail(item != null);
        item.update_property(prop);
    }

    public signal void property_activate(string key, int state); 
}

public interface IPropItem : GLib.Object {
    public abstract void update_property(IBus.Property prop);
    public signal void property_activate(string key, int state);
}

public class PropImageMenuItem : Gtk.ImageMenuItem, IPropItem {
    private IBus.Property m_property;
    public PropImageMenuItem(IBus.Property property) {
        assert(property != null);

        m_property = property;
        set_no_show_all(true);
        sync();
    }

    public void update_property(IBus.Property property) {
        m_property.set_label(property.get_label());
        m_property.set_icon(property.get_icon());
        m_property.set_visible(property.get_visible());
        m_property.set_sensitive(property.get_sensitive());
        m_property.set_tooltip(property.get_tooltip());
        m_property.set_state(property.get_state());
        sync();
    }

    private void sync() {
        set_label(m_property.get_label().get_text());
        set_icon(m_property.get_icon());
        set_visible(m_property.get_visible());
        set_sensitive(m_property.get_sensitive());
    }

    private void set_icon(string icon) {
        int width, height;
        Gtk.icon_size_lookup(Gtk.IconSize.MENU, out width, out height);
        set_image(new IconWidget(icon, width));
    }

    public override void activate() {
        property_activate(m_property.get_key(), m_property.get_state());
    }
}

public class PropCheckMenuItem : Gtk.RadioMenuItem, IPropItem {
    private IBus.Property m_property;
    public PropCheckMenuItem(IBus.Property property) {
        assert(property != null);

        m_property = property;
        set_no_show_all(true);
        sync();
    }

    public void update_property(IBus.Property property) {
        m_property.set_label(property.get_label());
        m_property.set_icon(property.get_icon());
        m_property.set_visible(property.get_visible());
        m_property.set_sensitive(property.get_sensitive());
        m_property.set_tooltip(property.get_tooltip());
        m_property.set_state(property.get_state());
        sync();
    }

    private void sync() {
        set_label(m_property.get_label().get_text());
        set_visible(m_property.get_visible());
        set_sensitive(m_property.get_sensitive());
        set_active(m_property.get_state() == IBus.PropState.CHECKED);
    }

    public override void toggled() {
        IBus.PropState new_state = 
            get_active() ? IBus.PropState.CHECKED : IBus.PropState.UNCHECKED;
        if (m_property.get_state() != new_state) {
            m_property.set_state(new_state);
            property_activate(m_property.get_key(), m_property.get_state());
        }
    }
}

public class PropRadioMenuItem : PropCheckMenuItem {
    public PropRadioMenuItem(IBus.Property property, GLib.SList<PropRadioMenuItem> group) {
        base(property);
        set_group(group);
    }
}

public class PropSeparatorMenuItem : Gtk.SeparatorMenuItem, IPropItem {
    private IBus.Property m_property;
    public PropSeparatorMenuItem(IBus.Property property) {
        assert(property != null);
        m_property = property;
    }

    public void update_property(IBus.Property property) {
    }
}
