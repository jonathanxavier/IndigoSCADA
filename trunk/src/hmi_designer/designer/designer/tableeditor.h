/****************************************************************************
** Form interface generated from reading ui file 'tableeditor.ui'
**
** Created: Thu Jul 16 09:45:48 2009
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.2   edited Dec 19 11:45 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef TABLEEDITORBASE_H
#define TABLEEDITORBASE_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QTable;
class QPushButton;
class QTabWidget;
class QWidget;
class QListBox;
class QListBoxItem;
class QLabel;
class QLineEdit;
class QComboBox;

class TableEditorBase : public QDialog
{
    Q_OBJECT

public:
    TableEditorBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~TableEditorBase();

    QTable* table;
    QPushButton* buttonHelp;
    QPushButton* buttonApply;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QTabWidget* TabWidget;
    QWidget* columns_tab;
    QListBox* listColumns;
    QPushButton* buttonColumnUp;
    QPushButton* buttonColumnDown;
    QPushButton* buttonDeleteColumn;
    QPushButton* buttonNewColumn;
    QLabel* labelTable;
    QLabel* labelColumnPixmap;
    QPushButton* buttonDeleteColPixmap;
    QPushButton* buttonChooseColPixmap;
    QLabel* TextLabel2;
    QLabel* Label2;
    QLabel* labelFields;
    QLineEdit* editColumnText;
    QComboBox* comboFields;
    QLabel* labelTableValue;
    QWidget* rows_tab;
    QPushButton* buttonRowUp;
    QListBox* listRows;
    QPushButton* buttonRowDown;
    QPushButton* buttonNewRow;
    QPushButton* buttonDeleteRow;
    QLabel* TextLabel2_3;
    QLabel* Label2_2;
    QLabel* labelRowPixmap;
    QPushButton* buttonDeleteRowPixmap;
    QPushButton* buttonChooseRowPixmap;
    QLineEdit* editRowText;

protected:
    QGridLayout* TableEditorBaseLayout;
    QHBoxLayout* Layout11;
    QGridLayout* columns_tabLayout;
    QGridLayout* Layout6;
    QHBoxLayout* Layout2;
    QGridLayout* rows_tabLayout;
    QGridLayout* Layout7_2;
    QHBoxLayout* Layout2_2;

protected slots:
    virtual void languageChange();

    virtual void init();
    virtual void destroy();
    virtual void applyClicked();
    virtual void chooseRowPixmapClicked();
    virtual void columnTextChanged( const QString & );
    virtual void columnUpClicked();
    virtual void currentColumnChanged( QListBoxItem * );
    virtual void currentFieldChanged( const QString & );
    virtual void currentRowChanged( QListBoxItem * );
    virtual void deleteColPixmapClicked();
    virtual void deleteColumnClicked();
    virtual void deleteRowClicked();
    virtual void deleteRowPixmapClicked();
    virtual void newColumnClicked();
    virtual void newRowClicked();
    virtual void chooseColPixmapClicked();
    virtual void okClicked();
    virtual void rowDownClicked();
    virtual void rowTextChanged( const QString & );
    virtual void rowUpClicked();
    virtual void columnDownClicked();


};

#endif // TABLEEDITORBASE_H
