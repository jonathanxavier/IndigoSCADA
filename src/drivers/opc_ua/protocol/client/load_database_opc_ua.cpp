/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2014 Enscada
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifdef _WIN32

#include <crtdbg.h>
#include <stdio.h>
#include <sqlite3.h>
#include "iec104types.h"
#include "iec_item.h"
#include "itrace.h"	
#include "opc_ua_imp.h"
#include "stdlib.h"
#include "string.h"
#include "inifile.h"
#include "opc_ua_db.h"
#endif // _WIN32

static gl_row_counter = 0;
static gl_column_counter = 0;
static struct opcuaDbRecord* gl_Config_db = 0;

static int db_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;

	gl_column_counter = argc;
	
	for(i = 0; i < argc; i++)
	{
		fprintf(stderr, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		fflush(stderr);

		switch(i)
		{
			case 0:
			{
				//column 1 in table opcua_table
				//node ID
				if(argv[i] != NULL)
					strcpy(gl_Config_db[gl_row_counter].nodeid, argv[i]);
			}
			break;
			case 1:
			{
				//column 2 in table opcua_table
				//namespace_index
				if(argv[i] != NULL)
					gl_Config_db[gl_row_counter].namespace_index = atoi(argv[i]);
			}
			break;
			case 2:
			{
				//column 3 in table opcua_table
				//ioa_control_center Unstructured
				if(argv[i] != NULL)
					gl_Config_db[gl_row_counter].ioa_control_center = atoi(argv[i]);
			}
			break;
			default:
			break;
		}
	}

	//ended to read a record
	gl_row_counter++;

	fprintf(stderr, "\n");
	fflush(stderr);
	return 0;
}

#define MAX_CONFIGURABLE_OPCUA_ITEMIDS 30000

int opcua_imp::AddItems(void)
{
	IT_IT("opcua_imp::AddItems");

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char line_number[10];

	itoa(lineNumber, line_number, 10);
	
	//project directory 04-12-2020
	char project_dir[_MAX_PATH];
	char ini_file[_MAX_PATH];
		
	ini_file[0] = '\0';
	if(GetModuleFileName(NULL, ini_file, _MAX_PATH))
	{
		*(strrchr(ini_file, '\\')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(ini_file, '\\')) = '\0';        // Strip \\bin off path
		
		strcat(ini_file, "\\bin\\project.ini");
		Inifile iniFile(ini_file);

		if(iniFile.find("path","project_directory"))
		{
			strcpy(project_dir, iniFile.find("path","project_directory"));
		}
    }

	strcpy(database_name, project_dir);
	strcat(database_name, "\\");
	strcat(database_name, "opcua_database");
	strcat(database_name, line_number);
	strcat(database_name, ".db");

	rc = sqlite3_open(database_name, &db);

	if(rc)
	{
	  fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	  fflush(stderr);
	  sqlite3_close(db);
	  IT_EXIT;
	  return 1;
	}

	g_dwNumItems = MAX_CONFIGURABLE_OPCUA_ITEMIDS;
	
	Config_db = (struct opcuaDbRecord*)calloc(1, g_dwNumItems*sizeof(struct opcuaDbRecord));

	gl_Config_db = Config_db;

	gl_row_counter = 0;

	rc = sqlite3_exec(db, "select * from opcua_table;", db_callback, 0, &zErrMsg);

	if(rc != SQLITE_OK)
	{
	  fprintf(stderr, "SQL error: %s\n", zErrMsg);
	  fflush(stderr);
	  sqlite3_free(zErrMsg);
	}

	sqlite3_close(db);

	db_n_rows = gl_row_counter;
	db_m_columns = gl_column_counter;

	if(db_n_rows == 0)
	{
		fprintf(stderr, "Error: db_n_rows = %d\n", db_n_rows);
		fflush(stderr);
		IT_EXIT;
		return 1;
	}
	
	g_dwNumItems = db_n_rows;


	return(0);
}
