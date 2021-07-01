/******************************************************************
 *  $Id: nanohttp-logging.h,v 1.1 2006/07/09 16:22:52 snowdrop Exp $
 * 
 * CSOAP Project:  A http client/server library in C
 * Copyright (C) 2003-2006  Ferhat Ayaz
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 * 
 * Email: hero@persua.de
 ******************************************************************/
#ifndef __nanohttp_logging_h
#define __nanohttp_logging_h

#define NHTTP_ARG_LOGFILE	"-NHTTPlog"
#define NHTTP_ARG_LOGLEVEL	"-NHTTPloglevel"

/* logging stuff */
typedef enum log_level
{
  HLOG_VERBOSE,
  HLOG_DEBUG,
  HLOG_INFO,
  HLOG_WARN,
  HLOG_ERROR,
  HLOG_FATAL
} log_level_t;


#ifdef __cplusplus
extern "C" {
#endif

extern log_level_t hlog_set_level(log_level_t level);
extern log_level_t hlog_get_level(void);

extern void hlog_set_file(const char *filename);
extern char *hlog_get_file();

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER <= 1200
char *VisualC_funcname(const char *file, int line);     /* not thread safe! */
#define __FUNCTION__  VisualC_funcname(__FILE__, __LINE__)
#endif
#endif

extern void hlog_verbose(const char *FUNC, const char *format, ...);
extern void hlog_debug(const char *FUNC, const char *format, ...);
extern void hlog_info(const char *FUNC, const char *format, ...);
extern void hlog_warn(const char *FUNC, const char *format, ...);
extern void hlog_error(const char *FUNC, const char *format, ...);

#ifdef __cplusplus
}
#endif

/*
 * XXX: This isn't the "right" way
 *
 * #define log_debug(fmt, ...)	fprintf(stderr, "EMERGENCY: %s: " fmt "\n", \
 *                                              __FUNCTION__, ## __VA_ARGS__)
 *
 */
#define log_verbose1(a1) hlog_verbose(__FUNCTION__, a1)
#define log_verbose2(a1,a2) hlog_verbose(__FUNCTION__, a1,a2)
#define log_verbose3(a1,a2,a3) hlog_verbose(__FUNCTION__, a1,a2,a3)
#define log_verbose4(a1,a2,a3,a4) hlog_verbose(__FUNCTION__, a1,a2,a3,a4)
#define log_verbose5(a1,a2,a3,a4,a5) hlog_verbose(__FUNCTION__, a1,a2,a3,a4,a5)

#define log_debug1(a1) hlog_debug(__FUNCTION__, a1)
#define log_debug2(a1,a2) hlog_debug(__FUNCTION__, a1,a2)
#define log_debug3(a1,a2,a3) hlog_debug(__FUNCTION__, a1,a2,a3)
#define log_debug4(a1,a2,a3,a4) hlog_debug(__FUNCTION__, a1,a2,a3,a4)
#define log_debug5(a1,a2,a3,a4,a5) hlog_debug(__FUNCTION__, a1,a2,a3,a4,a5)

#define log_info1(a1) hlog_info(__FUNCTION__, a1)
#define log_info2(a1,a2) hlog_info(__FUNCTION__, a1,a2)
#define log_info3(a1,a2,a3) hlog_info(__FUNCTION__, a1,a2,a3)
#define log_info4(a1,a2,a3,a4) hlog_info(__FUNCTION__, a1,a2,a3,a4)
#define log_info5(a1,a2,a3,a4,a5) hlog_info(__FUNCTION__, a1,a2,a3,a4,a5)

#define log_warn1(a1) hlog_warn(__FUNCTION__, a1)
#define log_warn2(a1,a2) hlog_warn(__FUNCTION__, a1,a2)
#define log_warn3(a1,a2,a3) hlog_warn(__FUNCTION__, a1,a2,a3)
#define log_warn4(a1,a2,a3,a4) hlog_warn(__FUNCTION__, a1,a2,a3,a4)
#define log_warn5(a1,a2,a3,a4,a5) hlog_warn(__FUNCTION__, a1,a2,a3,a4,a5)

#define log_error1(a1) hlog_error(__FUNCTION__, a1)
#define log_error2(a1,a2) hlog_error(__FUNCTION__, a1,a2)
#define log_error3(a1,a2,a3) hlog_error(__FUNCTION__, a1,a2,a3)
#define log_error4(a1,a2,a3,a4) hlog_error(__FUNCTION__, a1,a2,a3,a4)
#define log_error5(a1,a2,a3,a4,a5) hlog_error(__FUNCTION__, a1,a2,a3,a4,a5)

//#ifdef __cplusplus
//}
//#endif

#endif
