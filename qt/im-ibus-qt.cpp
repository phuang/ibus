/*
 * IBus
 *
 * Copyright (c) 2008 Huang Peng <shawn.p.huang@gmail.com>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.LGPL included in the package of this file.
 * You can also redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.GPL included in the package of this file.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <cassert>
#include <Qt>
#include <QInputContextPlugin>
#include "ibus-input-context.h"

using namespace Qt;

#define IBUS_IDENTIFIER_NAME "ibus"

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
	fprintf (stderr, "Init\n");
}


IBusInputContextPlugin::~IBusInputContextPlugin ()
{
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
        return new IBusInputContext (0);
    }
}


QString IBusInputContextPlugin::displayName (const QString &key)
{
    return key;
}

Q_EXPORT_PLUGIN2 (IBusInputContextPlugin, IBusInputContextPlugin)
