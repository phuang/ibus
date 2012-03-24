namespace IBus {
	// For some reason, ibus_text_new_from_static_string is hidden in GIR
	// https://github.com/ibus/ibus/commit/37e6e587
	[CCode (type_id = "ibus_text_get_type ()", cheader_filename = "ibus.h")]
	public class Text : IBus.Serializable {
		[CCode (cname = "ibus_text_new_from_static_string", has_construct_function = false)]
		public Text.from_static_string (string str);
	}
}
