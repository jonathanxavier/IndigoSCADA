#include <time.h>

time_t time(time_t *t)
{ return(0); }

void sleep(long st)
{
int i;
for(i=0; i<st*100000; i++ ) ;
}


