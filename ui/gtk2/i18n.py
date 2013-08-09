# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright(c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright(c) 2007-2010 Google, Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
# USA

import locale
import gettext
import os

DOMAINNAME = "ibus10"

_ = lambda a: gettext.dgettext(DOMAINNAME, a)
N_ = lambda a: a

def init():
    localedir = os.getenv("IBUS_LOCALEDIR")
    # Python's locale module doesn't provide all methods on some
    # operating systems like FreeBSD
    try:
        # for non-standard localedir
        locale.bindtextdomain(DOMAINNAME, localedir)
        locale.bind_textdomain_codeset(DOMAINNAME, "UTF-8")
    except AttributeError:
        pass
    gettext.bindtextdomain(DOMAINNAME, localedir)
    gettext.bind_textdomain_codeset(DOMAINNAME, "UTF-8")
