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
#include <QtDebug>
#include <QFile>
#include <QDBusConnection>
#include <QCoreApplication>
#include <QDBusMessage>
#include <QDBusArgument>

#include "ibus-client.h"
#include "ibus-input-context.h"

#ifdef Q_WS_X11
# include <QX11Info>
# include <X11/Xlib.h>
# include <X11/keysym.h>
# include <X11/Xutil.h>
#endif

#define IBUS_NAME	"org.freedesktop.IBus"
#define IBUS_PATH	"/org/freedesktop/IBus"
#define IBUS_INTERFACE	"org.freedesktop.IBus"


IBusClient::IBusClient ()
	: ibus (NULL), focused_context (NULL)
{
	connectToBus ();

	QObject::connect (
		&watcher,
		SIGNAL(directoryChanged(const QString &)),
		this,
		SLOT(slotDirectoryChanged(const QString &)));
	QString ibus_dir;
	ibus_dir = QString ("/tmp/ibus-%1/").arg (getenv ("USER"));
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

	if (focused_context != ctx) {
		focusIn (ctx);
	}

	if (ibus == NULL || !ibus->isConnected () || ctx->getIC().isEmpty ())
		return false;

	if (!translate_x_key_event (xevent, &keyval, &is_press, &state))
		return false;
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

	if (focused_context != ctx) {
		focusIn (ctx);
	}

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
	if (focused_context != ctx && focused_context != NULL)
		focusOut (focused_context);
	focused_context = ctx;

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

	if (focused_context != ctx)
		return;

	focused_context = NULL;

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
IBusClient::widgetDestroyed (IBusInputContext * /* ctx */, QWidget * /* widget */)
{
}

bool
IBusClient::connectToBus ()
{
	QString address;
	QString session;
	QString username;
	QDBusConnection *connection = NULL;

	if (ibus != NULL)
		return false;

	session = getenv ("DISPLAY");
	session.replace (":", "-");
	
	username = getlogin ();
	if (username.isNull ())
		username = getenv ("LOGNAME");
	if (username.isNull ())
		username = getenv ("USER");
	if (username.isNull ())
		username = getenv ("LNAME");
	if (username.isNull ())
		username = getenv ("USERNAME");

	address = QString("unix:path=/tmp/ibus-%1/ibus-%2").arg (username, session);
	connection = new QDBusConnection (
		QDBusConnection::connectToBus (
			address,
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
		QString session = getenv ("DISPLAY");
		session.replace (":", "-");
		QString path = QString("/tmp/ibus-%1/ibus-%2").arg (getenv ("USER"), session);
		if (QFile::exists (path)) {
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

