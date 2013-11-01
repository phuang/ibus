[CCode (cprefix = "IBusCompose", gir_namespace = "IBus", gir_version = "1.0", lower_case_cprefix = "ibus_compose_")]
namespace IBusCompose {
	[Compact]
	[CCode (cname = "IBusComposeTable", destroy_function = "", cheader_filename = "ibuscomposetable.h")]
	public struct Table {
		public const uint16[] data;
		public int max_seq_len;
		public int n_seqs;
	}
	[CCode (cname = "ibus_compose_seqs_el_gr", cheader_filename = "ibuscomposetable.h")]
        public const uint16[] seqs_el_gr;
	[CCode (cname = "ibus_compose_table_el_gr", cheader_filename = "ibuscomposetable.h")]
	public const Table table_el_gr;
	[CCode (cname = "ibus_compose_seqs_fi_fi", cheader_filename = "ibuscomposetable.h")]
        public const uint16[] seqs_fi_fi;
	[CCode (cname = "ibus_compose_table_fi_fi", cheader_filename = "ibuscomposetable.h")]
	public const Table table_fi_fi;
	[CCode (cname = "ibus_compose_seqs_pt_br", cheader_filename = "ibuscomposetable.h")]
        public const uint16[] seqs_pt_br;
	[CCode (cname = "ibus_compose_table_pt_br", cheader_filename = "ibuscomposetable.h")]
	public const Table table_pt_br;
}
