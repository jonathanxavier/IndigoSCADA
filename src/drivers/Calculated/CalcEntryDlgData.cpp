/**********************************************************************
--- Qt Architect generated file ---
File: CalcEntryDlgData.cpp
Last generated: Thu Jan 4 16:11:04 2001
DO NOT EDIT!!!  This file will be automatically
regenerated by qtarch.  All changes will be lost.
*********************************************************************/
#include <qt.h>
#include "CalcEntryDlgData.h"

CalcEntryDlgData::CalcEntryDlgData(QWidget *parent, const char *name)
: QDialog(parent, name, TRUE, 0)
{
	Expression = new QLineEdit(this, "LineEdit_1");
	Expression->setGeometry(110, 30, 430, 30);
	Expression->setMinimumSize(0, 0);
	Expression->setMaximumSize(32767, 32767);
	Expression->setFocusPolicy(QWidget::StrongFocus);
	Expression->setBackgroundMode(QWidget::PaletteBase);
	#if QT_VERSION < 300
	Expression->setFontPropagation(QWidget::SameFont);
	Expression->setPalettePropagation(QWidget::SameFont);
	#endif
	Expression->setText( "" );
	Expression->setMaxLength( 32767 );
	Expression->setFrame( QLineEdit::Normal );
	Expression->setFrame( TRUE );
	Expression->setAlignment( AlignLeft );
	QPushButton *qtarch_PushButton_5 = new QPushButton(this, "PushButton_5");
	qtarch_PushButton_5->setGeometry(10, 80, 100, 30);
	qtarch_PushButton_5->setMinimumSize(0, 0);
	qtarch_PushButton_5->setMaximumSize(32767, 32767);
	qtarch_PushButton_5->setFocusPolicy(QWidget::TabFocus);
	qtarch_PushButton_5->setBackgroundMode(QWidget::PaletteButton);
	#if QT_VERSION < 300
	qtarch_PushButton_5->setFontPropagation(QWidget::SameFont);
	qtarch_PushButton_5->setPalettePropagation(QWidget::SameFont);
	#endif
	qtarch_PushButton_5->setText( tr( "Ok" ) );
	qtarch_PushButton_5->setAutoRepeat( FALSE );
	qtarch_PushButton_5->setAutoResize( FALSE );
	qtarch_PushButton_5->setToggleButton( FALSE );
	qtarch_PushButton_5->setDefault( FALSE );
	qtarch_PushButton_5->setAutoDefault( FALSE );
	qtarch_PushButton_5->setIsMenuButton( FALSE );
	connect(qtarch_PushButton_5, SIGNAL(clicked()), SLOT(OkClicked()));
	QPushButton *qtarch_PushButton_6 = new QPushButton(this, "PushButton_6");
	qtarch_PushButton_6->setGeometry(440, 80, 100, 30);
	qtarch_PushButton_6->setMinimumSize(0, 0);
	qtarch_PushButton_6->setMaximumSize(32767, 32767);
	qtarch_PushButton_6->setFocusPolicy(QWidget::TabFocus);
	qtarch_PushButton_6->setBackgroundMode(QWidget::PaletteButton);
	#if QT_VERSION < 300
	qtarch_PushButton_6->setFontPropagation(QWidget::SameFont);
	qtarch_PushButton_6->setPalettePropagation(QWidget::SameFont);
	#endif
	qtarch_PushButton_6->setText( tr( "Cancel" ) );
	qtarch_PushButton_6->setAutoRepeat( FALSE );
	qtarch_PushButton_6->setAutoResize( FALSE );
	qtarch_PushButton_6->setToggleButton( FALSE );
	qtarch_PushButton_6->setDefault( FALSE );
	qtarch_PushButton_6->setAutoDefault( FALSE );
	qtarch_PushButton_6->setIsMenuButton( FALSE );
	connect(qtarch_PushButton_6, SIGNAL(clicked()), SLOT(reject()));
	QLabel *qtarch_Label_6 = new QLabel(this, "Label_6");
	qtarch_Label_6->setGeometry(10, 30, 90, 30);
	qtarch_Label_6->setMinimumSize(0, 0);
	qtarch_Label_6->setMaximumSize(32767, 32767);
	qtarch_Label_6->setFocusPolicy(QWidget::NoFocus);
	qtarch_Label_6->setBackgroundMode(QWidget::PaletteBackground);
	#if QT_VERSION < 300
	qtarch_Label_6->setFontPropagation(QWidget::SameFont);
	qtarch_Label_6->setPalettePropagation(QWidget::SameFont);
	#endif
	qtarch_Label_6->setFrameStyle( 0 );
	qtarch_Label_6->setLineWidth( 1 );
	qtarch_Label_6->setMidLineWidth( 0 );
	qtarch_Label_6->QFrame::setMargin( 0 );
	qtarch_Label_6->setText( tr( "Expression" ) );
	qtarch_Label_6->setAlignment( AlignLeft|AlignVCenter|ExpandTabs );
	qtarch_Label_6->setMargin( 0 );
	resize(560,120);
	setMinimumSize(0, 0);
	setMaximumSize(32767, 32767);
}
CalcEntryDlgData::~CalcEntryDlgData()
{
}
