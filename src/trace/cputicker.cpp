// TITLE:
// High-Resolution Counter Class.
//
// VERSION:
// 1.24
//
// AUTHOR:
// Created by J.M.McGuiness, http://www.hussar.demon.co.uk
//            PJ Naughter,   Email: pjn@indigo.ie Web: http://indigo.ie/~pjn
//
// DESCRIPTION:
// This file declares a class the wraps the Pentium-specific time stamp counter.
// This counter has a resolution in terms of PCLKS (processor clocks) so it can
// be used for direct instruction timings.
//
// VERSION HISTORY:
// 26/3/96  J.M.McGuiness Initially created
// 16/7/97  PJ Naughter,
//              1. A number of additions including:
//              2. Support for running on Windows NT
//              3. now uses the build-in 64 bit data type "__int64"
//              4. Improved diagnostic info thanks to the above
//              5. Usage of static variables to improve efficiency
//              6. Addition of a function which will convert from CPU ticks to seconds
//              7. Improved adherence to the MFC coding style and standards
// 14/1/99  PJ Naughter
//              1. Fixed a bug discovered by David Green-Seed where he was experiencing 
//                 access violations when the code was compiled with optimizations turned 
//                 on a Pentium II. The problem was that (on a PII) the RDTSC instruction 
//                 touches more registers than expected. This has now been fixed by saved 
//                 and restoring the EAX and EBX registers around the call to RDTSC.
//              2. General code tidy up of const functions, parameters etc.
// 03/12/99 PJ Naughter 
//              1. Fixed a problem in when compiling in VC 6 in release mode.
// 17/05/00 PJ Naughter
//              1. Measure now returns the value measured.
//              2. Code is now UNICODE enabled and build configurations are provided
// 30/05/01 PJ Naughter
//              1. Minor changes to the GetTickCountAsSeconds function.
//              2. Updated the sample app to report the processor frequency
//              3. Added a GetCachedCPUFrequency function
//
// LEGALITIES:
// Copyright © 1996-2001 by J.M.McGuiness and PJ Naughter, all rights reserved.
//
// This file must not be distributed without the authors prior
// consent.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

#include <windows.h>
#include <time.h>
#include <math.h>
#include "cputicker.h"


typedef bool (WINAPI *lpfnGetVersionEx) (LPOSVERSIONINFO);


//////////////////////////////// Implementation ///////////////////////////////
bool CCPUTicker::m_bHasRDTSC = FALSE;
bool CCPUTicker::m_bRunningOnNT = FALSE;
bool CCPUTicker::m_bStaticsCalculated = FALSE;
double CCPUTicker::m_deviation = 0;
double CCPUTicker::m_freq = 0;
bool CCPUTicker::m_bFrequencyCalculated = FALSE;


#pragma optimize("",off)
bool CCPUTicker::CheckHasRDTSC()
{
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	if (sys_info.dwProcessorType == PROCESSOR_INTEL_PENTIUM)
	{
		try
		{
			_asm
			{
				_emit 0x0f				; rdtsc	
				_emit 0x31
			}
		}
		catch (...)	// Check to see if the opcode is defined.
		{
			////TRACE0("RDTSC instruction NOT present.\n");
			return FALSE;
		}
		// Check to see if the instruction ticks accesses something that changes.
		volatile ULARGE_INTEGER ts1,ts2;
		_asm
		{
			xor eax,eax
			_emit 0x0f					; cpuid
			_emit 0xa2
			_emit 0x0f					; rdtsc
			_emit 0x31
			mov ts1.HighPart,edx
			mov ts1.LowPart,eax
			xor eax,eax
			_emit 0x0f					; cpuid
			_emit 0xa2
			_emit 0x0f					; rdtsc
			_emit 0x31
			mov ts2.HighPart,edx
			mov ts2.LowPart,eax
		}
		// If we return true then there's a very good chance it's a real RDTSC instruction!
		if (ts2.HighPart == ts1.HighPart)
		{
			if (ts2.LowPart > ts1.LowPart)
			{
				//TRACE0("RDTSC instruction probably present.\n");
				return TRUE;
			}
			else
			{
				//TRACE0("RDTSC instruction NOT present.\n");
				return FALSE;
			}
		}
		else if (ts2.HighPart > ts1.HighPart)
		{
			//TRACE0("RDTSC instruction probably present.\n");
			return TRUE;
		}
		else
		{
			//TRACE0("RDTSC instruction NOT present.\n");
			return FALSE;
		}
	}
	else
	{
		//TRACE0("RDTSC instruction NOT present.\n");
		return FALSE;
	}
}
#pragma optimize("",on)

bool CCPUTicker::IsAvailable() const
{
  return m_bHasRDTSC;
}

CCPUTicker::CCPUTicker()
{
  m_TickCount = 0;

  //precalculate the statics
  if (!m_bStaticsCalculated)
  {
    m_bStaticsCalculated = TRUE;
    m_bHasRDTSC = CheckHasRDTSC();
    m_bRunningOnNT = RunningOnNT();
  }
}

CCPUTicker::CCPUTicker(const CCPUTicker& ticker)
{
  m_TickCount = ticker.m_TickCount;
}

CCPUTicker &CCPUTicker::operator=(const CCPUTicker& ticker)
{
  m_TickCount = ticker.m_TickCount;
	return *this;
}

bool CCPUTicker::RunningOnNT()
{
  //determine dynamically if GetVersionEx is available, if 
  //not then drop back to using GetVersion

  // Get Kernel handle
  HMODULE hKernel32 = GetModuleHandle("KERNEL32.DLL");
  if (hKernel32 == NULL)
    return FALSE;

  #ifdef _UNICODE
    lpfnGetVersionEx lpGetVersionEx = (lpfnGetVersionEx) GetProcAddress(hKernel32, "GetVersionExW");
  #else
    lpfnGetVersionEx lpGetVersionEx = (lpfnGetVersionEx) GetProcAddress(hKernel32, "GetVersionExA");
  #endif

  if (lpGetVersionEx)
  {
    OSVERSIONINFO osvi;
    memset(&osvi, 0, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!lpGetVersionEx(&osvi))
      return FALSE;

    return (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);
  }
  else
  {
    //Since GetVersionEx is not available we known that 
    //we are running on NT 3.1 as all modern versions of NT and
    //any version of Windows 95/98 support GetVersionEx
    return TRUE;
  }
}


#pragma optimize("",off)
__int64 CCPUTicker::Measure()
{
	if (m_bHasRDTSC)
	{
		volatile ULARGE_INTEGER ts;

    //on NT don't bother disabling interrupts as doing
    //so will generate a priviledge instruction exception
		if (!m_bRunningOnNT)
		{
			_asm
			{
				cli
			}
		}

		_asm
		{
			xor eax,eax
      push eax
      push ebx
			push ecx
			push edx
			_emit 0x0f					; cpuid - serialise the processor
			_emit 0xa2
			pop edx
			pop ecx
      pop ebx
      pop eax
			_emit 0x0f					; rdtsc
			_emit 0x31
			mov ts.HighPart,edx
			mov ts.LowPart,eax
		}

		if (!m_bRunningOnNT)
		{
			_asm
			{
				sti
			}
		}

    m_TickCount = ts.QuadPart;
	}
	else
	{
    m_TickCount=0;
	}

  return m_TickCount;
}
#pragma optimize("",on)

// The following function will work out the processor clock frequency to a
// specified accuracy determined by the target average deviation required.
// Note that the worst average deviation of the result is less than 5MHz for
// a mean frequency of 90MHz. So basically the target average deviation is
// supplied only if you want a more accurate result, it won't let you get a
// worse one. (Units are Hz.)
//
// (The average deviation is a better and more robust measure than it's cousin
// the standard deviation of a quantity. The item determined by each is
// essentially similar. See "Numerical Recipies", W.Press et al for more
// details.)
//
// This function will run for a maximum of 20 seconds before giving up on
// trying to improve the average deviation, with the average deviation
// actually achieved replacing the supplied target value. Use "max_loops" to
// change this. To improve the value the function converges to increase
// "interval" (which is in units of ms, default value=1000ms).

#pragma optimize("",off)
bool CCPUTicker::GetCPUFrequency(double& frequency, double& target_ave_dev, 
                                 unsigned long interval, unsigned int max_loops)
{
	register LARGE_INTEGER goal,period,current;
	register unsigned int ctr=0;
	double curr_freq,ave_freq;	// In Hz.
	double ave_dev,tmp=0;
	CCPUTicker s,f;
	if (!QueryPerformanceFrequency(&period))
		return FALSE;
	period.QuadPart*=interval;
	period.QuadPart/=1000;

	// Start of tight timed loop.
	QueryPerformanceCounter(&goal);
	goal.QuadPart+=period.QuadPart;
	s.Measure();
	do
	{
		QueryPerformanceCounter(&current);
	} while(current.QuadPart<goal.QuadPart);
	f.Measure();
	// End of tight timed loop.

	ave_freq=1000*((double) (f.m_TickCount - s.m_TickCount))/interval;
	do
	{
		// Start of tight timed loop.
		QueryPerformanceCounter(&goal);
		goal.QuadPart+=period.QuadPart;
		s.Measure();
		do
		{
			QueryPerformanceCounter(&current);
		} while(current.QuadPart<goal.QuadPart);
		f.Measure();
		// End of tight timed loop.

		// Average of the old frequency plus the new.
		curr_freq=1000*((double) (f.m_TickCount - s.m_TickCount))/interval;
		ave_freq=(curr_freq+ave_freq)/2;

		// Work out the current average deviation of the frequency.
		tmp+=fabs(curr_freq-ave_freq);
		ave_dev=tmp/++ctr;
	} while (ave_dev>target_ave_dev && ctr<max_loops);
	target_ave_dev=ave_dev;
	//TRACE2("Estimated the processor clock frequency =%gHz, dev.=±%gHz.\n",ave_freq,ave_dev);
  frequency = ave_freq;
  m_bFrequencyCalculated = TRUE;

	return TRUE;
}
#pragma optimize("",on)

bool CCPUTicker::GetCachedCPUFrequency(double& frequency, double& ave_dev)
{
  bool bSuccess = FALSE;
  if (m_bFrequencyCalculated)
  {
    frequency = m_freq;
    ave_dev = m_deviation;
    bSuccess = TRUE;
  }
  else
    bSuccess = CCPUTicker::GetCPUFrequency(m_freq, m_deviation);
  return bSuccess;
}


double CCPUTicker::GetTickCountAsSeconds() const
{
  if (!m_bFrequencyCalculated)
  {
    bool bSuccess = CCPUTicker::GetCPUFrequency(m_freq, m_deviation);
    if (!bSuccess)
      return 0;
  }

  return ((double) m_TickCount) / m_freq;
}

#endif //WIN32