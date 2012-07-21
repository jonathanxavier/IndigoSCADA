/****************************************************************************
** Form implementation generated from reading ui file 'dbconnection.ui'
**
** Created: Thu Jul 16 09:45:50 2009
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.2   edited Dec 19 11:45 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "dbconnection.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a DatabaseConnectionWidget as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
DatabaseConnectionWidget::DatabaseConnectionWidget( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "DatabaseConnectionWidget" );
    DatabaseConnectionWidgetLayout = new QGridLayout( this, 1, 1, 0, 6, "DatabaseConnectionWidgetLayout"); 

    TextLabel3 = new QLabel( this, "TextLabel3" );

    DatabaseConnectionWidgetLayout->addWidget( TextLabel3, 2, 0 );

    TextLabel4 = new QLabel( this, "TextLabel4" );

    DatabaseConnectionWidgetLayout->addWidget( TextLabel4, 3, 0 );

    TextLabel4_2 = new QLabel( this, "TextLabel4_2" );

    DatabaseConnectionWidgetLayout->addWidget( TextLabel4_2, 4, 0 );

    TextLabel2 = new QLabel( this, "TextLabel2" );

    DatabaseConnectionWidgetLayout->addWidget( TextLabel2, 1, 0 );

    editName = new QLineEdit( this, "editName" );
    editName->setEnabled( FALSE );

    DatabaseConnectionWidgetLayout->addWidget( editName, 0, 1 );

    TextLabel5 = new QLabel( this, "TextLabel5" );

    DatabaseConnectionWidgetLayout->addWidget( TextLabel5, 5, 0 );

    comboDriver = new QComboBox( FALSE, this, "comboDriver" );
    comboDriver->setEnabled( FALSE );
    comboDriver->setEditable( TRUE );

    DatabaseConnectionWidgetLayout->addWidget( comboDriver, 1, 1 );

    TextLabel1 = new QLabel( this, "TextLabel1" );

    DatabaseConnectionWidgetLayout->addWidget( TextLabel1, 0, 0 );

    editHostname = new QLineEdit( this, "editHostname" );
    editHostname->setEnabled( FALSE );

    DatabaseConnectionWidgetLayout->addWidget( editHostname, 5, 1 );

    TextLabel1_2 = new QLabel( this, "TextLabel1_2" );

    DatabaseConnectionWidgetLayout->addWidget( TextLabel1_2, 6, 0 );

    editUsername = new QLineEdit( this, "editUsername" );
    editUsername->setEnabled( FALSE );

    DatabaseConnectionWidgetLayout->addWidget( editUsername, 3, 1 );

    editPassword = new QLineEdit( this, "editPassword" );
    editPassword->setEnabled( FALSE );
    editPassword->setEchoMode( QLineEdit::Password );

    DatabaseConnectionWidgetLayout->addWidget( editPassword, 4, 1 );

    editDatabase = new QLineEdit( this, "editDatabase" );
    editDatabase->setEnabled( FALSE );

    DatabaseConnectionWidgetLayout->addWidget( editDatabase, 2, 1 );

    editPort = new QSpinBox( this, "editPort" );
    editPort->setEnabled( FALSE );
    editPort->setMaxValue( 65535 );
    editPort->setMinValue( -1 );
    editPort->setValue( -1 );

    DatabaseConnectionWidgetLayout->addWidget( editPort, 6, 1 );
    languageChange();
    resize( QSize(199, 185).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections

    // tab order
    setTabOrder( editName, comboDriver );
    setTabOrder( comboDriver, editDatabase );
    setTabOrder( editDatabase, editUsername );
    setTabOrder( editUsername, editPassword );
    setTabOrder( editPassword, editHostname );
    setTabOrder( editHostname, editPort );

    // buddies
    TextLabel3->setBuddy( editDatabase );
    TextLabel4->setBuddy( editUsername );
    TextLabel4_2->setBuddy( editPassword );
    TextLabel2->setBuddy( comboDriver );
    TextLabel5->setBuddy( editHostname );
    TextLabel1->setBuddy( editName );
    TextLabel1_2->setBuddy( editPort );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
DatabaseConnectionWidget::~DatabaseConnectionWidget()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void DatabaseConnectionWidget::languageChange()
{
    setCaption( tr( "Edit Database Connection" ) );
    TextLabel3->setText( tr( "&Database Name:" ) );
    TextLabel4->setText( tr( "&Username:" ) );
    TextLabel4_2->setText( tr( "&Password:" ) );
    TextLabel2->setText( tr( "D&river" ) );
    TextLabel5->setText( tr( "&Hostname:" ) );
    TextLabel1->setText( tr( "&Name:" ) );
    TextLabel1_2->setText( tr( "P&ort" ) );
    editPort->setSpecialValueText( tr( "Default" ) );
}

void DatabaseConnectionWidget::init()
{
}

void DatabaseConnectionWidget::destroy()
{
}

