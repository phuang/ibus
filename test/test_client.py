#! /usr/bin/python
# Test program for client APIs.
import time
import os
import sys
import select
import glib
import termios
import tty
import ibus
from ibus import keysyms
from ibus import modifier

class DemoTerm:
	def __init__(self):
		self.__term_old = termios.tcgetattr(0)
		tty.setraw(0)

		self.__bus = ibus.Bus()
		self.__ic = self.__bus.create_input_context("DemoTerm")
		self.__bus.set_capabilities(self.__ic, 7)
		self.__bus.connect("commit-string", self.__commit_string_cb)
		self.__bus.connect("update-preedit", self.__update_preedit_cb)
		self.__bus.connect("show-preedit", self.__show_preedit_cb)
		self.__bus.connect("hide-preedit", self.__hide_preedit_cb)
		self.__bus.connect("update-lookup-table", self.__update_lookup_table_cb)
		self.__bus.connect("show-lookup-table", self.__show_lookup_table_cb)
		self.__bus.connect("hide-lookup-table", self.__hide_lookup_table_cb)
		glib.io_add_watch(0, glib.IO_IN, self.__stdin_cb)
		# self.__master_fd, self.__slave_fd = os.openpty()
		# self.__run_shell()
	
	def __stdin_cb(self, fd, condition):
		c = ord(os.read(0, 1))
		if c == 3:
			self.__loop.quit()
		try:
			if c == 22: # Ctrl + V => Ctrl + space
				retval = self.__bus.process_key_event(self.__ic,
					keysyms.space, True, modifier.CONTROL_MASK)
			elif c == 127: # BackSpace
				self.__bus.process_key_event(self.__ic,
					keysyms.BackSpace, True, 0)
				retval = True
			else:
				retval = self.__bus.process_key_event(self.__ic, c, True, 0)
		except:
			retval = False
		if retval == False:
			os.write(1, chr(c))
		return True

	def __commit_string_cb(self, bus, ic, text):
		print "commit: %s\r" % text

	def __update_preedit_cb(self, bus, ic, text, attrs, cursor_pos, visible):
		if visible:
			print "preedit: %s\r" % text
		else:
			print "preedit:\r"
	
	def __show_preedit_cb(self, bus, ic):
		print "preedit show\r"
	
	def __hide_preedit_cb(self, bus, ic):
		print "preedit hide\r"

	def __update_lookup_table_cb(self, bus, ic, lookup_table, visible):
		if visible:
		    candidates = lookup_table.get_canidates_in_current_page()
		    i = 1
		    line = u"lookup table:"
		    for c in candidates:
		    	line += " %d.%s" % (i, c[0])
		    	i += 1
		    print line, "\r"
		else:
			print "lookup table:\r"
	
	def __show_lookup_table_cb(self, bus, ic):
		print "lookup table show\r"
	
	def __hide_lookup_table_cb(self, bus, ic):
		print "lookup table hide\r"

	def __run_shell(self):
		pid = os.fork()
		if pid == 0: # child
			os.close(0)
			os.close(1)
			os.close(2)
			os.close(self.__master_fd)
			os.dup2(self.__slave_fd, 0)
			os.dup2(self.__slave_fd, 1)
			os.dup2(self.__slave_fd, 2)
			os.close(self.__slave_fd)
			os.execv('/bin/bash', ["bash"])
			os.exit(1)



	def run(self):
		self.__loop = glib.MainLoop()
		self.__loop.run()

	def close(self):
		termios.tcsetattr(0, termios.TCSAFLUSH, self.__term_old)

def main():
	term = DemoTerm()
	try:
		term.run()
	except:
		import traceback
		traceback.print_exc()
	term.close()

if __name__ == "__main__":
	main()
