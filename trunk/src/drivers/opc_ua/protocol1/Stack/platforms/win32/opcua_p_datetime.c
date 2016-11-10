/* Copyright (c) 1996-2016, OPC Foundation. All rights reserved.

   The source code in this file is covered under a dual-license scenario:
     - RCL: for OPC Foundation members in good-standing
     - GPL V2: everybody else

   RCL license terms accompanied with this source code. See http://opcfoundation.org/License/RCL/1.00/

   GNU General Public License as published by the Free Software Foundation;
   version 2 of the License are accompanied with this source code. See http://opcfoundation.org/License/GPLv2

   This source code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/******************************************************************************************************/
/* Platform Portability Layer                                                                         */
/* Modify the content of this file according to the event implementation on your system.              */
/* This is the win32 implementation                                                                   */
/******************************************************************************************************/

/* System Headers */
#include <windows.h>
#include <time.h>
#ifdef _WIN32_WCE
#include <altcecrt.h>
#define mktime _mktime64
#endif


/* UA platform definitions */
#include <opcua_p_internal.h>

/* additional UA dependencies */
/* reference to stack; string is needed for conversion */
#include <opcua_string.h>

/* own headers */
#include <opcua_datetime.h>
#include <opcua_p_datetime.h>

/*============================================================================
 * The OpcUa_GetTimeOfDay function (returns the time in OpcUa_TimeVal format)
 *===========================================================================*/
OpcUa_Void OPCUA_DLLCALL OpcUa_P_DateTime_GetTimeOfDay(OpcUa_TimeVal* a_pTimeVal)
{
  SYSTEMTIME    SystemTime;
  time_t        TimeType;  /* may be int64 */
  struct tm     structTM;

  GetLocalTime(&SystemTime);

  structTM.tm_sec   = SystemTime.wSecond;
  structTM.tm_min   = SystemTime.wMinute;
  structTM.tm_hour  = SystemTime.wHour;
  structTM.tm_mday  = SystemTime.wDay;
  structTM.tm_mon   = SystemTime.wMonth - 1;
  structTM.tm_year  = SystemTime.wYear - 1900;
  structTM.tm_isdst = -1;

  TimeType = mktime(&structTM);

  a_pTimeVal->uintSeconds = (OpcUa_UInt32)TimeType;
  a_pTimeVal->uintMicroSeconds = SystemTime.wMilliseconds * 1000;
}

/*============================================================================
 * The OpcUa_UtcNow function (returns the time in OpcUa_DateTime format)
 *===========================================================================*/
OpcUa_DateTime OPCUA_DLLCALL OpcUa_P_DateTime_UtcNow()
{
    FILETIME ftTime;
#ifdef _WIN32_WCE
    SYSTEMTIME sysTime;
#endif /* _WIN32_WCE */
    OpcUa_DateTime tmpDateTime;

#ifdef _WIN32_WCE
    GetSystemTime(&sysTime);
    SystemTimeToFileTime(&sysTime, &ftTime);
#else /* _WIN32_WCE */
    GetSystemTimeAsFileTime(&ftTime);
#endif /* _WIN32_WCE */

    tmpDateTime.dwHighDateTime = (OpcUa_UInt32)ftTime.dwHighDateTime;
    tmpDateTime.dwLowDateTime  = (OpcUa_UInt32)ftTime.dwLowDateTime;

    return tmpDateTime;
}

/*============================================================================
 * Convert DateTime into String
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_DateTime_GetStringFromDateTime(  OpcUa_DateTime a_DateTime, 
                                                                        OpcUa_StringA  a_pBuffer,
                                                                        OpcUa_UInt32   a_uLength)
{
    OpcUa_Int           apiResult    = 0;
    const char*         formatString = "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ";
    FILETIME            FileTime;
    SYSTEMTIME          SystemTime;

    FileTime.dwLowDateTime  = (DWORD)(a_DateTime.dwLowDateTime);
    FileTime.dwHighDateTime = (DWORD)(a_DateTime.dwHighDateTime);

    FileTimeToSystemTime(&FileTime, &SystemTime);

#if OPCUA_USE_SAFE_FUNCTIONS
    apiResult = OpcUa_SPrintfA(a_pBuffer, a_uLength, formatString, SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
    OpcUa_ReferenceParameter(a_uLength);
    if(a_uLength < 25)
    {
        return OpcUa_BadInvalidArgument;
    }
    apiResult = OpcUa_SPrintfA(a_pBuffer, formatString, SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */
    
    if(apiResult < 20)
    {
        return OpcUa_Bad;
    }

    return OpcUa_Good;
}

/*============================================================================
 * Convert String into DateTime
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_DateTime_GetDateTimeFromString(OpcUa_StringA   a_pchDateTimeString,
                                                                OpcUa_DateTime* a_pDateTime)
{
    BOOL        bResult         = FALSE;
    BOOL        milliSet        = FALSE;
    BOOL        leapYear        = FALSE;
    FILETIME    FileTime;
    SYSTEMTIME  SystemTime;
    size_t      stringLength;
    size_t      maxStringLength;
    char        years[5]    = "YYYY";
    char        months[3]   = "MM";
    char        days[3]     = "DD";
    char        hours[3]    = "HH";
    char        minutes[3]  = "MM";
    char        seconds[3]  = "SS";
    char        millis[4]   = "000";
    char        timeZone[4] = "000";
    int         zoneValue   = 0;
    int         signPosition;
    int         tmpVar;
    int         dayChange   = 0;
    int         monthChange = 0;

    /***************************************************************
     *  ToDo: 
     *  Timezone can have values from -12 to +14 
     *  At the moment only timezones from -12 to +12 are expected
     *  timezones can also have minutes 
     *  at the moment minutes are ignored
     ***************************************************************/

    if(    a_pchDateTimeString  == OpcUa_Null
        || a_pDateTime          == OpcUa_Null)
    {
        return OpcUa_BadInvalidArgument;
    }

    /* ToDo: set max stringlength we accept */
    maxStringLength = 50;

    stringLength = strlen(a_pchDateTimeString);

    /*  check length of string -> there can be any number of digits behind ms */
    /*  we'll ignore anything beyond 3 */
    if(stringLength < 20 || stringLength > maxStringLength)
    {
        return OpcUa_BadSyntaxError;
    }

    /* simple syntax check */
    /* ToDo: we might add some syntax checks here */
    if(a_pchDateTimeString[4] == '-' && a_pchDateTimeString[7] == '-' && (a_pchDateTimeString[10] == 'T' || a_pchDateTimeString[10] == 't') && a_pchDateTimeString[13] == ':' &&  a_pchDateTimeString[16] == ':' )
    {
        /* copy strings */
#if OPCUA_USE_SAFE_FUNCTIONS
        strncpy_s(years, 5, a_pchDateTimeString, 4);
        strncpy_s(months, 3, a_pchDateTimeString+5, 2);
        strncpy_s(days, 3, a_pchDateTimeString+8, 2);
        strncpy_s(hours, 3, a_pchDateTimeString+11, 2);
        strncpy_s(minutes, 3, a_pchDateTimeString+14, 2);
        strncpy_s(seconds, 3, a_pchDateTimeString+17, 2);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
        strncpy(years, a_pchDateTimeString, 5);
        strncpy(months, a_pchDateTimeString+5, 2);
        strncpy(days, a_pchDateTimeString+8, 2);
        strncpy(hours, a_pchDateTimeString+11, 2);
        strncpy(minutes, a_pchDateTimeString+14, 2);
        strncpy(seconds, a_pchDateTimeString+17, 2);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */

        /* initialize */
        OpcUa_MemSet(&SystemTime, 0, sizeof(SYSTEMTIME));

        /* parse date and time */
        SystemTime.wYear = (WORD)strtol(years, 0, 10);
        if(SystemTime.wYear < 1601 || SystemTime.wYear > 9999)
        {
            return OpcUa_BadOutOfRange;
        }
        SystemTime.wMonth = (WORD)strtol(months, 0, 10);
        if(SystemTime.wMonth == 0 || SystemTime.wMonth > 12)
        {
            return OpcUa_BadOutOfRange;
        }
        SystemTime.wDay = (WORD)strtol(days, 0, 10);
        if(SystemTime.wDay == 0 || SystemTime.wDay > 31)
        {
            return OpcUa_BadOutOfRange;
        }
        SystemTime.wHour = (WORD)strtol(hours, 0, 10);
        if(SystemTime.wHour > 23)
        {
            return OpcUa_BadOutOfRange;
        }
        SystemTime.wMinute = (WORD)strtol(minutes, 0, 10);
        if(SystemTime.wMinute > 59)
        {
            return OpcUa_BadOutOfRange;
        }
        SystemTime.wSecond = (WORD)strtol(seconds, 0, 10);
        if(SystemTime.wSecond > 59)
        {
            return OpcUa_BadOutOfRange;
        }

        signPosition = 19;
        
        /* check if ms are set */
        if(a_pchDateTimeString[signPosition] == '.')
        {
            milliSet = TRUE;
        }

        /* find sign for timezone or Z (we accept 'z' and 'Z' here) */
        while(a_pchDateTimeString[signPosition] != '\0' && a_pchDateTimeString[signPosition] != '+' && a_pchDateTimeString[signPosition] != '-' && a_pchDateTimeString[signPosition] != 'Z' && a_pchDateTimeString[signPosition] != 'z')
        {
            ++signPosition;
        }

        if(a_pchDateTimeString[signPosition] == 'z' ||a_pchDateTimeString[signPosition] == 'Z')
        {
            /* utc time */
            if(milliSet)
            {
                /* be careful we can have more or less than 3 digits of milliseconds */
                tmpVar = signPosition - 20;
                if(tmpVar > 3)
                {
                    tmpVar = 3;
                }

#if OPCUA_USE_SAFE_FUNCTIONS
                strncpy_s(millis, 4, a_pchDateTimeString+20, tmpVar);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
                strncpy(millis, a_pchDateTimeString+20, tmpVar);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */

                SystemTime.wMilliseconds =  (WORD)strtol(millis, 0, 10);
            }
        }
        else if(a_pchDateTimeString[signPosition] == '+' || a_pchDateTimeString[signPosition] == '-')
        {
            /* copy timezone */
#if OPCUA_USE_SAFE_FUNCTIONS
            strncpy_s(timeZone, 4, a_pchDateTimeString+signPosition, 3);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
            strncpy(timeZone, a_pchDateTimeString+signPosition, 3);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */

            /* strtol will take care of the sign */
            zoneValue = strtol(timeZone, 0, 10);

            if(zoneValue < -12 || zoneValue > 12)
            {
                return OpcUa_BadOutOfRange;
            }

            if(milliSet)
            {
                /* be careful we can have more or less than 3 digits of milliseconds */
                tmpVar = signPosition - 20;
                if(tmpVar > 3)
                {
                    tmpVar = 3;
                }

                /* copy timezone */
#if OPCUA_USE_SAFE_FUNCTIONS
                strncpy_s(millis, 4, a_pchDateTimeString+20, tmpVar);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
                strncpy(millis, a_pchDateTimeString+20, tmpVar);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */
                
                SystemTime.wMilliseconds =  (WORD)strtol(millis, 0, 10);
            }
        }
        else
        {
            /* error -> no timezone specified */
            /* a time without timezone is not an absolute time but a time span */
            /* we might handle this as UTC */
            return OpcUa_BadSyntaxError;
        }

        /* correct time to UTC */
        tmpVar = SystemTime.wHour - zoneValue;
        if(tmpVar > 23)
        {
            SystemTime.wHour = (WORD)(tmpVar - 24);
            dayChange = 1;     /* add one day to date */
        }
        else if(tmpVar < 0)
        {
            SystemTime.wHour = (WORD)(tmpVar + 24);
            dayChange = -1;    /* substract one day from date */
        }
        else
        {
            SystemTime.wHour = (WORD)(SystemTime.wHour - zoneValue);
            dayChange = 0; 
        }

        /* day will change */
        if(dayChange != 0)
        {
            /* set day */
            tmpVar = SystemTime.wDay + dayChange;

            /* month will decrease */
            if(tmpVar == 0)
            {
                /* check which month */
                switch(SystemTime.wMonth)
                {
                    /* prior month has 31 days */
                    case 1:
                    case 2:
                    case 4:
                    case 6:
                    case 8:
                    case 9:
                    case 11:
                        monthChange = -1;
                        SystemTime.wDay = 31;
                        break;

                    /* prior month has 30 days */
                    case 5:
                    case 7:
                    case 10:
                    case 12:
                        monthChange = -1;
                        SystemTime.wDay = 30;
                        break;

                    /* prior month has 28/29 days */
                    case 3: 

                        /* check if it's a leap year */
                        if(SystemTime.wYear % 4 == 0)
                        {
                            if(SystemTime.wYear % 100 == 0)
                            {
                                /* no leapyear */
                                if(SystemTime.wYear % 400 == 0)
                                {
                                    leapYear = TRUE;
                                }
                                else
                                {
                                    leapYear = FALSE;
                                }
                            }
                            else
                            {
                                leapYear = TRUE;
                            }
                        }
                        else
                        {
                            leapYear = FALSE;
                        }

                        if(leapYear == TRUE)
                        {
                            monthChange = -1;
                            SystemTime.wDay = 29;
                        }
                        else
                        {
                            monthChange = -1;
                            SystemTime.wDay = 28;
                        }

                        break;
                }                   
            }
            /* month might increase */
            else if(tmpVar > 27)
            {
                switch(SystemTime.wMonth)
                {
                    /* month has 31 days */
                    case 1:
                    case 3:
                    case 5:
                    case 7:
                    case 8:
                    case 10:
                    case 12:
                        if(SystemTime.wDay == 31)
                        {
                            /* increase month */
                            monthChange = 1;
                            SystemTime.wDay = 1;
                        }
                        else
                        {
                            SystemTime.wDay++;
                            monthChange = 0;
                        }                        
                        break;  

                    /* month has 30 days */
                    case 4:
                    case 6:
                    case 9:
                    case 11:
                        if(SystemTime.wDay == 30)
                        {
                            /* increase month */
                            monthChange = 1;
                            SystemTime.wDay = 1;
                        }
                        else
                        {
                            SystemTime.wDay++;
                            monthChange = 0;
                        } 
                        break;

                    /* month has 27/28 days */
                    case 2:
                        /* check if it's a leap year */
                        if(SystemTime.wYear % 4 == 0)
                        {
                            if(SystemTime.wYear % 100 == 0)
                            {
                                /* no leapyear */
                                if(SystemTime.wYear % 400 == 0)
                                {
                                    leapYear = TRUE;
                                }
                                else
                                {
                                    leapYear = FALSE;
                                }
                            }
                            else
                            {
                                leapYear = TRUE;
                            }
                        }
                        else
                        {
                            leapYear = FALSE;
                        }

                        if(leapYear == TRUE)
                        {
                            if(SystemTime.wDay == 29)
                            {
                                /* increase month */
                                monthChange = 1;
                                SystemTime.wDay = 1;
                            }
                            else
                            {
                                SystemTime.wDay++;
                                monthChange = 0;
                            }
                        }
                        else
                        {
                            if(SystemTime.wDay == 28)
                            {
                                /* increase month */
                                monthChange = 1;
                                SystemTime.wDay = 1;
                            }
                            else
                            {
                                SystemTime.wDay++;
                                monthChange = 0;
                            }
                        }
                        break;
                }
            }
            /* month will not change */
            else
            {
                SystemTime.wDay = (WORD)(SystemTime.wDay + dayChange);
                monthChange = 0;
            }

            /* set month */
            if(monthChange != 0)
            {
                tmpVar = SystemTime.wMonth + monthChange;
                
                /* decrease year */
                if(tmpVar == 0)
                {
                    SystemTime.wMonth = 12;
                    SystemTime.wYear--;
                }
                /* decrease year */
                else if(tmpVar == 13)
                {
                    SystemTime.wMonth = 1;
                    SystemTime.wYear++;
                }
                /* no year change */
                else
                {
                    SystemTime.wMonth = (WORD)(SystemTime.wMonth + monthChange);
                }
            }
        }
    }
    else /* if(strchr(a_pchDateTimeString, ':')) */
    {
        /* other formats are not supported at the moment */ 
        /* 20060606T06:48:48Z */
        /* 20060606T064848Z */
        return OpcUa_BadSyntaxError;
    }

    bResult = SystemTimeToFileTime(&SystemTime, &FileTime);
    if(bResult == FALSE)
    {
        return OpcUa_Bad;
    }

    a_pDateTime->dwHighDateTime = FileTime.dwHighDateTime;
    a_pDateTime->dwLowDateTime  = FileTime.dwLowDateTime;

    return OpcUa_Good;
}
