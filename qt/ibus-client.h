#ifndef __IBUS_CLIENT_H_
#define __IBUS_CLIENT_H_
#include <QList>
#include <QHash>
#include <QObject>
#include <QInputContext>
#include <QFileSystemWatcher>

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
	void reset (IBusInputContext *ctx);
	void widgetDestroyed (IBusInputContext *ctx, QWidget *widget);

#ifdef Q_WS_X11
	bool x11FilterEvent (IBusInputContext *ctx, QWidget *keywidget, XEvent *xevent);
#endif

public:
	QInputContext *createInputContext ();
	void releaseInputContext (IBusInputContext *ctx);

private slots:
	void slotDirectoryChanged (const QString &path);
	void slotFileChanged (const QString &path);
	void slotIBusDisconnected ();

private:
	bool connectToBus ();
	void disconnectFromBus ();
	QString createInputContextRemote ();

	QDBusConnection *ibus;
	QFileSystemWatcher watcher;
	QList <IBusInputContext *> context_list;
	QHash <QString, IBusInputContext *>context_dict;
};

#endif // __IBUS_CLIENT_H_
