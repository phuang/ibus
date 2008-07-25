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
        self.name = name
        self.lang = lang
        self.icon = icon
        self.author = author
        self.credits = credits
        self._exec = _exec
        self.pid = pid

    def start(self):
        if self.pid != 0:
            return
        pid = os.fork()
        if pid > 0: # parent
            self.pid = pid
        elif pid == 0: # child
            os.setpgrp()
            args = self._exec.split()
            os.execv(args[0], args)
            sys.exit(1)

    def stop(self, force = False):
        if self.pid == 0:
            return
        try:
            if force:
                os.kill(-self.pid, signal.SIGKILL)
            else:
                os.kill(-self.pid, signal.SIGTERM)
        except:
            pass

    def engine_exit(self, pid):
        if self.pid == pid:
            self.pid = 0
            return True
        return False

    def __eq__(self, o):
        # We don't test icon author & credits
        return self.name == o.name and \
            self.lang == o.lang and \
            self._exec == o._exec

    def __str__(self):
        return "Engine('%s', '%s', '%s', '%s', '%s', '%s', %d" % (self.name, self.lang, \
            self.icon, self.author, \
            self.credits, self._exec, \
            self.pid)

class Register(ibus.Object):
    def __init__(self):
        super(Register, self).__init__()
        self.__engines = dict()
        self.__load()
        signal.signal(signal.SIGCHLD, self.__sigchld_cb)

    def start_engine(self, lang, name):
        key = (lang, name)
        if key not in self.__engines:
            raise ibus.IBusException("Can not find engine(%s, %s)" % (lang, name))

        engine = self.__engines[(lang, name)]
        engine.start()

    def stop_engine(self, lang, name):
        key = (lang, name)
        if key not in self.__engines:
            raise ibus.IBusException("Can not find engine(%s, %s)" % (lang, name))

        engine = self.__engines[(lang, name)]
        engine.stop()

    def restart_engine(self, lang, name):
        key = (lang, name)
        if key not in self.__engines:
            raise ibus.IBusException("Can not find engine (%s, %s)" % (lang, name))

        engine = self.__engines[(lang, name)]
        engine.stop()
        engine.start()

    def list_engines(self):
        engines = []
        for key, e in self.__engines.items():
            engines.append((e.name, e.lang, e.icon, e.author, e.credits, e._exec, e.pid != 0))
        return engines

    def __sigchld_cb(self, sig, f):
        pid, state = os.wait()
        for key, engine in self.__engines.items():
            if engine.engine_exit(pid):
                break

    def __load(self):
        _file = path.abspath(__file__)
        _dir = path.dirname(_file) + "./../engine"
        _dir = path.abspath(_dir)
        _dir = "/usr/share/ibus/engine"
        for _file in glob.glob(_dir + "/*.engine"):
            engine = self.__load_engine(_file)
            if (engine.lang, engine.name) in self.__engines:
                old_engine = self.__engines[(engine.lang, engine.name)]
                if old_engine == engine:
                    engine.pid = old_engine.pid
                    self.__engines[(engine.lang, engine.name)] = engine
                else:
                    self.__engines[(engine.lang, engine.name + " (old)")] = old_engine
                    self.__engines[(engine.lang, engine.name)] = engine
            else:
                self.__engines[(engine.lang, engine.name)] = engine



    def __load_engine(self, _file):
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

