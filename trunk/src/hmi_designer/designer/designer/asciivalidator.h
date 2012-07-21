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

#ifndef ASCIIVALIDATOR_H
#define ASCIIVALIDATOR_H


#include <qvalidator.h>


class AsciiValidator: public QValidator
{
    Q_OBJECT
public:
    AsciiValidator( QObject * parent, const char *name = 0 );
    AsciiValidator( bool funcName, QObject * parent, const char *name = 0 );
    AsciiValidator( const QString &allow, QObject * parent, const char *name = 0 );
    ~AsciiValidator();

    QValidator::State validate( QString &, int & ) const;

private:
    bool functionName;
    QString allowedChars;

};


#endif
