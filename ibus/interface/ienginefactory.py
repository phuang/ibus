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

__all__ = ("IEngineFactory", )

import dbus.service
from ibus.common import \
	IBUS_ENGINE_FACTORY_IFACE

class IEngineFactory (dbus.service.Object):
	# define method decorator.
	method = lambda **args: \
		dbus.service.method (dbus_interface = IBUS_ENGINE_FACTORY_IFACE, \
							**args)

	# define async method decorator.
	async_method = lambda **args: \
		dbus.service.method (dbus_interface = IBUS_ENGINE_FACTORY_IFACE, \
							async_callbacks = ("reply_cb", "error_cb"), \
							**args)
	# Return a array. [name, default_language, icon_path, authors, credits]
	@method (out_signature = "as")
	def GetInfo (self): pass

	# Factory should allocate all resources in this method
	@method ()
	def Initialize (self): pass

	# Factory should free all allocated resources in this method
	@method ()
	def Uninitialize (self): pass

	# Create an input context and return the id of the context.
	# If failed, it will return "" or None.
	@method (out_signature = "o")
	def CreateEngine (self): pass

