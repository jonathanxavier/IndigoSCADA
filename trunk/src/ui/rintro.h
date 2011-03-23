#ifndef RINTRO_H
#define RINTRO_H

#include <qt.h>

class RIntro : public QWidget
{
  Q_OBJECT
public:
  RIntro(const QString& _version,
         QWidget* _parent=0, 
         const char* _name=0);
  ~RIntro();

private:
  QString    sVersion;
  QPixmap*   pIntro;

protected:
  virtual void paintEvent(QPaintEvent* _ev);
  
};

#endif
