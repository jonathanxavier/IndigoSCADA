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

#ifndef DBCONNECTIONSIMPL_H
#define DBCONNECTIONSIMPL_H

#include "dbconnections.h"
#include "dbconnection.h"

class Project;

class DatabaseConnectionsEditor : public DatabaseConnectionBase
{
    Q_OBJECT

public:
    DatabaseConnectionsEditor( Project *pro, QWidget* parent = 0,
			       const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~DatabaseConnectionsEditor();

protected slots:
    void deleteConnection();
    void newConnection();
    void doConnect();
    void currentConnectionChanged( const QString & );
    void connectionNameChanged( const QString &s );

private:
    void enableAll( bool b );

private:
    Project *project;
    DatabaseConnectionWidget* connectionWidget;

};

#endif // DBCONNECTIONSIMPL_H
