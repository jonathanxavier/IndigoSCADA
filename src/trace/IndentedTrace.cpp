/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2009 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
// IndentedTrace.cpp: implementation of the CIndentedTrace class.
//
// Better tracing class and macros
// 
// 
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include "IndentedTrace.h"
#include <assert.h>
#include <rpc.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
	// Add this class to a program to print a trace of function calls in the 
	// output window for debug compiles. For non-debug compiles, the class 
	// contains only empty functions. Tracing must be enabled for this to work.
	// 
	// Declare a CIndentedTrace variable at the beginning of each function 
	// to be traced. Give the class and function names as the argument. The 
	// IndentedTrace constructor will print the class and function names, 
	// and an opening brace in the output window and increment the indent 
	// level. 
	//
	// Example of use:
	// 
	// CMyClass::MyFunc()
	// {
	//		CIndentedTrace IT("CMyClass::MyFunc()");
	//		...
	// }
	// 
	// Traced functions called by the first function are printed at the new 
	// indent level. 
	// 
	// The variable goes out of scope at the end of the function. The 
	// destructor decrements the indent and prints a closing brace. 
	// 
	// Some functions contain many repetitive function calls. To suppress 
	// CIndentedTrace tracing inside such functions, call SuppressTrace(). 
	// Tracing will resume when the function exits.
	// 
	// To print a comment at the current indent level, call Comment().


//////////////////////////////////////////////////////////////////////
	// Initialize static data members.

#ifdef _DEBUG_TRACE

	// Set initial indent level to no indentation. 
int CIndentedTrace::ms_iTraceDepth = 0; 
	// Deactivate trace suppression. See SuppressTrace() for more info.  
int CIndentedTrace::ms_iSuppressDepth = 0;	

	// Initialize to 0 so first serial number will be 1. 
int CIndentedTrace::ms_iSerialNumber = 0;

	// By default, do not display serial numbers.
bool CIndentedTrace::ms_bEnableSerialNumber = false;

// By default, display thread ID.
bool CIndentedTrace::ms_bEnableThreadID = true;

bool CIndentedTrace::ms_bEnableTime = false;

unsigned CIndentedTrace::mainThread = 0;

bool CIndentedTrace::seeClassFuncOnExit = true;

FILE * CIndentedTrace::fp = NULL;

unsigned long CIndentedTrace::writecounter = 0;

CCPUTicker* CIndentedTrace::m_tick1 = NULL;
CCPUTicker* CIndentedTrace::m_tick2 = NULL;

 
#endif // _DEBUG_TRACE

//////////////////////////////////////////////////////////////////////
// Implement private helper functions. 

// ---------------------------------------------------------------------------
// AppendSN()
// 
// Appends the serial number of this instance to strText (unless modified by 
// trace suppression or relase compilation). 
// 
// strText - IN. String to which the serial number will be appended. 
// 
// Each time a CIndentedTrace object is constructed, the constructor reserves 
// a serial number for object. Serial numbers start at 1, and are incremented 
// each time the constructor is called. 
// 
// This function is only called inside CIndentedTrace functions that are 
// changed when tracing is suppressed. 
// 
// ---------------------------------------------------------------------------
void CIndentedTrace::AppendSN(char* strText)
{
#ifdef _DEBUG_TRACE

	if (ms_bEnableSerialNumber)
	{
		char strLSN[STR_LEN];
		sprintf(strLSN,"\t\t// S/N %d", m_nLocalSerialNumber);
		strcat(strText,strLSN);
	}

#endif // _DEBUG_TRACE
}


// ---------------------------------------------------------------------------
// AppendThdId()
// 
// Appends the ID of the thread this instance runs in to strText, unless it 
// is the main thread (or unless modified by trace suppression or relase 
// compilation).  
// 
// strText - IN. String to which the thread ID will be appended. 
// 
// 
// ---------------------------------------------------------------------------
void CIndentedTrace::AppendThdId(char* strText)
{
#ifdef _DEBUG_TRACE
	if (ms_bEnableThreadID)
	{
		if ((unsigned)GetCurrentThreadId() != mainThread )
		{
			char strThdId[STR_LEN];

			sprintf(strThdId,"\t\t// Thd ID %u", (unsigned)GetCurrentThreadId());
			strcat(strText,strThdId);
		}
	}
#endif // _DEBUG_TRACE
}



// ---------------------------------------------------------------------------
// AppendTime()
// 
// Appends the Time elapsed since the constructor was callen. 
// 
// strText - IN. String to which the thread ID will be appended. 
//
// ---------------------------------------------------------------------------
void CIndentedTrace::AppendTime(char* strText)
{
#if defined( _DEBUG_TRACE ) && defined (WIN32)
	if (ms_bEnableTime)
	{
			char strThdId[STR_LEN];
			
			m_tick2->Measure();
			sprintf(strThdId,"\t\t%f s.",m_tick2->GetTickCountAsSeconds() - m_tick1->GetTickCountAsSeconds());
			strcat(strText,strThdId);
	}
#endif // _DEBUG_TRACE
}


//////////////////////////////////////////////////////////////////////
// Constructors/Destructors

// ---------------------------------------------------------------------------
// CIndentedTrace( const char * )
// 
// This constructor assigns this instance a serial number, prints an indented 
// line in the output window with opening brace, and increments the indent 
// level (unless modified by trace suppression or relase compilation). 
// 
// strClass_Func - IN. String that is printed. 
// 
// strClass_Func usually contains the name of the class and function that 
// calls this constructor. E.g. "CMyClass::MyFunc()".
// 
// When strClass_Func is printed in the output window, spaces are added to the 
// beginning to indent it, and an opening brace is added to the end. Depending 
// on ms_bEnableSerialNumber and ms_bEnableThreadID, a serial number and/or 
// thread ID may be added to the end as well. 
// 
// The static member variable ms_iSerialNumber contains the most recently used 
// taken serial number. The serial number is reserved by copying it and 
// incrementing ms_iSerialNumber. 
// 
// ---------------------------------------------------------------------------
CIndentedTrace::CIndentedTrace(const char * strClass_Func)
{
#ifdef _DEBUG_TRACE
		
		m_msg = new char[STR_LEN];
	
		if(fp == NULL)
		{
			#ifdef WIN32
			char log_file_name[_MAX_PATH];
			//char buffer[50];

			UUID uuid;
			UuidCreate(&uuid);

			unsigned char* str;

			UuidToString(&uuid, &str);

			log_file_name[0] = '\0';
			if(GetModuleFileName(NULL, log_file_name, _MAX_PATH))
			{
				*(strrchr(log_file_name, '\\')) = '\0';        // Strip \\filename.exe off path
				*(strrchr(log_file_name, '\\')) = '\0';        // Strip \\bin off path
            
				strcat(log_file_name,"\\logs\\Process_");
				//strcat(log_file_name, _ltoa(GetCurrentProcessId(),buffer,10));  
				strcat(log_file_name, (const char*)str);  
				strcat(log_file_name,".cpp");
			}

			RpcStringFree(&str);
				
			if((fp = fopen(log_file_name,"w+")) == NULL)
			{
			//	OutputDebugString("UNABLE TO OPEN FILE : \n");
			//	OutputDebugString(log_file_name);
			//	sprintf(log_file_name,"Process_%d.cpp", GetCurrentProcessId());
			//	fp = fopen(log_file_name,"w+");
			}

			if (ms_bEnableTime)
			{
				//initialize the timers.
				m_tick1 = new CCPUTicker();
				m_tick2 = new CCPUTicker();

				m_tick1->GetTickCountAsSeconds();

				//start m_tick1 counter
				m_tick1->Measure();
			}
	
			#else //WIN32
			
			//TO DO: Find in UNIX the fnction like the GetModuleFileName
			char log_file_name[256];
			sprintf(log_file_name,"Process_%d.cpp", GetCurrentProcessId());
			fp = fopen(log_file_name,"w+");

			#endif  //UNIX
		}

		strcpy(LocalClassNameFunc,strClass_Func);
		m_nLocalSerialNumber = ++ms_iSerialNumber;

		if(ms_bEnableThreadID && !mainThread)
		{
			mainThread = GetCurrentThreadId();
		}

		Entry(strClass_Func);

#endif // _DEBUG_TRACE
}


// ---------------------------------------------------------------------------
// ~CIndentedTrace()
// 
// The destructor decrements the indent level and prints an indented closing 
// brace (unless modified by trace suppression or relase compilation).
// 
// 
// ---------------------------------------------------------------------------
CIndentedTrace::~CIndentedTrace()
{
#ifdef _DEBUG_TRACE

		Exit(LocalClassNameFunc);

		delete[] m_msg;

#endif // _DEBUG_TRACE
}


//////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Entry()
// 
// Prints an indented line in the output window with opening brace, and 
// increments the indent level (unless trace suppression is active or this is 
// a relase compile). 
// 
// strClass_Func - IN. String that is printed. 
// 
// strClass_Func usually contains the name of the class and function that 
// created this instance of CIndentedTrace. E.g. "CMyClass::MyFunc()".
// 
// When strClass_Func is printed in the output window, spaces are added to the 
// beginning to indent it, and an opening brace is added to the end. Depending 
// on ms_bEnableSerialNumber and ms_bEnableThreadID, a serial number and/or 
// thread ID may be added to the end as well. 
// 
// ---------------------------------------------------------------------------
void CIndentedTrace::Entry(const char * strClass_Func)
{
#ifdef _DEBUG_TRACE

	// Sanity checks.
	assert (ms_iTraceDepth >= 0);
	assert ((ms_iTraceDepth >= ms_iSuppressDepth) || (ms_iSuppressDepth == 0));

	if ((unsigned)GetCurrentThreadId() == mainThread )//apa added
	{
		m_nLocalTraceDepth = ms_iTraceDepth;
	}
	else
		m_nLocalTraceDepth = 30;

	if (ms_iSuppressDepth == 0)
	{		// Suppression inactive. Do trace. 
		//QChar spazio(' ');

		char strIndent[STR_LEN];
		strIndent[0] = '\0';

		for(int i= 0; i<=INDENT_SPACES * m_nLocalTraceDepth; i++)
		{
			strcat(strIndent," ");
		}
		
		char strText[STR_LEN];
		strcpy(strText, strIndent);
		strcat(strText,strClass_Func);
		strcat(strText," {");

		//strText = strIndent + strClass_Func + " {";

		AppendThdId(strText); 
		AppendTime(strText);
		AppendSN(strText);	   
		strcat(strText,"\n");
		
		#ifdef WIN32
		OutputDebugString((const char *)strText);
		#endif
		fprintf(fp,(const char *)strText);
		fflush(fp);
		writecounter++;
	}

	if ((unsigned)GetCurrentThreadId() == mainThread )//apa added
	{
		++ms_iTraceDepth;
	}
	++m_nLocalTraceDepth;
	

#else // _DEBUG_TRACE

	//UNUSED(strClass_Func);	// Prevent compiler warning

#endif // _DEBUG_TRACE
}


// ---------------------------------------------------------------------------
// Exit()
// 
// Opposite of Entry(). Decrements the indent level and prints an indented 
// closing brace (unless trace suppression is active or this is a relase 
// compile). 
// 
// Exit() is also responsible for turning off trace suppression. For more 
// info, see SuppressTrace(). 
// 
// ---------------------------------------------------------------------------
void CIndentedTrace::Exit(const char * strClass_Func)
{
#ifdef _DEBUG_TRACE

	// Sanity checks.
	assert (ms_iTraceDepth >= 0);
	assert ((ms_iTraceDepth >= ms_iSuppressDepth) || (ms_iSuppressDepth == 0));

	if ((unsigned)GetCurrentThreadId() == mainThread ) //apa added
	{
		--ms_iTraceDepth;
	}
	--m_nLocalTraceDepth;
	

	if (ms_iTraceDepth < ms_iSuppressDepth)
	{		// Exiting function that started suppression. Deactivate it.
		ms_iSuppressDepth = 0;
	}
	
	if (ms_iSuppressDepth == 0)
	{		// Suppression inactive. Do trace. 
		char strIndent[STR_LEN];
		strIndent[0] = '\0';

		for(int i= 0; i<=INDENT_SPACES * m_nLocalTraceDepth; i++)
		{
			strcat(strIndent," ");
		}
		
		char strText[STR_LEN];
		strText[0] = '\0';

		if(seeClassFuncOnExit)
		{
			//strText = strIndent + "}" + " //" + strClass_Func;
			strcpy(strText, strIndent);
			strcat(strText, "}");
			strcat(strText, "//");
			strcat(strText, strClass_Func);
		}
		else
		{
			//strText = strIndent + "}";
			strcat(strIndent, "}");
		}

		AppendThdId(strText);  
		AppendTime(strText);
		AppendSN(strText);	   
		strcat(strText,"\n");

		#ifdef WIN32
		OutputDebugString((const char *)strText);
		#endif
		fprintf(fp,(const char *)strText);
		fflush(fp);
		writecounter++;
		
		if (m_nLocalTraceDepth == 0)
		{		// Insert extra line to separate blocks
			
			#ifdef WIN32
			OutputDebugString ((const char *)"\n"); 
			#endif
			fprintf(fp,(const char *)"\n");
			fflush(fp);
			writecounter++;
		}
	}

	#ifdef LOG_ONLY_LAST_PART
	if(writecounter > 100000l)
	{
		writecounter = 0;

		if(fseek(fp, 0, SEEK_SET))
		{
			assert(0);			
		}
	}
	#endif


#endif // _DEBUG_TRACE
}


// ---------------------------------------------------------------------------
// Comment()
// 
// Prints an indented comment line in the output window (unless trace 
// suppression is active or this is a relase compile). 
// 
// strComment - IN. String that is printed. 
// 
// When strComment is printed in the output window, spaces are added to the 
// beginning to indent it, and "// " is added to the beginning. Depending 
// on ms_bEnableSerialNumber and ms_bEnableThreadID, a serial number and/or 
// thread ID may be added to the end as well. 
// 
// ---------------------------------------------------------------------------
void CIndentedTrace::Comment(const char * strComment)
{
		// Print an indented comment. 
	if(!strComment) return;

#ifdef _DEBUG_TRACE
	if (ms_iSuppressDepth == 0)
	{
		char strIndent[STR_LEN];
		strIndent[0] = '\0';

		for(int i= 0; i<=INDENT_SPACES * m_nLocalTraceDepth; i++)
		{
			strcat(strIndent," ");
		}

		strcat(strIndent,"// "); 
		strcat(strIndent, strComment); 
		AppendThdId(strIndent);
		AppendTime(strIndent);
		AppendSN(strIndent); 
		strcat(strIndent,"\n");

		#ifdef WIN32
		OutputDebugString ((const char *)strIndent);
		#endif
		fprintf(fp,(const char *)strIndent);
		fflush(fp);
		writecounter++;
		
	}

	#ifdef LOG_ONLY_LAST_PART
	if(writecounter > 100000l)
	{
		writecounter = 0;

		if(fseek(fp, 0, SEEK_SET))
		{
			assert(0);			
		}
	}
	#endif


#else // _DEBUG_TRACE
//	UNUSED(strComment);				// Prevent compiler warning.
#endif // _DEBUG_TRACE
}


// ---------------------------------------------------------------------------
// SuppressTrace()
// 
// Suppresses action of CIndentedTrace functions until the the instance where 
// SuppressTrace() is called exits (unless this is a release compile). 
// 
// This is function was intended to prevent functions that produce a lot of 
// CIndentedTrace output from cluttering the output window. It turns out not 
// to be very useful. 
// 
// Each CIndentedTrace function that produces output checks ms_iSuppressDepth. 
// If ms_iSuppressDepth > 0, suppression is active, and the CIndentedTrace 
// function does little or nothing. 
// 
// Normally, ms_iSuppressDepth is 0, which disables suppression. 
// 
// Suppose a function, MyFunc(), creates an instance of CIndentedTrace and 
// calls SuppressTrace(). This sets ms_iSuppressDepth to the current indent 
// level, starting suppression. 
// 
// If MyFunc() calls more functions, the indent level may increase. But it 
// should never drop below the current level until MyFunc() exits. 
// 
// Exit() checks for when the indent level drops below the value stored in 
// ms_iSuppressDepth. When this happens, Exit() sets ms_iSuppressDepth to 0, 
// turning off suppression. 
// 
// 
// SuppressTrace() and multithreaded apps do not mix. 
// 
// ---------------------------------------------------------------------------
void CIndentedTrace::SuppressTrace()
{
	// CIndentedTrace functions will do no further tracing until the function 
	// that called SuppressTrace() exits.
	//

#ifdef _DEBUG_TRACE
	assert (m_nLocalTraceDepth > 0);

	if (ms_iSuppressDepth == 0)
	{		// Start trace suppression only if it is not already started.
		ms_iSuppressDepth = m_nLocalTraceDepth; 

			// Show suppression is starting.
		
		char strIndent[STR_LEN];
		strIndent[0] = '\0';

		for(int i= 0; i<=INDENT_SPACES * m_nLocalTraceDepth; i++)
		{
			strcat(strIndent," ");
		}
	
		strcat(strIndent,"// CIndentedTrace is suppressed for the rest of this function\n");

		#ifdef WIN32
		OutputDebugString ((const char*)strIndent);
		#endif
		fprintf(fp,(const char *)strIndent);
		fflush(fp);
		writecounter++;
	}

#endif // _DEBUG_TRACE
}


// ---------------------------------------------------------------------------
// DumpStr() 
// 
// Dumps the contents of a string or buffer as ASCII characters and as hex 
// numbers (unless trace suppression is active or this is a relase compile). 
// 
// str - IN. string or buffer to be dumped. 
// 
// NChars - IN. Number of chars in str. 
// 
// 
// ---------------------------------------------------------------------------
void CIndentedTrace::DumpStr(const char * str, const UINT NChars)
{
#ifdef _DEBUG_TRACE

	if (ms_iSuppressDepth != 0)
	{
		// Tracing is suppressed.
		return;
	}

	char strAsc[STR_LEN];					// Chars as ASCII
	strAsc[0] = '\0';
	unsigned int k = 0;
	for (unsigned int j = 0; j < NChars; ++j)
	{
		if(isprint((int) str[j]))
		{
			strAsc[k++] = str[j]; 
		}
		else
		{			
			switch((int) str[j])
			{
				case 6:
				strAsc[k++] = 'A';
				strAsc[k++] = 'C';
				strAsc[k++] = 'K';
				break;
				case 21:
				strAsc[k++] = 'N';
				strAsc[k++] = 'A';
				strAsc[k++] = 'K';
				break;
				default:
				strAsc[k++] = '.'; 
			}
		}
	}

	strAsc[k] = '\0';
	
	char strIndent[STR_LEN];
	strIndent[0] = '\0';

	for(int i= 0; i<=INDENT_SPACES * m_nLocalTraceDepth; i++)
	{
		strcat(strIndent," ");
	}

	strcat(strIndent,"// "); 
	strcat(strIndent, strAsc); 
	AppendThdId(strIndent);
	AppendTime(strIndent);
	AppendSN(strIndent); 
	strcat(strIndent,"\n");

	#ifdef WIN32
	OutputDebugString ((const char *)strIndent);
	#endif
	fprintf(fp,(const char *)strIndent);
	fflush(fp);
	writecounter++;

#endif // _DEBUG_TRACE
}


// ---------------------------------------------------------------------------
// TraceIOErr()
// 
// Many IO functions return errors through ::GetLastError(). This will obtain 
// and display in the output window the string that matches the error from the 
// operating system (unless trace suppression is active or this is a relase 
// compile). 
//
// GetLastError() will be left as is. 
// 
// For more info, see help on FormatMessage() or GetLastError().
// 
// ---------------------------------------------------------------------------
void CIndentedTrace::TraceIOErr()
{
#ifdef _DEBUG_TRACE

	if (ms_iSuppressDepth != 0)
	{
		// Tracing is suppressed.
		return;
	}

	LPVOID lpMsgBuf;		// Ptr to buffer (allocated by FormatMessage()
	char ErrMsg[STR_LEN];
	ErrMsg[0] = '\0';

	FormatMessage
	( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
		NULL, 
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf, 
		0, 
		NULL 
	);

	sprintf(ErrMsg,"GetLastError() reports %d - ", GetLastError());
	strcat(ErrMsg, (const char *) lpMsgBuf);
	Comment(ErrMsg);

	LocalFree( lpMsgBuf );

#endif // _DEBUG_TRACE
}

void CIndentedTrace::Sprintf( const char *format, ... )
{
#ifdef _DEBUG_TRACE
	m_msg[0] ='\0';
  	va_list ap;
    va_start( ap, format );
    vsprintf( m_msg, format, ap );
    va_end( ap );
#else // _DEBUG_TRACE
#endif // _DEBUG_TRACE
}
