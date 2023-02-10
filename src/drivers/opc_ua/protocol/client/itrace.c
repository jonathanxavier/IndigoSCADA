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

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include "itrace.h"
#include <assert.h>
#ifdef USE_UUID
#include <rpc.h>
#endif
#include <signal.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////
// Initialize static in cpp or global in c data members.
//Le variabili static in c sono globali (e quindi inizializzate a 0) e hanno scope di file,
//cioe' sono visibili sollo alle funzioni contenute nel file
//Le varibili globali sono visibili in tutti i files mediante la extern

#ifdef _DEBUG_TRACE

	// Set initial indent level to no indentation. 
static int ms_iTraceDepth[MAX_THREADS_IN_PROCESS]; 
	// Deactivate trace suppression. See SuppressTrace() for more info.  
static int ms_iSuppressDepth = 0;	

	// Initialize to 0 so first serial number will be 1. 
static int ms_iSerialNumber = 0;

	// By default, do not display serial numbers.
static int ms_bEnableSerialNumber = 0;
// By default, do display serial thread id.
#ifndef LOG_IN_ONE_FILE
static int ms_bEnableThreadID = 0;
#else
static int ms_bEnableThreadID = 1;
#endif
static int active_thread = 0;

#ifdef ENABLE_TIME_STAMP
static int ms_bEnableTime = 1;
#else
static int ms_bEnableTime = 0;
#endif

static int seeClassFuncOnExit = 1;

static FILE * fp[MAX_THREADS_IN_PROCESS];

static unsigned long writecounter[MAX_THREADS_IN_PROCESS];

#ifdef ENABLE_TIME_STAMP
	#ifdef CPU_TICKER
	static CCPUTicker* m_tick1 = NULL;
	static CCPUTicker* m_tick2 = NULL;
	#else
	static unsigned long t1 = 0;
	static unsigned long t2 = 0;
	#endif

#endif

unsigned char thread_idx[MAXIMUM_THREAD_INDEX_GENERATED_BY_THE_OPERATING_SYSTEM];
int thread_counter = 0;
unsigned long memory_log_counter[MAX_THREADS_IN_PROCESS];
char *memory_log[MAX_THREADS_IN_PROCESS];
unsigned long memory_size[MAX_THREADS_IN_PROCESS];

/////////critical  section////////////////////////////////////////////////
static int are_critical_section_initialized = 0;
#define TRACE_RESOURCE_QUANTITY 1
static CRITICAL_SECTION traceCriticalSections[TRACE_RESOURCE_QUANTITY];

#define TRACE_RESOURCE_UNLOCK(resourceNum) \
		(LeaveCriticalSection(&traceCriticalSections[resourceNum]))

#define TRACE_RESOURCE_LOCK(resourceNum) \
		(EnterCriticalSection(&traceCriticalSections[resourceNum]))

///////////////////////////////////////////////////////////////////////////

#ifdef LOG_INTO_MEMORY
#ifdef TRACE_SIGNAL_HANDELR
static void __cdecl SignalHandler (int signal)
{
	DumpMemoryLog(NULL);
    return;
}
#endif
#endif // LOG_INTO_MEMORY


static int __cdecl GetCurrentThreadIndex()
{
	int index = 1;
	#ifndef LOG_IN_ONE_FILE
	index = thread_idx[GetCurrentThreadId()];
	#endif
	return index;
}

static int __cdecl GetCurrentThreadIndexForIndent()
{
	int index = 1;
	index = thread_idx[GetCurrentThreadId()];
	return index;
}

//////////////////////////////////////////////////////////////////////
// Implement private helper functions. 

int __cdecl GetSN(IndentedTrace* This)
{
	IndentedTrace* obj = This;
	return obj->m_nLocalSerialNumber;
}

// ---------------------------------------------------------------------------
// AppendSN()
// 
// Appends the serial number of this instance to strText (unless modified by 
// trace suppression or relase compilation). 
// 
// strText - IN. String to which the serial number will be appended. 
// 
// Each time a IndentedTrace object is constructed, the constructor reserves 
// a serial number for object. Serial numbers start at 1, and are incremented 
// each time the constructor is called. 
// 
// This function is only called inside IndentedTrace functions that are 
// changed when tracing is suppressed. 
// 
// ---------------------------------------------------------------------------
void __cdecl AppendSN(IndentedTrace* This, char* strText)
{
	if (ms_bEnableSerialNumber)
	{
		IndentedTrace* obj = This;
		char strLSN[STR_LEN];
		sprintf(strLSN,"\t\t// S/N %d", obj->m_nLocalSerialNumber);
		strcat(strText, strLSN);
		if(strlen(strText) > STR_LEN) assert(0);
	}
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
void __cdecl AppendThdId(IndentedTrace* This, char* strText)
{
	int c_t;
	//TRACE_RESOURCE_LOCK(0);

	if (ms_bEnableThreadID)
	{
		c_t = GetCurrentThreadId();

		if (c_t != active_thread)
		{
			char strThdId[STR_LEN];
			sprintf(strThdId,"\t\t//Thd ID 0x%x->0x%x", (unsigned)active_thread, (unsigned)c_t);
			strcat(strText, strThdId);
			if(strlen(strText) > STR_LEN) assert(0);
			active_thread = c_t;
		}
	}

	//TRACE_RESOURCE_UNLOCK(0);

}

// ---------------------------------------------------------------------------
// AppendTime()
// 
// Appends the Time elapsed since the constructor was callen. 
// 
// strText - IN. String to which the thread ID will be appended. 
//
// ---------------------------------------------------------------------------

#ifdef ENABLE_TIME_STAMP

void __cdecl AppendTime(IndentedTrace* This,char* strText)
{
	if(ms_bEnableTime)
	{
		char str[STR_LEN];
		unsigned long t;
		#ifdef CPU_TICKER
		m_tick2->Measure();
		sprintf(str,"\t\t%f s.",m_tick2->GetTickCountAsSeconds() - m_tick1->GetTickCountAsSeconds());
		#else
		t2 = GetTickCount();

		t = t2 - t1;
		sprintf(str,"\t\t%u", t);
		#endif
		
		strcat(strText, str);
		if(strlen(strText) > STR_LEN) assert(0);
	}
}

#endif

//////////////////////////////////////////////////////////////////////
// Constructors/Destructors

// ---------------------------------------------------------------------------
// IndentedTrace( const char * )
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

#include <time.h>
#include <sys/timeb.h>

void __cdecl Constructor(IndentedTrace* This, const char * strClass_Func)
{
		IndentedTrace* obj = This;
		char log_file_name[_MAX_PATH];
		char strThdId[STR_LEN];
		#ifdef USE_UUID
		UUID uuid;
		unsigned char* str;
		#endif

		#ifdef USE_PROCESS_ID
		char buffer[20];
		char sz[50];
		struct timeb tb;
		struct tm *ptm;
		#endif
	    int i;
		//char parc[50];
		
		if(thread_idx[GetCurrentThreadId()] == 0) //or not initialized
		{
			thread_counter++; //abbiamo trovato un nuovo thread
			thread_idx[GetCurrentThreadId()] = thread_counter;

			//sprintf(parc, "thread_counter = %d\n", thread_counter);

			//OutputDebugString(parc);

			#ifdef WIN32
			
			#ifdef USE_UUID
			UuidCreate(&uuid);
			UuidToString(&uuid, &str);
			#endif

			log_file_name[0] = '\0';

			if(GetModuleFileName(NULL, log_file_name, _MAX_PATH))
			{
				*(strrchr(log_file_name, '\\')) = '\0';        // Strip \\filename.exe off path
				*(strrchr(log_file_name, '\\')) = '\0';        // Strip \\bin off path
    
				#ifdef USE_PROCESS_ID
				strcat(log_file_name,"\\logs\\ProcessPID_");
				strcat(log_file_name, _ltoa(GetCurrentProcessId(),buffer,10));
				#endif
				#ifdef USE_UUID
				strcat(log_file_name,"\\logs\\Process_");
				strcat(log_file_name, (const char*)str);
				#endif
				#ifndef LOG_IN_ONE_FILE
				sprintf(strThdId,"_TheadID_%x", (unsigned)GetCurrentThreadId()); //thread ID
				strcat(log_file_name, strThdId);
				#else
				strThdId;//per tacitare lo warning
				#endif

				#ifdef USE_PROCESS_ID
				ftime (&tb);
				ptm = localtime(&tb.time);
				sprintf(sz, "_start_time_%d_%d_%d_%d_%d_%d",
				ptm->tm_hour,					//<0.23>
				ptm->tm_min,					//<0..59>
				ptm->tm_sec*1000 + tb.millitm, //<0.. 59999>
				ptm->tm_mday, //<1..31>
				ptm->tm_mon + 1, //<1..12>
				ptm->tm_year - 100); //<0.99>
				strcat(log_file_name, sz);
				#endif

				strcat(log_file_name,".c");

				#ifdef LOG_INTO_MEMORY
				#ifdef TRACE_SIGNAL_HANDELR

				#ifdef INTERCETTA_TERM
				signal (SIGTERM, SignalHandler);
				#endif

				#ifdef INTERCETTA_CONTROL_C
				signal (SIGINT, SignalHandler);
				#endif

				signal (SIGBREAK, SignalHandler);
				signal (SIGABRT, SignalHandler);
				signal (SIGFPE, SignalHandler);
				signal (SIGSEGV, SignalHandler);
				signal (SIGILL, SignalHandler);
				#endif
				#endif // LOG_INTO_MEMORY
			}
			
			#ifdef USE_UUID
			RpcStringFree(&str);
			#endif

			if(fp[GetCurrentThreadIndex()] == NULL)
			{
				fp[GetCurrentThreadIndex()] = fopen(log_file_name,"w+");

				if(fp[GetCurrentThreadIndex()] == NULL)
				{
					#ifndef LOG_INTO_MEMORY
					OutputDebugString("UNABLE TO OPEN FILE : \n");
					OutputDebugString(log_file_name);
					#endif
				//	sprintf(log_file_name,"Process_%d.cpp", GetCurrentProcessId());
				//	fp[GetCurrentThreadIndex()] = fopen(log_file_name,"w+");
				}
			}

			//allocate memory if log into memory is active
			#ifdef LOG_INTO_MEMORY
			if(memory_log[GetCurrentThreadIndex()] == NULL)
			{
				memory_size[GetCurrentThreadIndex()] = MEMORY_BLOCK;
				memory_log[GetCurrentThreadIndex()] = (char*)malloc(memory_size[GetCurrentThreadIndex()]);

				//sprintf(parc,"Allocata memoria per thread %d\n", GetCurrentThreadIndex());

				//OutputDebugString(parc);

				//sprintf(parc,"memory_log[%d] = %x\n", GetCurrentThreadIndex(), memory_log[GetCurrentThreadIndex()]);

				//OutputDebugString(parc);

				if(memory_log[GetCurrentThreadIndex()] == NULL)
				{
					assert(0);
				}
			}
			#endif

			if(!are_critical_section_initialized)
			{
				are_critical_section_initialized = 1;
				
				for (i = 0; i < TRACE_RESOURCE_QUANTITY; i++)
				{
					InitializeCriticalSection(&traceCriticalSections[i]);
				}
			}

			#ifdef ENABLE_TIME_STAMP
			if(thread_counter == 1)
			{
				//Init counters only at the first running thread
				if(ms_bEnableTime)
				{
					//initialize the timers.
					#ifdef CPU_TICKER
					m_tick1 = new CCPUTicker();
					m_tick2 = new CCPUTicker();
					m_tick1->GetTickCountAsSeconds();
					//start m_tick1 counter
					m_tick1->Measure();
					#else
					t1 = GetTickCount();
					#endif

				}
			}
			#endif
	
			#else //WIN32
			
			//TO DO: Find in UNIX the fnction like the GetModuleFileName
			sprintf(log_file_name,"Process_%d.cpp", GetCurrentProcessId());
			fp[GetCurrentThreadIndex()] = fopen(log_file_name,"w+");
			#endif  //UNIX

		}

		//TRACE_RESOURCE_LOCK(0);

		strcpy(obj->LocalClassNameFunc, strClass_Func);
		obj->m_nLocalSerialNumber = ++ms_iSerialNumber;

		Entry(obj, strClass_Func);

		//TRACE_RESOURCE_UNLOCK(0);
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
// created this instance of IndentedTrace. E.g. "CMyClass::MyFunc()".
// 
// When strClass_Func is printed in the output window, spaces are added to the 
// beginning to indent it, and an opening brace is added to the end. Depending 
// on ms_bEnableSerialNumber and ms_bEnableThreadID, a serial number and/or 
// thread ID may be added to the end as well. 
// 
// ---------------------------------------------------------------------------
void __cdecl Entry(IndentedTrace* This, const char * strClass_Func)
{
	IndentedTrace* obj = This;
	char strIndent[STR_LEN];
	char strText[STR_LEN];
	int i;
	//char parc[50];
	#ifdef LOG_INTO_MEMORY
	int str_length;
	#endif
		
	// Sanity checks.
	assert (ms_iTraceDepth[GetCurrentThreadIndexForIndent()] >= 0);
	assert ((ms_iTraceDepth[GetCurrentThreadIndexForIndent()] >= ms_iSuppressDepth) || (ms_iSuppressDepth == 0));
	
	obj->m_nLocalTraceDepth = ms_iTraceDepth[GetCurrentThreadIndexForIndent()];

	if (ms_iSuppressDepth == 0)
	{		// Suppression inactive. Do trace. 
		
		strIndent[0] = '\0';

		for(i = 0; i<=INDENT_SPACES * (obj->m_nLocalTraceDepth); i++)
		{
			strcat(strIndent," ");
			if(strlen(strIndent) > STR_LEN) assert(0);
		}
		
		strcpy(strText, strIndent);
		if(strlen(strText) > STR_LEN) assert(0);
		strcat(strText, strClass_Func);
		if(strlen(strText) > STR_LEN) assert(0);
		strcat(strText," {");
		if(strlen(strText) > STR_LEN) assert(0);

		AppendThdId(This,strText); 
		if(strlen(strText) > STR_LEN) assert(0);
		AppendTime(This,strText);
		AppendSN(This,strText);	   
		if(strlen(strText) > STR_LEN) assert(0);
		strcat(strText,"\n");
		if(strlen(strText) > STR_LEN) assert(0);
		
		#ifdef OUTPUT_DEBUG_STRING
		OutputDebugString((const char *)strText);
		#endif
		
		#ifndef LOG_INTO_MEMORY
		fprintf(fp[GetCurrentThreadIndex()],(const char *)strText);
		fflush(fp[GetCurrentThreadIndex()]);
		#else

		str_length = strlen(strText);
		
		if(memory_log_counter[GetCurrentThreadIndex()] >= MEMORY_LIMIT)
		{
		   memory_log_counter[GetCurrentThreadIndex()] = 0;
		}

		//th = GetCurrentThreadIndex();

		//sprintf(parc, "memory_log[th] = %x\n", memory_log[th]);
		//OutputDebugString(parc);
		//sprintf(parc, "memory_log_counter[th] = %d\n", memory_log_counter[th]);
		//OutputDebugString(parc);

		memcpy(memory_log[GetCurrentThreadIndex()] + memory_log_counter[GetCurrentThreadIndex()], strText, str_length);
		memory_log_counter[GetCurrentThreadIndex()] += str_length;
		#endif
		(writecounter[GetCurrentThreadIndex()])++;
	}
	
	++(ms_iTraceDepth[GetCurrentThreadIndexForIndent()]);

	++(obj->m_nLocalTraceDepth);
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
void __cdecl Exit(IndentedTrace* This, const char * strClass_Func)
{
	IndentedTrace* obj = This;
	int i, j;
	#ifdef LOG_INTO_MEMORY
	int str_length;
	#endif

	char strIndent[STR_LEN];
	char strText[STR_LEN];
	#ifdef LOG_INTO_MEMORY
	char new_line[2];
	#endif
		
	// Sanity checks.
	assert (ms_iTraceDepth[GetCurrentThreadIndexForIndent()] >= 0);
	assert ((ms_iTraceDepth[GetCurrentThreadIndexForIndent()] >= ms_iSuppressDepth) || (ms_iSuppressDepth == 0));

	--(ms_iTraceDepth[GetCurrentThreadIndexForIndent()]);
	
	--(obj->m_nLocalTraceDepth);
	
	if (ms_iTraceDepth[GetCurrentThreadIndexForIndent()] < ms_iSuppressDepth)
	{		// Exiting function that started suppression. Deactivate it.
		ms_iSuppressDepth = 0;
	}
	
	if (ms_iSuppressDepth == 0)
	{	// Suppression inactive. Do trace. 

		strIndent[0] = '\0';

		for(i = 0; i<=INDENT_SPACES * (obj->m_nLocalTraceDepth); i++)
		{
			strcat(strIndent," ");
			j = strlen(strIndent);
			if(j > STR_LEN) assert(0);
		}
		
		strText[0] = '\0';

		if(seeClassFuncOnExit)
		{
			strcpy(strText, strIndent);
			if(strlen(strText) > STR_LEN) assert(0);
			strcat(strText, "}");
			if(strlen(strText) > STR_LEN) assert(0);
			strcat(strText, "//");
			if(strlen(strText) > STR_LEN) assert(0);
			strcat(strText, strClass_Func);
			if(strlen(strText) > STR_LEN) assert(0);
		}
		else
		{
			strcat(strIndent, "}");
			if(strlen(strIndent) > STR_LEN) assert(0);
		}

		AppendThdId(obj, strText);  
		AppendTime(This,strText);
		AppendSN(obj, strText);
		strcat(strText,"\n");
		if(strlen(strText) > STR_LEN) assert(0);

		#ifdef OUTPUT_DEBUG_STRING
		OutputDebugString((const char *)strText);
		#endif

		#ifndef LOG_INTO_MEMORY
		fprintf(fp[GetCurrentThreadIndex()],(const char *)strText);
		fflush(fp[GetCurrentThreadIndex()]);
		#else

		str_length = strlen(strText);

		if(memory_log_counter[GetCurrentThreadIndex()] >= MEMORY_LIMIT)
		{
		   memory_log_counter[GetCurrentThreadIndex()] = 0;
		}

		memcpy(memory_log[GetCurrentThreadIndex()] + memory_log_counter[GetCurrentThreadIndex()], strText, str_length);
		memory_log_counter[GetCurrentThreadIndex()] += str_length;
		#endif

		(writecounter[GetCurrentThreadIndex()])++;
		
		if ((obj->m_nLocalTraceDepth) == 0)
		{		// Insert extra line to separate blocks
			
			#ifdef OUTPUT_DEBUG_STRING
			OutputDebugString ((const char *)"\n"); 
			#endif

			#ifndef LOG_INTO_MEMORY
			fprintf(fp[GetCurrentThreadIndex()],(const char *)"\n");
			fflush(fp[GetCurrentThreadIndex()]);
			#else

			if(memory_log_counter[GetCurrentThreadIndex()] >= MEMORY_LIMIT)
			{
			   memory_log_counter[GetCurrentThreadIndex()] = 0;
			}

			strcpy(new_line, "\n");
			memcpy(memory_log[GetCurrentThreadIndex()] + memory_log_counter[GetCurrentThreadIndex()], new_line, 1);
			memory_log_counter[GetCurrentThreadIndex()] += 1;
			#endif
			(writecounter[GetCurrentThreadIndex()])++;
		}
	}

	#ifdef LOG_ONLY_LAST_PART
	if((writecounter[GetCurrentThreadIndex()]) > 100000UL)
	{
		(writecounter[GetCurrentThreadIndex()]) = 0;

		if(fseek(fp[GetCurrentThreadIndex()], 0, SEEK_SET))
		{
			assert(0);			
		}
	}
	#endif
}

// ---------------------------------------------------------------------------
// ~IndentedTrace()
// 
// The destructor decrements the indent level and prints an indented closing 
// brace (unless modified by trace suppression or relase compilation).
// 
// 
// ---------------------------------------------------------------------------
void __cdecl Destructor(IndentedTrace* This)
{
		IndentedTrace* obj = This;

		//TRACE_RESOURCE_LOCK(0);

		Exit(obj, obj->LocalClassNameFunc);

		//TRACE_RESOURCE_UNLOCK(0);
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
void __cdecl Comment(IndentedTrace* This, const char * strComment)
{
	IndentedTrace* obj = This;
	int i;
	
	#ifdef LOG_INTO_MEMORY
	int str_length;
	#endif

	//TRACE_RESOURCE_LOCK(0);
		
	// Print an indented comment. 
	if(!strComment) return;

	if (ms_iSuppressDepth == 0)
	{
		char strIndent[STR_LEN];
		strIndent[0] = '\0';

		for(i = 0; i<=INDENT_SPACES * (obj->m_nLocalTraceDepth); i++)
		{
			strcat(strIndent," ");
			if(strlen(strIndent) > STR_LEN) assert(0);
		}

		strcat(strIndent,"// "); 
		if(strlen(strIndent) > STR_LEN) assert(0);
		strcat(strIndent, strComment); 
		if(strlen(strIndent) > STR_LEN) assert(0);
		AppendThdId(This, strIndent);
		AppendTime(This,strIndent);
		AppendSN(This, strIndent); 
		strcat(strIndent,"\n");
		if(strlen(strIndent) > STR_LEN) assert(0);

		#ifdef OUTPUT_DEBUG_STRING
		OutputDebugString ((const char *)strIndent);
		#endif

		#ifndef LOG_INTO_MEMORY
		fprintf(fp[GetCurrentThreadIndex()],(const char *)strIndent);
		fflush(fp[GetCurrentThreadIndex()]);
		#else
		
		str_length = strlen(strIndent);

		if(memory_log_counter[GetCurrentThreadIndex()] >= MEMORY_LIMIT)
		{
		   memory_log_counter[GetCurrentThreadIndex()] = 0;
		}


		memcpy(memory_log[GetCurrentThreadIndex()] + memory_log_counter[GetCurrentThreadIndex()], strIndent, str_length);
		memory_log_counter[GetCurrentThreadIndex()] += str_length;
		
		#endif
		(writecounter[GetCurrentThreadIndex()])++;
		
	}

	#ifdef LOG_ONLY_LAST_PART
	if((writecounter[GetCurrentThreadIndex()]) > 100000UL)
	{
		(writecounter[GetCurrentThreadIndex()]) = 0;

		if(fseek(fp[GetCurrentThreadIndex()], 0, SEEK_SET))
		{
			assert(0);			
		}
	}
	#endif

	//TRACE_RESOURCE_UNLOCK(0);
}


// ---------------------------------------------------------------------------
// SuppressTrace()
// 
// Suppresses action of IndentedTrace functions until the the instance where 
// SuppressTrace() is called exits (unless this is a release compile). 
// 
// This is function was intended to prevent functions that produce a lot of 
// IndentedTrace output from cluttering the output window. It turns out not 
// to be very useful. 
// 
// Each IndentedTrace function that produces output checks ms_iSuppressDepth. 
// If ms_iSuppressDepth > 0, suppression is active, and the IndentedTrace 
// function does little or nothing. 
// 
// Normally, ms_iSuppressDepth is 0, which disables suppression. 
// 
// Suppose a function, MyFunc(), creates an instance of IndentedTrace and 
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
void __cdecl SuppressTrace(IndentedTrace* This)
{
	// IndentedTrace functions will do no further tracing until the function 
	// that called SuppressTrace() exits.
	//
	IndentedTrace* obj = This;
	char strIndent[STR_LEN];
	int i;

	#ifdef LOG_INTO_MEMORY
	int str_length;
	#endif

	assert ((obj->m_nLocalTraceDepth) > 0);

	if (ms_iSuppressDepth == 0)
	{		// Start trace suppression only if it is not already started.
		ms_iSuppressDepth = (obj->m_nLocalTraceDepth); 

		// Show suppression is starting.
				
		strIndent[0] = '\0';

		for(i = 0; i<=INDENT_SPACES * (obj->m_nLocalTraceDepth); i++)
		{
			strcat(strIndent," ");
			if(strlen(strIndent) > STR_LEN) assert(0);
		}
	
		strcat(strIndent,"// IndentedTrace is suppressed for the rest of this function\r\n");
		if(strlen(strIndent) > STR_LEN) assert(0);

		#ifdef OUTPUT_DEBUG_STRING
		OutputDebugString ((const char*)strIndent);
		#endif

		#ifndef LOG_INTO_MEMORY
		fprintf(fp[GetCurrentThreadIndex()],(const char *)strIndent);
		fflush(fp[GetCurrentThreadIndex()]);
		#else

		str_length = strlen(strIndent);

		if(memory_log_counter[GetCurrentThreadIndex()] >= MEMORY_LIMIT)
		{
		   memory_log_counter[GetCurrentThreadIndex()] = 0;
		}

		memcpy(memory_log[GetCurrentThreadIndex()] + memory_log_counter[GetCurrentThreadIndex()], strIndent, str_length);
		memory_log_counter[GetCurrentThreadIndex()] += str_length;
		#endif
		(writecounter[GetCurrentThreadIndex()])++;
	}
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
void __cdecl DumpStr(IndentedTrace* This, const char * str, const UINT NChars)
{
	IndentedTrace* obj = This;
	char strAsc[STR_LEN];					// Chars as ASCII
	unsigned int k = 0;
	unsigned int j;
	char strIndent[STR_LEN];
	int i;

	#ifdef LOG_INTO_MEMORY
	int str_length;
	#endif

	if (ms_iSuppressDepth != 0)
	{
		// Tracing is suppressed.
		return;
	}
	
	strAsc[0] = '\0';
	
	for (j = 0; j < NChars; ++j)
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
		
	strIndent[0] = '\0';

	for(i = 0; i<=INDENT_SPACES * (obj->m_nLocalTraceDepth); i++)
	{
		strcat(strIndent," ");
		if(strlen(strIndent) > STR_LEN) assert(0);
	}

	strcat(strIndent,"// "); 
	if(strlen(strIndent) > STR_LEN) assert(0);
	strcat(strIndent, strAsc);
	if(strlen(strIndent) > STR_LEN) assert(0);
	AppendThdId(This, strIndent);
	AppendTime(This,strIndent);
	AppendSN(This, strIndent); 
	strcat(strIndent,"\n");
	if(strlen(strIndent) > STR_LEN) assert(0);

	#ifdef OUTPUT_DEBUG_STRING
	OutputDebugString ((const char *)strIndent);
	#endif

	#ifndef LOG_INTO_MEMORY
	fprintf(fp[GetCurrentThreadIndex()],(const char *)strIndent);
	fflush(fp[GetCurrentThreadIndex()]);
	#else

	str_length = strlen(strIndent);

	if(memory_log_counter[GetCurrentThreadIndex()] >= MEMORY_LIMIT)
	{
	   memory_log_counter[GetCurrentThreadIndex()] = 0;
	}

	memcpy(memory_log[GetCurrentThreadIndex()] + memory_log_counter[GetCurrentThreadIndex()], strIndent, str_length);
	memory_log_counter[GetCurrentThreadIndex()] += str_length;
	#endif
	(writecounter[GetCurrentThreadIndex()])++;
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
void __cdecl TraceIOErr(IndentedTrace* This)
{
	IndentedTrace* obj = This;
	char ErrMsg[STR_LEN];
	LPVOID lpMsgBuf;		// Ptr to buffer (allocated by FormatMessage()

	if (ms_iSuppressDepth != 0)
	{
		// Tracing is suppressed.
		return;
	}
	
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
	Comment(This, ErrMsg);

	LocalFree(lpMsgBuf);
}

void __cdecl Sprintf(IndentedTrace* This, const char *format, ... )
{
	IndentedTrace* obj = This;
	va_list ap;

	//TRACE_RESOURCE_LOCK(0);

	obj->m_msg[0] ='\0';
  	va_start(ap, format);
    vsprintf(obj->m_msg, format, ap);
	if(strlen(obj->m_msg) > STR_LEN_FOR_COMMENT) assert(0);
    va_end(ap);

	//TRACE_RESOURCE_UNLOCK(0);
}

void __cdecl DumpMemoryLog(IndentedTrace* This)
{
#ifdef LOG_INTO_MEMORY
	int i;
	char null = '\0';

	for(i = 1; i <= thread_counter; i++)
	{
		if(fp[i])
		{
			if(memory_log[i])
			{
				memcpy(memory_log[i] + memory_log_counter[i], &null, 1);
				//fprintf stampa finche' non trova un carattere '\0'
				fprintf(fp[i],"%s",(const char *)memory_log[i]);
				fflush(fp[i]);
			}

			fclose(fp[i]);
		}
	}

	if(are_critical_section_initialized)
	{
		are_critical_section_initialized = 0;
		
		for (i = 0; i < TRACE_RESOURCE_QUANTITY; i++)
		{
			DeleteCriticalSection(&traceCriticalSections[i]);
		}
	}

#endif // LOG_INTO_MEMORY
}

#endif // _DEBUG_TRACE