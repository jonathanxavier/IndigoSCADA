#ifndef __INCL_LED
#define __INCL_LED

#include <qt.h>
#include "common.h"

class QSEXPORT Led : public QFrame
{
  Q_OBJECT
public:
   Led(QWidget *parent=0);
   enum State { On, Off };
   State state() const { return s; }
   void setState(State state) { s= state; repaint(); }
   void toggleState() { if (s == On) s= Off; else if (s == Off) s= On; repaint(); }
   virtual ~Led();
   void setText(const char *s){text=s;}
   void setColor(const char *c){color=QColor(c);}
   void startFlash();
   void stopFlash();

   QSize sizeHint() const;
   QSizePolicy sizePolicy() const;
   
 public slots:
   void toggle() { toggleState(); };
   void on() { setState(On); };
   void off() { setState(Off); };
 
 protected:
   void drawContents(QPainter *);
 
 private:
   const int lwidth;
   const int lheight;
   const int dx;
   State s;

 protected slots:
   void timerSlot();
   
 protected:
   QTimer *timer;
   QColor color;
   QString text;
   signals:
   
};

#endif //__INCL_LED
