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
#include "ibus-input-context.h"
#include "ibus-client.h"
#include <QtDebug>
#include <QInputMethodEvent>
#include <QTextCharFormat>

typedef QInputMethodEvent::Attribute QAttribute;

IBusInputContext::IBusInputContext (QObject *parent, IBusClient *client, QString &ic)
	: QInputContext (parent),
	  client (client),
	  ic (ic),
	  preedit_visible (false),
	  has_focus (false)
{
}

IBusInputContext::~IBusInputContext ()
{
	client->releaseInputContext (this);
}

bool
IBusInputContext::filterEvent (const QEvent *event)
{
#ifndef Q_WS_X11
	if (client->filterEvent (this, event))
		return true;
	return QInputContext::filterEvent (event);
#else
	return QInputContext::filterEvent (event);
#endif
}

QFont
IBusInputContext::font () const
{
	return QInputContext::font ();
}

QString
IBusInputContext::identifierName ()
{
	return QString ("ibus");
}

QString
IBusInputContext::language()
{
	return QString ("");
}

void
IBusInputContext::mouseHandler (int x, QMouseEvent *event)
{
	client->mouseHandler (this, x, event);
	QInputContext::mouseHandler (x, event);
}

void
IBusInputContext::reset()
{
	client->reset (this);
}

void
IBusInputContext::update ()
{
	QWidget *widget;

	if ((widget = focusWidget ()) == NULL)
		return;

	QRect rect = widget->inputMethodQuery(Qt::ImMicroFocus).toRect ();

	QPoint topleft = widget->mapToGlobal(QPoint(0,0));
	rect.translate (topleft);

	client->setCursorLocation (this, rect);

#if 0
	QVariant value;
	qDebug () << "== update == ";
	value = widget->inputMethodQuery(Qt::ImMicroFocus);
	qDebug () << "Qt::ImMicroFocus " << value;
	value = widget->inputMethodQuery(Qt::ImFont);
	qDebug () << "Qt::ImFont " <<value;
	value = widget->inputMethodQuery(Qt::ImCursorPosition);
	qDebug () << "Qt::ImCursorPosition " << value;
	value = widget->inputMethodQuery(Qt::ImSurroundingText);
	qDebug () << "Qt::ImSurroundingText " << value;
	value = widget->inputMethodQuery(Qt::ImCurrentSelection);
	qDebug () << "Qt::ImCurrentSelection " << value;
#endif
}

bool
IBusInputContext::isComposing() const
{
	return (!preedit_string.isEmpty ()) && preedit_visible;
}

void
IBusInputContext::setFocusWidget (QWidget *widget)
{
	QInputContext::setFocusWidget (widget);

	if (widget == NULL) {
		has_focus = false;
		client->focusOut (this);
	}
	else {
		/* KateView can not support preedit well. */
		if (widget->inherits("KateViewInternal"))
			client->setCapabilities (this, 0);
		else
			client->setCapabilities (this, 1);

		has_focus = true;
		client->focusIn (this);
		update ();
	}
}

void
IBusInputContext::widgetDestroyed (QWidget *widget)
{
	QInputContext::widgetDestroyed (widget);
	update ();
}

#ifdef Q_WS_X11
bool
IBusInputContext::x11FilterEvent (QWidget *keywidget, XEvent *xevent)
{
	if (has_focus && client->x11FilterEvent (this, keywidget, xevent))
			return true;
	return QInputContext::x11FilterEvent (keywidget, xevent);
}
#endif

void
IBusInputContext::setIC (QString ic)
{
	this->ic = ic;
	if (has_focus && !ic.isEmpty ()) {
		client->focusIn (this);
	}
}

QString
IBusInputContext::getIC ()
{
	return ic;
}

void
IBusInputContext::commitString (QString text)
{
	QInputMethodEvent event;
	event.setCommitString (text);
	sendEvent (event);
	update ();
}

void
IBusInputContext::updatePreedit (QString text, QList <QList <quint32> > attr_list, int cursor_pos, bool visible)
{
	// qDebug () << text << cursor_pos << show;
	QList <QAttribute> qattrs;

	if (visible) {
		// append cursor pos
		qattrs.append (QAttribute (QInputMethodEvent::Cursor, cursor_pos, true, 0));

		// append attributes
		for (QList <QList <quint32> >::iterator it = attr_list.begin (); it != attr_list.end(); ++ it) {

			QList <quint32> attr = *it;
			QTextCharFormat format;

			switch (attr[0]) {
			case 1: // underline
				format.setUnderlineStyle (QTextCharFormat::SingleUnderline);
				break;
			case 2: // foreground
				format.setForeground (QBrush (QColor (attr[1])));
				break;
			case 3: // background
				format.setBackground (QBrush (QColor (attr[1])));
				break;
			default:
				break;
			}

			qattrs.append (QAttribute (QInputMethodEvent::TextFormat, attr[2], attr[3] - attr[2], QVariant (format)));
			// qDebug () << attr[0] << attr[2] << attr[3] - attr[2];
		}
	}
	else {
		qattrs.append (QAttribute (QInputMethodEvent::Cursor, 0, true, 0));
		text = "";
		cursor_pos = 0;
	}

	preedit_string = text;
	preedit_visible = visible;
	preedit_attrs = attr_list;
	preedit_cursor_pos = cursor_pos;

	QInputMethodEvent event (text, qattrs);
	sendEvent (event);
	update ();
}

void
IBusInputContext::showPreedit ()
{
	if (preedit_visible)
		return;

	updatePreedit (preedit_string, preedit_attrs, preedit_cursor_pos, TRUE);
}

void
IBusInputContext::hidePreedit ()
{
	if (!preedit_visible)
		return;

	updatePreedit (preedit_string, preedit_attrs, preedit_cursor_pos, FALSE);
}

