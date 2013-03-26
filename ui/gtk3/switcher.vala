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

class Switcher : Gtk.Window {
    public extern const bool USE_SYMBOL_ICON;
    private const int DEFAULT_FONT_SIZE = 16;
    private const int DESC_LABEL_MAX_LEN = 20;

    private class IBusEngineButton : Gtk.Button {
        public IBusEngineButton(IBus.EngineDesc engine) {
            GLib.Object();

            this.longname = engine.get_longname();

            Gtk.Alignment align = new Gtk.Alignment(0.5f, 0.5f, 0.0f, 0.0f);
            add(align);

            if (!USE_SYMBOL_ICON) {
                IconWidget icon = new IconWidget(engine.get_icon(),
                                                 Gtk.IconSize.DIALOG);
                align.add(icon);
            } else {
                var language = engine.get_language();
                var symbol = engine.get_symbol();
                var id = language;

                if (id.length > 2) {
                    id = id[0:2];
                }

                if (symbol.length != 0) {
                    id = symbol;
                }

                Gtk.Label label = new Gtk.Label(id);
                string id_font = "%d".printf(DEFAULT_FONT_SIZE);
                string markup = "<span font=\"%s\">%s</span>".printf(id_font, id);

                label.set_markup(markup);
                align.add(label);
            }
        }

        public string longname { get; set; }

        public override bool draw(Cairo.Context cr) {
            base.draw(cr);
            if (is_focus) {
                cr.save();
                cr.rectangle(
                        0, 0, get_allocated_width(), get_allocated_height());
                cr.set_source_rgba(0.0, 0.0, 1.0, 0.1);
                cr.fill();
                cr.restore();
            }
            return true;
        }
    }

    private Gtk.Box m_box;
    private Gtk.Label m_label;
    private IBusEngineButton[] m_buttons = {};
    private IBus.EngineDesc[] m_engines;
    private uint m_selected_engine;
    private uint m_keyval;
    private uint m_modifiers;
    private Gdk.ModifierType m_primary_modifier;
    private GLib.MainLoop m_loop;
    private int m_result;
    private uint m_popup_delay_time = 0;
    private uint m_popup_delay_time_id = 0;
    private int m_root_x;
    private int m_root_y;

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
        Gtk.Box vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
        add(vbox);
        Gtk.Alignment align = new Gtk.Alignment(0.5f, 0.5f, 0.0f, 0.0f);
        vbox.pack_start(align, true, true, 0);
        m_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
        align.add(m_box);
        m_label = new Gtk.Label("");

        /* Set the accessible role of the label to a status bar so it
         * will emit name changed events that can be used by screen
         * readers.
         */
        Atk.Object obj = m_label.get_accessible();
        obj.set_role (Atk.Role.STATUSBAR);

        m_label.set_padding(3, 3);
        vbox.pack_end(m_label, false, false, 0);

        grab_focus();
    }

    public int run(uint keyval,
                   uint state,
                   Gdk.Event event,
                   IBus.EngineDesc[] engines,
                   int index) {
        assert (m_loop == null);
        assert (index < engines.length);

        m_keyval = keyval;
        m_modifiers = state;
        m_primary_modifier =
            KeybindingManager.get_primary_modifier(
                state & KeybindingManager.MODIFIER_FILTER);

        update_engines(engines);
        /* Let gtk recalculate the window size. */
        resize(1, 1);

        m_selected_engine = index;
        m_label.set_text(m_buttons[index].longname);
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
            unowned GLib.List<Gdk.Device> devices =
                    device_manager.list_devices(Gdk.DeviceType.MASTER);
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

        // Avoid regressions.
        if (m_popup_delay_time > 0) {
            get_position(out m_root_x, out m_root_y);
            // Pull the window from the screen so that the window gets
            // the key press and release events but mouse does not select
            // an IME unexpectedly.
            move(-1000, -1000);
        }

        show_all();

        if (m_popup_delay_time > 0) {
            // Restore the window position after m_popup_delay_time
            m_popup_delay_time_id = GLib.Timeout.add(m_popup_delay_time,
                                                     () => {
                restore_window_position("timeout");
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

    /* Based on metacity/src/ui/tabpopup.c:meta_ui_tab_popup_new */
    private void update_engines(IBus.EngineDesc[] engines) {
        foreach (var button in m_buttons) {
            button.destroy();
        }
        m_buttons = {};

        if (engines == null) {
            m_engines = {};
            return;
        }

        m_engines = engines;
        int max_label_width = 0;

        for (int i = 0; i < m_engines.length; i++) {
            var index = i;
            var engine = m_engines[i];
            var button = new IBusEngineButton(engine);
            var longname = engine.get_longname();
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

            if (longname.length > DESC_LABEL_MAX_LEN) {
                longname = longname[0:DESC_LABEL_MAX_LEN];
            }

            button.longname = longname;
            m_label.set_label(longname);

            int width;
            m_label.get_preferred_width(null, out width);
            max_label_width = int.max(max_label_width, width);

            m_box.pack_start(button, true, true);
            m_buttons += button;
        }

        m_label.set_text(m_buttons[0].longname);
        m_label.set_ellipsize(Pango.EllipsizeMode.END);

        Gdk.Display display = Gdk.Display.get_default();
        Gdk.Screen screen = (display != null) ?
                display.get_default_screen() : null;
        int screen_width = 0;

        if (screen != null) {
            screen_width = screen.get_width();
        }

        if (screen_width > 0 && max_label_width > (screen_width / 4)) {
            max_label_width = screen_width / 4;
        }

        /* add random padding */
        max_label_width += 20;
        set_default_size(max_label_width, -1);
    }

    private void next_engine() {
        if (m_selected_engine == m_engines.length - 1)
            m_selected_engine = 0;
        else
            m_selected_engine ++;
        m_label.set_text(m_buttons[m_selected_engine].longname);
        set_focus(m_buttons[m_selected_engine]);
    }

    private void previous_engine() {
        if (m_selected_engine == 0)
            m_selected_engine = m_engines.length - 1;
        else
            m_selected_engine --;
        m_label.set_text(m_buttons[m_selected_engine].longname);
        set_focus(m_buttons[m_selected_engine]);
    }

    private void restore_window_position(string debug_str) {
        debug("restore_window_position %s: (%ld, %ld)\n",
                debug_str, m_root_x, m_root_y);

        if (m_popup_delay_time_id == 0) {
            return;
        }

        GLib.Source.remove(m_popup_delay_time_id);
        m_popup_delay_time_id = 0;
        move(m_root_x, m_root_y);
    }

    /* override virtual functions */
    public override void show() {
        base.show();
        set_focus_visible(true);
    }

    public override bool key_press_event(Gdk.EventKey e) {
        bool retval = true;
        Gdk.EventKey *pe = &e;

        if (m_popup_delay_time > 0) {
            restore_window_position("pressed");
        }

        do {
            uint modifiers = KeybindingManager.MODIFIER_FILTER & pe->state;

            if ((modifiers != m_modifiers) &&
                (modifiers != (m_modifiers | Gdk.ModifierType.SHIFT_MASK))) {
                break;
            }

            if (pe->keyval == m_keyval) {
                if (modifiers == m_modifiers)
                    next_engine();
                else // modififers == m_modifiers | SHIFT_MASK
                    previous_engine();
                break;
            }

            switch (pe->keyval) {
                case 0x08fb: /* leftarrow */
                case 0xff51: /* Left */
                    previous_engine();
                    break;
                case 0x08fc: /* uparrow */
                case 0xff52: /* Up */
                    break;
                case 0x08fd: /* rightarrow */
                case 0xff53: /* Right */
                    next_engine();
                    break;
                case 0x08fe: /* downarrow */
                case 0xff54: /* Down */
                    break;
                default:
                    debug("0x%04x", pe->keyval);
                    break;
            }
        } while (false);
        return retval;
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

        if (m_popup_delay_time > 0) {
            if (m_popup_delay_time_id != 0) {
                GLib.Source.remove(m_popup_delay_time_id);
                m_popup_delay_time_id = 0;
            }
        }

        m_loop.quit();
        m_result = (int)m_selected_engine;
        return true;
    }

    public void set_popup_delay_time(uint popup_delay_time) {
        m_popup_delay_time = popup_delay_time;
    }
}
