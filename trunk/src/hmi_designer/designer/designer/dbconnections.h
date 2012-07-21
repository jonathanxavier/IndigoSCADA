/****************************************************************************
** Form interface generated from reading ui file 'dbconnections.ui'
**
** Created: Thu Jul 16 09:45:48 2009
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.2   edited Dec 19 11:45 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef DATABASECONNECTIONBASE_H
#define DATABASECONNECTIONBASE_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPushButton;
class QListBox;
class QListBoxItem;
class QGroupBox;

class DatabaseConnectionBase : public QDialog
{
    Q_OBJECT

public:
    DatabaseConnectionBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~DatabaseConnectionBase();

    QPushButton* buttonNew;
    QListBox* listConnections;
    QPushButton* buttonDelete;
    QPushButton* buttonHelp;
    QPushButton* buttonClose;
    QGroupBox* grp;
    QPushButton* buttonConnect;

protected:
    QGridLayout* DatabaseConnectionBaseLayout;
    QHBoxLayout* Layout5;
    QGridLayout* grpLayout;
    QHBoxLayout* Layout4;

protected slots:
    virtual void languageChange();

    virtual void init();
    virtual void destroy();
    virtual void currentConnectionChanged( const QString & );
    virtual void deleteConnection();
    virtual void newConnection();
    virtual void connectionNameChanged( const QString & );
    virtual void doConnect();


};

#endif // DATABASECONNECTIONBASE_H
