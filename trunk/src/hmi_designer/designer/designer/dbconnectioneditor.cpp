/****************************************************************************
** Form implementation generated from reading ui file 'dbconnectioneditor.ui'
**
** Created: Thu Jul 16 09:45:50 2009
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.2   edited Dec 19 11:45 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "dbconnectioneditor.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a DatabaseConnectionEditorBase as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DatabaseConnectionEditorBase::DatabaseConnectionEditorBase( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "DatabaseConnectionEditorBase" );
    DatabaseConnectionEditorBaseLayout = new QGridLayout( this, 1, 1, 11, 6, "DatabaseConnectionEditorBaseLayout"); 

    PushButton2 = new QPushButton( this, "PushButton2" );

    DatabaseConnectionEditorBaseLayout->addWidget( PushButton2, 1, 2 );

    PushButton1 = new QPushButton( this, "PushButton1" );
    PushButton1->setDefault( TRUE );

    DatabaseConnectionEditorBaseLayout->addWidget( PushButton1, 1, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    DatabaseConnectionEditorBaseLayout->addItem( spacer, 1, 0 );

    grp = new QGroupBox( this, "grp" );
    grp->setColumnLayout(0, Qt::Vertical );
    grp->layout()->setSpacing( 6 );
    grp->layout()->setMargin( 11 );
    grpLayout = new QGridLayout( grp->layout() );
    grpLayout->setAlignment( Qt::AlignTop );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    grpLayout->addItem( spacer_2, 0, 0 );

    DatabaseConnectionEditorBaseLayout->addMultiCellWidget( grp, 0, 0, 0, 2 );
    languageChange();
    resize( QSize(393, 286).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( PushButton2, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( PushButton1, SIGNAL( clicked() ), this, SLOT( accept() ) );

    // tab order
    setTabOrder( PushButton1, PushButton2 );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
DatabaseConnectionEditorBase::~DatabaseConnectionEditorBase()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void DatabaseConnectionEditorBase::languageChange()
{
    setCaption( tr( "Connect" ) );
    PushButton2->setText( tr( "&Cancel" ) );
    PushButton1->setText( tr( "&OK" ) );
    grp->setTitle( tr( "Connection Details" ) );
}

void DatabaseConnectionEditorBase::init()
{
}

void DatabaseConnectionEditorBase::destroy()
{
}

