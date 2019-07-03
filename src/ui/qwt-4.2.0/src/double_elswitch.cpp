/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2019 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include <qwidget.h>
#include <qfontmetrics.h>
#include "double_elswitch.h"

#include "breaker_closed.xpm"
#include "breaker_opened.xpm"
#include "breaker_invalid00.xpm"
#include "breaker_invalid11.xpm"

/*!
  \brief Constructor
  \param parent Parent Widget
  \param name Name
*/
DoubleBreaker::DoubleBreaker(QWidget *parent, const char *name)
        : QWidget(parent,name)
{
    OnString="on";
    OffString="off";
    QFontMetrics fm( this->font() );
    int maxwidth=QMAX( fm.size(SingleLine,OnString).width(), fm.size(SingleLine,OffString).width());
    setGeometry(0,0,maxwidth+70,80);
    value=false;
    setFocusPolicy(QWidget::StrongFocus);

	lOnpixmap = new QPixmap((const char **)breaker_closed_xpm);
	lOffpixmap = new QPixmap((const char **)breaker_opened_xpm);
	lInvalid11pixmap = new QPixmap((const char **)breaker_invalid11_xpm);
	lInvalid00pixmap = new QPixmap((const char **)breaker_invalid00_xpm);
}

/*!
  \brief Repaint the widget
*/
void DoubleBreaker::paintEvent(QPaintEvent *)
{
    drawSwitch();
}

/*!
  \brief Mouse Release Event management
*/
void DoubleBreaker::mouseReleaseEvent(QMouseEvent * e)
{
	if(e->button() == Qt::RightButton)
	{
		//emit RightClicked();
		QString name = this->name();

		emit RightClicked(QString("DoubleBreaker"), name);
	}
	//else
	//{
	//	toggleValueManually();
	//	update();
	//}
}

/*!
  \brief Key Release Event management
*/
void DoubleBreaker::keyReleaseEvent(QKeyEvent *e)
{
//    int keyPressed=e->key();

//    if(keyPressed==Qt::Key_Return || keyPressed==Qt::Key_Space)
//    {
//        toggleValueManually();
//        update();
//    }
}


/*!
  \brief Repaint the widget
*/
void DoubleBreaker::drawSwitch()
{
	QPainter painter(this);
	
	if(value == 2)
	{
		painter.begin(this);
		painter.drawPixmap(0, 0, *lOnpixmap);
		painter.setFont(QFont("helvetica", 10));
		painter.setPen(black);
		painter.end();
	}
	else if(value == 1)
	{
		painter.begin(this);
		painter.drawPixmap(0, 0, *lOffpixmap);
		painter.setFont(QFont("helvetica", 10));
		painter.setPen(black);
		painter.end();
	}
	else if(value == 0)
	{
		painter.begin(this);
		painter.drawPixmap(0, 0, *lInvalid00pixmap);
		painter.setFont(QFont("helvetica", 10));
		painter.setPen(black);
		
		painter.end();
	}
	else if(value == 3)
	{
		painter.begin(this);
		painter.drawPixmap(0, 0, *lInvalid11pixmap);
		painter.setFont(QFont("helvetica", 10));
		painter.setPen(black);
		
		painter.end();
	}
}

/*!
  \brief Toggle the switch value
*/
void DoubleBreaker::toggleValue()
{
    if(isEnabled())
    {
        if (value == 2)
            value = 1;
        else
            value = 2;
        //emit valueChanged(value);
    }
}

/*!
  \brief Toggle the switch value manually
*/
void DoubleBreaker::toggleValueManually()
{
    if(isEnabled())
    {
        if (value == 2)
            value=1;
        else
            value=2;
        emit valueChanged(value);
    }
}


void DoubleBreaker::undoToggle()
{
    if(isEnabled())
    {
        if (value == 2)
            value=1;
        else
            value=2;
    }
}


/*!
  \brief Set the switch value
*/
void DoubleBreaker::setBreakerValue(int newValue)
{
	setEnabled(true);

    if(getBreakerValue()!=newValue)
    {
        //toggleValue();
		value = newValue;
        update();
    }
}

/*!
  \brief Set the switch value as invalid
*/
void DoubleBreaker::setBreakerValueInvalid(int newValue)
{
	setEnabled(false);
}

/*!
  \brief Set the On label
*/
void DoubleBreaker::setOnLabel(QString onstring)
{
    QRect rect;
    OnString=onstring;
    rect= geometry();
    QFontMetrics fm( this->font() );
    int maxwidth=QMAX( fm.size(SingleLine,OnString).width(), fm.size(SingleLine,OffString).width());
    setGeometry(rect.x(),rect.y(),maxwidth+70,80);
    update();
}

/*!
  \brief Set the Off label
*/
void DoubleBreaker::setOffLabel(QString offstring)
{
    QRect rect;
    OffString=offstring;
    rect= geometry();
    QFontMetrics fm( this->font() );
    int maxwidth=QMAX( fm.size(SingleLine,OnString).width(), fm.size(SingleLine,OffString).width());
    setGeometry(rect.x(),rect.y(),maxwidth+70,80);
    update();
}

/*!
    \property DoubleBreaker::pixmap
    \brief the label's pixmap

    If no pixmap has been set this will return an invalid pixmap.

    Setting the pixmap clears any previous content
*/

void DoubleBreaker::setOnPixmap( const QPixmap &pixmap )
{
    QSize osh = sizeHint();

    if ( !lOnpixmap || lOnpixmap->serialNumber() != pixmap.serialNumber() ) 
	{
		if(lOnpixmap)
		{
			delete lOnpixmap;
			lOnpixmap = 0;
		}

		lOnpixmap = new QPixmap( pixmap );
    }

    if ( lOnpixmap->depth() == 1 && !lOnpixmap->mask() )
	lOnpixmap->setMask( *((QBitmap *)lOnpixmap) );

	update();
}

void DoubleBreaker::setOffPixmap( const QPixmap &pixmap )
{
    QSize osh = sizeHint();

    if ( !lOffpixmap || lOffpixmap->serialNumber() != pixmap.serialNumber() ) 
	{
		if(lOffpixmap)
		{
			delete lOffpixmap;
			lOffpixmap = 0;
		}

		lOffpixmap = new QPixmap( pixmap );
    }

    if ( lOffpixmap->depth() == 1 && !lOffpixmap->mask() )
	lOffpixmap->setMask( *((QBitmap *)lOffpixmap) );

	update();
}

void DoubleBreaker::setInvalid00Pixmap( const QPixmap &pixmap )
{
    QSize osh = sizeHint();

    if ( !lInvalid00pixmap || lInvalid00pixmap->serialNumber() != pixmap.serialNumber() ) 
	{
		if(lInvalid00pixmap)
		{
			delete lInvalid00pixmap;
			lInvalid00pixmap = 0;
		}

		lInvalid00pixmap = new QPixmap( pixmap );
    }

    if ( lInvalid00pixmap->depth() == 1 && !lInvalid00pixmap->mask() )
	lInvalid00pixmap->setMask( *((QBitmap *)lInvalid00pixmap) );

	update();
}

void DoubleBreaker::setInvalid11Pixmap( const QPixmap &pixmap )
{
    QSize osh = sizeHint();

    if ( !lInvalid11pixmap || lInvalid11pixmap->serialNumber() != pixmap.serialNumber() ) 
	{
		if(lInvalid11pixmap)
		{
			delete lInvalid11pixmap;
			lInvalid11pixmap = 0;
		}

		lInvalid11pixmap = new QPixmap( pixmap );
    }

    if ( lInvalid11pixmap->depth() == 1 && !lInvalid11pixmap->mask() )
	lInvalid11pixmap->setMask( *((QBitmap *)lInvalid11pixmap) );

	update();
}
