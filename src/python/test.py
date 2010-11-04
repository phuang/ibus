import ibus
bus = ibus.Bus()
if not bus.is_connected():
    print "Can not connect to ibus-daemon"
else:
    for e in bus.list_engines():
        print e.get_name()
