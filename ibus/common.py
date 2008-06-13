import os
import sys
import getpass

IBUS_ADDR = "unix:path=/tmp/ibus-%s/ibus-%s" % (getpass.getuser (), os.environ["DISPLAY"].replace (":", "-"))
# IBUS_ADDR  = "tcp:host=localhost,port=7799"

IBUS_IFACE = "org.freedesktop.IBus"
IBUS_PATH  = "/org/freedesktop/IBus"
IBUS_NAME  = "org.freedesktop.IBus"

IBUS_PANEL_IFACE = "org.freedesktop.IBus.Panel"

IBUS_ENGINE_FACTORY_IFACE = "org.freedesktop.IBus.EngineFactory"
IBUS_ENGINE_IFACE = "org.freedesktop.IBus.Engine"

def default_reply_handler ( *args):
	pass

def default_error_handler (e):
	print >> sys.stderr, e
	sys.exit (1)

DEFAULT_ASYNC_HANDLERS = {
	"reply_handler" : default_reply_handler,
	"error_handler" : default_error_handler
}
