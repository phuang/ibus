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

def menu_position (menu, button):
	screen = button.get_screen ()
	monitor = screen.get_monitor_at_window (button.window)
	monitor_allocation = screen.get_monitor_geometry (monitor)

	x, y = button.window.get_origin ()
	x += button.allocation.x
	y += button.allocation.y

	menu_width, menu_height = menu.size_request ()

	if x + menu_width >= monitor_allocation.width:
		x -= menu_width - button.allocation.width
	elif x - menu_width <= 0:
		pass
	else:
		if x <= monitor_allocation.width * 3 / 4:
			pass
		else:
			x -= menu_width - button.allocation.width

	if y + button.allocation.height + menu_height >= monitor_allocation.height:
		y -= menu_height
	elif y - menu_height <= 0:
		y += button.allocation.height
	else:
		if y <= monitor_allocation.height * 3 / 4:
			y += button.allocation.height
		else:
			y -= menu_height

	return (x, y, False)

