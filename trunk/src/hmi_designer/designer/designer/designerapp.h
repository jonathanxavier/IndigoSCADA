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

#ifndef DESIGNERAPP_H
#define DESIGNERAPP_H

class QLabel;


#include <qapplication.h>

class DesignerApplication : public QApplication
{
public:
    const char *className() const { return "DesignerApplication"; }

    DesignerApplication( int &argc, char **argv );

    QLabel *showSplash();
    static void closeSplash();

    static QString settingsKey();
    static QString oldSettingsKey();
    static void setSettingsKey( const QString &key );

protected:
    QDateTime lastMod;

#if defined(Q_WS_WIN)
    bool winEventFilter( MSG *msg );
    uint DESIGNER_OPENFILE;
#endif

};


#endif
