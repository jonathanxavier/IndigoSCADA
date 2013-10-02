# Microsoft Developer Studio Project File - Name="opc_client_hda_driver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=opc_client_hda_driver - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "opc_client_hda_driver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "opc_client_hda_driver.mak" CFG="opc_client_hda_driver - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "opc_client_hda_driver - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "opc_client_hda_driver - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "opc_client_hda_driver - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OPC_CLIENT_AE_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "." /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /I "..\..\gigabase" /I "..\..\common" /I ".." /I "..\..\database" /I "..\..\ui\widgets" /I "..\..\trace" /I "..\..\utilities" /I "..\..\fifo" /I "..\..\common\libds-2.2" /I "..\..\middleware\rtps\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_USRDLL" /D "OPC_CLIENT_HDA_EXPORTS" /D "USING_GARRET" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "QT_H_CPP" /D "_WIN32_DCOM" /FD /GZ /c
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
# ADD LINK32 advapi32.lib scadad.lib utilitiesd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib uuid.lib WSOCK32.LIB snmpapi.lib qt-mt3.lib rtps.lib /nologo /dll /incremental:no /pdb:"Debug/opc_client_hda_driver.pdb" /debug /machine:I386 /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"c:\scada\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "opc_client_hda_driver - Win32 Release"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "$(QTDIR)\include" /I "..\..\gigabase" /I "..\..\common" /I ".." /I "..\..\database" /I "..\..\ui\widgets" /I "..\..\STL" /I "..\..\trace" /I "..\..\utilities" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_USRDLL" /D "OPC_CLIENT_AE_EXPORTS" /D "USING_GARRET" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /I "..\..\gigabase" /I "..\..\common" /I ".." /I "..\..\database" /I "..\..\ui\widgets" /I "..\..\trace" /I "..\..\utilities" /I "..\..\fifo" /I "..\..\common\libds-2.2" /I "..\..\middleware\rtps\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_USRDLL" /D "OPC_CLIENT_HDA_EXPORTS" /D "USING_GARRET" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "QT_H_CPP" /D "_WIN32_DCOM" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x410 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib WSOCK32.LIB scada.lib snmpapi.lib qtdll.lib /nologo /dll /pdb:"Debug/opc_client_hda_driver.pdb" /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"c:\scada\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 scada.lib utilities.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib uuid.lib WSOCK32.LIB snmpapi.lib qt-mt3.lib /nologo /dll /incremental:no /pdb:"Release/opc_client_hda_driver.pdb" /debug /machine:I386 /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"c:\scada\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "opc_client_hda_driver - Win32 Debug"
# Name "opc_client_hda_driver - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\middleware\rtps\librtps\cdr.c
# End Source File
# Begin Source File

SOURCE=..\..\utilities\clear_crc_eight.c
# End Source File
# Begin Source File

SOURCE=..\..\common\iec_item_type.c
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_hda.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_hda_instance.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_hdaCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_hdaCommandData.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_hdaConfiguration.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_hdaConfigurationData.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_hdaInput.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_opc_client_hdaInputData.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_hda.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_hda.h

!IF  "$(CFG)" == "opc_client_hda_driver - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hda.h...
InputDir=.
InputPath=.\opc_client_hda.h
InputName=opc_client_hda

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_hda_driver - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hda.h...
InputDir=.
InputPath=.\opc_client_hda.h
InputName=opc_client_hda

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_hda_instance.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_hda_instance.h

!IF  "$(CFG)" == "opc_client_hda_driver - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hda_instance.h...
InputDir=.
InputPath=.\opc_client_hda_instance.h
InputName=opc_client_hda_instance

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_hda_driver - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hda_instance.h...
InputDir=.
InputPath=.\opc_client_hda_instance.h
InputName=opc_client_hda_instance

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaCommand.h

!IF  "$(CFG)" == "opc_client_hda_driver - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaCommand.h...
InputDir=.
InputPath=.\opc_client_hdaCommand.h
InputName=opc_client_hdaCommand

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_hda_driver - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaCommand.h...
InputDir=.
InputPath=.\opc_client_hdaCommand.h
InputName=opc_client_hdaCommand

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaCommandData.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaCommandData.h

!IF  "$(CFG)" == "opc_client_hda_driver - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaCommandData.h...
InputDir=.
InputPath=.\opc_client_hdaCommandData.h
InputName=opc_client_hdaCommandData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_hda_driver - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaCommandData.h...
InputDir=.
InputPath=.\opc_client_hdaCommandData.h
InputName=opc_client_hdaCommandData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaConfiguration.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaConfiguration.h

!IF  "$(CFG)" == "opc_client_hda_driver - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaConfiguration.h...
InputDir=.
InputPath=.\opc_client_hdaConfiguration.h
InputName=opc_client_hdaConfiguration

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_hda_driver - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaConfiguration.h...
InputDir=.
InputPath=.\opc_client_hdaConfiguration.h
InputName=opc_client_hdaConfiguration

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaConfigurationData.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaConfigurationData.h

!IF  "$(CFG)" == "opc_client_hda_driver - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaConfigurationData.h...
InputDir=.
InputPath=.\opc_client_hdaConfigurationData.h
InputName=opc_client_hdaConfigurationData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_hda_driver - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaConfigurationData.h...
InputDir=.
InputPath=.\opc_client_hdaConfigurationData.h
InputName=opc_client_hdaConfigurationData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_hdadriverthread.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_hdadriverthread.h
# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaInput.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaInput.h

!IF  "$(CFG)" == "opc_client_hda_driver - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaInput.h...
InputDir=.
InputPath=.\opc_client_hdaInput.h
InputName=opc_client_hdaInput

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_hda_driver - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaInput.h...
InputDir=.
InputPath=.\opc_client_hdaInput.h
InputName=opc_client_hdaInput

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaInputData.cpp
# End Source File
# Begin Source File

SOURCE=.\opc_client_hdaInputData.h

!IF  "$(CFG)" == "opc_client_hda_driver - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaInputData.h...
InputDir=.
InputPath=.\opc_client_hdaInputData.h
InputName=opc_client_hdaInputData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "opc_client_hda_driver - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing opc_client_hdaInputData.h...
InputDir=.
InputPath=.\opc_client_hdaInputData.h
InputName=opc_client_hdaInputData

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project