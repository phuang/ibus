#include <QtDebug>
#include "ibus-input-context.h"
#include "ibus-client.h"

IBusInputContext::IBusInputContext (QObject *parent, IBusClient *client)
	: QInputContext (parent), client (client)
{
}

IBusInputContext::~IBusInputContext ()
{
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
#endif

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

