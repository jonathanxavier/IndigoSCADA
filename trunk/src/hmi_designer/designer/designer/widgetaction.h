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

#ifndef WIDGETACTION_H
#define WIDGETACTION_H

#include <qaction.h>

class WidgetAction : public QAction
{
    Q_OBJECT

public:
    WidgetAction( const QString &grp, QObject* parent,
		  const char* name = 0, bool toggle = FALSE  )
	: QAction( parent, name, toggle ) { init( grp ); }
    WidgetAction( const QString &grp, const QString& text,
		  const QIconSet& icon, const QString& menuText, QKeySequence accel,
		  QObject* parent, const char* name = 0, bool toggle = FALSE )
	: QAction( text, icon, menuText, accel, parent, name, toggle ) { init( grp ); }
    WidgetAction( const QString &grp, const QString& text,
		  const QString& menuText, QKeySequence accel, QObject* parent,
		  const char* name = 0, bool toggle = FALSE )
	: QAction( text, menuText, accel, parent, name, toggle ) { init( grp ); }
    ~WidgetAction();

#if !defined(Q_NO_USING_KEYWORD)
    using QAction::addedTo;
#endif
    void addedTo( QWidget *, QWidget * );

    QString group() const { return grp; }

private:
    void init( const QString &g );

private:
    QString grp;

};

#endif
