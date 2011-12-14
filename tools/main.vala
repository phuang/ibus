/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright(c) 2011 Peng Huang <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or(at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */
using GLib;

string opt1 = null;

int list_engine(string[] argv) throws Error {
    const OptionEntry[] options = {
        { "opt1", 0, 0, OptionArg.STRING, out opt1, "opt1 desc", "opt2 short desc" },
        { null }
    };

    var option = new OptionContext("command [OPTIONS]");
    option.add_main_entries(options, "ibus");
    option.parse(ref argv);

    foreach (var v in argv) {
        debug("v = %s", v);
    }

    return 0;
}

delegate int EntryFunc(string[] argv) throws Error;

struct CommandEntry {
    string name;
    EntryFunc entry;
}



public int main(string[] argv) {
    const CommandEntry commands[]  = {
        { "list-engine", list_engine }
    };

    if (argv.length >= 2) {
        string[] new_argv = argv[1:argv.length];
        foreach (var command in commands) {
            if (command.name == argv[1])
                return command.entry(new_argv);
        }
        warning("%s is unknown command!", argv[1]);
    }

    return -1;
}
