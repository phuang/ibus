/* vim:set noet ts=4: */
/*
 * ibus - The Input Bus
 *
 * Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
#include <config.h>
#include <QtDebug>
#include <QFile>
#include <QDBusConnection>
#include <QCoreApplication>
#include <QDBusMessage>
#include <QDBusArgument>

#include <pwd.h>

#include "ibus-client.h"
#include "ibus-input-context.h"

#ifdef Q_WS_X11
# include <QX11Info>
# include <X11/Xlib.h>
# include <X11/keysym.h>
# include <X11/Xutil.h>
#endif

#ifdef HAVE_X11_XKBLIB_H
#  define HAVE_XKB
#  include <X11/XKBlib.h>
#endif


#define IBUS_NAME	"org.freedesktop.IBus"
#define IBUS_PATH	"/org/freedesktop/IBus"
#define IBUS_INTERFACE	"org.freedesktop.IBus"


IBusClient::IBusClient ()
	: ibus (NULL), japan_groups (0)
{
	findYenBarKeys ();
	username = getlogin ();
	if (username.isEmpty ())
		username = getenv ("SUDO_USER");
	if (username.isEmpty ()) {
		QString uid = getenv ("USERHELPER_UID");
		if (!uid.isEmpty ()) {
			bool ok;
			uid_t id = uid.toInt (&ok);
			if (ok) {
				struct passwd *pw = getpwuid (id);
				if ( pw != NULL)
					username = pw->pw_name;
			}
		}
	}

	if (username.isEmpty ())
		username = getenv ("USERNAME");
	if (username.isEmpty ())
		username = getenv ("LOGNAME");
	if (username.isEmpty ())
		username = getenv ("USER");
	if (username.isEmpty ())
		username = getenv ("LNAME");

	session = getenv ("DISPLAY");
	if (session.indexOf (".") == -1) {
		session += ".0";
	}
	session.replace (":", "-");

	ibus_path = QString("/tmp/ibus-%1/ibus-%2").arg (username, session);
	ibus_addr = QString("unix:path=/tmp/ibus-%1/ibus-%2").arg (username, session);
	connectToBus ();

	QObject::connect (
		&watcher,
		SIGNAL(directoryChanged(const QString &)),
		this,
		SLOT(slotDirectoryChanged(const QString &)));

	QString ibus_dir;

	ibus_dir = QString ("/tmp/ibus-%1/").arg (username);
	watcher.addPath (ibus_dir);
}

IBusClient::~IBusClient ()
{
	if (ibus)
		delete ibus;
}

QString
IBusClient::createInputContextRemote ()
{
	QString ic;
	if (ibus) {
		QDBusMessage message = QDBusMessage::createMethodCall (
								IBUS_NAME,
								IBUS_PATH,
								IBUS_INTERFACE,
								"CreateInputContext");
		message << QCoreApplication::applicationName ();
		message = ibus->call (message);

		if (message.type () == QDBusMessage::ErrorMessage) {
			qWarning() << message.errorMessage ();
		}
		else if (message.type () == QDBusMessage::ReplyMessage) {
			ic = message.arguments () [0].toString ();
		}
	}
	return ic;
}

void
IBusClient::findYenBarKeys ()
{
#ifdef HAVE_XKB
	bool retval;
	Status status;
	XkbDescPtr desc;

	japan_groups = 0;
	japan_yen_bar_keys.clear ();

	desc = XkbGetMap (QX11Info::display (), 0, XkbUseCoreKbd);
    if (desc == NULL) {
        qWarning ("Can not allocate XkbDescRec!");
        return;
    }

    retval =
        Success == (status = XkbGetControls (QX11Info::display (),
                                XkbSlowKeysMask,
                                desc)) &&
        Success == (status = XkbGetNames (QX11Info::display (),
                                XkbGroupNamesMask | XkbIndicatorNamesMask,
                                desc)) &&
        Success == (status = XkbGetIndicatorMap (QX11Info::display (),
                                XkbAllIndicatorsMask,
                                desc));
    if (retval) {
        Atom *pa = desc->names->groups;
        int i;
        for (i = 0; i < desc->ctrls->num_groups; pa++, i++) {
            QString group_name;
            if (*pa == None)
                continue;
            group_name = XGetAtomName (QX11Info::display (), *pa);
            if (group_name == "Japan") {
                japan_groups |= (1 << i);
            }
        }
    }
    else {
        qWarning ("Can not get groups' names from Xkb");
    }

	int min_keycode, max_keycode;
	int keycode_count;
	int keysyms_per_keycode;
	int keycode;
	int group;
	KeySym *map, *syms;

	XDisplayKeycodes (QX11Info::display (), &min_keycode, &max_keycode);
	keycode_count = max_keycode - min_keycode + 1;
	map = XGetKeyboardMapping (QX11Info::display (),
				min_keycode, keycode_count, &keysyms_per_keycode);

	for (group = 0; group < desc->ctrls->num_groups && map != NULL; group ++) {
		if (((group << 1) & japan_groups) == 0)
			continue;
		for (syms = map, keycode = min_keycode; keycode <= max_keycode;
				keycode++, syms += keysyms_per_keycode) {
			if (syms[group * 2] != XK_backslash || syms[group * 2 + 1] != XK_bar)
				continue;
			japan_yen_bar_keys.append (keycode);
		}
	}

	if (map != NULL)
		XFree (map);
    XkbFreeKeyboard (desc, XkbAllComponentsMask, True);
#endif
}

QInputContext *
IBusClient::createInputContext ()
{
	IBusInputContext *ctx;
	QString ic;

	ic = createInputContextRemote ();

	ctx = new IBusInputContext (0, this, ic);
	context_list.append (ctx);

	if (! ic.isEmpty ()) {
		context_dict[ic] = ctx;
	}

	return (QInputContext *) ctx;
}

void
IBusClient::releaseInputContext (IBusInputContext *ctx)
{
	Q_ASSERT (ctx);

	QString ic = ctx->getIC ();

	if (ibus && !ic.isEmpty ()) {
		QDBusMessage message = QDBusMessage::createMethodCall (
								IBUS_NAME,
								IBUS_PATH,
								IBUS_INTERFACE,
								"ReleaseInputContext");
		message << ctx->getIC ();
		message = ibus->call (message);

		if (message.type () == QDBusMessage::ErrorMessage) {
			qWarning() << message.errorMessage ();
		}
		context_dict.remove (ic);
	}
	context_list.removeAll (ctx);
}

#ifndef Q_WS_X11
static void
translate_key_event (const QKeyEvent *event, quint32 *keyval, bool *is_press, quint32 *state)
{
	Q_ASSERT (event);
	Q_ASSERT (keyval);
	Q_ASSERT (is_press);
	Q_ASSERT (state);

	*keyval = event->key ();
	*is_press = (event->type() == QEvent::KeyPress);

	Qt::KeyboardModifiers modifiers = event->modifiers ();
	*state = 0;
	if (modifiers & Qt::ShiftModifier) {
		*state |= (1<< 0);
	}
	if (modifiers & Qt::ControlModifier) {
		*state |= (1<< 2);
	}
	if (modifiers & Qt::AltModifier) {
		*state |= (1<< 3);
	}
	if (modifiers & Qt::MetaModifier) {
		*state |= (1<< 28);
	}
	if (modifiers & Qt::KeypadModifier) {
		// *state |= (1<< 28);
	}
	if (modifiers & Qt::GroupSwitchModifier) {
		// *state |= (1<< 28);
	}
}

bool
IBusClient::filterEvent (IBusInputContext *ctx, QEvent *event)
{
	return true;
}
#endif

#ifdef Q_WS_X11
static inline bool
translate_x_key_event (XEvent *xevent, quint32 *keyval, bool *is_press, quint32 *state)
{
	Q_ASSERT (xevent);
	Q_ASSERT (keyval);
	Q_ASSERT (state);
	Q_ASSERT (is_press);

	if (xevent->type != KeyPress && xevent->type != KeyRelease)
		return false;

	*is_press = (xevent->type == KeyPress);
	*state = xevent->xkey.state;

	char key_str[64];

	if (XLookupString (&xevent->xkey, key_str, sizeof (key_str), (KeySym *)keyval, 0) <= 0) {
		*keyval = (quint32) XLookupKeysym (&xevent->xkey, 0);
	}

	return true;

}

bool
IBusClient::x11FilterEvent (IBusInputContext *ctx, QWidget * /* keywidget */, XEvent *xevent)
{
	Q_ASSERT (ctx);
	Q_ASSERT (keywidget);
	Q_ASSERT (xevent);

	quint32 keyval;
	quint32 state;
	bool is_press;

	if (ibus == NULL || !ibus->isConnected () || ctx->getIC().isEmpty ())
		return false;

	if (!translate_x_key_event (xevent, &keyval, &is_press, &state))
		return false;

#ifdef HAVE_XKB
	int group = XkbGroupForCoreState (state);
	if (keyval == XK_backslash && japan_groups & (1 << group)) {
		if (japan_yen_bar_keys.indexOf (xevent->xkey.keycode) != -1)
			keyval = XK_yen;
	}
#endif

	QDBusMessage message = QDBusMessage::createMethodCall (
							IBUS_NAME,
							IBUS_PATH,
							IBUS_INTERFACE,
							"ProcessKeyEvent");
	message << ctx->getIC ();
	message << keyval;
	message << is_press;
	message << state;

	message = ibus->call (message);

	if (message.type() == QDBusMessage::ErrorMessage) {
		qWarning() << message.errorMessage ();
		return false;
	}
	else
		return message.arguments ()[0].toBool ();
}
#endif

void
IBusClient::mouseHandler (IBusInputContext * /*ctx */, int /* x */, QMouseEvent * /* event */)
{
	return;
}

void
IBusClient::setCursorLocation (IBusInputContext *ctx, QRect &rect)
{
	Q_ASSERT (ctx);

	if (ibus == NULL || !ibus->isConnected () || ctx->getIC().isEmpty ())
		return;

	QDBusMessage message = QDBusMessage::createMethodCall (
							IBUS_NAME,
							IBUS_PATH,
							IBUS_INTERFACE,
							"SetCursorLocation");
	message << ctx->getIC ();
	message << rect.x ();
	message << rect.y ();
	message << rect.width ();
	message << rect.height ();
	message = ibus->call (message);
	if (message.type() == QDBusMessage::ErrorMessage) {
		qWarning() << message.errorMessage ();
	}
}

void
IBusClient::reset (IBusInputContext *ctx)
{
	Q_ASSERT (ctx);

	if (ibus == NULL || !ibus->isConnected () || ctx->getIC().isEmpty ())
		return;
	QDBusMessage message = QDBusMessage::createMethodCall (
							IBUS_NAME,
							IBUS_PATH,
							IBUS_INTERFACE,
							"Reset");
	message << ctx->getIC ();
	message = ibus->call (message);
	if (message.type() == QDBusMessage::ErrorMessage) {
		qWarning() << message.errorMessage ();
	}
}

void
IBusClient::focusIn (IBusInputContext *ctx)
{
	Q_ASSERT (ctx);

	if (ibus == NULL || !ibus->isConnected () || ctx->getIC().isEmpty ())
		return;
	QDBusMessage message = QDBusMessage::createMethodCall (
							IBUS_NAME,
							IBUS_PATH,
							IBUS_INTERFACE,
							"FocusIn");
	message << ctx->getIC ();
	message = ibus->call (message);
	if (message.type() == QDBusMessage::ErrorMessage) {
		qWarning() << message.errorMessage ();
	}

}

void
IBusClient::focusOut (IBusInputContext *ctx)
{
	Q_ASSERT (ctx);

	if (ibus == NULL || !ibus->isConnected () || ctx->getIC().isEmpty ())
		return;

	QDBusMessage message = QDBusMessage::createMethodCall (
							IBUS_NAME,
							IBUS_PATH,
							IBUS_INTERFACE,
							"FocusOut");
	message << ctx->getIC ();
	message = ibus->call (message);
	if (message.type() == QDBusMessage::ErrorMessage) {
		qWarning() << message.errorMessage ();
	}
}

void
IBusClient::setCapabilities (IBusInputContext *ctx, int caps)
{
	Q_ASSERT (ctx);

	if (ibus == NULL || !ibus->isConnected () || ctx->getIC().isEmpty ())
		return;

	QDBusMessage message = QDBusMessage::createMethodCall (
							IBUS_NAME,
							IBUS_PATH,
							IBUS_INTERFACE,
							"SetCapabilities");
	message << ctx->getIC ();
	message << caps;
	message = ibus->call (message);
	if (message.type() == QDBusMessage::ErrorMessage) {
		qWarning() << message.errorMessage ();
	}
}

void
IBusClient::widgetDestroyed (IBusInputContext * /* ctx */, QWidget * /* widget */)
{
}

bool
IBusClient::connectToBus ()
{
	QDBusConnection *connection = NULL;

	if (ibus != NULL)
		return false;

	connection = new QDBusConnection (
		QDBusConnection::connectToBus (
			ibus_addr,
			QString ("ibus")));

	if (!connection->isConnected ()) {
		delete connection;
		QDBusConnection::disconnectFromBus ("ibus");
		return false;
	}

	if (!connection->connect ("",
			"",
			"org.freedesktop.DBus.Local",
			"Disconnected",
			this, SLOT (slotIBusDisconnected()))) {
		qWarning () << "Can not connect Disconnected signal";
		delete connection;
		QDBusConnection::disconnectFromBus ("ibus");
		return false;
	}

	if (!connection->connect ("",
			IBUS_PATH,
			IBUS_INTERFACE,
			"CommitString",
			this, SLOT (slotCommitString(QString, QString)))) {
		qWarning () << "Can not connect CommitString signal";
		delete connection;
		QDBusConnection::disconnectFromBus ("ibus");
		return false;
	}

	if (!connection->connect ("",
			IBUS_PATH,
			IBUS_INTERFACE,
			"UpdatePreedit",
			this, SLOT (slotUpdatePreedit(QDBusMessage)))) {
		qWarning () << "Can not connect UpdatePreedit signal";
		delete connection;
		QDBusConnection::disconnectFromBus ("ibus");
		return false;
	}

	if (!connection->connect ("",
			IBUS_PATH,
			IBUS_INTERFACE,
			"ShowPreedit",
			this, SLOT (slotShowPreedit(QDBusMessage)))) {
		qWarning () << "Can not connect ShowPreedit signal";
		delete connection;
		QDBusConnection::disconnectFromBus ("ibus");
		return false;
	}

	if (!connection->connect ("",
			IBUS_PATH,
			IBUS_INTERFACE,
			"HidePreedit",
			this, SLOT (slotHidePreedit(QDBusMessage)))) {
		qWarning () << "Can not connect ShowPreedit signal";
		delete connection;
		QDBusConnection::disconnectFromBus ("ibus");
		return false;
	}

	ibus = connection;

	QList <IBusInputContext *>::iterator i;
	for (i = context_list.begin (); i != context_list.end (); ++i ) {
		QString ic = createInputContextRemote ();
		(*i)->setIC (ic);
		if (ic.isEmpty ())
			continue;
		context_dict[ic] = *i;
	}

	return true;
}

void
IBusClient::disconnectFromBus ()
{
	if (ibus) {
		delete ibus;
		ibus = NULL;
		QDBusConnection::disconnectFromBus ("ibus");
		QList <IBusInputContext *>::iterator i;
		for (i = context_list.begin (); i != context_list.end (); ++i ) {
			(*i)->setIC ("");
		}
		context_dict.clear ();
	}
}

void
IBusClient::slotDirectoryChanged (const QString & /*path*/)
{
	if (ibus && !ibus->isConnected ())
		disconnectFromBus ();

	if (ibus == NULL ) {
		if (QFile::exists (ibus_path)) {
			usleep (500);
			connectToBus ();
		}
	}
}

void
IBusClient::slotIBusDisconnected ()
{
	disconnectFromBus ();
}


void
IBusClient::slotCommitString (QString ic, QString text)
{
	IBusInputContext *ctx = context_dict[ic];
	ctx->commitString (text);
}

void
IBusClient::slotUpdatePreedit (QDBusMessage message)
{
	QString ic;
	QString text;
	QVariant attrs;
	int cursor_pos;
	bool visible;

	QList<QVariant> args = message.arguments ();

	ic = args[0].toString ();
	text = args[1].toString ();
	attrs = args[2];
	cursor_pos = args[3].toInt ();
	visible = args[4].toBool ();
	QList <QList <quint32> > attr_list;
	const QDBusArgument arg = attrs.value <QDBusArgument> ();
	arg.beginArray ();
	while ( !arg.atEnd ()) {
		quint32 type, value, start_index, end_index;

		arg.beginArray ();
		arg >> type >> value >> start_index >> end_index;
		arg.endArray ();
		QList <quint32> attr;
		attr.append (type);
		attr.append (value);
		attr.append (start_index);
		attr.append (end_index);
		attr_list.append (attr);
	}
	arg.endArray ();

	IBusInputContext *ctx = context_dict[ic];
	ctx->updatePreedit (text, attr_list, cursor_pos, visible);
}

void
IBusClient::slotShowPreedit (QDBusMessage message)
{
	QString ic;

	QList<QVariant> args = message.arguments ();

	ic = args[0].toString ();
	IBusInputContext *ctx = context_dict[ic];
	ctx->showPreedit ();
}

void
IBusClient::slotHidePreedit (QDBusMessage message)
{
	QString ic;

	QList<QVariant> args = message.arguments ();

	ic = args[0].toString ();
	IBusInputContext *ctx = context_dict[ic];
	ctx->hidePreedit ();
}

