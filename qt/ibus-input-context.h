#ifndef __IBUS_INPUT_CONTEXT_H_
#define __IBUS_INPUT_CONTEXT_H_
#include <QInputContext>

class IBusInputContext : public QInputContext  {
public:
	IBusInputContext (QObject * parent = 0);
public:
	bool filterEvent (const QEvent *event);
	QFont font () const;
	QString identifierName ();
	bool isComposing() const;
	QString language();
	void mouseHandler (int x, QMouseEvent *event);
	void reset();
	void update ();
	void widgetDestroyed (QWidget *widget);
	bool x11FilterEvent (QWidget *keywidget, XEvent *event);

};

#endif //__IBUS_INPUT_CONTEXT_H_
