/****************************************************************************
** Form implementation generated from reading ui file 'tableeditor.ui'
**
** Created: Thu Jul 16 09:45:50 2009
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.2   edited Dec 19 11:45 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "tableeditor.h"

#include <qvariant.h>
#include <qtable.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

/* 
 *  Constructs a TableEditorBase as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
TableEditorBase::TableEditorBase( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "TableEditorBase" );
    setSizeGripEnabled( TRUE );
    TableEditorBaseLayout = new QGridLayout( this, 1, 1, 11, 6, "TableEditorBaseLayout"); 

    table = new QTable( this, "table" );
    table->setNumRows( 1 );
    table->setNumCols( 1 );
    table->setReadOnly( TRUE );

    TableEditorBaseLayout->addWidget( table, 0, 0 );

    Layout11 = new QHBoxLayout( 0, 0, 6, "Layout11"); 

    buttonHelp = new QPushButton( this, "buttonHelp" );
    buttonHelp->setAutoDefault( TRUE );
    Layout11->addWidget( buttonHelp );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout11->addItem( spacer );

    buttonApply = new QPushButton( this, "buttonApply" );
    buttonApply->setAutoDefault( TRUE );
    Layout11->addWidget( buttonApply );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    Layout11->addWidget( buttonOk );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    Layout11->addWidget( buttonCancel );

    TableEditorBaseLayout->addMultiCellLayout( Layout11, 1, 1, 0, 1 );

    TabWidget = new QTabWidget( this, "TabWidget" );

    columns_tab = new QWidget( TabWidget, "columns_tab" );
    columns_tabLayout = new QGridLayout( columns_tab, 1, 1, 11, 6, "columns_tabLayout"); 
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    columns_tabLayout->addItem( spacer_2, 4, 2 );

    listColumns = new QListBox( columns_tab, "listColumns" );

    columns_tabLayout->addMultiCellWidget( listColumns, 0, 5, 0, 0 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    columns_tabLayout->addItem( spacer_3, 3, 1 );

    buttonColumnUp = new QPushButton( columns_tab, "buttonColumnUp" );
    buttonColumnUp->setPixmap( QPixmap::fromMimeSource( "s_up.png" ) );

    columns_tabLayout->addWidget( buttonColumnUp, 4, 1 );

    buttonColumnDown = new QPushButton( columns_tab, "buttonColumnDown" );
    buttonColumnDown->setPixmap( QPixmap::fromMimeSource( "s_down.png" ) );

    columns_tabLayout->addWidget( buttonColumnDown, 5, 1 );

    buttonDeleteColumn = new QPushButton( columns_tab, "buttonDeleteColumn" );

    columns_tabLayout->addWidget( buttonDeleteColumn, 1, 1 );

    buttonNewColumn = new QPushButton( columns_tab, "buttonNewColumn" );

    columns_tabLayout->addWidget( buttonNewColumn, 0, 1 );

    Layout6 = new QGridLayout( 0, 1, 1, 0, 6, "Layout6"); 

    labelTable = new QLabel( columns_tab, "labelTable" );

    Layout6->addWidget( labelTable, 0, 0 );

    Layout2 = new QHBoxLayout( 0, 0, 6, "Layout2"); 

    labelColumnPixmap = new QLabel( columns_tab, "labelColumnPixmap" );
    Layout2->addWidget( labelColumnPixmap );

    buttonDeleteColPixmap = new QPushButton( columns_tab, "buttonDeleteColPixmap" );
    buttonDeleteColPixmap->setMaximumSize( QSize( 30, 22 ) );
    buttonDeleteColPixmap->setPixmap( QPixmap::fromMimeSource( "s_editcut.png" ) );
    Layout2->addWidget( buttonDeleteColPixmap );

    buttonChooseColPixmap = new QPushButton( columns_tab, "buttonChooseColPixmap" );
    buttonChooseColPixmap->setMaximumSize( QSize( 30, 22 ) );
    Layout2->addWidget( buttonChooseColPixmap );

    Layout6->addLayout( Layout2, 3, 1 );

    TextLabel2 = new QLabel( columns_tab, "TextLabel2" );

    Layout6->addWidget( TextLabel2, 2, 0 );

    Label2 = new QLabel( columns_tab, "Label2" );

    Layout6->addWidget( Label2, 3, 0 );

    labelFields = new QLabel( columns_tab, "labelFields" );

    Layout6->addWidget( labelFields, 1, 0 );

    editColumnText = new QLineEdit( columns_tab, "editColumnText" );

    Layout6->addWidget( editColumnText, 2, 1 );

    comboFields = new QComboBox( FALSE, columns_tab, "comboFields" );
    comboFields->setEditable( TRUE );

    Layout6->addWidget( comboFields, 1, 1 );

    labelTableValue = new QLabel( columns_tab, "labelTableValue" );

    Layout6->addWidget( labelTableValue, 0, 1 );

    columns_tabLayout->addMultiCellLayout( Layout6, 2, 2, 1, 2 );
    TabWidget->insertTab( columns_tab, QString("") );

    rows_tab = new QWidget( TabWidget, "rows_tab" );
    rows_tabLayout = new QGridLayout( rows_tab, 1, 1, 11, 6, "rows_tabLayout"); 
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    rows_tabLayout->addItem( spacer_4, 4, 2 );

    buttonRowUp = new QPushButton( rows_tab, "buttonRowUp" );
    buttonRowUp->setPixmap( QPixmap::fromMimeSource( "s_up.png" ) );

    rows_tabLayout->addWidget( buttonRowUp, 4, 1 );

    listRows = new QListBox( rows_tab, "listRows" );

    rows_tabLayout->addMultiCellWidget( listRows, 0, 5, 0, 0 );

    buttonRowDown = new QPushButton( rows_tab, "buttonRowDown" );
    buttonRowDown->setPixmap( QPixmap::fromMimeSource( "s_down.png" ) );

    rows_tabLayout->addWidget( buttonRowDown, 5, 1 );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    rows_tabLayout->addItem( spacer_5, 3, 1 );

    buttonNewRow = new QPushButton( rows_tab, "buttonNewRow" );

    rows_tabLayout->addWidget( buttonNewRow, 0, 1 );

    buttonDeleteRow = new QPushButton( rows_tab, "buttonDeleteRow" );

    rows_tabLayout->addWidget( buttonDeleteRow, 1, 1 );

    Layout7_2 = new QGridLayout( 0, 1, 1, 0, 6, "Layout7_2"); 

    TextLabel2_3 = new QLabel( rows_tab, "TextLabel2_3" );

    Layout7_2->addWidget( TextLabel2_3, 0, 0 );

    Label2_2 = new QLabel( rows_tab, "Label2_2" );

    Layout7_2->addWidget( Label2_2, 1, 0 );

    Layout2_2 = new QHBoxLayout( 0, 0, 6, "Layout2_2"); 

    labelRowPixmap = new QLabel( rows_tab, "labelRowPixmap" );
    Layout2_2->addWidget( labelRowPixmap );

    buttonDeleteRowPixmap = new QPushButton( rows_tab, "buttonDeleteRowPixmap" );
    buttonDeleteRowPixmap->setMaximumSize( QSize( 30, 22 ) );
    buttonDeleteRowPixmap->setPixmap( QPixmap::fromMimeSource( "s_editcut.png" ) );
    Layout2_2->addWidget( buttonDeleteRowPixmap );

    buttonChooseRowPixmap = new QPushButton( rows_tab, "buttonChooseRowPixmap" );
    buttonChooseRowPixmap->setMaximumSize( QSize( 30, 22 ) );
    Layout2_2->addWidget( buttonChooseRowPixmap );

    Layout7_2->addLayout( Layout2_2, 1, 1 );

    editRowText = new QLineEdit( rows_tab, "editRowText" );

    Layout7_2->addWidget( editRowText, 0, 1 );

    rows_tabLayout->addMultiCellLayout( Layout7_2, 2, 2, 1, 2 );
    TabWidget->insertTab( rows_tab, QString("") );

    TableEditorBaseLayout->addWidget( TabWidget, 0, 1 );
    languageChange();
    resize( QSize(484, 406).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonApply, SIGNAL( clicked() ), this, SLOT( applyClicked() ) );
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( okClicked() ) );
    connect( listColumns, SIGNAL( currentChanged(QListBoxItem*) ), this, SLOT( currentColumnChanged(QListBoxItem*) ) );
    connect( listColumns, SIGNAL( selectionChanged(QListBoxItem*) ), this, SLOT( currentColumnChanged(QListBoxItem*) ) );
    connect( editColumnText, SIGNAL( textChanged(const QString&) ), this, SLOT( columnTextChanged(const QString&) ) );
    connect( buttonNewColumn, SIGNAL( clicked() ), this, SLOT( newColumnClicked() ) );
    connect( buttonDeleteColumn, SIGNAL( clicked() ), this, SLOT( deleteColumnClicked() ) );
    connect( buttonColumnUp, SIGNAL( clicked() ), this, SLOT( columnUpClicked() ) );
    connect( buttonColumnDown, SIGNAL( clicked() ), this, SLOT( columnDownClicked() ) );
    connect( comboFields, SIGNAL( activated(const QString&) ), this, SLOT( currentFieldChanged(const QString&) ) );
    connect( listRows, SIGNAL( selectionChanged(QListBoxItem*) ), this, SLOT( currentRowChanged(QListBoxItem*) ) );
    connect( listRows, SIGNAL( currentChanged(QListBoxItem*) ), this, SLOT( currentRowChanged(QListBoxItem*) ) );
    connect( buttonNewRow, SIGNAL( clicked() ), this, SLOT( newRowClicked() ) );
    connect( buttonDeleteRow, SIGNAL( clicked() ), this, SLOT( deleteRowClicked() ) );
    connect( buttonRowUp, SIGNAL( clicked() ), this, SLOT( rowUpClicked() ) );
    connect( buttonRowDown, SIGNAL( clicked() ), this, SLOT( rowDownClicked() ) );
    connect( buttonChooseRowPixmap, SIGNAL( clicked() ), this, SLOT( chooseRowPixmapClicked() ) );
    connect( buttonDeleteColPixmap, SIGNAL( clicked() ), this, SLOT( deleteColPixmapClicked() ) );
    connect( editRowText, SIGNAL( textChanged(const QString&) ), this, SLOT( rowTextChanged(const QString&) ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( buttonChooseColPixmap, SIGNAL( clicked() ), this, SLOT( chooseColPixmapClicked() ) );
    connect( buttonDeleteRowPixmap, SIGNAL( clicked() ), this, SLOT( deleteRowPixmapClicked() ) );

    // tab order
    setTabOrder( buttonOk, buttonCancel );
    setTabOrder( buttonCancel, table );
    setTabOrder( table, TabWidget );
    setTabOrder( TabWidget, listColumns );
    setTabOrder( listColumns, buttonNewColumn );
    setTabOrder( buttonNewColumn, buttonDeleteColumn );
    setTabOrder( buttonDeleteColumn, comboFields );
    setTabOrder( comboFields, editColumnText );
    setTabOrder( editColumnText, buttonDeleteColPixmap );
    setTabOrder( buttonDeleteColPixmap, buttonChooseColPixmap );
    setTabOrder( buttonChooseColPixmap, buttonColumnUp );
    setTabOrder( buttonColumnUp, buttonColumnDown );
    setTabOrder( buttonColumnDown, buttonApply );
    setTabOrder( buttonApply, listRows );
    setTabOrder( listRows, buttonNewRow );
    setTabOrder( buttonNewRow, buttonDeleteRow );
    setTabOrder( buttonDeleteRow, editRowText );
    setTabOrder( editRowText, buttonDeleteRowPixmap );
    setTabOrder( buttonDeleteRowPixmap, buttonChooseRowPixmap );
    setTabOrder( buttonChooseRowPixmap, buttonRowUp );
    setTabOrder( buttonRowUp, buttonRowDown );
    setTabOrder( buttonRowDown, buttonHelp );

    // buddies
    TextLabel2->setBuddy( editColumnText );
    labelFields->setBuddy( comboFields );
    TextLabel2_3->setBuddy( editRowText );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
TableEditorBase::~TableEditorBase()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void TableEditorBase::languageChange()
{
    setCaption( tr( "Edit Table" ) );
    buttonHelp->setText( tr( "&Help" ) );
    buttonApply->setText( tr( "&Apply" ) );
    QWhatsThis::add( buttonApply, tr( "Apply all changes." ) );
    buttonOk->setText( tr( "&OK" ) );
    QWhatsThis::add( buttonOk, tr( "Close the dialog and apply all the changes." ) );
    buttonCancel->setText( tr( "&Cancel" ) );
    QWhatsThis::add( buttonCancel, tr( "Close the dialog and discard any changes." ) );
    buttonColumnUp->setText( QString::null );
    QToolTip::add( buttonColumnUp, tr( "Move up" ) );
    QWhatsThis::add( buttonColumnUp, tr( "<b>Move the selected item up.</b><p>The top-most column will be the first column of the list.</p>" ) );
    buttonColumnDown->setText( QString::null );
    QToolTip::add( buttonColumnDown, tr( "Move down" ) );
    QWhatsThis::add( buttonColumnDown, tr( "<b>Move the selected item down.</b><p>The top-most column will be the first column of the list.</p>" ) );
    buttonDeleteColumn->setText( tr( "&Delete Column" ) );
    buttonNewColumn->setText( tr( "&New Column" ) );
    labelTable->setText( tr( "Table:" ) );
    labelColumnPixmap->setText( tr( "Label4" ) );
    buttonDeleteColPixmap->setText( QString::null );
    QToolTip::add( buttonDeleteColPixmap, tr( "Delete Pixmap" ) );
    QWhatsThis::add( buttonDeleteColPixmap, tr( "<b>Delete the selected item's pixmap.</b><p>The pixmap in the current column of the selected item will be deleted.</p>" ) );
    buttonChooseColPixmap->setText( tr( "..." ) );
    QToolTip::add( buttonChooseColPixmap, tr( "Select a Pixmap" ) );
    QWhatsThis::add( buttonChooseColPixmap, tr( "<b>Select a pixmap file for the item.</b><p>The pixmap will be changed in the current column of the selected item.</p>" ) );
    TextLabel2->setText( tr( "&Label:" ) );
    Label2->setText( tr( "Pixmap:" ) );
    labelFields->setText( tr( "&Field:" ) );
    labelTableValue->setText( tr( "<no table>" ) );
    TabWidget->changeTab( columns_tab, tr( "Co&lumns" ) );
    buttonRowUp->setText( QString::null );
    QToolTip::add( buttonRowUp, tr( "Move up" ) );
    QWhatsThis::add( buttonRowUp, tr( "<b>Move the selected item up.</b><p>The top-most column will be the first column of the list.</p>" ) );
    buttonRowDown->setText( QString::null );
    QToolTip::add( buttonRowDown, tr( "Move down" ) );
    QWhatsThis::add( buttonRowDown, tr( "<b>Move the selected item down.</b><p>The top-most column will be the first column of the list.</p>" ) );
    buttonNewRow->setText( tr( "&New Row" ) );
    buttonDeleteRow->setText( tr( "&Delete Row" ) );
    TextLabel2_3->setText( tr( "&Label:" ) );
    Label2_2->setText( tr( "Pixmap:" ) );
    labelRowPixmap->setText( tr( "Label4" ) );
    buttonDeleteRowPixmap->setText( QString::null );
    QToolTip::add( buttonDeleteRowPixmap, tr( "Delete Pixmap" ) );
    QWhatsThis::add( buttonDeleteRowPixmap, tr( "<b>Delete the selected item's pixmap.</b><p>The pixmap in the current column of the selected item will be deleted.</p>" ) );
    buttonChooseRowPixmap->setText( tr( "..." ) );
    QToolTip::add( buttonChooseRowPixmap, tr( "Select a Pixmap" ) );
    QWhatsThis::add( buttonChooseRowPixmap, tr( "<b>Select a pixmap file for the item.</b><p>The pixmap will be changed in the current column of the selected item.</p>" ) );
    TabWidget->changeTab( rows_tab, tr( "&Rows" ) );
}

void TableEditorBase::init()
{
}

void TableEditorBase::destroy()
{
}

void TableEditorBase::applyClicked()
{
    qWarning( "TableEditorBase::applyClicked(): Not implemented yet" );
}

void TableEditorBase::chooseRowPixmapClicked()
{
    qWarning( "TableEditorBase::chooseRowPixmapClicked(): Not implemented yet" );
}

void TableEditorBase::columnTextChanged(const QString&)
{
    qWarning( "TableEditorBase::columnTextChanged(const QString&): Not implemented yet" );
}

void TableEditorBase::columnUpClicked()
{
    qWarning( "TableEditorBase::columnUpClicked(): Not implemented yet" );
}

void TableEditorBase::currentColumnChanged(QListBoxItem*)
{
    qWarning( "TableEditorBase::currentColumnChanged(QListBoxItem*): Not implemented yet" );
}

void TableEditorBase::currentFieldChanged(const QString&)
{
    qWarning( "TableEditorBase::currentFieldChanged(const QString&): Not implemented yet" );
}

void TableEditorBase::currentRowChanged(QListBoxItem*)
{
    qWarning( "TableEditorBase::currentRowChanged(QListBoxItem*): Not implemented yet" );
}

void TableEditorBase::deleteColPixmapClicked()
{
    qWarning( "TableEditorBase::deleteColPixmapClicked(): Not implemented yet" );
}

void TableEditorBase::deleteColumnClicked()
{
    qWarning( "TableEditorBase::deleteColumnClicked(): Not implemented yet" );
}

void TableEditorBase::deleteRowClicked()
{
    qWarning( "TableEditorBase::deleteRowClicked(): Not implemented yet" );
}

void TableEditorBase::deleteRowPixmapClicked()
{
    qWarning( "TableEditorBase::deleteRowPixmapClicked(): Not implemented yet" );
}

void TableEditorBase::newColumnClicked()
{
    qWarning( "TableEditorBase::newColumnClicked(): Not implemented yet" );
}

void TableEditorBase::newRowClicked()
{
    qWarning( "TableEditorBase::newRowClicked(): Not implemented yet" );
}

void TableEditorBase::chooseColPixmapClicked()
{
    qWarning( "TableEditorBase::chooseColPixmapClicked(): Not implemented yet" );
}

void TableEditorBase::okClicked()
{
    qWarning( "TableEditorBase::okClicked(): Not implemented yet" );
}

void TableEditorBase::rowDownClicked()
{
    qWarning( "TableEditorBase::rowDownClicked(): Not implemented yet" );
}

void TableEditorBase::rowTextChanged(const QString&)
{
    qWarning( "TableEditorBase::rowTextChanged(const QString&): Not implemented yet" );
}

void TableEditorBase::rowUpClicked()
{
    qWarning( "TableEditorBase::rowUpClicked(): Not implemented yet" );
}

void TableEditorBase::columnDownClicked()
{
    qWarning( "TableEditorBase::columnDownClicked(): Not implemented yet" );
}

