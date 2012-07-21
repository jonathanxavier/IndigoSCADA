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

#include "asciivalidator.h"

#include <qstring.h>

AsciiValidator::AsciiValidator( QObject * parent, const char *name )
    : QValidator( parent, name ), functionName( FALSE )
{
}

AsciiValidator::AsciiValidator( bool funcName, QObject * parent, const char *name )
    : QValidator( parent, name ), functionName( funcName )
{
}

AsciiValidator::AsciiValidator( const QString &allow, QObject * parent, const char *name )
    : QValidator( parent, name ), functionName( FALSE ), allowedChars( allow )
{
}

AsciiValidator::~AsciiValidator()
{
}

QValidator::State AsciiValidator::validate( QString &s, int & ) const
{
    bool inParen = FALSE;
    if ( !s.isEmpty() && s[0].row() == 0 && s[0].cell() >= '0' && s[0].cell() <= '9' ) {
	s[0] = '_';
    }
    for ( int i = 0; i < (int) s.length(); i++ ) {
	uchar r = s[i].row();
	uchar c = s[i].cell();
	if ( functionName && inParen ) {
	    if ( c != ')' )
		continue;
	    s.truncate( i + 1 );
	    return QValidator::Acceptable;
	}
	if ( r == 0 && ( ( c >= '0' && c <= '9' ) ||
			 ( c >= 'a' && c <= 'z' ) ||
			 ( c >= 'A' && c <= 'Z' ) ) )
	    continue;
	if ( functionName ) {
	    if ( c == '(' ) {
		inParen = TRUE;
		continue;
	    }
	}
	
	if ( allowedChars.find( s[ i ] ) != -1 )
	    continue;
	
	s[i] = '_';
    }
    return QValidator::Acceptable;
}
