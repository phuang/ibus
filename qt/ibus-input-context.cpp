#include <QtDebug>
#include "ibus-input-context.h"
#ifdef Q_WS_X11
# include <QX11Info>
# include <X11/Xlib.h>
# include <X11/keysym.h>
# include <X11/Xutil.h>
#endif

IBusClient *IBusInputContext::client = (IBusClient *)0;
int IBusInputContext::client_ref = 0;

IBusInputContext::IBusInputContext (QObject *parent)
	: QInputContext (parent)
{
	if (client == 0) {
		client = new IBusClient ();
		client_ref = 0;
	}
	client_ref ++;
}

IBusInputContext::~IBusInputContext ()
{
	client_ref --;
	if (client_ref <= 0) {
		delete client;
		client = (IBusClient *) 0;
		client_ref = 0;
	}
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

#ifdef Q_WS_X11
static bool
translate_x_key_event (XEvent *event, quint32 *keyval, bool *is_press, quint32 *state)
{
	Q_ASSERT (event);
	Q_ASSERT (keyval);
	Q_ASSERT (is_press);
	Q_ASSERT (state);

	if (event->type == KeyPress) {
		*is_press = true;
	}
	else if (event->type == KeyRelease) {
		*is_press = false;
	}
	else {
		return false;
	}
	char key_str[64];
	if (XLookupString (&event->xkey, key_str, sizeof (key_str), (KeySym *)keyval, 0) <= 0) {
		*keyval = XLookupKeysym (&event->xkey, 0);
	}
	*state = event->xkey.state;
	return true;
}
#endif

bool
IBusInputContext::x11FilterEvent (QWidget *keywidget, XEvent *xevent)
{
#ifdef Q_WS_X11
	quint32 keyval;
	bool is_press;
	quint32 state;

	if (translate_x_key_event (xevent, &keyval, &is_press, &state)) {
		qDebug ("%c %d %d\n", keyval, is_press, state);

		return false;
	}
	return QInputContext::x11FilterEvent (keywidget, xevent);
#else
	return QInputContext::x11FilterEvent (keywidget, xevent);
#endif
}

