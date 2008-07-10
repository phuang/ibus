from os import path
import gtk
import gobject
import ibus
from gtk import gdk, glade
import dbus.mainloop.glib

(
	NAME_COLUMN,
	ENABLE_COLUMN,
	VISIBLE_COLUMN,
	DATA_COLUMN,
) = range (4)

class Setup:
	def __init__ (self):
		self._conn = ibus.Connection ()
		self._ibus = self._conn.get_ibus ()
		glade_file = path.join (path.dirname (__file__), "./setup.glade")
		self._xml = glade.XML (glade_file)
		self._dialog = self._xml.get_widget ("dialog_setup")
		self._tree = self._xml.get_widget ("treeview_engines")
		model = self.__create_model ()
		self._tree.set_model (model)
		
		# column for holiday names
		renderer = gtk.CellRendererText()
		renderer.set_property("xalign", 0.0)
		
		#col_offset = gtk.TreeViewColumn("Holiday", renderer, text=HOLIDAY_NAME_COLUMN)
		column = gtk.TreeViewColumn("Name", renderer, text = NAME_COLUMN)
		# column.set_clickable(True)
		self._tree.append_column(column)
		
		# column for holiday names
		renderer = gtk.CellRendererToggle()
		renderer.set_data ('column', ENABLE_COLUMN)
		renderer.set_property("xalign", 0.0)
		renderer.connect("toggled", self.__item_toggled_cb, model)
		
		#col_offset = gtk.TreeViewColumn("Holiday", renderer, text=HOLIDAY_NAME_COLUMN)
		column = gtk.TreeViewColumn("Started", renderer, active = ENABLE_COLUMN, visible = VISIBLE_COLUMN)
		column.set_clickable(True)
		self._tree.append_column(column)


	def _init_ui (self):
		model = self.__create_model ()

	def __item_toggled_cb (self, cell, path_str, model):
		
		# get toggled iter
		iter = model.get_iter_from_string(path_str)
		data = model.get_value (iter, DATA_COLUMN)
		
		# do something with the value
		if data[6] == False:
			try:
				self._ibus.RegisterStartEngine (data[1], data[0])
			except Exception, e:
				dlg = gtk.MessageDialog (type = gtk.MESSAGE_ERROR,
						buttons = gtk.BUTTONS_CLOSE,
						message_format = str(e))
				dlg.run ()
				return
		else:
			try:
				self._ibus.RegisterStopEngine (data[1], data[0])
			except Exception, e:
				dlg = gtk.MessageDialog (type = gtk.MESSAGE_ERROR,
						buttons = gtk.BUTTONS_CLOSE,
						message_format = str(e))
				dlg.run ()
				return
		data[6] = not data[6]
		
		# set new value
		model.set(iter, ENABLE_COLUMN, data[6])
		
	
	def __create_model (self):
		# create tree store
		model = gtk.TreeStore (
			gobject.TYPE_STRING,
			gobject.TYPE_BOOLEAN,
			gobject.TYPE_BOOLEAN,
			gobject.TYPE_PYOBJECT)

		langs = {}

		for name, lang, icon, author, credits, _exec, started in self._ibus.RegisterListEngines ():
			_lang = ibus.LANGUAGES.get (lang, "other")
			if _lang not in langs:
				langs[_lang] = []
			langs[_lang].append ([name, lang, icon, author, credits, _exec, started])

		keys = langs.keys ()
		keys.sort ()
		for key in keys:
			iter = model.append (None)
			model.set (iter,
				NAME_COLUMN, key,
				ENABLE_COLUMN, False,
				VISIBLE_COLUMN, False,
				DATA_COLUMN, None)
			langs[key].sort ()
			for name, lang, icon, author, credits, _exec, started in langs[key]:
				child_iter = model.append (iter)
				model.set (child_iter,
					NAME_COLUMN, name,
					ENABLE_COLUMN, started,
					VISIBLE_COLUMN, True,
					DATA_COLUMN, 
					[name, lang, icon, author, credits, _exec, started])

		return model


	def run (self):
		return self._dialog.run ()

if __name__ == "__main__":
	dbus.mainloop.glib.DBusGMainLoop (set_as_default = True)
	Setup ().run ()
