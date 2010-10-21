/* vim:set et sts=4 ai: */
using GLib;
using Enchant;
using IBus;

class TestEngine : Engine {
    // static values
    private static Broker broker = new Broker ();
    private static unowned Dict dict = broker.request_dict("en");

    // const values
    private const uint PREEDIT_BGCOLOR = 0x00a0a0a0;

    private StringBuilder text = new StringBuilder();
    private LookupTable table = new LookupTable (6, 0, true, true);
    private string[] words;

    enum UpdateType {
        TEXT,
        LOOKUP_TABLE_CURSOR,
    }

    // override process_key_event to handle key events
    public override bool process_key_event (uint keyval, uint keycode, uint state) {
        // ignore release event
        if ((ModifierType.RELEASE_MASK & state) != 0)
            return false;

        bool retval = (text.len != 0);
        UpdateType update = 0;
        // process letter key events
        if ((keyval >= IBus.a && keyval <= IBus.z) ||
            (keyval >= IBus.A && keyval <= IBus.Z)) {
            char c = (char)keyval;
            text.append_c(c);
            update = UpdateType.TEXT;
        }
        // process return and space key event
        else if (keyval == IBus.Return || keyval == IBus.space) {
            // ignore if text is empty
            if (text.len != 0) {
                commit_text (new Text.from_string(text.str));
                text.erase();
                update = UpdateType.TEXT;
            }
        }
        // process backspace
        else if (keyval == IBus.BackSpace) {
            if (text.len != 0) {
                text.truncate (text.len - 1);
                update = UpdateType.TEXT;
            }
        }
        // process arrow & page key events
        else if (keyval == IBus.Up) {
            if (text.len != 0) {
                if (table.cursor_up ())
                    update = UpdateType.LOOKUP_TABLE_CURSOR;
            }
        }
        else if (keyval == IBus.Down) {
            if (text.len != 0) {
                if (table.cursor_down ())
                    update = UpdateType.LOOKUP_TABLE_CURSOR;
            }
        }
        else if (keyval == IBus.Page_Up) {
            if (text.len != 0) {
                if (table.page_up ())
                    update = UpdateType.LOOKUP_TABLE_CURSOR;
            }
        }
        else if (keyval == IBus.Page_Down) {
            if (text.len != 0) {
                if (table.page_down ())
                    update = UpdateType.LOOKUP_TABLE_CURSOR;
            }
        }
        // process escape key event
        else if (keyval == IBus.Escape) {
            if (text.len > 0) {
                text.erase();
                update = UpdateType.TEXT;
            }
        }

        // text has been updated
        if (update == UpdateType.TEXT) {
            update_preedit_text ();
            update_lookup_table ();
            update_auxiliary_text ();
        }
        // only lookup table cursor
        else if (update == UpdateType.LOOKUP_TABLE_CURSOR) {
            base.update_lookup_table (table, true);
            update_auxiliary_text ();
        }

        return retval;
    }

    // update preedit text
    private new void update_preedit_text() {
        if (text.len == 0) {
            hide_preedit_text ();
        }
        else {
            var tmp = new Text.from_string(text.str);
            tmp.append_attribute(AttrType.BACKGROUND, PREEDIT_BGCOLOR, 0, -1);
            base.update_preedit_text(tmp, (uint)text.len, true);
        }
    }

	// update lookup table
    private new void update_lookup_table () {
        if (text.len == 0) {
            words = null;
            hide_lookup_table ();
        }
        else {
            words = dict.suggest(text.str);
            table.clear ();
            foreach (string s in words)
                table.append_candidate(new Text.from_string(s));
            base.update_lookup_table (table, true);
        }
    }

    // update auxiliary text
    private new void update_auxiliary_text () {
        if (text.len == 0 || words == null) {
            hide_auxiliary_text ();
        }
        else {
            uint i = table.get_cursor_pos ();
            base.update_auxiliary_text (new Text.from_string(words[i]), true);
        }
    }

    public static void main (string []argv) {
        var bus = new IBus.Bus();

        if (!bus.is_connected ()) {
            stderr.printf ("Can not connect to ibus-daemon!\n");
            return;
        }

        var factory = new Factory(bus.get_connection());
        factory.add_engine("vala-debug", typeof(TestEngine));
        var component = new Component (
                                "org.freedesktop.IBus.Vala",
                                "ValaTest", "0.0.1", "GPL",
                                "Peng Huang <shawn.p.huang@gmail.com>",
                                "http://code.google.com/p/ibus/",
                                "",
                                "ibus-vala");
        var engine = new EngineDesc ("vala-debug",
                                     "Vala (debug)",
                                     "Vala demo input method",
                                     "zh_CN",
                                     "GPL",
                                     "Peng Huang <shawn.p.huang@gmail.com>",
                                     "",
                                     "us");
        component.add_engine (engine);
        bus.register_component (component);
        IBus.main ();
    }
}
