/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2011 Enscada
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <crtdbg.h>
#include <stdio.h>
#include <stdio.h>
#include <sqlite3.h>
#include "stdint.h"
#include "iec104types.h"
#include "iec_item.h"
#include "itrace.h"	
#include "opc_client_da_app.h"
#endif // _WIN32

static gl_row_counter = 0;
static gl_column_counter = 0;
static struct structItem* gl_Config_db = 0;

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
				//column 1 in table opc_client_da_table
				//IOA Unstructured
				gl_Config_db[gl_row_counter].ioa_control_center = atoi(argv[i]);
			}
			break;
			case 1:
			{
				//column 2 in table opc_client_da_table
				//ItemID name
				strcpy(gl_Config_db[gl_row_counter].spname, argv[i]);
			}
			break;
			case 2:
			{
				//column 3 in table opc_client_da_table
				//Engineering value min
				gl_Config_db[gl_row_counter].min_measure = (float)atof(argv[i]);
			}
			break;
			case 3:
			{
				//column 4 in table opc_client_da_table
				//Engineering value max
				gl_Config_db[gl_row_counter].max_measure = (float)atof(argv[i]);
			}
			break;
			case 4:
			{
				//column 5 in table opc_client_da_table
				//Signal Type
				if(strcmp(argv[i], "M_ME_TF_1") == 0)
				{
					gl_Config_db[gl_row_counter].io_list_iec_type = M_ME_TF_1;
				}
				else if(strcmp(argv[i], "M_SP_TB_1") == 0)
				{
					gl_Config_db[gl_row_counter].io_list_iec_type = M_SP_TB_1;
				}
				else if(strcmp(argv[i], "M_DP_TB_1") == 0)
				{
					gl_Config_db[gl_row_counter].io_list_iec_type = M_DP_TB_1;
				}
				else if(strcmp(argv[i], "C_DC_NA_1") == 0)
				{
					gl_Config_db[gl_row_counter].io_list_iec_type = C_DC_NA_1;
				}
				else if(strcmp(argv[i], "C_SC_NA_1") == 0)
				{
					gl_Config_db[gl_row_counter].io_list_iec_type = C_SC_NA_1;
				}
				else if(strcmp(argv[i], "M_IT_TB_1") == 0)
				{
					gl_Config_db[gl_row_counter].io_list_iec_type = M_IT_TB_1;
				}
				else
				{
					fprintf(stderr,"IEC type %s from I/O list NOT supported\n", argv[i]);
					fflush(stderr);
					ExitProcess(0);
				}
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

#define MAX_CONFIGURABLE_OPC_ITEMIDS 30000

int Opc_client_da_imp::AddItems()
{
	IT_IT("Opc_client_da_imp::AddItems");

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc, j;
	int n_rows = 0;
	int m_columns = 0;
	//FILE* fp = NULL;

	rc = sqlite3_open("C:\\scada\\bin\\ProtocolDatabase.db", &db);

	if(rc)
	{
	  fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	  fflush(stderr);
	  sqlite3_close(db);
	  IT_EXIT;
	  return 1;
	}

	g_dwNumItems = MAX_CONFIGURABLE_OPC_ITEMIDS;
	
	Config_db = (struct structItem*)calloc(1, g_dwNumItems*sizeof(struct structItem));

	gl_Config_db = Config_db;

	gl_row_counter = 0;

	rc = sqlite3_exec(db, "select * from opc_client_da_table;", db_callback, 0, &zErrMsg);

	if(rc != SQLITE_OK)
	{
	  fprintf(stderr, "SQL error: %s\n", zErrMsg);
	  fflush(stderr);
	  sqlite3_free(zErrMsg);
	}

	sqlite3_close(db);

	n_rows = gl_row_counter;
	m_columns = gl_column_counter;

	if(n_rows == 0)
	{
		fprintf(stderr, "Error: n_rows = %d\n", n_rows);
		fflush(stderr);
		IT_EXIT;
		return 1;
	}
	
	g_dwNumItems = n_rows;

	Item = (struct structItem*)calloc(1, g_dwNumItems*sizeof(struct structItem));

	//|----------------ItemID-----------------|---ioa_control_center---|---io_list_iec_type---|--min--|--max--|
	//	Simulated Card.Simulated Node.Random.Ra			1					31			
	//	Simulated Card.Simulated Node.Random.Rb			2					30					Engineering values (min and max)
	//	Simulated Card.Simulated Node.Random.Rc			3					46					of measure or set point
	
	
	//////dump for debug////////////////////////////////////////////////////////////////////////////////
	//fp = fopen("C:\\scada\\logs\\Config_db.txt", "w"); // only for test

	//for(j = 0; j < n_rows; j++)
	//{
	//	fprintf(fp, "%s\n" , Config_db[j].spname); // only for test
	//}

	//fclose(fp);
	///////////////////////////////////////////////////////////////////////////////////////////////////
	
	///Loading items into DA server
	
	//fp = fopen("C:\\scada\\logs\\opc_itemid_loaded.txt", "w"); // only for test

	// loop until all items are added
	HRESULT hr = 0;
	int nTestItem = 0; // how many items there are
	int iec_type = 0;
	int error_add_items = 0;
	int found_duplicated_item_id = 0;
	
	IEnumString* pEnumString = NULL;
	
	USES_CONVERSION;
	
	LPOLESTR pszName = NULL;
	ULONG count = 0;
	char buf[256];
	ULONG nCount = 0;

	nTestItem = 0;
	
	for(j = 0; j < n_rows; j++) //loop over each record
	{
		error_add_items = 0;

		found_duplicated_item_id = 0;
			
		strcpy(Item[nTestItem].spname, Config_db[j].spname);

		//fprintf(stderr,"Item[nTestItem].spname = %s\n", Item[nTestItem].spname);
		//fflush(stderr);

		//Look if the name is already loaded in the database
		
		for(int k_1 = 0; k_1 < nTestItem + 1; k_1++)
		{
			if(strcmp(Item[k_1].spname, Item[nTestItem].spname) == 0)
			{
				found_duplicated_item_id++; 
			}
		}

		if(found_duplicated_item_id == 2)
		{
			g_dwNumItems--;
			continue;
			//the current itemID name is already loaded in the DA server
			//so nTestItem is NOT incremented
		}
		
		//TODO: we need to free
		#define customA2W(lpa) (((_lpa = lpa) == NULL) ? NULL : (_convert = (lstrlenA(_lpa)+1), ATLA2WHELPER((LPWSTR) malloc(_convert*2), _lpa, _convert)))

		strcpy(buf, Item[nTestItem].spname);

		wcscpy(Item[nTestItem].wszName, customA2W(buf));
		
		//set VT_EMPTY and the server will select the right type////
		//strcpy(sz2,"VT_EMPTY");
		Item[nTestItem].vt = VT_EMPTY;
					
		OPCITEMRESULT *pItemResult = NULL;
		HRESULT *pErrors = NULL;
		OPCITEMDEF ItemDef;
		ItemDef.szAccessPath = L"";
		ItemDef.szItemID = Item[nTestItem].wszName;
		ItemDef.bActive = TRUE;
		ItemDef.hClient = g_dwClientHandle++; //it starts from 1
		ItemDef.dwBlobSize = 0;
		ItemDef.pBlob = NULL;
		ItemDef.vtRequestedDataType = Item[nTestItem].vt;
		Item[nTestItem].hClient = ItemDef.hClient;

		//fprintf(fp, "%s %d" ,Item[nTestItem].spname, g_dwClientHandle - 1); // only for test

		hr = g_pIOPCItemMgt->AddItems(1, &ItemDef, &pItemResult, &pErrors);

		if(FAILED(hr))
		{
			LogMessage(hr,"AddItems()");
			//nTestItem is NOT incremented
			error_add_items = 1;
			//fprintf(fp, " Removed\n");
			g_dwClientHandle--;
			g_dwNumItems--;
			continue;
		}

		hr = S_OK;

		if(pErrors == NULL)
		{
			//nTestItem is NOT incremented
			LogMessage(hr,"AddItems()");
			error_add_items = 1;
			//fprintf(fp, " Removed\n"); // only for test
			g_dwClientHandle--;
			g_dwNumItems--;
			continue;
		}

		if(FAILED(pErrors[0]))
		{
			LogMessage(pErrors[0],"AddItems() item");
			//nTestItem is NOT incremented
			error_add_items = 1;
			//fprintf(fp, " Removed\n"); // only for test
			g_dwClientHandle--;
			g_dwNumItems--;
			continue;

		}

		if(error_add_items == 0)
		{
			//fprintf(fp, " Added\n"); // only for test
		}

		// record unique handle for this item
		Item[nTestItem].hServer = pItemResult->hServer;
		Item[nTestItem].vt = pItemResult->vtCanonicalDataType;
		Item[nTestItem].dwAccessRights = pItemResult->dwAccessRights;

		::CoTaskMemFree(pItemResult);
		::CoTaskMemFree(pErrors);
			
		Item[nTestItem].ioa_control_center = Config_db[j].ioa_control_center;
		Item[nTestItem].io_list_iec_type = Config_db[j].io_list_iec_type;

		//ended to read a record

		nTestItem++;

		fprintf(stderr,"nTestItem = %d\r", nTestItem);
		fflush(stderr);
	}
		
	//fclose(fp); //only for test

	//WARNING: if g_dwNumItems is wrong the function g_pIOPCAsyncIO2->Read of General Interrogation does not work
	fprintf(stderr, "g_dwNumItems = %d\n", g_dwNumItems);
	fflush(stderr);
	
	fprintf(stderr,_T("OPC items: %d\n"), g_dwNumItems);
	fflush(stderr);

	if(g_dwNumItems <= 0)
	{
		IT_EXIT;
		return 1;
	}

	IT_EXIT;
	return(0);
}
