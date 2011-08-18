# Microsoft Developer Studio Project File - Name="modbus_master" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=modbus_master - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "modbus_master.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "modbus_master.mak" CFG="modbus_master - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "modbus_master - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "modbus_master - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "modbus_master - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IEC_104_DRIVER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "." /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /I "..\..\gigabase" /I "..\..\common" /I ".." /I "..\..\database" /I "..\..\ui\widgets" /I "..\..\trace" /I "..\..\utilities" /I "..\..\fifo" /I "..\..\common\libds-2.2" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_USRDLL" /D "IEC_104_DRIVER_EXPORTS" /D "USING_GARRET" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "QT_H_CPP" /D "_WIN32_DCOM" /FD /GZ /c
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
# ADD LINK32 advapi32.lib scadad.lib utilitiesd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib uuid.lib WSOCK32.LIB snmpapi.lib qt-mt$(QTVER).lib fifo.lib /nologo /dll /incremental:no /pdb:"Debug/modbus_master.pdb" /debug /machine:I386 /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"c:\scada\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "modbus_master - Win32 Release"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "$(QTDIR)\include" /I "..\..\gigabase" /I "..\..\common" /I ".." /I "..\..\database" /I "..\..\ui\widgets" /I "..\..\STL" /I "..\..\trace" /I "..\..\utilities" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_USRDLL" /D "IEC_104_DRIVER_EXPORTS" /D "USING_GARRET" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /I "..\..\gigabase" /I "..\..\common" /I ".." /I "..\..\database" /I "..\..\ui\widgets" /I "..\..\trace" /I "..\..\utilities" /I "..\..\fifo" /I "..\..\common\libds-2.2" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_USRDLL" /D "IEC_104_DRIVER_EXPORTS" /D "USING_GARRET" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "QT_H_CPP" /D "_WIN32_DCOM" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x410 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib WSOCK32.LIB scada.lib snmpapi.lib qtdll.lib /nologo /dll /pdb:"Debug/modbus_master.pdb" /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"c:\scada\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 scada.lib utilities.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib uuid.lib WSOCK32.LIB snmpapi.lib qt-mt$(QTVER).lib fifo.lib /nologo /dll /incremental:no /pdb:"Debug/modbus_master.pdb" /debug /machine:I386 /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"c:\scada\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "modbus_master - Win32 Debug"
# Name "modbus_master - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\modbus_master.cpp
# End Source File
# Begin Source File

SOURCE=.\modbus_master.h

!IF  "$(CFG)" == "modbus_master - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_master.h...
InputDir=.
InputPath=.\modbus_master.h
InputName=modbus_master

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "modbus_master - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_master.h...
InputDir=.
InputPath=.\modbus_master.h
InputName=modbus_master

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\modbus_master_instance.cpp
# End Source File
# Begin Source File

SOURCE=.\modbus_master_instance.h

!IF  "$(CFG)" == "modbus_master - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_master_instance.h...
InputDir=.
InputPath=.\modbus_master_instance.h
InputName=modbus_master_instance

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "modbus_master - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_master_instance.h...
InputDir=.
InputPath=.\modbus_master_instance.h
InputName=modbus_master_instance

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\modbus_masterCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\modbus_masterCommand.h

!IF  "$(CFG)" == "modbus_master - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterCommand.h...
InputDir=.
InputPath=.\modbus_masterCommand.h
InputName=modbus_masterCommand

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "modbus_master - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterCommand.h...
InputDir=.
InputPath=.\modbus_masterCommand.h
InputName=modbus_masterCommand

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\modbus_masterCommandData.cpp
# End Source File
# Begin Source File

SOURCE=.\modbus_masterCommandData.h

!IF  "$(CFG)" == "modbus_master - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterCommandData.h...
InputDir=.
InputPath=.\modbus_masterCommandData.h
InputName=modbus_masterCommandData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "modbus_master - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterCommandData.h...
InputDir=.
InputPath=.\modbus_masterCommandData.h
InputName=modbus_masterCommandData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\modbus_masterConfiguration.cpp
# End Source File
# Begin Source File

SOURCE=.\modbus_masterConfiguration.h

!IF  "$(CFG)" == "modbus_master - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterConfiguration.h...
InputDir=.
InputPath=.\modbus_masterConfiguration.h
InputName=modbus_masterConfiguration

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "modbus_master - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterConfiguration.h...
InputDir=.
InputPath=.\modbus_masterConfiguration.h
InputName=modbus_masterConfiguration

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\modbus_masterConfigurationData.cpp
# End Source File
# Begin Source File

SOURCE=.\modbus_masterConfigurationData.h

!IF  "$(CFG)" == "modbus_master - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterConfigurationData.h...
InputDir=.
InputPath=.\modbus_masterConfigurationData.h
InputName=modbus_masterConfigurationData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "modbus_master - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterConfigurationData.h...
InputDir=.
InputPath=.\modbus_masterConfigurationData.h
InputName=modbus_masterConfigurationData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\modbus_masterInput.cpp
# End Source File
# Begin Source File

SOURCE=.\modbus_masterInput.h

!IF  "$(CFG)" == "modbus_master - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterInput.h...
InputDir=.
InputPath=.\modbus_masterInput.h
InputName=modbus_masterInput

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "modbus_master - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterInput.h...
InputDir=.
InputPath=.\modbus_masterInput.h
InputName=modbus_masterInput

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\modbus_masterInputData.cpp
# End Source File
# Begin Source File

SOURCE=.\modbus_masterInputData.h

!IF  "$(CFG)" == "modbus_master - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterInputData.h...
InputDir=.
InputPath=.\modbus_masterInputData.h
InputName=modbus_masterInputData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "modbus_master - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing modbus_masterInputData.h...
InputDir=.
InputPath=.\modbus_masterInputData.h
InputName=modbus_masterInputData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\moc_modbus_master.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_modbus_master_instance.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_modbus_masterCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_modbus_masterCommandData.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_modbus_masterConfiguration.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_modbus_masterConfigurationData.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_modbus_masterInput.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_modbus_masterInputData.cpp
# End Source File
# End Group
# End Target
# End Project
