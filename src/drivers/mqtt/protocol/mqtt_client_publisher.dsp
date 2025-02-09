# Microsoft Developer Studio Project File - Name="mqtt_client_publisher" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=mqtt_client_publisher - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mqtt_client_publisher.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mqtt_client_publisher.mak" CFG="mqtt_client_publisher - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mqtt_client_publisher - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "mqtt_client_publisher - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mqtt_client_publisher - Win32 Release"

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
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "$(QTDIR)\include" /I ".." /I "..\.." /I "..\..\..\common" /I "..\..\..\database" /I "..\..\..\utilities" /I "..\..\..\fifo" /I "..\..\..\configurator\sqlite" /I "..\..\..\middleware\ripc\inc" /I "..\..\..\middleware\rtps\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "BUILDING_WOLFMQTT" /D "RIPC_DLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib rpcrt4.lib fifo.lib rtps.lib utilities.lib /nologo /subsystem:console /machine:I386 /out:"c:\scada\bin/mqtt_client_publisher.exe" /libpath:"$(QTDIR)\lib" /libpath:"c:\scada\lib"

!ELSEIF  "$(CFG)" == "mqtt_client_publisher - Win32 Debug"

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
F90=df.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "." /I "$(QTDIR)\include" /I ".." /I "..\.." /I "..\..\..\common" /I "..\..\..\database" /I "..\..\..\utilities" /I "..\..\..\fifo" /I "..\..\..\configurator\sqlite" /I "..\..\fifo" /I "..\..\..\middleware\ripc\inc" /I "..\..\..\middleware\rtps\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "BUILDING_WOLFMQTT" /D "RIPC_DLL" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib rpcrt4.lib fifo.lib rtps.lib utilitiesd.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"c:\scada\bin/mqtt_client_publisher.exe" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"c:\scada\lib"

!ENDIF 

# Begin Target

# Name "mqtt_client_publisher - Win32 Release"
# Name "mqtt_client_publisher - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\json\cJSON.c
# End Source File
# Begin Source File

SOURCE=..\..\..\utilities\clear_crc_eight.c
# End Source File
# Begin Source File

SOURCE=.\GeneralHashFunctions.c
# End Source File
# Begin Source File

SOURCE=..\..\..\utilities\getopt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\iec_item_type.c
# End Source File
# Begin Source File

SOURCE=.\json\json.c
# End Source File
# Begin Source File

SOURCE=.\load_database_publisher.cpp
# End Source File
# Begin Source File

SOURCE=.\main_publisher.cpp
# End Source File
# Begin Source File

SOURCE=.\mqtt_client.c
# End Source File
# Begin Source File

SOURCE=.\mqtt_client_app_publisher.cpp
# End Source File
# Begin Source File

SOURCE=.\mqtt_packet.c
# End Source File
# Begin Source File

SOURCE=.\mqtt_socket.c
# End Source File
# Begin Source File

SOURCE=.\mqttnet.c
# End Source File
# Begin Source File

SOURCE=.\tahu\src\pb_common.c
# End Source File
# Begin Source File

SOURCE=.\tahu\src\pb_decode.c
# End Source File
# Begin Source File

SOURCE=.\tahu\src\pb_encode.c
# End Source File
# Begin Source File

SOURCE=..\..\..\configurator\sqlite\sqlite3.c
# End Source File
# Begin Source File

SOURCE=.\tahu\src\tahu.c
# End Source File
# Begin Source File

SOURCE=.\tahu\src\tahu.pb.c
# End Source File
# End Group
# End Target
# End Project
