<?xml version="1.0"?>
<api version="1.0">
	<namespace name="IBus">
		<function name="attr_background_new" symbol="ibus_attr_background_new">
			<return-type type="IBusAttribute*"/>
			<parameters>
				<parameter name="color" type="guint"/>
				<parameter name="start_index" type="guint"/>
				<parameter name="end_index" type="guint"/>
			</parameters>
		</function>
		<function name="attr_foreground_new" symbol="ibus_attr_foreground_new">
			<return-type type="IBusAttribute*"/>
			<parameters>
				<parameter name="color" type="guint"/>
				<parameter name="start_index" type="guint"/>
				<parameter name="end_index" type="guint"/>
			</parameters>
		</function>
		<function name="attr_underline_new" symbol="ibus_attr_underline_new">
			<return-type type="IBusAttribute*"/>
			<parameters>
				<parameter name="underline_type" type="guint"/>
				<parameter name="start_index" type="guint"/>
				<parameter name="end_index" type="guint"/>
			</parameters>
		</function>
		<function name="dbus_connection_setup" symbol="ibus_dbus_connection_setup">
			<return-type type="void"/>
			<parameters>
				<parameter name="connection" type="DBusConnection*"/>
			</parameters>
		</function>
		<function name="dbus_server_setup" symbol="ibus_dbus_server_setup">
			<return-type type="void"/>
			<parameters>
				<parameter name="server" type="DBusServer*"/>
			</parameters>
		</function>
		<function name="free_strv" symbol="ibus_free_strv">
			<return-type type="void"/>
			<parameters>
				<parameter name="strv" type="gchar**"/>
			</parameters>
		</function>
		<function name="get_address" symbol="ibus_get_address">
			<return-type type="gchar*"/>
		</function>
		<function name="get_daemon_uid" symbol="ibus_get_daemon_uid">
			<return-type type="glong"/>
		</function>
		<function name="get_local_machine_id" symbol="ibus_get_local_machine_id">
			<return-type type="gchar*"/>
		</function>
		<function name="get_socket_path" symbol="ibus_get_socket_path">
			<return-type type="gchar*"/>
		</function>
		<function name="get_user_name" symbol="ibus_get_user_name">
			<return-type type="gchar*"/>
		</function>
		<function name="init" symbol="ibus_init">
			<return-type type="void"/>
		</function>
		<function name="key_event_from_string" symbol="ibus_key_event_from_string">
			<return-type type="gboolean"/>
			<parameters>
				<parameter name="string" type="gchar*"/>
				<parameter name="keyval" type="guint*"/>
				<parameter name="modifiers" type="guint*"/>
			</parameters>
		</function>
		<function name="key_event_to_string" symbol="ibus_key_event_to_string">
			<return-type type="gchar*"/>
			<parameters>
				<parameter name="keyval" type="guint"/>
				<parameter name="modifiers" type="guint"/>
			</parameters>
		</function>
		<function name="keyval_from_name" symbol="ibus_keyval_from_name">
			<return-type type="guint"/>
			<parameters>
				<parameter name="keyval_name" type="gchar*"/>
			</parameters>
		</function>
		<function name="keyval_name" symbol="ibus_keyval_name">
			<return-type type="gchar*"/>
			<parameters>
				<parameter name="keyval" type="guint"/>
			</parameters>
		</function>
		<function name="main" symbol="ibus_main">
			<return-type type="void"/>
		</function>
		<function name="mainloop_setup" symbol="ibus_mainloop_setup">
			<return-type type="void"/>
			<parameters>
				<parameter name="connection_func" type="DBusConnectionSetupFunc"/>
				<parameter name="server_func" type="DBusServerSetupFunc"/>
				<parameter name="user_data" type="gpointer"/>
			</parameters>
		</function>
		<function name="quit" symbol="ibus_quit">
			<return-type type="void"/>
		</function>
		<function name="set_display" symbol="ibus_set_display">
			<return-type type="void"/>
			<parameters>
				<parameter name="display" type="gchar*"/>
			</parameters>
		</function>
		<function name="type_get_array" symbol="ibus_type_get_array">
			<return-type type="GType"/>
		</function>
		<function name="type_get_dict_entry" symbol="ibus_type_get_dict_entry">
			<return-type type="GType"/>
		</function>
		<function name="type_get_object_path" symbol="ibus_type_get_object_path">
			<return-type type="GType"/>
		</function>
		<function name="type_get_struct" symbol="ibus_type_get_struct">
			<return-type type="GType"/>
		</function>
		<function name="type_get_variant" symbol="ibus_type_get_variant">
			<return-type type="GType"/>
		</function>
		<function name="write_address" symbol="ibus_write_address">
			<return-type type="void"/>
			<parameters>
				<parameter name="address" type="gchar*"/>
			</parameters>
		</function>
		<function name="xml_free" symbol="ibus_xml_free">
			<return-type type="void"/>
			<parameters>
				<parameter name="node" type="XMLNode*"/>
			</parameters>
		</function>
		<function name="xml_output" symbol="ibus_xml_output">
			<return-type type="void"/>
			<parameters>
				<parameter name="node" type="XMLNode*"/>
				<parameter name="output" type="GString*"/>
			</parameters>
		</function>
		<function name="xml_parse_buffer" symbol="ibus_xml_parse_buffer">
			<return-type type="XMLNode*"/>
			<parameters>
				<parameter name="buffer" type="gchar*"/>
			</parameters>
		</function>
		<function name="xml_parse_file" symbol="ibus_xml_parse_file">
			<return-type type="XMLNode*"/>
			<parameters>
				<parameter name="name" type="gchar*"/>
			</parameters>
		</function>
		<callback name="DBusConnectionSetupFunc">
			<return-type type="void"/>
			<parameters>
				<parameter name="connection" type="DBusConnection*"/>
				<parameter name="user_data" type="gpointer"/>
			</parameters>
		</callback>
		<callback name="DBusServerSetupFunc">
			<return-type type="void"/>
			<parameters>
				<parameter name="server" type="DBusServer*"/>
				<parameter name="user_data" type="gpointer"/>
			</parameters>
		</callback>
		<callback name="IBusConnectionReplyFunc">
			<return-type type="void"/>
			<parameters>
				<parameter name="connection" type="IBusConnection*"/>
				<parameter name="reply" type="IBusMessage*"/>
				<parameter name="user_data" type="gpointer"/>
			</parameters>
		</callback>
		<callback name="IBusFreeFunc">
			<return-type type="void"/>
			<parameters>
				<parameter name="object" type="gpointer"/>
			</parameters>
		</callback>
		<callback name="IBusIBusMessageFunc">
			<return-type type="gboolean"/>
			<parameters>
				<parameter name="connection" type="IBusConnection*"/>
				<parameter name="message" type="IBusMessage*"/>
			</parameters>
		</callback>
		<callback name="IBusIBusSignalFunc">
			<return-type type="gboolean"/>
			<parameters>
				<parameter name="connection" type="IBusConnection*"/>
				<parameter name="message" type="IBusMessage*"/>
			</parameters>
		</callback>
		<callback name="IBusMessageFunc">
			<return-type type="gboolean"/>
			<parameters>
				<parameter name="connection" type="IBusConnection*"/>
				<parameter name="message" type="IBusMessage*"/>
				<parameter name="user_data" type="gpointer"/>
			</parameters>
		</callback>
		<callback name="IBusNewConnectionFunc">
			<return-type type="void"/>
			<parameters>
				<parameter name="server" type="IBusServer*"/>
				<parameter name="connection" type="IBusConnection*"/>
			</parameters>
		</callback>
		<callback name="IBusObjectDestroyFunc">
			<return-type type="void"/>
			<parameters>
				<parameter name="p1" type="IBusObject*"/>
			</parameters>
		</callback>
		<callback name="IBusPendingCallNotifyFunction">
			<return-type type="void"/>
			<parameters>
				<parameter name="pending" type="IBusPendingCall*"/>
				<parameter name="user_data" type="gpointer"/>
			</parameters>
		</callback>
		<callback name="IBusSerializableCopyFunc">
			<return-type type="gboolean"/>
			<parameters>
				<parameter name="dest" type="IBusSerializable*"/>
				<parameter name="src" type="IBusSerializable*"/>
			</parameters>
		</callback>
		<callback name="IBusSerializableDeserializeFunc">
			<return-type type="gboolean"/>
			<parameters>
				<parameter name="object" type="IBusSerializable*"/>
				<parameter name="iter" type="IBusMessageIter*"/>
			</parameters>
		</callback>
		<callback name="IBusSerializableSerializeFunc">
			<return-type type="gboolean"/>
			<parameters>
				<parameter name="object" type="IBusSerializable*"/>
				<parameter name="iter" type="IBusMessageIter*"/>
			</parameters>
		</callback>
		<callback name="ServiceIBusMessageFunc">
			<return-type type="gboolean"/>
			<parameters>
				<parameter name="service" type="IBusService*"/>
				<parameter name="connection" type="IBusConnection*"/>
				<parameter name="message" type="IBusMessage*"/>
			</parameters>
		</callback>
		<callback name="ServiceIBusSignalFunc">
			<return-type type="gboolean"/>
			<parameters>
				<parameter name="service" type="IBusService*"/>
				<parameter name="connection" type="IBusConnection*"/>
				<parameter name="message" type="IBusMessage*"/>
			</parameters>
		</callback>
		<struct name="BusComponent">
		</struct>
		<struct name="DBusConnection">
		</struct>
		<struct name="DBusError">
		</struct>
		<struct name="DBusMessage">
		</struct>
		<struct name="DBusMessageIter">
		</struct>
		<struct name="DBusPendingCall">
		</struct>
		<struct name="DBusServer">
		</struct>
		<struct name="IBusError">
			<method name="free" symbol="ibus_error_free">
				<return-type type="void"/>
				<parameters>
					<parameter name="error" type="IBusError*"/>
				</parameters>
			</method>
			<method name="new" symbol="ibus_error_new">
				<return-type type="IBusError*"/>
			</method>
			<method name="new_from_message" symbol="ibus_error_new_from_message">
				<return-type type="IBusError*"/>
				<parameters>
					<parameter name="message" type="DBusMessage*"/>
				</parameters>
			</method>
			<method name="new_from_printf" symbol="ibus_error_new_from_printf">
				<return-type type="IBusError*"/>
				<parameters>
					<parameter name="name" type="gchar*"/>
					<parameter name="format_message" type="gchar*"/>
				</parameters>
			</method>
			<method name="new_from_text" symbol="ibus_error_new_from_text">
				<return-type type="IBusError*"/>
				<parameters>
					<parameter name="name" type="gchar*"/>
					<parameter name="message" type="gchar*"/>
				</parameters>
			</method>
		</struct>
		<struct name="IBusMessage">
			<method name="append_args" symbol="ibus_message_append_args">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="first_arg_type" type="GType"/>
				</parameters>
			</method>
			<method name="append_args_valist" symbol="ibus_message_append_args_valist">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="first_arg_type" type="GType"/>
					<parameter name="va_args" type="va_list"/>
				</parameters>
			</method>
			<method name="get_args" symbol="ibus_message_get_args">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="error" type="IBusError**"/>
					<parameter name="first_arg_type" type="GType"/>
				</parameters>
			</method>
			<method name="get_args_valist" symbol="ibus_message_get_args_valist">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="error" type="IBusError**"/>
					<parameter name="first_arg_type" type="GType"/>
					<parameter name="va_args" type="va_list"/>
				</parameters>
			</method>
			<method name="get_destination" symbol="ibus_message_get_destination">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="get_error_message" symbol="ibus_message_get_error_message">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="get_error_name" symbol="ibus_message_get_error_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="get_interface" symbol="ibus_message_get_interface">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="get_member" symbol="ibus_message_get_member">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="get_no_reply" symbol="ibus_message_get_no_reply">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="get_path" symbol="ibus_message_get_path">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="get_reply_serial" symbol="ibus_message_get_reply_serial">
				<return-type type="guint32"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="get_sender" symbol="ibus_message_get_sender">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="get_serial" symbol="ibus_message_get_serial">
				<return-type type="guint32"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="is_error" symbol="ibus_message_is_error">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="error_name" type="gchar*"/>
				</parameters>
			</method>
			<method name="is_method_call" symbol="ibus_message_is_method_call">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="interface" type="gchar*"/>
					<parameter name="method" type="gchar*"/>
				</parameters>
			</method>
			<method name="is_signal" symbol="ibus_message_is_signal">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="interface" type="gchar*"/>
					<parameter name="signal_name" type="gchar*"/>
				</parameters>
			</method>
			<method name="new" symbol="ibus_message_new">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="message_type" type="gint"/>
				</parameters>
			</method>
			<method name="new_error" symbol="ibus_message_new_error">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="reply_to" type="IBusMessage*"/>
					<parameter name="error_name" type="gchar*"/>
					<parameter name="error_message" type="gchar*"/>
				</parameters>
			</method>
			<method name="new_error_printf" symbol="ibus_message_new_error_printf">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="reply_to" type="IBusMessage*"/>
					<parameter name="error_name" type="gchar*"/>
					<parameter name="error_format" type="gchar*"/>
				</parameters>
			</method>
			<method name="new_method_call" symbol="ibus_message_new_method_call">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="destination" type="gchar*"/>
					<parameter name="path" type="gchar*"/>
					<parameter name="interface" type="gchar*"/>
					<parameter name="method" type="gchar*"/>
				</parameters>
			</method>
			<method name="new_method_return" symbol="ibus_message_new_method_return">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="reply_to" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="new_signal" symbol="ibus_message_new_signal">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="path" type="gchar*"/>
					<parameter name="interface" type="gchar*"/>
					<parameter name="method" type="gchar*"/>
				</parameters>
			</method>
			<method name="ref" symbol="ibus_message_ref">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="set_destination" symbol="ibus_message_set_destination">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="destination" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_error_name" symbol="ibus_message_set_error_name">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="error_name" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_interface" symbol="ibus_message_set_interface">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="interface" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_member" symbol="ibus_message_set_member">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="member" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_no_reply" symbol="ibus_message_set_no_reply">
				<return-type type="void"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="no_reply" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_path" symbol="ibus_message_set_path">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="path" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_reply_serial" symbol="ibus_message_set_reply_serial">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="reply_serial" type="guint32"/>
				</parameters>
			</method>
			<method name="set_sender" symbol="ibus_message_set_sender">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="sender" type="gchar*"/>
				</parameters>
			</method>
			<method name="to_string" symbol="ibus_message_to_string">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="unref" symbol="ibus_message_unref">
				<return-type type="void"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
		</struct>
		<struct name="IBusMessageIter">
			<method name="append" symbol="ibus_message_iter_append">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="iter" type="IBusMessageIter*"/>
					<parameter name="type" type="GType"/>
					<parameter name="value" type="gconstpointer"/>
				</parameters>
			</method>
			<method name="close_container" symbol="ibus_message_iter_close_container">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="iter" type="IBusMessageIter*"/>
					<parameter name="sub" type="IBusMessageIter*"/>
				</parameters>
			</method>
			<method name="copy_data" symbol="ibus_message_iter_copy_data">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="dst" type="IBusMessageIter*"/>
					<parameter name="src" type="IBusMessageIter*"/>
				</parameters>
			</method>
			<method name="get" symbol="ibus_message_iter_get">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="iter" type="IBusMessageIter*"/>
					<parameter name="type" type="GType"/>
					<parameter name="value" type="gpointer"/>
				</parameters>
			</method>
			<method name="get_arg_type" symbol="ibus_message_iter_get_arg_type">
				<return-type type="GType"/>
				<parameters>
					<parameter name="iter" type="IBusMessageIter*"/>
				</parameters>
			</method>
			<method name="get_basic" symbol="ibus_message_iter_get_basic">
				<return-type type="void"/>
				<parameters>
					<parameter name="iter" type="IBusMessageIter*"/>
					<parameter name="value" type="gpointer"/>
				</parameters>
			</method>
			<method name="get_element_type" symbol="ibus_message_iter_get_element_type">
				<return-type type="GType"/>
				<parameters>
					<parameter name="iter" type="IBusMessageIter*"/>
				</parameters>
			</method>
			<method name="has_next" symbol="ibus_message_iter_has_next">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="iter" type="IBusMessageIter*"/>
				</parameters>
			</method>
			<method name="init" symbol="ibus_message_iter_init">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="iter" type="IBusMessageIter*"/>
				</parameters>
			</method>
			<method name="init_append" symbol="ibus_message_iter_init_append">
				<return-type type="void"/>
				<parameters>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="iter" type="IBusMessageIter*"/>
				</parameters>
			</method>
			<method name="next" symbol="ibus_message_iter_next">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="iter" type="IBusMessageIter*"/>
				</parameters>
			</method>
			<method name="open_container" symbol="ibus_message_iter_open_container">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="iter" type="IBusMessageIter*"/>
					<parameter name="type" type="GType"/>
					<parameter name="contained_signature" type="gchar*"/>
					<parameter name="sub" type="IBusMessageIter*"/>
				</parameters>
			</method>
			<method name="recurse" symbol="ibus_message_iter_recurse">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="iter" type="IBusMessageIter*"/>
					<parameter name="type" type="GType"/>
					<parameter name="sub" type="IBusMessageIter*"/>
				</parameters>
			</method>
		</struct>
		<struct name="IBusPendingCall">
			<method name="allocate_data_slot" symbol="ibus_pending_call_allocate_data_slot">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="slot_p" type="gint*"/>
				</parameters>
			</method>
			<method name="block" symbol="ibus_pending_call_block">
				<return-type type="void"/>
				<parameters>
					<parameter name="pending" type="IBusPendingCall*"/>
				</parameters>
			</method>
			<method name="cancel" symbol="ibus_pending_call_cancel">
				<return-type type="void"/>
				<parameters>
					<parameter name="pending" type="IBusPendingCall*"/>
				</parameters>
			</method>
			<method name="free_data_slot" symbol="ibus_pending_call_free_data_slot">
				<return-type type="void"/>
				<parameters>
					<parameter name="slot_p" type="gint*"/>
				</parameters>
			</method>
			<method name="get_completed" symbol="ibus_pending_call_get_completed">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="pending" type="IBusPendingCall*"/>
				</parameters>
			</method>
			<method name="get_data" symbol="ibus_pending_call_get_data">
				<return-type type="gpointer"/>
				<parameters>
					<parameter name="pending" type="IBusPendingCall*"/>
					<parameter name="slot" type="gint"/>
				</parameters>
			</method>
			<method name="ref" symbol="ibus_pending_call_ref">
				<return-type type="IBusPendingCall*"/>
				<parameters>
					<parameter name="pending" type="IBusPendingCall*"/>
				</parameters>
			</method>
			<method name="set_data" symbol="ibus_pending_call_set_data">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="pending" type="IBusPendingCall*"/>
					<parameter name="slot" type="gint"/>
					<parameter name="data" type="gpointer"/>
					<parameter name="free_data_func" type="GDestroyNotify"/>
				</parameters>
			</method>
			<method name="set_notify" symbol="ibus_pending_call_set_notify">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="pending" type="IBusPendingCall*"/>
					<parameter name="function" type="IBusPendingCallNotifyFunction"/>
					<parameter name="user_data" type="gpointer"/>
					<parameter name="free_user_data" type="GDestroyNotify"/>
				</parameters>
			</method>
			<method name="steal_reply" symbol="ibus_pending_call_steal_reply">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="pending" type="IBusPendingCall*"/>
				</parameters>
			</method>
			<method name="unref" symbol="ibus_pending_call_unref">
				<return-type type="void"/>
				<parameters>
					<parameter name="pending" type="IBusPendingCall*"/>
				</parameters>
			</method>
			<method name="wait" symbol="ibus_pending_call_wait">
				<return-type type="void"/>
				<parameters>
					<parameter name="pending" type="IBusPendingCall*"/>
				</parameters>
			</method>
		</struct>
		<struct name="IBusRectangle">
			<field name="x" type="gint"/>
			<field name="y" type="gint"/>
			<field name="width" type="gint"/>
			<field name="height" type="gint"/>
		</struct>
		<struct name="XMLNode">
			<field name="name" type="gchar*"/>
			<field name="text" type="gchar*"/>
			<field name="attributes" type="gchar**"/>
			<field name="sub_nodes" type="GList*"/>
		</struct>
		<enum name="IBusAttrType">
			<member name="IBUS_ATTR_TYPE_UNDERLINE" value="1"/>
			<member name="IBUS_ATTR_TYPE_FOREGROUND" value="2"/>
			<member name="IBUS_ATTR_TYPE_BACKGROUND" value="3"/>
		</enum>
		<enum name="IBusAttrUnderline">
			<member name="IBUS_ATTR_UNDERLINE_NONE" value="0"/>
			<member name="IBUS_ATTR_UNDERLINE_SINGLE" value="1"/>
			<member name="IBUS_ATTR_UNDERLINE_DOUBLE" value="2"/>
			<member name="IBUS_ATTR_UNDERLINE_LOW" value="3"/>
			<member name="IBUS_ATTR_UNDERLINE_ERROR" value="4"/>
		</enum>
		<enum name="IBusCapabilite">
			<member name="IBUS_CAP_PREEDIT_TEXT" value="1"/>
			<member name="IBUS_CAP_AUXILIARY_TEXT" value="2"/>
			<member name="IBUS_CAP_LOOKUP_TABLE" value="4"/>
			<member name="IBUS_CAP_FOCUS" value="8"/>
			<member name="IBUS_CAP_PROPERTY" value="16"/>
			<member name="IBUS_CAP_SURROUNDING_TEXT" value="32"/>
		</enum>
		<enum name="IBusModifierType">
			<member name="IBUS_SHIFT_MASK" value="1"/>
			<member name="IBUS_LOCK_MASK" value="2"/>
			<member name="IBUS_CONTROL_MASK" value="4"/>
			<member name="IBUS_MOD1_MASK" value="8"/>
			<member name="IBUS_MOD2_MASK" value="16"/>
			<member name="IBUS_MOD3_MASK" value="32"/>
			<member name="IBUS_MOD4_MASK" value="64"/>
			<member name="IBUS_MOD5_MASK" value="128"/>
			<member name="IBUS_BUTTON1_MASK" value="256"/>
			<member name="IBUS_BUTTON2_MASK" value="512"/>
			<member name="IBUS_BUTTON3_MASK" value="1024"/>
			<member name="IBUS_BUTTON4_MASK" value="2048"/>
			<member name="IBUS_BUTTON5_MASK" value="4096"/>
			<member name="IBUS_HANDLED_MASK" value="16777216"/>
			<member name="IBUS_FORWARD_MASK" value="33554432"/>
			<member name="IBUS_IGNORED_MASK" value="33554432"/>
			<member name="IBUS_SUPER_MASK" value="67108864"/>
			<member name="IBUS_HYPER_MASK" value="134217728"/>
			<member name="IBUS_META_MASK" value="268435456"/>
			<member name="IBUS_RELEASE_MASK" value="1073741824"/>
			<member name="IBUS_MODIFIER_MASK" value="1593843711"/>
		</enum>
		<enum name="IBusObjectFlags">
			<member name="IBUS_IN_DESTRUCTION" value="1"/>
			<member name="IBUS_DESTROYED" value="2"/>
			<member name="IBUS_RESERVED_1" value="4"/>
			<member name="IBUS_RESERVED_2" value="8"/>
		</enum>
		<enum name="IBusOrientation">
			<member name="IBUS_ORIENTATION_HORIZONTAL" value="0"/>
			<member name="IBUS_ORIENTATION_VERTICAL" value="1"/>
			<member name="IBUS_ORIENTATION_SYSTEM" value="2"/>
		</enum>
		<enum name="IBusPreeditFocusMode">
			<member name="IBUS_ENGINE_PREEDIT_CLEAR" value="0"/>
			<member name="IBUS_ENGINE_PREEDIT_COMMIT" value="1"/>
		</enum>
		<enum name="IBusPropState">
			<member name="PROP_STATE_UNCHECKED" value="0"/>
			<member name="PROP_STATE_CHECKED" value="1"/>
			<member name="PROP_STATE_INCONSISTENT" value="2"/>
		</enum>
		<enum name="IBusPropType">
			<member name="PROP_TYPE_NORMAL" value="0"/>
			<member name="PROP_TYPE_TOGGLE" value="1"/>
			<member name="PROP_TYPE_RADIO" value="2"/>
			<member name="PROP_TYPE_MENU" value="3"/>
			<member name="PROP_TYPE_SEPARATOR" value="4"/>
		</enum>
		<object name="IBusAttrList" parent="IBusSerializable" type-name="IBusAttrList" get-type="ibus_attr_list_get_type">
			<method name="append" symbol="ibus_attr_list_append">
				<return-type type="void"/>
				<parameters>
					<parameter name="attr_list" type="IBusAttrList*"/>
					<parameter name="attr" type="IBusAttribute*"/>
				</parameters>
			</method>
			<method name="get" symbol="ibus_attr_list_get">
				<return-type type="IBusAttribute*"/>
				<parameters>
					<parameter name="attr_list" type="IBusAttrList*"/>
					<parameter name="index" type="guint"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_attr_list_new">
				<return-type type="IBusAttrList*"/>
			</constructor>
			<field name="attributes" type="GArray*"/>
		</object>
		<object name="IBusAttribute" parent="IBusSerializable" type-name="IBusAttribute" get-type="ibus_attribute_get_type">
			<constructor name="new" symbol="ibus_attribute_new">
				<return-type type="IBusAttribute*"/>
				<parameters>
					<parameter name="type" type="guint"/>
					<parameter name="value" type="guint"/>
					<parameter name="start_index" type="guint"/>
					<parameter name="end_index" type="guint"/>
				</parameters>
			</constructor>
			<field name="type" type="guint"/>
			<field name="value" type="guint"/>
			<field name="start_index" type="guint"/>
			<field name="end_index" type="guint"/>
		</object>
		<object name="IBusBus" parent="IBusObject" type-name="IBusBus" get-type="ibus_bus_get_type">
			<method name="add_match" symbol="ibus_bus_add_match">
				<return-type type="void"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
					<parameter name="rule" type="gchar*"/>
				</parameters>
			</method>
			<method name="create_input_context" symbol="ibus_bus_create_input_context">
				<return-type type="IBusInputContext*"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
					<parameter name="client_name" type="gchar*"/>
				</parameters>
			</method>
			<method name="current_input_context" symbol="ibus_bus_current_input_context">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="exit" symbol="ibus_bus_exit">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
					<parameter name="restart" type="gboolean"/>
				</parameters>
			</method>
			<method name="get_config" symbol="ibus_bus_get_config">
				<return-type type="IBusConfig*"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="get_connection" symbol="ibus_bus_get_connection">
				<return-type type="IBusConnection*"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="get_global_engine" symbol="ibus_bus_get_global_engine">
				<return-type type="IBusEngineDesc*"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="get_name_owner" symbol="ibus_bus_get_name_owner">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</method>
			<method name="get_use_global_engine" symbol="ibus_bus_get_use_global_engine">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="get_use_sys_layout" symbol="ibus_bus_get_use_sys_layout">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="hello" symbol="ibus_bus_hello">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="is_connected" symbol="ibus_bus_is_connected">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="is_global_engine_enabled" symbol="ibus_bus_is_global_engine_enabled">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="list_active_engines" symbol="ibus_bus_list_active_engines">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="list_engines" symbol="ibus_bus_list_engines">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="list_names" symbol="ibus_bus_list_names">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
				</parameters>
			</method>
			<method name="name_has_owner" symbol="ibus_bus_name_has_owner">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_bus_new">
				<return-type type="IBusBus*"/>
			</constructor>
			<method name="register_component" symbol="ibus_bus_register_component">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
					<parameter name="component" type="IBusComponent*"/>
				</parameters>
			</method>
			<method name="release_name" symbol="ibus_bus_release_name">
				<return-type type="guint"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</method>
			<method name="remove_match" symbol="ibus_bus_remove_match">
				<return-type type="void"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
					<parameter name="rule" type="gchar*"/>
				</parameters>
			</method>
			<method name="request_name" symbol="ibus_bus_request_name">
				<return-type type="guint"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="flags" type="guint"/>
				</parameters>
			</method>
			<method name="set_global_engine" symbol="ibus_bus_set_global_engine">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="bus" type="IBusBus*"/>
					<parameter name="global_engine" type="gchar*"/>
				</parameters>
			</method>
			<signal name="connected" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusBus*"/>
				</parameters>
			</signal>
			<signal name="disconnected" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusBus*"/>
				</parameters>
			</signal>
			<signal name="global-engine-changed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusBus*"/>
				</parameters>
			</signal>
		</object>
		<object name="IBusComponent" parent="IBusSerializable" type-name="IBusComponent" get-type="ibus_component_get_type">
			<method name="add_engine" symbol="ibus_component_add_engine">
				<return-type type="void"/>
				<parameters>
					<parameter name="component" type="IBusComponent*"/>
					<parameter name="engine" type="IBusEngineDesc*"/>
				</parameters>
			</method>
			<method name="add_observed_path" symbol="ibus_component_add_observed_path">
				<return-type type="void"/>
				<parameters>
					<parameter name="component" type="IBusComponent*"/>
					<parameter name="path" type="gchar*"/>
					<parameter name="access_fs" type="gboolean"/>
				</parameters>
			</method>
			<method name="check_modification" symbol="ibus_component_check_modification">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="component" type="IBusComponent*"/>
				</parameters>
			</method>
			<method name="get_engines" symbol="ibus_component_get_engines">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="component" type="IBusComponent*"/>
				</parameters>
			</method>
			<method name="get_from_engine" symbol="ibus_component_get_from_engine">
				<return-type type="IBusComponent*"/>
				<parameters>
					<parameter name="engine" type="IBusEngineDesc*"/>
				</parameters>
			</method>
			<method name="is_running" symbol="ibus_component_is_running">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="component" type="IBusComponent*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_component_new">
				<return-type type="IBusComponent*"/>
				<parameters>
					<parameter name="name" type="gchar*"/>
					<parameter name="description" type="gchar*"/>
					<parameter name="version" type="gchar*"/>
					<parameter name="license" type="gchar*"/>
					<parameter name="author" type="gchar*"/>
					<parameter name="homepage" type="gchar*"/>
					<parameter name="exec" type="gchar*"/>
					<parameter name="textdomain" type="gchar*"/>
				</parameters>
			</constructor>
			<constructor name="new_from_file" symbol="ibus_component_new_from_file">
				<return-type type="IBusComponent*"/>
				<parameters>
					<parameter name="filename" type="gchar*"/>
				</parameters>
			</constructor>
			<constructor name="new_from_xml_node" symbol="ibus_component_new_from_xml_node">
				<return-type type="IBusComponent*"/>
				<parameters>
					<parameter name="node" type="XMLNode*"/>
				</parameters>
			</constructor>
			<method name="output" symbol="ibus_component_output">
				<return-type type="void"/>
				<parameters>
					<parameter name="component" type="IBusComponent*"/>
					<parameter name="output" type="GString*"/>
					<parameter name="indent" type="gint"/>
				</parameters>
			</method>
			<method name="output_engines" symbol="ibus_component_output_engines">
				<return-type type="void"/>
				<parameters>
					<parameter name="component" type="IBusComponent*"/>
					<parameter name="output" type="GString*"/>
					<parameter name="indent" type="gint"/>
				</parameters>
			</method>
			<method name="start" symbol="ibus_component_start">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="component" type="IBusComponent*"/>
					<parameter name="verbose" type="gboolean"/>
				</parameters>
			</method>
			<method name="stop" symbol="ibus_component_stop">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="component" type="IBusComponent*"/>
				</parameters>
			</method>
			<field name="name" type="gchar*"/>
			<field name="description" type="gchar*"/>
			<field name="version" type="gchar*"/>
			<field name="license" type="gchar*"/>
			<field name="author" type="gchar*"/>
			<field name="homepage" type="gchar*"/>
			<field name="exec" type="gchar*"/>
			<field name="textdomain" type="gchar*"/>
			<field name="engines" type="GList*"/>
			<field name="observed_paths" type="GList*"/>
			<field name="pid" type="GPid"/>
			<field name="child_source_id" type="guint"/>
			<field name="pdummy" type="gpointer[]"/>
		</object>
		<object name="IBusConfig" parent="IBusProxy" type-name="IBusConfig" get-type="ibus_config_get_type">
			<method name="get_value" symbol="ibus_config_get_value">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="config" type="IBusConfig*"/>
					<parameter name="section" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="value" type="GValue*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_config_new">
				<return-type type="IBusConfig*"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</constructor>
			<method name="set_value" symbol="ibus_config_set_value">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="config" type="IBusConfig*"/>
					<parameter name="section" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="value" type="GValue*"/>
				</parameters>
			</method>
			<method name="unset" symbol="ibus_config_unset">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="config" type="IBusConfig*"/>
					<parameter name="section" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</method>
			<signal name="value-changed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusConfig*"/>
					<parameter name="p0" type="char*"/>
					<parameter name="p1" type="char*"/>
					<parameter name="p2" type="GValue*"/>
				</parameters>
			</signal>
		</object>
		<object name="IBusConfigService" parent="IBusService" type-name="IBusConfigService" get-type="ibus_config_service_get_type">
			<constructor name="new" symbol="ibus_config_service_new">
				<return-type type="IBusConfigService*"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</constructor>
			<method name="value_changed" symbol="ibus_config_service_value_changed">
				<return-type type="void"/>
				<parameters>
					<parameter name="config" type="IBusConfigService*"/>
					<parameter name="section" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="value" type="GValue*"/>
				</parameters>
			</method>
			<property name="connection" type="IBusConnection*" readable="1" writable="1" construct="0" construct-only="1"/>
			<vfunc name="get_value">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="config" type="IBusConfigService*"/>
					<parameter name="section" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="value" type="GValue*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="set_value">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="config" type="IBusConfigService*"/>
					<parameter name="section" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="value" type="GValue*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="unset">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="config" type="IBusConfigService*"/>
					<parameter name="section" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
		</object>
		<object name="IBusConnection" parent="IBusObject" type-name="IBusConnection" get-type="ibus_connection_get_type">
			<method name="call" symbol="ibus_connection_call">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="path" type="gchar*"/>
					<parameter name="interface" type="gchar*"/>
					<parameter name="member" type="gchar*"/>
					<parameter name="error" type="IBusError**"/>
					<parameter name="first_arg_type" type="GType"/>
				</parameters>
			</method>
			<method name="call_with_reply" symbol="ibus_connection_call_with_reply">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="path" type="gchar*"/>
					<parameter name="interface" type="gchar*"/>
					<parameter name="member" type="gchar*"/>
					<parameter name="error" type="IBusError**"/>
					<parameter name="first_arg_type" type="GType"/>
				</parameters>
			</method>
			<method name="close" symbol="ibus_connection_close">
				<return-type type="void"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</method>
			<method name="flush" symbol="ibus_connection_flush">
				<return-type type="void"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</method>
			<method name="get_connection" symbol="ibus_connection_get_connection">
				<return-type type="DBusConnection*"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</method>
			<method name="get_unix_user" symbol="ibus_connection_get_unix_user">
				<return-type type="glong"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</method>
			<method name="is_authenticated" symbol="ibus_connection_is_authenticated">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</method>
			<method name="is_connected" symbol="ibus_connection_is_connected">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_connection_new">
				<return-type type="IBusConnection*"/>
			</constructor>
			<method name="open" symbol="ibus_connection_open">
				<return-type type="IBusConnection*"/>
				<parameters>
					<parameter name="address" type="gchar*"/>
				</parameters>
			</method>
			<method name="open_private" symbol="ibus_connection_open_private">
				<return-type type="IBusConnection*"/>
				<parameters>
					<parameter name="address" type="gchar*"/>
				</parameters>
			</method>
			<method name="read_write_dispatch" symbol="ibus_connection_read_write_dispatch">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="timeout" type="gint"/>
				</parameters>
			</method>
			<method name="register_object_path" symbol="ibus_connection_register_object_path">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="path" type="gchar*"/>
					<parameter name="message_func" type="IBusMessageFunc"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="send" symbol="ibus_connection_send">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="send_signal" symbol="ibus_connection_send_signal">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="path" type="gchar*"/>
					<parameter name="interface" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="first_arg_type" type="GType"/>
				</parameters>
			</method>
			<method name="send_signal_valist" symbol="ibus_connection_send_signal_valist">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="path" type="gchar*"/>
					<parameter name="interface" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="first_arg_type" type="GType"/>
					<parameter name="args" type="va_list"/>
				</parameters>
			</method>
			<method name="send_valist" symbol="ibus_connection_send_valist">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="message_type" type="gint"/>
					<parameter name="path" type="gchar*"/>
					<parameter name="interface" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="first_arg_type" type="GType"/>
					<parameter name="args" type="va_list"/>
				</parameters>
			</method>
			<method name="send_with_reply" symbol="ibus_connection_send_with_reply">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="pending_return" type="IBusPendingCall**"/>
					<parameter name="timeout_milliseconds" type="gint"/>
				</parameters>
			</method>
			<method name="send_with_reply_and_block" symbol="ibus_connection_send_with_reply_and_block">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="timeout_milliseconds" type="gint"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</method>
			<method name="set_connection" symbol="ibus_connection_set_connection">
				<return-type type="void"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="dbus_connection" type="DBusConnection*"/>
					<parameter name="shared" type="gboolean"/>
				</parameters>
			</method>
			<method name="unregister_object_path" symbol="ibus_connection_unregister_object_path">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="path" type="gchar*"/>
				</parameters>
			</method>
			<signal name="authenticate-unix-user" when="LAST">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="uid" type="gulong"/>
				</parameters>
			</signal>
			<signal name="disconnected" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</signal>
			<signal name="ibus-message" when="LAST">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="message" type="gpointer"/>
				</parameters>
			</signal>
			<signal name="ibus-message-sent" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="message" type="gpointer"/>
				</parameters>
			</signal>
			<signal name="ibus-signal" when="LAST">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="message" type="gpointer"/>
				</parameters>
			</signal>
		</object>
		<object name="IBusEngine" parent="IBusService" type-name="IBusEngine" get-type="ibus_engine_get_type">
			<method name="commit_text" symbol="ibus_engine_commit_text">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="text" type="IBusText*"/>
				</parameters>
			</method>
			<method name="delete_surrounding_text" symbol="ibus_engine_delete_surrounding_text">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="offset" type="gint"/>
					<parameter name="nchars" type="guint"/>
				</parameters>
			</method>
			<method name="forward_key_event" symbol="ibus_engine_forward_key_event">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="keyval" type="guint"/>
					<parameter name="keycode" type="guint"/>
					<parameter name="state" type="guint"/>
				</parameters>
			</method>
			<method name="get_name" symbol="ibus_engine_get_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</method>
			<method name="hide_auxiliary_text" symbol="ibus_engine_hide_auxiliary_text">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</method>
			<method name="hide_lookup_table" symbol="ibus_engine_hide_lookup_table">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</method>
			<method name="hide_preedit_text" symbol="ibus_engine_hide_preedit_text">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_engine_new">
				<return-type type="IBusEngine*"/>
				<parameters>
					<parameter name="name" type="gchar*"/>
					<parameter name="path" type="gchar*"/>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</constructor>
			<method name="register_properties" symbol="ibus_engine_register_properties">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="prop_list" type="IBusPropList*"/>
				</parameters>
			</method>
			<method name="show_auxiliary_text" symbol="ibus_engine_show_auxiliary_text">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</method>
			<method name="show_lookup_table" symbol="ibus_engine_show_lookup_table">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</method>
			<method name="show_preedit_text" symbol="ibus_engine_show_preedit_text">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</method>
			<method name="update_auxiliary_text" symbol="ibus_engine_update_auxiliary_text">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="text" type="IBusText*"/>
					<parameter name="visible" type="gboolean"/>
				</parameters>
			</method>
			<method name="update_lookup_table" symbol="ibus_engine_update_lookup_table">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="lookup_table" type="IBusLookupTable*"/>
					<parameter name="visible" type="gboolean"/>
				</parameters>
			</method>
			<method name="update_lookup_table_fast" symbol="ibus_engine_update_lookup_table_fast">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="lookup_table" type="IBusLookupTable*"/>
					<parameter name="visible" type="gboolean"/>
				</parameters>
			</method>
			<method name="update_preedit_text" symbol="ibus_engine_update_preedit_text">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="text" type="IBusText*"/>
					<parameter name="cursor_pos" type="guint"/>
					<parameter name="visible" type="gboolean"/>
				</parameters>
			</method>
			<method name="update_preedit_text_with_mode" symbol="ibus_engine_update_preedit_text_with_mode">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="text" type="IBusText*"/>
					<parameter name="cursor_pos" type="guint"/>
					<parameter name="visible" type="gboolean"/>
					<parameter name="mode" type="IBusPreeditFocusMode"/>
				</parameters>
			</method>
			<method name="update_property" symbol="ibus_engine_update_property">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="prop" type="IBusProperty*"/>
				</parameters>
			</method>
			<property name="connection" type="IBusConnection*" readable="1" writable="1" construct="0" construct-only="1"/>
			<property name="name" type="char*" readable="1" writable="1" construct="0" construct-only="1"/>
			<signal name="candidate-clicked" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="index" type="guint"/>
					<parameter name="button" type="guint"/>
					<parameter name="state" type="guint"/>
				</parameters>
			</signal>
			<signal name="cursor-down" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</signal>
			<signal name="cursor-up" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</signal>
			<signal name="disable" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</signal>
			<signal name="enable" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</signal>
			<signal name="focus-in" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</signal>
			<signal name="focus-out" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</signal>
			<signal name="page-down" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</signal>
			<signal name="page-up" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</signal>
			<signal name="process-key-event" when="LAST">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="keyval" type="guint"/>
					<parameter name="keycode" type="guint"/>
					<parameter name="state" type="guint"/>
				</parameters>
			</signal>
			<signal name="property-activate" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="prop_name" type="char*"/>
					<parameter name="prop_state" type="guint"/>
				</parameters>
			</signal>
			<signal name="property-hide" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="prop_name" type="char*"/>
				</parameters>
			</signal>
			<signal name="property-show" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="prop_name" type="char*"/>
				</parameters>
			</signal>
			<signal name="reset" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
				</parameters>
			</signal>
			<signal name="set-capabilities" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="caps" type="guint"/>
				</parameters>
			</signal>
			<signal name="set-cursor-location" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="engine" type="IBusEngine*"/>
					<parameter name="x" type="gint"/>
					<parameter name="y" type="gint"/>
					<parameter name="w" type="gint"/>
					<parameter name="h" type="gint"/>
				</parameters>
			</signal>
			<field name="enabled" type="gboolean"/>
			<field name="has_focus" type="gboolean"/>
			<field name="cursor_area" type="IBusRectangle"/>
			<field name="client_capabilities" type="guint"/>
		</object>
		<object name="IBusEngineDesc" parent="IBusSerializable" type-name="IBusEngineDesc" get-type="ibus_engine_desc_get_type">
			<constructor name="new" symbol="ibus_engine_desc_new">
				<return-type type="IBusEngineDesc*"/>
				<parameters>
					<parameter name="name" type="gchar*"/>
					<parameter name="longname" type="gchar*"/>
					<parameter name="description" type="gchar*"/>
					<parameter name="language" type="gchar*"/>
					<parameter name="license" type="gchar*"/>
					<parameter name="author" type="gchar*"/>
					<parameter name="icon" type="gchar*"/>
					<parameter name="layout" type="gchar*"/>
				</parameters>
			</constructor>
			<constructor name="new_from_xml_node" symbol="ibus_engine_desc_new_from_xml_node">
				<return-type type="IBusEngineDesc*"/>
				<parameters>
					<parameter name="node" type="XMLNode*"/>
				</parameters>
			</constructor>
			<method name="output" symbol="ibus_engine_desc_output">
				<return-type type="void"/>
				<parameters>
					<parameter name="info" type="IBusEngineDesc*"/>
					<parameter name="output" type="GString*"/>
					<parameter name="indent" type="gint"/>
				</parameters>
			</method>
			<field name="name" type="gchar*"/>
			<field name="longname" type="gchar*"/>
			<field name="description" type="gchar*"/>
			<field name="language" type="gchar*"/>
			<field name="license" type="gchar*"/>
			<field name="author" type="gchar*"/>
			<field name="icon" type="gchar*"/>
			<field name="layout" type="gchar*"/>
			<field name="rank" type="guint"/>
		</object>
		<object name="IBusFactory" parent="IBusService" type-name="IBusFactory" get-type="ibus_factory_get_type">
			<method name="add_engine" symbol="ibus_factory_add_engine">
				<return-type type="void"/>
				<parameters>
					<parameter name="factory" type="IBusFactory*"/>
					<parameter name="engine_name" type="gchar*"/>
					<parameter name="engine_type" type="GType"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_factory_new">
				<return-type type="IBusFactory*"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</constructor>
			<property name="connection" type="IBusConnection*" readable="1" writable="1" construct="0" construct-only="1"/>
		</object>
		<object name="IBusHotkeyProfile" parent="IBusSerializable" type-name="IBusHotkeyProfile" get-type="ibus_hotkey_profile_get_type">
			<method name="add_hotkey" symbol="ibus_hotkey_profile_add_hotkey">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="profile" type="IBusHotkeyProfile*"/>
					<parameter name="keyval" type="guint"/>
					<parameter name="modifiers" type="guint"/>
					<parameter name="event" type="GQuark"/>
				</parameters>
			</method>
			<method name="add_hotkey_from_string" symbol="ibus_hotkey_profile_add_hotkey_from_string">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="profile" type="IBusHotkeyProfile*"/>
					<parameter name="str" type="gchar*"/>
					<parameter name="event" type="GQuark"/>
				</parameters>
			</method>
			<method name="filter_key_event" symbol="ibus_hotkey_profile_filter_key_event">
				<return-type type="GQuark"/>
				<parameters>
					<parameter name="profile" type="IBusHotkeyProfile*"/>
					<parameter name="keyval" type="guint"/>
					<parameter name="modifiers" type="guint"/>
					<parameter name="prev_keyval" type="guint"/>
					<parameter name="prev_modifiers" type="guint"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_hotkey_profile_new">
				<return-type type="IBusHotkeyProfile*"/>
			</constructor>
			<method name="remove_hotkey" symbol="ibus_hotkey_profile_remove_hotkey">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="profile" type="IBusHotkeyProfile*"/>
					<parameter name="keyval" type="guint"/>
					<parameter name="modifiers" type="guint"/>
				</parameters>
			</method>
			<method name="remove_hotkey_by_event" symbol="ibus_hotkey_profile_remove_hotkey_by_event">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="profile" type="IBusHotkeyProfile*"/>
					<parameter name="event" type="GQuark"/>
				</parameters>
			</method>
			<signal name="trigger" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="profile" type="IBusHotkeyProfile*"/>
					<parameter name="event" type="guint"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</signal>
		</object>
		<object name="IBusInputContext" parent="IBusProxy" type-name="IBusInputContext" get-type="ibus_input_context_get_type">
			<method name="disable" symbol="ibus_input_context_disable">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
				</parameters>
			</method>
			<method name="enable" symbol="ibus_input_context_enable">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
				</parameters>
			</method>
			<method name="focus_in" symbol="ibus_input_context_focus_in">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
				</parameters>
			</method>
			<method name="focus_out" symbol="ibus_input_context_focus_out">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
				</parameters>
			</method>
			<method name="get_engine" symbol="ibus_input_context_get_engine">
				<return-type type="IBusEngineDesc*"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
				</parameters>
			</method>
			<method name="get_input_context" symbol="ibus_input_context_get_input_context">
				<return-type type="IBusInputContext*"/>
				<parameters>
					<parameter name="path" type="gchar*"/>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</method>
			<method name="is_enabled" symbol="ibus_input_context_is_enabled">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_input_context_new">
				<return-type type="IBusInputContext*"/>
				<parameters>
					<parameter name="path" type="gchar*"/>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</constructor>
			<method name="process_key_event" symbol="ibus_input_context_process_key_event">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
					<parameter name="keyval" type="guint32"/>
					<parameter name="keycode" type="guint32"/>
					<parameter name="state" type="guint32"/>
				</parameters>
			</method>
			<method name="property_activate" symbol="ibus_input_context_property_activate">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
					<parameter name="prop_name" type="gchar*"/>
					<parameter name="state" type="gint32"/>
				</parameters>
			</method>
			<method name="reset" symbol="ibus_input_context_reset">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
				</parameters>
			</method>
			<method name="set_capabilities" symbol="ibus_input_context_set_capabilities">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
					<parameter name="capabilities" type="guint32"/>
				</parameters>
			</method>
			<method name="set_cursor_location" symbol="ibus_input_context_set_cursor_location">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
					<parameter name="x" type="gint32"/>
					<parameter name="y" type="gint32"/>
					<parameter name="w" type="gint32"/>
					<parameter name="h" type="gint32"/>
				</parameters>
			</method>
			<method name="set_engine" symbol="ibus_input_context_set_engine">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="IBusInputContext*"/>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</method>
			<signal name="commit-text" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
					<parameter name="p0" type="IBusText*"/>
				</parameters>
			</signal>
			<signal name="cursor-down-lookup-table" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="cursor-up-lookup-table" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="delete-surrounding-text" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
					<parameter name="p0" type="gint"/>
					<parameter name="p1" type="guint"/>
				</parameters>
			</signal>
			<signal name="disabled" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="enabled" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="forward-key-event" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
					<parameter name="p0" type="guint"/>
					<parameter name="p1" type="guint"/>
					<parameter name="p2" type="guint"/>
				</parameters>
			</signal>
			<signal name="hide-auxiliary-text" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="hide-lookup-table" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="hide-preedit-text" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="page-down-lookup-table" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="page-up-lookup-table" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="register-properties" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
					<parameter name="p0" type="IBusPropList*"/>
				</parameters>
			</signal>
			<signal name="show-auxiliary-text" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="show-lookup-table" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="show-preedit-text" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
				</parameters>
			</signal>
			<signal name="update-auxiliary-text" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
					<parameter name="p0" type="IBusText*"/>
					<parameter name="p1" type="gboolean"/>
				</parameters>
			</signal>
			<signal name="update-lookup-table" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
					<parameter name="p0" type="IBusLookupTable*"/>
					<parameter name="p1" type="gboolean"/>
				</parameters>
			</signal>
			<signal name="update-preedit-text" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
					<parameter name="p0" type="IBusText*"/>
					<parameter name="p1" type="guint"/>
					<parameter name="p2" type="gboolean"/>
				</parameters>
			</signal>
			<signal name="update-property" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusInputContext*"/>
					<parameter name="p0" type="IBusProperty*"/>
				</parameters>
			</signal>
		</object>
		<object name="IBusKeymap" parent="IBusObject" type-name="IBusKeymap" get-type="ibus_keymap_get_type">
			<method name="get" symbol="ibus_keymap_get">
				<return-type type="IBusKeymap*"/>
				<parameters>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</method>
			<method name="lookup_keysym" symbol="ibus_keymap_lookup_keysym">
				<return-type type="guint"/>
				<parameters>
					<parameter name="keymap" type="IBusKeymap*"/>
					<parameter name="keycode" type="guint16"/>
					<parameter name="state" type="guint32"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_keymap_new">
				<return-type type="IBusKeymap*"/>
				<parameters>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</constructor>
			<field name="name" type="gchar*"/>
			<field name="keymap" type="guint[][]"/>
		</object>
		<object name="IBusLookupTable" parent="IBusSerializable" type-name="IBusLookupTable" get-type="ibus_lookup_table_get_type">
			<method name="append_candidate" symbol="ibus_lookup_table_append_candidate">
				<return-type type="void"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
					<parameter name="text" type="IBusText*"/>
				</parameters>
			</method>
			<method name="append_label" symbol="ibus_lookup_table_append_label">
				<return-type type="void"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
					<parameter name="text" type="IBusText*"/>
				</parameters>
			</method>
			<method name="clear" symbol="ibus_lookup_table_clear">
				<return-type type="void"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<method name="cursor_down" symbol="ibus_lookup_table_cursor_down">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<method name="cursor_up" symbol="ibus_lookup_table_cursor_up">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<method name="get_candidate" symbol="ibus_lookup_table_get_candidate">
				<return-type type="IBusText*"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
					<parameter name="index" type="guint"/>
				</parameters>
			</method>
			<method name="get_cursor_in_page" symbol="ibus_lookup_table_get_cursor_in_page">
				<return-type type="guint"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<method name="get_cursor_pos" symbol="ibus_lookup_table_get_cursor_pos">
				<return-type type="guint"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<method name="get_label" symbol="ibus_lookup_table_get_label">
				<return-type type="IBusText*"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
					<parameter name="index" type="guint"/>
				</parameters>
			</method>
			<method name="get_number_of_candidates" symbol="ibus_lookup_table_get_number_of_candidates">
				<return-type type="guint"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<method name="get_orientation" symbol="ibus_lookup_table_get_orientation">
				<return-type type="gint"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<method name="get_page_size" symbol="ibus_lookup_table_get_page_size">
				<return-type type="guint"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<method name="is_cursor_visible" symbol="ibus_lookup_table_is_cursor_visible">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<method name="is_round" symbol="ibus_lookup_table_is_round">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_lookup_table_new">
				<return-type type="IBusLookupTable*"/>
				<parameters>
					<parameter name="page_size" type="guint"/>
					<parameter name="cursor_pos" type="guint"/>
					<parameter name="cursor_visible" type="gboolean"/>
					<parameter name="round" type="gboolean"/>
				</parameters>
			</constructor>
			<method name="page_down" symbol="ibus_lookup_table_page_down">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<method name="page_up" symbol="ibus_lookup_table_page_up">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
				</parameters>
			</method>
			<method name="set_cursor_pos" symbol="ibus_lookup_table_set_cursor_pos">
				<return-type type="void"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
					<parameter name="cursor_pos" type="guint"/>
				</parameters>
			</method>
			<method name="set_cursor_visible" symbol="ibus_lookup_table_set_cursor_visible">
				<return-type type="void"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
					<parameter name="visible" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_label" symbol="ibus_lookup_table_set_label">
				<return-type type="void"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
					<parameter name="index" type="guint"/>
					<parameter name="text" type="IBusText*"/>
				</parameters>
			</method>
			<method name="set_orientation" symbol="ibus_lookup_table_set_orientation">
				<return-type type="void"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
					<parameter name="orientation" type="gint"/>
				</parameters>
			</method>
			<method name="set_page_size" symbol="ibus_lookup_table_set_page_size">
				<return-type type="void"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
					<parameter name="page_size" type="guint"/>
				</parameters>
			</method>
			<method name="set_round" symbol="ibus_lookup_table_set_round">
				<return-type type="void"/>
				<parameters>
					<parameter name="table" type="IBusLookupTable*"/>
					<parameter name="round" type="gboolean"/>
				</parameters>
			</method>
			<field name="page_size" type="guint"/>
			<field name="cursor_pos" type="guint"/>
			<field name="cursor_visible" type="gboolean"/>
			<field name="round" type="gboolean"/>
			<field name="orientation" type="gint"/>
			<field name="candidates" type="GArray*"/>
			<field name="labels" type="GArray*"/>
		</object>
		<object name="IBusObject" parent="GInitiallyUnowned" type-name="IBusObject" get-type="ibus_object_get_type">
			<method name="destroy" symbol="ibus_object_destroy">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusObject*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_object_new">
				<return-type type="IBusObject*"/>
			</constructor>
			<signal name="destroy" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusObject*"/>
				</parameters>
			</signal>
			<field name="flags" type="guint32"/>
		</object>
		<object name="IBusObservedPath" parent="IBusSerializable" type-name="IBusObservedPath" get-type="ibus_observed_path_get_type">
			<method name="check_modification" symbol="ibus_observed_path_check_modification">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="path" type="IBusObservedPath*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_observed_path_new">
				<return-type type="IBusObservedPath*"/>
				<parameters>
					<parameter name="path" type="gchar*"/>
					<parameter name="fill_stat" type="gboolean"/>
				</parameters>
			</constructor>
			<constructor name="new_from_xml_node" symbol="ibus_observed_path_new_from_xml_node">
				<return-type type="IBusObservedPath*"/>
				<parameters>
					<parameter name="node" type="XMLNode*"/>
					<parameter name="fill_stat" type="gboolean"/>
				</parameters>
			</constructor>
			<method name="output" symbol="ibus_observed_path_output">
				<return-type type="void"/>
				<parameters>
					<parameter name="path" type="IBusObservedPath*"/>
					<parameter name="output" type="GString*"/>
					<parameter name="indent" type="gint"/>
				</parameters>
			</method>
			<method name="traverse" symbol="ibus_observed_path_traverse">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="path" type="IBusObservedPath*"/>
				</parameters>
			</method>
			<field name="path" type="gchar*"/>
			<field name="mtime" type="glong"/>
			<field name="is_dir" type="gboolean"/>
			<field name="is_exist" type="gboolean"/>
		</object>
		<object name="IBusPanelService" parent="IBusService" type-name="IBusPanelService" get-type="ibus_panel_service_get_type">
			<method name="candidate_clicked" symbol="ibus_panel_service_candidate_clicked">
				<return-type type="void"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="index" type="guint"/>
					<parameter name="button" type="guint"/>
					<parameter name="state" type="guint"/>
				</parameters>
			</method>
			<method name="cursor_down" symbol="ibus_panel_service_cursor_down">
				<return-type type="void"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
				</parameters>
			</method>
			<method name="cursor_up" symbol="ibus_panel_service_cursor_up">
				<return-type type="void"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_panel_service_new">
				<return-type type="IBusPanelService*"/>
				<parameters>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</constructor>
			<method name="page_down" symbol="ibus_panel_service_page_down">
				<return-type type="void"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
				</parameters>
			</method>
			<method name="page_up" symbol="ibus_panel_service_page_up">
				<return-type type="void"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
				</parameters>
			</method>
			<method name="property_active" symbol="ibus_panel_service_property_active">
				<return-type type="void"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="prop_name" type="gchar*"/>
					<parameter name="prop_state" type="int"/>
				</parameters>
			</method>
			<method name="property_hide" symbol="ibus_panel_service_property_hide">
				<return-type type="void"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="prop_name" type="gchar*"/>
				</parameters>
			</method>
			<method name="property_show" symbol="ibus_panel_service_property_show">
				<return-type type="void"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="prop_name" type="gchar*"/>
				</parameters>
			</method>
			<property name="connection" type="IBusConnection*" readable="1" writable="1" construct="0" construct-only="1"/>
			<vfunc name="cursor_down_lookup_table">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="cursor_up_lookup_table">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="destroy">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="focus_in">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="input_context_path" type="gchar*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="focus_out">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="input_context_path" type="gchar*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="hide_auxiliary_text">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="hide_language_bar">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="hide_lookup_table">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="hide_preedit_text">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="page_down_lookup_table">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="page_up_lookup_table">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="register_properties">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="prop_list" type="IBusPropList*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="reset">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="set_cursor_location">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="x" type="gint"/>
					<parameter name="y" type="gint"/>
					<parameter name="w" type="gint"/>
					<parameter name="h" type="gint"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="show_auxiliary_text">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="show_language_bar">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="show_lookup_table">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="show_preedit_text">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="start_setup">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="state_changed">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="update_auxiliary_text">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="text" type="IBusText*"/>
					<parameter name="visible" type="gboolean"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="update_lookup_table">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="lookup_table" type="IBusLookupTable*"/>
					<parameter name="visible" type="gboolean"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="update_preedit_text">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="text" type="IBusText*"/>
					<parameter name="cursor_pos" type="guint"/>
					<parameter name="visible" type="gboolean"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
			<vfunc name="update_property">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="panel" type="IBusPanelService*"/>
					<parameter name="prop" type="IBusProperty*"/>
					<parameter name="error" type="IBusError**"/>
				</parameters>
			</vfunc>
		</object>
		<object name="IBusPropList" parent="IBusSerializable" type-name="IBusPropList" get-type="ibus_prop_list_get_type">
			<method name="append" symbol="ibus_prop_list_append">
				<return-type type="void"/>
				<parameters>
					<parameter name="prop_list" type="IBusPropList*"/>
					<parameter name="prop" type="IBusProperty*"/>
				</parameters>
			</method>
			<method name="get" symbol="ibus_prop_list_get">
				<return-type type="IBusProperty*"/>
				<parameters>
					<parameter name="prop_list" type="IBusPropList*"/>
					<parameter name="index" type="guint"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_prop_list_new">
				<return-type type="IBusPropList*"/>
			</constructor>
			<method name="update_property" symbol="ibus_prop_list_update_property">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="prop_list" type="IBusPropList*"/>
					<parameter name="prop" type="IBusProperty*"/>
				</parameters>
			</method>
			<field name="properties" type="GArray*"/>
		</object>
		<object name="IBusProperty" parent="IBusSerializable" type-name="IBusProperty" get-type="ibus_property_get_type">
			<constructor name="new" symbol="ibus_property_new">
				<return-type type="IBusProperty*"/>
				<parameters>
					<parameter name="key" type="gchar*"/>
					<parameter name="type" type="IBusPropType"/>
					<parameter name="label" type="IBusText*"/>
					<parameter name="icon" type="gchar*"/>
					<parameter name="tooltip" type="IBusText*"/>
					<parameter name="sensitive" type="gboolean"/>
					<parameter name="visible" type="gboolean"/>
					<parameter name="state" type="IBusPropState"/>
					<parameter name="prop_list" type="IBusPropList*"/>
				</parameters>
			</constructor>
			<method name="set_icon" symbol="ibus_property_set_icon">
				<return-type type="void"/>
				<parameters>
					<parameter name="prop" type="IBusProperty*"/>
					<parameter name="icon" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_label" symbol="ibus_property_set_label">
				<return-type type="void"/>
				<parameters>
					<parameter name="prop" type="IBusProperty*"/>
					<parameter name="label" type="IBusText*"/>
				</parameters>
			</method>
			<method name="set_sensitive" symbol="ibus_property_set_sensitive">
				<return-type type="void"/>
				<parameters>
					<parameter name="prop" type="IBusProperty*"/>
					<parameter name="sensitive" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_state" symbol="ibus_property_set_state">
				<return-type type="void"/>
				<parameters>
					<parameter name="prop" type="IBusProperty*"/>
					<parameter name="state" type="IBusPropState"/>
				</parameters>
			</method>
			<method name="set_sub_props" symbol="ibus_property_set_sub_props">
				<return-type type="void"/>
				<parameters>
					<parameter name="prop" type="IBusProperty*"/>
					<parameter name="prop_list" type="IBusPropList*"/>
				</parameters>
			</method>
			<method name="set_tooltip" symbol="ibus_property_set_tooltip">
				<return-type type="void"/>
				<parameters>
					<parameter name="prop" type="IBusProperty*"/>
					<parameter name="tooltip" type="IBusText*"/>
				</parameters>
			</method>
			<method name="set_visible" symbol="ibus_property_set_visible">
				<return-type type="void"/>
				<parameters>
					<parameter name="prop" type="IBusProperty*"/>
					<parameter name="visible" type="gboolean"/>
				</parameters>
			</method>
			<method name="update" symbol="ibus_property_update">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="prop" type="IBusProperty*"/>
					<parameter name="prop_update" type="IBusProperty*"/>
				</parameters>
			</method>
			<field name="key" type="gchar*"/>
			<field name="icon" type="gchar*"/>
			<field name="label" type="IBusText*"/>
			<field name="tooltip" type="IBusText*"/>
			<field name="sensitive" type="gboolean"/>
			<field name="visible" type="gboolean"/>
			<field name="type" type="guint"/>
			<field name="state" type="guint"/>
			<field name="sub_props" type="IBusPropList*"/>
		</object>
		<object name="IBusProxy" parent="IBusObject" type-name="IBusProxy" get-type="ibus_proxy_get_type">
			<method name="call" symbol="ibus_proxy_call">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
					<parameter name="method" type="gchar*"/>
					<parameter name="first_arg_type" type="GType"/>
				</parameters>
			</method>
			<method name="call_with_reply" symbol="ibus_proxy_call_with_reply">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
					<parameter name="method" type="gchar*"/>
					<parameter name="pending" type="IBusPendingCall**"/>
					<parameter name="timeout_milliseconds" type="gint"/>
					<parameter name="error" type="IBusError**"/>
					<parameter name="first_arg_type" type="GType"/>
				</parameters>
			</method>
			<method name="call_with_reply_and_block" symbol="ibus_proxy_call_with_reply_and_block">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
					<parameter name="method" type="gchar*"/>
					<parameter name="timeout_milliseconds" type="gint"/>
					<parameter name="error" type="IBusError**"/>
					<parameter name="first_arg_type" type="GType"/>
				</parameters>
			</method>
			<method name="get_connection" symbol="ibus_proxy_get_connection">
				<return-type type="IBusConnection*"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
				</parameters>
			</method>
			<method name="get_interface" symbol="ibus_proxy_get_interface">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
				</parameters>
			</method>
			<method name="get_name" symbol="ibus_proxy_get_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
				</parameters>
			</method>
			<method name="get_path" symbol="ibus_proxy_get_path">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
				</parameters>
			</method>
			<method name="get_unique_name" symbol="ibus_proxy_get_unique_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
				</parameters>
			</method>
			<method name="handle_signal" symbol="ibus_proxy_handle_signal">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_proxy_new">
				<return-type type="IBusProxy*"/>
				<parameters>
					<parameter name="name" type="gchar*"/>
					<parameter name="path" type="gchar*"/>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</constructor>
			<method name="send" symbol="ibus_proxy_send">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<method name="send_with_reply" symbol="ibus_proxy_send_with_reply">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
					<parameter name="message" type="IBusMessage*"/>
					<parameter name="pending" type="IBusPendingCall**"/>
					<parameter name="timeout_milliseconds" type="gint"/>
				</parameters>
			</method>
			<method name="send_with_reply_and_block" symbol="ibus_proxy_send_with_reply_and_block">
				<return-type type="IBusMessage*"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<property name="connection" type="IBusConnection*" readable="1" writable="1" construct="0" construct-only="1"/>
			<property name="interface" type="char*" readable="1" writable="1" construct="0" construct-only="1"/>
			<property name="name" type="char*" readable="1" writable="1" construct="0" construct-only="1"/>
			<property name="path" type="char*" readable="1" writable="1" construct="0" construct-only="1"/>
			<signal name="ibus-signal" when="LAST">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="proxy" type="IBusProxy*"/>
					<parameter name="message" type="gpointer"/>
				</parameters>
			</signal>
		</object>
		<object name="IBusSerializable" parent="IBusObject" type-name="IBusSerializable" get-type="ibus_serializable_get_type">
			<method name="copy" symbol="ibus_serializable_copy">
				<return-type type="IBusSerializable*"/>
				<parameters>
					<parameter name="object" type="IBusSerializable*"/>
				</parameters>
			</method>
			<method name="deserialize" symbol="ibus_serializable_deserialize">
				<return-type type="IBusSerializable*"/>
				<parameters>
					<parameter name="iter" type="IBusMessageIter*"/>
				</parameters>
			</method>
			<method name="get_qattachment" symbol="ibus_serializable_get_qattachment">
				<return-type type="GValue*"/>
				<parameters>
					<parameter name="object" type="IBusSerializable*"/>
					<parameter name="key" type="GQuark"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_serializable_new">
				<return-type type="IBusSerializable*"/>
			</constructor>
			<method name="remove_qattachment" symbol="ibus_serializable_remove_qattachment">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="IBusSerializable*"/>
					<parameter name="key" type="GQuark"/>
				</parameters>
			</method>
			<method name="serialize" symbol="ibus_serializable_serialize">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="object" type="IBusSerializable*"/>
					<parameter name="iter" type="IBusMessageIter*"/>
				</parameters>
			</method>
			<method name="set_qattachment" symbol="ibus_serializable_set_qattachment">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="object" type="IBusSerializable*"/>
					<parameter name="key" type="GQuark"/>
					<parameter name="value" type="GValue*"/>
				</parameters>
			</method>
			<vfunc name="copy">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="dest" type="IBusSerializable*"/>
					<parameter name="src" type="IBusSerializable*"/>
				</parameters>
			</vfunc>
			<vfunc name="deserialize">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="object" type="IBusSerializable*"/>
					<parameter name="iter" type="IBusMessageIter*"/>
				</parameters>
			</vfunc>
			<vfunc name="serialize">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="object" type="IBusSerializable*"/>
					<parameter name="iter" type="IBusMessageIter*"/>
				</parameters>
			</vfunc>
			<field name="flags" type="guint32"/>
		</object>
		<object name="IBusServer" parent="IBusObject" type-name="IBusServer" get-type="ibus_server_get_type">
			<method name="disconnect" symbol="ibus_server_disconnect">
				<return-type type="void"/>
				<parameters>
					<parameter name="server" type="IBusServer*"/>
				</parameters>
			</method>
			<method name="get_address" symbol="ibus_server_get_address">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="server" type="IBusServer*"/>
				</parameters>
			</method>
			<method name="get_id" symbol="ibus_server_get_id">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="server" type="IBusServer*"/>
				</parameters>
			</method>
			<method name="is_connected" symbol="ibus_server_is_connected">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="server" type="IBusServer*"/>
				</parameters>
			</method>
			<method name="listen" symbol="ibus_server_listen">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="server" type="IBusServer*"/>
					<parameter name="address" type="gchar*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_server_new">
				<return-type type="IBusServer*"/>
			</constructor>
			<method name="set_auth_mechanisms" symbol="ibus_server_set_auth_mechanisms">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="server" type="IBusServer*"/>
					<parameter name="mechanisms" type="gchar**"/>
				</parameters>
			</method>
			<property name="connection-type" type="GType" readable="1" writable="1" construct="0" construct-only="0"/>
			<signal name="new-connection" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="server" type="IBusServer*"/>
					<parameter name="connectin" type="GObject*"/>
				</parameters>
			</signal>
		</object>
		<object name="IBusService" parent="IBusObject" type-name="IBusService" get-type="ibus_service_get_type">
			<method name="add_to_connection" symbol="ibus_service_add_to_connection">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="service" type="IBusService*"/>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</method>
			<method name="get_connections" symbol="ibus_service_get_connections">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="service" type="IBusService*"/>
				</parameters>
			</method>
			<method name="get_path" symbol="ibus_service_get_path">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="service" type="IBusService*"/>
				</parameters>
			</method>
			<method name="handle_message" symbol="ibus_service_handle_message">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="service" type="IBusService*"/>
					<parameter name="connection" type="IBusConnection*"/>
					<parameter name="message" type="IBusMessage*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ibus_service_new">
				<return-type type="IBusService*"/>
				<parameters>
					<parameter name="path" type="gchar*"/>
				</parameters>
			</constructor>
			<method name="remove_from_all_connections" symbol="ibus_service_remove_from_all_connections">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="service" type="IBusService*"/>
				</parameters>
			</method>
			<method name="remove_from_connection" symbol="ibus_service_remove_from_connection">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="service" type="IBusService*"/>
					<parameter name="connection" type="IBusConnection*"/>
				</parameters>
			</method>
			<method name="send_signal" symbol="ibus_service_send_signal">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="service" type="IBusService*"/>
					<parameter name="interface" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="first_arg_type" type="GType"/>
				</parameters>
			</method>
			<property name="path" type="char*" readable="1" writable="1" construct="0" construct-only="1"/>
			<signal name="ibus-message" when="LAST">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="service" type="IBusService*"/>
					<parameter name="connection" type="gpointer"/>
					<parameter name="message" type="gpointer"/>
				</parameters>
			</signal>
			<signal name="ibus-signal" when="LAST">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="service" type="IBusService*"/>
					<parameter name="connection" type="gpointer"/>
					<parameter name="message" type="gpointer"/>
				</parameters>
			</signal>
		</object>
		<object name="IBusText" parent="IBusSerializable" type-name="IBusText" get-type="ibus_text_get_type">
			<method name="append_attribute" symbol="ibus_text_append_attribute">
				<return-type type="void"/>
				<parameters>
					<parameter name="text" type="IBusText*"/>
					<parameter name="type" type="guint"/>
					<parameter name="value" type="guint"/>
					<parameter name="start_index" type="guint"/>
					<parameter name="end_index" type="gint"/>
				</parameters>
			</method>
			<method name="get_length" symbol="ibus_text_get_length">
				<return-type type="guint"/>
				<parameters>
					<parameter name="text" type="IBusText*"/>
				</parameters>
			</method>
			<constructor name="new_from_printf" symbol="ibus_text_new_from_printf">
				<return-type type="IBusText*"/>
				<parameters>
					<parameter name="fmt" type="gchar*"/>
				</parameters>
			</constructor>
			<constructor name="new_from_static_string" symbol="ibus_text_new_from_static_string">
				<return-type type="IBusText*"/>
				<parameters>
					<parameter name="str" type="gchar*"/>
				</parameters>
			</constructor>
			<constructor name="new_from_string" symbol="ibus_text_new_from_string">
				<return-type type="IBusText*"/>
				<parameters>
					<parameter name="str" type="gchar*"/>
				</parameters>
			</constructor>
			<constructor name="new_from_ucs4" symbol="ibus_text_new_from_ucs4">
				<return-type type="IBusText*"/>
				<parameters>
					<parameter name="str" type="gunichar*"/>
				</parameters>
			</constructor>
			<constructor name="new_from_unichar" symbol="ibus_text_new_from_unichar">
				<return-type type="IBusText*"/>
				<parameters>
					<parameter name="c" type="gunichar"/>
				</parameters>
			</constructor>
			<field name="is_static" type="gboolean"/>
			<field name="text" type="gchar*"/>
			<field name="attrs" type="IBusAttrList*"/>
		</object>
		<constant name="IBUS_0" type="int" value="48"/>
		<constant name="IBUS_1" type="int" value="49"/>
		<constant name="IBUS_2" type="int" value="50"/>
		<constant name="IBUS_3" type="int" value="51"/>
		<constant name="IBUS_3270_AltCursor" type="int" value="64784"/>
		<constant name="IBUS_3270_Attn" type="int" value="64782"/>
		<constant name="IBUS_3270_BackTab" type="int" value="64773"/>
		<constant name="IBUS_3270_ChangeScreen" type="int" value="64793"/>
		<constant name="IBUS_3270_Copy" type="int" value="64789"/>
		<constant name="IBUS_3270_CursorBlink" type="int" value="64783"/>
		<constant name="IBUS_3270_CursorSelect" type="int" value="64796"/>
		<constant name="IBUS_3270_DeleteWord" type="int" value="64794"/>
		<constant name="IBUS_3270_Duplicate" type="int" value="64769"/>
		<constant name="IBUS_3270_Enter" type="int" value="64798"/>
		<constant name="IBUS_3270_EraseEOF" type="int" value="64774"/>
		<constant name="IBUS_3270_EraseInput" type="int" value="64775"/>
		<constant name="IBUS_3270_ExSelect" type="int" value="64795"/>
		<constant name="IBUS_3270_FieldMark" type="int" value="64770"/>
		<constant name="IBUS_3270_Ident" type="int" value="64787"/>
		<constant name="IBUS_3270_Jump" type="int" value="64786"/>
		<constant name="IBUS_3270_KeyClick" type="int" value="64785"/>
		<constant name="IBUS_3270_Left2" type="int" value="64772"/>
		<constant name="IBUS_3270_PA1" type="int" value="64778"/>
		<constant name="IBUS_3270_PA2" type="int" value="64779"/>
		<constant name="IBUS_3270_PA3" type="int" value="64780"/>
		<constant name="IBUS_3270_Play" type="int" value="64790"/>
		<constant name="IBUS_3270_PrintScreen" type="int" value="64797"/>
		<constant name="IBUS_3270_Quit" type="int" value="64777"/>
		<constant name="IBUS_3270_Record" type="int" value="64792"/>
		<constant name="IBUS_3270_Reset" type="int" value="64776"/>
		<constant name="IBUS_3270_Right2" type="int" value="64771"/>
		<constant name="IBUS_3270_Rule" type="int" value="64788"/>
		<constant name="IBUS_3270_Setup" type="int" value="64791"/>
		<constant name="IBUS_3270_Test" type="int" value="64781"/>
		<constant name="IBUS_4" type="int" value="52"/>
		<constant name="IBUS_5" type="int" value="53"/>
		<constant name="IBUS_6" type="int" value="54"/>
		<constant name="IBUS_7" type="int" value="55"/>
		<constant name="IBUS_8" type="int" value="56"/>
		<constant name="IBUS_9" type="int" value="57"/>
		<constant name="IBUS_A" type="int" value="65"/>
		<constant name="IBUS_AE" type="int" value="198"/>
		<constant name="IBUS_Aacute" type="int" value="193"/>
		<constant name="IBUS_Abelowdot" type="int" value="16785056"/>
		<constant name="IBUS_Abreve" type="int" value="451"/>
		<constant name="IBUS_Abreveacute" type="int" value="16785070"/>
		<constant name="IBUS_Abrevebelowdot" type="int" value="16785078"/>
		<constant name="IBUS_Abrevegrave" type="int" value="16785072"/>
		<constant name="IBUS_Abrevehook" type="int" value="16785074"/>
		<constant name="IBUS_Abrevetilde" type="int" value="16785076"/>
		<constant name="IBUS_AccessX_Enable" type="int" value="65136"/>
		<constant name="IBUS_AccessX_Feedback_Enable" type="int" value="65137"/>
		<constant name="IBUS_Acircumflex" type="int" value="194"/>
		<constant name="IBUS_Acircumflexacute" type="int" value="16785060"/>
		<constant name="IBUS_Acircumflexbelowdot" type="int" value="16785068"/>
		<constant name="IBUS_Acircumflexgrave" type="int" value="16785062"/>
		<constant name="IBUS_Acircumflexhook" type="int" value="16785064"/>
		<constant name="IBUS_Acircumflextilde" type="int" value="16785066"/>
		<constant name="IBUS_Adiaeresis" type="int" value="196"/>
		<constant name="IBUS_Agrave" type="int" value="192"/>
		<constant name="IBUS_Ahook" type="int" value="16785058"/>
		<constant name="IBUS_Alt_L" type="int" value="65513"/>
		<constant name="IBUS_Alt_R" type="int" value="65514"/>
		<constant name="IBUS_Amacron" type="int" value="960"/>
		<constant name="IBUS_Aogonek" type="int" value="417"/>
		<constant name="IBUS_Arabic_0" type="int" value="16778848"/>
		<constant name="IBUS_Arabic_1" type="int" value="16778849"/>
		<constant name="IBUS_Arabic_2" type="int" value="16778850"/>
		<constant name="IBUS_Arabic_3" type="int" value="16778851"/>
		<constant name="IBUS_Arabic_4" type="int" value="16778852"/>
		<constant name="IBUS_Arabic_5" type="int" value="16778853"/>
		<constant name="IBUS_Arabic_6" type="int" value="16778854"/>
		<constant name="IBUS_Arabic_7" type="int" value="16778855"/>
		<constant name="IBUS_Arabic_8" type="int" value="16778856"/>
		<constant name="IBUS_Arabic_9" type="int" value="16778857"/>
		<constant name="IBUS_Arabic_ain" type="int" value="1497"/>
		<constant name="IBUS_Arabic_alef" type="int" value="1479"/>
		<constant name="IBUS_Arabic_alefmaksura" type="int" value="1513"/>
		<constant name="IBUS_Arabic_beh" type="int" value="1480"/>
		<constant name="IBUS_Arabic_comma" type="int" value="1452"/>
		<constant name="IBUS_Arabic_dad" type="int" value="1494"/>
		<constant name="IBUS_Arabic_dal" type="int" value="1487"/>
		<constant name="IBUS_Arabic_damma" type="int" value="1519"/>
		<constant name="IBUS_Arabic_dammatan" type="int" value="1516"/>
		<constant name="IBUS_Arabic_ddal" type="int" value="16778888"/>
		<constant name="IBUS_Arabic_farsi_yeh" type="int" value="16778956"/>
		<constant name="IBUS_Arabic_fatha" type="int" value="1518"/>
		<constant name="IBUS_Arabic_fathatan" type="int" value="1515"/>
		<constant name="IBUS_Arabic_feh" type="int" value="1505"/>
		<constant name="IBUS_Arabic_fullstop" type="int" value="16778964"/>
		<constant name="IBUS_Arabic_gaf" type="int" value="16778927"/>
		<constant name="IBUS_Arabic_ghain" type="int" value="1498"/>
		<constant name="IBUS_Arabic_ha" type="int" value="1511"/>
		<constant name="IBUS_Arabic_hah" type="int" value="1485"/>
		<constant name="IBUS_Arabic_hamza" type="int" value="1473"/>
		<constant name="IBUS_Arabic_hamza_above" type="int" value="16778836"/>
		<constant name="IBUS_Arabic_hamza_below" type="int" value="16778837"/>
		<constant name="IBUS_Arabic_hamzaonalef" type="int" value="1475"/>
		<constant name="IBUS_Arabic_hamzaonwaw" type="int" value="1476"/>
		<constant name="IBUS_Arabic_hamzaonyeh" type="int" value="1478"/>
		<constant name="IBUS_Arabic_hamzaunderalef" type="int" value="1477"/>
		<constant name="IBUS_Arabic_heh" type="int" value="1511"/>
		<constant name="IBUS_Arabic_heh_doachashmee" type="int" value="16778942"/>
		<constant name="IBUS_Arabic_heh_goal" type="int" value="16778945"/>
		<constant name="IBUS_Arabic_jeem" type="int" value="1484"/>
		<constant name="IBUS_Arabic_jeh" type="int" value="16778904"/>
		<constant name="IBUS_Arabic_kaf" type="int" value="1507"/>
		<constant name="IBUS_Arabic_kasra" type="int" value="1520"/>
		<constant name="IBUS_Arabic_kasratan" type="int" value="1517"/>
		<constant name="IBUS_Arabic_keheh" type="int" value="16778921"/>
		<constant name="IBUS_Arabic_khah" type="int" value="1486"/>
		<constant name="IBUS_Arabic_lam" type="int" value="1508"/>
		<constant name="IBUS_Arabic_madda_above" type="int" value="16778835"/>
		<constant name="IBUS_Arabic_maddaonalef" type="int" value="1474"/>
		<constant name="IBUS_Arabic_meem" type="int" value="1509"/>
		<constant name="IBUS_Arabic_noon" type="int" value="1510"/>
		<constant name="IBUS_Arabic_noon_ghunna" type="int" value="16778938"/>
		<constant name="IBUS_Arabic_peh" type="int" value="16778878"/>
		<constant name="IBUS_Arabic_percent" type="int" value="16778858"/>
		<constant name="IBUS_Arabic_qaf" type="int" value="1506"/>
		<constant name="IBUS_Arabic_question_mark" type="int" value="1471"/>
		<constant name="IBUS_Arabic_ra" type="int" value="1489"/>
		<constant name="IBUS_Arabic_rreh" type="int" value="16778897"/>
		<constant name="IBUS_Arabic_sad" type="int" value="1493"/>
		<constant name="IBUS_Arabic_seen" type="int" value="1491"/>
		<constant name="IBUS_Arabic_semicolon" type="int" value="1467"/>
		<constant name="IBUS_Arabic_shadda" type="int" value="1521"/>
		<constant name="IBUS_Arabic_sheen" type="int" value="1492"/>
		<constant name="IBUS_Arabic_sukun" type="int" value="1522"/>
		<constant name="IBUS_Arabic_superscript_alef" type="int" value="16778864"/>
		<constant name="IBUS_Arabic_switch" type="int" value="65406"/>
		<constant name="IBUS_Arabic_tah" type="int" value="1495"/>
		<constant name="IBUS_Arabic_tatweel" type="int" value="1504"/>
		<constant name="IBUS_Arabic_tcheh" type="int" value="16778886"/>
		<constant name="IBUS_Arabic_teh" type="int" value="1482"/>
		<constant name="IBUS_Arabic_tehmarbuta" type="int" value="1481"/>
		<constant name="IBUS_Arabic_thal" type="int" value="1488"/>
		<constant name="IBUS_Arabic_theh" type="int" value="1483"/>
		<constant name="IBUS_Arabic_tteh" type="int" value="16778873"/>
		<constant name="IBUS_Arabic_veh" type="int" value="16778916"/>
		<constant name="IBUS_Arabic_waw" type="int" value="1512"/>
		<constant name="IBUS_Arabic_yeh" type="int" value="1514"/>
		<constant name="IBUS_Arabic_yeh_baree" type="int" value="16778962"/>
		<constant name="IBUS_Arabic_zah" type="int" value="1496"/>
		<constant name="IBUS_Arabic_zain" type="int" value="1490"/>
		<constant name="IBUS_Aring" type="int" value="197"/>
		<constant name="IBUS_Armenian_AT" type="int" value="16778552"/>
		<constant name="IBUS_Armenian_AYB" type="int" value="16778545"/>
		<constant name="IBUS_Armenian_BEN" type="int" value="16778546"/>
		<constant name="IBUS_Armenian_CHA" type="int" value="16778569"/>
		<constant name="IBUS_Armenian_DA" type="int" value="16778548"/>
		<constant name="IBUS_Armenian_DZA" type="int" value="16778561"/>
		<constant name="IBUS_Armenian_E" type="int" value="16778551"/>
		<constant name="IBUS_Armenian_FE" type="int" value="16778582"/>
		<constant name="IBUS_Armenian_GHAT" type="int" value="16778562"/>
		<constant name="IBUS_Armenian_GIM" type="int" value="16778547"/>
		<constant name="IBUS_Armenian_HI" type="int" value="16778565"/>
		<constant name="IBUS_Armenian_HO" type="int" value="16778560"/>
		<constant name="IBUS_Armenian_INI" type="int" value="16778555"/>
		<constant name="IBUS_Armenian_JE" type="int" value="16778571"/>
		<constant name="IBUS_Armenian_KE" type="int" value="16778580"/>
		<constant name="IBUS_Armenian_KEN" type="int" value="16778559"/>
		<constant name="IBUS_Armenian_KHE" type="int" value="16778557"/>
		<constant name="IBUS_Armenian_LYUN" type="int" value="16778556"/>
		<constant name="IBUS_Armenian_MEN" type="int" value="16778564"/>
		<constant name="IBUS_Armenian_NU" type="int" value="16778566"/>
		<constant name="IBUS_Armenian_O" type="int" value="16778581"/>
		<constant name="IBUS_Armenian_PE" type="int" value="16778570"/>
		<constant name="IBUS_Armenian_PYUR" type="int" value="16778579"/>
		<constant name="IBUS_Armenian_RA" type="int" value="16778572"/>
		<constant name="IBUS_Armenian_RE" type="int" value="16778576"/>
		<constant name="IBUS_Armenian_SE" type="int" value="16778573"/>
		<constant name="IBUS_Armenian_SHA" type="int" value="16778567"/>
		<constant name="IBUS_Armenian_TCHE" type="int" value="16778563"/>
		<constant name="IBUS_Armenian_TO" type="int" value="16778553"/>
		<constant name="IBUS_Armenian_TSA" type="int" value="16778558"/>
		<constant name="IBUS_Armenian_TSO" type="int" value="16778577"/>
		<constant name="IBUS_Armenian_TYUN" type="int" value="16778575"/>
		<constant name="IBUS_Armenian_VEV" type="int" value="16778574"/>
		<constant name="IBUS_Armenian_VO" type="int" value="16778568"/>
		<constant name="IBUS_Armenian_VYUN" type="int" value="16778578"/>
		<constant name="IBUS_Armenian_YECH" type="int" value="16778549"/>
		<constant name="IBUS_Armenian_ZA" type="int" value="16778550"/>
		<constant name="IBUS_Armenian_ZHE" type="int" value="16778554"/>
		<constant name="IBUS_Armenian_accent" type="int" value="16778587"/>
		<constant name="IBUS_Armenian_amanak" type="int" value="16778588"/>
		<constant name="IBUS_Armenian_apostrophe" type="int" value="16778586"/>
		<constant name="IBUS_Armenian_at" type="int" value="16778600"/>
		<constant name="IBUS_Armenian_ayb" type="int" value="16778593"/>
		<constant name="IBUS_Armenian_ben" type="int" value="16778594"/>
		<constant name="IBUS_Armenian_but" type="int" value="16778589"/>
		<constant name="IBUS_Armenian_cha" type="int" value="16778617"/>
		<constant name="IBUS_Armenian_da" type="int" value="16778596"/>
		<constant name="IBUS_Armenian_dza" type="int" value="16778609"/>
		<constant name="IBUS_Armenian_e" type="int" value="16778599"/>
		<constant name="IBUS_Armenian_exclam" type="int" value="16778588"/>
		<constant name="IBUS_Armenian_fe" type="int" value="16778630"/>
		<constant name="IBUS_Armenian_full_stop" type="int" value="16778633"/>
		<constant name="IBUS_Armenian_ghat" type="int" value="16778610"/>
		<constant name="IBUS_Armenian_gim" type="int" value="16778595"/>
		<constant name="IBUS_Armenian_hi" type="int" value="16778613"/>
		<constant name="IBUS_Armenian_ho" type="int" value="16778608"/>
		<constant name="IBUS_Armenian_hyphen" type="int" value="16778634"/>
		<constant name="IBUS_Armenian_ini" type="int" value="16778603"/>
		<constant name="IBUS_Armenian_je" type="int" value="16778619"/>
		<constant name="IBUS_Armenian_ke" type="int" value="16778628"/>
		<constant name="IBUS_Armenian_ken" type="int" value="16778607"/>
		<constant name="IBUS_Armenian_khe" type="int" value="16778605"/>
		<constant name="IBUS_Armenian_ligature_ew" type="int" value="16778631"/>
		<constant name="IBUS_Armenian_lyun" type="int" value="16778604"/>
		<constant name="IBUS_Armenian_men" type="int" value="16778612"/>
		<constant name="IBUS_Armenian_nu" type="int" value="16778614"/>
		<constant name="IBUS_Armenian_o" type="int" value="16778629"/>
		<constant name="IBUS_Armenian_paruyk" type="int" value="16778590"/>
		<constant name="IBUS_Armenian_pe" type="int" value="16778618"/>
		<constant name="IBUS_Armenian_pyur" type="int" value="16778627"/>
		<constant name="IBUS_Armenian_question" type="int" value="16778590"/>
		<constant name="IBUS_Armenian_ra" type="int" value="16778620"/>
		<constant name="IBUS_Armenian_re" type="int" value="16778624"/>
		<constant name="IBUS_Armenian_se" type="int" value="16778621"/>
		<constant name="IBUS_Armenian_separation_mark" type="int" value="16778589"/>
		<constant name="IBUS_Armenian_sha" type="int" value="16778615"/>
		<constant name="IBUS_Armenian_shesht" type="int" value="16778587"/>
		<constant name="IBUS_Armenian_tche" type="int" value="16778611"/>
		<constant name="IBUS_Armenian_to" type="int" value="16778601"/>
		<constant name="IBUS_Armenian_tsa" type="int" value="16778606"/>
		<constant name="IBUS_Armenian_tso" type="int" value="16778625"/>
		<constant name="IBUS_Armenian_tyun" type="int" value="16778623"/>
		<constant name="IBUS_Armenian_verjaket" type="int" value="16778633"/>
		<constant name="IBUS_Armenian_vev" type="int" value="16778622"/>
		<constant name="IBUS_Armenian_vo" type="int" value="16778616"/>
		<constant name="IBUS_Armenian_vyun" type="int" value="16778626"/>
		<constant name="IBUS_Armenian_yech" type="int" value="16778597"/>
		<constant name="IBUS_Armenian_yentamna" type="int" value="16778634"/>
		<constant name="IBUS_Armenian_za" type="int" value="16778598"/>
		<constant name="IBUS_Armenian_zhe" type="int" value="16778602"/>
		<constant name="IBUS_Atilde" type="int" value="195"/>
		<constant name="IBUS_AudibleBell_Enable" type="int" value="65146"/>
		<constant name="IBUS_B" type="int" value="66"/>
		<constant name="IBUS_Babovedot" type="int" value="16784898"/>
		<constant name="IBUS_BackSpace" type="int" value="65288"/>
		<constant name="IBUS_Begin" type="int" value="65368"/>
		<constant name="IBUS_BounceKeys_Enable" type="int" value="65140"/>
		<constant name="IBUS_Break" type="int" value="65387"/>
		<constant name="IBUS_Byelorussian_SHORTU" type="int" value="1726"/>
		<constant name="IBUS_Byelorussian_shortu" type="int" value="1710"/>
		<constant name="IBUS_C" type="int" value="67"/>
		<constant name="IBUS_Cabovedot" type="int" value="709"/>
		<constant name="IBUS_Cacute" type="int" value="454"/>
		<constant name="IBUS_Cancel" type="int" value="65385"/>
		<constant name="IBUS_Caps_Lock" type="int" value="65509"/>
		<constant name="IBUS_Ccaron" type="int" value="456"/>
		<constant name="IBUS_Ccedilla" type="int" value="199"/>
		<constant name="IBUS_Ccircumflex" type="int" value="710"/>
		<constant name="IBUS_Clear" type="int" value="65291"/>
		<constant name="IBUS_Codeinput" type="int" value="65335"/>
		<constant name="IBUS_ColonSign" type="int" value="16785569"/>
		<constant name="IBUS_Control_L" type="int" value="65507"/>
		<constant name="IBUS_Control_R" type="int" value="65508"/>
		<constant name="IBUS_CruzeiroSign" type="int" value="16785570"/>
		<constant name="IBUS_Cyrillic_A" type="int" value="1761"/>
		<constant name="IBUS_Cyrillic_BE" type="int" value="1762"/>
		<constant name="IBUS_Cyrillic_CHE" type="int" value="1790"/>
		<constant name="IBUS_Cyrillic_CHE_descender" type="int" value="16778422"/>
		<constant name="IBUS_Cyrillic_CHE_vertstroke" type="int" value="16778424"/>
		<constant name="IBUS_Cyrillic_DE" type="int" value="1764"/>
		<constant name="IBUS_Cyrillic_DZHE" type="int" value="1727"/>
		<constant name="IBUS_Cyrillic_E" type="int" value="1788"/>
		<constant name="IBUS_Cyrillic_EF" type="int" value="1766"/>
		<constant name="IBUS_Cyrillic_EL" type="int" value="1772"/>
		<constant name="IBUS_Cyrillic_EM" type="int" value="1773"/>
		<constant name="IBUS_Cyrillic_EN" type="int" value="1774"/>
		<constant name="IBUS_Cyrillic_EN_descender" type="int" value="16778402"/>
		<constant name="IBUS_Cyrillic_ER" type="int" value="1778"/>
		<constant name="IBUS_Cyrillic_ES" type="int" value="1779"/>
		<constant name="IBUS_Cyrillic_GHE" type="int" value="1767"/>
		<constant name="IBUS_Cyrillic_GHE_bar" type="int" value="16778386"/>
		<constant name="IBUS_Cyrillic_HA" type="int" value="1768"/>
		<constant name="IBUS_Cyrillic_HARDSIGN" type="int" value="1791"/>
		<constant name="IBUS_Cyrillic_HA_descender" type="int" value="16778418"/>
		<constant name="IBUS_Cyrillic_I" type="int" value="1769"/>
		<constant name="IBUS_Cyrillic_IE" type="int" value="1765"/>
		<constant name="IBUS_Cyrillic_IO" type="int" value="1715"/>
		<constant name="IBUS_Cyrillic_I_macron" type="int" value="16778466"/>
		<constant name="IBUS_Cyrillic_JE" type="int" value="1720"/>
		<constant name="IBUS_Cyrillic_KA" type="int" value="1771"/>
		<constant name="IBUS_Cyrillic_KA_descender" type="int" value="16778394"/>
		<constant name="IBUS_Cyrillic_KA_vertstroke" type="int" value="16778396"/>
		<constant name="IBUS_Cyrillic_LJE" type="int" value="1721"/>
		<constant name="IBUS_Cyrillic_NJE" type="int" value="1722"/>
		<constant name="IBUS_Cyrillic_O" type="int" value="1775"/>
		<constant name="IBUS_Cyrillic_O_bar" type="int" value="16778472"/>
		<constant name="IBUS_Cyrillic_PE" type="int" value="1776"/>
		<constant name="IBUS_Cyrillic_SCHWA" type="int" value="16778456"/>
		<constant name="IBUS_Cyrillic_SHA" type="int" value="1787"/>
		<constant name="IBUS_Cyrillic_SHCHA" type="int" value="1789"/>
		<constant name="IBUS_Cyrillic_SHHA" type="int" value="16778426"/>
		<constant name="IBUS_Cyrillic_SHORTI" type="int" value="1770"/>
		<constant name="IBUS_Cyrillic_SOFTSIGN" type="int" value="1784"/>
		<constant name="IBUS_Cyrillic_TE" type="int" value="1780"/>
		<constant name="IBUS_Cyrillic_TSE" type="int" value="1763"/>
		<constant name="IBUS_Cyrillic_U" type="int" value="1781"/>
		<constant name="IBUS_Cyrillic_U_macron" type="int" value="16778478"/>
		<constant name="IBUS_Cyrillic_U_straight" type="int" value="16778414"/>
		<constant name="IBUS_Cyrillic_U_straight_bar" type="int" value="16778416"/>
		<constant name="IBUS_Cyrillic_VE" type="int" value="1783"/>
		<constant name="IBUS_Cyrillic_YA" type="int" value="1777"/>
		<constant name="IBUS_Cyrillic_YERU" type="int" value="1785"/>
		<constant name="IBUS_Cyrillic_YU" type="int" value="1760"/>
		<constant name="IBUS_Cyrillic_ZE" type="int" value="1786"/>
		<constant name="IBUS_Cyrillic_ZHE" type="int" value="1782"/>
		<constant name="IBUS_Cyrillic_ZHE_descender" type="int" value="16778390"/>
		<constant name="IBUS_Cyrillic_a" type="int" value="1729"/>
		<constant name="IBUS_Cyrillic_be" type="int" value="1730"/>
		<constant name="IBUS_Cyrillic_che" type="int" value="1758"/>
		<constant name="IBUS_Cyrillic_che_descender" type="int" value="16778423"/>
		<constant name="IBUS_Cyrillic_che_vertstroke" type="int" value="16778425"/>
		<constant name="IBUS_Cyrillic_de" type="int" value="1732"/>
		<constant name="IBUS_Cyrillic_dzhe" type="int" value="1711"/>
		<constant name="IBUS_Cyrillic_e" type="int" value="1756"/>
		<constant name="IBUS_Cyrillic_ef" type="int" value="1734"/>
		<constant name="IBUS_Cyrillic_el" type="int" value="1740"/>
		<constant name="IBUS_Cyrillic_em" type="int" value="1741"/>
		<constant name="IBUS_Cyrillic_en" type="int" value="1742"/>
		<constant name="IBUS_Cyrillic_en_descender" type="int" value="16778403"/>
		<constant name="IBUS_Cyrillic_er" type="int" value="1746"/>
		<constant name="IBUS_Cyrillic_es" type="int" value="1747"/>
		<constant name="IBUS_Cyrillic_ghe" type="int" value="1735"/>
		<constant name="IBUS_Cyrillic_ghe_bar" type="int" value="16778387"/>
		<constant name="IBUS_Cyrillic_ha" type="int" value="1736"/>
		<constant name="IBUS_Cyrillic_ha_descender" type="int" value="16778419"/>
		<constant name="IBUS_Cyrillic_hardsign" type="int" value="1759"/>
		<constant name="IBUS_Cyrillic_i" type="int" value="1737"/>
		<constant name="IBUS_Cyrillic_i_macron" type="int" value="16778467"/>
		<constant name="IBUS_Cyrillic_ie" type="int" value="1733"/>
		<constant name="IBUS_Cyrillic_io" type="int" value="1699"/>
		<constant name="IBUS_Cyrillic_je" type="int" value="1704"/>
		<constant name="IBUS_Cyrillic_ka" type="int" value="1739"/>
		<constant name="IBUS_Cyrillic_ka_descender" type="int" value="16778395"/>
		<constant name="IBUS_Cyrillic_ka_vertstroke" type="int" value="16778397"/>
		<constant name="IBUS_Cyrillic_lje" type="int" value="1705"/>
		<constant name="IBUS_Cyrillic_nje" type="int" value="1706"/>
		<constant name="IBUS_Cyrillic_o" type="int" value="1743"/>
		<constant name="IBUS_Cyrillic_o_bar" type="int" value="16778473"/>
		<constant name="IBUS_Cyrillic_pe" type="int" value="1744"/>
		<constant name="IBUS_Cyrillic_schwa" type="int" value="16778457"/>
		<constant name="IBUS_Cyrillic_sha" type="int" value="1755"/>
		<constant name="IBUS_Cyrillic_shcha" type="int" value="1757"/>
		<constant name="IBUS_Cyrillic_shha" type="int" value="16778427"/>
		<constant name="IBUS_Cyrillic_shorti" type="int" value="1738"/>
		<constant name="IBUS_Cyrillic_softsign" type="int" value="1752"/>
		<constant name="IBUS_Cyrillic_te" type="int" value="1748"/>
		<constant name="IBUS_Cyrillic_tse" type="int" value="1731"/>
		<constant name="IBUS_Cyrillic_u" type="int" value="1749"/>
		<constant name="IBUS_Cyrillic_u_macron" type="int" value="16778479"/>
		<constant name="IBUS_Cyrillic_u_straight" type="int" value="16778415"/>
		<constant name="IBUS_Cyrillic_u_straight_bar" type="int" value="16778417"/>
		<constant name="IBUS_Cyrillic_ve" type="int" value="1751"/>
		<constant name="IBUS_Cyrillic_ya" type="int" value="1745"/>
		<constant name="IBUS_Cyrillic_yeru" type="int" value="1753"/>
		<constant name="IBUS_Cyrillic_yu" type="int" value="1728"/>
		<constant name="IBUS_Cyrillic_ze" type="int" value="1754"/>
		<constant name="IBUS_Cyrillic_zhe" type="int" value="1750"/>
		<constant name="IBUS_Cyrillic_zhe_descender" type="int" value="16778391"/>
		<constant name="IBUS_D" type="int" value="68"/>
		<constant name="IBUS_Dabovedot" type="int" value="16784906"/>
		<constant name="IBUS_Dcaron" type="int" value="463"/>
		<constant name="IBUS_Delete" type="int" value="65535"/>
		<constant name="IBUS_DongSign" type="int" value="16785579"/>
		<constant name="IBUS_Down" type="int" value="65364"/>
		<constant name="IBUS_Dstroke" type="int" value="464"/>
		<constant name="IBUS_E" type="int" value="69"/>
		<constant name="IBUS_ENG" type="int" value="957"/>
		<constant name="IBUS_ETH" type="int" value="208"/>
		<constant name="IBUS_Eabovedot" type="int" value="972"/>
		<constant name="IBUS_Eacute" type="int" value="201"/>
		<constant name="IBUS_Ebelowdot" type="int" value="16785080"/>
		<constant name="IBUS_Ecaron" type="int" value="460"/>
		<constant name="IBUS_Ecircumflex" type="int" value="202"/>
		<constant name="IBUS_Ecircumflexacute" type="int" value="16785086"/>
		<constant name="IBUS_Ecircumflexbelowdot" type="int" value="16785094"/>
		<constant name="IBUS_Ecircumflexgrave" type="int" value="16785088"/>
		<constant name="IBUS_Ecircumflexhook" type="int" value="16785090"/>
		<constant name="IBUS_Ecircumflextilde" type="int" value="16785092"/>
		<constant name="IBUS_EcuSign" type="int" value="16785568"/>
		<constant name="IBUS_Ediaeresis" type="int" value="203"/>
		<constant name="IBUS_Egrave" type="int" value="200"/>
		<constant name="IBUS_Ehook" type="int" value="16785082"/>
		<constant name="IBUS_Eisu_Shift" type="int" value="65327"/>
		<constant name="IBUS_Eisu_toggle" type="int" value="65328"/>
		<constant name="IBUS_Emacron" type="int" value="938"/>
		<constant name="IBUS_End" type="int" value="65367"/>
		<constant name="IBUS_Eogonek" type="int" value="458"/>
		<constant name="IBUS_Escape" type="int" value="65307"/>
		<constant name="IBUS_Eth" type="int" value="208"/>
		<constant name="IBUS_Etilde" type="int" value="16785084"/>
		<constant name="IBUS_EuroSign" type="int" value="8364"/>
		<constant name="IBUS_Execute" type="int" value="65378"/>
		<constant name="IBUS_F" type="int" value="70"/>
		<constant name="IBUS_F1" type="int" value="65470"/>
		<constant name="IBUS_F10" type="int" value="65479"/>
		<constant name="IBUS_F11" type="int" value="65480"/>
		<constant name="IBUS_F12" type="int" value="65481"/>
		<constant name="IBUS_F13" type="int" value="65482"/>
		<constant name="IBUS_F14" type="int" value="65483"/>
		<constant name="IBUS_F15" type="int" value="65484"/>
		<constant name="IBUS_F16" type="int" value="65485"/>
		<constant name="IBUS_F17" type="int" value="65486"/>
		<constant name="IBUS_F18" type="int" value="65487"/>
		<constant name="IBUS_F19" type="int" value="65488"/>
		<constant name="IBUS_F2" type="int" value="65471"/>
		<constant name="IBUS_F20" type="int" value="65489"/>
		<constant name="IBUS_F21" type="int" value="65490"/>
		<constant name="IBUS_F22" type="int" value="65491"/>
		<constant name="IBUS_F23" type="int" value="65492"/>
		<constant name="IBUS_F24" type="int" value="65493"/>
		<constant name="IBUS_F25" type="int" value="65494"/>
		<constant name="IBUS_F26" type="int" value="65495"/>
		<constant name="IBUS_F27" type="int" value="65496"/>
		<constant name="IBUS_F28" type="int" value="65497"/>
		<constant name="IBUS_F29" type="int" value="65498"/>
		<constant name="IBUS_F3" type="int" value="65472"/>
		<constant name="IBUS_F30" type="int" value="65499"/>
		<constant name="IBUS_F31" type="int" value="65500"/>
		<constant name="IBUS_F32" type="int" value="65501"/>
		<constant name="IBUS_F33" type="int" value="65502"/>
		<constant name="IBUS_F34" type="int" value="65503"/>
		<constant name="IBUS_F35" type="int" value="65504"/>
		<constant name="IBUS_F4" type="int" value="65473"/>
		<constant name="IBUS_F5" type="int" value="65474"/>
		<constant name="IBUS_F6" type="int" value="65475"/>
		<constant name="IBUS_F7" type="int" value="65476"/>
		<constant name="IBUS_F8" type="int" value="65477"/>
		<constant name="IBUS_F9" type="int" value="65478"/>
		<constant name="IBUS_FFrancSign" type="int" value="16785571"/>
		<constant name="IBUS_Fabovedot" type="int" value="16784926"/>
		<constant name="IBUS_Farsi_0" type="int" value="16778992"/>
		<constant name="IBUS_Farsi_1" type="int" value="16778993"/>
		<constant name="IBUS_Farsi_2" type="int" value="16778994"/>
		<constant name="IBUS_Farsi_3" type="int" value="16778995"/>
		<constant name="IBUS_Farsi_4" type="int" value="16778996"/>
		<constant name="IBUS_Farsi_5" type="int" value="16778997"/>
		<constant name="IBUS_Farsi_6" type="int" value="16778998"/>
		<constant name="IBUS_Farsi_7" type="int" value="16778999"/>
		<constant name="IBUS_Farsi_8" type="int" value="16779000"/>
		<constant name="IBUS_Farsi_9" type="int" value="16779001"/>
		<constant name="IBUS_Farsi_yeh" type="int" value="16778956"/>
		<constant name="IBUS_Find" type="int" value="65384"/>
		<constant name="IBUS_First_Virtual_Screen" type="int" value="65232"/>
		<constant name="IBUS_G" type="int" value="71"/>
		<constant name="IBUS_Gabovedot" type="int" value="725"/>
		<constant name="IBUS_Gbreve" type="int" value="683"/>
		<constant name="IBUS_Gcaron" type="int" value="16777702"/>
		<constant name="IBUS_Gcedilla" type="int" value="939"/>
		<constant name="IBUS_Gcircumflex" type="int" value="728"/>
		<constant name="IBUS_Georgian_an" type="int" value="16781520"/>
		<constant name="IBUS_Georgian_ban" type="int" value="16781521"/>
		<constant name="IBUS_Georgian_can" type="int" value="16781546"/>
		<constant name="IBUS_Georgian_char" type="int" value="16781549"/>
		<constant name="IBUS_Georgian_chin" type="int" value="16781545"/>
		<constant name="IBUS_Georgian_cil" type="int" value="16781548"/>
		<constant name="IBUS_Georgian_don" type="int" value="16781523"/>
		<constant name="IBUS_Georgian_en" type="int" value="16781524"/>
		<constant name="IBUS_Georgian_fi" type="int" value="16781558"/>
		<constant name="IBUS_Georgian_gan" type="int" value="16781522"/>
		<constant name="IBUS_Georgian_ghan" type="int" value="16781542"/>
		<constant name="IBUS_Georgian_hae" type="int" value="16781552"/>
		<constant name="IBUS_Georgian_har" type="int" value="16781556"/>
		<constant name="IBUS_Georgian_he" type="int" value="16781553"/>
		<constant name="IBUS_Georgian_hie" type="int" value="16781554"/>
		<constant name="IBUS_Georgian_hoe" type="int" value="16781557"/>
		<constant name="IBUS_Georgian_in" type="int" value="16781528"/>
		<constant name="IBUS_Georgian_jhan" type="int" value="16781551"/>
		<constant name="IBUS_Georgian_jil" type="int" value="16781547"/>
		<constant name="IBUS_Georgian_kan" type="int" value="16781529"/>
		<constant name="IBUS_Georgian_khar" type="int" value="16781541"/>
		<constant name="IBUS_Georgian_las" type="int" value="16781530"/>
		<constant name="IBUS_Georgian_man" type="int" value="16781531"/>
		<constant name="IBUS_Georgian_nar" type="int" value="16781532"/>
		<constant name="IBUS_Georgian_on" type="int" value="16781533"/>
		<constant name="IBUS_Georgian_par" type="int" value="16781534"/>
		<constant name="IBUS_Georgian_phar" type="int" value="16781540"/>
		<constant name="IBUS_Georgian_qar" type="int" value="16781543"/>
		<constant name="IBUS_Georgian_rae" type="int" value="16781536"/>
		<constant name="IBUS_Georgian_san" type="int" value="16781537"/>
		<constant name="IBUS_Georgian_shin" type="int" value="16781544"/>
		<constant name="IBUS_Georgian_tan" type="int" value="16781527"/>
		<constant name="IBUS_Georgian_tar" type="int" value="16781538"/>
		<constant name="IBUS_Georgian_un" type="int" value="16781539"/>
		<constant name="IBUS_Georgian_vin" type="int" value="16781525"/>
		<constant name="IBUS_Georgian_we" type="int" value="16781555"/>
		<constant name="IBUS_Georgian_xan" type="int" value="16781550"/>
		<constant name="IBUS_Georgian_zen" type="int" value="16781526"/>
		<constant name="IBUS_Georgian_zhar" type="int" value="16781535"/>
		<constant name="IBUS_Greek_ALPHA" type="int" value="1985"/>
		<constant name="IBUS_Greek_ALPHAaccent" type="int" value="1953"/>
		<constant name="IBUS_Greek_BETA" type="int" value="1986"/>
		<constant name="IBUS_Greek_CHI" type="int" value="2007"/>
		<constant name="IBUS_Greek_DELTA" type="int" value="1988"/>
		<constant name="IBUS_Greek_EPSILON" type="int" value="1989"/>
		<constant name="IBUS_Greek_EPSILONaccent" type="int" value="1954"/>
		<constant name="IBUS_Greek_ETA" type="int" value="1991"/>
		<constant name="IBUS_Greek_ETAaccent" type="int" value="1955"/>
		<constant name="IBUS_Greek_GAMMA" type="int" value="1987"/>
		<constant name="IBUS_Greek_IOTA" type="int" value="1993"/>
		<constant name="IBUS_Greek_IOTAaccent" type="int" value="1956"/>
		<constant name="IBUS_Greek_IOTAdiaeresis" type="int" value="1957"/>
		<constant name="IBUS_Greek_IOTAdieresis" type="int" value="1957"/>
		<constant name="IBUS_Greek_KAPPA" type="int" value="1994"/>
		<constant name="IBUS_Greek_LAMBDA" type="int" value="1995"/>
		<constant name="IBUS_Greek_LAMDA" type="int" value="1995"/>
		<constant name="IBUS_Greek_MU" type="int" value="1996"/>
		<constant name="IBUS_Greek_NU" type="int" value="1997"/>
		<constant name="IBUS_Greek_OMEGA" type="int" value="2009"/>
		<constant name="IBUS_Greek_OMEGAaccent" type="int" value="1963"/>
		<constant name="IBUS_Greek_OMICRON" type="int" value="1999"/>
		<constant name="IBUS_Greek_OMICRONaccent" type="int" value="1959"/>
		<constant name="IBUS_Greek_PHI" type="int" value="2006"/>
		<constant name="IBUS_Greek_PI" type="int" value="2000"/>
		<constant name="IBUS_Greek_PSI" type="int" value="2008"/>
		<constant name="IBUS_Greek_RHO" type="int" value="2001"/>
		<constant name="IBUS_Greek_SIGMA" type="int" value="2002"/>
		<constant name="IBUS_Greek_TAU" type="int" value="2004"/>
		<constant name="IBUS_Greek_THETA" type="int" value="1992"/>
		<constant name="IBUS_Greek_UPSILON" type="int" value="2005"/>
		<constant name="IBUS_Greek_UPSILONaccent" type="int" value="1960"/>
		<constant name="IBUS_Greek_UPSILONdieresis" type="int" value="1961"/>
		<constant name="IBUS_Greek_XI" type="int" value="1998"/>
		<constant name="IBUS_Greek_ZETA" type="int" value="1990"/>
		<constant name="IBUS_Greek_accentdieresis" type="int" value="1966"/>
		<constant name="IBUS_Greek_alpha" type="int" value="2017"/>
		<constant name="IBUS_Greek_alphaaccent" type="int" value="1969"/>
		<constant name="IBUS_Greek_beta" type="int" value="2018"/>
		<constant name="IBUS_Greek_chi" type="int" value="2039"/>
		<constant name="IBUS_Greek_delta" type="int" value="2020"/>
		<constant name="IBUS_Greek_epsilon" type="int" value="2021"/>
		<constant name="IBUS_Greek_epsilonaccent" type="int" value="1970"/>
		<constant name="IBUS_Greek_eta" type="int" value="2023"/>
		<constant name="IBUS_Greek_etaaccent" type="int" value="1971"/>
		<constant name="IBUS_Greek_finalsmallsigma" type="int" value="2035"/>
		<constant name="IBUS_Greek_gamma" type="int" value="2019"/>
		<constant name="IBUS_Greek_horizbar" type="int" value="1967"/>
		<constant name="IBUS_Greek_iota" type="int" value="2025"/>
		<constant name="IBUS_Greek_iotaaccent" type="int" value="1972"/>
		<constant name="IBUS_Greek_iotaaccentdieresis" type="int" value="1974"/>
		<constant name="IBUS_Greek_iotadieresis" type="int" value="1973"/>
		<constant name="IBUS_Greek_kappa" type="int" value="2026"/>
		<constant name="IBUS_Greek_lambda" type="int" value="2027"/>
		<constant name="IBUS_Greek_lamda" type="int" value="2027"/>
		<constant name="IBUS_Greek_mu" type="int" value="2028"/>
		<constant name="IBUS_Greek_nu" type="int" value="2029"/>
		<constant name="IBUS_Greek_omega" type="int" value="2041"/>
		<constant name="IBUS_Greek_omegaaccent" type="int" value="1979"/>
		<constant name="IBUS_Greek_omicron" type="int" value="2031"/>
		<constant name="IBUS_Greek_omicronaccent" type="int" value="1975"/>
		<constant name="IBUS_Greek_phi" type="int" value="2038"/>
		<constant name="IBUS_Greek_pi" type="int" value="2032"/>
		<constant name="IBUS_Greek_psi" type="int" value="2040"/>
		<constant name="IBUS_Greek_rho" type="int" value="2033"/>
		<constant name="IBUS_Greek_sigma" type="int" value="2034"/>
		<constant name="IBUS_Greek_switch" type="int" value="65406"/>
		<constant name="IBUS_Greek_tau" type="int" value="2036"/>
		<constant name="IBUS_Greek_theta" type="int" value="2024"/>
		<constant name="IBUS_Greek_upsilon" type="int" value="2037"/>
		<constant name="IBUS_Greek_upsilonaccent" type="int" value="1976"/>
		<constant name="IBUS_Greek_upsilonaccentdieresis" type="int" value="1978"/>
		<constant name="IBUS_Greek_upsilondieresis" type="int" value="1977"/>
		<constant name="IBUS_Greek_xi" type="int" value="2030"/>
		<constant name="IBUS_Greek_zeta" type="int" value="2022"/>
		<constant name="IBUS_H" type="int" value="72"/>
		<constant name="IBUS_Hangul" type="int" value="65329"/>
		<constant name="IBUS_Hangul_A" type="int" value="3775"/>
		<constant name="IBUS_Hangul_AE" type="int" value="3776"/>
		<constant name="IBUS_Hangul_AraeA" type="int" value="3830"/>
		<constant name="IBUS_Hangul_AraeAE" type="int" value="3831"/>
		<constant name="IBUS_Hangul_Banja" type="int" value="65337"/>
		<constant name="IBUS_Hangul_Cieuc" type="int" value="3770"/>
		<constant name="IBUS_Hangul_Codeinput" type="int" value="65335"/>
		<constant name="IBUS_Hangul_Dikeud" type="int" value="3751"/>
		<constant name="IBUS_Hangul_E" type="int" value="3780"/>
		<constant name="IBUS_Hangul_EO" type="int" value="3779"/>
		<constant name="IBUS_Hangul_EU" type="int" value="3793"/>
		<constant name="IBUS_Hangul_End" type="int" value="65331"/>
		<constant name="IBUS_Hangul_Hanja" type="int" value="65332"/>
		<constant name="IBUS_Hangul_Hieuh" type="int" value="3774"/>
		<constant name="IBUS_Hangul_I" type="int" value="3795"/>
		<constant name="IBUS_Hangul_Ieung" type="int" value="3767"/>
		<constant name="IBUS_Hangul_J_Cieuc" type="int" value="3818"/>
		<constant name="IBUS_Hangul_J_Dikeud" type="int" value="3802"/>
		<constant name="IBUS_Hangul_J_Hieuh" type="int" value="3822"/>
		<constant name="IBUS_Hangul_J_Ieung" type="int" value="3816"/>
		<constant name="IBUS_Hangul_J_Jieuj" type="int" value="3817"/>
		<constant name="IBUS_Hangul_J_Khieuq" type="int" value="3819"/>
		<constant name="IBUS_Hangul_J_Kiyeog" type="int" value="3796"/>
		<constant name="IBUS_Hangul_J_KiyeogSios" type="int" value="3798"/>
		<constant name="IBUS_Hangul_J_KkogjiDalrinIeung" type="int" value="3833"/>
		<constant name="IBUS_Hangul_J_Mieum" type="int" value="3811"/>
		<constant name="IBUS_Hangul_J_Nieun" type="int" value="3799"/>
		<constant name="IBUS_Hangul_J_NieunHieuh" type="int" value="3801"/>
		<constant name="IBUS_Hangul_J_NieunJieuj" type="int" value="3800"/>
		<constant name="IBUS_Hangul_J_PanSios" type="int" value="3832"/>
		<constant name="IBUS_Hangul_J_Phieuf" type="int" value="3821"/>
		<constant name="IBUS_Hangul_J_Pieub" type="int" value="3812"/>
		<constant name="IBUS_Hangul_J_PieubSios" type="int" value="3813"/>
		<constant name="IBUS_Hangul_J_Rieul" type="int" value="3803"/>
		<constant name="IBUS_Hangul_J_RieulHieuh" type="int" value="3810"/>
		<constant name="IBUS_Hangul_J_RieulKiyeog" type="int" value="3804"/>
		<constant name="IBUS_Hangul_J_RieulMieum" type="int" value="3805"/>
		<constant name="IBUS_Hangul_J_RieulPhieuf" type="int" value="3809"/>
		<constant name="IBUS_Hangul_J_RieulPieub" type="int" value="3806"/>
		<constant name="IBUS_Hangul_J_RieulSios" type="int" value="3807"/>
		<constant name="IBUS_Hangul_J_RieulTieut" type="int" value="3808"/>
		<constant name="IBUS_Hangul_J_Sios" type="int" value="3814"/>
		<constant name="IBUS_Hangul_J_SsangKiyeog" type="int" value="3797"/>
		<constant name="IBUS_Hangul_J_SsangSios" type="int" value="3815"/>
		<constant name="IBUS_Hangul_J_Tieut" type="int" value="3820"/>
		<constant name="IBUS_Hangul_J_YeorinHieuh" type="int" value="3834"/>
		<constant name="IBUS_Hangul_Jamo" type="int" value="65333"/>
		<constant name="IBUS_Hangul_Jeonja" type="int" value="65336"/>
		<constant name="IBUS_Hangul_Jieuj" type="int" value="3768"/>
		<constant name="IBUS_Hangul_Khieuq" type="int" value="3771"/>
		<constant name="IBUS_Hangul_Kiyeog" type="int" value="3745"/>
		<constant name="IBUS_Hangul_KiyeogSios" type="int" value="3747"/>
		<constant name="IBUS_Hangul_KkogjiDalrinIeung" type="int" value="3827"/>
		<constant name="IBUS_Hangul_Mieum" type="int" value="3761"/>
		<constant name="IBUS_Hangul_MultipleCandidate" type="int" value="65341"/>
		<constant name="IBUS_Hangul_Nieun" type="int" value="3748"/>
		<constant name="IBUS_Hangul_NieunHieuh" type="int" value="3750"/>
		<constant name="IBUS_Hangul_NieunJieuj" type="int" value="3749"/>
		<constant name="IBUS_Hangul_O" type="int" value="3783"/>
		<constant name="IBUS_Hangul_OE" type="int" value="3786"/>
		<constant name="IBUS_Hangul_PanSios" type="int" value="3826"/>
		<constant name="IBUS_Hangul_Phieuf" type="int" value="3773"/>
		<constant name="IBUS_Hangul_Pieub" type="int" value="3762"/>
		<constant name="IBUS_Hangul_PieubSios" type="int" value="3764"/>
		<constant name="IBUS_Hangul_PostHanja" type="int" value="65339"/>
		<constant name="IBUS_Hangul_PreHanja" type="int" value="65338"/>
		<constant name="IBUS_Hangul_PreviousCandidate" type="int" value="65342"/>
		<constant name="IBUS_Hangul_Rieul" type="int" value="3753"/>
		<constant name="IBUS_Hangul_RieulHieuh" type="int" value="3760"/>
		<constant name="IBUS_Hangul_RieulKiyeog" type="int" value="3754"/>
		<constant name="IBUS_Hangul_RieulMieum" type="int" value="3755"/>
		<constant name="IBUS_Hangul_RieulPhieuf" type="int" value="3759"/>
		<constant name="IBUS_Hangul_RieulPieub" type="int" value="3756"/>
		<constant name="IBUS_Hangul_RieulSios" type="int" value="3757"/>
		<constant name="IBUS_Hangul_RieulTieut" type="int" value="3758"/>
		<constant name="IBUS_Hangul_RieulYeorinHieuh" type="int" value="3823"/>
		<constant name="IBUS_Hangul_Romaja" type="int" value="65334"/>
		<constant name="IBUS_Hangul_SingleCandidate" type="int" value="65340"/>
		<constant name="IBUS_Hangul_Sios" type="int" value="3765"/>
		<constant name="IBUS_Hangul_Special" type="int" value="65343"/>
		<constant name="IBUS_Hangul_SsangDikeud" type="int" value="3752"/>
		<constant name="IBUS_Hangul_SsangJieuj" type="int" value="3769"/>
		<constant name="IBUS_Hangul_SsangKiyeog" type="int" value="3746"/>
		<constant name="IBUS_Hangul_SsangPieub" type="int" value="3763"/>
		<constant name="IBUS_Hangul_SsangSios" type="int" value="3766"/>
		<constant name="IBUS_Hangul_Start" type="int" value="65330"/>
		<constant name="IBUS_Hangul_SunkyeongeumMieum" type="int" value="3824"/>
		<constant name="IBUS_Hangul_SunkyeongeumPhieuf" type="int" value="3828"/>
		<constant name="IBUS_Hangul_SunkyeongeumPieub" type="int" value="3825"/>
		<constant name="IBUS_Hangul_Tieut" type="int" value="3772"/>
		<constant name="IBUS_Hangul_U" type="int" value="3788"/>
		<constant name="IBUS_Hangul_WA" type="int" value="3784"/>
		<constant name="IBUS_Hangul_WAE" type="int" value="3785"/>
		<constant name="IBUS_Hangul_WE" type="int" value="3790"/>
		<constant name="IBUS_Hangul_WEO" type="int" value="3789"/>
		<constant name="IBUS_Hangul_WI" type="int" value="3791"/>
		<constant name="IBUS_Hangul_YA" type="int" value="3777"/>
		<constant name="IBUS_Hangul_YAE" type="int" value="3778"/>
		<constant name="IBUS_Hangul_YE" type="int" value="3782"/>
		<constant name="IBUS_Hangul_YEO" type="int" value="3781"/>
		<constant name="IBUS_Hangul_YI" type="int" value="3794"/>
		<constant name="IBUS_Hangul_YO" type="int" value="3787"/>
		<constant name="IBUS_Hangul_YU" type="int" value="3792"/>
		<constant name="IBUS_Hangul_YeorinHieuh" type="int" value="3829"/>
		<constant name="IBUS_Hangul_switch" type="int" value="65406"/>
		<constant name="IBUS_Hankaku" type="int" value="65321"/>
		<constant name="IBUS_Hcircumflex" type="int" value="678"/>
		<constant name="IBUS_Hebrew_switch" type="int" value="65406"/>
		<constant name="IBUS_Help" type="int" value="65386"/>
		<constant name="IBUS_Henkan" type="int" value="65315"/>
		<constant name="IBUS_Henkan_Mode" type="int" value="65315"/>
		<constant name="IBUS_Hiragana" type="int" value="65317"/>
		<constant name="IBUS_Hiragana_Katakana" type="int" value="65319"/>
		<constant name="IBUS_Home" type="int" value="65360"/>
		<constant name="IBUS_Hstroke" type="int" value="673"/>
		<constant name="IBUS_Hyper_L" type="int" value="65517"/>
		<constant name="IBUS_Hyper_R" type="int" value="65518"/>
		<constant name="IBUS_I" type="int" value="73"/>
		<constant name="IBUS_INTERFACE_CONFIG" type="char*" value="org.freedesktop.IBus.Config"/>
		<constant name="IBUS_INTERFACE_ENGINE" type="char*" value="org.freedesktop.IBus.Engine"/>
		<constant name="IBUS_INTERFACE_FACTORY" type="char*" value="org.freedesktop.IBus.Factory"/>
		<constant name="IBUS_INTERFACE_IBUS" type="char*" value="org.freedesktop.IBus"/>
		<constant name="IBUS_INTERFACE_INPUT_CONTEXT" type="char*" value="org.freedesktop.IBus.InputContext"/>
		<constant name="IBUS_INTERFACE_NOTIFICATIONS" type="char*" value="org.freedesktop.IBus.Notifications"/>
		<constant name="IBUS_INTERFACE_PANEL" type="char*" value="org.freedesktop.IBus.Panel"/>
		<constant name="IBUS_ISO_Center_Object" type="int" value="65075"/>
		<constant name="IBUS_ISO_Continuous_Underline" type="int" value="65072"/>
		<constant name="IBUS_ISO_Discontinuous_Underline" type="int" value="65073"/>
		<constant name="IBUS_ISO_Emphasize" type="int" value="65074"/>
		<constant name="IBUS_ISO_Enter" type="int" value="65076"/>
		<constant name="IBUS_ISO_Fast_Cursor_Down" type="int" value="65071"/>
		<constant name="IBUS_ISO_Fast_Cursor_Left" type="int" value="65068"/>
		<constant name="IBUS_ISO_Fast_Cursor_Right" type="int" value="65069"/>
		<constant name="IBUS_ISO_Fast_Cursor_Up" type="int" value="65070"/>
		<constant name="IBUS_ISO_First_Group" type="int" value="65036"/>
		<constant name="IBUS_ISO_First_Group_Lock" type="int" value="65037"/>
		<constant name="IBUS_ISO_Group_Latch" type="int" value="65030"/>
		<constant name="IBUS_ISO_Group_Lock" type="int" value="65031"/>
		<constant name="IBUS_ISO_Group_Shift" type="int" value="65406"/>
		<constant name="IBUS_ISO_Last_Group" type="int" value="65038"/>
		<constant name="IBUS_ISO_Last_Group_Lock" type="int" value="65039"/>
		<constant name="IBUS_ISO_Left_Tab" type="int" value="65056"/>
		<constant name="IBUS_ISO_Level2_Latch" type="int" value="65026"/>
		<constant name="IBUS_ISO_Level3_Latch" type="int" value="65028"/>
		<constant name="IBUS_ISO_Level3_Lock" type="int" value="65029"/>
		<constant name="IBUS_ISO_Level3_Shift" type="int" value="65027"/>
		<constant name="IBUS_ISO_Level5_Latch" type="int" value="65042"/>
		<constant name="IBUS_ISO_Level5_Lock" type="int" value="65043"/>
		<constant name="IBUS_ISO_Level5_Shift" type="int" value="65041"/>
		<constant name="IBUS_ISO_Lock" type="int" value="65025"/>
		<constant name="IBUS_ISO_Move_Line_Down" type="int" value="65058"/>
		<constant name="IBUS_ISO_Move_Line_Up" type="int" value="65057"/>
		<constant name="IBUS_ISO_Next_Group" type="int" value="65032"/>
		<constant name="IBUS_ISO_Next_Group_Lock" type="int" value="65033"/>
		<constant name="IBUS_ISO_Partial_Line_Down" type="int" value="65060"/>
		<constant name="IBUS_ISO_Partial_Line_Up" type="int" value="65059"/>
		<constant name="IBUS_ISO_Partial_Space_Left" type="int" value="65061"/>
		<constant name="IBUS_ISO_Partial_Space_Right" type="int" value="65062"/>
		<constant name="IBUS_ISO_Prev_Group" type="int" value="65034"/>
		<constant name="IBUS_ISO_Prev_Group_Lock" type="int" value="65035"/>
		<constant name="IBUS_ISO_Release_Both_Margins" type="int" value="65067"/>
		<constant name="IBUS_ISO_Release_Margin_Left" type="int" value="65065"/>
		<constant name="IBUS_ISO_Release_Margin_Right" type="int" value="65066"/>
		<constant name="IBUS_ISO_Set_Margin_Left" type="int" value="65063"/>
		<constant name="IBUS_ISO_Set_Margin_Right" type="int" value="65064"/>
		<constant name="IBUS_Iabovedot" type="int" value="681"/>
		<constant name="IBUS_Iacute" type="int" value="205"/>
		<constant name="IBUS_Ibelowdot" type="int" value="16785098"/>
		<constant name="IBUS_Ibreve" type="int" value="16777516"/>
		<constant name="IBUS_Icircumflex" type="int" value="206"/>
		<constant name="IBUS_Idiaeresis" type="int" value="207"/>
		<constant name="IBUS_Igrave" type="int" value="204"/>
		<constant name="IBUS_Ihook" type="int" value="16785096"/>
		<constant name="IBUS_Imacron" type="int" value="975"/>
		<constant name="IBUS_Insert" type="int" value="65379"/>
		<constant name="IBUS_Iogonek" type="int" value="967"/>
		<constant name="IBUS_Itilde" type="int" value="933"/>
		<constant name="IBUS_J" type="int" value="74"/>
		<constant name="IBUS_Jcircumflex" type="int" value="684"/>
		<constant name="IBUS_K" type="int" value="75"/>
		<constant name="IBUS_KP_0" type="int" value="65456"/>
		<constant name="IBUS_KP_1" type="int" value="65457"/>
		<constant name="IBUS_KP_2" type="int" value="65458"/>
		<constant name="IBUS_KP_3" type="int" value="65459"/>
		<constant name="IBUS_KP_4" type="int" value="65460"/>
		<constant name="IBUS_KP_5" type="int" value="65461"/>
		<constant name="IBUS_KP_6" type="int" value="65462"/>
		<constant name="IBUS_KP_7" type="int" value="65463"/>
		<constant name="IBUS_KP_8" type="int" value="65464"/>
		<constant name="IBUS_KP_9" type="int" value="65465"/>
		<constant name="IBUS_KP_Add" type="int" value="65451"/>
		<constant name="IBUS_KP_Begin" type="int" value="65437"/>
		<constant name="IBUS_KP_Decimal" type="int" value="65454"/>
		<constant name="IBUS_KP_Delete" type="int" value="65439"/>
		<constant name="IBUS_KP_Divide" type="int" value="65455"/>
		<constant name="IBUS_KP_Down" type="int" value="65433"/>
		<constant name="IBUS_KP_End" type="int" value="65436"/>
		<constant name="IBUS_KP_Enter" type="int" value="65421"/>
		<constant name="IBUS_KP_Equal" type="int" value="65469"/>
		<constant name="IBUS_KP_F1" type="int" value="65425"/>
		<constant name="IBUS_KP_F2" type="int" value="65426"/>
		<constant name="IBUS_KP_F3" type="int" value="65427"/>
		<constant name="IBUS_KP_F4" type="int" value="65428"/>
		<constant name="IBUS_KP_Home" type="int" value="65429"/>
		<constant name="IBUS_KP_Insert" type="int" value="65438"/>
		<constant name="IBUS_KP_Left" type="int" value="65430"/>
		<constant name="IBUS_KP_Multiply" type="int" value="65450"/>
		<constant name="IBUS_KP_Next" type="int" value="65435"/>
		<constant name="IBUS_KP_Page_Down" type="int" value="65435"/>
		<constant name="IBUS_KP_Page_Up" type="int" value="65434"/>
		<constant name="IBUS_KP_Prior" type="int" value="65434"/>
		<constant name="IBUS_KP_Right" type="int" value="65432"/>
		<constant name="IBUS_KP_Separator" type="int" value="65452"/>
		<constant name="IBUS_KP_Space" type="int" value="65408"/>
		<constant name="IBUS_KP_Subtract" type="int" value="65453"/>
		<constant name="IBUS_KP_Tab" type="int" value="65417"/>
		<constant name="IBUS_KP_Up" type="int" value="65431"/>
		<constant name="IBUS_Kana_Lock" type="int" value="65325"/>
		<constant name="IBUS_Kana_Shift" type="int" value="65326"/>
		<constant name="IBUS_Kanji" type="int" value="65313"/>
		<constant name="IBUS_Kanji_Bangou" type="int" value="65335"/>
		<constant name="IBUS_Katakana" type="int" value="65318"/>
		<constant name="IBUS_Kcedilla" type="int" value="979"/>
		<constant name="IBUS_Korean_Won" type="int" value="3839"/>
		<constant name="IBUS_L" type="int" value="76"/>
		<constant name="IBUS_L1" type="int" value="65480"/>
		<constant name="IBUS_L10" type="int" value="65489"/>
		<constant name="IBUS_L2" type="int" value="65481"/>
		<constant name="IBUS_L3" type="int" value="65482"/>
		<constant name="IBUS_L4" type="int" value="65483"/>
		<constant name="IBUS_L5" type="int" value="65484"/>
		<constant name="IBUS_L6" type="int" value="65485"/>
		<constant name="IBUS_L7" type="int" value="65486"/>
		<constant name="IBUS_L8" type="int" value="65487"/>
		<constant name="IBUS_L9" type="int" value="65488"/>
		<constant name="IBUS_Lacute" type="int" value="453"/>
		<constant name="IBUS_Last_Virtual_Screen" type="int" value="65236"/>
		<constant name="IBUS_Lbelowdot" type="int" value="16784950"/>
		<constant name="IBUS_Lcaron" type="int" value="421"/>
		<constant name="IBUS_Lcedilla" type="int" value="934"/>
		<constant name="IBUS_Left" type="int" value="65361"/>
		<constant name="IBUS_Linefeed" type="int" value="65290"/>
		<constant name="IBUS_LiraSign" type="int" value="16785572"/>
		<constant name="IBUS_Lstroke" type="int" value="419"/>
		<constant name="IBUS_M" type="int" value="77"/>
		<constant name="IBUS_MAJOR_VERSION" type="int" value="1"/>
		<constant name="IBUS_MICRO_VERSION" type="int" value="4"/>
		<constant name="IBUS_MINOR_VERSION" type="int" value="3"/>
		<constant name="IBUS_Mabovedot" type="int" value="16784960"/>
		<constant name="IBUS_Macedonia_DSE" type="int" value="1717"/>
		<constant name="IBUS_Macedonia_GJE" type="int" value="1714"/>
		<constant name="IBUS_Macedonia_KJE" type="int" value="1724"/>
		<constant name="IBUS_Macedonia_dse" type="int" value="1701"/>
		<constant name="IBUS_Macedonia_gje" type="int" value="1698"/>
		<constant name="IBUS_Macedonia_kje" type="int" value="1708"/>
		<constant name="IBUS_Mae_Koho" type="int" value="65342"/>
		<constant name="IBUS_Massyo" type="int" value="65324"/>
		<constant name="IBUS_Menu" type="int" value="65383"/>
		<constant name="IBUS_Meta_L" type="int" value="65511"/>
		<constant name="IBUS_Meta_R" type="int" value="65512"/>
		<constant name="IBUS_MillSign" type="int" value="16785573"/>
		<constant name="IBUS_Mode_switch" type="int" value="65406"/>
		<constant name="IBUS_MouseKeys_Accel_Enable" type="int" value="65143"/>
		<constant name="IBUS_MouseKeys_Enable" type="int" value="65142"/>
		<constant name="IBUS_Muhenkan" type="int" value="65314"/>
		<constant name="IBUS_Multi_key" type="int" value="65312"/>
		<constant name="IBUS_MultipleCandidate" type="int" value="65341"/>
		<constant name="IBUS_N" type="int" value="78"/>
		<constant name="IBUS_Nacute" type="int" value="465"/>
		<constant name="IBUS_NairaSign" type="int" value="16785574"/>
		<constant name="IBUS_Ncaron" type="int" value="466"/>
		<constant name="IBUS_Ncedilla" type="int" value="977"/>
		<constant name="IBUS_NewSheqelSign" type="int" value="16785578"/>
		<constant name="IBUS_Next" type="int" value="65366"/>
		<constant name="IBUS_Next_Virtual_Screen" type="int" value="65234"/>
		<constant name="IBUS_Ntilde" type="int" value="209"/>
		<constant name="IBUS_Num_Lock" type="int" value="65407"/>
		<constant name="IBUS_O" type="int" value="79"/>
		<constant name="IBUS_OE" type="int" value="5052"/>
		<constant name="IBUS_Oacute" type="int" value="211"/>
		<constant name="IBUS_Obarred" type="int" value="16777631"/>
		<constant name="IBUS_Obelowdot" type="int" value="16785100"/>
		<constant name="IBUS_Ocaron" type="int" value="16777681"/>
		<constant name="IBUS_Ocircumflex" type="int" value="212"/>
		<constant name="IBUS_Ocircumflexacute" type="int" value="16785104"/>
		<constant name="IBUS_Ocircumflexbelowdot" type="int" value="16785112"/>
		<constant name="IBUS_Ocircumflexgrave" type="int" value="16785106"/>
		<constant name="IBUS_Ocircumflexhook" type="int" value="16785108"/>
		<constant name="IBUS_Ocircumflextilde" type="int" value="16785110"/>
		<constant name="IBUS_Odiaeresis" type="int" value="214"/>
		<constant name="IBUS_Odoubleacute" type="int" value="469"/>
		<constant name="IBUS_Ograve" type="int" value="210"/>
		<constant name="IBUS_Ohook" type="int" value="16785102"/>
		<constant name="IBUS_Ohorn" type="int" value="16777632"/>
		<constant name="IBUS_Ohornacute" type="int" value="16785114"/>
		<constant name="IBUS_Ohornbelowdot" type="int" value="16785122"/>
		<constant name="IBUS_Ohorngrave" type="int" value="16785116"/>
		<constant name="IBUS_Ohornhook" type="int" value="16785118"/>
		<constant name="IBUS_Ohorntilde" type="int" value="16785120"/>
		<constant name="IBUS_Omacron" type="int" value="978"/>
		<constant name="IBUS_Ooblique" type="int" value="216"/>
		<constant name="IBUS_Oslash" type="int" value="216"/>
		<constant name="IBUS_Otilde" type="int" value="213"/>
		<constant name="IBUS_Overlay1_Enable" type="int" value="65144"/>
		<constant name="IBUS_Overlay2_Enable" type="int" value="65145"/>
		<constant name="IBUS_P" type="int" value="80"/>
		<constant name="IBUS_PATH_CONFIG" type="char*" value="/org/freedesktop/IBus/Config"/>
		<constant name="IBUS_PATH_FACTORY" type="char*" value="/org/freedesktop/IBus/Factory"/>
		<constant name="IBUS_PATH_IBUS" type="char*" value="/org/freedesktop/IBus"/>
		<constant name="IBUS_PATH_INPUT_CONTEXT" type="char*" value="/org/freedesktop/IBus/InputContext_%d"/>
		<constant name="IBUS_PATH_NOTIFICATIONS" type="char*" value="/org/freedesktop/IBus/Notifications"/>
		<constant name="IBUS_PATH_PANEL" type="char*" value="/org/freedesktop/IBus/Panel"/>
		<constant name="IBUS_Pabovedot" type="int" value="16784982"/>
		<constant name="IBUS_Page_Down" type="int" value="65366"/>
		<constant name="IBUS_Page_Up" type="int" value="65365"/>
		<constant name="IBUS_Pause" type="int" value="65299"/>
		<constant name="IBUS_PesetaSign" type="int" value="16785575"/>
		<constant name="IBUS_Pointer_Accelerate" type="int" value="65274"/>
		<constant name="IBUS_Pointer_Button1" type="int" value="65257"/>
		<constant name="IBUS_Pointer_Button2" type="int" value="65258"/>
		<constant name="IBUS_Pointer_Button3" type="int" value="65259"/>
		<constant name="IBUS_Pointer_Button4" type="int" value="65260"/>
		<constant name="IBUS_Pointer_Button5" type="int" value="65261"/>
		<constant name="IBUS_Pointer_Button_Dflt" type="int" value="65256"/>
		<constant name="IBUS_Pointer_DblClick1" type="int" value="65263"/>
		<constant name="IBUS_Pointer_DblClick2" type="int" value="65264"/>
		<constant name="IBUS_Pointer_DblClick3" type="int" value="65265"/>
		<constant name="IBUS_Pointer_DblClick4" type="int" value="65266"/>
		<constant name="IBUS_Pointer_DblClick5" type="int" value="65267"/>
		<constant name="IBUS_Pointer_DblClick_Dflt" type="int" value="65262"/>
		<constant name="IBUS_Pointer_DfltBtnNext" type="int" value="65275"/>
		<constant name="IBUS_Pointer_DfltBtnPrev" type="int" value="65276"/>
		<constant name="IBUS_Pointer_Down" type="int" value="65251"/>
		<constant name="IBUS_Pointer_DownLeft" type="int" value="65254"/>
		<constant name="IBUS_Pointer_DownRight" type="int" value="65255"/>
		<constant name="IBUS_Pointer_Drag1" type="int" value="65269"/>
		<constant name="IBUS_Pointer_Drag2" type="int" value="65270"/>
		<constant name="IBUS_Pointer_Drag3" type="int" value="65271"/>
		<constant name="IBUS_Pointer_Drag4" type="int" value="65272"/>
		<constant name="IBUS_Pointer_Drag5" type="int" value="65277"/>
		<constant name="IBUS_Pointer_Drag_Dflt" type="int" value="65268"/>
		<constant name="IBUS_Pointer_EnableKeys" type="int" value="65273"/>
		<constant name="IBUS_Pointer_Left" type="int" value="65248"/>
		<constant name="IBUS_Pointer_Right" type="int" value="65249"/>
		<constant name="IBUS_Pointer_Up" type="int" value="65250"/>
		<constant name="IBUS_Pointer_UpLeft" type="int" value="65252"/>
		<constant name="IBUS_Pointer_UpRight" type="int" value="65253"/>
		<constant name="IBUS_Prev_Virtual_Screen" type="int" value="65233"/>
		<constant name="IBUS_PreviousCandidate" type="int" value="65342"/>
		<constant name="IBUS_Print" type="int" value="65377"/>
		<constant name="IBUS_Prior" type="int" value="65365"/>
		<constant name="IBUS_Q" type="int" value="81"/>
		<constant name="IBUS_R" type="int" value="82"/>
		<constant name="IBUS_R1" type="int" value="65490"/>
		<constant name="IBUS_R10" type="int" value="65499"/>
		<constant name="IBUS_R11" type="int" value="65500"/>
		<constant name="IBUS_R12" type="int" value="65501"/>
		<constant name="IBUS_R13" type="int" value="65502"/>
		<constant name="IBUS_R14" type="int" value="65503"/>
		<constant name="IBUS_R15" type="int" value="65504"/>
		<constant name="IBUS_R2" type="int" value="65491"/>
		<constant name="IBUS_R3" type="int" value="65492"/>
		<constant name="IBUS_R4" type="int" value="65493"/>
		<constant name="IBUS_R5" type="int" value="65494"/>
		<constant name="IBUS_R6" type="int" value="65495"/>
		<constant name="IBUS_R7" type="int" value="65496"/>
		<constant name="IBUS_R8" type="int" value="65497"/>
		<constant name="IBUS_R9" type="int" value="65498"/>
		<constant name="IBUS_Racute" type="int" value="448"/>
		<constant name="IBUS_Rcaron" type="int" value="472"/>
		<constant name="IBUS_Rcedilla" type="int" value="931"/>
		<constant name="IBUS_Redo" type="int" value="65382"/>
		<constant name="IBUS_RepeatKeys_Enable" type="int" value="65138"/>
		<constant name="IBUS_Return" type="int" value="65293"/>
		<constant name="IBUS_Right" type="int" value="65363"/>
		<constant name="IBUS_Romaji" type="int" value="65316"/>
		<constant name="IBUS_RupeeSign" type="int" value="16785576"/>
		<constant name="IBUS_S" type="int" value="83"/>
		<constant name="IBUS_SCHWA" type="int" value="16777615"/>
		<constant name="IBUS_SERVICE_CONFIG" type="char*" value="org.freedesktop.IBus.Config"/>
		<constant name="IBUS_SERVICE_IBUS" type="char*" value="org.freedesktop.IBus"/>
		<constant name="IBUS_SERVICE_NOTIFICATIONS" type="char*" value="org.freedesktop.IBus.Notifications"/>
		<constant name="IBUS_SERVICE_PANEL" type="char*" value="org.freedesktop.IBus.Panel"/>
		<constant name="IBUS_Sabovedot" type="int" value="16784992"/>
		<constant name="IBUS_Sacute" type="int" value="422"/>
		<constant name="IBUS_Scaron" type="int" value="425"/>
		<constant name="IBUS_Scedilla" type="int" value="426"/>
		<constant name="IBUS_Scircumflex" type="int" value="734"/>
		<constant name="IBUS_Scroll_Lock" type="int" value="65300"/>
		<constant name="IBUS_Select" type="int" value="65376"/>
		<constant name="IBUS_Serbian_DJE" type="int" value="1713"/>
		<constant name="IBUS_Serbian_DZE" type="int" value="1727"/>
		<constant name="IBUS_Serbian_JE" type="int" value="1720"/>
		<constant name="IBUS_Serbian_LJE" type="int" value="1721"/>
		<constant name="IBUS_Serbian_NJE" type="int" value="1722"/>
		<constant name="IBUS_Serbian_TSHE" type="int" value="1723"/>
		<constant name="IBUS_Serbian_dje" type="int" value="1697"/>
		<constant name="IBUS_Serbian_dze" type="int" value="1711"/>
		<constant name="IBUS_Serbian_je" type="int" value="1704"/>
		<constant name="IBUS_Serbian_lje" type="int" value="1705"/>
		<constant name="IBUS_Serbian_nje" type="int" value="1706"/>
		<constant name="IBUS_Serbian_tshe" type="int" value="1707"/>
		<constant name="IBUS_Shift_L" type="int" value="65505"/>
		<constant name="IBUS_Shift_Lock" type="int" value="65510"/>
		<constant name="IBUS_Shift_R" type="int" value="65506"/>
		<constant name="IBUS_SingleCandidate" type="int" value="65340"/>
		<constant name="IBUS_SlowKeys_Enable" type="int" value="65139"/>
		<constant name="IBUS_StickyKeys_Enable" type="int" value="65141"/>
		<constant name="IBUS_Super_L" type="int" value="65515"/>
		<constant name="IBUS_Super_R" type="int" value="65516"/>
		<constant name="IBUS_Sys_Req" type="int" value="65301"/>
		<constant name="IBUS_T" type="int" value="84"/>
		<constant name="IBUS_THORN" type="int" value="222"/>
		<constant name="IBUS_Tab" type="int" value="65289"/>
		<constant name="IBUS_Tabovedot" type="int" value="16785002"/>
		<constant name="IBUS_Tcaron" type="int" value="427"/>
		<constant name="IBUS_Tcedilla" type="int" value="478"/>
		<constant name="IBUS_Terminate_Server" type="int" value="65237"/>
		<constant name="IBUS_Thai_baht" type="int" value="3551"/>
		<constant name="IBUS_Thai_bobaimai" type="int" value="3514"/>
		<constant name="IBUS_Thai_chochan" type="int" value="3496"/>
		<constant name="IBUS_Thai_chochang" type="int" value="3498"/>
		<constant name="IBUS_Thai_choching" type="int" value="3497"/>
		<constant name="IBUS_Thai_chochoe" type="int" value="3500"/>
		<constant name="IBUS_Thai_dochada" type="int" value="3502"/>
		<constant name="IBUS_Thai_dodek" type="int" value="3508"/>
		<constant name="IBUS_Thai_fofa" type="int" value="3517"/>
		<constant name="IBUS_Thai_fofan" type="int" value="3519"/>
		<constant name="IBUS_Thai_hohip" type="int" value="3531"/>
		<constant name="IBUS_Thai_honokhuk" type="int" value="3534"/>
		<constant name="IBUS_Thai_khokhai" type="int" value="3490"/>
		<constant name="IBUS_Thai_khokhon" type="int" value="3493"/>
		<constant name="IBUS_Thai_khokhuat" type="int" value="3491"/>
		<constant name="IBUS_Thai_khokhwai" type="int" value="3492"/>
		<constant name="IBUS_Thai_khorakhang" type="int" value="3494"/>
		<constant name="IBUS_Thai_kokai" type="int" value="3489"/>
		<constant name="IBUS_Thai_lakkhangyao" type="int" value="3557"/>
		<constant name="IBUS_Thai_lekchet" type="int" value="3575"/>
		<constant name="IBUS_Thai_lekha" type="int" value="3573"/>
		<constant name="IBUS_Thai_lekhok" type="int" value="3574"/>
		<constant name="IBUS_Thai_lekkao" type="int" value="3577"/>
		<constant name="IBUS_Thai_leknung" type="int" value="3569"/>
		<constant name="IBUS_Thai_lekpaet" type="int" value="3576"/>
		<constant name="IBUS_Thai_leksam" type="int" value="3571"/>
		<constant name="IBUS_Thai_leksi" type="int" value="3572"/>
		<constant name="IBUS_Thai_leksong" type="int" value="3570"/>
		<constant name="IBUS_Thai_leksun" type="int" value="3568"/>
		<constant name="IBUS_Thai_lochula" type="int" value="3532"/>
		<constant name="IBUS_Thai_loling" type="int" value="3525"/>
		<constant name="IBUS_Thai_lu" type="int" value="3526"/>
		<constant name="IBUS_Thai_maichattawa" type="int" value="3563"/>
		<constant name="IBUS_Thai_maiek" type="int" value="3560"/>
		<constant name="IBUS_Thai_maihanakat" type="int" value="3537"/>
		<constant name="IBUS_Thai_maihanakat_maitho" type="int" value="3550"/>
		<constant name="IBUS_Thai_maitaikhu" type="int" value="3559"/>
		<constant name="IBUS_Thai_maitho" type="int" value="3561"/>
		<constant name="IBUS_Thai_maitri" type="int" value="3562"/>
		<constant name="IBUS_Thai_maiyamok" type="int" value="3558"/>
		<constant name="IBUS_Thai_moma" type="int" value="3521"/>
		<constant name="IBUS_Thai_ngongu" type="int" value="3495"/>
		<constant name="IBUS_Thai_nikhahit" type="int" value="3565"/>
		<constant name="IBUS_Thai_nonen" type="int" value="3507"/>
		<constant name="IBUS_Thai_nonu" type="int" value="3513"/>
		<constant name="IBUS_Thai_oang" type="int" value="3533"/>
		<constant name="IBUS_Thai_paiyannoi" type="int" value="3535"/>
		<constant name="IBUS_Thai_phinthu" type="int" value="3546"/>
		<constant name="IBUS_Thai_phophan" type="int" value="3518"/>
		<constant name="IBUS_Thai_phophung" type="int" value="3516"/>
		<constant name="IBUS_Thai_phosamphao" type="int" value="3520"/>
		<constant name="IBUS_Thai_popla" type="int" value="3515"/>
		<constant name="IBUS_Thai_rorua" type="int" value="3523"/>
		<constant name="IBUS_Thai_ru" type="int" value="3524"/>
		<constant name="IBUS_Thai_saraa" type="int" value="3536"/>
		<constant name="IBUS_Thai_saraaa" type="int" value="3538"/>
		<constant name="IBUS_Thai_saraae" type="int" value="3553"/>
		<constant name="IBUS_Thai_saraaimaimalai" type="int" value="3556"/>
		<constant name="IBUS_Thai_saraaimaimuan" type="int" value="3555"/>
		<constant name="IBUS_Thai_saraam" type="int" value="3539"/>
		<constant name="IBUS_Thai_sarae" type="int" value="3552"/>
		<constant name="IBUS_Thai_sarai" type="int" value="3540"/>
		<constant name="IBUS_Thai_saraii" type="int" value="3541"/>
		<constant name="IBUS_Thai_sarao" type="int" value="3554"/>
		<constant name="IBUS_Thai_sarau" type="int" value="3544"/>
		<constant name="IBUS_Thai_saraue" type="int" value="3542"/>
		<constant name="IBUS_Thai_sarauee" type="int" value="3543"/>
		<constant name="IBUS_Thai_sarauu" type="int" value="3545"/>
		<constant name="IBUS_Thai_sorusi" type="int" value="3529"/>
		<constant name="IBUS_Thai_sosala" type="int" value="3528"/>
		<constant name="IBUS_Thai_soso" type="int" value="3499"/>
		<constant name="IBUS_Thai_sosua" type="int" value="3530"/>
		<constant name="IBUS_Thai_thanthakhat" type="int" value="3564"/>
		<constant name="IBUS_Thai_thonangmontho" type="int" value="3505"/>
		<constant name="IBUS_Thai_thophuthao" type="int" value="3506"/>
		<constant name="IBUS_Thai_thothahan" type="int" value="3511"/>
		<constant name="IBUS_Thai_thothan" type="int" value="3504"/>
		<constant name="IBUS_Thai_thothong" type="int" value="3512"/>
		<constant name="IBUS_Thai_thothung" type="int" value="3510"/>
		<constant name="IBUS_Thai_topatak" type="int" value="3503"/>
		<constant name="IBUS_Thai_totao" type="int" value="3509"/>
		<constant name="IBUS_Thai_wowaen" type="int" value="3527"/>
		<constant name="IBUS_Thai_yoyak" type="int" value="3522"/>
		<constant name="IBUS_Thai_yoying" type="int" value="3501"/>
		<constant name="IBUS_Thorn" type="int" value="222"/>
		<constant name="IBUS_Touroku" type="int" value="65323"/>
		<constant name="IBUS_Tslash" type="int" value="940"/>
		<constant name="IBUS_U" type="int" value="85"/>
		<constant name="IBUS_Uacute" type="int" value="218"/>
		<constant name="IBUS_Ubelowdot" type="int" value="16785124"/>
		<constant name="IBUS_Ubreve" type="int" value="733"/>
		<constant name="IBUS_Ucircumflex" type="int" value="219"/>
		<constant name="IBUS_Udiaeresis" type="int" value="220"/>
		<constant name="IBUS_Udoubleacute" type="int" value="475"/>
		<constant name="IBUS_Ugrave" type="int" value="217"/>
		<constant name="IBUS_Uhook" type="int" value="16785126"/>
		<constant name="IBUS_Uhorn" type="int" value="16777647"/>
		<constant name="IBUS_Uhornacute" type="int" value="16785128"/>
		<constant name="IBUS_Uhornbelowdot" type="int" value="16785136"/>
		<constant name="IBUS_Uhorngrave" type="int" value="16785130"/>
		<constant name="IBUS_Uhornhook" type="int" value="16785132"/>
		<constant name="IBUS_Uhorntilde" type="int" value="16785134"/>
		<constant name="IBUS_Ukrainian_GHE_WITH_UPTURN" type="int" value="1725"/>
		<constant name="IBUS_Ukrainian_I" type="int" value="1718"/>
		<constant name="IBUS_Ukrainian_IE" type="int" value="1716"/>
		<constant name="IBUS_Ukrainian_YI" type="int" value="1719"/>
		<constant name="IBUS_Ukrainian_ghe_with_upturn" type="int" value="1709"/>
		<constant name="IBUS_Ukrainian_i" type="int" value="1702"/>
		<constant name="IBUS_Ukrainian_ie" type="int" value="1700"/>
		<constant name="IBUS_Ukrainian_yi" type="int" value="1703"/>
		<constant name="IBUS_Ukranian_I" type="int" value="1718"/>
		<constant name="IBUS_Ukranian_JE" type="int" value="1716"/>
		<constant name="IBUS_Ukranian_YI" type="int" value="1719"/>
		<constant name="IBUS_Ukranian_i" type="int" value="1702"/>
		<constant name="IBUS_Ukranian_je" type="int" value="1700"/>
		<constant name="IBUS_Ukranian_yi" type="int" value="1703"/>
		<constant name="IBUS_Umacron" type="int" value="990"/>
		<constant name="IBUS_Undo" type="int" value="65381"/>
		<constant name="IBUS_Uogonek" type="int" value="985"/>
		<constant name="IBUS_Up" type="int" value="65362"/>
		<constant name="IBUS_Uring" type="int" value="473"/>
		<constant name="IBUS_Utilde" type="int" value="989"/>
		<constant name="IBUS_V" type="int" value="86"/>
		<constant name="IBUS_VoidSymbol" type="int" value="16777215"/>
		<constant name="IBUS_W" type="int" value="87"/>
		<constant name="IBUS_Wacute" type="int" value="16785026"/>
		<constant name="IBUS_Wcircumflex" type="int" value="16777588"/>
		<constant name="IBUS_Wdiaeresis" type="int" value="16785028"/>
		<constant name="IBUS_Wgrave" type="int" value="16785024"/>
		<constant name="IBUS_WonSign" type="int" value="16785577"/>
		<constant name="IBUS_X" type="int" value="88"/>
		<constant name="IBUS_Xabovedot" type="int" value="16785034"/>
		<constant name="IBUS_Y" type="int" value="89"/>
		<constant name="IBUS_Yacute" type="int" value="221"/>
		<constant name="IBUS_Ybelowdot" type="int" value="16785140"/>
		<constant name="IBUS_Ycircumflex" type="int" value="16777590"/>
		<constant name="IBUS_Ydiaeresis" type="int" value="5054"/>
		<constant name="IBUS_Ygrave" type="int" value="16785138"/>
		<constant name="IBUS_Yhook" type="int" value="16785142"/>
		<constant name="IBUS_Ytilde" type="int" value="16785144"/>
		<constant name="IBUS_Z" type="int" value="90"/>
		<constant name="IBUS_Zabovedot" type="int" value="431"/>
		<constant name="IBUS_Zacute" type="int" value="428"/>
		<constant name="IBUS_Zcaron" type="int" value="430"/>
		<constant name="IBUS_Zen_Koho" type="int" value="65341"/>
		<constant name="IBUS_Zenkaku" type="int" value="65320"/>
		<constant name="IBUS_Zenkaku_Hankaku" type="int" value="65322"/>
		<constant name="IBUS_Zstroke" type="int" value="16777653"/>
		<constant name="IBUS_a" type="int" value="97"/>
		<constant name="IBUS_aacute" type="int" value="225"/>
		<constant name="IBUS_abelowdot" type="int" value="16785057"/>
		<constant name="IBUS_abovedot" type="int" value="511"/>
		<constant name="IBUS_abreve" type="int" value="483"/>
		<constant name="IBUS_abreveacute" type="int" value="16785071"/>
		<constant name="IBUS_abrevebelowdot" type="int" value="16785079"/>
		<constant name="IBUS_abrevegrave" type="int" value="16785073"/>
		<constant name="IBUS_abrevehook" type="int" value="16785075"/>
		<constant name="IBUS_abrevetilde" type="int" value="16785077"/>
		<constant name="IBUS_acircumflex" type="int" value="226"/>
		<constant name="IBUS_acircumflexacute" type="int" value="16785061"/>
		<constant name="IBUS_acircumflexbelowdot" type="int" value="16785069"/>
		<constant name="IBUS_acircumflexgrave" type="int" value="16785063"/>
		<constant name="IBUS_acircumflexhook" type="int" value="16785065"/>
		<constant name="IBUS_acircumflextilde" type="int" value="16785067"/>
		<constant name="IBUS_acute" type="int" value="180"/>
		<constant name="IBUS_adiaeresis" type="int" value="228"/>
		<constant name="IBUS_ae" type="int" value="230"/>
		<constant name="IBUS_agrave" type="int" value="224"/>
		<constant name="IBUS_ahook" type="int" value="16785059"/>
		<constant name="IBUS_amacron" type="int" value="992"/>
		<constant name="IBUS_ampersand" type="int" value="38"/>
		<constant name="IBUS_aogonek" type="int" value="433"/>
		<constant name="IBUS_apostrophe" type="int" value="39"/>
		<constant name="IBUS_approxeq" type="int" value="16785992"/>
		<constant name="IBUS_approximate" type="int" value="2248"/>
		<constant name="IBUS_aring" type="int" value="229"/>
		<constant name="IBUS_asciicircum" type="int" value="94"/>
		<constant name="IBUS_asciitilde" type="int" value="126"/>
		<constant name="IBUS_asterisk" type="int" value="42"/>
		<constant name="IBUS_at" type="int" value="64"/>
		<constant name="IBUS_atilde" type="int" value="227"/>
		<constant name="IBUS_b" type="int" value="98"/>
		<constant name="IBUS_babovedot" type="int" value="16784899"/>
		<constant name="IBUS_backslash" type="int" value="92"/>
		<constant name="IBUS_ballotcross" type="int" value="2804"/>
		<constant name="IBUS_bar" type="int" value="124"/>
		<constant name="IBUS_because" type="int" value="16785973"/>
		<constant name="IBUS_blank" type="int" value="2527"/>
		<constant name="IBUS_botintegral" type="int" value="2213"/>
		<constant name="IBUS_botleftparens" type="int" value="2220"/>
		<constant name="IBUS_botleftsqbracket" type="int" value="2216"/>
		<constant name="IBUS_botleftsummation" type="int" value="2226"/>
		<constant name="IBUS_botrightparens" type="int" value="2222"/>
		<constant name="IBUS_botrightsqbracket" type="int" value="2218"/>
		<constant name="IBUS_botrightsummation" type="int" value="2230"/>
		<constant name="IBUS_bott" type="int" value="2550"/>
		<constant name="IBUS_botvertsummationconnector" type="int" value="2228"/>
		<constant name="IBUS_braceleft" type="int" value="123"/>
		<constant name="IBUS_braceright" type="int" value="125"/>
		<constant name="IBUS_bracketleft" type="int" value="91"/>
		<constant name="IBUS_bracketright" type="int" value="93"/>
		<constant name="IBUS_braille_blank" type="int" value="16787456"/>
		<constant name="IBUS_braille_dot_1" type="int" value="65521"/>
		<constant name="IBUS_braille_dot_10" type="int" value="65530"/>
		<constant name="IBUS_braille_dot_2" type="int" value="65522"/>
		<constant name="IBUS_braille_dot_3" type="int" value="65523"/>
		<constant name="IBUS_braille_dot_4" type="int" value="65524"/>
		<constant name="IBUS_braille_dot_5" type="int" value="65525"/>
		<constant name="IBUS_braille_dot_6" type="int" value="65526"/>
		<constant name="IBUS_braille_dot_7" type="int" value="65527"/>
		<constant name="IBUS_braille_dot_8" type="int" value="65528"/>
		<constant name="IBUS_braille_dot_9" type="int" value="65529"/>
		<constant name="IBUS_braille_dots_1" type="int" value="16787457"/>
		<constant name="IBUS_braille_dots_12" type="int" value="16787459"/>
		<constant name="IBUS_braille_dots_123" type="int" value="16787463"/>
		<constant name="IBUS_braille_dots_1234" type="int" value="16787471"/>
		<constant name="IBUS_braille_dots_12345" type="int" value="16787487"/>
		<constant name="IBUS_braille_dots_123456" type="int" value="16787519"/>
		<constant name="IBUS_braille_dots_1234567" type="int" value="16787583"/>
		<constant name="IBUS_braille_dots_12345678" type="int" value="16787711"/>
		<constant name="IBUS_braille_dots_1234568" type="int" value="16787647"/>
		<constant name="IBUS_braille_dots_123457" type="int" value="16787551"/>
		<constant name="IBUS_braille_dots_1234578" type="int" value="16787679"/>
		<constant name="IBUS_braille_dots_123458" type="int" value="16787615"/>
		<constant name="IBUS_braille_dots_12346" type="int" value="16787503"/>
		<constant name="IBUS_braille_dots_123467" type="int" value="16787567"/>
		<constant name="IBUS_braille_dots_1234678" type="int" value="16787695"/>
		<constant name="IBUS_braille_dots_123468" type="int" value="16787631"/>
		<constant name="IBUS_braille_dots_12347" type="int" value="16787535"/>
		<constant name="IBUS_braille_dots_123478" type="int" value="16787663"/>
		<constant name="IBUS_braille_dots_12348" type="int" value="16787599"/>
		<constant name="IBUS_braille_dots_1235" type="int" value="16787479"/>
		<constant name="IBUS_braille_dots_12356" type="int" value="16787511"/>
		<constant name="IBUS_braille_dots_123567" type="int" value="16787575"/>
		<constant name="IBUS_braille_dots_1235678" type="int" value="16787703"/>
		<constant name="IBUS_braille_dots_123568" type="int" value="16787639"/>
		<constant name="IBUS_braille_dots_12357" type="int" value="16787543"/>
		<constant name="IBUS_braille_dots_123578" type="int" value="16787671"/>
		<constant name="IBUS_braille_dots_12358" type="int" value="16787607"/>
		<constant name="IBUS_braille_dots_1236" type="int" value="16787495"/>
		<constant name="IBUS_braille_dots_12367" type="int" value="16787559"/>
		<constant name="IBUS_braille_dots_123678" type="int" value="16787687"/>
		<constant name="IBUS_braille_dots_12368" type="int" value="16787623"/>
		<constant name="IBUS_braille_dots_1237" type="int" value="16787527"/>
		<constant name="IBUS_braille_dots_12378" type="int" value="16787655"/>
		<constant name="IBUS_braille_dots_1238" type="int" value="16787591"/>
		<constant name="IBUS_braille_dots_124" type="int" value="16787467"/>
		<constant name="IBUS_braille_dots_1245" type="int" value="16787483"/>
		<constant name="IBUS_braille_dots_12456" type="int" value="16787515"/>
		<constant name="IBUS_braille_dots_124567" type="int" value="16787579"/>
		<constant name="IBUS_braille_dots_1245678" type="int" value="16787707"/>
		<constant name="IBUS_braille_dots_124568" type="int" value="16787643"/>
		<constant name="IBUS_braille_dots_12457" type="int" value="16787547"/>
		<constant name="IBUS_braille_dots_124578" type="int" value="16787675"/>
		<constant name="IBUS_braille_dots_12458" type="int" value="16787611"/>
		<constant name="IBUS_braille_dots_1246" type="int" value="16787499"/>
		<constant name="IBUS_braille_dots_12467" type="int" value="16787563"/>
		<constant name="IBUS_braille_dots_124678" type="int" value="16787691"/>
		<constant name="IBUS_braille_dots_12468" type="int" value="16787627"/>
		<constant name="IBUS_braille_dots_1247" type="int" value="16787531"/>
		<constant name="IBUS_braille_dots_12478" type="int" value="16787659"/>
		<constant name="IBUS_braille_dots_1248" type="int" value="16787595"/>
		<constant name="IBUS_braille_dots_125" type="int" value="16787475"/>
		<constant name="IBUS_braille_dots_1256" type="int" value="16787507"/>
		<constant name="IBUS_braille_dots_12567" type="int" value="16787571"/>
		<constant name="IBUS_braille_dots_125678" type="int" value="16787699"/>
		<constant name="IBUS_braille_dots_12568" type="int" value="16787635"/>
		<constant name="IBUS_braille_dots_1257" type="int" value="16787539"/>
		<constant name="IBUS_braille_dots_12578" type="int" value="16787667"/>
		<constant name="IBUS_braille_dots_1258" type="int" value="16787603"/>
		<constant name="IBUS_braille_dots_126" type="int" value="16787491"/>
		<constant name="IBUS_braille_dots_1267" type="int" value="16787555"/>
		<constant name="IBUS_braille_dots_12678" type="int" value="16787683"/>
		<constant name="IBUS_braille_dots_1268" type="int" value="16787619"/>
		<constant name="IBUS_braille_dots_127" type="int" value="16787523"/>
		<constant name="IBUS_braille_dots_1278" type="int" value="16787651"/>
		<constant name="IBUS_braille_dots_128" type="int" value="16787587"/>
		<constant name="IBUS_braille_dots_13" type="int" value="16787461"/>
		<constant name="IBUS_braille_dots_134" type="int" value="16787469"/>
		<constant name="IBUS_braille_dots_1345" type="int" value="16787485"/>
		<constant name="IBUS_braille_dots_13456" type="int" value="16787517"/>
		<constant name="IBUS_braille_dots_134567" type="int" value="16787581"/>
		<constant name="IBUS_braille_dots_1345678" type="int" value="16787709"/>
		<constant name="IBUS_braille_dots_134568" type="int" value="16787645"/>
		<constant name="IBUS_braille_dots_13457" type="int" value="16787549"/>
		<constant name="IBUS_braille_dots_134578" type="int" value="16787677"/>
		<constant name="IBUS_braille_dots_13458" type="int" value="16787613"/>
		<constant name="IBUS_braille_dots_1346" type="int" value="16787501"/>
		<constant name="IBUS_braille_dots_13467" type="int" value="16787565"/>
		<constant name="IBUS_braille_dots_134678" type="int" value="16787693"/>
		<constant name="IBUS_braille_dots_13468" type="int" value="16787629"/>
		<constant name="IBUS_braille_dots_1347" type="int" value="16787533"/>
		<constant name="IBUS_braille_dots_13478" type="int" value="16787661"/>
		<constant name="IBUS_braille_dots_1348" type="int" value="16787597"/>
		<constant name="IBUS_braille_dots_135" type="int" value="16787477"/>
		<constant name="IBUS_braille_dots_1356" type="int" value="16787509"/>
		<constant name="IBUS_braille_dots_13567" type="int" value="16787573"/>
		<constant name="IBUS_braille_dots_135678" type="int" value="16787701"/>
		<constant name="IBUS_braille_dots_13568" type="int" value="16787637"/>
		<constant name="IBUS_braille_dots_1357" type="int" value="16787541"/>
		<constant name="IBUS_braille_dots_13578" type="int" value="16787669"/>
		<constant name="IBUS_braille_dots_1358" type="int" value="16787605"/>
		<constant name="IBUS_braille_dots_136" type="int" value="16787493"/>
		<constant name="IBUS_braille_dots_1367" type="int" value="16787557"/>
		<constant name="IBUS_braille_dots_13678" type="int" value="16787685"/>
		<constant name="IBUS_braille_dots_1368" type="int" value="16787621"/>
		<constant name="IBUS_braille_dots_137" type="int" value="16787525"/>
		<constant name="IBUS_braille_dots_1378" type="int" value="16787653"/>
		<constant name="IBUS_braille_dots_138" type="int" value="16787589"/>
		<constant name="IBUS_braille_dots_14" type="int" value="16787465"/>
		<constant name="IBUS_braille_dots_145" type="int" value="16787481"/>
		<constant name="IBUS_braille_dots_1456" type="int" value="16787513"/>
		<constant name="IBUS_braille_dots_14567" type="int" value="16787577"/>
		<constant name="IBUS_braille_dots_145678" type="int" value="16787705"/>
		<constant name="IBUS_braille_dots_14568" type="int" value="16787641"/>
		<constant name="IBUS_braille_dots_1457" type="int" value="16787545"/>
		<constant name="IBUS_braille_dots_14578" type="int" value="16787673"/>
		<constant name="IBUS_braille_dots_1458" type="int" value="16787609"/>
		<constant name="IBUS_braille_dots_146" type="int" value="16787497"/>
		<constant name="IBUS_braille_dots_1467" type="int" value="16787561"/>
		<constant name="IBUS_braille_dots_14678" type="int" value="16787689"/>
		<constant name="IBUS_braille_dots_1468" type="int" value="16787625"/>
		<constant name="IBUS_braille_dots_147" type="int" value="16787529"/>
		<constant name="IBUS_braille_dots_1478" type="int" value="16787657"/>
		<constant name="IBUS_braille_dots_148" type="int" value="16787593"/>
		<constant name="IBUS_braille_dots_15" type="int" value="16787473"/>
		<constant name="IBUS_braille_dots_156" type="int" value="16787505"/>
		<constant name="IBUS_braille_dots_1567" type="int" value="16787569"/>
		<constant name="IBUS_braille_dots_15678" type="int" value="16787697"/>
		<constant name="IBUS_braille_dots_1568" type="int" value="16787633"/>
		<constant name="IBUS_braille_dots_157" type="int" value="16787537"/>
		<constant name="IBUS_braille_dots_1578" type="int" value="16787665"/>
		<constant name="IBUS_braille_dots_158" type="int" value="16787601"/>
		<constant name="IBUS_braille_dots_16" type="int" value="16787489"/>
		<constant name="IBUS_braille_dots_167" type="int" value="16787553"/>
		<constant name="IBUS_braille_dots_1678" type="int" value="16787681"/>
		<constant name="IBUS_braille_dots_168" type="int" value="16787617"/>
		<constant name="IBUS_braille_dots_17" type="int" value="16787521"/>
		<constant name="IBUS_braille_dots_178" type="int" value="16787649"/>
		<constant name="IBUS_braille_dots_18" type="int" value="16787585"/>
		<constant name="IBUS_braille_dots_2" type="int" value="16787458"/>
		<constant name="IBUS_braille_dots_23" type="int" value="16787462"/>
		<constant name="IBUS_braille_dots_234" type="int" value="16787470"/>
		<constant name="IBUS_braille_dots_2345" type="int" value="16787486"/>
		<constant name="IBUS_braille_dots_23456" type="int" value="16787518"/>
		<constant name="IBUS_braille_dots_234567" type="int" value="16787582"/>
		<constant name="IBUS_braille_dots_2345678" type="int" value="16787710"/>
		<constant name="IBUS_braille_dots_234568" type="int" value="16787646"/>
		<constant name="IBUS_braille_dots_23457" type="int" value="16787550"/>
		<constant name="IBUS_braille_dots_234578" type="int" value="16787678"/>
		<constant name="IBUS_braille_dots_23458" type="int" value="16787614"/>
		<constant name="IBUS_braille_dots_2346" type="int" value="16787502"/>
		<constant name="IBUS_braille_dots_23467" type="int" value="16787566"/>
		<constant name="IBUS_braille_dots_234678" type="int" value="16787694"/>
		<constant name="IBUS_braille_dots_23468" type="int" value="16787630"/>
		<constant name="IBUS_braille_dots_2347" type="int" value="16787534"/>
		<constant name="IBUS_braille_dots_23478" type="int" value="16787662"/>
		<constant name="IBUS_braille_dots_2348" type="int" value="16787598"/>
		<constant name="IBUS_braille_dots_235" type="int" value="16787478"/>
		<constant name="IBUS_braille_dots_2356" type="int" value="16787510"/>
		<constant name="IBUS_braille_dots_23567" type="int" value="16787574"/>
		<constant name="IBUS_braille_dots_235678" type="int" value="16787702"/>
		<constant name="IBUS_braille_dots_23568" type="int" value="16787638"/>
		<constant name="IBUS_braille_dots_2357" type="int" value="16787542"/>
		<constant name="IBUS_braille_dots_23578" type="int" value="16787670"/>
		<constant name="IBUS_braille_dots_2358" type="int" value="16787606"/>
		<constant name="IBUS_braille_dots_236" type="int" value="16787494"/>
		<constant name="IBUS_braille_dots_2367" type="int" value="16787558"/>
		<constant name="IBUS_braille_dots_23678" type="int" value="16787686"/>
		<constant name="IBUS_braille_dots_2368" type="int" value="16787622"/>
		<constant name="IBUS_braille_dots_237" type="int" value="16787526"/>
		<constant name="IBUS_braille_dots_2378" type="int" value="16787654"/>
		<constant name="IBUS_braille_dots_238" type="int" value="16787590"/>
		<constant name="IBUS_braille_dots_24" type="int" value="16787466"/>
		<constant name="IBUS_braille_dots_245" type="int" value="16787482"/>
		<constant name="IBUS_braille_dots_2456" type="int" value="16787514"/>
		<constant name="IBUS_braille_dots_24567" type="int" value="16787578"/>
		<constant name="IBUS_braille_dots_245678" type="int" value="16787706"/>
		<constant name="IBUS_braille_dots_24568" type="int" value="16787642"/>
		<constant name="IBUS_braille_dots_2457" type="int" value="16787546"/>
		<constant name="IBUS_braille_dots_24578" type="int" value="16787674"/>
		<constant name="IBUS_braille_dots_2458" type="int" value="16787610"/>
		<constant name="IBUS_braille_dots_246" type="int" value="16787498"/>
		<constant name="IBUS_braille_dots_2467" type="int" value="16787562"/>
		<constant name="IBUS_braille_dots_24678" type="int" value="16787690"/>
		<constant name="IBUS_braille_dots_2468" type="int" value="16787626"/>
		<constant name="IBUS_braille_dots_247" type="int" value="16787530"/>
		<constant name="IBUS_braille_dots_2478" type="int" value="16787658"/>
		<constant name="IBUS_braille_dots_248" type="int" value="16787594"/>
		<constant name="IBUS_braille_dots_25" type="int" value="16787474"/>
		<constant name="IBUS_braille_dots_256" type="int" value="16787506"/>
		<constant name="IBUS_braille_dots_2567" type="int" value="16787570"/>
		<constant name="IBUS_braille_dots_25678" type="int" value="16787698"/>
		<constant name="IBUS_braille_dots_2568" type="int" value="16787634"/>
		<constant name="IBUS_braille_dots_257" type="int" value="16787538"/>
		<constant name="IBUS_braille_dots_2578" type="int" value="16787666"/>
		<constant name="IBUS_braille_dots_258" type="int" value="16787602"/>
		<constant name="IBUS_braille_dots_26" type="int" value="16787490"/>
		<constant name="IBUS_braille_dots_267" type="int" value="16787554"/>
		<constant name="IBUS_braille_dots_2678" type="int" value="16787682"/>
		<constant name="IBUS_braille_dots_268" type="int" value="16787618"/>
		<constant name="IBUS_braille_dots_27" type="int" value="16787522"/>
		<constant name="IBUS_braille_dots_278" type="int" value="16787650"/>
		<constant name="IBUS_braille_dots_28" type="int" value="16787586"/>
		<constant name="IBUS_braille_dots_3" type="int" value="16787460"/>
		<constant name="IBUS_braille_dots_34" type="int" value="16787468"/>
		<constant name="IBUS_braille_dots_345" type="int" value="16787484"/>
		<constant name="IBUS_braille_dots_3456" type="int" value="16787516"/>
		<constant name="IBUS_braille_dots_34567" type="int" value="16787580"/>
		<constant name="IBUS_braille_dots_345678" type="int" value="16787708"/>
		<constant name="IBUS_braille_dots_34568" type="int" value="16787644"/>
		<constant name="IBUS_braille_dots_3457" type="int" value="16787548"/>
		<constant name="IBUS_braille_dots_34578" type="int" value="16787676"/>
		<constant name="IBUS_braille_dots_3458" type="int" value="16787612"/>
		<constant name="IBUS_braille_dots_346" type="int" value="16787500"/>
		<constant name="IBUS_braille_dots_3467" type="int" value="16787564"/>
		<constant name="IBUS_braille_dots_34678" type="int" value="16787692"/>
		<constant name="IBUS_braille_dots_3468" type="int" value="16787628"/>
		<constant name="IBUS_braille_dots_347" type="int" value="16787532"/>
		<constant name="IBUS_braille_dots_3478" type="int" value="16787660"/>
		<constant name="IBUS_braille_dots_348" type="int" value="16787596"/>
		<constant name="IBUS_braille_dots_35" type="int" value="16787476"/>
		<constant name="IBUS_braille_dots_356" type="int" value="16787508"/>
		<constant name="IBUS_braille_dots_3567" type="int" value="16787572"/>
		<constant name="IBUS_braille_dots_35678" type="int" value="16787700"/>
		<constant name="IBUS_braille_dots_3568" type="int" value="16787636"/>
		<constant name="IBUS_braille_dots_357" type="int" value="16787540"/>
		<constant name="IBUS_braille_dots_3578" type="int" value="16787668"/>
		<constant name="IBUS_braille_dots_358" type="int" value="16787604"/>
		<constant name="IBUS_braille_dots_36" type="int" value="16787492"/>
		<constant name="IBUS_braille_dots_367" type="int" value="16787556"/>
		<constant name="IBUS_braille_dots_3678" type="int" value="16787684"/>
		<constant name="IBUS_braille_dots_368" type="int" value="16787620"/>
		<constant name="IBUS_braille_dots_37" type="int" value="16787524"/>
		<constant name="IBUS_braille_dots_378" type="int" value="16787652"/>
		<constant name="IBUS_braille_dots_38" type="int" value="16787588"/>
		<constant name="IBUS_braille_dots_4" type="int" value="16787464"/>
		<constant name="IBUS_braille_dots_45" type="int" value="16787480"/>
		<constant name="IBUS_braille_dots_456" type="int" value="16787512"/>
		<constant name="IBUS_braille_dots_4567" type="int" value="16787576"/>
		<constant name="IBUS_braille_dots_45678" type="int" value="16787704"/>
		<constant name="IBUS_braille_dots_4568" type="int" value="16787640"/>
		<constant name="IBUS_braille_dots_457" type="int" value="16787544"/>
		<constant name="IBUS_braille_dots_4578" type="int" value="16787672"/>
		<constant name="IBUS_braille_dots_458" type="int" value="16787608"/>
		<constant name="IBUS_braille_dots_46" type="int" value="16787496"/>
		<constant name="IBUS_braille_dots_467" type="int" value="16787560"/>
		<constant name="IBUS_braille_dots_4678" type="int" value="16787688"/>
		<constant name="IBUS_braille_dots_468" type="int" value="16787624"/>
		<constant name="IBUS_braille_dots_47" type="int" value="16787528"/>
		<constant name="IBUS_braille_dots_478" type="int" value="16787656"/>
		<constant name="IBUS_braille_dots_48" type="int" value="16787592"/>
		<constant name="IBUS_braille_dots_5" type="int" value="16787472"/>
		<constant name="IBUS_braille_dots_56" type="int" value="16787504"/>
		<constant name="IBUS_braille_dots_567" type="int" value="16787568"/>
		<constant name="IBUS_braille_dots_5678" type="int" value="16787696"/>
		<constant name="IBUS_braille_dots_568" type="int" value="16787632"/>
		<constant name="IBUS_braille_dots_57" type="int" value="16787536"/>
		<constant name="IBUS_braille_dots_578" type="int" value="16787664"/>
		<constant name="IBUS_braille_dots_58" type="int" value="16787600"/>
		<constant name="IBUS_braille_dots_6" type="int" value="16787488"/>
		<constant name="IBUS_braille_dots_67" type="int" value="16787552"/>
		<constant name="IBUS_braille_dots_678" type="int" value="16787680"/>
		<constant name="IBUS_braille_dots_68" type="int" value="16787616"/>
		<constant name="IBUS_braille_dots_7" type="int" value="16787520"/>
		<constant name="IBUS_braille_dots_78" type="int" value="16787648"/>
		<constant name="IBUS_braille_dots_8" type="int" value="16787584"/>
		<constant name="IBUS_breve" type="int" value="418"/>
		<constant name="IBUS_brokenbar" type="int" value="166"/>
		<constant name="IBUS_c" type="int" value="99"/>
		<constant name="IBUS_cabovedot" type="int" value="741"/>
		<constant name="IBUS_cacute" type="int" value="486"/>
		<constant name="IBUS_careof" type="int" value="2744"/>
		<constant name="IBUS_caret" type="int" value="2812"/>
		<constant name="IBUS_caron" type="int" value="439"/>
		<constant name="IBUS_ccaron" type="int" value="488"/>
		<constant name="IBUS_ccedilla" type="int" value="231"/>
		<constant name="IBUS_ccircumflex" type="int" value="742"/>
		<constant name="IBUS_cedilla" type="int" value="184"/>
		<constant name="IBUS_cent" type="int" value="162"/>
		<constant name="IBUS_checkerboard" type="int" value="2529"/>
		<constant name="IBUS_checkmark" type="int" value="2803"/>
		<constant name="IBUS_circle" type="int" value="3023"/>
		<constant name="IBUS_club" type="int" value="2796"/>
		<constant name="IBUS_colon" type="int" value="58"/>
		<constant name="IBUS_comma" type="int" value="44"/>
		<constant name="IBUS_containsas" type="int" value="16785931"/>
		<constant name="IBUS_copyright" type="int" value="169"/>
		<constant name="IBUS_cr" type="int" value="2532"/>
		<constant name="IBUS_crossinglines" type="int" value="2542"/>
		<constant name="IBUS_cuberoot" type="int" value="16785947"/>
		<constant name="IBUS_currency" type="int" value="164"/>
		<constant name="IBUS_cursor" type="int" value="2815"/>
		<constant name="IBUS_d" type="int" value="100"/>
		<constant name="IBUS_dabovedot" type="int" value="16784907"/>
		<constant name="IBUS_dagger" type="int" value="2801"/>
		<constant name="IBUS_dcaron" type="int" value="495"/>
		<constant name="IBUS_dead_abovecomma" type="int" value="65124"/>
		<constant name="IBUS_dead_abovedot" type="int" value="65110"/>
		<constant name="IBUS_dead_abovereversedcomma" type="int" value="65125"/>
		<constant name="IBUS_dead_abovering" type="int" value="65112"/>
		<constant name="IBUS_dead_acute" type="int" value="65105"/>
		<constant name="IBUS_dead_belowbreve" type="int" value="65131"/>
		<constant name="IBUS_dead_belowcircumflex" type="int" value="65129"/>
		<constant name="IBUS_dead_belowdiaeresis" type="int" value="65132"/>
		<constant name="IBUS_dead_belowdot" type="int" value="65120"/>
		<constant name="IBUS_dead_belowmacron" type="int" value="65128"/>
		<constant name="IBUS_dead_belowring" type="int" value="65127"/>
		<constant name="IBUS_dead_belowtilde" type="int" value="65130"/>
		<constant name="IBUS_dead_breve" type="int" value="65109"/>
		<constant name="IBUS_dead_caron" type="int" value="65114"/>
		<constant name="IBUS_dead_cedilla" type="int" value="65115"/>
		<constant name="IBUS_dead_circumflex" type="int" value="65106"/>
		<constant name="IBUS_dead_dasia" type="int" value="65125"/>
		<constant name="IBUS_dead_diaeresis" type="int" value="65111"/>
		<constant name="IBUS_dead_doubleacute" type="int" value="65113"/>
		<constant name="IBUS_dead_grave" type="int" value="65104"/>
		<constant name="IBUS_dead_hook" type="int" value="65121"/>
		<constant name="IBUS_dead_horn" type="int" value="65122"/>
		<constant name="IBUS_dead_iota" type="int" value="65117"/>
		<constant name="IBUS_dead_macron" type="int" value="65108"/>
		<constant name="IBUS_dead_ogonek" type="int" value="65116"/>
		<constant name="IBUS_dead_perispomeni" type="int" value="65107"/>
		<constant name="IBUS_dead_psili" type="int" value="65124"/>
		<constant name="IBUS_dead_semivoiced_sound" type="int" value="65119"/>
		<constant name="IBUS_dead_stroke" type="int" value="65123"/>
		<constant name="IBUS_dead_tilde" type="int" value="65107"/>
		<constant name="IBUS_dead_voiced_sound" type="int" value="65118"/>
		<constant name="IBUS_decimalpoint" type="int" value="2749"/>
		<constant name="IBUS_degree" type="int" value="176"/>
		<constant name="IBUS_diaeresis" type="int" value="168"/>
		<constant name="IBUS_diamond" type="int" value="2797"/>
		<constant name="IBUS_digitspace" type="int" value="2725"/>
		<constant name="IBUS_dintegral" type="int" value="16785964"/>
		<constant name="IBUS_division" type="int" value="247"/>
		<constant name="IBUS_dollar" type="int" value="36"/>
		<constant name="IBUS_doubbaselinedot" type="int" value="2735"/>
		<constant name="IBUS_doubleacute" type="int" value="445"/>
		<constant name="IBUS_doubledagger" type="int" value="2802"/>
		<constant name="IBUS_doublelowquotemark" type="int" value="2814"/>
		<constant name="IBUS_downarrow" type="int" value="2302"/>
		<constant name="IBUS_downcaret" type="int" value="2984"/>
		<constant name="IBUS_downshoe" type="int" value="3030"/>
		<constant name="IBUS_downstile" type="int" value="3012"/>
		<constant name="IBUS_downtack" type="int" value="3010"/>
		<constant name="IBUS_dstroke" type="int" value="496"/>
		<constant name="IBUS_e" type="int" value="101"/>
		<constant name="IBUS_eabovedot" type="int" value="1004"/>
		<constant name="IBUS_eacute" type="int" value="233"/>
		<constant name="IBUS_ebelowdot" type="int" value="16785081"/>
		<constant name="IBUS_ecaron" type="int" value="492"/>
		<constant name="IBUS_ecircumflex" type="int" value="234"/>
		<constant name="IBUS_ecircumflexacute" type="int" value="16785087"/>
		<constant name="IBUS_ecircumflexbelowdot" type="int" value="16785095"/>
		<constant name="IBUS_ecircumflexgrave" type="int" value="16785089"/>
		<constant name="IBUS_ecircumflexhook" type="int" value="16785091"/>
		<constant name="IBUS_ecircumflextilde" type="int" value="16785093"/>
		<constant name="IBUS_ediaeresis" type="int" value="235"/>
		<constant name="IBUS_egrave" type="int" value="232"/>
		<constant name="IBUS_ehook" type="int" value="16785083"/>
		<constant name="IBUS_eightsubscript" type="int" value="16785544"/>
		<constant name="IBUS_eightsuperior" type="int" value="16785528"/>
		<constant name="IBUS_elementof" type="int" value="16785928"/>
		<constant name="IBUS_ellipsis" type="int" value="2734"/>
		<constant name="IBUS_em3space" type="int" value="2723"/>
		<constant name="IBUS_em4space" type="int" value="2724"/>
		<constant name="IBUS_emacron" type="int" value="954"/>
		<constant name="IBUS_emdash" type="int" value="2729"/>
		<constant name="IBUS_emfilledcircle" type="int" value="2782"/>
		<constant name="IBUS_emfilledrect" type="int" value="2783"/>
		<constant name="IBUS_emopencircle" type="int" value="2766"/>
		<constant name="IBUS_emopenrectangle" type="int" value="2767"/>
		<constant name="IBUS_emptyset" type="int" value="16785925"/>
		<constant name="IBUS_emspace" type="int" value="2721"/>
		<constant name="IBUS_endash" type="int" value="2730"/>
		<constant name="IBUS_enfilledcircbullet" type="int" value="2790"/>
		<constant name="IBUS_enfilledsqbullet" type="int" value="2791"/>
		<constant name="IBUS_eng" type="int" value="959"/>
		<constant name="IBUS_enopencircbullet" type="int" value="2784"/>
		<constant name="IBUS_enopensquarebullet" type="int" value="2785"/>
		<constant name="IBUS_enspace" type="int" value="2722"/>
		<constant name="IBUS_eogonek" type="int" value="490"/>
		<constant name="IBUS_equal" type="int" value="61"/>
		<constant name="IBUS_eth" type="int" value="240"/>
		<constant name="IBUS_etilde" type="int" value="16785085"/>
		<constant name="IBUS_exclam" type="int" value="33"/>
		<constant name="IBUS_exclamdown" type="int" value="161"/>
		<constant name="IBUS_f" type="int" value="102"/>
		<constant name="IBUS_fabovedot" type="int" value="16784927"/>
		<constant name="IBUS_femalesymbol" type="int" value="2808"/>
		<constant name="IBUS_ff" type="int" value="2531"/>
		<constant name="IBUS_figdash" type="int" value="2747"/>
		<constant name="IBUS_filledlefttribullet" type="int" value="2780"/>
		<constant name="IBUS_filledrectbullet" type="int" value="2779"/>
		<constant name="IBUS_filledrighttribullet" type="int" value="2781"/>
		<constant name="IBUS_filledtribulletdown" type="int" value="2793"/>
		<constant name="IBUS_filledtribulletup" type="int" value="2792"/>
		<constant name="IBUS_fiveeighths" type="int" value="2757"/>
		<constant name="IBUS_fivesixths" type="int" value="2743"/>
		<constant name="IBUS_fivesubscript" type="int" value="16785541"/>
		<constant name="IBUS_fivesuperior" type="int" value="16785525"/>
		<constant name="IBUS_fourfifths" type="int" value="2741"/>
		<constant name="IBUS_foursubscript" type="int" value="16785540"/>
		<constant name="IBUS_foursuperior" type="int" value="16785524"/>
		<constant name="IBUS_fourthroot" type="int" value="16785948"/>
		<constant name="IBUS_function" type="int" value="2294"/>
		<constant name="IBUS_g" type="int" value="103"/>
		<constant name="IBUS_gabovedot" type="int" value="757"/>
		<constant name="IBUS_gbreve" type="int" value="699"/>
		<constant name="IBUS_gcaron" type="int" value="16777703"/>
		<constant name="IBUS_gcedilla" type="int" value="955"/>
		<constant name="IBUS_gcircumflex" type="int" value="760"/>
		<constant name="IBUS_grave" type="int" value="96"/>
		<constant name="IBUS_greater" type="int" value="62"/>
		<constant name="IBUS_greaterthanequal" type="int" value="2238"/>
		<constant name="IBUS_guillemotleft" type="int" value="171"/>
		<constant name="IBUS_guillemotright" type="int" value="187"/>
		<constant name="IBUS_h" type="int" value="104"/>
		<constant name="IBUS_hairspace" type="int" value="2728"/>
		<constant name="IBUS_hcircumflex" type="int" value="694"/>
		<constant name="IBUS_heart" type="int" value="2798"/>
		<constant name="IBUS_hebrew_aleph" type="int" value="3296"/>
		<constant name="IBUS_hebrew_ayin" type="int" value="3314"/>
		<constant name="IBUS_hebrew_bet" type="int" value="3297"/>
		<constant name="IBUS_hebrew_beth" type="int" value="3297"/>
		<constant name="IBUS_hebrew_chet" type="int" value="3303"/>
		<constant name="IBUS_hebrew_dalet" type="int" value="3299"/>
		<constant name="IBUS_hebrew_daleth" type="int" value="3299"/>
		<constant name="IBUS_hebrew_doublelowline" type="int" value="3295"/>
		<constant name="IBUS_hebrew_finalkaph" type="int" value="3306"/>
		<constant name="IBUS_hebrew_finalmem" type="int" value="3309"/>
		<constant name="IBUS_hebrew_finalnun" type="int" value="3311"/>
		<constant name="IBUS_hebrew_finalpe" type="int" value="3315"/>
		<constant name="IBUS_hebrew_finalzade" type="int" value="3317"/>
		<constant name="IBUS_hebrew_finalzadi" type="int" value="3317"/>
		<constant name="IBUS_hebrew_gimel" type="int" value="3298"/>
		<constant name="IBUS_hebrew_gimmel" type="int" value="3298"/>
		<constant name="IBUS_hebrew_he" type="int" value="3300"/>
		<constant name="IBUS_hebrew_het" type="int" value="3303"/>
		<constant name="IBUS_hebrew_kaph" type="int" value="3307"/>
		<constant name="IBUS_hebrew_kuf" type="int" value="3319"/>
		<constant name="IBUS_hebrew_lamed" type="int" value="3308"/>
		<constant name="IBUS_hebrew_mem" type="int" value="3310"/>
		<constant name="IBUS_hebrew_nun" type="int" value="3312"/>
		<constant name="IBUS_hebrew_pe" type="int" value="3316"/>
		<constant name="IBUS_hebrew_qoph" type="int" value="3319"/>
		<constant name="IBUS_hebrew_resh" type="int" value="3320"/>
		<constant name="IBUS_hebrew_samech" type="int" value="3313"/>
		<constant name="IBUS_hebrew_samekh" type="int" value="3313"/>
		<constant name="IBUS_hebrew_shin" type="int" value="3321"/>
		<constant name="IBUS_hebrew_taf" type="int" value="3322"/>
		<constant name="IBUS_hebrew_taw" type="int" value="3322"/>
		<constant name="IBUS_hebrew_tet" type="int" value="3304"/>
		<constant name="IBUS_hebrew_teth" type="int" value="3304"/>
		<constant name="IBUS_hebrew_waw" type="int" value="3301"/>
		<constant name="IBUS_hebrew_yod" type="int" value="3305"/>
		<constant name="IBUS_hebrew_zade" type="int" value="3318"/>
		<constant name="IBUS_hebrew_zadi" type="int" value="3318"/>
		<constant name="IBUS_hebrew_zain" type="int" value="3302"/>
		<constant name="IBUS_hebrew_zayin" type="int" value="3302"/>
		<constant name="IBUS_hexagram" type="int" value="2778"/>
		<constant name="IBUS_horizconnector" type="int" value="2211"/>
		<constant name="IBUS_horizlinescan1" type="int" value="2543"/>
		<constant name="IBUS_horizlinescan3" type="int" value="2544"/>
		<constant name="IBUS_horizlinescan5" type="int" value="2545"/>
		<constant name="IBUS_horizlinescan7" type="int" value="2546"/>
		<constant name="IBUS_horizlinescan9" type="int" value="2547"/>
		<constant name="IBUS_hstroke" type="int" value="689"/>
		<constant name="IBUS_ht" type="int" value="2530"/>
		<constant name="IBUS_hyphen" type="int" value="173"/>
		<constant name="IBUS_i" type="int" value="105"/>
		<constant name="IBUS_iacute" type="int" value="237"/>
		<constant name="IBUS_ibelowdot" type="int" value="16785099"/>
		<constant name="IBUS_ibreve" type="int" value="16777517"/>
		<constant name="IBUS_icircumflex" type="int" value="238"/>
		<constant name="IBUS_identical" type="int" value="2255"/>
		<constant name="IBUS_idiaeresis" type="int" value="239"/>
		<constant name="IBUS_idotless" type="int" value="697"/>
		<constant name="IBUS_ifonlyif" type="int" value="2253"/>
		<constant name="IBUS_igrave" type="int" value="236"/>
		<constant name="IBUS_ihook" type="int" value="16785097"/>
		<constant name="IBUS_imacron" type="int" value="1007"/>
		<constant name="IBUS_implies" type="int" value="2254"/>
		<constant name="IBUS_includedin" type="int" value="2266"/>
		<constant name="IBUS_includes" type="int" value="2267"/>
		<constant name="IBUS_infinity" type="int" value="2242"/>
		<constant name="IBUS_integral" type="int" value="2239"/>
		<constant name="IBUS_intersection" type="int" value="2268"/>
		<constant name="IBUS_iogonek" type="int" value="999"/>
		<constant name="IBUS_itilde" type="int" value="949"/>
		<constant name="IBUS_j" type="int" value="106"/>
		<constant name="IBUS_jcircumflex" type="int" value="700"/>
		<constant name="IBUS_jot" type="int" value="3018"/>
		<constant name="IBUS_k" type="int" value="107"/>
		<constant name="IBUS_kana_A" type="int" value="1201"/>
		<constant name="IBUS_kana_CHI" type="int" value="1217"/>
		<constant name="IBUS_kana_E" type="int" value="1204"/>
		<constant name="IBUS_kana_FU" type="int" value="1228"/>
		<constant name="IBUS_kana_HA" type="int" value="1226"/>
		<constant name="IBUS_kana_HE" type="int" value="1229"/>
		<constant name="IBUS_kana_HI" type="int" value="1227"/>
		<constant name="IBUS_kana_HO" type="int" value="1230"/>
		<constant name="IBUS_kana_HU" type="int" value="1228"/>
		<constant name="IBUS_kana_I" type="int" value="1202"/>
		<constant name="IBUS_kana_KA" type="int" value="1206"/>
		<constant name="IBUS_kana_KE" type="int" value="1209"/>
		<constant name="IBUS_kana_KI" type="int" value="1207"/>
		<constant name="IBUS_kana_KO" type="int" value="1210"/>
		<constant name="IBUS_kana_KU" type="int" value="1208"/>
		<constant name="IBUS_kana_MA" type="int" value="1231"/>
		<constant name="IBUS_kana_ME" type="int" value="1234"/>
		<constant name="IBUS_kana_MI" type="int" value="1232"/>
		<constant name="IBUS_kana_MO" type="int" value="1235"/>
		<constant name="IBUS_kana_MU" type="int" value="1233"/>
		<constant name="IBUS_kana_N" type="int" value="1245"/>
		<constant name="IBUS_kana_NA" type="int" value="1221"/>
		<constant name="IBUS_kana_NE" type="int" value="1224"/>
		<constant name="IBUS_kana_NI" type="int" value="1222"/>
		<constant name="IBUS_kana_NO" type="int" value="1225"/>
		<constant name="IBUS_kana_NU" type="int" value="1223"/>
		<constant name="IBUS_kana_O" type="int" value="1205"/>
		<constant name="IBUS_kana_RA" type="int" value="1239"/>
		<constant name="IBUS_kana_RE" type="int" value="1242"/>
		<constant name="IBUS_kana_RI" type="int" value="1240"/>
		<constant name="IBUS_kana_RO" type="int" value="1243"/>
		<constant name="IBUS_kana_RU" type="int" value="1241"/>
		<constant name="IBUS_kana_SA" type="int" value="1211"/>
		<constant name="IBUS_kana_SE" type="int" value="1214"/>
		<constant name="IBUS_kana_SHI" type="int" value="1212"/>
		<constant name="IBUS_kana_SO" type="int" value="1215"/>
		<constant name="IBUS_kana_SU" type="int" value="1213"/>
		<constant name="IBUS_kana_TA" type="int" value="1216"/>
		<constant name="IBUS_kana_TE" type="int" value="1219"/>
		<constant name="IBUS_kana_TI" type="int" value="1217"/>
		<constant name="IBUS_kana_TO" type="int" value="1220"/>
		<constant name="IBUS_kana_TSU" type="int" value="1218"/>
		<constant name="IBUS_kana_TU" type="int" value="1218"/>
		<constant name="IBUS_kana_U" type="int" value="1203"/>
		<constant name="IBUS_kana_WA" type="int" value="1244"/>
		<constant name="IBUS_kana_WO" type="int" value="1190"/>
		<constant name="IBUS_kana_YA" type="int" value="1236"/>
		<constant name="IBUS_kana_YO" type="int" value="1238"/>
		<constant name="IBUS_kana_YU" type="int" value="1237"/>
		<constant name="IBUS_kana_a" type="int" value="1191"/>
		<constant name="IBUS_kana_closingbracket" type="int" value="1187"/>
		<constant name="IBUS_kana_comma" type="int" value="1188"/>
		<constant name="IBUS_kana_conjunctive" type="int" value="1189"/>
		<constant name="IBUS_kana_e" type="int" value="1194"/>
		<constant name="IBUS_kana_fullstop" type="int" value="1185"/>
		<constant name="IBUS_kana_i" type="int" value="1192"/>
		<constant name="IBUS_kana_middledot" type="int" value="1189"/>
		<constant name="IBUS_kana_o" type="int" value="1195"/>
		<constant name="IBUS_kana_openingbracket" type="int" value="1186"/>
		<constant name="IBUS_kana_switch" type="int" value="65406"/>
		<constant name="IBUS_kana_tsu" type="int" value="1199"/>
		<constant name="IBUS_kana_tu" type="int" value="1199"/>
		<constant name="IBUS_kana_u" type="int" value="1193"/>
		<constant name="IBUS_kana_ya" type="int" value="1196"/>
		<constant name="IBUS_kana_yo" type="int" value="1198"/>
		<constant name="IBUS_kana_yu" type="int" value="1197"/>
		<constant name="IBUS_kappa" type="int" value="930"/>
		<constant name="IBUS_kcedilla" type="int" value="1011"/>
		<constant name="IBUS_kra" type="int" value="930"/>
		<constant name="IBUS_l" type="int" value="108"/>
		<constant name="IBUS_lacute" type="int" value="485"/>
		<constant name="IBUS_latincross" type="int" value="2777"/>
		<constant name="IBUS_lbelowdot" type="int" value="16784951"/>
		<constant name="IBUS_lcaron" type="int" value="437"/>
		<constant name="IBUS_lcedilla" type="int" value="950"/>
		<constant name="IBUS_leftanglebracket" type="int" value="2748"/>
		<constant name="IBUS_leftarrow" type="int" value="2299"/>
		<constant name="IBUS_leftcaret" type="int" value="2979"/>
		<constant name="IBUS_leftdoublequotemark" type="int" value="2770"/>
		<constant name="IBUS_leftmiddlecurlybrace" type="int" value="2223"/>
		<constant name="IBUS_leftopentriangle" type="int" value="2764"/>
		<constant name="IBUS_leftpointer" type="int" value="2794"/>
		<constant name="IBUS_leftradical" type="int" value="2209"/>
		<constant name="IBUS_leftshoe" type="int" value="3034"/>
		<constant name="IBUS_leftsinglequotemark" type="int" value="2768"/>
		<constant name="IBUS_leftt" type="int" value="2548"/>
		<constant name="IBUS_lefttack" type="int" value="3036"/>
		<constant name="IBUS_less" type="int" value="60"/>
		<constant name="IBUS_lessthanequal" type="int" value="2236"/>
		<constant name="IBUS_lf" type="int" value="2533"/>
		<constant name="IBUS_logicaland" type="int" value="2270"/>
		<constant name="IBUS_logicalor" type="int" value="2271"/>
		<constant name="IBUS_lowleftcorner" type="int" value="2541"/>
		<constant name="IBUS_lowrightcorner" type="int" value="2538"/>
		<constant name="IBUS_lstroke" type="int" value="435"/>
		<constant name="IBUS_m" type="int" value="109"/>
		<constant name="IBUS_mabovedot" type="int" value="16784961"/>
		<constant name="IBUS_macron" type="int" value="175"/>
		<constant name="IBUS_malesymbol" type="int" value="2807"/>
		<constant name="IBUS_maltesecross" type="int" value="2800"/>
		<constant name="IBUS_marker" type="int" value="2751"/>
		<constant name="IBUS_masculine" type="int" value="186"/>
		<constant name="IBUS_minus" type="int" value="45"/>
		<constant name="IBUS_minutes" type="int" value="2774"/>
		<constant name="IBUS_mu" type="int" value="181"/>
		<constant name="IBUS_multiply" type="int" value="215"/>
		<constant name="IBUS_musicalflat" type="int" value="2806"/>
		<constant name="IBUS_musicalsharp" type="int" value="2805"/>
		<constant name="IBUS_n" type="int" value="110"/>
		<constant name="IBUS_nabla" type="int" value="2245"/>
		<constant name="IBUS_nacute" type="int" value="497"/>
		<constant name="IBUS_ncaron" type="int" value="498"/>
		<constant name="IBUS_ncedilla" type="int" value="1009"/>
		<constant name="IBUS_ninesubscript" type="int" value="16785545"/>
		<constant name="IBUS_ninesuperior" type="int" value="16785529"/>
		<constant name="IBUS_nl" type="int" value="2536"/>
		<constant name="IBUS_nobreakspace" type="int" value="160"/>
		<constant name="IBUS_notapproxeq" type="int" value="16785991"/>
		<constant name="IBUS_notelementof" type="int" value="16785929"/>
		<constant name="IBUS_notequal" type="int" value="2237"/>
		<constant name="IBUS_notidentical" type="int" value="16786018"/>
		<constant name="IBUS_notsign" type="int" value="172"/>
		<constant name="IBUS_ntilde" type="int" value="241"/>
		<constant name="IBUS_numbersign" type="int" value="35"/>
		<constant name="IBUS_numerosign" type="int" value="1712"/>
		<constant name="IBUS_o" type="int" value="111"/>
		<constant name="IBUS_oacute" type="int" value="243"/>
		<constant name="IBUS_obarred" type="int" value="16777845"/>
		<constant name="IBUS_obelowdot" type="int" value="16785101"/>
		<constant name="IBUS_ocaron" type="int" value="16777682"/>
		<constant name="IBUS_ocircumflex" type="int" value="244"/>
		<constant name="IBUS_ocircumflexacute" type="int" value="16785105"/>
		<constant name="IBUS_ocircumflexbelowdot" type="int" value="16785113"/>
		<constant name="IBUS_ocircumflexgrave" type="int" value="16785107"/>
		<constant name="IBUS_ocircumflexhook" type="int" value="16785109"/>
		<constant name="IBUS_ocircumflextilde" type="int" value="16785111"/>
		<constant name="IBUS_odiaeresis" type="int" value="246"/>
		<constant name="IBUS_odoubleacute" type="int" value="501"/>
		<constant name="IBUS_oe" type="int" value="5053"/>
		<constant name="IBUS_ogonek" type="int" value="434"/>
		<constant name="IBUS_ograve" type="int" value="242"/>
		<constant name="IBUS_ohook" type="int" value="16785103"/>
		<constant name="IBUS_ohorn" type="int" value="16777633"/>
		<constant name="IBUS_ohornacute" type="int" value="16785115"/>
		<constant name="IBUS_ohornbelowdot" type="int" value="16785123"/>
		<constant name="IBUS_ohorngrave" type="int" value="16785117"/>
		<constant name="IBUS_ohornhook" type="int" value="16785119"/>
		<constant name="IBUS_ohorntilde" type="int" value="16785121"/>
		<constant name="IBUS_omacron" type="int" value="1010"/>
		<constant name="IBUS_oneeighth" type="int" value="2755"/>
		<constant name="IBUS_onefifth" type="int" value="2738"/>
		<constant name="IBUS_onehalf" type="int" value="189"/>
		<constant name="IBUS_onequarter" type="int" value="188"/>
		<constant name="IBUS_onesixth" type="int" value="2742"/>
		<constant name="IBUS_onesubscript" type="int" value="16785537"/>
		<constant name="IBUS_onesuperior" type="int" value="185"/>
		<constant name="IBUS_onethird" type="int" value="2736"/>
		<constant name="IBUS_ooblique" type="int" value="248"/>
		<constant name="IBUS_openrectbullet" type="int" value="2786"/>
		<constant name="IBUS_openstar" type="int" value="2789"/>
		<constant name="IBUS_opentribulletdown" type="int" value="2788"/>
		<constant name="IBUS_opentribulletup" type="int" value="2787"/>
		<constant name="IBUS_ordfeminine" type="int" value="170"/>
		<constant name="IBUS_oslash" type="int" value="248"/>
		<constant name="IBUS_otilde" type="int" value="245"/>
		<constant name="IBUS_overbar" type="int" value="3008"/>
		<constant name="IBUS_overline" type="int" value="1150"/>
		<constant name="IBUS_p" type="int" value="112"/>
		<constant name="IBUS_pabovedot" type="int" value="16784983"/>
		<constant name="IBUS_paragraph" type="int" value="182"/>
		<constant name="IBUS_parenleft" type="int" value="40"/>
		<constant name="IBUS_parenright" type="int" value="41"/>
		<constant name="IBUS_partdifferential" type="int" value="16785922"/>
		<constant name="IBUS_partialderivative" type="int" value="2287"/>
		<constant name="IBUS_percent" type="int" value="37"/>
		<constant name="IBUS_period" type="int" value="46"/>
		<constant name="IBUS_periodcentered" type="int" value="183"/>
		<constant name="IBUS_phonographcopyright" type="int" value="2811"/>
		<constant name="IBUS_plus" type="int" value="43"/>
		<constant name="IBUS_plusminus" type="int" value="177"/>
		<constant name="IBUS_prescription" type="int" value="2772"/>
		<constant name="IBUS_prolongedsound" type="int" value="1200"/>
		<constant name="IBUS_punctspace" type="int" value="2726"/>
		<constant name="IBUS_q" type="int" value="113"/>
		<constant name="IBUS_quad" type="int" value="3020"/>
		<constant name="IBUS_question" type="int" value="63"/>
		<constant name="IBUS_questiondown" type="int" value="191"/>
		<constant name="IBUS_quotedbl" type="int" value="34"/>
		<constant name="IBUS_quoteleft" type="int" value="96"/>
		<constant name="IBUS_quoteright" type="int" value="39"/>
		<constant name="IBUS_r" type="int" value="114"/>
		<constant name="IBUS_racute" type="int" value="480"/>
		<constant name="IBUS_radical" type="int" value="2262"/>
		<constant name="IBUS_rcaron" type="int" value="504"/>
		<constant name="IBUS_rcedilla" type="int" value="947"/>
		<constant name="IBUS_registered" type="int" value="174"/>
		<constant name="IBUS_rightanglebracket" type="int" value="2750"/>
		<constant name="IBUS_rightarrow" type="int" value="2301"/>
		<constant name="IBUS_rightcaret" type="int" value="2982"/>
		<constant name="IBUS_rightdoublequotemark" type="int" value="2771"/>
		<constant name="IBUS_rightmiddlecurlybrace" type="int" value="2224"/>
		<constant name="IBUS_rightmiddlesummation" type="int" value="2231"/>
		<constant name="IBUS_rightopentriangle" type="int" value="2765"/>
		<constant name="IBUS_rightpointer" type="int" value="2795"/>
		<constant name="IBUS_rightshoe" type="int" value="3032"/>
		<constant name="IBUS_rightsinglequotemark" type="int" value="2769"/>
		<constant name="IBUS_rightt" type="int" value="2549"/>
		<constant name="IBUS_righttack" type="int" value="3068"/>
		<constant name="IBUS_s" type="int" value="115"/>
		<constant name="IBUS_sabovedot" type="int" value="16784993"/>
		<constant name="IBUS_sacute" type="int" value="438"/>
		<constant name="IBUS_scaron" type="int" value="441"/>
		<constant name="IBUS_scedilla" type="int" value="442"/>
		<constant name="IBUS_schwa" type="int" value="16777817"/>
		<constant name="IBUS_scircumflex" type="int" value="766"/>
		<constant name="IBUS_script_switch" type="int" value="65406"/>
		<constant name="IBUS_seconds" type="int" value="2775"/>
		<constant name="IBUS_section" type="int" value="167"/>
		<constant name="IBUS_semicolon" type="int" value="59"/>
		<constant name="IBUS_semivoicedsound" type="int" value="1247"/>
		<constant name="IBUS_seveneighths" type="int" value="2758"/>
		<constant name="IBUS_sevensubscript" type="int" value="16785543"/>
		<constant name="IBUS_sevensuperior" type="int" value="16785527"/>
		<constant name="IBUS_signaturemark" type="int" value="2762"/>
		<constant name="IBUS_signifblank" type="int" value="2732"/>
		<constant name="IBUS_similarequal" type="int" value="2249"/>
		<constant name="IBUS_singlelowquotemark" type="int" value="2813"/>
		<constant name="IBUS_sixsubscript" type="int" value="16785542"/>
		<constant name="IBUS_sixsuperior" type="int" value="16785526"/>
		<constant name="IBUS_slash" type="int" value="47"/>
		<constant name="IBUS_soliddiamond" type="int" value="2528"/>
		<constant name="IBUS_space" type="int" value="32"/>
		<constant name="IBUS_squareroot" type="int" value="16785946"/>
		<constant name="IBUS_ssharp" type="int" value="223"/>
		<constant name="IBUS_sterling" type="int" value="163"/>
		<constant name="IBUS_stricteq" type="int" value="16786019"/>
		<constant name="IBUS_t" type="int" value="116"/>
		<constant name="IBUS_tabovedot" type="int" value="16785003"/>
		<constant name="IBUS_tcaron" type="int" value="443"/>
		<constant name="IBUS_tcedilla" type="int" value="510"/>
		<constant name="IBUS_telephone" type="int" value="2809"/>
		<constant name="IBUS_telephonerecorder" type="int" value="2810"/>
		<constant name="IBUS_therefore" type="int" value="2240"/>
		<constant name="IBUS_thinspace" type="int" value="2727"/>
		<constant name="IBUS_thorn" type="int" value="254"/>
		<constant name="IBUS_threeeighths" type="int" value="2756"/>
		<constant name="IBUS_threefifths" type="int" value="2740"/>
		<constant name="IBUS_threequarters" type="int" value="190"/>
		<constant name="IBUS_threesubscript" type="int" value="16785539"/>
		<constant name="IBUS_threesuperior" type="int" value="179"/>
		<constant name="IBUS_tintegral" type="int" value="16785965"/>
		<constant name="IBUS_topintegral" type="int" value="2212"/>
		<constant name="IBUS_topleftparens" type="int" value="2219"/>
		<constant name="IBUS_topleftradical" type="int" value="2210"/>
		<constant name="IBUS_topleftsqbracket" type="int" value="2215"/>
		<constant name="IBUS_topleftsummation" type="int" value="2225"/>
		<constant name="IBUS_toprightparens" type="int" value="2221"/>
		<constant name="IBUS_toprightsqbracket" type="int" value="2217"/>
		<constant name="IBUS_toprightsummation" type="int" value="2229"/>
		<constant name="IBUS_topt" type="int" value="2551"/>
		<constant name="IBUS_topvertsummationconnector" type="int" value="2227"/>
		<constant name="IBUS_trademark" type="int" value="2761"/>
		<constant name="IBUS_trademarkincircle" type="int" value="2763"/>
		<constant name="IBUS_tslash" type="int" value="956"/>
		<constant name="IBUS_twofifths" type="int" value="2739"/>
		<constant name="IBUS_twosubscript" type="int" value="16785538"/>
		<constant name="IBUS_twosuperior" type="int" value="178"/>
		<constant name="IBUS_twothirds" type="int" value="2737"/>
		<constant name="IBUS_u" type="int" value="117"/>
		<constant name="IBUS_uacute" type="int" value="250"/>
		<constant name="IBUS_ubelowdot" type="int" value="16785125"/>
		<constant name="IBUS_ubreve" type="int" value="765"/>
		<constant name="IBUS_ucircumflex" type="int" value="251"/>
		<constant name="IBUS_udiaeresis" type="int" value="252"/>
		<constant name="IBUS_udoubleacute" type="int" value="507"/>
		<constant name="IBUS_ugrave" type="int" value="249"/>
		<constant name="IBUS_uhook" type="int" value="16785127"/>
		<constant name="IBUS_uhorn" type="int" value="16777648"/>
		<constant name="IBUS_uhornacute" type="int" value="16785129"/>
		<constant name="IBUS_uhornbelowdot" type="int" value="16785137"/>
		<constant name="IBUS_uhorngrave" type="int" value="16785131"/>
		<constant name="IBUS_uhornhook" type="int" value="16785133"/>
		<constant name="IBUS_uhorntilde" type="int" value="16785135"/>
		<constant name="IBUS_umacron" type="int" value="1022"/>
		<constant name="IBUS_underbar" type="int" value="3014"/>
		<constant name="IBUS_underscore" type="int" value="95"/>
		<constant name="IBUS_union" type="int" value="2269"/>
		<constant name="IBUS_uogonek" type="int" value="1017"/>
		<constant name="IBUS_uparrow" type="int" value="2300"/>
		<constant name="IBUS_upcaret" type="int" value="2985"/>
		<constant name="IBUS_upleftcorner" type="int" value="2540"/>
		<constant name="IBUS_uprightcorner" type="int" value="2539"/>
		<constant name="IBUS_upshoe" type="int" value="3011"/>
		<constant name="IBUS_upstile" type="int" value="3027"/>
		<constant name="IBUS_uptack" type="int" value="3022"/>
		<constant name="IBUS_uring" type="int" value="505"/>
		<constant name="IBUS_utilde" type="int" value="1021"/>
		<constant name="IBUS_v" type="int" value="118"/>
		<constant name="IBUS_variation" type="int" value="2241"/>
		<constant name="IBUS_vertbar" type="int" value="2552"/>
		<constant name="IBUS_vertconnector" type="int" value="2214"/>
		<constant name="IBUS_voicedsound" type="int" value="1246"/>
		<constant name="IBUS_vt" type="int" value="2537"/>
		<constant name="IBUS_w" type="int" value="119"/>
		<constant name="IBUS_wacute" type="int" value="16785027"/>
		<constant name="IBUS_wcircumflex" type="int" value="16777589"/>
		<constant name="IBUS_wdiaeresis" type="int" value="16785029"/>
		<constant name="IBUS_wgrave" type="int" value="16785025"/>
		<constant name="IBUS_x" type="int" value="120"/>
		<constant name="IBUS_xabovedot" type="int" value="16785035"/>
		<constant name="IBUS_y" type="int" value="121"/>
		<constant name="IBUS_yacute" type="int" value="253"/>
		<constant name="IBUS_ybelowdot" type="int" value="16785141"/>
		<constant name="IBUS_ycircumflex" type="int" value="16777591"/>
		<constant name="IBUS_ydiaeresis" type="int" value="255"/>
		<constant name="IBUS_yen" type="int" value="165"/>
		<constant name="IBUS_ygrave" type="int" value="16785139"/>
		<constant name="IBUS_yhook" type="int" value="16785143"/>
		<constant name="IBUS_ytilde" type="int" value="16785145"/>
		<constant name="IBUS_z" type="int" value="122"/>
		<constant name="IBUS_zabovedot" type="int" value="447"/>
		<constant name="IBUS_zacute" type="int" value="444"/>
		<constant name="IBUS_zcaron" type="int" value="446"/>
		<constant name="IBUS_zerosubscript" type="int" value="16785536"/>
		<constant name="IBUS_zerosuperior" type="int" value="16785520"/>
		<constant name="IBUS_zstroke" type="int" value="16777654"/>
	</namespace>
</api>
