# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright(c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or(at your option) any later version.
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

import gtk
import gtk.gdk as gdk
import gobject
import pango
import ibus
from ibus.gtk import PangoAttrList

class HSeparator(gtk.HBox):
    def __init__ (self):
        gtk.HBox.__init__ (self)
        self.pack_start(gtk.HSeparator(), True, True, 4)

class VSeparator(gtk.VBox):
    def __init__ (self):
        gtk.VBox.__init__ (self)
        self.pack_start(gtk.VSeparator(), True, True, 4)

class CandidateArea(gtk.HBox):
    def __init__ (self, orientation):
        gtk.HBox.__init__ (self)
        self.__orientation = orientation
        self.__labels = []
        self.__create_ui()

    def __create_ui(self):
        if self.__orientation == gtk.ORIENTATION_VERTICAL:
            self.__vbox1 = gtk.VBox()
            self.__vbox1.set_homogeneous(True)
            self.__vbox2 = gtk.VBox()
            self.__vbox2.set_homogeneous(True)
            self.pack_start(self.__vbox1, False, False, 4)
            self.pack_start(VSeparator(), False, False, 0)
            self.pack_start(self.__vbox2, True, True, 4)

        for i in xrange(1, 11):
            label1 = gtk.Label("%d." % (i % 10))
            label1.set_alignment(0.0, 0.5)
            label1.set_no_show_all(True)

            label2 = gtk.Label()
            label2.set_alignment(0.0, 0.5)
            label2.set_no_show_all(True)

            if self.__orientation == gtk.ORIENTATION_VERTICAL:
                label1.set_property("xpad", 8)
                label2.set_property("xpad", 8)
                self.__vbox1.pack_start(label1, False, False, 2)
                self.__vbox2.pack_start(label2, False, False, 2)
            else:
                hbox = gtk.HBox()
                hbox.pack_start(label1, False, False, 1)
                hbox.pack_start(label2, False, False, 1)
                self.pack_start(hbox, False, False, 4)

            self.__labels.append((label1, label2))

        self.__labels[0][0].show()
        self.__labels[0][1].show()

    def set_candidates(self, candidates, focus_candidate = 0):
        assert len(candidates) <= len(self.__labels)
        i = 0
        for text, attrs in candidates:
            if i == focus_candidate:
                if attrs == None:
                    attrs = pango.AttrList()
                color = self.__labels[i][1].style.base[gtk.STATE_SELECTED]
                end_index = len(text.encode("utf8"))
                attr = pango.AttrBackground(color.red, color.green, color.blue, 0, end_index)
                attrs.change(attr)
                color = self.__labels[i][1].style.text[gtk.STATE_SELECTED]
                attr = pango.AttrForeground(color.red, color.green, color.blue, 0, end_index)
                attrs.insert(attr)

            self.__labels[i][1].set_text(text)
            self.__labels[i][1].set_property("attributes", attrs)
            self.__labels[i][0].show()
            self.__labels[i][1].show()

            i += 1

        for label1, label2 in self.__labels[max(1, len(candidates)):]:
            label1.hide()
            label2.hide()

        if len(candidates) == 0:
            self.__labels[0][0].set_text("")
            self.__labels[0][1].set_text("")
        else:
            self.__labels[0][0].set_text("1.")

class CandidatePanel(gtk.VBox):
    __gproperties__ = {
        'orientation' : (gtk.Orientation,        # type
        'orientation of candidates',            # nick name
        'the orientation of candidates list',    # description
        0,
        gobject.PARAM_READWRITE)                # flags
        }

    __gsignals__ = {
        "cursor-up" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
        "cursor-down" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
    }

    def __init__ (self):
        gtk.VBox.__init__ (self)
        self.__tooltips = gtk.Tooltips()

        self.__toplevel = gtk.Window(gtk.WINDOW_POPUP)
        self.__viewport = gtk.Viewport()
        self.__viewport.set_shadow_type(gtk.SHADOW_IN)
        self.__toplevel.add(self.__viewport)
        self.__viewport.add(self)
        self.__toplevel.add_events(
            gdk.BUTTON_PRESS_MASK | \
            gdk.BUTTON_RELEASE_MASK | \
            gdk.BUTTON1_MOTION_MASK)
        self.__begin_move = False
        # self.__toplevel.connect("button-press-event", self.__button_press_event_cb)
        # self.__toplevel.connect("button-release-event", self.__button_release_event_cb)
        # self.__toplevel.connect("motion-notify-event", self.__motion_notify_event_cb)

        self.__orientation = gtk.ORIENTATION_HORIZONTAL
        self.__orientation = gtk.ORIENTATION_VERTICAL
        self.__preedit_visible = False
        self.__aux_string_visible = False
        self.__lookup_table_visible = False
        self.__preedit_string = ""
        self.__preedit_attrs = pango.AttrList()
        self.__aux_string = ""
        self.__aux_attrs = pango.AttrList()
        self.__lookup_table = None

        self.__cursor_location = (0, 0)

        self.__recreate_ui()

    def __recreate_ui(self):
        for w in self:
            self.remove(w)
            w.destroy()
        # create preedit label
        self.__preedit_label = gtk.Label(self.__preedit_string)
        self.__preedit_label.set_attributes(self.__preedit_attrs)
        self.__preedit_label.set_alignment(0.0, 0.5)
        self.__preedit_label.set_padding(8, 0)
        self.__preedit_label.set_no_show_all(True)
        if self.__preedit_visible:
            self.__preedit_label.show()

        # create aux label
        self.__aux_label = gtk.Label(self.__aux_string)
        self.__aux_label.set_attributes(self.__aux_attrs)
        self.__aux_label.set_alignment(0.0, 0.5)
        self.__aux_label.set_padding(8, 0)
        self.__tooltips.set_tip(self.__aux_label, "Aux string")
        self.__aux_label.set_no_show_all(True)
        if self.__aux_string_visible:
            self.__aux_label.show()

        # create candidates area
        self.__candidate_area = CandidateArea(self.__orientation)
        self.__candidate_area.set_no_show_all(True)
        self.update_lookup_table(self.__lookup_table, self.__lookup_table_visible)

        # create state label
        self.__state_label = gtk.Label()
        self.__state_label.set_size_request(20, -1)

        # create buttons
        self.__prev_button = gtk.Button()
        self.__prev_button.connect("clicked", lambda x: self.emit("cursor-up"))
        self.__prev_button.set_relief(gtk.RELIEF_NONE)
        self.__tooltips.set_tip(self.__prev_button, "Previous candidate")

        self.__next_button = gtk.Button()
        self.__next_button.connect("clicked", lambda x: self.emit("cursor-down"))
        self.__next_button.set_relief(gtk.RELIEF_NONE)
        self.__tooltips.set_tip(self.__next_button, "Next candidate")

        self.__pack_all_widgets()

    def __pack_all_widgets(self):
        if self.__orientation == gtk.ORIENTATION_VERTICAL:
            # package all widgets in vertical mode
            image = gtk.Image()
            image.set_from_stock(gtk.STOCK_GO_UP, gtk.ICON_SIZE_MENU)
            self.__prev_button.set_image(image)

            image = gtk.Image()
            image.set_from_stock(gtk.STOCK_GO_DOWN, gtk.ICON_SIZE_MENU)
            self.__next_button.set_image(image)
            vbox = gtk.VBox()
            vbox.pack_start(self.__preedit_label, False, False, 0)
            vbox.pack_start(self.__aux_label, False, False, 0)
            self.pack_start(vbox, False, False, 5)
            self.pack_start(HSeparator(), False, False)
            self.pack_start(self.__candidate_area, False, False, 2)
            self.pack_start(HSeparator(), False, False)
            hbox= gtk.HBox()
            hbox.pack_start(self.__state_label, True, True)
            hbox.pack_start(VSeparator(), False, False)
            hbox.pack_start(self.__prev_button, False, False, 2)
            hbox.pack_start(self.__next_button, False, False, 2)
            self.pack_start(hbox, False, False)
        else:
            # package all widgets in HORIZONTAL mode
            image = gtk.Image()
            image.set_from_stock(gtk.STOCK_GO_BACK, gtk.ICON_SIZE_MENU)
            self.__prev_button.set_image(image)

            image = gtk.Image()
            image.set_from_stock(gtk.STOCK_GO_FORWARD, gtk.ICON_SIZE_MENU)
            self.__next_button.set_image(image)

            vbox = gtk.VBox()
            vbox.pack_start(self.__preedit_label, False, False, 0)
            vbox.pack_start(self.__aux_label, False, False, 0)
            self.pack_start(vbox, False, False, 5)
            self.pack_start(HSeparator(), False, False)
            hbox= gtk.HBox()
            hbox.pack_start(self.__candidate_area, True, True, 2)
            hbox.pack_start(VSeparator(), False, False)
            hbox.pack_start(self.__prev_button, False, False, 2)
            hbox.pack_start(self.__next_button, False, False, 2)
            self.pack_start(hbox, False, False)

        # self.hide_all()
        # self.show_all()

    def show_preedit(self):
        self.__preedit_visible = True
        self.__preedit_label.show()
        self.__check_show_states()

    def hide_preedit(self):
        self.__preedit_visible = False
        self.__preedit_label.hide()
        self.__check_show_states()

    def update_preedit(self, text, attrs, cursor_pos, visible):
        if attrs:
            attrs = PangoAttrList(attrs, text)
        if visible:
            self.show_preedit()
        else:
            self.hide_preedit()
        self.__preedit_string = text
        self.__preedit_label.set_text(text)
        if attrs == None:
            attrs = pango.AttrList()
        self.__preedit_attrs = attrs
        self.__preedit_label.set_attributes(attrs)

    def show_aux_string(self):
        self.__aux_string_visible = True
        self.__aux_label.show()
        self.__check_show_states()

    def hide_aux_string(self):
        self.__aux_string_visible = False
        self.__aux_label.hide()
        self.__check_show_states()

    def update_aux_string(self, text, attrs, show):
        attrs = PangoAttrList(attrs, text)

        if show:
            self.show_aux_string()
        else:
            self.hide_aux_string()

        self.__aux_string = text
        self.__aux_label.set_text(text)
        if attrs == None:
            attrs = pango.AttrList()
        self.__aux_attrs = attrs
        self.__aux_label.set_attributes(attrs)

    def __refresh_candidates(self):
        candidates = self.__lookup_table.get_canidates_in_current_page()
        candidates = map(lambda x: (x[0], PangoAttrList(x[1], x[0]) if x[1] else None), candidates)
        self.__candidate_area.set_candidates(candidates, self.__lookup_table.get_cursor_pos_in_current_page())

    def update_lookup_table(self, lookup_table, visible):
        if lookup_table == None:
            lookup_table = ibus.LookupTable()

        if visible:
            self.show_lookup_table()
        else:
            self.hide_lookup_table()

        self.__lookup_table = lookup_table
        self.__refresh_candidates()

    def show_lookup_table(self):
        self.__lookup_table_visible = True
        self.__candidate_area.set_no_show_all(False)
        self.__candidate_area.show_all()
        self.__check_show_states()

    def hide_lookup_table(self):
        self.__lookup_table_visible = False
        self.__candidate_area.hide_all()
        self.__candidate_area.set_no_show_all(True)
        self.__check_show_states()

    def page_up_lookup_table(self):
        self.__lookup_table.page_up()
        self.__refresh_candidates()

    def page_down_lookup_table(self):
        self.__lookup_table.page_down()
        self.__refresh_candidates()

    def cursor_up_lookup_table(self):
        self.__lookup_table.cursor_up()
        self.__refresh_candidates()

    def cursor_down_lookup_table(self):
        self.__lookup_table.cursor_down()
        self.__refresh_candidates()

    def set_cursor_location(self, x, y):
        self.__cursor_location = (x, y)
        self.__check_position()

    def __check_show_states(self):
        if self.__preedit_visible or \
            self.__aux_string_visible or \
            self.__lookup_table_visible:
            self.show_all()
            self.emit("show")
        else:
            self.hide_all()
            self.emit("hide")

    def reset(self):
        self.update_preedit("", None, 0, False)
        self.update_aux_string("", None, False)
        self.update_lookup_table(None, False)
        self.hide()

    def set_orientation(self, orientation):
        if self.__orientation == orientation:
            return
        self.__orientation = orientation
        self.__recreate_ui()
        if self.__toplevel.flags() & gtk.VISIBLE:
            self.show_all()

    def get_orientation(self):
        return self.__orientation

    def do_set_property(self, property, value):
        if property == 'orientation':
            self.set_orientation(value)
        else:
            return gtk.DrawingArea.do_set_property(property, value)

    def do_get_property(self, property):
        if property == 'orientation':
            return self.__orientation
        else:
            return gtk.DrawingArea.do_get_property(property)

    # def do_expose_event(self, event):
    #     self.style.paint_box(self.window,
    #                 gtk.STATE_NORMAL,
    #                 gtk.SHADOW_IN,
    #                 event.area,
    #                 self,
    #                 "panel",
    #                 self.allocation.x, self.allocation.y,
    #                 self.allocation.width, self.allocation.height)

    #     gtk.VBox.do_expose_event(self, event)

    def do_size_request(self, requisition):
        gtk.VBox.do_size_request(self, requisition)
        self.__toplevel.resize(1, 1)

        self.__check_position()

    def __check_position(self):
        bx = self.__cursor_location[0] + self.__toplevel.allocation.width
        by = self.__cursor_location[1] + self.__toplevel.allocation.height

        root_window = gdk.get_default_root_window()
        sx, sy = root_window.get_size()

        if bx > sx:
            x = sx - self.__toplevel.allocation.width
        else:
            x = self.__cursor_location[0]

        if by > sy:
            y = sy - self.__toplevel.allocation.height
        else:
            y = self.__cursor_location[1]

        self.move(x, y)

    def __button_press_event_cb(self, widget, event):
        if event.button == 1:
            self.__begin_move = True
            self.__press_pos = event.x_root, event.y_root
            self.__toplevel.window.set_cursor(gdk.Cursor(gdk.FLEUR))
            return True

        if event.button == 3:
            if self.get_orientation() == gtk.ORIENTATION_HORIZONTAL:
                self.set_orientation(gtk.ORIENTATION_VERTICAL)
            else:

                self.set_orientation(gtk.ORIENTATION_HORIZONTAL)
            return True
        return False

    def __button_release_event_cb(self, widget, event):
        if event.button == 1:
            del self.__press_pos
            self.__begin_move = False
            self.__toplevel.window.set_cursor(gdk.Cursor(gdk.LEFT_PTR))
            return True
        return False

    def __motion_notify_event_cb(self, widget, event):
        if self.__begin_move != True:
            return False
        x, y = self.__toplevel.get_position()
        x  = int(x + event.x_root - self.__press_pos[0])
        y  = int(y + event.y_root - self.__press_pos[1])
        self.move(x, y)
        self.__press_pos = event.x_root, event.y_root
        return True

    def show_all(self):
        gtk.VBox.show_all(self)
        self.__toplevel.show_all()

    def hide_all(self):
        gtk.VBox.hide_all(self)
        self.__toplevel.hide_all()

    def move(self, x, y):
        self.__toplevel.move(x, y)

gobject.type_register(CandidatePanel, "IBusCandidate")

