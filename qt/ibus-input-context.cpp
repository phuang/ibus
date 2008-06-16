#include <QtDebug>
#include "ibus-input-context.h"
#include "ibus-client.h"

IBusInputContext::IBusInputContext (QObject *parent, IBusClient *client, QString &ic)
	: QInputContext (parent), client (client), ic (ic)
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
	QInputContext::update ();
}

bool
IBusInputContext::isComposing() const
{
	return client->isComposing (this);
}

void
IBusInputContext::widgetDestroyed (QWidget *widget)
{
	client->widgetDestroyed (this, widget);
	QInputContext::widgetDestroyed (widget);
}


bool
IBusInputContext::x11FilterEvent (QWidget *keywidget, XEvent *xevent)
{
#ifdef Q_WS_X11
	if (client->x11FilterEvent (this, keywidget, xevent))
		return true;
	return QInputContext::x11FilterEvent (keywidget, xevent);
#else
	return QInputContext::x11FilterEvent (keywidget, xevent);
#endif
}

void
IBusInputContext::setIC (QString ic)
{
	this->ic = ic;
}

QString
IBusInputContext::getIC ()
{
	return ic;
}
