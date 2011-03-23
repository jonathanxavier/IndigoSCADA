# Microsoft Developer Studio Project File - Name="opc_client_com" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=opc_client_com - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "opc_client_com.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "opc_client_com.mak" CFG="opc_client_com - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "opc_client_com - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "opc_client_com - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "opc_client_com - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\scada\Drivers"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OPC_CLIENT_COM_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "." /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /I "..\..\gigabase" /I "..\..\common" /I ".." /I "..\..\database" /I "..\..\ui\widgets" /I "..\..\trace" /I "..\..\utilities" /I "..\..\containers\sglib-1.0.4" /I "..\..\fifo" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_USRDLL" /D "OPC_CLIENT_COM_EXPORTS" /D "USING_GARRET" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "QT_H_CPP" /D "_WIN32_DCOM" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x410 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 advapi32.lib scadad.lib utilitiesd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib uuid.lib WSOCK32.LIB snmpapi.lib qt-mt$(QTVER).lib fifo.lib /nologo /dll /incremental:no /pdb:"Debug/opc_client_com.pdb" /debug /machine:I386 /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"c:\scada\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "opc_client_com - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\scada\Drivers"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "$(QTDIR)\include" /I "..\..\gigabase" /I "..\..\common" /I ".." /I "..\..\database" /I "..\..\ui\widgets" /I "..\..\STL" /I "..\..\trace" /I "..\..\utilities" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_USRDLL" /D "OPC_CLIENT_COM_EXPORTS" /D "USING_GARRET" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /I "..\..\gigabase" /I "..\..\common" /I ".." /I "..\..\database" /I "..\..\ui\widgets" /I "..\..\trace" /I "..\..\utilities" /I "..\..\containers\sglib-1.0.4" /I "..\..\fifo" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_USRDLL" /D "OPC_CLIENT_COM_EXPORTS" /D "USING_GARRET" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "QT_H_CPP" /D "_WIN32_DCOM" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x410 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib WSOCK32.LIB scada.lib snmpapi.lib qtdll.lib /nologo /dll /pdb:"Debug/opc_client_com.pdb" /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"c:\scada\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 scada.lib utilities.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib uuid.lib WSOCK32.LIB snmpapi.lib qt-mt$(QTVER).lib fifo.lib /nologo /dll /incremental:no /pdb:"Debug/opc_client_com.pdb" /debug /machine:I386 /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"c:\scada\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "opc_client_com - Win32 Debug"
# Name "opc_client_com - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\moc_opc_client_com.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_com_instance.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_comCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_comCommandData.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_comConfiguration.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_comConfigurationData.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_comInput.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_comInputData.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_com.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_com.h

!IF  "$(CFG)" == "opc_client_com - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_com.h...
InputDir=.
InputPath=.\opc_client_com.h
InputName=opc_client_com

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_com - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_com.h...
InputDir=.
InputPath=.\opc_client_com.h
InputName=opc_client_com

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_com_instance.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_com_instance.h

!IF  "$(CFG)" == "opc_client_com - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_com_instance.h...
InputDir=.
InputPath=.\opc_client_com_instance.h
InputName=opc_client_com_instance

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_com - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_com_instance.h...
InputDir=.
InputPath=.\opc_client_com_instance.h
InputName=opc_client_com_instance

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_comCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_comCommand.h

!IF  "$(CFG)" == "opc_client_com - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comCommand.h...
InputDir=.
InputPath=.\opc_client_comCommand.h
InputName=opc_client_comCommand

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_com - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comCommand.h...
InputDir=.
InputPath=.\opc_client_comCommand.h
InputName=opc_client_comCommand

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_comCommandData.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_comCommandData.h

!IF  "$(CFG)" == "opc_client_com - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comCommandData.h...
InputDir=.
InputPath=.\opc_client_comCommandData.h
InputName=opc_client_comCommandData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_com - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comCommandData.h...
InputDir=.
InputPath=.\opc_client_comCommandData.h
InputName=opc_client_comCommandData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_comConfiguration.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_comConfiguration.h

!IF  "$(CFG)" == "opc_client_com - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comConfiguration.h...
InputDir=.
InputPath=.\opc_client_comConfiguration.h
InputName=opc_client_comConfiguration

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_com - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comConfiguration.h...
InputDir=.
InputPath=.\opc_client_comConfiguration.h
InputName=opc_client_comConfiguration

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_comConfigurationData.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_comConfigurationData.h

!IF  "$(CFG)" == "opc_client_com - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comConfigurationData.h...
InputDir=.
InputPath=.\opc_client_comConfigurationData.h
InputName=opc_client_comConfigurationData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_com - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comConfigurationData.h...
InputDir=.
InputPath=.\opc_client_comConfigurationData.h
InputName=opc_client_comConfigurationData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_comdriverthread.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_comdriverthread.h
# End Source File
# Begin Source File

SOURCE=.\opc_client_comInput.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_comInput.h

!IF  "$(CFG)" == "opc_client_com - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comInput.h...
InputDir=.
InputPath=.\opc_client_comInput.h
InputName=opc_client_comInput

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_com - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comInput.h...
InputDir=.
InputPath=.\opc_client_comInput.h
InputName=opc_client_comInput

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_comInputData.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_comInputData.h

!IF  "$(CFG)" == "opc_client_com - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comInputData.h...
InputDir=.
InputPath=.\opc_client_comInputData.h
InputName=opc_client_comInputData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_com - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_comInputData.h...
InputDir=.
InputPath=.\opc_client_comInputData.h
InputName=opc_client_comInputData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opcclient.cpp
# End Source File
# Begin Source File

SOURCE=.\opccomn.h
# End Source File
# Begin Source File

SOURCE=.\opccomn_i.c
# End Source File
# Begin Source File

SOURCE=.\opcda.h
# End Source File
# Begin Source File

SOURCE=.\Opcda_i.c
# End Source File
# Begin Source File

SOURCE=.\OpcEnum_i.c
# End Source File
# End Group
# End Target
# End Project
