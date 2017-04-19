/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright (c) 2017 Peng Wu <alexepico@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

string emoji_font = null;
string annotation_lang = null;

public class EmojiApplication : Application {
    private IBusEmojier? m_emojier;
    GLib.Settings m_settings_emoji =
            new GLib.Settings("org.freedesktop.ibus.panel.emoji");


    private EmojiApplication() {
        Object(application_id: "org.freedesktop.ibus.panel.emojier",
               flags: ApplicationFlags.HANDLES_COMMAND_LINE);
        set_inactivity_timeout(100000);
    }


    private void show_dialog(ApplicationCommandLine command_line) {
        m_emojier = new IBusEmojier();
        string emoji = m_emojier.run("");
        if (emoji == null) {
            m_emojier = null;
            command_line.print("%s\n", _("Canceled to choose an emoji."));
            return;
        }
        Gtk.Clipboard clipboard = Gtk.Clipboard.get(Gdk.SELECTION_CLIPBOARD);
        clipboard.set_text(emoji, -1);
        clipboard.store();
        m_emojier = null;
        command_line.print("%s\n", _("Copied an emoji to your clipboard."));
    }


    public void activate_dialog(ApplicationCommandLine command_line) {
        this.hold ();
        show_dialog(command_line);
        this.release ();
    }


    private int _command_line (ApplicationCommandLine command_line) {
        const OptionEntry[] options = {
            { "font", 0, 0, OptionArg.STRING, out emoji_font,
                /* TRANSLATORS: "FONT" should be capital and translatable.
                 * It's used for an argument command --font=FONT
                 */
                N_("\"FONT\" for emoji chracters on emoji dialog"),
                N_("FONT") },
            { "lang", 0, 0, OptionArg.STRING, out annotation_lang,
                /* TRANSLATORS: "LANG" should be capital and translatable.
                 * It's used for an argument command --lang=LANG
                 */
                N_("\"LANG\" for annotations on emoji dialog. E.g. \"en\""),
                N_("LANG") },
            { null }
        };

        var option = new OptionContext();
        option.add_main_entries(options, Config.GETTEXT_PACKAGE);

        // We have to make an extra copy of the array,
        // since .parse assumes that it can remove strings
        // from the array without freeing them.
        string[] args = command_line.get_arguments();
        string*[] _args = new string[args.length];
        for (int i = 0; i < args.length; i++) {
            _args[i] = args[i];
        }

        try {
            unowned string[] tmp = _args;
            option.parse(ref tmp);
        } catch (OptionError e) {
            stderr.printf("%s\n", e.message);
            return Posix.EXIT_FAILURE;
        }

        if (m_emojier != null && m_emojier.is_running()) {
            m_emojier.present_centralize();
            return Posix.EXIT_SUCCESS;
        }

        if (emoji_font == null)
            emoji_font = m_settings_emoji.get_string("font");
        if (annotation_lang == null)
            annotation_lang = m_settings_emoji.get_string("lang");

        IBusEmojier.set_annotation_lang(annotation_lang);
        IBusEmojier.set_emoji_font(emoji_font);

        activate_dialog(command_line);

        return Posix.EXIT_SUCCESS;
    }


    public override int command_line (ApplicationCommandLine command_line) {
        // keep the application running until we are done with this commandline
        this.hold();
        int result = _command_line(command_line);
        this.release();
        return result;
    }


    public static int main (string[] args) {
        GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE,
                                 Config.GLIB_LOCALE_DIR);
        GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE, "UTF-8");
        GLib.Intl.textdomain(Config.GETTEXT_PACKAGE);

        IBus.init();

        Gtk.init(ref args);

        EmojiApplication app = new EmojiApplication();
        int status = app.run(args);
        return status;
    }
}
