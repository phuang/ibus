IBUS_ADDR  = "unix:abstract=/tmp/ibus"
# IBUS_ADDR  = "tcp:host=localhost,port=7799"

IBUS_IFACE = "org.freedesktop.IBus"
IBUS_PATH  = "/org/freedesktop/IBus"
IBUS_NAME  = "org.freedesktop.IBus"

IBUS_ENGINE_FACTORY_IFACE = "org.freedesktop.IBus.EngineFactory"
IBUS_ENGINE_IFACE = "org.freedesktop.IBus.Engine"

def default_reply_handler ( *args):
	pass

def default_error_handler (e):
	print e

DEFAULT_ASYNC_HANDLERS = {
	"reply_handler" : default_reply_handler,
	"error_handler" : default_error_handler
}
