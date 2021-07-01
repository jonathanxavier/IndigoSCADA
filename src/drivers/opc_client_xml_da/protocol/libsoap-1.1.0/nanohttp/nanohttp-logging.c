/******************************************************************
*  $Id: nanohttp-logging.c,v 1.1 2006/07/09 16:22:52 snowdrop Exp $
*
* CSOAP Project:  A http client/server library in C
* Copyright (C) 2003  Ferhat Ayaz
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
* Email: ayaz@jprogrammer.net
******************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#include "nanohttp-logging.h"

#ifdef WIN32
#ifndef __MINGW32__

#include <stdio.h>
#include <stdarg.h>

/* not thread safe!*/
char *
VisualC_funcname(const char *file, int line)
{
  static char buffer[256];
  int i = strlen(file) - 1;
  while (i > 0 && file[i] != '\\')
    i--;
  sprintf(buffer, "%s:%d", (file[i] != '\\') ? file : (file + i + 1), line);
  return buffer;
}

#endif
#endif

static log_level_t loglevel = HLOG_DEBUG;
static char logfile[75] = { '\0' };
static int log_background = 0;

log_level_t
hlog_set_level(log_level_t level)
{
  log_level_t old = loglevel;
  loglevel = level;
  return old;
}


log_level_t
hlog_get_level(void)
{
  return loglevel;
}


void
hlog_set_file(const char *filename)
{
  if (filename)
    strncpy(logfile, filename, 75);
  else
    logfile[0] = '\0';
}

void
hlog_set_background(int state)
{
  log_background = state;
}

char *
hlog_get_file()
{
  if (logfile[0] == '\0')
    return NULL;
  return logfile;
}

static void
_log_write(log_level_t level, const char *prefix,
          const char *func, const char *format, va_list ap)
{
  char buffer[1054];
  char buffer2[1054];
  FILE *f;

  if (level < loglevel)
    return;

  if (!log_background || hlog_get_file())
  {
#ifdef WIN32
    sprintf(buffer, "*%s*: [%s] %s\n", prefix, func, format);
#else
    sprintf(buffer, "*%s*:(%ld) [%s] %s\n",
            prefix, pthread_self(), func, format);
#endif
    vsprintf(buffer2, buffer, ap);
    if (!log_background)
    {
      printf(buffer2);
      fflush(stdout);
    }

    if (hlog_get_file())
    {
      f = fopen(hlog_get_file(), "a");
      if (!f)
        f = fopen(hlog_get_file(), "w");
      if (f)
      {
        fprintf(f, buffer2);
        fflush(f);
        fclose(f);
      }
    }
  }
}

void
hlog_verbose(const char *FUNC, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  _log_write(HLOG_VERBOSE, "VERBOSE", FUNC, format, ap);
  va_end(ap);
}

void
hlog_debug(const char *FUNC, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  _log_write(HLOG_DEBUG, "DEBUG", FUNC, format, ap);
  va_end(ap);
}

void
hlog_info(const char *FUNC, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  _log_write(HLOG_INFO, "INFO", FUNC, format, ap);
  va_end(ap);
}

void
hlog_warn(const char *FUNC, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  _log_write(HLOG_WARN, "WARN", FUNC, format, ap);
  va_end(ap);
}

void
hlog_error(const char *FUNC, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  _log_write(HLOG_ERROR, "ERROR", FUNC, format, ap);
  va_end(ap);
}

