// IndentedTrace.h: interface for the CIndentedTrace class.
//
// Better tracing class and macros
// 
// 
///////////////////////////////////////////////////////////////////////
//EDIT ONLY HERE!
//define the next line if you need TRACE DEBUGGING
//#define _DEBUG_TRACE 
//#define LOG_ONLY_LAST_PART //trace only the last part
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#if !defined(INDENTED_TRACE_H)
#define INDENTED_TRACE_H

#ifndef TRACE_DLL_EXP_IMP
	#ifdef WIN32
		#ifdef TRACE_EXPORTS
		#define TRACE_DLL_EXP_IMP  __declspec(dllexport)
		#else 
		#define TRACE_DLL_EXP_IMP __declspec(dllimport)
		#endif
	#else
		// under UNIX we have no such thing as exports
		#define TRACE_DLL_EXP_IMP 
	#endif
#endif


#include <stdio.h>
#include "CPUTicker.h"

#define STR_LEN 50000

class TRACE_DLL_EXP_IMP CIndentedTrace  
{
#ifdef _DEBUG_TRACE

private:
	
	static int ms_iTraceDepth;
	static int ms_iSuppressDepth;
	static int ms_iSerialNumber;

	static bool ms_bEnableSerialNumber;
	static bool ms_bEnableThreadID;
	static bool ms_bEnableTime;
	static unsigned mainThread;//apa added
	static bool seeClassFuncOnExit;
	static FILE* fp;
	static unsigned long writecounter;

	static CCPUTicker* m_tick1;
	static CCPUTicker* m_tick2;

	int m_nLocalTraceDepth;
	int m_nLocalSerialNumber;
	char LocalClassNameFunc[255];

	int loc_thread_id;
	
	enum 
	{ 
		INDENT_SPACES = 3
	};

public:
		// For use by macros like IT_COMMENTn. May be changed at any time by 
		// internal functions. Must be public so macros can use it. 
	char* m_msg;	

public:
	inline int GetSN()		const {return m_nLocalSerialNumber;}

#endif // _DEBUG_TRACE

private:
	void AppendSN(char* strText);
	void AppendThdId(char* strText);
	void AppendTime(char* strText);

public:
	void Entry(const char * strClass_Func);
	void Exit(const char * strClass_Func);

	static void EnableSerialNumber(bool bEnable);
	static void EnableThreadID(bool bEnable);

private:
	CIndentedTrace();	// Forbid default constructor. Declared, but not def'd.

public:
	CIndentedTrace(const char * strClass_Func);
	virtual ~CIndentedTrace();

	void Comment(const char * strComment);
	void SuppressTrace();

	void DumpStr(const char * str, const unsigned int NChars);
	void TraceIOErr();
	void Sprintf( const char* format, ... )
#if defined(_CC_GNU_) && !defined(__INSURE__)
	__attribute__ ((format (printf, 2, 3)))
#endif
	;

};


//////////////////////////////////////////////////////////////////////
// MACROS
//
// CIndentedTrace is a diagnostic class. It is intended to be like TRACE - 
// it is available for use in debug compiles but disappears during release 
// compiles. This is done by putting macros in source code instead of using 
// CIndentedTrace directly. 

#ifdef _DEBUG_TRACE		// --- Debug builds ---

// --------------------------------------------------------------------------
// In debug builds, the IT_IT() and IT_EMBEDDED_IT macros create a variable of 
// type CIndentedTrace. It is given the name _IT_vnirrrdaewu because this is 
// an unlikely name for a program to contain. 
// 
// The other macros all use _IT_vnirrrdaewu.

#define IT_IT(str)				CIndentedTrace _IT_vnirrrdaewu(str)
#define IT_EMBEDDED_IT			CIndentedTrace _IT_vnirrrdaewu
#define IT_EMBEDDED_INIT(str)	_IT_vnirrrdaewu(str)
#define IT_ENTRY(str)			_IT_vnirrrdaewu.Entry(str)
#define IT_EXIT					_IT_vnirrrdaewu.Exit()
#define IT_COMMENT(str)			_IT_vnirrrdaewu.Comment(str)
#define IT_SUPPRESS_TRACE		_IT_vnirrrdaewu.SuppressTrace()
#define IT_DUMP_STR(str, n)		_IT_vnirrrdaewu.DumpStr((str), (n))
#define IT_TRACE_IO_ERR			_IT_vnirrrdaewu.TraceIOErr()

#define IT_COMMENT1(str, x1)	_IT_vnirrrdaewu.Sprintf((str),(x1));		\
								_IT_vnirrrdaewu.Comment(_IT_vnirrrdaewu.m_msg)
#define IT_COMMENT2(str, x1, x2)											\
						_IT_vnirrrdaewu.Sprintf((str),(x1),(x2));		\
						_IT_vnirrrdaewu.Comment(_IT_vnirrrdaewu.m_msg)
#define IT_COMMENT3(str, x1, x2, x3)										\
						_IT_vnirrrdaewu.Sprintf((str),(x1),(x2),(x3));	\
						_IT_vnirrrdaewu.Comment(_IT_vnirrrdaewu.m_msg)

#define IT_COMMENT4(str, x1, x2, x3, x4)									\
						_IT_vnirrrdaewu.Sprintf((str),(x1),(x2),(x3),(x4));		\
						_IT_vnirrrdaewu.Comment(_IT_vnirrrdaewu.m_msg)

#define IT_ENABLE_SERIAL_NUM(bEnable)	EnableSerialNumber(bEnable)
#define IT_ENABLE_THREAD_ID(bEnable)	EnableThreadID(bEnable)

// --------------------------------------------------------------------------
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

#define IT_COMMENT1(str, x1)
#define IT_COMMENT2(str, x1, x2)
#define IT_COMMENT3(str, x1, x2, x3)

#define IT_ENABLE_SERIAL_NUM(bEnable)
#define IT_ENABLE_THREAD_ID(bEnable)

#endif // _DEBUG_TRACE

#endif 
