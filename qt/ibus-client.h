#ifndef __IBUS_CLIENT_H_
#define __IBUS_CLIENT_H_
#include <QObject>
#include <QInputContext>

class IBusClient : public QObject {
	Q_OBJECT
public:
	IBusClient ();
	~IBusClient ();

#ifdef Q_WS_X11
	bool x11FilterEvent (QInputContext *ctx, QWidget *keywidget, XEvent *xevent);
#endif

public:
	QInputContext *createInputContext ();
};

#endif // __IBUS_CLIENT_H_
