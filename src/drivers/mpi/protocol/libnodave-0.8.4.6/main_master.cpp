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

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <winsock2.h>
#include "clear_crc_eight.h"
#include "iec104types.h"
#include "iec_item.h"
#include "process.h"

///////////////////Start MPI specific/////////////////////////////////////////////////

//#define PLAY_WITH_KEEPALIVE
#include "nodave.h"
#include "openSocket.h"

#ifdef LINUX
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#define UNIX_STYLE
#endif

#ifdef BCCWIN
#include <time.h>
    void usage(void);
    void wait(void);
#define WIN_STYLE    
#endif

#ifdef PLAY_WITH_KEEPALIVE
#include <sys/socket.h>
#endif
#include <errno.h>

void readSZL(daveConnection *dc,int id, int index) {
    int res, SZLid, indx, SZcount, SZlen,i,j,len;
    uc * d,*dd;
    uc ddd[3000];
    printf("Trying to read SZL-ID %04X index %02X.\n",id,index);
    res=daveReadSZL(dc,id,index, ddd, 3000);
    printf("Function result: %d %s len:%d\n",res,daveStrerror(res),dc->AnswLen);
//    _daveDump("Data",dc->resultPointer,dc->AnswLen);
    _daveDump("Data",ddd,dc->AnswLen);
    
    if ((dc->AnswLen)>=4) {
	d=ddd;
        dd=ddd;
	dd+=8;
	len=dc->AnswLen-8;
	SZLid=0x100*d[0]+d[1]; 
        indx=0x100*d[2]+d[3]; 
	printf("result SZL ID %04X %02X \n",SZLid,indx);
    
	if ((dc->AnswLen)>=8) {
    	    SZlen=0x100*d[4]+d[5]; 
    	    SZcount=0x100*d[6]+d[7]; 
	    printf("%d elements of %d bytes\n",SZcount,SZlen);
	    if(len>0){
	    for (i=0;i<SZcount;i++){
		if(len>0){
		for (j=0; j<SZlen; j++){
		    if(len>0){
		    printf("%02X,",*dd);
		    dd++;
		    }
		    len--;
		}
		printf("\n");
		}
	    }
	    }
	}
    }
    printf("\n");
}    

void readSZLAll(daveConnection *dc) {
    uc SzlList[1000];
    int res, SZLid, indx, SZcount, SZlen,i,j, rid, rind;
    uc * d,*dd;
    
    res=daveReadSZL(dc,0,0,SzlList, 1000);
    printf("%d %d\n",res,dc->AnswLen);
    if ((dc->AnswLen)>=4) {
	d=dc->resultPointer;
        dd=SzlList;
	dd+=8;
	SZLid=0x100*d[0]+d[1]; 
        indx=0x100*d[2]+d[3]; 
	printf("result SZL ID %04X %02X \n",SZLid,indx);
    
	if ((dc->AnswLen)>=8) {
    	    SZlen=0x100*d[4]+d[5]; 
    	    SZcount=0x100*d[6]+d[7]; 
	    printf("%d elements of %d bytes\n",SZcount,SZlen);
	    for (i=0;i<SZcount;i++){
//		rind=*dd;
//		rid=*(dd+1); 
		rid=*(dd+1)+256*(*dd); 
		rind=0;
		printf("ID:%04X Index:%02X\n",rid,rind);
		readSZL(dc, rid, rind);
		for (j=0; j<SZlen; j++){
		    printf("%02X,",*dd);
		    dd++;
		}
		printf("\nID:%04X Index:%02X\n",rid,rind);
		
	    }
	}
    }
    printf("\n");
}


void wait() {
//    printf("Press return to continue.\n");
#ifdef UNIX_STYLE
//    read(0,&c,1);
#endif
}    
/*
void usage()
{
    printf("Usage: mpi_master [-d] [-w] IP-Address of CP\n");
    printf("-2 uses a protocol variant for the CP243. You need to set it, if you use such a CP.\n");
    printf("-w will try to write to Flag words. It will overwrite FB0 to FB15 (MB0 to MB15) !\n");
    printf("-d will produce a lot of debug messages.\n");
    printf("-b will run benchmarks. Specify -b and -w to run write benchmarks.\n");
    printf("-m will run a test for multiple variable reads.\n");
    printf("-c will write 0 to the PLC memory used in write tests.\n");
    printf("-z will read some SZL list items (diagnostic information).\n"
	    "  Works 300 and 400 family only.\n");
    printf("-s stops the PLC.\n");
    printf("-r tries to put the PLC in run mode.\n");
    printf("--readout read program and data blocks from PLC.\n");
    printf("--readoutall read all program and data blocks from PLC. Includes SFBs and SFCs.\n");
    printf("--slot=<number> sets slot for PLC (default is 2).\n");
    printf("--ram2rom tries to Copy RAM to ROM.\n");    
    printf("--debug=<number> will set daveDebug to number.\n");
    printf("Example: mpi_master --debug=131071 192.168.19.1\n");
}
*/

void loadBlocksOfType(daveConnection * dc, int blockType) {
    int j, i, uploadID, len, more;
#ifdef UNIX_STYLE
    int fd;
#endif	        
#ifdef WIN_STYLE	    
    HANDLE fd;
    unsigned long res;
#endif	        
    char blockName [20];
    uc blockBuffer[20000],*bb;
    daveBlockEntry dbe[256];   
    j=daveListBlocksOfType(dc, blockType, dbe);
    if (j<0) {
	printf("error %d = %s\n",-j,daveStrerror(-j));
	return;
    }
    printf("%d blocks of type %s\n",j,daveBlockName(blockType));
    for (i=0; i<j; i++) {
	printf("%s%d  %d %d\n",
	    daveBlockName(blockType),
	    dbe[i].number, dbe[i].type[0],dbe[i].type[1]);	
	bb=blockBuffer;
	len=0;
	if (0==initUpload(dc, blockType, dbe[i].number, &uploadID)) {
    	    do {
		doUpload(dc,&more,&bb,&len,uploadID);
	    } while (more);
	    sprintf(blockName,"%s%d.mc7",daveBlockName(blockType), dbe[i].number);	
#ifdef UNIX_STYLE
    	    fd=open(blockName,O_RDWR|O_CREAT|O_TRUNC,0644);
    	    write(fd, blockBuffer, len);
    	    close(fd);
#endif	    
#ifdef WIN_STYLE
	    fd = CreateFile(blockName,
    	      GENERIC_WRITE, 0, 0, 2,
    		FILE_FLAG_WRITE_THROUGH, 0);
    	    WriteFile(fd, blockBuffer, len, &res, NULL);
    	    CloseHandle(fd);
#endif	    

    	    endUpload(dc,uploadID);
	}    
    }
}

int seconds, thirds;

#include "benchmark.c"

////////////////////////////end MPI spedific ////////////////////////////////////////

static int timeout_connection_with_parent = 0;

#define SUPPLIER "@ enscada.com"
#define APPLICATION "mpi_master.exe"

int IsSingleInstance(const char* name)
{
   HANDLE hMutex;
   char mutex_name[200];

   strcpy(mutex_name, ""APPLICATION"");
   strcat(mutex_name, name);

   hMutex = CreateMutex(NULL, TRUE, mutex_name);

   if(GetLastError() == ERROR_ALREADY_EXISTS)
   {
      if(hMutex)
	  {
		 CloseHandle(hMutex);
	  }

	  return 0;
   }

   return 1;
}

#include "getopt.h"

struct args{
	char line_number[80];
};

void PipeWorker(void* pParam);

int main( int argc, char **argv )
{
	char version[100];
	char mpi_ServerAddress[80];
	char mpi_ServerPort[80];
	char line_number[80];
	char fifo_monitor_direction_name[70];
	char fifo_control_direction_name[70];
	char OldConsoleTitle[500];
	char NewConsoleTitle[500];
	int c;
		
	mpi_ServerAddress[0] = '\0';
	mpi_ServerPort[0] = '\0';
	line_number[0] = '\0';
	fifo_monitor_direction_name[0] = '\0';
	fifo_control_direction_name[0] = '\0';

	//version control///////////////////////////////////////////////////////////////
	sprintf(version, ""APPLICATION" - Built on %s %s %s",__DATE__,__TIME__,SUPPLIER);
	fprintf(stderr, "%s\n", version);
	fflush(stderr);
	SYSTEMTIME oT;
	::GetLocalTime(&oT);
	fprintf(stderr,"%02d/%02d/%04d, %02d:%02d:%02d Starting ... %s\n",oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,APPLICATION); 
	fflush(stderr);
	////////////////////////////////////////////////////////////////////////////////

	while( ( c = getopt ( argc, argv, "a:p:l:t:?" )) != EOF ) {
		switch ( c ) {
			case 'a' :
			strcpy(mpi_ServerAddress, optarg);
			break;
			case 'p' :
			strcpy(mpi_ServerPort, optarg);
			break;
			case 'l' :
			strcpy(line_number, optarg);
			break;
			case '?' :
			fprintf(stderr, "Run time usage: %s -a server IP address -p server IP port -l line number -t polling time\n", argv[0]);
			fflush(stderr);
			exit( 0 );
		}
	}

	if(strlen(mpi_ServerAddress) == 0)
	{
		fprintf(stderr,"MPI server IP address is not known\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}

	if(strlen(mpi_ServerPort) == 0)
	{
		fprintf(stderr,"MPI port is not known\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}

	if(strlen(line_number) == 0)
	{
		fprintf(stderr,"line_number is not known\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	
	strcpy(fifo_monitor_direction_name, "fifo_monitor_direction");
	strcpy(fifo_control_direction_name, "fifo_control_direction");
	
	if(strlen(line_number) > 0)
	{
		strcat(fifo_monitor_direction_name, line_number);
		strcat(fifo_control_direction_name, line_number);
	}

    strcat(fifo_control_direction_name, "mpi");
    strcat(fifo_monitor_direction_name, "mpi");

	strcpy(NewConsoleTitle, "mpi_master IP ");
	strcat(NewConsoleTitle, mpi_ServerAddress);
	strcat(NewConsoleTitle, " PORT ");
	strcat(NewConsoleTitle, mpi_ServerPort);
	strcat(NewConsoleTitle, " LINE ");
	strcat(NewConsoleTitle, line_number);

	if(!IsSingleInstance(NewConsoleTitle))
	{
		fprintf(stderr,"Another instance is already running\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}

	int rc;
	if((rc = GetConsoleTitle(OldConsoleTitle, sizeof(OldConsoleTitle))) > 0)
	{
		SetConsoleTitle(NewConsoleTitle);
	}

    struct args arg;
    strcpy(arg.line_number, line_number);

	if(_beginthread(PipeWorker, 0, (void*)&arg) == -1)
	{
		long nError = GetLastError();

		fprintf(stderr,"PipeWorker _beginthread failed, error code = %d", nError);
		fflush(stderr);
		return EXIT_FAILURE;	
	}

	///////////////////////////////////Start MPI client protocol//////////////////////////////////

	int a, b, adrPos,doWrite,doBenchmark, doSZLread, doMultiple, doClear,
	res, useProtocol,doSZLreadAll, doRun, doStop, doCopyRAMtoROM, doReadout, doSFBandSFC,
	doNewfunctions, saveDebug,
	useSlot, mpi_port;
#ifdef PLAY_WITH_KEEPALIVE    	
    int opt;
#endif    
    float d;
    daveInterface * di;
    daveConnection * dc;
    _daveOSserialType fds;
    PDU p;
    daveResultSet rs;
    
    daveSetDebug(daveDebugPrintErrors);
    adrPos=1;
    doWrite=0;
    doBenchmark=0;
    doMultiple=0;
    doClear=0;
    doSZLread=0;
    doSZLreadAll=0;
    doRun=0;
    doStop=0;
    doCopyRAMtoROM=0;
    doReadout=0;
    doSFBandSFC=0;
    doNewfunctions=0;
    
    useProtocol=daveProtoISOTCP;
    useSlot=2;
    
    /*
    if (argc<2) {
	usage();
	exit(-1);
    }    
    
    while (argv[adrPos][0]=='-') {
	if (strcmp(argv[adrPos],"-2")==0) {
	    useProtocol=daveProtoISOTCP243;
	} else	if (strncmp(argv[adrPos],"--debug=",8)==0) {
	    daveSetDebug(atol(argv[adrPos]+8));
	    printf("setting debug to: 0x%lx\n",atol(argv[adrPos]+8));
	} else if (strcmp(argv[adrPos],"-s")==0) {
	    doStop=1;
	} else if (strcmp(argv[adrPos],"-r")==0) {
	    doRun=1;
	} else if (strncmp(argv[adrPos],"--ram2rom",9)==0) {
	    doCopyRAMtoROM=1;    
	} else if (strncmp(argv[adrPos],"--readoutall",12)==0) {
	    doReadout=1;
	    doSFBandSFC=1;
	} else if (strncmp(argv[adrPos],"--readout",9)==0) {
	    doReadout=1;
	} else if (strncmp(argv[adrPos],"--slot=",7)==0) {
	    useSlot=atol(argv[adrPos]+7);
	} else if (strcmp(argv[adrPos],"-d")==0) {
	    daveSetDebug(daveDebugAll);
	} else if (strcmp(argv[adrPos],"-n")==0) {
	    doNewfunctions=1;
	} else
	if (strcmp(argv[adrPos],"-w")==0) {
	    doWrite=1;
	} else
	if (strcmp(argv[adrPos],"-b")==0) {
	    doBenchmark=1;
	} else
	if (strcmp(argv[adrPos],"-z")==0) {
	    doSZLread=1;
	} else
	if (strcmp(argv[adrPos],"-a")==0) {
	    doSZLreadAll=1;
	} else
	if (strcmp(argv[adrPos],"-m")==0) {
	    doMultiple=1;
	}
	adrPos++;
	if (argc<=adrPos) {
	    usage();
	    exit(-1);
	}	
    }
	*/

	mpi_port = atoi(mpi_ServerPort);
    
    fds.rfd=openSocket(mpi_port, mpi_ServerAddress);
#ifdef PLAY_WITH_KEEPALIVE
    errno=0;    
    opt=1;
    res=setsockopt(fds.rfd, SOL_SOCKET, SO_KEEPALIVE, &opt, 4);
    LOG3("setsockopt %s %d\n", strerror(errno),res);
#endif
    fds.wfd=fds.rfd;
    
    if (fds.rfd>0) 
	{ 
		di =daveNewInterface(fds,"IF1",0, useProtocol, daveSpeed187k);

		daveSetTimeout(di,5000000);

		dc =daveNewConnection(di,2,0,useSlot);  // insert your rack and slot here
	
		if (daveConnectPLC(dc) == 0) 
		{
			printf("Connected.\n");

	//	    di->timeout=1000;

	/*		
		printf("Trying to read 1 byte from system data,address 0x1e3 like MicroWin does.\n");
		daveReadBytes(dc,3,0,0x1e3,1,NULL);
		
		printf("Trying to read 20 bytes from system data,address 0 like MicroWin does.\n");
		daveReadBytes(dc,3,0,0,20,NULL);
	*/	
		printf("Trying to read 64 bytes (16 dwords) from data block 1.\n");

		wait();

		res=daveReadBytes(dc,daveDB,1,0,64,NULL);
			if(0==res) {
			a=daveGetU16(dc);
			printf("DB1:DW0: %d\n",a);
			a=daveGetU16(dc);
			printf("DB1:DW1: %d\n...\n",a);
			a=daveGetU16At(dc,62);
			printf("DB1:DW32: %d\n",a);
		} else 
			printf("failed! (%d)\n",res);  

	printf("Trying to read 16 bytes from FW0.\n");
	wait();
	/*
	 * Some comments about daveReadBytes():
	 *
	 * Here we read flags and not data blocks, because we cannot know which data blocks will
	 * exist in a user's PLC, but flags are always present. To read from data block 5 use:
	 * 	daveReadBytes(dc, daveDB, 5, 0, 16, NULL);
	 * to read DBD68 and DBW72 use:
	 * 	daveReadBytes(dc, daveDB, 5, 68, 6, NULL);
	 * to read DBD68 and DBW72 into your applications buffer appBuffer use:	
	 * 	daveReadBytes(dc, daveDB, 5, 68, 6, appBuffer);
	 * to read DBD68 and DBD78 into your applications buffer appBuffer use:	
	 * 	daveReadBytes(dc, daveDB, 5, 68, 14, appBuffer);
	 * this reads DBD68 and DBD78 and everything in between and fills the range
	 * appBuffer+4 to appBuffer+9 with unwanted bytes, but is much faster than:
	 *	daveReadBytes(dc, daveDB, 5, 68, 4, appBuffer);
	 *	daveReadBytes(dc, daveDB, 5, 78, 4, appBuffer+4);
	 *
	 * Users of the 200 family please note:
	 * The 200 family PLCs have the V area. This is accessed like a datablock with number 1.
	 * This is not a quirk or convention introduced by libnodave, but the command transmitted 
	 * to the PLC is exactly the same that would read from DB1 off a 300 or 400.
	 *
	 */	
		res=daveReadBytes(dc,daveFlags,0,0,16,NULL);
		if (0==res) { 
	/*
	 *	daveGetU32(dc); reads a word (2 bytes) from the current buffer position and increments
	 *	an internal pointer by 2, so next daveGetXXX() wil read from the new position behind that
	 *	word. If you want to read from a position you specify, use daveGetU16at(dc, position).	
	 */	
    			a=daveGetU32(dc);
    			b=daveGetU32(dc);
    			c=daveGetU32(dc);
    			d=daveGetFloat(dc);
			printf("FD0: %d\n",a);
			printf("FD4: %d\n",b);
			printf("FD8: %d\n",c);
			printf("FD12: %f\n",d);
		}  else 
			printf("failed! (%d)\n",res);  
		
		if(doSZLread) {
			readSZL(dc,0xA0,0x0);
			readSZL(dc,0x92,0x0);
			readSZL(dc,0xB4,0x1024);
			readSZL(dc,0x111,0x1);
			readSZL(dc,0x111,0x2);
			readSZL(dc,0xD91,0x0);
			readSZL(dc,0x232,0x4);
			readSZL(dc,0x1A0,0x0);
		}    
		if(doSZLreadAll) {
			readSZLAll(dc);
		}    
		if(doMultiple) {
    			printf("Now testing read multiple variables.\n"
			"This will read 1 Byte from inputs,\n"
			"4 bytes from flags, 2 bytes from DB6\n"
			" and other 2 bytes from flags\n");
    			wait();
			davePrepareReadRequest(dc, &p);
			daveAddVarToReadRequest(&p,daveInputs,0,0,1);
			daveAddVarToReadRequest(&p,daveFlags,0,0,4);
			daveAddVarToReadRequest(&p,daveDB,6,20,2);
			daveAddVarToReadRequest(&p,daveFlags,0,12,2);
			res=daveExecReadRequest(dc, &p, &rs);
			
			printf("Input Byte 0 ");
			res=daveUseResult(dc, &rs, 0); // first result
			if (res==0) {
			a=daveGetU8(dc);
        		printf("%d\n",a);
			} else 
			printf("*** Error: %s\n",daveStrerror(res));
			
			printf("Flag DWord 0 ");	
			res=daveUseResult(dc, &rs, 1); // 2nd result
			if (res==0) {
			a=daveGetS16(dc);
        		printf("%d\n",a);
			} else 
			printf("*** Error: %s\n",daveStrerror(res));
			
			printf("DB 6 Word 20: ");	
			res=daveUseResult(dc, &rs, 2); // 3rd result
			if (res==0) {
			a=daveGetS16(dc);
        		printf("%d\n",a);
			} else 
			printf("*** Error: %s\n",daveStrerror(res));
			
			printf("Flag Word 12: ");		
			res=daveUseResult(dc, &rs, 3); // 4th result
			if (res==0) {
			a=daveGetU16(dc);
        		printf("%d\n",a);
			} else 
			printf("*** Error: %s\n",daveStrerror(res));	
			
			printf("non existing result: ");		
			res=daveUseResult(dc, &rs, 4); // 5th result
			if (res==0) {
			a=daveGetU16(dc);
        		printf("%d\n",a);
			} else 
			printf("*** Error: %s\n",daveStrerror(res));		
			daveFreeResults(&rs);	
			
	/*	    
			for (i=0; i<rs.numResults;i++) {
			r2=&(rs.results[i]);
			printf("result: %s length:%d\n",daveStrerror(r2->error), r2->length);
			res=daveUseResult(dc, &rs, i);
			if (r2->length>0) _daveDump("bytes",r2->bytes,r2->length);
			if (r2->bytes!=NULL) {
	    			_daveDump("bytes",r2->bytes,r2->length);
					d=daveGetFloat(dc);
					printf("FD12: %f\n",d);
			}	 
			}
	*/	    
		}	    

    /*
	if(doWrite) {
    	    printf("Now we write back these data after incrementing the first 3 by 1,2,3 and the float by 1.1.\n");
	    wait();

//    Attention! you need to daveSwapIed little endian variables before using them as a buffer for
//    daveWriteBytes() or before copying them into a buffer for daveWriteBytes()!

    	    a=daveSwapIed_32(a+1);
    	    daveWriteBytes(dc,daveFlags,0,00,4,&a);
    	    b=daveSwapIed_32(b+2);
    	    daveWriteBytes(dc,daveFlags,0,4,4,&b);
    	    c=daveSwapIed_32(c+3);
	    daveWriteBytes(dc,daveFlags,0,8,4,&c);
    	    d=toPLCfloat((float)(d+1.1));
    	    daveWriteBytes(dc,daveFlags,0,12,4,&d);
    	    if(0==daveReadBytes(dc,daveFlags,0,0,16,NULL)) {
		a=daveGetU32(dc);
    		b=daveGetU32(dc);
    		c=daveGetU32(dc);
    		d=daveGetFloat(dc);
		printf("FD0: %d\n",a);
		printf("FD4: %d\n",b);
		printf("FD8: %d\n",c);
		printf("FD12: %f\n",d);
	    } else {
		
	    }	
    	    wait();
	} // doWrite
*/

	if(doClear) {
    	    printf("Now writing 0 to the bytes FB0...FB15.\n");
    	    wait();
	    a=0;
    	    daveWriteBytes(dc,daveFlags,0,0,4,&a);
    	    daveWriteBytes(dc,daveFlags,0,4,4,&a);
	    daveWriteBytes(dc,daveFlags,0,8,4,&a);
    	    daveWriteBytes(dc,daveFlags,0,12,4,&a);
	    daveReadBytes(dc,daveFlags,0,0,16,NULL);
    	    a=daveGetU32(dc);
    	    b=daveGetU32(dc);
    	    c=daveGetU32(dc);
    	    d=daveGetFloat(dc);
	    printf("FD0: %d\n",a);
	    printf("FD4: %d\n",b);
	    printf("FD8: %d\n",c);
	    printf("FD12: %f\n",d);
	} // doClear

	if(doNewfunctions) {
	    saveDebug=daveGetDebug();
	    
	    printf("\nTrying to read two consecutive bits from DB11.DBX0.1 (supposed to fail)\n");;
	    res=daveReadBits(dc, daveDB, 11, 1, 2,NULL);
	    printf("function result:%d=%s\n", res, daveStrerror(res));
	    
	    printf("\nTrying to read no bit (length 0) from DB17.DBX0.1 (supposed to fail)\n");
	    res=daveReadBits(dc, daveDB, 17, 1, 0,NULL);
	    printf("function result:%d=%s\n", res, daveStrerror(res));

	    daveSetDebug(daveGetDebug()|daveDebugPDU); 
	    printf("\nTrying to read a single bit from DB17.DBX0.3 (supposed to work if you have a DB17)\n");
	    res=daveReadBits(dc, daveDB, 17, 3, 1,NULL);
	    printf("function result:%d=%s\n", res, daveStrerror(res));
	
	    printf("\nTrying to read a single bit from E0.2 (supposed to work)\n");
	    res=daveReadBits(dc, daveInputs, 0, 2, 1,NULL);
	    printf("function result:%d=%s\n", res, daveStrerror(res));
	    
	    a=0;
	    printf("\nWriting 0 to EB0 (supposed to work)\n");
	    res=daveWriteBytes(dc, daveOutputs, 0, 0, 1, &a);

	    a=1;
	    printf("\nTrying to set single bit E0.5 (supposed to work)\n");
	    res=daveWriteBits(dc, daveOutputs, 0, 5, 1, &a);
	    printf("function result:%d=%s\n", res, daveStrerror(res));
	
	    printf("\nTrying to read 1 byte from AAW0 (supposed to work on S7-2xx)\n");
	    res=daveReadBytes(dc, daveAnaIn, 0, 0, 2,NULL);
	    printf("function result:%d=%s\n", res, daveStrerror(res));
	    
	    a=2341;
	    printf("\nTrying to write 1 word (2 bytes) to AAW0 (supposed to work on S7-2xx)\n");
	    res=daveWriteBytes(dc, daveAnaOut, 0, 0, 2,&a);
	    printf("function result:%d=%s\n", res, daveStrerror(res));
	
	    printf("\nTrying to read 4 items from Timers (supposed to work S7-3xx/4xx)\n");
	    res=daveReadBytes(dc, daveTimer, 0, 0, 4,NULL);
	    printf("function result:%d=%s\n", res, daveStrerror(res));

	    if (res==0) {
		printf("\nShowing the results using daveGetSeconds(dc)\n");
		d=daveGetSeconds(dc);
		printf("Time: %0.3f, ",d);
		d=daveGetSeconds(dc);
		printf("%0.3f, ",d);
		d=daveGetSeconds(dc);
		printf("%0.3f, ",d);
		d=daveGetSeconds(dc);
		printf(" %0.3f\n",d);
		printf("\nShowing the same results using daveGetSecondsAt(dc,position)\n");
		d=daveGetSecondsAt(dc,0);
		printf("Time: %0.3f, ",d);
		d=daveGetSecondsAt(dc,2);
		printf("%0.3f, ",d);
		d=daveGetSecondsAt(dc,4);
		printf("%0.3f, ",d);
		d=daveGetSecondsAt(dc,6);
		printf(" %0.3f\n",d);
	    }
	    printf("\nTrying to read 4 items from Counters (supposed to work S7-3xx/4xx)\n");
	    res=daveReadBytes(dc, daveCounter, 0, 0, 4,NULL);
	    printf("function result:%d=%s\n", res, daveStrerror(res));
	    if (res==0) {
		printf("\nShowing the results using daveGetCounterValue(dc)\n");
		c=daveGetCounterValue(dc);
		printf("Count: %d, ",c);
		c=daveGetCounterValue(dc);
		printf("%d, ",c);
		c=daveGetCounterValue(dc);
		printf("%d, ",c);
		c=daveGetCounterValue(dc);
		printf(" %d\n",c);
	    
		printf("\nShowing the same results using daveGetCounterValueAt(dc, position)\n");
		c=daveGetCounterValueAt(dc,0);
		printf("Count: %d, ",c);
		c=daveGetCounterValueAt(dc,2);
		printf("%d, ",c);
		c=daveGetCounterValueAt(dc,4);
		printf("%d, ",c);
		c=daveGetCounterValueAt(dc,6);
		printf(" %d\n",c);
	    }
	    
	    printf("\nTrying to read multiple items with multiple variable read. (1st 2 supposed to work ok, rests may debend on CPU type and contents)\n");
	    davePrepareReadRequest(dc, &p);
	    daveAddVarToReadRequest(&p,daveInputs,0,0,1);
	    daveAddVarToReadRequest(&p,daveFlags,0,0,4);
	    daveAddVarToReadRequest(&p,daveDB,6,20,2);
	    daveAddVarToReadRequest(&p,daveTimer,0,0,4);
	    daveAddVarToReadRequest(&p,daveTimer,0,1,4);
	    daveAddVarToReadRequest(&p,daveTimer,0,2,4);
	    daveAddVarToReadRequest(&p,daveCounter,0,0,4);
	    daveAddVarToReadRequest(&p,daveCounter,0,1,4);
	    daveAddVarToReadRequest(&p,daveCounter,0,2,4);
	    res=daveExecReadRequest(dc, &p, &rs);
	    
	    daveSetDebug(saveDebug);
	}

	if(doStop) {
	    daveStop(dc);
	}
	if(doRun) {
	    daveStart(dc);
	}

	/*
	if(doCopyRAMtoROM) {
	    res = daveCopyRAMtoROM(dc);
            printf("RetVal=(%04X)\n",res);  	    
	}
	*/

	if(doReadout) {
	    loadBlocksOfType(dc, daveBlockType_OB);
	    loadBlocksOfType(dc, daveBlockType_FB);
	    loadBlocksOfType(dc, daveBlockType_FC);
	    loadBlocksOfType(dc, daveBlockType_DB);
	    loadBlocksOfType(dc, daveBlockType_SDB);
	    if (doSFBandSFC){	    
		loadBlocksOfType(dc, daveBlockType_SFB);
	        loadBlocksOfType(dc, daveBlockType_SFC);
	    }
	}    
	
	/*
	if(doBenchmark) {

	    if(useProtocol==daveProtoISOTCP243) { // we have a 200 CPU, use V memory
		rBenchmark(dc, daveDB);
	        if(doWrite) {
		    wBenchmark(dc, daveDB);
	        } // doWrite
	    } else {				// we have a 300/400 CPU, use Flags(Merkers)
		rBenchmark(dc, daveFlags);
		if(doWrite) {
		    wBenchmark(dc, daveFlags);
		} // doWrite
	    }	
	} // doBenchmark
	*/

	closeSocket(fds.rfd);
	printf("Finished.\n");
	
	return 0;

	} 
	else 
	{
	    printf("Couldn't connect to PLC.\n Please make sure you use the -2 option with a CP243 but not with CPs 343 or 443.\n");	
	    closeSocket(fds.rfd);
	    return -2;
	}

    } 
	else 
	{
		printf("Couldn't open TCP port. \nPlease make sure a CP is connected and the IP address is ok. \n");	
    	return -1;
    }    

	///////////////////////////////////End MPI client protocol//////////////////////////////////
}

///////////////////////////////////Keep alive pipe management/////////////////////////////////////////////////////
#define BUF_SIZE 200
#define N_PIPES 3

void PipeWorker(void* pParam)
{
	HANDLE pipeHnds[N_PIPES];
	char in_buffer[N_PIPES][BUF_SIZE];
	OVERLAPPED ovrp[N_PIPES];
	HANDLE evnt[N_PIPES];
	DWORD rc, len, pipe_id;
    unsigned char buf[sizeof(struct iec_item)];
    struct iec_item* p_item;
	char pipe_name[150];
    int i;

    struct args* arg = (struct args*)pParam;

	strcpy(pipe_name, "\\\\.\\pipe\\mpi_master_namedpipe");
    strcat(pipe_name, arg->line_number);

	for(i = 0; i < N_PIPES; i++)
	{
		if ((pipeHnds[i] = CreateNamedPipe(
			pipe_name,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, N_PIPES,
			0, 
			0, 
			1000, 
			NULL)) == INVALID_HANDLE_VALUE)
		{
			fprintf(stderr,"CreateNamedPipe for pipe %d failed with error %d\n", i, GetLastError());
			fflush(stderr);
			ExitProcess(0);
		}

		if ((evnt[i] = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		{
			fprintf(stderr,"CreateEvent for pipe %d failed with error %d\n",	i, GetLastError());
			fflush(stderr);
			ExitProcess(0);
		}

		ZeroMemory(&ovrp[i], sizeof(OVERLAPPED));

		ovrp[i].hEvent = evnt[i];

		if (ConnectNamedPipe(pipeHnds[i], &ovrp[i]) == 0)
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				fprintf(stderr,"ConnectNamedPipe for pipe %d failed with error %d\n", i, GetLastError());
				fflush(stderr);
				
				CloseHandle(pipeHnds[i]);
				ExitProcess(0);
			}
		}
	}

	while(1)
	{
		if((rc = WaitForMultipleObjects(N_PIPES, evnt, FALSE, INFINITE)) == WAIT_FAILED)
		{
			fprintf(stderr,"WaitForMultipleObjects failed with error %d\n", GetLastError());
			fflush(stderr);
			ExitProcess(0);
		}

		pipe_id = rc - WAIT_OBJECT_0;

		ResetEvent(evnt[pipe_id]);

		if(GetOverlappedResult(pipeHnds[pipe_id], &ovrp[pipe_id], &len, TRUE) == 0)
		{
			fprintf(stderr,"GetOverlapped result failed %d start over\n", GetLastError());
			fflush(stderr);
		
			if(DisconnectNamedPipe(pipeHnds[pipe_id]) == 0)
			{
				fprintf(stderr,"DisconnectNamedPipe failed with error %d\n", GetLastError());
				fflush(stderr);
				ExitProcess(0);
			}

			if(ConnectNamedPipe(pipeHnds[pipe_id],	&ovrp[pipe_id]) == 0)
			{
				if(GetLastError() != ERROR_IO_PENDING)
				{
					fprintf(stderr,"ConnectNamedPipe for pipe %d failed with error %d\n", i, GetLastError());
					fflush(stderr);
					CloseHandle(pipeHnds[pipe_id]);
				}
			}
		}
		else
		{
			ZeroMemory(&ovrp[pipe_id], sizeof(OVERLAPPED));

			ovrp[pipe_id].hEvent = evnt[pipe_id];

			if((rc = ReadFile(pipeHnds[pipe_id], in_buffer[pipe_id], sizeof(struct iec_item), NULL, &ovrp[pipe_id])) == 0)
			{
				if(GetLastError() != ERROR_IO_PENDING)
				{
					fprintf(stderr,"ReadFile failed with error %d\n", GetLastError());
					fflush(stderr);
				}
			}
			
			memcpy(buf, in_buffer[pipe_id], sizeof(struct iec_item));
			
			if(len)
			{
				p_item = (struct iec_item*)buf;

				//fprintf(stderr, "Receiving from pipe %d th message\n", p_item->msg_id);
				//fflush(stderr);
											
				//for (j = 0; j < len; j++) 
				//{ 
					//unsigned char c = *((unsigned char*)buf + j);
					//fprintf(stderr, "rx pipe <--- 0x%02x-", c);
					//fflush(stderr);
					//
				//}
				
				rc = clearCrc((unsigned char *)buf, sizeof(struct iec_item));
				if(rc != 0)
				{
					ExitProcess(0);
				}

				if(p_item->iec_obj.ioa == 4004)
				{ 
					timeout_connection_with_parent = 0;
					//fprintf(stderr, "Receive keep alive # %d from front end\n", p_item->msg_id);
                    fprintf(stderr, "wdg %d\r", p_item->msg_id);
				    fflush(stderr);
				}
			}
		}
	}
}
