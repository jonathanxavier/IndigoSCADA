/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qvariant.h> // HP-UX compiler need this here
#include "widgetfactory.h"
#include <widgetdatabase.h>
#include "metadatabase.h"
#include "mainwindow.h"
#include "formwindow.h"
#include "layout.h"
#include "listboxeditorimpl.h"
#include "listvieweditorimpl.h"
#include "iconvieweditorimpl.h"
#include "formwindow.h"
#include "multilineeditorimpl.h"
#include "../interfaces/widgetinterface.h"
#ifndef QT_NO_TABLE
#include "tableeditorimpl.h"
#endif
#include "project.h"

#include <qfeatures.h>

#include <qpixmap.h>
#include <qgroupbox.h>
#include <qiconview.h>
#ifndef QT_NO_TABLE
#include <qtable.h>
#endif
#ifndef QT_NO_SQL
#include <qdatatable.h>
#endif
#include <qdatetimeedit.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qmultilineedit.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qcombobox.h>
#include <qtabbar.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qobjectlist.h>
#include <qlcdnumber.h>
#include <qslider.h>
#include <qdial.h>
#include <qprogressbar.h>
#include <qtextview.h>
#include <qtextbrowser.h>
#include <qframe.h>
#include <qmetaobject.h>
#include <qwidgetstack.h>
#include <qwizard.h>
#include <qvaluelist.h>
#include <qtimer.h>
#include <qscrollbar.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qapplication.h>
#include <qsplitter.h>
#ifndef QT_NO_SQL
#include "database.h"
#endif

#define NO_STATIC_COLORS
#include <globaldefs.h>
#include <qobject.h>

FormWindow *find_formwindow( QWidget *w )
{
    if ( !w )
	return 0;
    for (;;) {
	if ( w->inherits( "FormWindow" ) )
	    return (FormWindow*)w;
	if ( !w->parentWidget() )
	    return 0;
	w = w->parentWidget();
    }
}

void QLayoutWidget::paintEvent( QPaintEvent* )
{
    QPainter p ( this );
    p.setPen( red );
    p.drawRect( rect() );
}


QDesignerTabWidget::QDesignerTabWidget( QWidget *parent, const char *name )
    : QTabWidget( parent, name ), dropIndicator( 0 ), dragPage( 0 ), mousePressed( FALSE )
{
    tabBar()->setAcceptDrops( TRUE );
    tabBar()->installEventFilter( this );
}

int QDesignerTabWidget::currentPage() const
{
    return tabBar()->currentTab();
}

void QDesignerTabWidget::setCurrentPage( int i )
{
    tabBar()->setCurrentTab( i );
}

QString QDesignerTabWidget::pageTitle() const
{
    return ((QTabWidget*)this)->tabLabel( QTabWidget::currentPage() );
}

void QDesignerTabWidget::setPageTitle( const QString& title )
{
    changeTab( QTabWidget::currentPage(), title );
}

void QDesignerTabWidget::setPageName( const QCString& name )
{
    if ( QTabWidget::currentPage() )
	QTabWidget::currentPage()->setName( name );
}

QCString QDesignerTabWidget::pageName() const
{
    if ( !QTabWidget::currentPage() )
	return 0;
    return QTabWidget::currentPage()->name();
}

int QDesignerTabWidget::count() const
{
    return tabBar()->count();
}

bool QDesignerTabWidget::eventFilter( QObject *o, QEvent *e )
{
    if ( o != tabBar() ) return FALSE;

    switch ( e->type() ) {
    case QEvent::MouseButtonPress: {
	mousePressed = TRUE;
	QMouseEvent *me = (QMouseEvent*)e;
	pressPoint = me->pos();
    }
    break;
    case QEvent::MouseMove: {
	QMouseEvent *me = (QMouseEvent*)e;
	if ( mousePressed && ( pressPoint - me->pos()).manhattanLength() > QApplication::startDragDistance() ) {
	    QTextDrag *drg = new QTextDrag( QString::number( (long) this ) , this );
	    mousePressed = FALSE;
	    dragPage = QTabWidget::currentPage();
	    dragLabel = QTabWidget::tabLabel( dragPage );

	    int index = indexOf( dragPage );

	    removePage( dragPage );
	    if ( !drg->dragMove() ) {
		insertTab( dragPage, dragLabel, index );
		showPage( dragPage );
	    }
	    if ( dropIndicator )
		dropIndicator->hide();
	}
    }
    break;
    case QEvent::DragLeave:	{
	if ( dropIndicator )
	    dropIndicator->hide();
    }
    break;
    case QEvent::DragMove: {
	QDragEnterEvent *de = (QDragEnterEvent*) e;
	if ( QTextDrag::canDecode( de ) ) {
	    QString text;
	    QTextDrag::decode( de, text );
	    if ( text == QString::number( (long)this ) )
		de->accept();
	    else
		return FALSE;
	}

	int index = 0;
	QRect rect;
	for ( ; index < tabBar()->count(); index++ ) {
	    if ( tabBar()->tabAt( index )->rect().contains( de->pos() ) ) {
		rect = tabBar()->tabAt( index )->rect();
		break;
	    }
	}

	if ( index == tabBar()->count() -1 ) {
	    QRect rect2 = rect;
	    rect2.setLeft( rect2.left() + rect2.width() / 2 );
	    if ( rect2.contains( de->pos() ) )
		index++;
	}

	if ( ! dropIndicator ) {
	    dropIndicator = new QWidget( this );
	    dropIndicator->setBackgroundColor( red );
	}

	QPoint pos;
	if ( index == tabBar()->count() )
	    pos = tabBar()->mapToParent( QPoint( rect.x() + rect.width(), rect.y() ) );
	else
	    pos = tabBar()->mapToParent( QPoint( rect.x(), rect.y() ) );

	dropIndicator->setGeometry( pos.x(), pos.y() , 3, rect.height() );
	dropIndicator->show();
    }
    break;
    case QEvent::Drop: {
	QDragEnterEvent *de = (QDragEnterEvent*) e;
	if ( QTextDrag::canDecode( de ) ) {
	    QString text;
	    QTextDrag::decode( de, text );
	    if ( text == QString::number( (long)this ) ) {

		int newIndex = 0;
		for ( ; newIndex < tabBar()->count(); newIndex++ ) {
		    if ( tabBar()->tabAt( newIndex )->rect().contains( de->pos() ) )
			break;
		}

		if ( newIndex == tabBar()->count() -1 ) {
		    QRect rect2 = tabBar()->tabAt( newIndex )->rect();
		    rect2.setLeft( rect2.left() + rect2.width() / 2 );
		    if ( rect2.contains( de->pos() ) )
			newIndex++;
		}

		int oldIndex = 0;
		for ( ; oldIndex < tabBar()->count(); oldIndex++ ) {
		    if ( tabBar()->tabAt( oldIndex )->rect().contains( pressPoint ) )
			break;
		}

		FormWindow *fw = find_formwindow( this );
		MoveTabPageCommand *cmd =
		    new MoveTabPageCommand( tr( "Move Tab Page" ), fw, this,
					    dragPage, dragLabel, newIndex, oldIndex );
		fw->commandHistory()->addCommand( cmd );
		cmd->execute();
		de->accept();
	    }
	}
    }
    break;
    default:
	break;
    }
    return FALSE;
}


QDesignerWidgetStack::QDesignerWidgetStack( QWidget *parent, const char *name )
    : QWidgetStack( parent, name )
{
    prev = new QToolButton( Qt::LeftArrow, this, "designer_wizardstack_button" );
    prev->setAutoRaise( TRUE );
    prev->setAutoRepeat( TRUE );
    prev->setSizePolicy( QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored ) );
    next = new QToolButton( Qt::RightArrow, this, "designer_wizardstack_button" );
    next->setAutoRaise( TRUE );
    next->setAutoRepeat( TRUE );
    next->setSizePolicy( QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored ) );
    connect( prev, SIGNAL( clicked() ), this, SLOT( prevPage() ) );
    connect( next, SIGNAL( clicked() ), this, SLOT( nextPage() ) );
    updateButtons();
}

void QDesignerWidgetStack::updateButtons()
{
    prev->setGeometry( width() - 31, 1, 15, 15 );
    next->setGeometry( width() - 16, 1, 15, 15 );
    prev->show();
    next->show();
    prev->raise();
    next->raise();
}

void QDesignerWidgetStack::prevPage()
{
    setCurrentPage( currentPage() - 1 );
}

void QDesignerWidgetStack::nextPage()
{
    setCurrentPage( currentPage() + 1 );
}

int QDesignerWidgetStack::currentPage() const
{
    QDesignerWidgetStack* that = (QDesignerWidgetStack*) this;
    return that->pages.find( visibleWidget() );
}

void QDesignerWidgetStack::setCurrentPage( int i )
{
    // help next/prev page commands
    if ( i < 0 )
	i += count();
    if ( i >= count() )
	i -= count();

    if ( i < 0 || i >= count() )
	return;
    raiseWidget( pages.at( i ) );
    updateButtons();
}

QCString QDesignerWidgetStack::pageName() const
{
    if ( !visibleWidget() )
	return 0;
    return visibleWidget()->name();
}

void QDesignerWidgetStack::setPageName( const QCString& name )
{
    if ( visibleWidget() )
	visibleWidget()->setName( name );
}

int QDesignerWidgetStack::count() const
{
    return pages.count();
}

QWidget* QDesignerWidgetStack::page( int i ) const
{
    if ( i < 0 || i >= count() )
	return 0;
    QDesignerWidgetStack* that = (QDesignerWidgetStack*) this;
    return that->pages.at( i );
}


int QDesignerWidgetStack::insertPage( QWidget *p, int i )
{
    if ( i < 0 )
	pages.append( p );
    else
	pages.insert( (uint) i, p );
    addWidget( p );
    p->show();
    raiseWidget( p );
    QApplication::sendPostedEvents();
    updateButtons();
    return pages.find( p );
}

int QDesignerWidgetStack::removePage( QWidget *p )
{
    int i = pages.find( p );
    pages.remove( p );
    removeWidget( p );
    setCurrentPage( 0 );
    updateButtons();
    return i;
}


int QDesignerWizard::currentPageNum() const
{
    for ( int i = 0; i < pageCount(); ++i ) {
	if ( page( i ) == currentPage() )
	    return i;
    }
    return 0;
}





void QDesignerWizard::setCurrentPage( int i )
{
    if ( i < currentPageNum() ) {
	while ( i < currentPageNum() ) {
	    if ( currentPageNum() == 0 )
		break;
	    back();
	}

    } else {
	while ( i > currentPageNum() ) {
	    if ( currentPageNum() == pageCount() - 1 )
		break;
	    next();
	}
    }
}

QString QDesignerWizard::pageTitle() const
{
    return title( currentPage() );
}

void QDesignerWizard::setPageTitle( const QString& title )
{
    setTitle( currentPage(), title );
}

void QDesignerWizard::setPageName( const QCString& name )
{
    if ( QWizard::currentPage() )
	QWizard::currentPage()->setName( name );
}

QCString QDesignerWizard::pageName() const
{
    if ( !QWizard::currentPage() )
	return 0;
    return QWizard::currentPage()->name();
}

int QDesignerWizard::pageNum( QWidget *p )
{
    for ( int i = 0; i < pageCount(); ++i ) {
	if ( page( i ) == p )
	    return i;
    }
    return -1;
}

void QDesignerWizard::addPage( QWidget *p, const QString &t )
{
    QWizard::addPage( p, t );
    if ( removedPages.find( p ) )
	removedPages.remove( p );
}

void QDesignerWizard::removePage( QWidget *p )
{
    QWizard::removePage( p );
    removedPages.insert( p, p );
}

void QDesignerWizard::insertPage( QWidget *p, const QString &t, int index )
{
    QWizard::insertPage( p, t, index );
    if ( removedPages.find( p ) )
	removedPages.remove( p );
}

QMap< int, QMap< QString, QVariant> > *defaultProperties = 0;
QMap< int, QStringList > *changedProperties = 0;

/*!
  \class WidgetFactory widgetfactory.h
  \brief Set of static functions for creating widgets, layouts and do other stuff

  The widget factory offers functions to create widgets, create and
  delete layouts find out other details - all based on the
  WidgetDatabase's data. So the functions that use ids use the same
  ids as in the WidgetDatabase.
*/


void WidgetFactory::saveDefaultProperties( QObject *w, int id )
{
    QMap< QString, QVariant> propMap;
    QStrList lst = w->metaObject()->propertyNames( TRUE );
    for ( uint i = 0; i < lst.count(); ++i ) {
	QVariant var = w->property( lst.at( i ) );
	if ( !var.isValid() && qstrcmp( "pixmap", lst.at( i ) ) == 0 )
	    var = QVariant( QPixmap() );
	else if ( !var.isValid() && qstrcmp( "iconSet", lst.at( i ) ) == 0 )
	    var = QVariant( QIconSet() );
	propMap.replace( lst.at( i ), var );
    }
    defaultProperties->replace( id, propMap );
}

void WidgetFactory::saveChangedProperties( QObject *w, int id )
{
    QStringList l = MetaDataBase::changedProperties( w );
    changedProperties->insert( id, l );
}

/*!  Creates a widget of the type which is registered as \a id as
  child of \a parent. The \a name is optional. If \a init is TRUE, the
  widget is initialized with some defaults, else the plain widget is
  created.
*/

QWidget *WidgetFactory::create( int id, QWidget *parent, const char *name, bool init, const QRect *r, Qt::Orientation orient )
{
    QString n = WidgetDatabase::className( id );
    if ( n.isEmpty() )
	return 0;

    if ( !defaultProperties ) {
	defaultProperties = new QMap< int, QMap< QString, QVariant> >();
	changedProperties = new QMap< int, QStringList >();
    }

    QWidget *w = 0;
    QString str = WidgetDatabase::createWidgetName( id );
    str[0] = str[0].lower();
    const char *s = str.latin1();
    w = createWidget( n, parent, name ? name : s, init, r, orient );
    if ( w && w->inherits( "QScrollView" ) )
	( (QScrollView*)w )->disableSizeHintCaching();
    if ( !w && WidgetDatabase::isCustomWidget( id ) )
	w = createCustomWidget( parent, name ? name : s, MetaDataBase::customWidget( id ) );
    if ( !w )
	return 0;
    MetaDataBase::addEntry( w );

    if ( !defaultProperties->contains( id ) )
	saveDefaultProperties( w, id );
    if ( !changedProperties->contains( id ) )
	saveChangedProperties( w, id );

    return w;
}

/*!  Creates a layout on the widget \a widget of the type \a type
  which can be \c HBox, \c VBox or \c Grid.
*/

QLayout *WidgetFactory::createLayout( QWidget *widget, QLayout *layout, LayoutType type )
{
    int spacing = MainWindow::self->currentLayoutDefaultSpacing();
    int margin = 0;

    int metaspacing = MetaDataBase::spacing( widget );
    int metamargin = MetaDataBase::margin( widget );

    if ( widget && !widget->inherits( "QLayoutWidget" ) &&
	 ( WidgetDatabase::isContainer( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( widget ) ) ) ||
	   widget && widget->parentWidget() && widget->parentWidget()->inherits( "FormWindow" ) ) )
	margin = MainWindow::self->currentLayoutDefaultMargin();

    if ( !layout && widget && widget->inherits( "QTabWidget" ) )
	widget = ((QTabWidget*)widget)->currentPage();

    if ( !layout && widget && widget->inherits( "QWizard" ) )
	widget = ((QWizard*)widget)->currentPage();

    if ( !layout && widget && widget->inherits( "QMainWindow" ) )
	widget = ((QMainWindow*)widget)->centralWidget();

    if ( !layout && widget && widget->inherits( "QWidgetStack" ) )
	widget = ((QWidgetStack*)widget)->visibleWidget();

    MetaDataBase::addEntry( widget );

    QLayout *l = 0;
    int align = 0;
    if ( !layout && widget && widget->inherits( "QGroupBox" ) ) {
	QGroupBox *gb = (QGroupBox*)widget;
	gb->setColumnLayout( 0, Qt::Vertical );
	layout = gb->layout();
	layout->setMargin( 0 );
	layout->setSpacing( 0 );
	switch ( type ) {
	case HBox:
	    l = new QHBoxLayout( layout );
	    break;
	case VBox:
	    l = new QVBoxLayout( layout );
	    break;
	case Grid:
	    l = new QDesignerGridLayout( layout );
	    break;
	default:
	    return 0;
	}
	align = Qt::AlignTop;
	MetaDataBase::setMargin( gb, metamargin );
	MetaDataBase::setSpacing( gb, metaspacing );
    } else {
	if ( layout ) {
	    switch ( type ) {
	    case HBox:
		l = new QHBoxLayout( layout );
		break;
	    case VBox:
		l = new QVBoxLayout( layout );
		break;
	    case Grid:
		l = new QDesignerGridLayout( layout );
		break;
	    default:
		return 0;
	    }
	    MetaDataBase::addEntry( l );
	    l->setSpacing( spacing );
	    l->setMargin( margin );
	} else {
	    switch ( type ) {
	    case HBox:
		l = new QHBoxLayout( widget );
		break;
	    case VBox:
		l = new QVBoxLayout( widget );
		break;
	    case Grid:
		l = new QDesignerGridLayout( widget );
		break;
	    default:
		return 0;
	    }
	    MetaDataBase::addEntry( l );
	    if ( widget ) {
		MetaDataBase::setMargin( widget, metamargin );
		MetaDataBase::setSpacing( widget, metaspacing );
	    } else {
		l->setMargin( margin );
		l->setSpacing( spacing );
	    }
	}
    }
    l->setAlignment( align );
    MetaDataBase::addEntry( l );
    return l;
}

void WidgetFactory::deleteLayout( QWidget *widget )
{
    if ( !widget )
	return;

    if ( widget->inherits( "QTabWidget" ) )
	widget = ((QTabWidget*)widget)->currentPage();
    if ( widget->inherits( "QWizard" ) )
	widget = ((QWizard*)widget)->currentPage();
    if ( widget->inherits( "QMainWindow" ) )
	widget = ((QMainWindow*)widget)->centralWidget();
    if ( widget->inherits( "QWidgetStack" ) )
	widget = ((QWidgetStack*)widget)->visibleWidget();
    delete widget->layout();
}

/*!  Factory functions for creating a widget of the type \a className
  as child of \a parent with the name \a name.

  If \a init is TRUE, some initial default properties are set. This
  has to be in sync with the initChangedProperties() function!
*/

QWidget *WidgetFactory::createWidget( const QString &className, QWidget *parent, const char *name, bool init,
				      const QRect *r, Qt::Orientation orient )
{
    if ( className == "QPushButton" ) {
	QPushButton *b = 0;
	if ( init ) {
	    b = new QDesignerPushButton( parent, name );
	    b->setText( QString::fromLatin1( name ) );
	} else {
	    b = new QDesignerPushButton( parent, name );
	}
	QWidget *w = find_formwindow( b );
	b->setAutoDefault( w && ( (FormWindow*)w )->mainContainer()->inherits( "QDialog" ) );
	return b;
    } else if ( className == "QToolButton" ) {
	if ( init ) {
	    QDesignerToolButton *tb = new QDesignerToolButton( parent, name );
	    tb->setText( "..." );
	    return tb;
	}
	return new QDesignerToolButton( parent, name );
    } else if ( className == "QCheckBox" ) {
	if ( init ) {
	    QDesignerCheckBox *cb = new QDesignerCheckBox( parent, name );
	    cb->setText( QString::fromLatin1( name ) );
	    return cb;
	}
	return new QDesignerCheckBox( parent, name );
    } else if ( className == "QRadioButton" ) {
	if ( init ) {
	    QDesignerRadioButton *rb = new QDesignerRadioButton( parent, name );
	    rb->setText( QString::fromLatin1( name ) );
	    return rb;
	}
	return new QDesignerRadioButton( parent, name );
    } else if ( className == "QGroupBox" ) {
	if ( init )
	    return new QGroupBox( QString::fromLatin1( name ), parent, name );
	return new QGroupBox( parent, name );
    } else if ( className == "QButtonGroup" ) {
	if ( init )
	    return new QButtonGroup( QString::fromLatin1( name ), parent, name );
	return new QButtonGroup( parent, name );
    } else if ( className == "QIconView" ) {
#if !defined(QT_NO_ICONVIEW)
	QIconView* iv = new QIconView( parent, name );
	if ( init )
	    (void) new QIconViewItem( iv, MainWindow::tr( "New Item" ) );
	return iv;
#else
	return 0;
#endif
    } else if ( className == "QTable" ) {
#if !defined(QT_NO_TABLE)
	if ( init )
	    return new QTable( 3, 3, parent, name );
	return new QTable( parent, name );
#else
	return 0;
#endif
#ifndef QT_NO_SQL
    } else if ( className == "QDataTable" ) {
	return new QDataTable( parent, name );
#endif //QT_NO_SQL
    } else if ( className == "QDateEdit" ) {
	return new QDateEdit( parent, name );
    } else if ( className == "QTimeEdit" ) {
	return new QTimeEdit( parent, name );
    } else if ( className == "QDateTimeEdit" ) {
	return new QDateTimeEdit( parent, name );
    }
    else if ( className == "QListBox" ) {
	QListBox* lb = new QListBox( parent, name );
	if ( init ) {
	    lb->insertItem( MainWindow::tr( "New Item" ) );
	    lb->setCurrentItem( 0 );
	}
	return lb;
    } else if ( className == "QListView" ) {
	QListView *lv = new QListView( parent, name );
	lv->setSorting( -1 );
	if ( init ) {
	    lv->addColumn( MainWindow::tr( "Column 1" ) );
	    lv->setCurrentItem( new QListViewItem( lv, MainWindow::tr( "New Item" ) ) );
	}
	return lv;
    } else if ( className == "QLineEdit" )
	return new QLineEdit( parent, name );
    else if ( className == "QSpinBox" )
	return new QSpinBox( parent, name );
    else if ( className == "QSplitter" )
	return new QSplitter( parent, name );
    else if ( className == "QMultiLineEdit" )
	return new QMultiLineEdit( parent, name );
    else if ( className == "QTextEdit" )
	return new QTextEdit( parent, name );
    else if ( className == "QLabel"  || className == "TextLabel" ) {
	QDesignerLabel *l = new QDesignerLabel( parent, name );
	if ( init ) {
	    l->setText( QString::fromLatin1( name ) );
	    MetaDataBase::addEntry( l );
	    MetaDataBase::setPropertyChanged( l, "text", TRUE );
	}
	return l;
    } else if ( className == "PixmapLabel" ) {
	QDesignerLabel *l = new QDesignerLabel( parent, name );
	if ( init ) {
	    l->setPixmap( QPixmap::fromMimeSource( "logo.png" ) );
	    l->setScaledContents( TRUE );
	    MetaDataBase::addEntry( l );
	    MetaDataBase::setPropertyChanged( l, "pixmap", TRUE );
	    MetaDataBase::setPropertyChanged( l, "scaledContents", TRUE );
	}
	return l;
    } else if ( className == "QLayoutWidget" )
	return new QLayoutWidget( parent, name );
    else if ( className == "QTabWidget" ) {
	QTabWidget *tw = new QDesignerTabWidget( parent, name );
	if ( init ) {
	    FormWindow *fw = find_formwindow( parent );
	    QWidget *w = fw ? new QDesignerWidget( fw, tw, "tab" ) : new QWidget( tw, "tab" );
	    tw->addTab( w, MainWindow::tr("Tab 1") );
	    MetaDataBase::addEntry( w );
	    w = fw ? new QDesignerWidget( fw, tw, "tab" ) : new QWidget( tw, "tab" );
	    tw->addTab( w, MainWindow::tr("Tab 2") );
	    MetaDataBase::addEntry( tw );
	    MetaDataBase::addEntry( w );
	}
	return tw;
    } else if ( className == "QWidgetStack" ) {
	QDesignerWidgetStack *ws = new QDesignerWidgetStack( parent, name );
	if ( init ) {
	    FormWindow *fw = find_formwindow( parent );
	    QWidget *w = fw ? new QDesignerWidget( fw, ws, "page" ) : new QWidget( ws, "page" );
	    ws->insertPage( w );
	    MetaDataBase::addEntry( w );
	    MetaDataBase::addEntry( ws );
	}
	return ws;
    } else if ( className == "QComboBox" ) {
	return new QComboBox( FALSE, parent, name );
    } else if ( className == "QWidget" ) {
	if ( parent &&
	     ( parent->inherits( "FormWindow" ) || parent->inherits( "QWizard" ) ||
	       parent->inherits( "QTabWidget" ) || parent->inherits( "QWidgetStack" ) ||
	       parent->inherits( "QMainWindow" ) ) ) {
	    FormWindow *fw = find_formwindow( parent );
	    if ( fw ) {
		QDesignerWidget *dw = new QDesignerWidget( fw, parent, name );
		MetaDataBase::addEntry( dw );
		return dw;
	    }
	}
	return new QWidget( parent, name );
    } else if ( className == "QDialog" ) {
	QDialog *dia = 0;
	if ( parent && parent->inherits( "FormWindow" ) )
	    dia = new QDesignerDialog( (FormWindow*)parent, parent, name );
	else
	    dia = new QDialog( parent, name );
#if defined(QT_NON_COMMERCIAL)
	if ( parent && !parent->inherits("MainWindow") )
#else
	if ( parent )
#endif
	    dia->reparent( parent, QPoint( 0, 0 ), TRUE );
	return dia;
    } else if ( className == "QWizard" ) {
	QWizard *wiz = new QDesignerWizard( parent, name );
#if defined(QT_NON_COMMERCIAL)
	if ( parent && !parent->inherits("MainWindow") )
#else
	if ( parent )
#endif
	    wiz->reparent( parent, QPoint( 0, 0 ), TRUE );
	if ( init && parent && parent->inherits( "FormWindow" ) ) {
	    QDesignerWidget *dw = new QDesignerWidget( (FormWindow*)parent, wiz, "page" );
	    MetaDataBase::addEntry( dw );
	    wiz->addPage( dw, FormWindow::tr( "Page" ) );
	    QTimer::singleShot( 0, wiz, SLOT( next() ) );
	}
	return wiz;
    } else if ( className == "Spacer" ) {
	Spacer *s = new Spacer( parent, name );
	MetaDataBase::addEntry( s );
	MetaDataBase::setPropertyChanged( s, "orientation", TRUE );
	MetaDataBase::setPropertyChanged( s, "sizeType", TRUE );
	if ( !r )
	    return s;
	if ( !r->isValid() || r->width() < 2 && r->height() < 2 )
	    s->setOrientation( orient );
	else if ( r->width() < r->height() )
	    s->setOrientation( Qt::Vertical );
	else
	    s->setOrientation( Qt::Horizontal );
	return s;
    } else if ( className == "QLCDNumber" )
	return new QLCDNumber( parent, name );
    else if ( className == "QProgressBar" )
	return new QProgressBar( parent, name );
    else if ( className == "QTextView" )
	return new QTextView( parent, name );
    else if ( className == "QTextBrowser" )
	return new QTextBrowser( parent, name );
    else if ( className == "QDial" )
	return new QDial( parent, name );
    else if ( className == "QSlider" ) {
	QSlider *s = new QSlider( parent, name );
	if ( !r )
	    return s;
	if ( !r->isValid() || r->width() < 2 && r->height() < 2 )
	    s->setOrientation( orient );
	else if ( r->width() > r->height() )
	    s->setOrientation( Qt::Horizontal );
	MetaDataBase::addEntry( s );
	MetaDataBase::setPropertyChanged( s, "orientation", TRUE );
	return s;
    } else if ( className == "QScrollBar" ) {
	QScrollBar *s = new QScrollBar( parent, name );
	if ( !r )
	    return s;
	if ( !r->isValid() || r->width() < 2 && r->height() < 2 )
	    s->setOrientation( orient );
	else if ( r->width() > r->height() )
	    s->setOrientation( Qt::Horizontal );
	MetaDataBase::addEntry( s );
	MetaDataBase::setPropertyChanged( s, "orientation", TRUE );
	return s;
    } else if ( className == "QFrame" ) {
	if ( !init )
	    return new QFrame( parent, name );
	QFrame *f = new QFrame( parent, name );
	f->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
	return f;
    } else if ( className == "Line" ) {
	Line *l = new Line( parent, name );
	MetaDataBase::addEntry( l );
	MetaDataBase::setPropertyChanged( l, "orientation", TRUE );
	MetaDataBase::setPropertyChanged( l, "frameShadow", TRUE );
	MetaDataBase::setPropertyChanged( l, "frameShape", TRUE );
	if ( !r )
	    return l;
	if ( !r->isValid() || r->width() < 2 && r->height() < 2 )
	    l->setOrientation( orient );
	else if ( r->width() < r->height() )
	    l->setOrientation( Qt::Vertical );
	return l;
    } else if ( className == "QMainWindow" ) {
	QMainWindow *mw = new QMainWindow( parent, name, 0 );
	mw->setDockEnabled( Qt::DockMinimized, FALSE );
	QDesignerWidget *dw = new QDesignerWidget( (FormWindow*)parent, mw, "central widget" );
	mw->setDockMenuEnabled( FALSE );
	MetaDataBase::addEntry( dw );
	mw->setCentralWidget( dw );
	(void)mw->statusBar();
	dw->show();
	return mw;
    }
#ifndef QT_NO_SQL
    else if ( className == "QDataBrowser" ) {
	QWidget *w = new QDesignerDataBrowser( parent, name );
	if ( parent )
	    w->reparent( parent, QPoint( 0, 0 ), TRUE );
	return w;
    } else if ( className == "QDataView" ) {
	QWidget *w = new QDesignerDataView( parent, name );
	if ( parent )
	    w->reparent( parent, QPoint( 0, 0 ), TRUE );
	return w;
    }
#endif

    WidgetInterface *iface = 0;
    widgetManager()->queryInterface( className, &iface );
    if ( !iface )
	return 0;

    QWidget *w = iface->create( className, parent, name );
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
    if ( init && WidgetDatabase::isCustomPluginWidget( WidgetDatabase::idFromClassName( className ) ) ) {
	QWidgetContainerInterfacePrivate *iface2 = 0;
	iface->queryInterface( IID_QWidgetContainer, (QUnknownInterface**)&iface2 );
	if ( iface2 ) {
	    iface2->addPage( className, w, "Page", -1 );
	    iface2->release();
	}
    }
#endif
    iface->release();
    return w;
}



/*!  Find out which type the layout of the widget is. Returns \c HBox,
  \c VBox, \c Grid or \c NoLayout.  \a layout points to this
  QWidget::layout() of \a w or to 0 after the function call.
*/

WidgetFactory::LayoutType WidgetFactory::layoutType( QWidget *w, QLayout *&layout )
{
    layout = 0;

    if ( w && w->inherits( "QTabWidget" ) )
	w = ((QTabWidget*)w)->currentPage();
    if ( w && w->inherits( "QWizard" ) )
	w = ((QWizard*)w)->currentPage();
    if ( w && w->inherits( "QMainWindow" ) )
	w = ((QMainWindow*)w)->centralWidget();
    if ( w && w->inherits( "QWidgetStack" ) )
	w = ((QWidgetStack*)w)->visibleWidget();

    if ( w && w->inherits( "QSplitter" ) )
	return ( (QSplitter*)w )->orientation() == Horizontal ? HBox : VBox;

    if ( !w || !w->layout() )
	return NoLayout;
    QLayout *lay = w->layout();

    if ( w->inherits( "QGroupBox" ) ) {
	QObjectList *l = lay->queryList( "QLayout" );
	if ( l && l->first() )
	    lay = (QLayout*)l->first();
	delete l;
    }
    layout = lay;

    if ( lay->inherits( "QHBoxLayout" ) )
	return HBox;
    else if ( lay->inherits( "QVBoxLayout" ) )
	return VBox;
    else if ( lay->inherits( "QGridLayout" ) )
	return Grid;
    return NoLayout;
}

/*!
  \overload
*/
WidgetFactory::LayoutType WidgetFactory::layoutType( QLayout *layout )
{
    if ( layout->inherits( "QHBoxLayout" ) )
	return HBox;
    else if ( layout->inherits( "QVBoxLayout" ) )
	return VBox;
    else if ( layout->inherits( "QGridLayout" ) )
	return Grid;
    return NoLayout;
}

/*!
  \overload
*/
WidgetFactory::LayoutType WidgetFactory::layoutType( QWidget *w )
{
    QLayout *l = 0;
    return layoutType( w, l );
}


QWidget *WidgetFactory::layoutParent( QLayout *layout )
{
    QObject *o = layout;
    while ( o ) {
	if ( o->isWidgetType() )
	    return (QWidget*)o;
	o = o->parent();
    }
    return 0;
}

/*!  Returns the widget into which children should be inserted when \a
  w is a container known to the designer.

  Usually that is \a w itself, sometimes it is different (e.g. a
  tabwidget is known to the designer as a container but the child
  widgets should be inserted into the current page of the
  tabwidget. So in this case this function returns the current page of
  the tabwidget.)
 */
QWidget* WidgetFactory::containerOfWidget( QWidget *w )
{
    if ( !w )
	return w;
    if ( w->inherits( "QTabWidget" ) )
	return ((QTabWidget*)w)->currentPage();
    if ( w->inherits( "QWizard" ) )
	return ((QWizard*)w)->currentPage();
    if ( w->inherits( "QWidgetStack" ) )
	return ((QWidgetStack*)w)->visibleWidget();
    if ( w->inherits( "QMainWindow" ) )
	return ((QMainWindow*)w)->centralWidget();
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
    if ( !WidgetDatabase::isCustomPluginWidget( WidgetDatabase::idFromClassName( classNameOf( w ) ) ) )
	return w;
    WidgetInterface *iface = 0;
    widgetManager()->queryInterface( classNameOf( w ), &iface );
    if ( !iface )
	return w;
    QWidgetContainerInterfacePrivate *iface2 = 0;
    iface->queryInterface( IID_QWidgetContainer, (QUnknownInterface**)&iface2 );
    if ( !iface2 )
	return w;
    QWidget *c = iface2->containerOfWidget( w->className(), w );
    iface2->release();
    iface->release();
    if ( c )
	return c;
#endif
    return w;
}

/*!  Returns the actual designer widget of the container \a w. This is
  normally \a w itself, but might be a parent or grand parent of \a w
  (e.g. when working with a tabwidget and \a w is the container which
  contains and layouts childs, but the actual widget known to the
  designer is the tabwidget which is the parent of \a w. So this
  function returns the tabwidget then.)
*/

QWidget* WidgetFactory::widgetOfContainer( QWidget *w )
{
    if ( w->parentWidget() && w->parentWidget()->inherits( "QWidgetStack" ) )
	w = w->parentWidget();
    while ( w ) {
	int id = WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) );
	if ( WidgetDatabase::isContainer( id ) ||
	     w && w->parentWidget() && w->parentWidget()->inherits( "FormWindow" ) )
	    return w;
	w = w->parentWidget();
    }
    return w;
}

/*!
  Returns whether \a o is a passive interactor or not.
 */

bool WidgetFactory::lastWasAPassiveInteractor = FALSE;
QGuardedPtr<QObject> *WidgetFactory::lastPassiveInteractor = new QGuardedPtr<QObject>();

bool WidgetFactory::isPassiveInteractor( QObject* o )
{
    if ( lastPassiveInteractor && *lastPassiveInteractor && (QObject*)(*lastPassiveInteractor) == o )
	return lastWasAPassiveInteractor;
    lastWasAPassiveInteractor = FALSE;
    (*lastPassiveInteractor) = o;
    if ( QApplication::activePopupWidget() ) // if a popup is open, we have to make sure that this one is closed, else X might do funny things
	return ( lastWasAPassiveInteractor = TRUE );

    if ( o->inherits( "QTabBar" ) )
	return ( lastWasAPassiveInteractor = TRUE );
    else if ( o->inherits( "QSizeGrip" ) )
	return ( lastWasAPassiveInteractor = TRUE );
    else if ( o->inherits( "QToolButton" ) && o->parent() && o->parent()->inherits( "QTabBar" ) )
	return ( lastWasAPassiveInteractor = TRUE );
    else if ( o->parent() && o->parent()->inherits( "QWizard" ) && o->inherits( "QPushButton" ) )
	return ( lastWasAPassiveInteractor = TRUE );
    else if ( o->parent() && o->parent()->inherits( "QMainWindow" ) && o->inherits( "QMenuBar" ) )
	return ( lastWasAPassiveInteractor = TRUE );
    else if ( o->inherits( "QDockWindowHandle" ) )
	return ( lastWasAPassiveInteractor = TRUE );
    else if ( o->inherits( "QHideDock" ) )
	return ( lastWasAPassiveInteractor = TRUE );
    else if ( qstrcmp( o->name(), "designer_wizardstack_button" ) == 0 )
	return ( lastWasAPassiveInteractor = TRUE );

#ifdef QT_CONTAINER_CUSTOM_WIDGETS
    if ( !o->isWidgetType() )
	return ( lastWasAPassiveInteractor = FALSE );
    WidgetInterface *iface = 0;
    QWidget *w = (QWidget*)o;
    while ( !iface && w && !w->inherits( "FormWindow" ) ) {
	widgetManager()->queryInterface( classNameOf( w ), &iface );
	w = w->parentWidget();
    }
    if ( !iface )
	return ( lastWasAPassiveInteractor = FALSE );
    QWidgetContainerInterfacePrivate *iface2 = 0;
    iface->queryInterface( IID_QWidgetContainer, (QUnknownInterface**)&iface2 );
    if ( !iface2 )
	return ( lastWasAPassiveInteractor = FALSE );
    QWidget *fw = MainWindow::self->isAFormWindowChild( (QWidget*)o );
    if ( !fw )
	return ( lastWasAPassiveInteractor = FALSE );
    QWidget *dw = ( (FormWindow*)fw )->designerWidget( (QWidget*)o );
    if ( !dw )
	return ( lastWasAPassiveInteractor = FALSE );
    lastWasAPassiveInteractor = iface2->isPassiveInteractor( dw->className(), (QWidget*)o );
    iface2->release();
    iface->release();
#endif
    return lastWasAPassiveInteractor;
}


/*!
  Returns the class name of object \a o that should be used for externally (i.e. for saving)
 */
const char* WidgetFactory::classNameOf( QObject* o )
{
    if ( o->inherits( "QDesignerTabWidget" ) )
	return "QTabWidget";
    else if ( o->inherits( "QDesignerWidgetStack" ) )
	return "QWidgetStack";
    else if ( o->inherits( "QWidgetStack" ) )
	return "QWeDoNotWantToBreakTabWidget";
    else if ( o->inherits( "QDesignerDialog" ) )
	return "QDialog";
    else if ( o->inherits( "QDesignerWidget" ) )
	return "QWidget";
    else if ( o->inherits( "CustomWidget" ) )
	return ( (CustomWidget*)o )->realClassName().latin1();
    else if ( o->inherits( "QDesignerLabel" ) )
	return "QLabel";
    else if ( o->inherits( "QDesignerWizard" ) )
	return "QWizard";
    else if ( o->inherits( "QDesignerPushButton" ) )
	return "QPushButton";
    else if ( o->inherits( "QDesignerToolButton" ) )
	return "QToolButton";
    else if ( o->inherits( "QDesignerRadioButton" ) )
	return "QRadioButton";
    else if ( o->inherits( "QDesignerCheckBox" ) )
	return "QCheckBox";
    else if ( o->inherits( "QDesignerMenuBar" ) )
	return "QMenuBar";
    else if ( o->inherits( "QDesignerToolBar" ) )
	return "QToolBar";
    else if ( o->inherits( "QDesignerAction" ) )
	return "QAction";
    else if ( o->inherits( "QDesignerActionGroup" ) )
	return "QActionGroup";
    else if ( o->inherits( "QDesignerPopupMenu" ) )
	return "QPopupMenu";
#ifndef QT_NO_SQL
    else if ( o->inherits( "QDesignerDataBrowser" ) )
	return "QDataBrowser";
    else if ( o->inherits( "QDesignerDataView" ) )
	return "QDataView";
#endif
    return o->className();
}

QString WidgetFactory::defaultSignal( QObject *w )
{
    if ( w->inherits( "QRadioButton" ) || w->inherits( "QCheckBox" ) )
	return "toggled";
    else if ( w->inherits( "QButton" ) || w->inherits( "QButtonGroup" ) )
	return "clicked";
    else if ( w->inherits( "QTextBrowser" ) )
	return "linkClicked";
    else if ( w->inherits( "QLineEdit" ) || w->inherits( "QTextEdit" ) )
	return "textChanged";
    else if ( w->inherits( "QListView" ) || w->inherits( "QIconView" ) ||
	      w->inherits( "QListBox" ) || w->inherits( "QTable" ) )
	return "selectionChanged";
    else if ( w->inherits( "QTabWidget" ) )
	return "selected";
    else if ( w->inherits( "QWidgetStack" ) )
	return "aboutToShow";
    else if ( w->inherits( "QSpinBox" ) || w->inherits( "QSlider" ) ||
	      w->inherits( "QScrollBar" ) || w->inherits( "QDateEdit" ) ||
	      w->inherits( "QTimeEdit" ) || w->inherits( "QDateTimeEdit" ) ||
	      w->inherits( "QDial" ) )
	return "valueChanged";
    else if ( w->inherits( "QComboBox" ) )
	return "activated";
    return QString::null;
}

/*!  As some properties are set by default when creating a widget this
  functions markes this properties as changed. Has to be in sync with
  createWidget()!
*/

void WidgetFactory::initChangedProperties( QObject *o )
{
    if ( MainWindow::self && MainWindow::self->currProject() &&
	 MainWindow::self->currProject()->fakeFormFileFor( o ) )
	return;
    MetaDataBase::setPropertyChanged( o, "name", TRUE );
    if ( !o->inherits( "QDesignerToolBar" ) && !o->inherits( "QDesignerMenuBar" ) )
	MetaDataBase::setPropertyChanged( o, "geometry", TRUE );

    if ( o->inherits( "QPushButton" ) || o->inherits("QRadioButton") || o->inherits( "QCheckBox" ) || o->inherits( "QToolButton" ) )
	MetaDataBase::setPropertyChanged( o, "text", TRUE );
    else if ( o->inherits( "QGroupBox" ) )
	MetaDataBase::setPropertyChanged( o, "title", TRUE );
    else if ( o->isA( "QFrame" ) ) {
	MetaDataBase::setPropertyChanged( o, "frameShadow", TRUE );
	MetaDataBase::setPropertyChanged( o, "frameShape", TRUE );
    } else if ( o->inherits( "QTabWidget" ) || o->inherits( "QWizard" ) ) {
	MetaDataBase::setPropertyChanged( o, "pageTitle", TRUE );
	MetaDataBase::setPropertyChanged( o, "pageName", TRUE );
	MetaDataBase::setPropertyChanged( o, "currentPage", TRUE );
    } else if ( o->inherits( "QWidgetStack" ) ) {
	MetaDataBase::setPropertyChanged( o, "currentPage", TRUE );
	MetaDataBase::setPropertyChanged( o, "pageName", TRUE );
#ifndef QT_NO_TABLE
    } else if ( o->inherits( "QTable" ) && !o->inherits( "QDataTable" ) ) {
	MetaDataBase::setPropertyChanged( o, "numRows", TRUE );
	MetaDataBase::setPropertyChanged( o, "numCols", TRUE );
	QTable *t = (QTable*)o;
	for ( int i = 0; i < 3; ++i ) {
	    t->horizontalHeader()->setLabel( i, QString::number( i + 1 ) );
	    t->verticalHeader()->setLabel( i, QString::number( i + 1 ) );
	}
#endif
    } else if ( o->inherits( "QSplitter" )  ) {
	MetaDataBase::setPropertyChanged( o, "orientation", TRUE );
    } else if ( o->inherits( "QDesignerToolBar" )  ) {
	MetaDataBase::setPropertyChanged( o, "label", TRUE );
    } else if ( o->inherits( "QDesignerMenuBar" )  ) {
	MetaDataBase::setPropertyChanged( o, "itemName", TRUE );
	MetaDataBase::setPropertyChanged( o, "itemNumber", TRUE );
	MetaDataBase::setPropertyChanged( o, "itemText", TRUE );
    }
}

bool WidgetFactory::hasSpecialEditor( int id )
{
    QString className = WidgetDatabase::className( id );

    if ( className.contains( "ListBox" ) )
	return TRUE;
    if ( className.contains( "ComboBox" ) )
	return TRUE;
    if ( className.contains( "ListView" ) )
	return TRUE;
    if ( className.contains( "IconView" ) )
	return TRUE;
    if ( className == "QTextEdit" || className == "QMultiLineEdit" )
	return TRUE;
    if ( className.contains( "Table" ) )
	return TRUE;

    return FALSE;
}

bool WidgetFactory::hasItems( int id )
{
    QString className = WidgetDatabase::className( id );

    if ( className.contains( "ListBox" ) || className.contains( "ListView" ) ||
	 className.contains( "IconView" )  || className.contains( "ComboBox" ) ||
	 className.contains( "Table" ) )
	return TRUE;

    return FALSE;
}

void WidgetFactory::editWidget( int id, QWidget *parent, QWidget *editWidget, FormWindow *fw )
{
    QString className = WidgetDatabase::className( id );

    if ( className.contains( "ListBox" ) ) {
	if ( !editWidget->inherits( "QListBox" ) )
	    return;
	ListBoxEditor *e = new ListBoxEditor( parent, editWidget, fw );
	e->exec();
	delete e;
	return;
    }

    if ( className.contains( "ComboBox" ) ) {
	if ( !editWidget->inherits( "QComboBox" ) )
	    return;
	QComboBox *cb = (QComboBox*)editWidget;
	ListBoxEditor *e = new ListBoxEditor( parent, cb->listBox(), fw );
	e->exec();
	delete e;
	cb->update();
	return;
    }

    if ( className.contains( "ListView" ) ) {
	if ( !editWidget->inherits( "QListView" ) )
	    return;
	QListView *lv = (QListView*)editWidget;
	ListViewEditor *e = new ListViewEditor( parent, lv, fw );
	e->exec();
	delete e;
	return;
    }

    if ( className.contains( "IconView" ) ) {
	if ( !editWidget->inherits( "QIconView" ) )
	    return;
	IconViewEditor *e = new IconViewEditor( parent, editWidget, fw );
	e->exec();
	delete e;
	return;
    }

    if ( className == "QMultiLineEdit" || className == "QTextEdit" ) {
	MultiLineEditor *e = new MultiLineEditor( FALSE, TRUE, parent, editWidget, fw );
	e->exec();
	delete e;
	return;
    }
#ifndef QT_NO_TABLE
    if ( className.contains( "Table" ) ) {
	TableEditor *e = new TableEditor( parent, editWidget, fw );
	e->exec();
	delete e;
	return;
    }
#endif
}

bool WidgetFactory::canResetProperty( QObject *w, const QString &propName )
{
    if ( propName == "name" || propName == "geometry" )
	return FALSE;
    QStringList l = *changedProperties->find( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ) );
    return l.findIndex( propName ) == -1;
}

bool WidgetFactory::resetProperty( QObject *w, const QString &propName )
{
    const QMetaProperty *p = w->metaObject()->property( w->metaObject()->
							findProperty( propName, TRUE ), TRUE );
    if (!p )
	return FALSE;
    return p->reset( w );
}

QVariant WidgetFactory::defaultValue( QObject *w, const QString &propName )
{
    if ( propName == "wordwrap" ) {
	int v = defaultValue( w, "alignment" ).toInt();
	return QVariant( ( v & WordBreak ) == WordBreak, 0 );
    } else if ( propName == "toolTip" || propName == "whatsThis" ) {
	return QVariant( QString::fromLatin1( "" ) );
    } else if ( w->inherits( "CustomWidget" ) ) {
	return QVariant();
    } else if ( propName == "frameworkCode" ) {
	return QVariant( TRUE, 0 );
    } else if ( propName == "layoutMargin" || propName == "layoutSpacing" ) {
	return QVariant( -1 );
    }

    return *( *defaultProperties->find( WidgetDatabase::idFromClassName( classNameOf( w ) ) ) ).find( propName );
}

QString WidgetFactory::defaultCurrentItem( QObject *w, const QString &propName )
{
    const QMetaProperty *p = w->metaObject()->
			     property( w->metaObject()->findProperty( propName, TRUE ), TRUE );
    if ( !p ) {
	int v = defaultValue( w, "alignment" ).toInt();
	if ( propName == "hAlign" ) {
	    if ( ( v & AlignAuto ) == AlignAuto )
		return "AlignAuto";
	    if ( ( v & AlignLeft ) == AlignLeft )
		return "AlignLeft";
	    if ( ( v & AlignCenter ) == AlignCenter || ( v & AlignHCenter ) == AlignHCenter )
		return "AlignHCenter";
	    if ( ( v & AlignRight ) == AlignRight )
		return "AlignRight";
	    if ( ( v & AlignJustify ) == AlignJustify )
		return "AlignJustify";
	} else if ( propName == "vAlign" ) {
	    if ( ( v & AlignTop ) == AlignTop )
		return "AlignTop";
	    if ( ( v & AlignCenter ) == AlignCenter || ( v & AlignVCenter ) == AlignVCenter )
		return "AlignVCenter";
	    if ( ( v & AlignBottom ) == AlignBottom )
		return "AlignBottom";
	}
	return QString::null;

    }
    return p->valueToKey( defaultValue( w, propName ).toInt() );
}

QWidget *WidgetFactory::createCustomWidget( QWidget *parent, const char *name, MetaDataBase::CustomWidget *w )
{
    if ( !w )
	return 0;
    return new CustomWidget( parent, name, w );
}

QVariant WidgetFactory::property( QObject *w, const char *name )
{
    int id = w->metaObject()->findProperty( name, TRUE );
    const QMetaProperty* p = w->metaObject()->property( id, TRUE );
    if ( !p || !p->isValid() )
	return MetaDataBase::fakeProperty( w, name );
    return w->property( name );
}

void QDesignerLabel::updateBuddy()
{

    if ( myBuddy.isEmpty() )
	return;

    QObjectList *l = topLevelWidget()->queryList( "QWidget", myBuddy, FALSE, TRUE );
    if ( !l || !l->first() ) {
	delete l;
	return;
    }

    QLabel::setBuddy( (QWidget*)l->first() );
    delete l;
}


void QDesignerWidget::resizeEvent( QResizeEvent* e)
{
    if ( need_frame ) {
	QPainter p(this);
	p.setPen( backgroundColor() );
	p.drawRect( QRect( QPoint(0,0), e->oldSize() ) );
    }
}

void QDesignerWidget::paintEvent( QPaintEvent *e )
{
    if ( need_frame ) {
	QPainter p(this);
	p.setPen( backgroundColor().dark() );
	p.drawRect( rect() );
    }
    formwindow->paintGrid( this, e );
}

void QDesignerDialog::paintEvent( QPaintEvent *e )
{
    formwindow->paintGrid( this, e );
}

QSizePolicy QLayoutWidget::sizePolicy() const
{
    return sp;
}

bool QLayoutWidget::event( QEvent *e )
{
    if ( e && ( e->type() == QEvent::ChildInserted ||
		e->type() == QEvent::ChildRemoved ||
		e->type() == QEvent::LayoutHint ||
		e->type() == QEvent::Reparent ) )
	updateSizePolicy();
    return QWidget::event( e );
}

/*
  This function must be called on QLayoutWidget creation and whenever
  the QLayoutWidget's parent layout changes (e.g., from a QHBoxLayout
  to a QVBoxLayout), because of the (illogical) way layouting works.
*/
void QLayoutWidget::updateSizePolicy()
{
    if ( !children() || children()->count() == 0 ) {
	sp = QWidget::sizePolicy();
	return;
    }

    /*
      QSizePolicy::MayShrink & friends are private. Here we assume the
      following:

	  Fixed = 0
	  Maximum = MayShrink
	  Minimum = MayGrow
	  Preferred = MayShrink | MayGrow
    */

    int ht = (int) QSizePolicy::Preferred;
    int vt = (int) QSizePolicy::Preferred;

    if ( layout() ) {
	/*
	  parentLayout is set to the parent layout if there is one and if it is
	  top level, in which case layouting is illogical.
	*/
	QLayout *parentLayout = 0;
	if ( parent() && parent()->isWidgetType() ) {
	    parentLayout = ((QWidget *)parent())->layout();
	    if ( parentLayout && parentLayout->mainWidget()->inherits("QLayoutWidget") )
		parentLayout = 0;
	}

	QObjectListIt it( *children() );
	QObject *o;

	if ( layout()->inherits("QVBoxLayout") ) {
	    if ( parentLayout && parentLayout->inherits("QHBoxLayout") )
		vt = QSizePolicy::Minimum;
	    else
		vt = QSizePolicy::Fixed;

	    while ( ( o = it.current() ) ) {
		++it;
		if ( !o->isWidgetType() || ( (QWidget*)o )->testWState( WState_ForceHide ) )
		    continue;
		QWidget *w = (QWidget*)o;

		if ( !w->sizePolicy().mayGrowHorizontally() )
		    ht &= ~QSizePolicy::Minimum;
		if ( !w->sizePolicy().mayShrinkHorizontally() )
		    ht &= ~QSizePolicy::Maximum;
		if ( w->sizePolicy().mayGrowVertically() )
		    vt |= QSizePolicy::Minimum;
		if ( w->sizePolicy().mayShrinkVertically() )
		    vt |= QSizePolicy::Maximum;
	    }
	} else if ( layout()->inherits("QHBoxLayout") ) {
	    if ( parentLayout && parentLayout->inherits("QVBoxLayout") )
		ht = QSizePolicy::Minimum;
	    else
		ht = QSizePolicy::Fixed;

	    while ( ( o = it.current() ) ) {
		++it;
		if ( !o->isWidgetType() || ( (QWidget*)o )->testWState( WState_ForceHide ) )
		    continue;
		QWidget *w = (QWidget*)o;

		if ( w->sizePolicy().mayGrowHorizontally() )
		    ht |= QSizePolicy::Minimum;
		if ( w->sizePolicy().mayShrinkHorizontally() )
		    ht |= QSizePolicy::Maximum;
		if ( !w->sizePolicy().mayGrowVertically() )
		    vt &= ~QSizePolicy::Minimum;
		if ( !w->sizePolicy().mayShrinkVertically() )
		    vt &= ~QSizePolicy::Maximum;
	    }
	} else if ( layout()->inherits("QGridLayout") ) {
	    ht = QSizePolicy::Fixed;
	    vt = QSizePolicy::Fixed;
	    if ( parentLayout ) {
		if ( parentLayout->inherits("QVBoxLayout") )
		    ht = QSizePolicy::Minimum;
		else if ( parentLayout->inherits("QHBoxLayout") )
		    vt = QSizePolicy::Minimum;
	    }

	    while ( ( o = it.current() ) ) {
		++it;
		if ( !o->isWidgetType() || ( (QWidget*)o )->testWState( WState_ForceHide ) )
		    continue;
		QWidget *w = (QWidget*)o;

		if ( w->sizePolicy().mayGrowHorizontally() )
		    ht |= QSizePolicy::Minimum;
		if ( w->sizePolicy().mayShrinkHorizontally() )
		    ht |= QSizePolicy::Maximum;
		if ( w->sizePolicy().mayGrowVertically() )
		    vt |= QSizePolicy::Minimum;
		if ( w->sizePolicy().mayShrinkVertically() )
		    vt |= QSizePolicy::Maximum;
	    }
	}
	if ( layout()->expanding() & QSizePolicy::Horizontally )
	    ht = QSizePolicy::Expanding;
	if ( layout()->expanding() & QSizePolicy::Vertically )
	    vt = QSizePolicy::Expanding;

	layout()->invalidate();
    }

    sp = QSizePolicy( (QSizePolicy::SizeType) ht, (QSizePolicy::SizeType) vt );
    updateGeometry();
}

void CustomWidget::paintEvent( QPaintEvent *e )
{
    if ( parentWidget() && parentWidget()->inherits( "FormWindow" ) ) {
	( (FormWindow*)parentWidget() )->paintGrid( this, e );
    } else {
	QPainter p( this );
	p.fillRect( rect(), colorGroup().dark() );
	p.setPen( colorGroup().light() );
	p.drawText( 2, 2, width() - 4, height() - 4, Qt::AlignAuto | Qt::AlignTop, cusw->className );
	p.drawPixmap( ( width() - cusw->pixmap->width() ) / 2,
		      ( height() - cusw->pixmap->height() ) / 2,
		      *cusw->pixmap );
    }
}


CustomWidgetFactory::CustomWidgetFactory()
{
}

QWidget *CustomWidgetFactory::createWidget( const QString &className, QWidget *parent, const char *name ) const
{
    MetaDataBase::CustomWidget *w = MetaDataBase::customWidget( WidgetDatabase::idFromClassName( className ) );
    if ( !w )
	return 0;
    return WidgetFactory::createCustomWidget( parent, name, w );
}

