#include "DateNavigator.h"
#define Inherited DateNavigatorData
DateNavigator::DateNavigator
(
QWidget* parent,
const char* name
)
:
Inherited( parent, name )
{
	setCaption( tr("Select Date"));
}
DateNavigator::~DateNavigator()
{
}

