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

import os
from os import path
import signal
import glob
import ibus

class Engine(ibus.Object):
    def __init__(self, name, lang = "other", icon = "", author = "", credits = "", _exec = "", pid = 0):
        super(Engine, self).__init__()
        self._name = name
        self._lang = lang
        self._icon = icon
        self._author = author
        self._credits = credits
        self._exec = _exec
        self._pid = pid

    def start(self):
        if self._pid != 0:
            return
        pid = os.fork()
        if pid > 0: # parent
            self._pid = pid
        elif pid == 0: # child
            os.setpgrp()
            args = self._exec.split()
            os.execv(args[0], args)
            sys.exit(1)

    def stop(self, force = False):
        if self._pid == 0:
            return
        try:
            if force:
                os.kill(-self._pid, signal.SIGKILL)
            else:
                os.kill(-self._pid, signal.SIGTERM)
        except:
            pass

    def engine_exit(self, pid):
        if self._pid == pid:
            self._pid = 0
            return True
        return False

    def __eq__(self, o):
        # We don't test icon author & credits
        return self._name == o._name and \
            self._lang == o._lang and \
            self._exec == o._exec

    def __str__(self):
        return "Engine('%s', '%s', '%s', '%s', '%s', '%s', %d" % (self._name, self._lang, \
            self._icon, self._author, \
            self._credits, self._exec, \
            self._pid)

class Register(ibus.Object):
    def __init__(self):
        super(Register, self).__init__()
        self._engines = dict()
        self._load()
        signal.signal(signal.SIGCHLD, self._sigchld_cb)

    def start_engine(self, lang, name):
        key = (lang, name)
        if key not in self._engines:
            raise ibus.IBusException("Can not find engine(%s, %s)" % (lang, name))

        engine = self._engines[(lang, name)]
        engine.start()

    def stop_engine(self, lang, name):
        key = (lang, name)
        if key not in self._engines:
            raise ibus.IBusException("Can not find engine(%s, %s)" % (lang, name))

        engine = self._engines[(lang, name)]
        engine.stop()

    def restart_engine(self, lang, name):
        key = (lang, name)
        if key not in self._engines:
            raise ibus.IBusException("Can not find engine (%s, %s)" % (lang, name))

        engine = self._engines[(lang, name)]
        engine.stop()
        engine.start()

    def list_engines(self):
        engines = []
        for key, e in self._engines.items():
            engines.append((e._name, e._lang, e._icon, e._author, e._credits, e._exec, e._pid != 0))
        return engines

    def _sigchld_cb(self, sig, f):
        pid, state = os.wait()
        for key, engine in self._engines.items():
            if engine.engine_exit(pid):
                break

    def _load(self):
        _file = path.abspath(__file__)
        _dir = path.dirname(_file) + "./../engine"
        _dir = path.abspath(_dir)
        _dir = "/usr/share/ibus/engine"
        for _file in glob.glob(_dir + "/*.engine"):
            engine = self._load_engine(_file)
            if (engine._lang, engine._name) in self._engines:
                old_engine = self._engines[(engine._lang, engine._name)]
                if old_engine == engine:
                    engine._pid = old_engine._pid
                    self._engines[(engine._lang, engine._name)] = engine
                else:
                    self._engines[(engine._lang, engine._name + " (old)")] = old_engine
                    self._engines[(engine._lang, engine._name)] = engine
            else:
                self._engines[(engine._lang, engine._name)] = engine



    def _load_engine(self, _file):
        f = file(_file)
        name = None
        lang = "other"
        icon = ""
        author = ""
        credits = ""
        _exec = None
        line = 0
        for l in f:
            line += 1
            l = l.strip()
            if l.startswith("#"):
                continue
            n, v = l.split("=")
            if n == "Name":
                name = v
            elif n == "Lang":
                lang = v
            elif n == "Icon":
                icon = v
            elif n == "Author":
                author = v
            elif n == "Credits":
                credits = v
            elif n == "Exec":
                _exec = v
            else:
                raise Exception("%s:%d\nUnknown value name = %s" % (_file, line, n))

        if name == None:
            raise Exception("%s: no name" % _file)
        if _exec == None:
            raise Exception("%s: no exec" % _file)

        return Engine(name, lang, icon, author, credits, _exec)

if __name__ == "__main__":
    import time
    reg = Register()
    reg.start_engine("zh", "py")
    time.sleep(3)
    reg.stop_engine("zh", "py")

