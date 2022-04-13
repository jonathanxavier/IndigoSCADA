/****************************************************************************
** Form implementation generated from reading ui file 'login_db.ui'
**
** Created: Sat Apr 2 13:44:36 2022
**      by: The User Interface Compiler ($Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "login_db.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a LoginDatabase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
LoginDatabase::LoginDatabase( QWidget* parent, const char* name, bool modal, WFlags fl, bool is_realtimedb)
    : QDialog( parent, name, modal, fl )
{
	IsRealtimedb = is_realtimedb;
    if ( !name )
	setName( "LoginDatabase" );

    textLabel1 = new QLabel( this, "textLabel1" );
    textLabel1->setGeometry( QRect( 20, 50, 49, 20 ) );

    textLabel2 = new QLabel( this, "textLabel2" );
    textLabel2->setGeometry( QRect( 20, 100, 60, 20 ) );

    pushButton1 = new QPushButton( this, "pushButton1" );
    pushButton1->setGeometry( QRect( 50, 190, 82, 26 ) );

    pushButton2 = new QPushButton( this, "pushButton2" );
    pushButton2->setGeometry( QRect( 170, 190, 82, 26 ) );

    lineEdit2 = new QLineEdit( this, "lineEdit2" );
    lineEdit2->setGeometry( QRect( 90, 100, 160, 22 ) );
    lineEdit2->setFrameShape( QLineEdit::LineEditPanel );
    lineEdit2->setFrameShadow( QLineEdit::Sunken );

    lineEdit1 = new QLineEdit( this, "lineEdit1" );
    lineEdit1->setGeometry( QRect( 90, 50, 160, 22 ) );
    languageChange();
    resize( QSize(300, 233).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

	// signals and slots connections
    connect( pushButton1, SIGNAL( clicked() ), this, SLOT( OkClicked() ) );
    connect( pushButton2, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
LoginDatabase::~LoginDatabase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void LoginDatabase::languageChange()
{
    setCaption( tr( "Set credentials" ) );
    textLabel1->setText( tr( "User" ) );
    textLabel2->setText( tr( "Password" ) );
    pushButton1->setText( tr( "Ok" ) );
    pushButton2->setText( tr( "Cancel" ) );
}

#include "common.h"
#include "inifile.h"
#include "helper_functions.h"
#include "utilities.h"
#define OWCRYPT_SZ 64

void LoginDatabase::OkClicked()
{
    if(IsRealtimedb)
	{
		//set realtimedb.ini
		QString ini_file = GetScadaProjectDirectory() + "\\realtimedb.ini";
		Inifile iniFile((const char*)ini_file);
		
		QString user;
		QString password;
		user = lineEdit1->text();
		password = lineEdit2->text();
		char db_password[OWCRYPT_SZ];
		strcpy(db_password, (const char*)password);

		char encrypted[OWCRYPT_SZ];
        my_owcrypt(db_password, "saltsalt$1", encrypted);
		
		#ifdef WIN32

		char section[50];
		char key[50];
		strcpy(section,"rtsqlserver");
		strcpy(key, "user");

		WritePrivateProfileString(section, key, (const char*)user, ini_file);

		strcpy(key, "password");

		WritePrivateProfileString(section, key, encrypted, ini_file);

		#endif

		QString msg = tr("User has changed real time database credentials: ") + user + QString(" ") + QString(encrypted);
		QSLogEvent("HMI",msg);

		accept();
	
	}
	else
	{
		//set historicdb.ini
		QString ini_file = GetScadaProjectDirectory() + "\\historicdb.ini";
		Inifile iniFile((const char*)ini_file);
		QString user;
		QString password;
		user = lineEdit1->text();
		password = lineEdit2->text();
		char db_password[OWCRYPT_SZ];
		strcpy(db_password, (const char*)password);

		char encrypted[OWCRYPT_SZ];
        my_owcrypt(db_password, "saltsalt$1", encrypted);
		
		#ifdef WIN32

		char section[50];
		char key[50];
		strcpy(section,"sqlserver");
		strcpy(key, "user");

		WritePrivateProfileString(section, key, (const char*)user, ini_file);

		strcpy(key, "password");

		WritePrivateProfileString(section, key, encrypted, ini_file);

		#endif

		QString msg = tr("User has changed historical database credentials: ") + user + QString(" ") + QString(encrypted);
		QSLogEvent("HMI",msg);

		accept();
	}
}
