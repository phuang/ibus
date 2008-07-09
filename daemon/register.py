# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

from os import path
import glob
import ibus

class Engine:
	def __init__ (self, name, lang = "other", icon = "", author = "", credits = "", _exec = "", pid = 0):
		self._name = name
		self._lang = lang
		self._icon = icon
		self._author = author
		self._credits = credits
		self._exec = _exec
		self._pid = pid

	def __eq__ (self, o):
		# We don't test icon author & credits
		return self._name == o._name and \
			self._lang == o._lang and \
			self._exec == o._exec

	def __str__ (self):
		return "Engine ('%s', '%s', '%s', '%s', '%s', '%s', %d" % (self._name, self._lang, \
			self._icon, self._author, \
			self._credits, self._exec, \
			self._pid)

class Register (ibus.Object):
	def __init__ (self):
		ibus.Object.__init__ (self)
		self._engines = {}

	def load (self):
		_file = path.abspath (__file__)
		_dir = path.dirname (_file) + "./../engine"
		_dir = path.abspath (_dir)
		_dir = "/usr/share/ibus/engine"
		for _file in glob.glob (_dir + "/*.engine"):
			engine = self._load_engine (_file)
			if (engine._lang, engine._name) in self._engines:
				old_engine = self._engines[(engine._lang, engine._name)]
				if old_engine == engine:
					engine._pid = old_engine._pid
					self._engines[(engine._lang, engine._name)] = engine
				else:
					self._engines[(engine._lang, engine._name + " (old)")] = old_engine
					self._engines[(engine._lang, engine._name)] = engine
			else:
				self._engines[(engine._lang, engine._name)] = engine

	def _load_engine (self, _file):
		f = file (_file)
		name = None
		lang = "other"
		icon = ""
		author = ""
		credits = ""
		_exec = None
		line = 0
		for l in f:
			line += 1
			l = l.strip ()
			if l.startswith ("#"):
				continue
			n, v = l.split ("=")
			if n == "Name":
				name = v
			elif n == "Lang":
				lang = v
			elif n == "Icon":
				icon = v
			elif n == "Author":
				author = v
			elif n == "Credits":
				credits = v
			elif n == "Exec":
				_exec = v
			else:
				raise Exception ("%s:%d\nUnknown value name = %s" % (_file, line, n))

		if name == None:
			raise Exception ("%s: no name" % _file)
		if _exec == None:
			raise Exception ("%s: no exec" % _file)

		return Engine (name, lang, icon, author, credits, _exec)

if __name__ == "__main__":
	Register ().load ()

