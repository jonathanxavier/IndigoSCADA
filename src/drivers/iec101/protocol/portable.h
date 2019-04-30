/*
 This header file provides some definitions for unix compatibility.
 @version: 0.03
 @last change: 23.12.2011
*/

#ifndef __PORTABLE_H
#define __PORTABLE_H

#define __cdecl	__attribute__((cdecl))   
#define __stdcall  	__attribute__((stdcall))
#define __fastcall	__attribute__((fastcall))


typedef unsigned long DWORD;
typedef int BOOL;
typedef void * HANDLE;
typedef unsigned char BYTE;

/*__int64 for gcc compiler */
#define __int64 long long int


/*MS! y u no use standard posix definitions? */
#define _stricmp(s1,s2) strcmp(s1,s2)

#define _stat stat
#define _fstat fstat
#define _open open
#define _close close
#define _O_RDONLY O_RDONLY
#define _O_BINARY 0

/*from win32.h*/
#define INVALID_HANDLE_VALUE ((HANDLE) -1)

#define TRUE 1
#define FALSE 0



#define Sleep(msec) if (msec < 1000)  usleep(msec*1000); else sleep(msec/1000);

#define GetCurrentProcessId getpid

/******************FROM WIN32 STDLIB.h*********************/
/*
 * Sizes for buffers used by the _makepath() and _splitpath() functions.
 * note that the sizes include space for 0-terminator
 */
#define _MAX_PATH   260 /* max. length of full pathname */
#define _MAX_DRIVE  3   /* max. length of drive component */
#define _MAX_DIR    256 /* max. length of path component */
#define _MAX_FNAME  256 /* max. length of file name component */
#define _MAX_EXT    256 /* max. length of extension component */

/******************END: FROM WIN32 STDLIB.h*********************/

#endif //__PORTABL_H
