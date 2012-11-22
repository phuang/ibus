# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2012 Daiki Ueno <ueno@unixuser.org>
# Copyright (c) 2011 Peng Huang <shawn.p.huang@gmail.com>
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
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA  02110-1301  USA

import unittest
from gi.repository import GLib, IBus

class TestOverride(unittest.TestCase):
    def setUp(self):
        self.__bus = IBus.Bus()

    def test_attribute(self):
        # construction with keyword args
        attr = IBus.Attribute(type=IBus.AttrType.UNDERLINE,
                              value=IBus.AttrUnderline.SINGLE,
                              start_index=0,
                              end_index=10)

    def test_component(self):
        # construction with keyword args
        component = IBus.Component(name='foo', description='foo desc')
        self.assertEqual(component.props.name, 'foo')
        # construction with non-keyword args
        component = IBus.Component('bar', 'bar desc')
        self.assertEqual(component.props.name, 'bar')

    def test_config(self):
        if not self.__bus.is_connected():
            self.skipTest('bus not connected')

        config = self.__bus.get_config()
        if config is None:
            self.skipTest('config service not running')

        config.unset("test", "v1")

        # get_value with no default arg
        retval = config.get_value("test", "v1")
        self.assertEqual(retval, None)

        # get_value with default arg
        retval = config.get_value("test", "v1", GLib.Variant('i', 43))
        self.assertEqual(retval, GLib.Variant('i', 43))

        # set_value with non-null arg
        retval = config.set_value("test", "v1", GLib.Variant('i', 43))
        retval = config.get_value("test", "v1")
        self.assertEqual(retval, GLib.Variant('i', 43))

        # set_value with null arg (= unset)
        retval = config.set_value("test", "v1", None)
        self.assertEqual(retval, None)

    def test_engine_desc(self):
        # construction with keyword args
        desc = IBus.EngineDesc(name='foo')
        self.assertEqual(desc.props.name, 'foo')
        # construction with non-keyword args
        desc = IBus.EngineDesc('bar')
        self.assertEqual(desc.props.name, 'bar')

    def test_factory(self):
        if not self.__bus.is_connected():
            self.skipTest('bus not connected')

        # construction with keyword args
        factory = IBus.Factory(connection=self.__bus.get_connection(),
                               object_path=IBus.PATH_FACTORY)
        self.assertEqual(factory.props.object_path, IBus.PATH_FACTORY)
        # construction with non-keyword args
        factory = IBus.Factory(self.__bus)
        self.assertEqual(factory.props.object_path, IBus.PATH_FACTORY)

    def test_keymap(self):
        # construction with non-keyword args
        keymap = IBus.Keymap('us')
        self.assertEqual(keymap.name, 'us')

    def test_lookup_table(self):
        # construction with keyword args
        table = IBus.LookupTable(page_size=6)
        self.assertEqual(table.page_size, 6)
        # construction with non-keyword args
        table = IBus.LookupTable()
        self.assertEqual(table.page_size, 5)
        table = IBus.LookupTable(7)
        self.assertEqual(table.page_size, 7)

    def test_property(self):
        # construction with keyword args
        prop = IBus.Property(key='foo')
        self.assertEqual(prop.props.key, 'foo')
        # construction with non-keyword args
        prop = IBus.Property('bar')
        self.assertEqual(prop.props.key, 'bar')

    def test_text(self):
        # construction with non-keyword args
        text = IBus.Text('foo')
        self.assertEqual(text.text, 'foo')
        text = IBus.Text.new_from_string('bar')
        self.assertEqual(text.text, 'bar')

if __name__ == '__main__':
    unittest.main()
