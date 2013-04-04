/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2013 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include <qwidget.h>
#include <qfontmetrics.h>
#include "elswitch.h"

/*!
  \brief Constructor
  \param parent Parent Widget
  \param name Name
*/
ELSwitch::ELSwitch(QWidget *parent, const char *name)
        : QWidget(parent,name)
{
    OnString="on";
    OffString="off";
    QFontMetrics fm( this->font() );
    int maxwidth=QMAX( fm.size(SingleLine,OnString).width(), fm.size(SingleLine,OffString).width());
    setGeometry(0,0,maxwidth+70,80);
    value=false;
    setFocusPolicy(QWidget::StrongFocus);
}

/*!
  \brief Repaint the widget
*/
void ELSwitch::paintEvent(QPaintEvent *)
{
    drawSwitch();
}

/*!
  \brief Mouse Release Event management
*/
void ELSwitch::mouseReleaseEvent(QMouseEvent * e)
{
	if(e->button() == Qt::RightButton)
	{
		//emit RightClicked();
		QString name = this->name();

		emit RightClicked(QString("ELSwitch"), name);
	}
	else
	{
		toggleValueManually();
		update();
	}
}

/*!
  \brief Key Release Event management
*/
void ELSwitch::keyReleaseEvent(QKeyEvent *e)
{
    int keyPressed=e->key();

    if(keyPressed==Qt::Key_Return || keyPressed==Qt::Key_Space)
    {
        toggleValueManually();
        update();
    }
}

static char*forward[]={
"16 16 5 1",
"# c #000000",
"a c #ffffff",
"c c #808080",
"b c #c0c0c0",
". c None",
"................",
"................",
".........#......",
".........##.....",
".........#a#....",
"..########aa#...",
"..#aaaaaaabaa#..",
"..#bbbbbbbbbaa#.",
"..#bbbbbbbbba#..",
"..########ba#c..",
"..ccccccc#a#c...",
"........c##c....",
"........c#c.....",
"........cc......",
"........c.......",
"................",
"................"};

static char*back[]={
"16 16 5 1",
"# c #000000",
"a c #ffffff",
"c c #808080",
"b c #c0c0c0",
". c None",
"................",
".......#........",
"......##........",
".....#a#........",
"....#aa########.",
"...#aabaaaaaaa#.",
"..#aabbbbbbbbb#.",
"...#abbbbbbbbb#.",
"...c#ab########.",
"....c#a#ccccccc.",
".....c##c.......",
"......c#c.......",
".......cc.......",
"........c.......",
"................",
"......................"};

/*!
  \brief Repaint the widget
*/
void ELSwitch::drawSwitch()
{
	QPainter painter(this);
	
	if(value)
	{
		painter.begin(this);
		painter.drawPixmap(0, 0, QPixmap((const char **)forward));
		painter.setFont(QFont("helvetica", 10));
		painter.setPen(black);
		//painter.drawText(0, 0, width(), height()-2,
        //       AlignLeft|AlignBottom,
        //         "State is:");
                 
		painter.drawText(0, 0, width(), height()-2,
                 AlignRight|AlignBottom,
                 "On");
		painter.end();
	}
	else
	{
		painter.begin(this);
		painter.drawPixmap(0, 0, QPixmap((const char **)back));
		painter.setFont(QFont("helvetica", 10));
		painter.setPen(black);
		//painter.drawText(0, 0, width(), height()-2,
        //       AlignLeft|AlignBottom,
        //         "State is:");
                 
		painter.drawText(0, 0, width(), height()-2,
                 AlignRight|AlignBottom,
                 "Off");
		painter.end();
	}
}

/*!
  \brief Toggle the switch value
*/
void ELSwitch::toggleValue()
{
    if(isEnabled())
    {
        if (value)
            value=false;
        else
            value=true;
        //emit valueChanged(value);
    }
}

/*!
  \brief Toggle the switch value manually
*/
void ELSwitch::toggleValueManually()
{
    if(isEnabled())
    {
        if (value)
            value=false;
        else
            value=true;
        emit valueChanged(value);
    }
}


void ELSwitch::undoToggle()
{
    if(isEnabled())
    {
        if (value)
            value=false;
        else
            value=true;
    }
}


/*!
  \brief Set the switch value
*/
void ELSwitch::setELSwitchValue(bool newValue)
{
	setEnabled(true);

    if(getELSwitchValue()!=newValue)
    {
        toggleValue();
        update();
    }
}

/*!
  \brief Set the switch value as invalid
*/
void ELSwitch::setELSwitchValueInvalid(bool newValue)
{
	setEnabled(false);
}

/*!
  \brief Set the On label
*/
void ELSwitch::setOnLabel(QString onstring)
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
void ELSwitch::setOffLabel(QString offstring)
{
    QRect rect;
    OffString=offstring;
    rect= geometry();
    QFontMetrics fm( this->font() );
    int maxwidth=QMAX( fm.size(SingleLine,OnString).width(), fm.size(SingleLine,OffString).width());
    setGeometry(rect.x(),rect.y(),maxwidth+70,80);
    update();
}
