/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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

#ifndef QCATEGORYWIDGET_H
#define QCATEGORYWIDGET_H

#include <qwidget.h>
#include <qlayout.h>
#include <qptrdict.h>

class QCategoryButton;
class QWidgetList;

class QCategoryWidget : public QWidget
{
    Q_OBJECT

public:
    QCategoryWidget( QWidget *parent = 0, const char *name = 0 );
    ~QCategoryWidget();

    void addCategory( const QString &name, QWidget *page );

private slots:
    void buttonClicked();

private:
    void updateTabs();

private:
    QPtrDict<QWidget> pages;
    QWidgetList *buttons;
    QVBoxLayout *layout;
    QWidget *currentPage;
    QCategoryButton *lastTab;

};

#endif
