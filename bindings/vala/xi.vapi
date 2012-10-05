[CCode (cprefix = "", lower_case_cprefix = "", cheader_filename = "X11/extensions/XInput2.h")]
namespace XI {
    public enum EventType {
        [CCode (cname = "XI_KeyPress")]
        KeyPress,
        [CCode (cname = "XI_KeyRelease")]
        KeyRelease,
    }

    [CCode (cname = "XIAllMasterDevices")]
    public const int AllMasterDevices;

    [CCode (cname = "XI_LASTEVENT")]
    public const int LASTEVENT;

    [Compact]
    [CCode (cname = "XIEventMask", destroy_function = "")]
    public struct EventMask {
        public int deviceid;
        [CCode (array_length_cname = "mask_len")]
        public uchar[] mask;
    }

    [CCode (cname = "XISetMask")]
    public void set_mask(void *mask, EventType type);

    [Compact]
    [CCode (cname = "XIGrabModifiers", destroy_function = "")]
    public struct GrabModifiers {
        public int modifiers;
        public int status;
    }

    [CCode (cname = "XIGrabKeycode")]
    public int grab_keycode (X.Display display,
                             int deviceid,
                             int keycode,
                             X.Window grab_window,
                             int grab_mode,
                             int paired_device_mode,
                             bool owner_events,
                             XI.EventMask mask,
                             [CCode (array_length_pos = 8.9)]
                             XI.GrabModifiers[] modifiers);

    [CCode (cname = "XIUngrabKeycode")]
    public int ungrab_keycode (X.Display display,
                               int deviceid,
                               int keycode,
                               X.Window grab_window,
                               [CCode (array_length_pos = 4.9)]
                               XI.GrabModifiers[] modifiers);
}
