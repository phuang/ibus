using GLib;
using Enchant;
using IBus;

class TestEngine : Engine {
    // static values
    private static Broker broker = new Broker ();
    private static unowned Dict dict = broker.request_dict("en");

    private StringBuilder text = new StringBuilder();
    private LookupTable table = new LookupTable (6, 0, true, true);

    public override bool process_key_event (uint keyval, uint keycode, uint state) {
        // ignore release event
        if ((ModifierType.RELEASE_MASK & state) != 0)
            return false;

        bool retval = (text.len != 0);
        // process letter key events
        if ((keyval >= IBus.a && keyval <= IBus.z) ||
            (keyval >= IBus.A && keyval <= IBus.Z)) {
            char c = (char)keyval;
            text.append_c(c);
            retval = true;
        }
        // process return and space key event
        else if (keyval == IBus.Return || keyval == IBus.space) {
            // ignore if text is empty
            if (text.len != 0) {
                commit_text (new Text.from_string(text.str));
                text.erase();
                retval = true;
            }
        }
        // process backspace
        else if (keyval == IBus.BackSpace) {
            if (text.len != 0) {
                text.truncate (text.len - 1);
                retval = true;
            }
        }
        // process escape key event
        else if (keyval == IBus.Escape) {
            if (text.len > 0) {
                text.erase();
                retval = true;
            }
        }
        if (retval) {
            update_preedit_text ();
            update_lookup_table ();
        }
        return retval;
    }

    private new void update_preedit_text() {
        if (text.len == 0) {
            hide_preedit_text ();
        }
        else {
            var tmp = new Text.from_string(text.str);
            tmp.append_attribute(AttrType.BACKGROUND, 0x00a0a0a0, 0, -1);
            base.update_preedit_text(tmp, (uint)text.len, true);
        }
    }

    private new void update_lookup_table () {
        if (text.len == 0) {
            hide_lookup_table ();
        }
        else {
            var words = dict.suggest(text.str);
            table.clear ();
            foreach (string s in words)
                table.append_candidate(new Text.from_string(s));
            base.update_lookup_table (table, true);
        }
    }
}

void main (string []argv) {
    var bus = new Bus();
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
