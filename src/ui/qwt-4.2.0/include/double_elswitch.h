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

#ifndef DOUBLE_ELSWITCH_H
#define DOUBLE_ELSWITCH_H

#include <qwidget.h>
#include <qpainter.h>
#include <qwidgetplugin.h>
#include "qwt_global.h"

/*!
  \brief The Electrical Switch Widget.

  The switch widget imitates look and behaviour of an double point electrical switch.
  4 dinamic states
  
*/

class QWT_EXPORT DoubleBreaker: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int switchValue READ getBreakerValue WRITE setBreakerValue);
    Q_PROPERTY(QString OnLabel READ getOnLabel WRITE setOnLabel);
    Q_PROPERTY(QString OffLabel READ getOffLabel WRITE setOffLabel);
	Q_PROPERTY( QPixmap Onpixmap READ Onpixmap WRITE setOnPixmap );
	Q_PROPERTY( QPixmap Offpixmap READ Offpixmap WRITE setOffPixmap );
	Q_PROPERTY( QPixmap Invalid11pixmap READ Invalid11pixmap WRITE setInvalid11Pixmap );
	Q_PROPERTY( QPixmap Invalid00pixmap READ Invalid00pixmap WRITE setInvalid00Pixmap );

public:
    DoubleBreaker(QWidget *parent=0,const char *name=0);
    /*!
      \return current switch value
    */
    int getBreakerValue() const
    {
        return value;
    }

    /*!
      \return current switch on label
    */
    QString getOnLabel() const
    {
        return OnString;
    }

    /*!
      \return current switch off label
    */
    QString getOffLabel() const
    {
        return OffString;
    }

    /*!
      \return size hint
    */
    QSize sizeHint() const
    {
        return minimumSizeHint();
    }

    /*!
      \return a minimum size hint
    */
    QSize minimumSizeHint() const
    {
        return QSize(60,60);
    }

	void undoToggle();

	QPixmap     *Onpixmap()	const	{ return lOnpixmap; }
	QPixmap     *Offpixmap()	const	{ return lOffpixmap; }
	QPixmap     *Invalid00pixmap()	const	{ return lInvalid00pixmap; }
	QPixmap     *Invalid11pixmap()	const	{ return lInvalid11pixmap; }

public slots:
    void setOnLabel(QString);
    void setOffLabel(QString);
    void setBreakerValue(int);
	void setBreakerValueInvalid(int);
	void setOnPixmap( const QPixmap & );
	void setOffPixmap( const QPixmap & );
	void setInvalid00Pixmap( const QPixmap & );
	void setInvalid11Pixmap( const QPixmap & );

signals:
    /*!
      \brief This signal reports when the switch is toggled
    */
    void valueChanged(int);
	//void RightClicked(void);
	void RightClicked(QString &class_name, QString &name);

protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyReleaseEvent(QKeyEvent *);
    QString OnString,OffString;
    int value;// 00 01 10 11 - 0 1 2 3
    void drawSwitch();
    void toggleValue();
	void toggleValueManually();
	QPixmap    *lOnpixmap;
	QPixmap    *lOffpixmap;
	QPixmap    *lInvalid00pixmap;
	QPixmap    *lInvalid11pixmap;
};

#endif //DOUBLE_ELSWITCH_H
