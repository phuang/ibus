#! /usr/bin/python
# Test program for client APIs.

import os
import sys
import glib
import termios
import tty
import locale
import curses
import ibus
from ibus import keysyms
from ibus import modifier

class DemoTerm:
	def __init__(self):
		self.__init_curses()
		self.__bus = ibus.Bus()
		self.__ic = self.__bus.create_input_context("DemoTerm")
		self.__bus.set_capabilities(self.__ic, 7)
		self.__bus.connect("commit-string", self.__commit_string_cb)

		self.__bus.connect("update-preedit", self.__update_preedit_cb)
		self.__bus.connect("show-preedit", self.__show_preedit_cb)
		self.__bus.connect("hide-preedit", self.__hide_preedit_cb)

		self.__bus.connect("update-aux-string", self.__update_aux_string_cb)
		self.__bus.connect("show-aux-string", self.__show_aux_string_cb)
		self.__bus.connect("hide-aux-string", self.__hide_aux_string_cb)

		self.__bus.connect("update-lookup-table", self.__update_lookup_table_cb)
		self.__bus.connect("show-lookup-table", self.__show_lookup_table_cb)
		self.__bus.connect("hide-lookup-table", self.__hide_lookup_table_cb)
		glib.io_add_watch(0, glib.IO_IN, self.__stdin_cb)
		# glib.timeout_add(500, self.__timeout_cb)

		# self.__master_fd, self.__slave_fd = os.openpty()
		# self.__run_shell()

		self.__is_invalidate = False
		self.__preedit = ""
		self.__preedit_visible = False
		self.__aux_string = ""
		self.__aux_string_visible = False
		self.__lookup_table = None
		self.__lookup_table_visible = False

		# self.__old_sigwinch_cb = signal.signal(signal.SIGWINCH, self.__sigwinch_cb)

	def __timeout_cb(self):
		self.__stdin_cb(0, 0)
		return True

	def __sigwinch_cb(self, a, b):
		self.__old_sigwinch_cb(a, b)
		self.__invalidate()

	def __init_curses(self):
		self.__screen = curses.initscr()
		curses.noecho()
		curses.raw()
		self.__screen.keypad(1)
		self.__screen.refresh()
		self.__screen.nodelay(1)
		self.__max_y, self.__max_x = self.__screen.getmaxyx()
		self.__state_pad = curses.newpad(2, self.__max_x)
		self.__state_pad.bkgd(' ', curses.A_REVERSE)
		self.__state_pad.addstr(0, 0, "Press Ctrl + v to enable or disable input method")
		self.__state_pad.refresh(0, 0, self.__max_y - 2, 0, self.__max_y, self.__max_x)

	def __fini_curses(self):
		curses.noraw()
		curses.echo()
		curses.endwin()

	def __stdin_cb(self, fd, condition):
		while self.__process_input():
			pass
		return True

	def __process_input(self):
		c = self.__screen.getch()
		if c < 0:
			return False

		if c == 3:
			self.__loop.quit()
		try:
			if c == 22: # Ctrl + V => Ctrl + space
				retval = self.__bus.process_key_event(self.__ic,
					keysyms.space, True, modifier.CONTROL_MASK)
			elif c == curses.KEY_BACKSPACE: # BackSpace
				self.__bus.process_key_event(self.__ic,
					keysyms.BackSpace, True, 0)
				retval = True
			elif c == curses.KEY_RESIZE:
				self.__invalidate()
				retval = True
			else:
				retval = self.__bus.process_key_event(self.__ic, c, True, 0)
		except:
			import backtrace
			backtrace.print_exc()
			retval = False
		if retval == False:
			self.__screen.addstr(unichr(c).encode("utf-8"))
			self.__screen.refresh()
		return True

	def __commit_string_cb(self, bus, ic, text):
		self.__screen.addstr(text)
		self.__screen.refresh()

	def __update_preedit_cb(self, bus, ic, text, attrs, cursor_pos, visible):
		self.__preedit = text
		self.__preedit_visible = visible
		self.__invalidate()

	def __show_preedit_cb(self, bus, ic):
		if self.__preedit_visible:
			return
		self.__preedit_visible = True
		self.__invalidate()

	def __hide_preedit_cb(self, bus, ic):
		if not self.__preedit_visible:
			return
		self.__preedit_visible = False
		self.__invalidate()

	def __update_aux_string_cb(self, bus, ic, text, attrs, visible):
		self.__aux_string = text
		self.__aux_string_visible = visible
		self.__invalidate()

	def __show_aux_string_cb(self, bus, ic):
		if self.__aux_string_visible:
			return
		self.__aux_string_visible = True
		self.__invalidate()

	def __hide_aux_string_cb(self, bus, ic):
		if not self.__aux_string_visible:
			return
		self.__aux_string_visible = False
		self.__invalidate()


	def __update_lookup_table_cb(self, bus, ic, lookup_table, visible):
		self.__lookup_table = lookup_table
		self.__lookup_table_visible = visible
		self.__invalidate()

	def __show_lookup_table_cb(self, bus, ic):
		if self.__lookup_table_visible:
			return
		self.__lookup_table_visible = True
		self.__invalidate()

	def __hide_lookup_table_cb(self, bus, ic):
		if not self.__lookup_table_visible:
			return
		self.__lookup_table_visible = False
		self.__invalidate()

	def __invalidate(self):
		if self.__is_invalidate:
			return
		self.__is_invalidate = True
		glib.idle_add(self.__update)

	def __update(self):
		if not self.__is_invalidate:
			return False
		self.__is_invalidate = False

		y, x = self.__screen.getmaxyx()
		if self.__max_x != x or self.__max_y != y:
			self.__max_x = x
			self.__max_y = y
			self.__state_pad = curses.newpad(2, self.__max_x)
			self.__state_pad.bkgd(' ', curses.A_REVERSE)
			self.__state_pad.addstr(0, 0, "Press Ctrl + v to enable or disable input method")
			self.__screen.clear()

		self.__state_pad.clear()

		# update preedit
		if self.__preedit_visible:
			self.__state_pad.addstr(0,0, self.__preedit, curses.A_REVERSE)

		# update aux string
		if self.__aux_string_visible:
			self.__state_pad.addstr("  ", curses.A_REVERSE)
			self.__state_pad.addstr(self.__aux_string, curses.A_REVERSE)
			self.__state_pad.addstr(1, 0, "", curses.A_REVERSE)

		# update lookup table
		if self.__lookup_table_visible:
			candidates = self.__lookup_table.get_canidates_in_current_page()
			i = 1
			for c in candidates:
				text = u"%d.%s " % (i, c[0])
				self.__state_pad.addstr(text.encode("utf-8"), curses.A_REVERSE)
				i += 1

		if self.__preedit_visible == False and self.__aux_string_visible == False and self.__lookup_table_visible == False:
			self.__state_pad.addstr(0, 0, "Press Ctrl + v to enable or disable input method")

		self.__state_pad.refresh(0, 0, self.__max_y - 2, 0, self.__max_y, self.__max_x)
		self.__screen.refresh()

		return False

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
		self.__fini_curses()
		pass

def main():
	locale.setlocale(locale.LC_ALL, "")
	old = termios.tcgetattr(0)
	term = DemoTerm()
	try:
		term.run()
	except:
		term.close()
		termios.tcsetattr(0, termios.TCSAFLUSH, old)
		import traceback
		traceback.print_exc()
	finally:
		term.close()
		termios.tcsetattr(0, termios.TCSAFLUSH, old)

if __name__ == "__main__":
	main()

