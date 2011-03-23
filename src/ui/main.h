/*h
*Header For: user interface common stuff
*Purpose:
*/

#ifndef include_main_hpp 
#define include_main_hpp
#include "realtimedb.h"
#include "common.h"
//

class QSApplication : public QApplication
{
	public:
	static bool fKeyMouseEvent;
	QSApplication(int &ac, char **av) : QApplication(ac,av)
	{
	};
	bool notify(QObject *o, QEvent *e);
};
#endif

