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

#ifndef STARTDIALOGIMPL_H
#define STARTDIALOGIMPL_H

#include <qiconview.h>
#include <qlistview.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qfiledialog.h>
#include <qmap.h>

#include "newformimpl.h"
#include "startdialog.h"

class FileDialog : public QFileDialog
{
    Q_OBJECT

public:
    FileDialog( QWidget *parent );

protected slots:
    void accept();

signals:
    void fileSelected();
};


class StartDialog : public StartDialogBase
{
    Q_OBJECT

public:
    StartDialog( QWidget *parent, const QString &templatePath );
    void setRecentlyFiles( QStringList& );
    void setRecentlyProjects( QStringList& );
    bool showDialogInFuture() const { return showInFuture; }

protected slots:
    void recentItemChanged( QIconViewItem *item );
    void clearFileInfo();
    void accept();
    void reject();

private:
    void initFileOpen();
    void insertRecentItems( QStringList &files, bool isProject );
    NewForm *newForm;
    FileDialog *fd;
    QMap<int, QString> recentFiles;
    bool showInFuture;
};

#endif
