/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2010 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef I_TRACE_H
#define I_TRACE_H

// itrace.h: interface for the IndentedTrace struct.
//
// Tracing macros
// 
// Copyright 2001-2010 Enscada
///////////////////////////////////////////////////////////////////////
//#define _DEBUG_TRACE				//<------abilita/disabilita i tracciati: EDIT ONLY HERE!

#ifdef _DEBUG_TRACE

	//#define LOG_IN_ONE_FILE		//<------log di tutti i thread in un solo file
	//#define SPEED_OF_EXECUTION	//<------se non si vuole che la fprintf() rallenti l'esecuzione de programma
	#define CAPTURE_ALL				//<------ scommentare per catturare tutto il log
	//#define OUTPUT_DEBUG_STRING	//<------define this to see the trace into the debugger. Se definita ed il programma avviato dal compilatore con F5, si possono verificare corruzioni dello stack, corruzione dell' ESP; DO NOT use for production code
	//#define TRACE_SIGNAL_HANDELR  //<---define this if the main has NOT his own signal handler function
	//#define INTERCETTA_CONTROL_C  //da scommentare quando si ha un main semplice
	//#define INTERCETTA_TERM

    #define ENABLE_TIME_STAMP		//<----------abilita il time stamp

	#ifdef LOG_IN_ONE_FILE
	#define USE_UUID				//<----------each log file is identified by UUID
	#else
	#define USE_PROCESS_ID			//<----------each log file is identified by PROCESSID_THREADID	
	#endif

	//////////////DO NOT EDIT BELOW/////////////////////////////////////////
	#ifdef SPEED_OF_EXECUTION
		#define LOG_INTO_MEMORY  //define this if you need speed of execution
		#ifdef CAPTURE_ALL
			#define MEMORY_BLOCK 50000000UL
			#define MEMORY_LIMIT 50000000UL
			//#pragma comment(linker, "/heap:0xBEBC200")
		#else
			#define MEMORY_BLOCK 10000000UL
			#define MEMORY_LIMIT 10000000UL
			//#pragma comment(linker, "/heap:0x989680")
		#endif
	#else
		#define ENABLE_PRINTF  //<------abilita i tracciati su disco con fprintf, non abilitare per codice in produzione!
		#ifndef CAPTURE_ALL
		#define LOG_ONLY_LAST_PART //trace only the last part
		#endif
	#endif

	#ifdef CAPTURE_ALL
		#undef CAPTURE_ALL
	#endif
	#ifdef SPEED_OF_EXECUTION
		#undef SPEED_OF_EXECUTION
	#endif
	///////////////////////////////////////////////////////////////////////

	#include <stdio.h>
    #ifdef CPU_TICKER
	#include "CPUTicker.h"
	#endif
	//Configuration parameters
	#define MAX_LENGHT_OF_NAME_FUNCTION 255
	#define MAX_THREADS_IN_PROCESS 40
	#define STR_LEN 2000
	#define STR_LEN_FOR_COMMENT 500 //<--questo parametro potrebbbe essere piccolo!
	#define MAXIMUM_THREAD_INDEX_GENERATED_BY_THE_OPERATING_SYSTEM 10000 //<---questo dipende da quanti thread stanno girando in un sistema operativo
	#define INDENT_SPACES 3

	typedef struct CIndentedTrace_ IndentedTrace;

	#ifdef __cplusplus
	extern "C"
	{
	#endif

	//external functions
	extern int __cdecl GetSN(IndentedTrace* This);
	extern void __cdecl AppendSN(IndentedTrace* This, char* strText);
	extern void __cdecl AppendThdId(IndentedTrace* This, char* strText);
	extern void __cdecl AppendTime(IndentedTrace* This, char* strText);
	extern void __cdecl Entry(IndentedTrace* This, const char * strClass_Func);
	extern void __cdecl Constructor(IndentedTrace* This, const char * strClass_Func);
	extern void __cdecl Exit(IndentedTrace* This, const char * strClass_Func);
	extern void __cdecl Destructor(IndentedTrace* This);
	extern void __cdecl Comment(IndentedTrace* This, const char * strComment);
	extern void __cdecl SuppressTrace(IndentedTrace* This);
	extern void __cdecl DumpStr(IndentedTrace* This, const char * str, const unsigned int NChars);
	extern void __cdecl TraceIOErr(IndentedTrace* This);
	extern void __cdecl Sprintf(IndentedTrace* This, const char *format, ... );
	extern void __cdecl DumpMemoryLog(IndentedTrace* This);

	#ifdef __cplusplus
	}
	#endif

	/* type definition for functions*/

	typedef int(__cdecl *GET_SN)(IndentedTrace*);   
	typedef	void(__cdecl *APPEND_SN)(IndentedTrace*,char* strText);
	typedef	void(__cdecl *APPEND_THD_ID)(IndentedTrace*,char* strText);
	typedef	void(__cdecl *APPEND_TIME)(IndentedTrace*,char* strText);
	typedef void(__cdecl *ENTRY)(IndentedTrace*,const char * strClass_Func);
	typedef void(__cdecl *EXIT)(IndentedTrace*,const char * strClass_Func);
	typedef void(__cdecl *INDENTED_TRACE_CONSTRUCTOR)(IndentedTrace*,const char * strClass_Func);
	typedef void(__cdecl *INDENTED_TRACE_DESTRUCTOR)(IndentedTrace*);
	typedef void(__cdecl *COMMENT)(IndentedTrace*,const char * strComment);
	typedef void(__cdecl *SUPPRESS_TRACE)(IndentedTrace*);
	typedef void(__cdecl *DUMP_STR)(IndentedTrace*,const char * str, const unsigned int NChars);
	typedef void(__cdecl *TRACE_IO_ERR)(IndentedTrace*);
	typedef void(__cdecl *S_PRINTF)(IndentedTrace*,const char* format, ... );
	typedef void(__cdecl *DUMP_MEMORY_LOG)(IndentedTrace*);
	typedef void(__cdecl *ALLOCATE_MEMORY_LOG)(IndentedTrace*);

	struct CIndentedTrace_
	{
		int m_nLocalTraceDepth;
		int m_nLocalSerialNumber;
		char LocalClassNameFunc[MAX_LENGHT_OF_NAME_FUNCTION];
		
		// For use by macros like IT_COMMENTn. May be changed at any time by 
		// internal functions. Must be public so macros can use it. 
		char m_msg[STR_LEN_FOR_COMMENT];

		GET_SN GetSN;

		APPEND_SN AppendSN;

		APPEND_THD_ID AppendThdId;

		APPEND_TIME AppendTime;

		ENTRY Entry;

		EXIT Exit;	

		INDENTED_TRACE_CONSTRUCTOR Constructor;

		INDENTED_TRACE_DESTRUCTOR Destructor;

		COMMENT	Comment;

		SUPPRESS_TRACE SuppressTrace;

		DUMP_STR DumpStr;

		TRACE_IO_ERR TraceIOErr;

		S_PRINTF Sprintf;

		DUMP_MEMORY_LOG DumpMemoryLog;
	};


//////////////////////////////////////////////////////////////////////
// MACROS
//
// IndentedTrace is a diagnostic class. It is intended to be like TRACE - 
// it is available for use in debug compiles but disappears during release 
// compiles. This is done by putting macros in source code instead of using 
// IndentedTrace directly. 

// --------------------------------------------------------------------------
// In debug builds, the IT_IT() and IT_EMBEDDED_IT macros create a variable of 
// type IndentedTrace. It is given the name _IT_vnirrrdaewu because this is 
// an unlikely name for a program to contain. 
// 
// The other macros all use _IT_vnirrrdaewu.

#define IT_IT(str)				IndentedTrace _IT_vnirrrdaewu ={ \
0,0,"","",GetSN,AppendSN, \
AppendThdId,AppendTime,Entry,Exit, \
Constructor, \
Destructor, \
Comment, \
SuppressTrace, \
DumpStr, \
TraceIOErr, \
Sprintf, \
DumpMemoryLog }; \
_IT_vnirrrdaewu.Constructor((&_IT_vnirrrdaewu),(str))
								
#define IT_EMBEDDED_IT			IndentedTrace _IT_vnirrrdaewu
#define IT_EMBEDDED_INIT(str)	_IT_vnirrrdaewu((&_IT_vnirrrdaewu),(str))
#define IT_ENTRY(str)			_IT_vnirrrdaewu.Entry((&_IT_vnirrrdaewu),(str))
#define IT_EXIT					_IT_vnirrrdaewu.Destructor(&_IT_vnirrrdaewu);
#define IT_COMMENT(str)			_IT_vnirrrdaewu.Comment((&_IT_vnirrrdaewu),(str))
#define IT_SUPPRESS_TRACE		_IT_vnirrrdaewu.SuppressTrace(&_IT_vnirrrdaewu)
#define IT_DUMP_STR(str, n)		_IT_vnirrrdaewu.DumpStr((&_IT_vnirrrdaewu),(str), (n))
#define IT_TRACE_IO_ERR			_IT_vnirrrdaewu.TraceIOErr(&_IT_vnirrrdaewu)
#define IT_DUMP_MEMORY_LOG		_IT_vnirrrdaewu.DumpMemoryLog(&_IT_vnirrrdaewu)
#define IT_COMMENT1(str, x1)	_IT_vnirrrdaewu.Sprintf((&_IT_vnirrrdaewu),(str),(x1));	\
_IT_vnirrrdaewu.Comment((&_IT_vnirrrdaewu),(_IT_vnirrrdaewu.m_msg))
#define IT_COMMENT2(str, x1, x2) \
_IT_vnirrrdaewu.Sprintf((&_IT_vnirrrdaewu),(str),(x1),(x2)); \
_IT_vnirrrdaewu.Comment((&_IT_vnirrrdaewu),(_IT_vnirrrdaewu.m_msg))
#define IT_COMMENT3(str, x1, x2, x3) \
_IT_vnirrrdaewu.Sprintf((&_IT_vnirrrdaewu),(str),(x1),(x2),(x3)); \
_IT_vnirrrdaewu.Comment((&_IT_vnirrrdaewu),(_IT_vnirrrdaewu.m_msg))
#define IT_COMMENT4(str, x1, x2, x3, x4) \
_IT_vnirrrdaewu.Sprintf((&_IT_vnirrrdaewu),(str),(x1),(x2),(x3),(x4)); \
_IT_vnirrrdaewu.Comment((&_IT_vnirrrdaewu),(_IT_vnirrrdaewu.m_msg))
#define IT_COMMENT5(str, x1, x2, x3, x4, x5) \
_IT_vnirrrdaewu.Sprintf((&_IT_vnirrrdaewu),(str),(x1),(x2),(x3),(x4),(x5));	\
_IT_vnirrrdaewu.Comment((&_IT_vnirrrdaewu),(_IT_vnirrrdaewu.m_msg))
#define IT_COMMENT6(str, x1, x2, x3, x4, x5, x6) \
_IT_vnirrrdaewu.Sprintf((&_IT_vnirrrdaewu),(str),(x1),(x2),(x3),(x4),(x5),(x6));	\
_IT_vnirrrdaewu.Comment((&_IT_vnirrrdaewu),(_IT_vnirrrdaewu.m_msg))
#define IT_COMMENT7(str, x1, x2, x3, x4, x5, x6, x7) \
_IT_vnirrrdaewu.Sprintf((&_IT_vnirrrdaewu),(str),(x1),(x2),(x3),(x4),(x5),(x6),(x7));	\
_IT_vnirrrdaewu.Comment((&_IT_vnirrrdaewu),(_IT_vnirrrdaewu.m_msg))
#define IT_COMMENT8(str, x1, x2, x3, x4, x5, x6, x7, x8) \
_IT_vnirrrdaewu.Sprintf((&_IT_vnirrrdaewu),(str),(x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8));	\
_IT_vnirrrdaewu.Comment((&_IT_vnirrrdaewu),(_IT_vnirrrdaewu.m_msg))
#define IT_COMMENT9(str, x1, x2, x3, x4, x5, x6, x7, x8, x9) \
_IT_vnirrrdaewu.Sprintf((&_IT_vnirrrdaewu),(str),(x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9));	\
_IT_vnirrrdaewu.Comment((&_IT_vnirrrdaewu),(_IT_vnirrrdaewu.m_msg))

//#ifndef NO_RETURN

//#define return(value)	\
//{					\
//_IT_vnirrrdaewu.Destructor(&_IT_vnirrrdaewu);			\
//return(value);	\
//}					

//#endif

//#define exit(value)	\
//{					\
//_IT_vnirrrdaewu.Destructor(&_IT_vnirrrdaewu);			\
//exit(value);	\
//}

#else // _DEBUG_TRACE	not defined	// --- Release builds ---

// In release builds, the macros are defined as nothing, much like TRACE().

#define IT_IT(str)
#define IT_EMBEDDED_IT
#define IT_ENTRY(str)			
#define IT_EXIT					
#define IT_COMMENT(str)			
#define IT_SUPPRESS_TRACE		
#define IT_DUMP_STR(str, n)	
#define IT_TRACE_IO_ERR	
#define IT_DUMP_MEMORY_LOG
#define IT_COMMENT1(str, x1)
#define IT_COMMENT2(str, x1, x2)
#define IT_COMMENT3(str, x1, x2, x3)
#define IT_COMMENT4(str, x1, x2, x3, x4)
#define IT_COMMENT5(str, x1, x2, x3, x4, x5)
#define IT_COMMENT6(str, x1, x2, x3, x4, x5, x6)
#define IT_COMMENT7(str, x1, x2, x3, x4, x5, x6, x7)
#define IT_COMMENT8(str, x1, x2, x3, x4, x5, x6, x7, x8)
#define IT_COMMENT9(str, x1, x2, x3, x4, x5, x6, x7, x8, x9)
#endif // _DEBUG_TRACE

#endif //I_TRACE_H
