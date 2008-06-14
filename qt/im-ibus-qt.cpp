/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
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

#ifdef QT4
#include <Qt>
#include <QInputContextPlugin>

using namespace Qt;
#else
#include <qinputcontextplugin.h>
#endif

/* Static Variables */
static IBusClientQt *client = NULL;

/* The class Definition */
class IBusInputContextPlugin: public QInputContextPlugin
{

    private:

        /**
         * The language list for SCIM.
         */
        static QStringList ibus_languages;

    public:

        IBusInputContextPlugin ();

        ~IBusInputContextPlugin ();

        QStringList keys () const;

        QStringList languages (const QString &key);

        QString description (const QString &key);
        
        QInputContext *create (const QString &key);

        QString displayName (const QString &key);

};


/* Implementations */
QStringList IBusInputContextPlugin::ibus_languages;


IBusInputContextPlugin::IBusInputContextPlugin ()
{
}


IBusInputContextPlugin::~IBusInputContextPlugin ()
{
    delete client;
    client = NULL;
}

QStringList 
IBusInputContextPlugin::keys () const 
{
    QStringList identifiers;
    identifiers.push_back (IBUS_IDENTIFIER_NAME);
    return identifiers;
}


QStringList ScimBridgeInputContextPlugin::languages (const QString &key)
{
    if (ibus_languages.empty ()) {
        ibus_languages.push_back ("zh_CN");
        ibus_languages.push_back ("zh_TW");
        ibus_languages.push_back ("zh_HK");
        ibus_languages.push_back ("ja");
        ibus_languages.push_back ("ko");
    }
    return ibus_languages;
}


QString ScimBridgeInputContextPlugin::description (const QString &key)
{
    return QString::fromUtf8 ("Qt immodule plugin for IBus");
}


QInputContext *ScimBridgeInputContextPlugin::create (const QString &key)
{
#ifdef QT4
    if (key.toLower () != IBUS_IDENTIFIER_NAME) {
#else
    if (key.lower () != IBUS_IDENTIFIER_NAME) {
#endif
        return NULL;
    } else {
        if (client == NULL) client = new IBusClientQt ();
        return IBusClientIMContext::alloc ();
    }
}


QString IBusInputContextPlugin::displayName (const QString &key)
{
    return key;
}

#ifdef QT4
Q_EXPORT_PLUGIN2 (IBusInputContextPlugin, IBusInputContextPlugin)
#else
Q_EXPORT_PLUGIN (IBusInputContextPlugin)
#endif
