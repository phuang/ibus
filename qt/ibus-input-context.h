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
	void updatePreedit (QString text, QList <QList <quint32> > attr_list, int cursor_pos, bool show);


private:
	IBusClient *client;
	QString ic;
	QString preedit_string;
	bool preedit_visible;
};

#endif //__IBUS_INPUT_CONTEXT_H_
