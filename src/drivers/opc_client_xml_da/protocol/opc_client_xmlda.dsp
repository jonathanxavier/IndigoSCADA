# Microsoft Developer Studio Project File - Name="opc_client_xmlda" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=opc_client_xmlda - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "opc_client_xmlda.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "opc_client_xmlda.mak" CFG="opc_client_xmlda - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "opc_client_xmlda - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "opc_client_xmlda - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "opc_client_xmlda - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\libxml2-2.7.2\include" /I ".\libsoap-1.1.0" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"c:\scada\bin/opc_client_xmlda.exe"

!ELSEIF  "$(CFG)" == "opc_client_xmlda - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".\libxml2-2.7.2\include" /I ".\libsoap-1.1.0" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /out:"c:\scada\bin/opc_client_xmlda.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "opc_client_xmlda - Win32 Release"
# Name "opc_client_xmlda - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=".\libxml2-2.7.2\c14n.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\catalog.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\chvalid.c"
# End Source File
# Begin Source File

SOURCE=.\client.cpp
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\debugXML.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\dict.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\DOCBparser.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\encoding.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\entities.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\error.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\globals.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\hash.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\HTMLparser.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\HTMLtree.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\legacy.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\list.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\nanoftp.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\nanohttp\nanohttp-base64.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\nanohttp\nanohttp-client.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\nanohttp\nanohttp-common.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\nanohttp\nanohttp-logging.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\nanohttp\nanohttp-mime.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\nanohttp\nanohttp-request.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\nanohttp\nanohttp-response.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\nanohttp\nanohttp-server.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\nanohttp\nanohttp-socket.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\nanohttp\nanohttp-ssl.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\nanohttp\nanohttp-stream.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\nanohttp.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\parser.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\parserInternals.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\pattern.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\relaxng.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\SAX.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\SAX2.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\schematron.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\libcsoap\soap-client.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\libcsoap\soap-ctx.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\libcsoap\soap-env.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\libcsoap\soap-fault.c"
# End Source File
# Begin Source File

SOURCE=".\libsoap-1.1.0\libcsoap\soap-xml.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\threads.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\tree.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\trio.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\trionan.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\triostr.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\uri.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\valid.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xinclude.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xlink.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xmlIO.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xmlmemory.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xmlmodule.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xmlreader.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xmlregexp.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xmlsave.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xmlschemas.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xmlschemastypes.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xmlstring.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xmlunicode.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xmlwriter.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xpath.c"
# End Source File
# Begin Source File

SOURCE=".\libxml2-2.7.2\xpointer.c"
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
