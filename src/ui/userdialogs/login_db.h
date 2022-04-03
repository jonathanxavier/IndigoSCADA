/****************************************************************************
** Form interface generated from reading ui file 'login_db.ui'
**
** Created: Sat Apr 2 13:44:32 2022
**      by: The User Interface Compiler ($Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef LOGINDATABASE_H
#define LOGINDATABASE_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QPushButton;
class QLineEdit;

class LoginDatabase : public QDialog
{
    Q_OBJECT

public:
    LoginDatabase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0,  bool is_realtimedb = 0);
    ~LoginDatabase();

    QLabel* textLabel1;
    QLabel* textLabel2;
    QPushButton* pushButton1;
    QPushButton* pushButton2;
    QLineEdit* lineEdit2;
    QLineEdit* lineEdit1;
	bool IsRelatimedb;

public slots:
    virtual void OkClicked();

protected:

protected slots:
    virtual void languageChange();

};

#endif // LOGINDATABASE_H
