/****************************************************************************
** Form implementation generated from reading ui file 'dbconnections.ui'
**
** Created: Thu Jul 16 09:45:49 2009
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.2   edited Dec 19 11:45 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "dbconnections.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a DatabaseConnectionBase as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DatabaseConnectionBase::DatabaseConnectionBase( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "DatabaseConnectionBase" );
    setSizeGripEnabled( TRUE );
    DatabaseConnectionBaseLayout = new QGridLayout( this, 1, 1, 11, 6, "DatabaseConnectionBaseLayout"); 

    buttonNew = new QPushButton( this, "buttonNew" );

    DatabaseConnectionBaseLayout->addWidget( buttonNew, 0, 1 );

    listConnections = new QListBox( this, "listConnections" );

    DatabaseConnectionBaseLayout->addMultiCellWidget( listConnections, 0, 4, 0, 0 );

    buttonDelete = new QPushButton( this, "buttonDelete" );

    DatabaseConnectionBaseLayout->addWidget( buttonDelete, 1, 1 );

    Layout5 = new QHBoxLayout( 0, 0, 6, "Layout5"); 

    buttonHelp = new QPushButton( this, "buttonHelp" );
    buttonHelp->setAutoDefault( TRUE );
    Layout5->addWidget( buttonHelp );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout5->addItem( spacer );

    buttonClose = new QPushButton( this, "buttonClose" );
    buttonClose->setAutoDefault( TRUE );
    Layout5->addWidget( buttonClose );

    DatabaseConnectionBaseLayout->addMultiCellLayout( Layout5, 5, 5, 0, 2 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    DatabaseConnectionBaseLayout->addMultiCell( spacer_2, 2, 4, 1, 1 );

    grp = new QGroupBox( this, "grp" );
    grp->setColumnLayout(0, Qt::Vertical );
    grp->layout()->setSpacing( 6 );
    grp->layout()->setMargin( 11 );
    grpLayout = new QGridLayout( grp->layout() );
    grpLayout->setAlignment( Qt::AlignTop );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    grpLayout->addItem( spacer_3, 0, 0 );

    DatabaseConnectionBaseLayout->addMultiCellWidget( grp, 0, 2, 2, 2 );

    Layout4 = new QHBoxLayout( 0, 0, 6, "Layout4"); 
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout4->addItem( spacer_4 );

    buttonConnect = new QPushButton( this, "buttonConnect" );
    buttonConnect->setEnabled( FALSE );
    Layout4->addWidget( buttonConnect );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout4->addItem( spacer_5 );

    DatabaseConnectionBaseLayout->addLayout( Layout4, 3, 2 );
    QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    DatabaseConnectionBaseLayout->addItem( spacer_6, 4, 2 );
    languageChange();
    resize( QSize(512, 309).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonClose, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonNew, SIGNAL( clicked() ), this, SLOT( newConnection() ) );
    connect( buttonDelete, SIGNAL( clicked() ), this, SLOT( deleteConnection() ) );
    connect( buttonConnect, SIGNAL( clicked() ), this, SLOT( doConnect() ) );
    connect( listConnections, SIGNAL( highlighted(const QString&) ), this, SLOT( currentConnectionChanged(const QString&) ) );
    connect( listConnections, SIGNAL( selected(const QString&) ), this, SLOT( currentConnectionChanged(const QString&) ) );

    // tab order
    setTabOrder( buttonClose, listConnections );
    setTabOrder( listConnections, buttonNew );
    setTabOrder( buttonNew, buttonDelete );
    setTabOrder( buttonDelete, buttonConnect );
    setTabOrder( buttonConnect, buttonHelp );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
DatabaseConnectionBase::~DatabaseConnectionBase()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void DatabaseConnectionBase::languageChange()
{
    setCaption( tr( "Edit Database Connections" ) );
    buttonNew->setText( tr( "&New Connection" ) );
    buttonDelete->setText( tr( "&Delete Connection" ) );
    buttonHelp->setText( tr( "&Help" ) );
    buttonClose->setText( tr( "&Close" ) );
    QWhatsThis::add( buttonClose, tr( "Close the dialog and discard any changes." ) );
    grp->setTitle( tr( "Connection" ) );
    buttonConnect->setText( tr( "Connec&t" ) );
}

void DatabaseConnectionBase::init()
{
}

void DatabaseConnectionBase::destroy()
{
}

void DatabaseConnectionBase::currentConnectionChanged(const QString&)
{
    qWarning( "DatabaseConnectionBase::currentConnectionChanged(const QString&): Not implemented yet" );
}

void DatabaseConnectionBase::deleteConnection()
{
    qWarning( "DatabaseConnectionBase::deleteConnection(): Not implemented yet" );
}

void DatabaseConnectionBase::newConnection()
{
    qWarning( "DatabaseConnectionBase::newConnection(): Not implemented yet" );
}

void DatabaseConnectionBase::connectionNameChanged(const QString&)
{
    qWarning( "DatabaseConnectionBase::connectionNameChanged(const QString&): Not implemented yet" );
}

void DatabaseConnectionBase::doConnect()
{
    qWarning( "DatabaseConnectionBase::doConnect(): Not implemented yet" );
}

