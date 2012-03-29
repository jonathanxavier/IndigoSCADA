/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2012 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stdio.h>
#include <time.h>
#include "unilog.h"
#include "opcda.h"

#define LOGID logg,0				// log identifiner
#define LOG_FNAME	"opc_server_da.log"	// log name
#define CFG_FILE	"opc_server_da.ini"	// cfg name
#define MAX_ENSCADA_NUM		5			// maximum number of controllers
#define TAGS_NUM_MAX	5000		// maximum number of tags
#define DATALEN_MAX		150			// maximum lenght of the tags

typedef unsigned UINT;

typedef struct _PrmEN PrmEN;

struct _PrmEN {
  SHORT	prm;
  CHAR  name[100];
};

typedef struct _DataEN DataEN;

struct _DataEN {
  UINT	device;
  CHAR  name[30];
  SHORT prm;
  SHORT status;
  UINT	tn;
  CHAR  value[20];
  SHORT pipe;
};

typedef struct _FlatEN FlatEN;

struct _FlatEN {
  UINT	flat;
  CHAR  name[30];
  SHORT prm;
  SHORT status;
  UINT	tn;
  CHAR  value[20];
};
