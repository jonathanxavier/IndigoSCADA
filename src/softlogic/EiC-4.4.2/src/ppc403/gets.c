#include <stdio.h>
#include <signal.h>

extern int ParseError;

void outbyte(int ch);
int inbyte(void);
char *ppcgets(char *s)
{
int i,j;
/*fflush(stdout);
fflush(stderr); */

for(i=0; i<1024; i++)
{
        j=inbyte();
	j=j&127;
	if(j<32 && j!=13 && j!=10 && j!=8 && j!=9 && j!=3) j=32;
	if(j==3) raise(SIGINT); /* control-C pressed */
        if(j==127 || j==8) 
	{ 
	  if(i>=1)
	  {
	    	i+=-2;
		outbyte(8); 
	    	outbyte(' '); 
	    	outbyte(8);
	  }
	  else 	i--;
	}
	else
	{
	  if(j!=13 && j!=10)
	  { 
	    s[i]=j; 
            outbyte(j); 
          }
	}
        if(j==13) { /* outbyte(j); */ break; }
}
/* outbyte(10); */
s[i]=0;
/* fflush(stdout);
fflush(stderr); */
puts("");
return(s);
}

