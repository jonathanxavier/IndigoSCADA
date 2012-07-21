 /**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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

#include "connectiontable.h"

ConnectionTable::ConnectionTable( QWidget *parent, const char *name )
    : QTable( 0, 4, parent, name )
{
    setSorting( TRUE );
    setShowGrid( FALSE );
    setFocusStyle( FollowStyle );
    setSelectionMode( SingleRow );
    horizontalHeader()->setLabel( 0, tr( "Sender" ) );
    horizontalHeader()->setLabel( 1, tr( "Signal" ) );
    horizontalHeader()->setLabel( 2, tr( "Receiver" ) );
    horizontalHeader()->setLabel( 3, tr( "Slot" ) );
    setColumnStretchable( 0, TRUE );
    setColumnStretchable( 1, TRUE );
    setColumnStretchable( 2, TRUE );
    setColumnStretchable( 3, TRUE );
}

void ConnectionTable::sortColumn( int col, bool ascending, bool )
{
    horizontalHeader()->setSortIndicator( col, ascending );
    if ( isEditing() )
	endEdit( currEditRow(), currEditCol(), FALSE, FALSE );
    QTable::sortColumn( col, ascending, TRUE );
    setCurrentCell( 0, 0 );
    emit resorted();
}
