#include "ibus-client.h"
#include "ibus-input-context.h"

#ifdef Q_WS_X11
# include <QX11Info>
# include <X11/Xlib.h>
# include <X11/keysym.h>
# include <X11/Xutil.h>
#endif

IBusClient::IBusClient ()
{
}

IBusClient::~IBusClient ()
{
}


QInputContext *
IBusClient::createInputContext ()
{
	IBusInputContext *ctx;

	ctx = new IBusInputContext (0, this);

	return (QInputContext *) ctx;
}

#ifdef Q_WS_X11
static inline bool
translate_x_key_event (XEvent *xevent, quint32 *keyval, quint32 *state, bool *is_press)
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

	}

	return true;

}

bool
IBusClient::x11FilterEvent (QInputContext *ctx, QWidget *keywidget, XEvent *xevent)
{
	quint32 keyval;
	quint32 state;
	bool is_press;

	if (!translate_x_key_event (xevent, &keyval, &state, &is_press))
		return false;

	qDebug ("%c 0x%08x %d", keyval, state, is_press);

	return true;
}
#endif
