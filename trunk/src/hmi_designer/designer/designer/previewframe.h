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

#ifndef PREVIEWFRAME_H
#define PREVIEWFRAME_H

#include <qvbox.h>
#include <qworkspace.h>

#include "previewwidgetimpl.h"

class PreviewWorkspace : public QWorkspace
{
    Q_OBJECT
public:
    PreviewWorkspace( QWidget* parent = 0, const char* name = 0 )
        : QWorkspace( parent, name ) {}
    ~PreviewWorkspace() {}

protected:
    void paintEvent( QPaintEvent* );
};

class PreviewFrame : public QVBox
{
    Q_OBJECT

public:
    PreviewFrame( QWidget *parent = 0, const char *name = 0 );
    void setPreviewPalette(QPalette);

private:
    PreviewWidget 	*previewWidget;
};

#endif
