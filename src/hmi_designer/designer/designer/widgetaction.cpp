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

#include "widgetaction.h"
#include <qwidget.h>
#include <qtoolbutton.h>
#include "mainwindow.h"
#include <qstylefactory.h>
#include <qstyle.h>

static QStyle *windowsStyle = 0;

void WidgetAction::addedTo( QWidget *w, QWidget *container )
{
    if ( w->inherits( "QToolButton" ) && container->parent()->parent() &&
	 container->parent()->parent()->inherits( "QScrollView" ) ) {
	if ( !windowsStyle )
	    windowsStyle = QStyleFactory::create( "windows" );
	w->setStyle( windowsStyle );
	( (QToolButton*)w )->setUsesTextLabel( TRUE );
	( (QToolButton*)w )->setTextPosition( QToolButton::Right );
	( (QToolButton*)w )->setBackgroundMode( container->backgroundMode() );
    }
}

WidgetAction::~WidgetAction()
{
    if ( MainWindow::self )
	MainWindow::self->toolActions.removeRef( this );
}

void WidgetAction::init( const QString &g )
{
    MainWindow::self->toolActions.append( this );
    grp = g;
}
