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

#ifndef PROC_MANAGER_H
#define PROC_MANAGER_H 

#ifdef __cplusplus
extern "C" {
#endif

extern BOOL StartProcess(char* pCommandLine, char* pWorkingDir);
extern void begin_process_checker(struct args* arg);

const int nBufferSize = 500;

struct args{
	char pCommandLine[nBufferSize+1];
	char pWorkingDir[nBufferSize+1];
	char pipe_name[150];
};

#ifdef __cplusplus
}
#endif

#endif