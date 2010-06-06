using IBus;

class TestEngine : Engine {
    public override bool process_key_event (uint keyval, uint keycode, uint state) {
        stdout.printf("process_key_event(%u, %u, 0x%x)\n", keyval, keycode, state);
        return true;
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
