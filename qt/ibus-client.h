#ifndef __IBUS_CLIENT_H_
#define __IBUS_CLIENT_H_
#include <QObject>
#include <QList>
#include <QHash>
#include <QInputContext>
#include <QFileSystemWatcher>
#include <QDBusMessage>

class QDBusConnection;
class IBusInputContext;

class IBusClient : public QObject {
	Q_OBJECT
public:
	IBusClient ();
	~IBusClient ();

public:

#ifndef Q_WS_X11
    bool filterEvent (IBusInputContext *ctx, const QEvent *event);
#endif

	bool isComposing (IBusInputContext const *ctx);
	void mouseHandler (IBusInputContext *ctx, int x, QMouseEvent *event);
	void widgetDestroyed (IBusInputContext *ctx, QWidget *widget);

#ifdef Q_WS_X11
	bool x11FilterEvent (IBusInputContext *ctx, QWidget *keywidget, XEvent *xevent);
#endif

public:
	QInputContext *createInputContext ();
	void releaseInputContext (IBusInputContext *ctx);
	void setCursorLocation (IBusInputContext *ctx, QRect &rect);
	void focusIn (IBusInputContext *ctx);
	void focusOut (IBusInputContext *ctx);
	void reset (IBusInputContext *ctx);

private slots:
	void slotDirectoryChanged (const QString &path);
	void slotFileChanged (const QString &path);
	void slotIBusDisconnected ();
	void slotCommitString (QString ic, QString text);
	//void slotUpdatePreedit (QString ic, QString text, QVariant attrs, int cursor_pos, bool show);
	void slotUpdatePreedit (QDBusMessage message);

private:
	bool connectToBus ();
	void disconnectFromBus ();
	QString createInputContextRemote ();

	QDBusConnection *ibus;
	QFileSystemWatcher watcher;
	QList <IBusInputContext *> context_list;
	QHash <QString, IBusInputContext *>context_dict;
	IBusInputContext *focused_context;
};

#endif // __IBUS_CLIENT_H_
