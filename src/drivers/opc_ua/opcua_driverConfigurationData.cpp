/****************************************************************************
** Form implementation generated from reading ui file 'opcua_driver_configuration.ui'
**
** Created: lun 7. apr 17:09:40 2014
**      by: The User Interface Compiler ($Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "opcua_driverConfigurationData.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a Opcua_driverConfigurationData as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
Opcua_driverConfigurationData::Opcua_driverConfigurationData( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "Form1" );

    textLabel1 = new QLabel( this, "textLabel1" );
    textLabel1->setGeometry( QRect( 40, 90, 49, 20 ) );

    OPCUAServerIPAddressText = new QLineEdit( this, "OPCUAServerIPAddressText" );
    OPCUAServerIPAddressText->setGeometry( QRect( 170, 130, 200, 20 ) );

    Name = new QLineEdit( this, "Name" );
    Name->setEnabled( FALSE );
    Name->setGeometry( QRect( 170, 90, 200, 20 ) );
    Name->setFrameShape( QLineEdit::LineEditPanel );
    Name->setFrameShadow( QLineEdit::Sunken );

    textLabel3 = new QLabel( this, "textLabel3" );
    textLabel3->setGeometry( QRect( 40, 130, 120, 20 ) );

    HelpButton = new QPushButton( this, "HelpButton" );
    HelpButton->setGeometry( QRect( 170, 250, 82, 26 ) );

    CancelButton = new QPushButton( this, "CancelButton" );
    CancelButton->setGeometry( QRect( 290, 250, 82, 26 ) );

    OkButton = new QPushButton( this, "OkButton" );
    OkButton->setGeometry( QRect( 50, 250, 82, 26 ) );
 
    PollInterval = new QSpinBox( this, "PollInterval" );
    PollInterval->setGeometry( QRect( 230, 210, 100, 20 ) );
    PollInterval->setMaxValue( 60000 );

    NItems = new QSpinBox( this, "NItems" );
    NItems->setGeometry( QRect( 230, 170, 100, 20 ) );
    NItems->setMaxValue( 1000 );

    textLabel5 = new QLabel( this, "textLabel5" );
    textLabel5->setGeometry( QRect( 40, 170, 49, 20 ) );

    textLabel6 = new QLabel( this, "textLabel6" );
    textLabel6->setGeometry( QRect( 40, 210, 110, 20 ) );

    languageChange();
    resize( QSize(429, 313).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( OkButton, SIGNAL( clicked() ), this, SLOT( OkClicked() ) );
    connect( CancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( HelpButton, SIGNAL( clicked() ), this, SLOT( Help() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
Opcua_driverConfigurationData::~Opcua_driverConfigurationData()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void Opcua_driverConfigurationData::languageChange()
{
    setCaption( tr( "Form1" ) );
    textLabel1->setText( tr( "Name" ) );
    textLabel3->setText( tr( "OPC UA server URL" ) );
    HelpButton->setText( tr( "Help" ) );
    CancelButton->setText( tr( "Cancel" ) );
    OkButton->setText( tr( "Ok" ) );
    textLabel5->setText( tr( "N Items" ) );
    textLabel6->setText( tr( "Poll interval (ms)" ) );
}

void Opcua_driverConfigurationData::Help()
{
    qWarning( "Opcua_driverConfigurationData::Help(): Not implemented yet" );
}

void Opcua_driverConfigurationData::OkClicked()
{
    qWarning( "Opcua_driverConfigurationData::OkClicked(): Not implemented yet" );
}

