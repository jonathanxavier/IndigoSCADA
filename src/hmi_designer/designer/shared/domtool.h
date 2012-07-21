/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DOMTOOL_H
#define DOMTOOL_H

#include <qvariant.h>
#include <qnamespace.h>

class QDomElement;
class QDomDocument;

class DomTool : public Qt
{
public:
    static QVariant readProperty( const QDomElement& e, const QString& name, const QVariant& defValue );
    static QVariant readProperty( const QDomElement& e, const QString& name, const QVariant& defValue, QString& comment );
    static bool hasProperty( const QDomElement& e, const QString& name );
    static QStringList propertiesOfType( const QDomElement& e, const QString& type );
    static QVariant elementToVariant( const QDomElement& e, const QVariant& defValue );
    static QVariant elementToVariant( const QDomElement& e, const QVariant& defValue, QString &comment );
    static QVariant readAttribute( const QDomElement& e, const QString& name, const QVariant& defValue );
    static QVariant readAttribute( const QDomElement& e, const QString& name, const QVariant& defValue, QString& comment );
    static bool hasAttribute( const QDomElement& e, const QString& name );
    static QColor readColor( const QDomElement &e );
    static void fixDocument( QDomDocument& );
};


#endif // DOMTOOL_H
