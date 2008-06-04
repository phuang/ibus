import dbus

PROP_TYPE_NORMAL = 0
PROP_TYPE_TOGGLE = 1
PROP_TYPE_RADIO  = 2
PROP_TYPE_SEPARATOR = 3

PROP_STATE_UNCHECKED = 0
PROP_STATE_CHECKED = 1
PROP_STATE_INCONSISTENT = 2

class Property:
	def __init__ (self, name,
						type = PROP_TYPE_NORMAL,
						label = "",
						icon = "",
						tip = "",
						sensitive = True,
						visible = True,
						state = PROP_STATE_CHECKED):
		self._name = name
		self._type = type
		self._label = label
		self._icon = icon
		self._tip = tip
		self._sensitive = sensitive
		self._visible = visible
		self._state = state
		self._sub_props = PropList ()

	def set_sub_props (self, props):
		self._sub_props = props

	def get_sub_props (self):
		return self._sub_props

	def to_dbus_value (self):
		sub_props = self._sub_props.to_dbus_value ()
		values = (dbus.String (self._name),
				dbus.Int32 (self._type),
				dbus.String (self._label),
				dbus.String (self._icon),
				dbus.String (self._tip),
				dbus.Boolean (self._sensitive),
				dbus.Boolean (self._visible),
				dbus.Int32 (self._state),
				sub_props)
		return dbus.Struct (values)
	
	def from_dbus_value (self, value):
		self._name, \
		self._type, \
		self._label, \
		self._icon, \
		self._tip, \
		self._sensitive, \
		self._visible, \
		self._state, \
		props = value

		self._sub_props = prop_list_from_dbus_value (props)

def property_from_dbus_value (value):
	p = Property ("")
	p.from_dbus_value (value)
	return p

class PropList:
	def __init__ (self):
		self._props = []

	def append (self, prop):
		self._props.append (prop)

	def prepand (self, prop):
		self._props.insert (0, prop)

	def insert (self, index, prop):
		self._props.insert (index, prop)

	def get_properties (self):
		return self._props[:]

	def to_dbus_value (self):
		props = map (lambda p: p.to_dbus_value (), self._props)
		return dbus.Array (props, signature = "v")

	def from_dbus_value (self, value):
		props = []
		for p in value:
			props.append (property_from_dbus_value (p))
		self._props = props

def prop_list_from_dbus_value (value):
	props = PropList ()
	props.from_dbus_value (value)
	return props

def test ():
	props = PropList ()
	props.append (Property ("a"))
	props.append (Property ("b"))
	props.append (Property ("c"))
	props.append (Property ("d"))
	value = props.to_dbus_value ()
	print prop_list_from_dbus_value (value)

	p = Property ("z")
	p.set_sub_props (props)
	props = PropList ()
	props.append (p)
	value = props.to_dbus_value ()
	print prop_list_from_dbus_value (value)

if __name__ == "__main__":
	test ()
