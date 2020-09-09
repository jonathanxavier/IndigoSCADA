// TITLE:
// High-Resolution Counter Class.
//
// VERSION:
// 1.24
//
// AUTHORS:
// Created by J.M.McGuiness, http://www.hussar.demon.co.uk
//            PJ Naughter,   Email: pjn@indigo.ie Web: http://indigo.ie/~pjn
//
// DESCRIPTION:
// This file declares a class the wraps the Pentium-specific time stamp counter.
// This counter has a resolution in terms of PCLKS (processor clocks) so it can
// be used for direct instruction timings.
//
// LEGALITIES:
// Copyright © 1996-2001 by J.M.McGuiness and PJ Naughter, all rights reserved.
//
// This file must not be distributed without the authors prior
// consent.
//
/////////////////////////////////////////////////////////////////////////////


////////////////////////////////// Macros ///////////////////////////////////

#ifndef __CPUTICKER_H__
#define __CPUTICKER_H__

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

////////////////////////// Classes //////////////////////////////////////////
class TRACE_DLL_EXP_IMP CCPUTicker 
{
public:
//constructors / Destructors
	CCPUTicker();
	CCPUTicker(const CCPUTicker& ticker);

//copy constructor
	inline CCPUTicker& operator=(const CCPUTicker& ticker);

//perform the actual measurement
  __int64 Measure();

//accessors to the actual measurement value
  double GetTickCountAsSeconds() const;  
  inline __int64 GetTickCount() const { return m_TickCount; };  

//static methods
	static bool GetCPUFrequency(double& frequency, double& target_ave_dev, 
                              unsigned long interval = 1000,
                              unsigned int max_loops = 20);
  static bool GetCachedCPUFrequency(double& frequency, double& target_ave_dev);

//Is high resolution available on this CPU
	bool IsAvailable() const;

protected:
  __int64 m_TickCount;

 	static bool m_bHasRDTSC;
  static bool m_bRunningOnNT;
  static bool m_bStaticsCalculated;

  static double m_deviation;
  static double m_freq;
  static bool m_bFrequencyCalculated;

  static bool CheckHasRDTSC();
  static bool RunningOnNT();

};

#endif //__CPUTICKER_H__