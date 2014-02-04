/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
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

using IBus;
using IBusCompose;
using Gtk;

class SendKeyEventsWindow : Gtk.Window {
    private IBus.Bus m_bus;
    private IBus.Engine m_engine;
    private uint16* m_test_compose_table;
    private int m_test_compose_table_length;
    public int m_index_stride = 0;

    public SendKeyEventsWindow() {
        /* Call ibus_bus_new() before gtk_widget_show_all() so that
         * ibus_bus_new_async() is not called in GTK+ IM modules. */
        ibus_init();

        Gtk.Entry entry = new Gtk.Entry();
        entry.focus_in_event.connect((w, e) => {
                m_bus.set_global_engine_async.begin("xkbtest:us::eng",
                                                    -1, null, set_engine_cb);
                return false;
        });

        int stride = 0;
        int n_loop = 0;

        entry.get_buffer().inserted_text.connect((w, p, s, l) => {
                if (m_test_compose_table == null)
                    return;

                /* entry.set_text("") calls this callback. */
                if (n_loop % 2 == 1) {
                    n_loop = 0;
                    return;
                }

                unichar code = s.get_char(0);
                int i = stride + (m_index_stride - 1);

                print("%05d %s expected: %04X typed: %04X\n",
                      i,
                      m_test_compose_table[i] == code ? "OK" : "NG",
                      m_test_compose_table[i],
                      code);

                stride += m_index_stride;
                n_loop++;
                entry.set_text("");
        });
        add(entry);
    }

    public void run_ibus_engine() {
        if (!m_bus.is_connected()) {
            warning("ibus-daemon does not exist.");
            return;
        }

        m_bus.disconnected.connect((bus) => {
            debug("bus disconnected");
            IBus.quit();
        });

        IBus.Factory factory = new IBus.Factory(m_bus.get_connection());
   
        int id = 0;

        factory.create_engine.connect((factory, name) => {
            const string path = "/org/freedesktop/IBus/engine/simpletest/%d";
            m_engine = new IBus.Engine.with_type(
                    typeof(IBus.EngineSimple), name,
                    path.printf(id++), m_bus.get_connection());

            string lang = Intl.setlocale(LocaleCategory.CTYPE, null);

            if (lang == null)
                lang = "C";

            IBus.EngineSimple simple = (IBus.EngineSimple) m_engine;

            if (lang.ascii_ncasecmp("el_gr", "el_gr".length) == 0) {
                simple.add_table_by_locale(lang);
                m_test_compose_table = (uint16 *) IBusCompose.seqs_el_gr;
                m_test_compose_table_length = IBusCompose.seqs_el_gr.length;
                m_index_stride = IBusCompose.table_el_gr.max_seq_len + 2;
            }
            else if (lang.ascii_ncasecmp("fi_fi", "fi_fi".length) == 0) {
                simple.add_table_by_locale(lang);
                m_test_compose_table = (uint16 *) IBusCompose.seqs_fi_fi;
                m_test_compose_table_length = IBusCompose.seqs_fi_fi.length;
                m_index_stride = IBusCompose.table_fi_fi.max_seq_len + 2;
            }
            else if (lang.ascii_ncasecmp("pt_br", "pt_br".length) == 0) {
                simple.add_table_by_locale(lang);
                m_test_compose_table = (uint16 *) IBusCompose.seqs_pt_br;
                m_test_compose_table_length = IBusCompose.seqs_pt_br.length;
                m_index_stride = IBusCompose.table_pt_br.max_seq_len + 2;
            }
            else {
                warning("Run no test. Need another $LANG.");
            }
            return m_engine;
        });

        var component = new IBus.Component(
                "org.freedesktop.IBus.SimpleTest",
                "Simple Engine Test",
                "0.0.1",
                "GPL",
                "Takao Fujiwara <takao.fujiwara1@gmail.com>",
                "http://code.google.com/p/ibus/",
                "",
                "ibus");

        component.add_engine (new IBus.EngineDesc(
                "xkbtest:us::eng",
                "XKB Test",
                "XKB Test",
                "en",
                "GPL",
                "Takao Fujiwara <takao.fujiwara1@gmail.com>",
                "ibus-engine",
                "us"));

        m_bus.register_component(component);
    }

    private void ibus_init() {
        IBus.init();

        m_bus = new IBus.Bus();
    }

    private void set_engine_cb(GLib.Object? object, GLib.AsyncResult res) {
        try {
            m_bus.set_global_engine_async_finish(res);
        } catch(GLib.Error error) {
            warning("%s", error.message);
            return;
        }

        for (int i = 0; i < m_test_compose_table_length; i += m_index_stride) {
            for (int j = i; j < i + (m_index_stride - 1); j++) {
                uint keyval = m_test_compose_table[j];
                uint keycode = 0;
                uint modifiers = 0;

                if (keyval == 0)
                    break;

                m_engine.process_key_event (keyval, keycode, modifiers);
                modifiers |= IBus.ModifierType.RELEASE_MASK;
                m_engine.process_key_event (keyval, keycode, modifiers);
            }
        }
    }

    public static int main(string[] args) {
        Intl.setlocale(LocaleCategory.ALL, "");

        Gtk.init(ref args);

        var window = new SendKeyEventsWindow();
        window.destroy.connect (Gtk.main_quit);
        window.show_all();
        window.run_ibus_engine();
        Gtk.main();
        return 0;
    }
}
