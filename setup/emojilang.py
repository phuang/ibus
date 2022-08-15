# vim:set et sts=4 sw=4:
# -*- coding: utf-8 -*-
#
# ibus - The Input Bus
#
# Copyright (c) 2017-2019 Takao Fujiwara <takao.fujiwara1@gmail.com>
# Copyright (c) 2017 Red Hat, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.

# for python2
from __future__ import print_function

__all__ = (
    "EmojiLangButton",
);

from gi.repository import Gtk
from gi.repository import GLib
from gi.repository import GObject
from gi.repository import IBus

import functools
import gettext
import i18n
import locale
import os

from icon import load_icon
from i18n import _, N_

ROW_TRAVEL_DIRECTION_NONE,      \
ROW_TRAVEL_DIRECTION_FORWARD,   \
ROW_TRAVEL_DIRECTION_BACKWARD = list(range(3))

class LanguageString:
    def __init__(self, id, trans = ""):
        self.id = id
        self.trans = trans

class EmojiLangChooser(Gtk.Dialog):
    __gtype_name__ = 'EmojiLangChooser'
    __initial_languages = [ IBus.get_language_name('en_US'),
                            IBus.get_language_name('en_GB'),
                            IBus.get_language_name('de_DE'),
                            IBus.get_language_name('fr_FR'),
                            IBus.get_language_name('es_ES'),
                            IBus.get_language_name('zh_CN'),
                            IBus.get_language_name('ja_JP'),
                            IBus.get_language_name('ru_RU'),
                            IBus.get_language_name('ar_EG') ]


    def __init__(self, id = None, transient_for = None):
        super(EmojiLangChooser, self).__init__(
                title = _("Select a language"),
                transient_for = transient_for,
                resizable = True)
        buttons = (_("_Cancel"), Gtk.ResponseType.CANCEL,
                   _("_OK"), Gtk.ResponseType.APPLY)
        self.add_buttons(*buttons)

        if id == None:
            id = 'en'
        self.__id = id
        self.__engines_for_lang = {}
        self.__untrans_for_lang = {}
        self.__langs = {}
        self.__lang_list = []

        self.__scrolled = Gtk.ScrolledWindow(
                hscrollbar_policy = Gtk.PolicyType.NEVER,
                vscrollbar_policy = Gtk.PolicyType.NEVER,
                shadow_type = Gtk.ShadowType.IN,
                margin_start = 6,
                margin_end = 6,
                margin_top = 6,
                margin_bottom = 6)
        self.vbox.add(self.__scrolled)
        viewport = Gtk.Viewport()
        self.__scrolled.add(viewport)
        self.__list = Gtk.ListBox(vexpand = True,
                                  halign = Gtk.Align.FILL,
                                  valign = Gtk.Align.FILL)
        viewport.add(self.__list)

        self.__adjustment = self.__scrolled.get_vadjustment()
        self.__list.set_adjustment(self.__adjustment)
        self.__list.set_filter_func(self.__list_filter, None)
        self.__list.connect('row-activated', self.__row_activated)

        self.__showing_extra = False
        self.__more_row = self.__more_row_new()
        self.__load_lang_list()
        self.__show_lang_rows()
        self.show_all()


    def __load_lang_list(self):
        dictdir = os.path.dirname(__file__) + '/../dicts'
        for filename in os.listdir(dictdir):
            suffix = '.dict'
            if not filename.endswith(suffix):
                continue
            lang_id = filename[0:len(filename) - len(suffix)]
            prefix = 'emoji-'
            if not lang_id.startswith(prefix):
                continue
            lang_id = lang_id[len(prefix):]
            lang = LanguageString(lang_id, IBus.get_language_name(lang_id))
            self.__lang_list.append(lang)
        if len(self.__lang_list) == 0:
            print("Not found dicts in %s" % dictdir, file=sys.stderr)
            lang = LanguageString('en', IBus.get_language_name('en'))
            self.__lang_list.append(lang)
            return

        def cmp_lang(a, b):
            label_a = a.trans + a.id
            label_b = b.trans + b.id
            return (label_a > label_b) - (label_a < label_b)

        self.__lang_list.sort(key = functools.cmp_to_key(cmp_lang))

        loc = locale.getlocale()[0]
        # None on C locale
        if loc == None or loc == 'C':
            loc = 'en_US'
        index = 0
        for lang in self.__lang_list:
            # move current language to the first place
            if lang.trans == IBus.get_language_name(loc):
                self.__lang_list.remove(lang)
                self.__lang_list.insert(index, lang)
                index += 1

        for lang in self.__lang_list:
            # move English to the second place
            if lang.trans == IBus.get_language_name('en'):
                self.__lang_list.remove(lang)
                self.__lang_list.insert(index, lang)
                index += 1


    def __list_filter(self, row, data):
        if row.id == self.__id:
            self.__list.select_row(row)
        if row == self.__more_row:
            return not self.__showing_extra
        if not self.__showing_extra and row.is_extra:
            return False
        return True


    def __row_activated(self, box, row):
        if row == self.__more_row:
            self.__show_more()
            return
        self.__id = row.id


    def __padded_label_new(self, text, icon, alignment, direction):
        hbox = Gtk.Box(orientation = Gtk.Orientation.HORIZONTAL)

        if direction == ROW_TRAVEL_DIRECTION_BACKWARD:
            rtl = (Gtk.Widget.get_default_direction() == \
                   Gtk.TextDirection.RTL)
            if rtl:
                arrow = Gtk.Image.new_from_icon_name(
                    'go-previous-rtl-symbolic', Gtk.IconSize.MENU)
            else:
                arrow = Gtk.Image.new_from_icon_name(
                    'go-previous-symbolic', Gtk.IconSize.MENU)
            hbox.pack_start(arrow, False, True, 0)

        if icon != None:
            pixbuf = load_icon(icon, Gtk.IconSize.LARGE_TOOLBAR)
            image = Gtk.Image(pixbuf = pixbuf)
            hbox.pack_start(image, False, True, 0)

        label = Gtk.Label(label = text)
        label.set_halign(alignment)
        label.set_valign(Gtk.Align.CENTER)
        label.set_margin_start(20)
        label.set_margin_end(20)
        label.set_margin_top(6)
        label.set_margin_bottom(6)
        hbox.pack_start(label, True, True, 0)
        return hbox


    def __list_box_row_new(self, lang):
        row = Gtk.ListBoxRow()
        row.trans = lang.trans
        row.id = lang.id
        row.is_extra = False
        return row


    def __lang_row_new(self, lang, prev_lang):
        row = self.__list_box_row_new(lang)
        label = lang.trans
        if lang.id == self.__id:
            row.is_extra = False
        elif prev_lang != None and label == prev_lang.trans:
            label = "%s (%s)" % (lang.trans, lang.id)
            row.is_extra = True
        elif not self.__showing_extra and \
           lang.trans not in self.__initial_languages:
            row.is_extra = True
        widget = self.__padded_label_new(label,
                                         None,
                                         Gtk.Align.CENTER,
                                         ROW_TRAVEL_DIRECTION_NONE)
        row.add(widget)
        return row


    def __more_row_new(self):
        row = Gtk.ListBoxRow()
        row.id = None
        hbox = Gtk.Box(orientation = Gtk.Orientation.HORIZONTAL)
        row.add(hbox)
        row.set_tooltip_text(_("More…"))
        arrow = Gtk.Image.new_from_icon_name('view-more-symbolic',
                                             Gtk.IconSize.MENU)
        arrow.set_margin_start(20)
        arrow.set_margin_end(20)
        arrow.set_margin_top(6)
        arrow.set_margin_bottom(6)
        arrow.set_halign(Gtk.Align.CENTER)
        arrow.set_valign(Gtk.Align.CENTER)
        hbox.pack_start(arrow, True, True, 0)
        return row


    def __set_fixed_size(self):
        if self.__scrolled.get_policy()[0] == Gtk.PolicyType.AUTOMATIC:
            return
        (width, height) = self.get_size()
        self.set_size_request(width, height)
        self.__scrolled.set_policy(Gtk.PolicyType.AUTOMATIC,
                                   Gtk.PolicyType.AUTOMATIC)


    def __remove_all_children(self):
        for l in self.__list.get_children():
            self.__list.remove(l)


    def __show_lang_rows(self):
        self.__remove_all_children()
        prev_lang = None
        for lang in self.__lang_list:
            row = self.__lang_row_new(lang, prev_lang)
            self.__list.add(row)
            prev_lang = lang
        self.__list.add(self.__more_row)
        self.__list.show_all()
        self.__adjustment.set_value(self.__adjustment.get_lower())
        self.__list.invalidate_filter()
        self.__list.set_selection_mode(Gtk.SelectionMode.SINGLE)


    def __show_more(self):
        self.__set_fixed_size()
        self.__showing_extra = True
        self.__list.invalidate_filter()


    def get_selected_lang(self):
        return self.__id


class EmojiLangButton(Gtk.Button):
    __gtype_name__ = 'EmojiLangButton'
    __gproperties__ = {
        'lang' : (
            str,
            'lang',
            'lang for emojo-*.dict',
            'en',
            GObject.ParamFlags.READABLE | GObject.ParamFlags.WRITABLE)
    }


    def __init__(self):
        super(EmojiLangButton, self).__init__()
        self.__lang = ''


    def do_get_property(self, prop):
        if prop.name == 'lang':
            return self.__lang
        else:
            raise AttributeError('unknown property %s' % prop.name)


    def do_set_property(self, prop, value):
        if prop.name == 'lang':
            self.set_lang(value)
        else:
            raise AttributeError('unknown property %s' % prop.name)


    def do_clicked(self):
        dialog = EmojiLangChooser(id = self.__lang,
                                  transient_for = self.get_toplevel())
        id = dialog.run()
        if id != Gtk.ResponseType.APPLY:
            dialog.destroy()
            return
        self.set_lang(dialog.get_selected_lang())
        dialog.destroy()


    def set_lang(self, lang):
        self.__lang = lang
        self.notify("lang")
        self.set_label(IBus.get_language_name(lang))


    def get_lang(self, lang):
        return self.__lang


GObject.type_register(EmojiLangButton)


if __name__ == "__main__":
        dialog = EmojiLangChooser()
        id = dialog.run()
        if id != Gtk.ResponseType.APPLY:
            dialog.destroy()
            import sys
            sys.exit(0)
        print("Selected language:", dialog.get_selected_lang())
