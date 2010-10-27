/* vim:set et sts=4 ai: */
using Vala;
using GLib;
using IBus;

class MemoryConfig : ConfigService {
    private HashMap<string, HashMap<string, Variant>> values;

    construct {
        values = new HashMap<string, HashMap<string, Variant>> (str_hash, str_equal);
    }

    public override bool set_value (string section,
                                    string name,
                                    Variant _value) {
        if (!values.contains (section))
            values[section] = new HashMap<string, Variant> (str_hash, str_equal);
        values[section][name] = _value;
        value_changed (section, name, _value);
        return true;
    }

    public override Variant get_value (string section,
                                               string name) throws GLib.Error {
        if (!values.contains (section) || !values[section].contains(name))
            throw new DBusError.FAILED("Can not get value %s", name);
        return values[section][name];
    }

    public static void main (string []argv) {
        var bus = new IBus.Bus();

        if (!bus.is_connected ()) {
            stderr.printf ("Can not connect to ibus-daemon!\n");
            return;
        }

        Type type = typeof (MemoryConfig);
        ConfigService config =
            (ConfigService) GLib.Object.new (type,
                                             "connection", bus.get_connection (),
                                             "object-path", "/org/freedesktop/IBus/Config");
        bus.request_name ("org.freedesktop.IBus.Config", 0);
        IBus.main ();
        config = null;
    }

}
