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

#include <opcua_p_internal.h>
/*#include <opcua_string.h>*/

#include <opcua_datetime.h>
#include <opcua_p_datetime.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static const long long SECS_BETWEEN_EPOCHS = 11644473600ll;
static const long long SECS_TO_100NS = 10000000ll; /* 10^7 */
static const long long MSECS_TO_100NS = 10000ll; /* 10^4 */
static const long long MICROSECS_TO_100NS = 10ll; /* 10 */

#ifndef __cplusplus
typedef int bool;
#define true 1
#define false 0
#endif

/*============================================================================
* The OpcUa_GetTimeOfDay function (returns the time in OpcUa_TimeVal format)
*===========================================================================*/
OpcUa_Void OpcUa_P_DateTime_GetTimeOfDay(OpcUa_TimeVal* a_pTimeVal)
{
    struct timeval tp;
    gettimeofday(&tp, 0);

    a_pTimeVal->uintSeconds         = (OpcUa_UInt32)tp.tv_sec;
    a_pTimeVal->uintMicroSeconds    = (OpcUa_UInt32)tp.tv_usec;
}

/*============================================================================
* The OpcUa_UtcNow function (returns the time in OpcUa_DateTime format)
*===========================================================================*/
OpcUa_DateTime OpcUa_P_DateTime_UtcNow()
{
    struct timeval now;
    long long unixtime = 0;
    OpcUa_DateTime dateTime;

    if(gettimeofday(&now, NULL) == 0)
    {
        unixtime = now.tv_sec;
        unixtime += SECS_BETWEEN_EPOCHS;
        unixtime *= SECS_TO_100NS;
        unixtime += now.tv_usec * MICROSECS_TO_100NS;
    }
    dateTime.dwHighDateTime = unixtime >> 32;
    dateTime.dwLowDateTime  = unixtime & 0xffffffff;

    return dateTime;
}

/*============================================================================
* Convert DateTime into String
*===========================================================================*/
OpcUa_StatusCode OpcUa_P_DateTime_GetStringFromDateTime(    OpcUa_DateTime a_DateTime,
                                                            OpcUa_StringA  a_pBuffer,
                                                            OpcUa_UInt32   a_uLength)
{
    long long unixtime = ((long long)a_DateTime.dwHighDateTime << 32) + a_DateTime.dwLowDateTime;
    int ms;
    time_t tTime;
    struct tm sTime;
    int apiResult;

    ms = (unixtime % SECS_TO_100NS) / MSECS_TO_100NS;
    unixtime /= SECS_TO_100NS;
    unixtime -= SECS_BETWEEN_EPOCHS;
    tTime = unixtime;

    /* check for overflow */
    if(tTime != unixtime)
    {
        return OpcUa_Bad;
    }

    if(gmtime_r(&tTime, &sTime) == NULL)
    {
        return OpcUa_Bad;
    }

    apiResult = snprintf(a_pBuffer, a_uLength,
                         "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                         sTime.tm_year+1900, sTime.tm_mon+1, sTime.tm_mday,
                         sTime.tm_hour, sTime.tm_min, sTime.tm_sec, ms);

    if(apiResult < 20)
    {
        return OpcUa_Bad;
    }

    return OpcUa_Good;
}

/*============================================================================
* Convert String into DateTime
*===========================================================================*/
OpcUa_StatusCode OpcUa_P_DateTime_GetDateTimeFromString(OpcUa_StringA   a_pchDateTimeString,
                                                        OpcUa_DateTime* a_pDateTime)
{
    bool        milliSet        = false;
    bool        leapYear        = false;
    struct tm   tmTime;
    time_t      tTime;
    int         ms          = 0;
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
    long long   unixtime;

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
        strncpy(years, a_pchDateTimeString, 4);
        strncpy(months, a_pchDateTimeString+5, 2);
        strncpy(days, a_pchDateTimeString+8, 2);
        strncpy(hours, a_pchDateTimeString+11, 2);
        strncpy(minutes, a_pchDateTimeString+14, 2);
        strncpy(seconds, a_pchDateTimeString+17, 2);

        /* initialize */
        OpcUa_MemSet(&tmTime, 0, sizeof(tmTime));

        /* parse date and time */
        tmTime.tm_year = strtol(years, 0, 10) - 1900;
        if(tmTime.tm_year < 1601-1900 || tmTime.tm_year > 9999-1900)
        {
            return OpcUa_BadOutOfRange;
        }
        tmTime.tm_mon = strtol(months, 0, 10) - 1;
        if(tmTime.tm_mon < 0 || tmTime.tm_mon > 11)
        {
            return OpcUa_BadOutOfRange;
        }
        tmTime.tm_mday = strtol(days, 0, 10);
        if(tmTime.tm_mday < 1 || tmTime.tm_mday > 31)
        {
            return OpcUa_BadOutOfRange;
        }
        tmTime.tm_hour = strtol(hours, 0, 10);
        if(tmTime.tm_hour < 0 || tmTime.tm_hour > 23)
        {
            return OpcUa_BadOutOfRange;
        }
        tmTime.tm_min = strtol(minutes, 0, 10);
        if(tmTime.tm_min < 0 || tmTime.tm_min > 59)
        {
            return OpcUa_BadOutOfRange;
        }
        tmTime.tm_sec = strtol(seconds, 0, 10);
        if(tmTime.tm_sec < 0 || tmTime.tm_sec > 59)
        {
            return OpcUa_BadOutOfRange;
        }

        signPosition = 19;

        /* check if ms are set */
        if(a_pchDateTimeString[signPosition] == '.')
        {
            milliSet = true;
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
                strncpy(millis, a_pchDateTimeString+20, tmpVar);
                ms = strtol(millis, 0, 10);
            }
        }
        else if(a_pchDateTimeString[signPosition] == '+' || a_pchDateTimeString[signPosition] == '-')
        {
            /* copy timezone */
            strncpy(timeZone, a_pchDateTimeString+signPosition, 3);
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
                strncpy(millis, a_pchDateTimeString+20, tmpVar);
                ms = strtol(millis, 0, 10);
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
        tmpVar = tmTime.tm_hour - zoneValue;
        if(tmpVar > 23)
        {
            tmTime.tm_hour = tmpVar - 24;
            dayChange = 1;     /* add one day to date */
        }
        else if(tmpVar < 0)
        {
            tmTime.tm_hour = tmpVar + 24;
            dayChange = -1;    /* substract one day from date */
        }
        else
        {
            tmTime.tm_hour = tmTime.tm_hour - zoneValue;
            dayChange = 0;
        }

        /* day will change */
        if(dayChange != 0)
        {
            /* set day */
            tmpVar = tmTime.tm_mday + dayChange;

            /* month will decrease */
            if(tmpVar == 0)
            {
                /* check which month */
                switch(tmTime.tm_mon)
                {
                    /* prior month has 31 days */
                    case 0:
                    case 1:
                    case 3:
                    case 5:
                    case 7:
                    case 8:
                    case 10:
                        monthChange = -1;
                        tmTime.tm_mday = 31;
                        break;

                    /* prior month has 30 days */
                    case 4:
                    case 6:
                    case 9:
                    case 11:
                        monthChange = -1;
                        tmTime.tm_mday = 30;
                        break;

                    /* prior month has 28/29 days */
                    case 2:

                        /* check if it's a leap year */
                        if(tmTime.tm_year % 4 == 0)
                        {
                            if(tmTime.tm_year % 100 == 0)
                            {
                                /* no leapyear */
                                if((tmTime.tm_year + 1900) % 400 == 0)
                                {
                                    leapYear = true;
                                }
                                else
                                {
                                    leapYear = false;
                                }
                            }
                            else
                            {
                                leapYear = true;
                            }
                        }
                        else
                        {
                            leapYear = false;
                        }

                        if(leapYear == true)
                        {
                            monthChange = -1;
                            tmTime.tm_mday = 29;
                        }
                        else
                        {
                            monthChange = -1;
                            tmTime.tm_mday = 28;
                        }

                        break;
                }
            }
            /* month might increase */
            else if(tmpVar > 27)
            {
                switch(tmTime.tm_mon)
                {
                    /* month has 31 days */
                    case 0:
                    case 2:
                    case 4:
                    case 6:
                    case 7:
                    case 9:
                    case 11:
                        if(tmTime.tm_mday == 31)
                        {
                            /* increase month */
                            monthChange = 1;
                            tmTime.tm_mday = 1;
                        }
                        else
                        {
                            tmTime.tm_mday++;
                            monthChange = 0;
                        }
                        break;

                    /* month has 30 days */
                    case 3:
                    case 5:
                    case 8:
                    case 10:
                        if(tmTime.tm_mday == 30)
                        {
                            /* increase month */
                            monthChange = 1;
                            tmTime.tm_mday = 1;
                        }
                        else
                        {
                            tmTime.tm_mday++;
                            monthChange = 0;
                        }
                        break;

                    /* month has 27/28 days */
                    case 1:
                        /* check if it's a leap year */
                        if(tmTime.tm_year % 4 == 0)
                        {
                            if(tmTime.tm_year % 100 == 0)
                            {
                                /* no leapyear */
                                if((tmTime.tm_year + 1900) % 400 == 0)
                                {
                                    leapYear = true;
                                }
                                else
                                {
                                    leapYear = false;
                                }
                            }
                            else
                            {
                                leapYear = true;
                            }
                        }
                        else
                        {
                            leapYear = false;
                        }

                        if(leapYear == true)
                        {
                            if(tmTime.tm_mday == 29)
                            {
                                /* increase month */
                                monthChange = 1;
                                tmTime.tm_mday = 1;
                            }
                            else
                            {
                                tmTime.tm_mday++;
                                monthChange = 0;
                            }
                        }
                        else
                        {
                            if(tmTime.tm_mday == 28)
                            {
                                /* increase month */
                                monthChange = 1;
                                tmTime.tm_mday = 1;
                            }
                            else
                            {
                                tmTime.tm_mday++;
                                monthChange = 0;
                            }
                        }
                        break;
                }
            }
            /* month will not change */
            else
            {
                tmTime.tm_mday = tmTime.tm_mday + dayChange;
                monthChange = 0;
            }

            /* set month */
            if(monthChange != 0)
            {
                tmpVar = tmTime.tm_mon + monthChange;

                /* decrease year */
                if(tmpVar == -1)
                {
                    tmTime.tm_mon = 11;
                    tmTime.tm_year--;
                }
                /* decrease year */
                else if(tmpVar == 12)
                {
                    tmTime.tm_mon = 0;
                    tmTime.tm_year++;
                }
                /* no year change */
                else
                {
                    tmTime.tm_mon += monthChange;
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

    /* convert tm struct to time_t, mktime assumes local time so we have to correct this afterwards */
    tTime = 3600*tmTime.tm_hour + 60*tmTime.tm_min + tmTime.tm_sec;
    tmTime.tm_hour = 12; /* mktime can change the tm_hour, so set to 12 o'clock local time */
    tmTime.tm_min = 0;
    tmTime.tm_sec = 0;
    tmTime.tm_isdst = 0;
    if(mktime(&tmTime) == -1)
    {
        return OpcUa_BadOutOfRange;
    }
    /* compute days since 1.1.1601, mktime fills in tm_yday for us */
    unixtime = 365*(tmTime.tm_year+299) + tmTime.tm_yday
               + (tmTime.tm_year+299)/4 - (tmTime.tm_year+299)/100 + (tmTime.tm_year+299)/400;

    /* convert to seconds */
    unixtime *= 24*3600;
    /* add day time to 64 bit value */
    unixtime += tTime;
    /* convert to FILETIME */
    unixtime *= SECS_TO_100NS;
    /* add the milliseconds */
    unixtime += ms * MSECS_TO_100NS;

    a_pDateTime->dwHighDateTime = unixtime >> 32;
    a_pDateTime->dwLowDateTime  = unixtime & 0xffffffff;

    return OpcUa_Good;
}

