#ifndef __IBUS_INPUT_CONTEXT_H_
#define __IBUS_INPUT_CONTEXT_H_
#include <QInputContext>
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
	void widgetDestroyed (QWidget *widget);
	bool x11FilterEvent (QWidget *keywidget, XEvent *event);
	void setIC (QString ic);
	QString getIC ();

signals:
#if 1
	bool filterEvent (IBusInputContext *ctx, const QEvent *event);
	QFont font (IBusInputContext *ctx) const;
	QString identifierName (IBusInputContext *ctx);
	bool isComposing(IBusInputContext *ctx) const;
	QString language(IBusInputContext *ctx);
	void mouseHandler (IBusInputContext *ctx, int x, QMouseEvent *event);
	void reset(IBusInputContext *ctx);
	void update (IBusInputContext *ctx);
	void widgetDestroyed (IBusInputContext *ctx, QWidget *widget);
	bool x11FilterEvent (IBusInputContext *ctx, QWidget *keywidget, XEvent *event);
#endif

private:
	IBusClient *client;
	QString ic;
};

#endif //__IBUS_INPUT_CONTEXT_H_
