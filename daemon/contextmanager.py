# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
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
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

import ibus
from inputcontext import InputContext

class ContextManager(ibus.Object):
    def __init__(self):
        super(ContextManager, self).__init__()
        self._contexts = {}

    def create_input_context(self, name, ibusconn):
        context = InputContext(name, ibusconn)
        self._contexts[context.get_id()] = context
        context.connect("destroy", self._context_destroy_cb)
        return context

    def release_input_context(self, ic, ibusconn):
        context = self._contexts[ic]
        context.destroy()

    def lookup_context(self, ic, ibusconn):
        return self._contexts[ic]

    def _context_destroy_cb(self, context):
        del self._contexts[context.get_id()]

