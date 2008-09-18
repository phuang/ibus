# vim:set et sts=4 sw=4:
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

__all__ = (
        "get_language_name",
    )

import locale
import xml.parsers.expat

_ = lambda a: locale.dgettext("ibus", a)
__languages_dict = {}
locale.setlocale(locale.LC_ALL, "")

def get_language_name(_locale):
    lang = _locale.split("_")[0]
    lang = lang.lower()
    if lang in __languages_dict:
        lang = __languages_dict[lang]
        lang = locale.dgettext("iso_639", lang)
    else:
        lang = _(u"Other")
        lang = locale.dgettext("ibus", lang)
    return lang

def __start_element(name, attrs):
    global __languages_dict
    try:
        iso_639_1_code = attrs["iso_639_1_code"].decode("utf-8")
        name = attrs["name"].decode("utf-8")
        __languages_dict[iso_639_1_code] = name
    except:
        pass

def __end_element(name):
    pass

def __char_data(data):
    pass

iso_639_xml = "/usr/share/xml/iso-codes/iso_639.xml"
p = xml.parsers.expat.ParserCreate()
p.StartElementHandler = __start_element
p.EndElementHandler = __end_element
p.CharacterDataHandler = __char_data
p.ParseFile(file(iso_639_xml))
