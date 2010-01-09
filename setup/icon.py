# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2010 Red Hat, Inc.
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

__all__ = (
    "load_icon"
)

import gtk
from gtk import gdk
from os import path


icon_theme = gtk.icon_theme_get_default()
dir = path.dirname(__file__)
icondir = path.join(dir, "..", "icons")
icon_theme.prepend_search_path(icondir)

icon_cache = {}

def load_icon(icon, size):
    if (icon, size) in icon_cache:
        return icon_cache[(icon, size)]

    icon_size = gtk.icon_size_lookup(size)[0]
    pixbuf = None
    try:
        pixbuf = gdk.pixbuf_new_from_file(icon)
        w, h = pixbuf.get_width(), pixbuf.get_height()
        rate = max(w, h) / float(icon_size)
        w = int(w / rate)
        h = int(h / rate)
        pixbuf = pixbuf.scale_simple(w, h, gdk.INTERP_BILINEAR)
    except:
        pass
    if pixbuf == None:
        try:
            theme = gtk.icon_theme_get_default()
            pixbuf = theme.load_icon(icon, icon_size, 0)
        except:
            pass
    icon_cache[(icon, size)] = pixbuf
    return pixbuf
