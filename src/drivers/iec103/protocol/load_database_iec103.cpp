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
#include "stdint.h"
#include "iec104types.h"
#include "iec_item.h"
#include "itrace.h"	
#include "iec103_db.h"
#include "iec103_imp.h"
#include "stdlib.h"
#include "string.h"
#include "inifile.h"
#endif // _WIN32

//IEC103 types

//<0> not defined
#define TYPE_1	1
#define TYPE_2	2
#define TYPE_3	3
#define TYPE_4	4
#define TYPE_5	5
#define TYPE_6	6
#define TYPE_7	7
#define TYPE_8	8
#define TYPE_9	9
#define TYPE_10	10
#define TYPE_11	11
#define TYPE_20	20
#define TYPE_21	21
#define TYPE_23	23
#define TYPE_24	24
#define TYPE_25	25
#define TYPE_26	26
#define TYPE_27	27
#define TYPE_28	28
#define TYPE_29	29
#define TYPE_30	30
#define TYPE_31	31

static int gl_row_counter = 0;
static int gl_column_counter = 0;
static struct iec103DbRecord* gl_Config_db = NULL;

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
				//column 1 in table iec103_table
				if(argv[i] != NULL)
					gl_Config_db[gl_row_counter].iec103_func = atoi(argv[i]);
			}
			break;
			case 1:
			{
				//column 2 in table iec103_table
				if(argv[i] != NULL)
					gl_Config_db[gl_row_counter].iec103_info = atoi(argv[i]);
			}
			break;
			case 2:
			{
				//column 3 in table iec103_table
				//iec103_type_command
				if(argv[i] != NULL)
				{
					if(strcmp(argv[i], "TYPE_1") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_1;
					}
					else if(strcmp(argv[i], "TYPE_2") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_2;
					}
					else if(strcmp(argv[i], "TYPE_3") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_3;
					}
					else if(strcmp(argv[i], "TYPE_4") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_4;
					}
					else if(strcmp(argv[i], "TYPE_5") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_5;
					}
					else if(strcmp(argv[i], "TYPE_6") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_6;
					}
					else if(strcmp(argv[i], "TYPE_7") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_7;
					}
					else if(strcmp(argv[i], "TYPE_8") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_8;
					}
					else if(strcmp(argv[i], "TYPE_9") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_9;
					}
					else if(strcmp(argv[i], "TYPE_10") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_10;
					}
					else if(strcmp(argv[i], "TYPE_11") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_11;
					}
					else if(strcmp(argv[i], "TYPE_20") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_20;
					}
					else if(strcmp(argv[i], "TYPE_21") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_21;
					}
					else if(strcmp(argv[i], "TYPE_23") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_23;
					}
					else if(strcmp(argv[i], "TYPE_24") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_24;
					}
					else if(strcmp(argv[i], "TYPE_25") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_25;
					}
					else if(strcmp(argv[i], "TYPE_26") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_26;
					}
					else if(strcmp(argv[i], "TYPE_27") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_27;
					}
					else if(strcmp(argv[i], "TYPE_28") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_28;
					}
					else if(strcmp(argv[i], "TYPE_29") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_29;
					}
					else if(strcmp(argv[i], "TYPE_30") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_30;
					}
					else if(strcmp(argv[i], "TYPE_31") == 0)
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_31;
					}
					else
					{
						gl_Config_db[gl_row_counter].iec103_type_command = TYPE_1;
					}
				}
				else
				{
					gl_Config_db[gl_row_counter].iec103_type_command = TYPE_1;
				}
			}
			break;
			case 3:
			{
				//column 4 in table iec103_table
				//ioa_control_center Unstructured
				if(argv[i] != NULL)
					gl_Config_db[gl_row_counter].ioa_control_center = atoi(argv[i]);
			}
			break;
			case 4:
			{
				//column 5 in table iec103_table
				//deadband
				if(argv[i] != NULL)
					gl_Config_db[gl_row_counter].deadband = (float)atof(argv[i]);
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

#define MAX_CONFIGURABLE_IEC103_RECORDS 30000

int iec103_imp::AddItems(void)
{
	IT_IT("iec103_imp::AddItems");

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
	strcat(database_name, "iec103_database");
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

	g_dwNumItems = MAX_CONFIGURABLE_IEC103_RECORDS;
	
	Config_db = (struct iec103DbRecord*)calloc(1, g_dwNumItems*sizeof(struct iec103DbRecord));

	gl_Config_db = Config_db;

	gl_row_counter = 0;

	rc = sqlite3_exec(db, "select * from iec103_table;", db_callback, 0, &zErrMsg);

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
