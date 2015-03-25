# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright(c) 2007-2015 Peng Huang <shawn.p.huang@gmail.com>
# Copyright(c) 2007-2015 Google, Inc.
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

LOCALEDIR = os.getenv("IBUS_LOCALEDIR")

def init_textdomain(domainname):
    if domainname == '':
        return
    # Python's locale module doesn't provide all methods on some
    # operating systems like FreeBSD
    try:
        locale.bindtextdomain(domainname, LOCALEDIR)
        locale.bind_textdomain_codeset(domainname, 'UTF-8')
    except AttributeError:
        pass
    gettext.bindtextdomain(domainname, LOCALEDIR)
    gettext.bind_textdomain_codeset(domainname, 'UTF-8')

def gettext_engine_longname(engine):
    name = engine.get_name()
    if (name.startswith('xkb:')):
        return gettext.dgettext('xkeyboard-config', engine.get_longname())
    textdomain = engine.get_textdomain()
    if textdomain == '':
        return engine.get_longname()
    return gettext.dgettext(textdomain, engine.get_longname())

def gettext_engine_description(engine):
    name = engine.get_name()
    if (name.startswith('xkb:')):
        return gettext.dgettext('xkeyboard-config', engine.get_description())
    textdomain = engine.get_textdomain()
    if textdomain == '':
        return engine.get_description()
    return gettext.dgettext(textdomain, engine.get_description())
