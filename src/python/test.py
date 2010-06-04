import ibus
bus = ibus.Bus()
ibus.Engine.new("pinyin", "/aa", bus.get_connection())
for e in bus.list_engines():
    print e.name
