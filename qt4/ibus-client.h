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
#ifndef __IBUS_CLIENT_H_
#define __IBUS_CLIENT_H_
#include <QObject>
#include <QList>
#include <QHash>
#include <QInputContext>
#include <QFileSystemWatcher>
#include <QDBusMessage>

class QDBusConnection;
class IBusInputContext;

class IBusClient : public QObject {
	Q_OBJECT
public:
	IBusClient ();
	~IBusClient ();

public:

#ifndef Q_WS_X11
    bool filterEvent (IBusInputContext *ctx, const QEvent *event);
#endif

	bool isComposing (IBusInputContext const *ctx);
	void mouseHandler (IBusInputContext *ctx, int x, QMouseEvent *event);
	void widgetDestroyed (IBusInputContext *ctx, QWidget *widget);

#ifdef Q_WS_X11
	bool x11FilterEvent (IBusInputContext *ctx, QWidget *keywidget, XEvent *xevent);
#endif

public:
	QInputContext *createInputContext ();
	void releaseInputContext (IBusInputContext *ctx);
	void setCursorLocation (IBusInputContext *ctx, QRect &rect);
	void focusIn (IBusInputContext *ctx);
	void focusOut (IBusInputContext *ctx);
	void reset (IBusInputContext *ctx);

private slots:
	void slotDirectoryChanged (const QString &path);
	void slotFileChanged (const QString &path);
	void slotIBusDisconnected ();
	void slotCommitString (QString ic, QString text);
	//void slotUpdatePreedit (QString ic, QString text, QVariant attrs, int cursor_pos, bool show);
	void slotUpdatePreedit (QDBusMessage message);

private:
	bool connectToBus ();
	void disconnectFromBus ();
	QString createInputContextRemote ();

	QDBusConnection *ibus;
	QFileSystemWatcher watcher;
	QList <IBusInputContext *> context_list;
	QHash <QString, IBusInputContext *>context_dict;
	IBusInputContext *focused_context;
};

#endif // __IBUS_CLIENT_H_
