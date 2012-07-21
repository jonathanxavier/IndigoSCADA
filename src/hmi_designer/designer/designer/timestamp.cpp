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

#include "timestamp.h"
#include <qfile.h>
#include <qfileinfo.h>

class QTimer;

TimeStamp::TimeStamp( QObject *parent, const QString &f )
    : QObject( parent ), filename( f ), autoCheck( FALSE )
{
    timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ), this, SLOT( autoCheckTimeStamp() ) );
    update();
}

void TimeStamp::setFileName( const QString &f )
{
    filename = f;
    update();
}

QString TimeStamp::fileName() const
{
    return filename;
}

void TimeStamp::setAutoCheckEnabled( bool a )
{
    autoCheck = a;
    if ( autoCheck )
	timer->start( 5000 );
    else
	timer->stop();
}

void TimeStamp::update()
{
    QFile f( filename );
    if ( !f.exists() )
	return;
    lastTimeStamp = QFileInfo( f ).lastModified();
}

bool TimeStamp::isUpToDate() const
{
    QFile f( filename );
    if ( !f.exists() )
	return TRUE;
    return lastTimeStamp == QFileInfo( f ).lastModified();
}

bool TimeStamp::isAutoCheckEnabled() const
{
    return autoCheck;
}

void TimeStamp::autoCheckTimeStamp()
{
    if ( !isUpToDate() )
	emit timeStampChanged();
}
