/* vim:set noet ts=4: */
/*
 * ibus - The Input Bus
 *
 * Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */
#include <cassert>
#include <Qt>
#include <QInputContextPlugin>
#include "ibus-client.h"

using namespace Qt;

#define IBUS_IDENTIFIER_NAME "ibus"

static IBusClient *client;

/* The class Definition */
class IBusInputContextPlugin: public QInputContextPlugin
{

    private:

        /**
         * The language list for SCIM.
         */
        static QStringList ibus_languages;

    public:

		IBusInputContextPlugin (QObject *parent = 0);

        ~IBusInputContextPlugin ();

        QStringList keys () const;

        QStringList languages (const QString &key);

        QString description (const QString &key);
        
        QInputContext *create (const QString &key);

        QString displayName (const QString &key);

};


/* Implementations */
QStringList IBusInputContextPlugin::ibus_languages;


IBusInputContextPlugin::IBusInputContextPlugin (QObject *parent)
	:QInputContextPlugin (parent)
{
}


IBusInputContextPlugin::~IBusInputContextPlugin ()
{
	if (client != NULL) {
		delete client;
		client = NULL;
	}
}

QStringList 
IBusInputContextPlugin::keys () const 
{
    QStringList identifiers;
    identifiers.push_back (IBUS_IDENTIFIER_NAME);
    return identifiers;
}


QStringList
IBusInputContextPlugin::languages (const QString & key)
{
	if (key.toLower () != IBUS_IDENTIFIER_NAME)
		return QStringList ();

    if (ibus_languages.empty ()) {
        ibus_languages.push_back ("zh_CN");
        ibus_languages.push_back ("zh_TW");
        ibus_languages.push_back ("zh_HK");
        ibus_languages.push_back ("ja");
        ibus_languages.push_back ("ko");
    }
    return ibus_languages;
}


QString
IBusInputContextPlugin::description (const QString &key)
{
	if (key.toLower () != IBUS_IDENTIFIER_NAME)
		return QString ("");
    
	return QString::fromUtf8 ("Qt immodule plugin for IBus");
}


QInputContext *
IBusInputContextPlugin::create (const QString &key)
{
    if (key.toLower () != IBUS_IDENTIFIER_NAME) {
        return NULL;
    } else {
		if (client == NULL) {
			client = new IBusClient ();
		}
        return client->createInputContext ();
    }
}


QString IBusInputContextPlugin::displayName (const QString &key)
{
    return key;
}

Q_EXPORT_PLUGIN2 (IBusInputContextPlugin, IBusInputContextPlugin)
