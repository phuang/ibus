#include <QTextStream>
#include "ibus-input-context.h"

IBusInputContext::IBusInputContext (QObject *parent)
	: QInputContext (parent)
{
}

IBusInputContext::~IBusInputContext ()
{
}

bool
IBusInputContext::filterEvent (const QEvent *event)
{
	return QInputContext::filterEvent (event);
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
	QInputContext::mouseHandler (x, event);
}

void
IBusInputContext::reset()
{
}

void
IBusInputContext::update ()
{
	QInputContext::update ();
}

bool
IBusInputContext::isComposing() const
{
	return false;
}

void
IBusInputContext::widgetDestroyed (QWidget *widget)
{
	QInputContext::widgetDestroyed (widget);
}

bool
IBusInputContext::x11FilterEvent (QWidget *keywidget, XEvent *event)
{
	return QInputContext::x11FilterEvent (keywidget, event);
}

