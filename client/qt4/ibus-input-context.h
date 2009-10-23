/* vim:set noet ts=4: */
/*
 * ibus - The Input Bus
 *
 * Copyright (c) 2007-2009 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (c) 2007-2009 Red Hat, Inc.
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
#ifndef __IBUS_INPUT_CONTEXT_H_
#define __IBUS_INPUT_CONTEXT_H_
#include <QInputContext>
#include <QList>
#include "ibus-client.h"

class IBusClient;

class IBusInputContext : public QInputContext  {
	Q_OBJECT
public:
	IBusInputContext (QObject *parent, IBusClient *client, QString &ic);
	~IBusInputContext ();

public:
	bool filterEvent (const QEvent *event);
	QFont font () const;
	QString identifierName ();
	bool isComposing() const;
	QString language();
	void mouseHandler (int x, QMouseEvent *event);
	void reset();
	void update ();
	void setFocusWidget (QWidget *widget );
	void widgetDestroyed (QWidget *widget);
#ifdef Q_WS_X11
	bool x11FilterEvent (QWidget *keywidget, XEvent *event);
#endif
	void setIC (QString ic);
	QString getIC ();

	void commitString (QString text);
	void updatePreedit (QString text, QList <QList <quint32> > attr_list, int cursor_pos, bool visible);
	void showPreedit ();
	void hidePreedit ();

private slots:

private:
	IBusClient *client;
	QString ic;
	QString preedit_string;
	bool preedit_visible;
	int preedit_cursor_pos;
	bool has_focus;
	int caps;
	QList <QList <quint32> > preedit_attrs;
};

#endif //__IBUS_INPUT_CONTEXT_H_
