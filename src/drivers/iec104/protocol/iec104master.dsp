# Microsoft Developer Studio Project File - Name="iec104master" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=iec104master - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iec104master.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iec104master.mak" CFG="iec104master - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iec104master - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "iec104master - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iec104master - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\libevent-1.4.13-stable\WIN32-Code" /I ".\libevent-1.4.13-stable" /I ".\libevent-1.4.13-stable\compat\sys" /I "..\..\..\fifo" /I "..\..\..\utilities" /I "..\..\..\middleware\ripc\inc" /I "..\..\..\middleware\rtps\include" /I "..\..\..\common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_EVENT_HAVE_GETTIMEOFDAY" /D "RIPC_DLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib libevent.lib rpcrt4.lib ripcclient.lib fifo.lib rtps.lib /nologo /subsystem:console /machine:I386 /out:"c:\scada\bin/iec104master.exe" /libpath:".\libevent-1.4.13-stable\WIN32-Prj\Release" /libpath:"c:\scada\lib"

!ELSEIF  "$(CFG)" == "iec104master - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I ".\libevent-1.4.13-stable\WIN32-Code" /I ".\libevent-1.4.13-stable" /I ".\libevent-1.4.13-stable\compat\sys" /I "..\..\..\fifo" /I "..\..\..\utilities" /I "..\..\..\middleware\ripc\inc" /I "..\..\..\middleware\rtps\include" /I "..\..\..\common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_EVENT_HAVE_GETTIMEOFDAY" /D "RIPC_DLL" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib libevent.lib rpcrt4.lib ripcclient.lib fifo.lib rtps.lib /nologo /stack:0x200000 /subsystem:console /incremental:no /debug /machine:I386 /out:"c:\scada\bin/iec104master.exe" /pdbtype:sept /libpath:".\libevent-1.4.13-stable\WIN32-Prj\Debug" /libpath:"c:\scada\lib"

!ENDIF 

# Begin Target

# Name "iec104master - Win32 Release"
# Name "iec104master - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\asdu.c
# End Source File
# Begin Source File

SOURCE=..\..\..\middleware\rtps\librtps\cdr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\utilities\clear_crc_eight.c
# End Source File
# Begin Source File

SOURCE=.\client.c
# End Source File
# Begin Source File

SOURCE=..\..\..\utilities\getopt.c
# End Source File
# Begin Source File

SOURCE=.\iec104_imp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\iec_item_type.c
# End Source File
# Begin Source File

SOURCE=.\iec_library.c
# End Source File
# Begin Source File

SOURCE=.\main_master.cpp
# End Source File
# End Group
# End Target
# End Project
