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

#ifndef TABLEEDITORIMPL_H
#define TABLEEDITORIMPL_H

#include "tableeditor.h"
#include <qmap.h>

class QListBoxItem;
class QTable;
class FormWindow;

class TableEditor : public TableEditorBase
{
    Q_OBJECT

public:
    TableEditor( QWidget* parent = 0, QWidget *editWidget = 0, FormWindow *fw = 0,
		 const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~TableEditor();

protected slots:
    void columnDownClicked();
    void columnTextChanged( const QString & );
    void columnUpClicked();
    void currentColumnChanged( QListBoxItem * );
    void currentFieldChanged( const QString & );
    void currentRowChanged( QListBoxItem * );
    void deleteColumnClicked();
    void deleteRowClicked();
    void newColumnClicked();
    void newRowClicked();
    void okClicked();
    void rowDownClicked();
    void rowTextChanged( const QString & );
    void rowUpClicked();
    void applyClicked();
    void chooseRowPixmapClicked();
    void deleteRowPixmapClicked();
    void chooseColPixmapClicked();
    void deleteColPixmapClicked();

private:
    void readFromTable();
    void readColumns();
    void readRows();
    void saveFieldMap();
    void restoreFieldMap();

private:
    QTable *editTable;
    FormWindow *formWindow;
    QMap<int, QString> fieldMap;
    QMap<QListBoxItem*, QString> tmpFieldMap;

};

#endif // TABLEEDITOR_H
