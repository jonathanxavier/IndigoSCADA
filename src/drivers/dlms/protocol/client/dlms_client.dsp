# Microsoft Developer Studio Project File - Name="dlms_client" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=dlms_client - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dlms_client.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dlms_client.mak" CFG="dlms_client - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dlms_client - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "dlms_client - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dlms_client - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "dlms_client - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "dlms_client - Win32 Release"
# Name "dlms_client - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\lib\src\apdu.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\bitarray.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\bytebuffer.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\ciphering.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\client.c
# End Source File
# Begin Source File

SOURCE=.\src\communication.c
# End Source File
# Begin Source File

SOURCE=.\src\connection.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\converters.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\cosem.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\datainfo.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\date.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\dlms.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\dlmsSettings.c
# End Source File
# Begin Source File

SOURCE=.\src\getopt.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\gxaes.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\gxarray.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\gxget.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\gxkey.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\gxmd5.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\gxobjects.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\gxserializer.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\gxset.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\gxsha1.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\gxsha256.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\gxvalueeventargs.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\helpers.c
# End Source File
# Begin Source File

SOURCE=.\src\main.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\message.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\objectarray.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\parameters.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\replydata.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\serverevents.c
# End Source File
# Begin Source File

SOURCE=..\lib\src\variant.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
