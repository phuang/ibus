#!/usr/bin/env python
import gtk

def press_cb(window, e, label):
    s = "keycode: %d,  keysym: %s,  keyval:%d ." % (e.hardware_keycode - 8, gtk.gdk.keyval_name(e.keyval), e.keyval)
    label.set_text(s)

def main():
    window = gtk.Window()
    window.set_default_size(400, 100)
    window.set_position(gtk.WIN_POS_CENTER_ALWAYS)
    label = gtk.Label("Please make sure you are using evdev of xorg!\nPress some key.")
    window.add(label)

    window.connect("delete-event", gtk.main_quit)
    window.connect("key-press-event", press_cb, label)

    window.show_all()
    gtk.main()

if __name__ == "__main__":
    main()
