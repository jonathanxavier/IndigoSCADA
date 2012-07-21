# Microsoft Developer Studio Project File - Name="designerlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=designerlib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "designerlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "designerlib.mak" CFG="designerlib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "designerlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "designerlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "designerlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "/lib"
# PROP BASE Intermediate_Dir "."
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GX /O1 /I "..\shared" /I "..\uilib" /I "$(QTDIR)\include" /I "." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "UNICODE" /D "DESIGNER" /D "QT_INTERNAL_XML" /D "QT_INTERNAL_WORKSPACE" /D "QT_INTERNAL_ICONVIEW" /D "QT_INTERNAL_TABLE" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /FD -Zm200 /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"C:\scada\lib\designerlib.lib"

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\shared" /I "..\uilib" /I "$(QTDIR)\include" /I "." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "UNICODE" /D "DESIGNER" /D "QT_INTERNAL_XML" /D "QT_INTERNAL_WORKSPACE" /D "QT_INTERNAL_ICONVIEW" /D "QT_INTERNAL_TABLE" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /FD /GZ -Zm200 /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"C:\scada\lib\designerlib.lib"

!ENDIF 

# Begin Target

# Name "designerlib - Win32 Release"
# Name "designerlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=actiondnd.cpp
# End Source File
# Begin Source File

SOURCE=actioneditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=actionlistview.cpp
# End Source File
# Begin Source File

SOURCE=asciivalidator.cpp
# End Source File
# Begin Source File

SOURCE=command.cpp
# End Source File
# Begin Source File

SOURCE=config.cpp
# End Source File
# Begin Source File

SOURCE=configtoolboxdialog.ui.h
# End Source File
# Begin Source File

SOURCE=connectiondialog.ui.h
# End Source File
# Begin Source File

SOURCE=connectionitems.cpp
# End Source File
# Begin Source File

SOURCE=connectiontable.cpp
# End Source File
# Begin Source File

SOURCE=customwidgeteditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\database.cpp
# End Source File
# Begin Source File

SOURCE=.\database.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing database.h...
InputDir=.
InputPath=.\database.h
InputName=database

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing database.h...
InputDir=.
InputPath=.\database.h
InputName=database

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\database2.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing database2.h...
InputDir=.
InputPath=.\database2.h
InputName=database2

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing database2.h...
InputDir=.
InputPath=.\database2.h
InputName=database2

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dbconnection.cpp
# End Source File
# Begin Source File

SOURCE=.\dbconnection.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing dbconnection.h...
InputDir=.
InputPath=.\dbconnection.h
InputName=dbconnection

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing dbconnection.h...
InputDir=.
InputPath=.\dbconnection.h
InputName=dbconnection

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dbconnectioneditor.cpp
# End Source File
# Begin Source File

SOURCE=.\dbconnectioneditor.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing dbconnectioneditor.h...
InputDir=.
InputPath=.\dbconnectioneditor.h
InputName=dbconnectioneditor

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing dbconnectioneditor.h...
InputDir=.
InputPath=.\dbconnectioneditor.h
InputName=dbconnectioneditor

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dbconnectionimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\dbconnectionimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing dbconnectionimpl.h...
InputDir=.
InputPath=.\dbconnectionimpl.h
InputName=dbconnectionimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing dbconnectionimpl.h...
InputDir=.
InputPath=.\dbconnectionimpl.h
InputName=dbconnectionimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dbconnections.cpp
# End Source File
# Begin Source File

SOURCE=.\dbconnections.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing dbconnections.h...
InputDir=.
InputPath=.\dbconnections.h
InputName=dbconnections

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing dbconnections.h...
InputDir=.
InputPath=.\dbconnections.h
InputName=dbconnections

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dbconnectionsimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\dbconnectionsimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing dbconnectionsimpl.h...
InputDir=.
InputPath=.\dbconnectionsimpl.h
InputName=dbconnectionsimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing dbconnectionsimpl.h...
InputDir=.
InputPath=.\dbconnectionsimpl.h
InputName=dbconnectionsimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=defs.cpp
# End Source File
# Begin Source File

SOURCE=designerapp.cpp
# End Source File
# Begin Source File

SOURCE=designerappiface.cpp
# End Source File
# Begin Source File

SOURCE=editfunctionsimpl.cpp
# End Source File
# Begin Source File

SOURCE=filechooser.cpp
# End Source File
# Begin Source File

SOURCE=finddialog.ui.h
# End Source File
# Begin Source File

SOURCE=formfile.cpp
# End Source File
# Begin Source File

SOURCE=formsettingsimpl.cpp
# End Source File
# Begin Source File

SOURCE=formwindow.cpp
# End Source File
# Begin Source File

SOURCE=gotolinedialog.ui.h
# End Source File
# Begin Source File

SOURCE=hierarchyview.cpp
# End Source File
# Begin Source File

SOURCE=iconvieweditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=layout.cpp
# End Source File
# Begin Source File

SOURCE=listboxdnd.cpp
# End Source File
# Begin Source File

SOURCE=listboxeditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=listboxrename.cpp
# End Source File
# Begin Source File

SOURCE=listdnd.cpp
# End Source File
# Begin Source File

SOURCE=listeditor.ui.h
# End Source File
# Begin Source File

SOURCE=listviewdnd.cpp
# End Source File
# Begin Source File

SOURCE=listvieweditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=mainwindow.cpp
# End Source File
# Begin Source File

SOURCE=mainwindowactions.cpp
# End Source File
# Begin Source File

SOURCE=metadatabase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_database.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_database2.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_dbconnection.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_dbconnectioneditor.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_dbconnectionimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_dbconnections.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_dbconnectionsimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_tableeditor.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_tableeditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=multilineeditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=newformimpl.cpp
# End Source File
# Begin Source File

SOURCE=orderindicator.cpp
# End Source File
# Begin Source File

SOURCE=outputwindow.cpp
# End Source File
# Begin Source File

SOURCE=paletteeditoradvancedimpl.cpp
# End Source File
# Begin Source File

SOURCE=paletteeditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\parser.cpp
# End Source File
# Begin Source File

SOURCE=pixmapchooser.cpp
# End Source File
# Begin Source File

SOURCE=pixmapcollection.cpp
# End Source File
# Begin Source File

SOURCE=pixmapcollectioneditor.ui.h
# End Source File
# Begin Source File

SOURCE=previewframe.cpp
# End Source File
# Begin Source File

SOURCE=previewwidgetimpl.cpp
# End Source File
# Begin Source File

SOURCE=project.cpp
# End Source File
# Begin Source File

SOURCE=projectsettingsimpl.cpp
# End Source File
# Begin Source File

SOURCE=propertyeditor.cpp
# End Source File
# Begin Source File

SOURCE=propertyobject.cpp
# End Source File
# Begin Source File

SOURCE=qcategorywidget.cpp
# End Source File
# Begin Source File

SOURCE=qcompletionedit.cpp
# End Source File
# Begin Source File

SOURCE=replacedialog.ui.h
# End Source File
# Begin Source File

SOURCE=resource.cpp
# End Source File
# Begin Source File

SOURCE=richtextfontdialog.ui.h
# End Source File
# Begin Source File

SOURCE=sizehandle.cpp
# End Source File
# Begin Source File

SOURCE=sourceeditor.cpp
# End Source File
# Begin Source File

SOURCE=sourcefile.cpp
# End Source File
# Begin Source File

SOURCE=startdialogimpl.cpp
# End Source File
# Begin Source File

SOURCE=styledbutton.cpp
# End Source File
# Begin Source File

SOURCE=syntaxhighlighter_html.cpp
# End Source File
# Begin Source File

SOURCE=.\tableeditor.cpp
# End Source File
# Begin Source File

SOURCE=.\tableeditor.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing tableeditor.h...
InputDir=.
InputPath=.\tableeditor.h
InputName=tableeditor

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing tableeditor.h...
InputDir=.
InputPath=.\tableeditor.h
InputName=tableeditor

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tableeditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\tableeditorimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing tableeditorimpl.h...
InputDir=.
InputPath=.\tableeditorimpl.h
InputName=tableeditorimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing tableeditorimpl.h...
InputDir=.
InputPath=.\tableeditorimpl.h
InputName=tableeditorimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"%qtdir%\bin\moc.exe" "$(InputDir)\$(InputName).h" -o "$(InputDir)\moc_$(InputName).cpp"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=timestamp.cpp
# End Source File
# Begin Source File

SOURCE=variabledialogimpl.cpp
# End Source File
# Begin Source File

SOURCE=widgetaction.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\widgetdatabase.cpp
# End Source File
# Begin Source File

SOURCE=widgetfactory.cpp
# End Source File
# Begin Source File

SOURCE=wizardeditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=workspace.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=actiondnd.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__ACTIO="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing actiondnd.h...
InputPath=actiondnd.h

"moc_actiondnd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc actiondnd.h -o moc_actiondnd.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__ACTIO="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing actiondnd.h...
InputPath=actiondnd.h

"moc_actiondnd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc actiondnd.h -o moc_actiondnd.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=actioneditorimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__ACTION="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing actioneditorimpl.h...
InputPath=actioneditorimpl.h

"moc_actioneditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc actioneditorimpl.h -o moc_actioneditorimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__ACTION="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing actioneditorimpl.h...
InputPath=actioneditorimpl.h

"moc_actioneditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc actioneditorimpl.h -o moc_actioneditorimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\interfaces\actioninterface.h
# End Source File
# Begin Source File

SOURCE=actionlistview.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__ACTIONL="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing actionlistview.h...
InputPath=actionlistview.h

"moc_actionlistview.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc actionlistview.h -o moc_actionlistview.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__ACTIONL="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing actionlistview.h...
InputPath=actionlistview.h

"moc_actionlistview.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc actionlistview.h -o moc_actionlistview.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=asciivalidator.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__ASCII="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing asciivalidator.h...
InputPath=asciivalidator.h

"moc_asciivalidator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc asciivalidator.h -o moc_asciivalidator.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__ASCII="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing asciivalidator.h...
InputPath=asciivalidator.h

"moc_asciivalidator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc asciivalidator.h -o moc_asciivalidator.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=command.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__COMMA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing command.h...
InputPath=command.h

"moc_command.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc command.h -o moc_command.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__COMMA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing command.h...
InputPath=command.h

"moc_command.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc command.h -o moc_command.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=config.h
# End Source File
# Begin Source File

SOURCE=connectionitems.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__CONNE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing connectionitems.h...
InputPath=connectionitems.h

"moc_connectionitems.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc connectionitems.h -o moc_connectionitems.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__CONNE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing connectionitems.h...
InputPath=connectionitems.h

"moc_connectionitems.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc connectionitems.h -o moc_connectionitems.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=connectiontable.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__CONNEC="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing connectiontable.h...
InputPath=connectiontable.h

"moc_connectiontable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc connectiontable.h -o moc_connectiontable.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__CONNEC="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing connectiontable.h...
InputPath=connectiontable.h

"moc_connectiontable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc connectiontable.h -o moc_connectiontable.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=customwidgeteditorimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__CUSTO="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing customwidgeteditorimpl.h...
InputPath=customwidgeteditorimpl.h

"moc_customwidgeteditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc customwidgeteditorimpl.h -o moc_customwidgeteditorimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__CUSTO="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing customwidgeteditorimpl.h...
InputPath=customwidgeteditorimpl.h

"moc_customwidgeteditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc customwidgeteditorimpl.h -o moc_customwidgeteditorimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=defs.h
# End Source File
# Begin Source File

SOURCE=designerapp.h
# End Source File
# Begin Source File

SOURCE=designerappiface.h
# End Source File
# Begin Source File

SOURCE=..\interfaces\designerinterface.h
# End Source File
# Begin Source File

SOURCE=editfunctionsimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__EDITF="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing editfunctionsimpl.h...
InputPath=editfunctionsimpl.h

"moc_editfunctionsimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc editfunctionsimpl.h -o moc_editfunctionsimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__EDITF="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing editfunctionsimpl.h...
InputPath=editfunctionsimpl.h

"moc_editfunctionsimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc editfunctionsimpl.h -o moc_editfunctionsimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=filechooser.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__FILEC="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing filechooser.h...
InputPath=filechooser.h

"moc_filechooser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc filechooser.h -o moc_filechooser.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__FILEC="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing filechooser.h...
InputPath=filechooser.h

"moc_filechooser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc filechooser.h -o moc_filechooser.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\interfaces\filterinterface.h
# End Source File
# Begin Source File

SOURCE=formfile.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__FORMF="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing formfile.h...
InputPath=formfile.h

"moc_formfile.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc formfile.h -o moc_formfile.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__FORMF="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing formfile.h...
InputPath=formfile.h

"moc_formfile.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc formfile.h -o moc_formfile.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=formsettingsimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__FORMS="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing formsettingsimpl.h...
InputPath=formsettingsimpl.h

"moc_formsettingsimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc formsettingsimpl.h -o moc_formsettingsimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__FORMS="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing formsettingsimpl.h...
InputPath=formsettingsimpl.h

"moc_formsettingsimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc formsettingsimpl.h -o moc_formsettingsimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=formwindow.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__FORMW="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing formwindow.h...
InputPath=formwindow.h

"moc_formwindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc formwindow.h -o moc_formwindow.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__FORMW="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing formwindow.h...
InputPath=formwindow.h

"moc_formwindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc formwindow.h -o moc_formwindow.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=hierarchyview.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__HIERA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing hierarchyview.h...
InputPath=hierarchyview.h

"moc_hierarchyview.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc hierarchyview.h -o moc_hierarchyview.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__HIERA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing hierarchyview.h...
InputPath=hierarchyview.h

"moc_hierarchyview.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc hierarchyview.h -o moc_hierarchyview.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=iconvieweditorimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__ICONV="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing iconvieweditorimpl.h...
InputPath=iconvieweditorimpl.h

"moc_iconvieweditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc iconvieweditorimpl.h -o moc_iconvieweditorimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__ICONV="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing iconvieweditorimpl.h...
InputPath=iconvieweditorimpl.h

"moc_iconvieweditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc iconvieweditorimpl.h -o moc_iconvieweditorimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=layout.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__LAYOU="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing layout.h...
InputPath=layout.h

"moc_layout.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc layout.h -o moc_layout.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__LAYOU="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing layout.h...
InputPath=layout.h

"moc_layout.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc layout.h -o moc_layout.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=listboxdnd.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__LISTB="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listboxdnd.h...
InputPath=listboxdnd.h

"moc_listboxdnd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listboxdnd.h -o moc_listboxdnd.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__LISTB="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listboxdnd.h...
InputPath=listboxdnd.h

"moc_listboxdnd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listboxdnd.h -o moc_listboxdnd.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=listboxeditorimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__LISTBO="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listboxeditorimpl.h...
InputPath=listboxeditorimpl.h

"moc_listboxeditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listboxeditorimpl.h -o moc_listboxeditorimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__LISTBO="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listboxeditorimpl.h...
InputPath=listboxeditorimpl.h

"moc_listboxeditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listboxeditorimpl.h -o moc_listboxeditorimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=listboxrename.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__LISTBOX="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listboxrename.h...
InputPath=listboxrename.h

"moc_listboxrename.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listboxrename.h -o moc_listboxrename.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__LISTBOX="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listboxrename.h...
InputPath=listboxrename.h

"moc_listboxrename.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listboxrename.h -o moc_listboxrename.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=listdnd.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__LISTD="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listdnd.h...
InputPath=listdnd.h

"moc_listdnd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listdnd.h -o moc_listdnd.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__LISTD="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listdnd.h...
InputPath=listdnd.h

"moc_listdnd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listdnd.h -o moc_listdnd.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=listviewdnd.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__LISTV="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listviewdnd.h...
InputPath=listviewdnd.h

"moc_listviewdnd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listviewdnd.h -o moc_listviewdnd.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__LISTV="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listviewdnd.h...
InputPath=listviewdnd.h

"moc_listviewdnd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listviewdnd.h -o moc_listviewdnd.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=listvieweditorimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__LISTVI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listvieweditorimpl.h...
InputPath=listvieweditorimpl.h

"moc_listvieweditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listvieweditorimpl.h -o moc_listvieweditorimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__LISTVI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing listvieweditorimpl.h...
InputPath=listvieweditorimpl.h

"moc_listvieweditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc listvieweditorimpl.h -o moc_listvieweditorimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=mainwindow.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__MAINW="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing mainwindow.h...
InputPath=mainwindow.h

"moc_mainwindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc mainwindow.h -o moc_mainwindow.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__MAINW="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing mainwindow.h...
InputPath=mainwindow.h

"moc_mainwindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc mainwindow.h -o moc_mainwindow.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=metadatabase.h
# End Source File
# Begin Source File

SOURCE=multilineeditorimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__MULTI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing multilineeditorimpl.h...
InputPath=multilineeditorimpl.h

"moc_multilineeditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc multilineeditorimpl.h -o moc_multilineeditorimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__MULTI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing multilineeditorimpl.h...
InputPath=multilineeditorimpl.h

"moc_multilineeditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc multilineeditorimpl.h -o moc_multilineeditorimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=newformimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__NEWFO="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing newformimpl.h...
InputPath=newformimpl.h

"moc_newformimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc newformimpl.h -o moc_newformimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__NEWFO="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing newformimpl.h...
InputPath=newformimpl.h

"moc_newformimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc newformimpl.h -o moc_newformimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=orderindicator.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__ORDER="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing orderindicator.h...
InputPath=orderindicator.h

"moc_orderindicator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc orderindicator.h -o moc_orderindicator.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__ORDER="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing orderindicator.h...
InputPath=orderindicator.h

"moc_orderindicator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc orderindicator.h -o moc_orderindicator.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=outputwindow.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__OUTPU="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing outputwindow.h...
InputPath=outputwindow.h

"moc_outputwindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc outputwindow.h -o moc_outputwindow.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__OUTPU="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing outputwindow.h...
InputPath=outputwindow.h

"moc_outputwindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc outputwindow.h -o moc_outputwindow.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=paletteeditoradvancedimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PALET="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing paletteeditoradvancedimpl.h...
InputPath=paletteeditoradvancedimpl.h

"moc_paletteeditoradvancedimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc paletteeditoradvancedimpl.h -o moc_paletteeditoradvancedimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PALET="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing paletteeditoradvancedimpl.h...
InputPath=paletteeditoradvancedimpl.h

"moc_paletteeditoradvancedimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc paletteeditoradvancedimpl.h -o moc_paletteeditoradvancedimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=paletteeditorimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PALETT="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing paletteeditorimpl.h...
InputPath=paletteeditorimpl.h

"moc_paletteeditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc paletteeditorimpl.h -o moc_paletteeditorimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PALETT="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing paletteeditorimpl.h...
InputPath=paletteeditorimpl.h

"moc_paletteeditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc paletteeditorimpl.h -o moc_paletteeditorimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\shared\parser.h
# End Source File
# Begin Source File

SOURCE=pixmapchooser.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PIXMA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing pixmapchooser.h...
InputPath=pixmapchooser.h

"moc_pixmapchooser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc pixmapchooser.h -o moc_pixmapchooser.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PIXMA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing pixmapchooser.h...
InputPath=pixmapchooser.h

"moc_pixmapchooser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc pixmapchooser.h -o moc_pixmapchooser.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=pixmapcollection.h
# End Source File
# Begin Source File

SOURCE=previewframe.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PREVI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing previewframe.h...
InputPath=previewframe.h

"moc_previewframe.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc previewframe.h -o moc_previewframe.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PREVI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing previewframe.h...
InputPath=previewframe.h

"moc_previewframe.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc previewframe.h -o moc_previewframe.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=previewwidgetimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PREVIE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing previewwidgetimpl.h...
InputPath=previewwidgetimpl.h

"moc_previewwidgetimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc previewwidgetimpl.h -o moc_previewwidgetimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PREVIE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing previewwidgetimpl.h...
InputPath=previewwidgetimpl.h

"moc_previewwidgetimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc previewwidgetimpl.h -o moc_previewwidgetimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=project.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PROJE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing project.h...
InputPath=project.h

"moc_project.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc project.h -o moc_project.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PROJE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing project.h...
InputPath=project.h

"moc_project.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc project.h -o moc_project.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=projectsettingsimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PROJEC="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing projectsettingsimpl.h...
InputPath=projectsettingsimpl.h

"moc_projectsettingsimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc projectsettingsimpl.h -o moc_projectsettingsimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PROJEC="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing projectsettingsimpl.h...
InputPath=projectsettingsimpl.h

"moc_projectsettingsimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc projectsettingsimpl.h -o moc_projectsettingsimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=propertyeditor.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PROPE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing propertyeditor.h...
InputPath=propertyeditor.h

"moc_propertyeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc propertyeditor.h -o moc_propertyeditor.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PROPE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing propertyeditor.h...
InputPath=propertyeditor.h

"moc_propertyeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc propertyeditor.h -o moc_propertyeditor.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=propertyobject.h
# End Source File
# Begin Source File

SOURCE=qcategorywidget.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__QCATE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing qcategorywidget.h...
InputPath=qcategorywidget.h

"moc_qcategorywidget.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc qcategorywidget.h -o moc_qcategorywidget.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__QCATE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing qcategorywidget.h...
InputPath=qcategorywidget.h

"moc_qcategorywidget.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc qcategorywidget.h -o moc_qcategorywidget.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=qcompletionedit.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__QCOMP="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing qcompletionedit.h...
InputPath=qcompletionedit.h

"moc_qcompletionedit.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc qcompletionedit.h -o moc_qcompletionedit.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__QCOMP="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing qcompletionedit.h...
InputPath=qcompletionedit.h

"moc_qcompletionedit.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc qcompletionedit.h -o moc_qcompletionedit.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=resource.h
# End Source File
# Begin Source File

SOURCE=sizehandle.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__SIZEH="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing sizehandle.h...
InputPath=sizehandle.h

"moc_sizehandle.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc sizehandle.h -o moc_sizehandle.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__SIZEH="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing sizehandle.h...
InputPath=sizehandle.h

"moc_sizehandle.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc sizehandle.h -o moc_sizehandle.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=sourceeditor.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__SOURC="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing sourceeditor.h...
InputPath=sourceeditor.h

"moc_sourceeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc sourceeditor.h -o moc_sourceeditor.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__SOURC="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing sourceeditor.h...
InputPath=sourceeditor.h

"moc_sourceeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc sourceeditor.h -o moc_sourceeditor.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=sourcefile.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__SOURCE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing sourcefile.h...
InputPath=sourcefile.h

"moc_sourcefile.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc sourcefile.h -o moc_sourcefile.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__SOURCE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing sourcefile.h...
InputPath=sourcefile.h

"moc_sourcefile.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc sourcefile.h -o moc_sourcefile.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=startdialogimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__START="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing startdialogimpl.h...
InputPath=startdialogimpl.h

"moc_startdialogimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc startdialogimpl.h -o moc_startdialogimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__START="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing startdialogimpl.h...
InputPath=startdialogimpl.h

"moc_startdialogimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc startdialogimpl.h -o moc_startdialogimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=styledbutton.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__STYLE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing styledbutton.h...
InputPath=styledbutton.h

"moc_styledbutton.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc styledbutton.h -o moc_styledbutton.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__STYLE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing styledbutton.h...
InputPath=styledbutton.h

"moc_styledbutton.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc styledbutton.h -o moc_styledbutton.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=syntaxhighlighter_html.h
# End Source File
# Begin Source File

SOURCE=timestamp.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__TIMES="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing timestamp.h...
InputPath=timestamp.h

"moc_timestamp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc timestamp.h -o moc_timestamp.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__TIMES="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing timestamp.h...
InputPath=timestamp.h

"moc_timestamp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc timestamp.h -o moc_timestamp.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=variabledialogimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__VARIA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing variabledialogimpl.h...
InputPath=variabledialogimpl.h

"moc_variabledialogimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc variabledialogimpl.h -o moc_variabledialogimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__VARIA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing variabledialogimpl.h...
InputPath=variabledialogimpl.h

"moc_variabledialogimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc variabledialogimpl.h -o moc_variabledialogimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=widgetaction.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__WIDGE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing widgetaction.h...
InputPath=widgetaction.h

"moc_widgetaction.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc widgetaction.h -o moc_widgetaction.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__WIDGE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing widgetaction.h...
InputPath=widgetaction.h

"moc_widgetaction.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc widgetaction.h -o moc_widgetaction.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\shared\widgetdatabase.h
# End Source File
# Begin Source File

SOURCE=widgetfactory.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__WIDGET="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing widgetfactory.h...
InputPath=widgetfactory.h

"moc_widgetfactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc widgetfactory.h -o moc_widgetfactory.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__WIDGET="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing widgetfactory.h...
InputPath=widgetfactory.h

"moc_widgetfactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc widgetfactory.h -o moc_widgetfactory.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\interfaces\widgetinterface.h
# End Source File
# Begin Source File

SOURCE=wizardeditorimpl.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__WIZAR="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing wizardeditorimpl.h...
InputPath=wizardeditorimpl.h

"moc_wizardeditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc wizardeditorimpl.h -o moc_wizardeditorimpl.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__WIZAR="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing wizardeditorimpl.h...
InputPath=wizardeditorimpl.h

"moc_wizardeditorimpl.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc wizardeditorimpl.h -o moc_wizardeditorimpl.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=workspace.h

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__WORKS="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing workspace.h...
InputPath=workspace.h

"moc_workspace.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc workspace.h -o moc_workspace.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__WORKS="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing workspace.h...
InputPath=workspace.h

"moc_workspace.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc workspace.h -o moc_workspace.cpp

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Forms"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=about.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__ABOUT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing about.ui...
InputPath=about.ui

BuildCmds= \
	$(QTDIR)\bin\uic about.ui -o about.h \
	$(QTDIR)\bin\uic about.ui -i about.h -o about.cpp \
	$(QTDIR)\bin\moc about.h -o moc_about.cpp \
	

"about.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"about.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_about.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__ABOUT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing about.ui...
InputPath=about.ui

BuildCmds= \
	$(QTDIR)\bin\uic about.ui -o about.h \
	$(QTDIR)\bin\uic about.ui -i about.h -o about.cpp \
	$(QTDIR)\bin\moc about.h -o moc_about.cpp \
	

"about.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"about.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_about.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=actioneditor.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__ACTIONE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing actioneditor.ui...
InputPath=actioneditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic actioneditor.ui -o actioneditor.h \
	$(QTDIR)\bin\uic actioneditor.ui -i actioneditor.h -o actioneditor.cpp \
	$(QTDIR)\bin\moc actioneditor.h -o moc_actioneditor.cpp \
	

"actioneditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"actioneditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_actioneditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__ACTIONE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing actioneditor.ui...
InputPath=actioneditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic actioneditor.ui -o actioneditor.h \
	$(QTDIR)\bin\uic actioneditor.ui -i actioneditor.h -o actioneditor.cpp \
	$(QTDIR)\bin\moc actioneditor.h -o moc_actioneditor.cpp \
	

"actioneditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"actioneditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_actioneditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=configtoolboxdialog.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__CONFI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing configtoolboxdialog.ui...
InputPath=configtoolboxdialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic configtoolboxdialog.ui -o configtoolboxdialog.h \
	$(QTDIR)\bin\uic configtoolboxdialog.ui -i configtoolboxdialog.h -o configtoolboxdialog.cpp \
	$(QTDIR)\bin\moc configtoolboxdialog.h -o moc_configtoolboxdialog.cpp \
	

"configtoolboxdialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"configtoolboxdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_configtoolboxdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__CONFI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing configtoolboxdialog.ui...
InputPath=configtoolboxdialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic configtoolboxdialog.ui -o configtoolboxdialog.h \
	$(QTDIR)\bin\uic configtoolboxdialog.ui -i configtoolboxdialog.h -o configtoolboxdialog.cpp \
	$(QTDIR)\bin\moc configtoolboxdialog.h -o moc_configtoolboxdialog.cpp \
	

"configtoolboxdialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"configtoolboxdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_configtoolboxdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=connectiondialog.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__CONNECT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing connectiondialog.ui...
InputPath=connectiondialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic connectiondialog.ui -o connectiondialog.h \
	$(QTDIR)\bin\uic connectiondialog.ui -i connectiondialog.h -o connectiondialog.cpp \
	$(QTDIR)\bin\moc connectiondialog.h -o moc_connectiondialog.cpp \
	

"connectiondialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"connectiondialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_connectiondialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__CONNECT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing connectiondialog.ui...
InputPath=connectiondialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic connectiondialog.ui -o connectiondialog.h \
	$(QTDIR)\bin\uic connectiondialog.ui -i connectiondialog.h -o connectiondialog.cpp \
	$(QTDIR)\bin\moc connectiondialog.h -o moc_connectiondialog.cpp \
	

"connectiondialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"connectiondialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_connectiondialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=createtemplate.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__CREAT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing createtemplate.ui...
InputPath=createtemplate.ui

BuildCmds= \
	$(QTDIR)\bin\uic createtemplate.ui -o createtemplate.h \
	$(QTDIR)\bin\uic createtemplate.ui -i createtemplate.h -o createtemplate.cpp \
	$(QTDIR)\bin\moc createtemplate.h -o moc_createtemplate.cpp \
	

"createtemplate.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"createtemplate.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_createtemplate.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__CREAT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing createtemplate.ui...
InputPath=createtemplate.ui

BuildCmds= \
	$(QTDIR)\bin\uic createtemplate.ui -o createtemplate.h \
	$(QTDIR)\bin\uic createtemplate.ui -i createtemplate.h -o createtemplate.cpp \
	$(QTDIR)\bin\moc createtemplate.h -o moc_createtemplate.cpp \
	

"createtemplate.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"createtemplate.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_createtemplate.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=customwidgeteditor.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__CUSTOM="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing customwidgeteditor.ui...
InputPath=customwidgeteditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic customwidgeteditor.ui -o customwidgeteditor.h \
	$(QTDIR)\bin\uic customwidgeteditor.ui -i customwidgeteditor.h -o customwidgeteditor.cpp \
	$(QTDIR)\bin\moc customwidgeteditor.h -o moc_customwidgeteditor.cpp \
	

"customwidgeteditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"customwidgeteditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_customwidgeteditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__CUSTOM="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing customwidgeteditor.ui...
InputPath=customwidgeteditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic customwidgeteditor.ui -o customwidgeteditor.h \
	$(QTDIR)\bin\uic customwidgeteditor.ui -i customwidgeteditor.h -o customwidgeteditor.cpp \
	$(QTDIR)\bin\moc customwidgeteditor.h -o moc_customwidgeteditor.cpp \
	

"customwidgeteditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"customwidgeteditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_customwidgeteditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=editfunctions.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__EDITFU="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing editfunctions.ui...
InputPath=editfunctions.ui

BuildCmds= \
	$(QTDIR)\bin\uic editfunctions.ui -o editfunctions.h \
	$(QTDIR)\bin\uic editfunctions.ui -i editfunctions.h -o editfunctions.cpp \
	$(QTDIR)\bin\moc editfunctions.h -o moc_editfunctions.cpp \
	

"editfunctions.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"editfunctions.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_editfunctions.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__EDITFU="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing editfunctions.ui...
InputPath=editfunctions.ui

BuildCmds= \
	$(QTDIR)\bin\uic editfunctions.ui -o editfunctions.h \
	$(QTDIR)\bin\uic editfunctions.ui -i editfunctions.h -o editfunctions.cpp \
	$(QTDIR)\bin\moc editfunctions.h -o moc_editfunctions.cpp \
	

"editfunctions.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"editfunctions.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_editfunctions.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=finddialog.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__FINDD="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing finddialog.ui...
InputPath=finddialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic finddialog.ui -o finddialog.h \
	$(QTDIR)\bin\uic finddialog.ui -i finddialog.h -o finddialog.cpp \
	$(QTDIR)\bin\moc finddialog.h -o moc_finddialog.cpp \
	

"finddialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"finddialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_finddialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__FINDD="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing finddialog.ui...
InputPath=finddialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic finddialog.ui -o finddialog.h \
	$(QTDIR)\bin\uic finddialog.ui -i finddialog.h -o finddialog.cpp \
	$(QTDIR)\bin\moc finddialog.h -o moc_finddialog.cpp \
	

"finddialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"finddialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_finddialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=formsettings.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__FORMSE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing formsettings.ui...
InputPath=formsettings.ui

BuildCmds= \
	$(QTDIR)\bin\uic formsettings.ui -o formsettings.h \
	$(QTDIR)\bin\uic formsettings.ui -i formsettings.h -o formsettings.cpp \
	$(QTDIR)\bin\moc formsettings.h -o moc_formsettings.cpp \
	

"formsettings.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"formsettings.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_formsettings.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__FORMSE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing formsettings.ui...
InputPath=formsettings.ui

BuildCmds= \
	$(QTDIR)\bin\uic formsettings.ui -o formsettings.h \
	$(QTDIR)\bin\uic formsettings.ui -i formsettings.h -o formsettings.cpp \
	$(QTDIR)\bin\moc formsettings.h -o moc_formsettings.cpp \
	

"formsettings.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"formsettings.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_formsettings.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=gotolinedialog.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__GOTOL="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing gotolinedialog.ui...
InputPath=gotolinedialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic gotolinedialog.ui -o gotolinedialog.h \
	$(QTDIR)\bin\uic gotolinedialog.ui -i gotolinedialog.h -o gotolinedialog.cpp \
	$(QTDIR)\bin\moc gotolinedialog.h -o moc_gotolinedialog.cpp \
	

"gotolinedialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"gotolinedialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_gotolinedialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__GOTOL="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing gotolinedialog.ui...
InputPath=gotolinedialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic gotolinedialog.ui -o gotolinedialog.h \
	$(QTDIR)\bin\uic gotolinedialog.ui -i gotolinedialog.h -o gotolinedialog.cpp \
	$(QTDIR)\bin\moc gotolinedialog.h -o moc_gotolinedialog.cpp \
	

"gotolinedialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"gotolinedialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_gotolinedialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=iconvieweditor.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__ICONVI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing iconvieweditor.ui...
InputPath=iconvieweditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic iconvieweditor.ui -o iconvieweditor.h \
	$(QTDIR)\bin\uic iconvieweditor.ui -i iconvieweditor.h -o iconvieweditor.cpp \
	$(QTDIR)\bin\moc iconvieweditor.h -o moc_iconvieweditor.cpp \
	

"iconvieweditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"iconvieweditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_iconvieweditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__ICONVI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing iconvieweditor.ui...
InputPath=iconvieweditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic iconvieweditor.ui -o iconvieweditor.h \
	$(QTDIR)\bin\uic iconvieweditor.ui -i iconvieweditor.h -o iconvieweditor.cpp \
	$(QTDIR)\bin\moc iconvieweditor.h -o moc_iconvieweditor.cpp \
	

"iconvieweditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"iconvieweditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_iconvieweditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=listboxeditor.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__LISTBOXE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing listboxeditor.ui...
InputPath=listboxeditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic listboxeditor.ui -o listboxeditor.h \
	$(QTDIR)\bin\uic listboxeditor.ui -i listboxeditor.h -o listboxeditor.cpp \
	$(QTDIR)\bin\moc listboxeditor.h -o moc_listboxeditor.cpp \
	

"listboxeditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"listboxeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_listboxeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__LISTBOXE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing listboxeditor.ui...
InputPath=listboxeditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic listboxeditor.ui -o listboxeditor.h \
	$(QTDIR)\bin\uic listboxeditor.ui -i listboxeditor.h -o listboxeditor.cpp \
	$(QTDIR)\bin\moc listboxeditor.h -o moc_listboxeditor.cpp \
	

"listboxeditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"listboxeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_listboxeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=listeditor.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__LISTE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing listeditor.ui...
InputPath=listeditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic listeditor.ui -o listeditor.h \
	$(QTDIR)\bin\uic listeditor.ui -i listeditor.h -o listeditor.cpp \
	$(QTDIR)\bin\moc listeditor.h -o moc_listeditor.cpp \
	

"listeditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"listeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_listeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__LISTE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing listeditor.ui...
InputPath=listeditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic listeditor.ui -o listeditor.h \
	$(QTDIR)\bin\uic listeditor.ui -i listeditor.h -o listeditor.cpp \
	$(QTDIR)\bin\moc listeditor.h -o moc_listeditor.cpp \
	

"listeditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"listeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_listeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=listvieweditor.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__LISTVIE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing listvieweditor.ui...
InputPath=listvieweditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic listvieweditor.ui -o listvieweditor.h \
	$(QTDIR)\bin\uic listvieweditor.ui -i listvieweditor.h -o listvieweditor.cpp \
	$(QTDIR)\bin\moc listvieweditor.h -o moc_listvieweditor.cpp \
	

"listvieweditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"listvieweditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_listvieweditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__LISTVIE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing listvieweditor.ui...
InputPath=listvieweditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic listvieweditor.ui -o listvieweditor.h \
	$(QTDIR)\bin\uic listvieweditor.ui -i listvieweditor.h -o listvieweditor.cpp \
	$(QTDIR)\bin\moc listvieweditor.h -o moc_listvieweditor.cpp \
	

"listvieweditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"listvieweditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_listvieweditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=multilineeditor.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__MULTIL="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing multilineeditor.ui...
InputPath=multilineeditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic multilineeditor.ui -o multilineeditor.h \
	$(QTDIR)\bin\uic multilineeditor.ui -i multilineeditor.h -o multilineeditor.cpp \
	$(QTDIR)\bin\moc multilineeditor.h -o moc_multilineeditor.cpp \
	

"multilineeditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"multilineeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_multilineeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__MULTIL="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing multilineeditor.ui...
InputPath=multilineeditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic multilineeditor.ui -o multilineeditor.h \
	$(QTDIR)\bin\uic multilineeditor.ui -i multilineeditor.h -o multilineeditor.cpp \
	$(QTDIR)\bin\moc multilineeditor.h -o moc_multilineeditor.cpp \
	

"multilineeditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"multilineeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_multilineeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=newform.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__NEWFOR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing newform.ui...
InputPath=newform.ui

BuildCmds= \
	$(QTDIR)\bin\uic newform.ui -o newform.h \
	$(QTDIR)\bin\uic newform.ui -i newform.h -o newform.cpp \
	$(QTDIR)\bin\moc newform.h -o moc_newform.cpp \
	

"newform.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"newform.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_newform.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__NEWFOR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing newform.ui...
InputPath=newform.ui

BuildCmds= \
	$(QTDIR)\bin\uic newform.ui -o newform.h \
	$(QTDIR)\bin\uic newform.ui -i newform.h -o newform.cpp \
	$(QTDIR)\bin\moc newform.h -o moc_newform.cpp \
	

"newform.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"newform.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_newform.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=paletteeditor.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PALETTE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing paletteeditor.ui...
InputPath=paletteeditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic paletteeditor.ui -o paletteeditor.h \
	$(QTDIR)\bin\uic paletteeditor.ui -i paletteeditor.h -o paletteeditor.cpp \
	$(QTDIR)\bin\moc paletteeditor.h -o moc_paletteeditor.cpp \
	

"paletteeditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"paletteeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_paletteeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PALETTE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing paletteeditor.ui...
InputPath=paletteeditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic paletteeditor.ui -o paletteeditor.h \
	$(QTDIR)\bin\uic paletteeditor.ui -i paletteeditor.h -o paletteeditor.cpp \
	$(QTDIR)\bin\moc paletteeditor.h -o moc_paletteeditor.cpp \
	

"paletteeditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"paletteeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_paletteeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=paletteeditoradvanced.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PALETTEE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing paletteeditoradvanced.ui...
InputPath=paletteeditoradvanced.ui

BuildCmds= \
	$(QTDIR)\bin\uic paletteeditoradvanced.ui -o paletteeditoradvanced.h \
	$(QTDIR)\bin\uic paletteeditoradvanced.ui -i paletteeditoradvanced.h -o paletteeditoradvanced.cpp \
	$(QTDIR)\bin\moc paletteeditoradvanced.h -o moc_paletteeditoradvanced.cpp \
	

"paletteeditoradvanced.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"paletteeditoradvanced.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_paletteeditoradvanced.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PALETTEE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing paletteeditoradvanced.ui...
InputPath=paletteeditoradvanced.ui

BuildCmds= \
	$(QTDIR)\bin\uic paletteeditoradvanced.ui -o paletteeditoradvanced.h \
	$(QTDIR)\bin\uic paletteeditoradvanced.ui -i paletteeditoradvanced.h -o paletteeditoradvanced.cpp \
	$(QTDIR)\bin\moc paletteeditoradvanced.h -o moc_paletteeditoradvanced.cpp \
	

"paletteeditoradvanced.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"paletteeditoradvanced.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_paletteeditoradvanced.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=pixmapcollectioneditor.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PIXMAP="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing pixmapcollectioneditor.ui...
InputPath=pixmapcollectioneditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic pixmapcollectioneditor.ui -o pixmapcollectioneditor.h \
	$(QTDIR)\bin\uic pixmapcollectioneditor.ui -i pixmapcollectioneditor.h -o pixmapcollectioneditor.cpp \
	$(QTDIR)\bin\moc pixmapcollectioneditor.h -o moc_pixmapcollectioneditor.cpp \
	

"pixmapcollectioneditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pixmapcollectioneditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_pixmapcollectioneditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PIXMAP="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing pixmapcollectioneditor.ui...
InputPath=pixmapcollectioneditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic pixmapcollectioneditor.ui -o pixmapcollectioneditor.h \
	$(QTDIR)\bin\uic pixmapcollectioneditor.ui -i pixmapcollectioneditor.h -o pixmapcollectioneditor.cpp \
	$(QTDIR)\bin\moc pixmapcollectioneditor.h -o moc_pixmapcollectioneditor.cpp \
	

"pixmapcollectioneditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pixmapcollectioneditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_pixmapcollectioneditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=pixmapfunction.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PIXMAPF="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing pixmapfunction.ui...
InputPath=pixmapfunction.ui

BuildCmds= \
	$(QTDIR)\bin\uic pixmapfunction.ui -o pixmapfunction.h \
	$(QTDIR)\bin\uic pixmapfunction.ui -i pixmapfunction.h -o pixmapfunction.cpp \
	$(QTDIR)\bin\moc pixmapfunction.h -o moc_pixmapfunction.cpp \
	

"pixmapfunction.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pixmapfunction.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_pixmapfunction.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PIXMAPF="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing pixmapfunction.ui...
InputPath=pixmapfunction.ui

BuildCmds= \
	$(QTDIR)\bin\uic pixmapfunction.ui -o pixmapfunction.h \
	$(QTDIR)\bin\uic pixmapfunction.ui -i pixmapfunction.h -o pixmapfunction.cpp \
	$(QTDIR)\bin\moc pixmapfunction.h -o moc_pixmapfunction.cpp \
	

"pixmapfunction.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pixmapfunction.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_pixmapfunction.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=preferences.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PREFE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing preferences.ui...
InputPath=preferences.ui

BuildCmds= \
	$(QTDIR)\bin\uic preferences.ui -o preferences.h \
	$(QTDIR)\bin\uic preferences.ui -i preferences.h -o preferences.cpp \
	$(QTDIR)\bin\moc preferences.h -o moc_preferences.cpp \
	

"preferences.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"preferences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_preferences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PREFE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing preferences.ui...
InputPath=preferences.ui

BuildCmds= \
	$(QTDIR)\bin\uic preferences.ui -o preferences.h \
	$(QTDIR)\bin\uic preferences.ui -i preferences.h -o preferences.cpp \
	$(QTDIR)\bin\moc preferences.h -o moc_preferences.cpp \
	

"preferences.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"preferences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_preferences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=previewwidget.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PREVIEW="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing previewwidget.ui...
InputPath=previewwidget.ui

BuildCmds= \
	$(QTDIR)\bin\uic previewwidget.ui -o previewwidget.h \
	$(QTDIR)\bin\uic previewwidget.ui -i previewwidget.h -o previewwidget.cpp \
	$(QTDIR)\bin\moc previewwidget.h -o moc_previewwidget.cpp \
	

"previewwidget.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"previewwidget.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_previewwidget.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PREVIEW="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing previewwidget.ui...
InputPath=previewwidget.ui

BuildCmds= \
	$(QTDIR)\bin\uic previewwidget.ui -o previewwidget.h \
	$(QTDIR)\bin\uic previewwidget.ui -i previewwidget.h -o previewwidget.cpp \
	$(QTDIR)\bin\moc previewwidget.h -o moc_previewwidget.cpp \
	

"previewwidget.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"previewwidget.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_previewwidget.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=projectsettings.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__PROJECT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing projectsettings.ui...
InputPath=projectsettings.ui

BuildCmds= \
	$(QTDIR)\bin\uic projectsettings.ui -o projectsettings.h \
	$(QTDIR)\bin\uic projectsettings.ui -i projectsettings.h -o projectsettings.cpp \
	$(QTDIR)\bin\moc projectsettings.h -o moc_projectsettings.cpp \
	

"projectsettings.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"projectsettings.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_projectsettings.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__PROJECT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing projectsettings.ui...
InputPath=projectsettings.ui

BuildCmds= \
	$(QTDIR)\bin\uic projectsettings.ui -o projectsettings.h \
	$(QTDIR)\bin\uic projectsettings.ui -i projectsettings.h -o projectsettings.cpp \
	$(QTDIR)\bin\moc projectsettings.h -o moc_projectsettings.cpp \
	

"projectsettings.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"projectsettings.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_projectsettings.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=replacedialog.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__REPLA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing replacedialog.ui...
InputPath=replacedialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic replacedialog.ui -o replacedialog.h \
	$(QTDIR)\bin\uic replacedialog.ui -i replacedialog.h -o replacedialog.cpp \
	$(QTDIR)\bin\moc replacedialog.h -o moc_replacedialog.cpp \
	

"replacedialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"replacedialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_replacedialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__REPLA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing replacedialog.ui...
InputPath=replacedialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic replacedialog.ui -o replacedialog.h \
	$(QTDIR)\bin\uic replacedialog.ui -i replacedialog.h -o replacedialog.cpp \
	$(QTDIR)\bin\moc replacedialog.h -o moc_replacedialog.cpp \
	

"replacedialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"replacedialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_replacedialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=richtextfontdialog.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__RICHT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing richtextfontdialog.ui...
InputPath=richtextfontdialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic richtextfontdialog.ui -o richtextfontdialog.h \
	$(QTDIR)\bin\uic richtextfontdialog.ui -i richtextfontdialog.h -o richtextfontdialog.cpp \
	$(QTDIR)\bin\moc richtextfontdialog.h -o moc_richtextfontdialog.cpp \
	

"richtextfontdialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"richtextfontdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_richtextfontdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__RICHT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing richtextfontdialog.ui...
InputPath=richtextfontdialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic richtextfontdialog.ui -o richtextfontdialog.h \
	$(QTDIR)\bin\uic richtextfontdialog.ui -i richtextfontdialog.h -o richtextfontdialog.cpp \
	$(QTDIR)\bin\moc richtextfontdialog.h -o moc_richtextfontdialog.cpp \
	

"richtextfontdialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"richtextfontdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_richtextfontdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=startdialog.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__STARTD="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing startdialog.ui...
InputPath=startdialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic startdialog.ui -o startdialog.h \
	$(QTDIR)\bin\uic startdialog.ui -i startdialog.h -o startdialog.cpp \
	$(QTDIR)\bin\moc startdialog.h -o moc_startdialog.cpp \
	

"startdialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"startdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_startdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__STARTD="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing startdialog.ui...
InputPath=startdialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic startdialog.ui -o startdialog.h \
	$(QTDIR)\bin\uic startdialog.ui -i startdialog.h -o startdialog.cpp \
	$(QTDIR)\bin\moc startdialog.h -o moc_startdialog.cpp \
	

"startdialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"startdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_startdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=variabledialog.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__VARIAB="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing variabledialog.ui...
InputPath=variabledialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic variabledialog.ui -o variabledialog.h \
	$(QTDIR)\bin\uic variabledialog.ui -i variabledialog.h -o variabledialog.cpp \
	$(QTDIR)\bin\moc variabledialog.h -o moc_variabledialog.cpp \
	

"variabledialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"variabledialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_variabledialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__VARIAB="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing variabledialog.ui...
InputPath=variabledialog.ui

BuildCmds= \
	$(QTDIR)\bin\uic variabledialog.ui -o variabledialog.h \
	$(QTDIR)\bin\uic variabledialog.ui -i variabledialog.h -o variabledialog.cpp \
	$(QTDIR)\bin\moc variabledialog.h -o moc_variabledialog.cpp \
	

"variabledialog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"variabledialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_variabledialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=wizardeditor.ui

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__WIZARD="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing wizardeditor.ui...
InputPath=wizardeditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic wizardeditor.ui -o wizardeditor.h \
	$(QTDIR)\bin\uic wizardeditor.ui -i wizardeditor.h -o wizardeditor.cpp \
	$(QTDIR)\bin\moc wizardeditor.h -o moc_wizardeditor.cpp \
	

"wizardeditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"wizardeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_wizardeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__WIZARD="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing wizardeditor.ui...
InputPath=wizardeditor.ui

BuildCmds= \
	$(QTDIR)\bin\uic wizardeditor.ui -o wizardeditor.h \
	$(QTDIR)\bin\uic wizardeditor.ui -i wizardeditor.h -o wizardeditor.cpp \
	$(QTDIR)\bin\moc wizardeditor.h -o moc_wizardeditor.cpp \
	

"wizardeditor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"wizardeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_wizardeditor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Translations"

# PROP Default_Filter "ts"
# Begin Source File

SOURCE=designer_de.ts
# End Source File
# Begin Source File

SOURCE=designer_fr.ts
# End Source File
# End Group
# Begin Group "Images"

# PROP Default_Filter "png jpeg bmp xpm"
# Begin Source File

SOURCE=images/adjustsize.png

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__ADJUS="images/adjustsize.png"	"images/edithlayoutsplit.png"	"images/left.png"	"images/sizeall.png"	"images/arrow.png"	"images/editlower.png"	"images/line.png"	"images/sizeb.png"	"images/background.png"	"images/editpaste.png"	"images/lineedit.png"	"images/sizef.png"	"images/book.png"	"images/editraise.png"	"images/listbox.png"	"images/sizeh.png"	"images/buttongroup.png"	"images/editslots.png"	"images/listview.png"	"images/sizev.png"	"images/checkbox.png"	"images/editvlayout.png"	"images/multilineedit.png"	"images/slider.png"	"images/combobox.png"	"images/editvlayoutsplit.png"	"images/newform.png"	"images/spacer.png"	"images/connecttool.png"	"images/filenew.png"	"images/no.png"	"images/spinbox.png"	"images/cross.png"	"images/fileopen.png"	"images/ordertool.png"	"images/splash.png"	"images/customwidget.png"	"images/filesave.png"	"images/pixlabel.png"	"images/table.png"	"images/databrowser.png"	"images/form.png"	"images/pointer.png"	"images/tabwidget.png"	"images/datatable.png"	"images/frame.png"	"images/print.png"	"images/textbrowser.png"	"images/dataview.png"	"images/groupbox.png"	"images/progress.png"	"images/textedit.png"	"images/dateedit.png"	"images/hand.png"\
	"images/project.png"	"images/textview.png"	"images/datetimeedit.png"	"images/help.png"	"images/pushbutton.png"	"images/timeedit.png"	"images/dial.png"	"images/home.png"	"images/qtlogo.png"	"images/toolbutton.png"	"images/down.png"	"images/hsplit.png"	"images/radiobutton.png"	"images/undo.png"	"images/editbreaklayout.png"	"images/ibeam.png"	"images/redo.png"	"images/up.png"	"images/resetproperty.png"	"images/editcopy.png"	"images/iconview.png"	"images/resetproperty.png"	"images/uparrow.png"	"images/editcut.png"	"images/image.png"	"images/richtextedit.png"	"images/vsplit.png"	"images/editdelete.png"	"images/label.png"	"images/right.png"	"images/wait.png"	"images/editgrid.png"	"images/layout.png"	"images/scrollbar.png"	"images/widgetstack.png"	"images/edithlayout.png"	"images/lcdnumber.png"	"images/searchfind.png"	"images/folder.png"	"images/setbuddy.png"	"images/textbold.png"	"images/textcenter.png"	"images/texth1.png"	"images/texth2.png"	"images/texth3.png"	"images/textitalic.png"	"images/textjustify.png"	"images/textlarger.png"	"images/textleft.png"	"images/textlinebreak.png"	"images/textparagraph.png"	"images/textright.png"	"images/textsmaller.png"	"images/textteletext.png"\
	"images/textunderline.png"	"images/wizarddata.png"	"images/wizarddialog.png"	"images/d_adjustsize.png"	"images/d_label.png"	"images/d_book.png"	"images/d_layout.png"	"images/d_buttongroup.png"	"images/d_lcdnumber.png"	"images/d_checkbox.png"	"images/d_left.png"	"images/d_combobox.png"	"images/d_line.png"	"images/d_connecttool.png"	"images/d_lineedit.png"	"images/d_customwidget.png"	"images/d_listbox.png"	"images/d_databrowser.png"	"images/d_listview.png"	"images/d_datatable.png"	"images/d_multilineedit.png"	"images/d_dataview.png"	"images/d_newform.png"	"images/d_dateedit.png"	"images/d_ordertool.png"	"images/d_datetimeedit.png"	"images/d_pixlabel.png"	"images/d_dial.png"	"images/d_pointer.png"	"images/d_down.png"	"images/d_print.png"	"images/d_editbreaklayout.png"	"images/d_progress.png"	"images/d_editcopy.png"	"images/d_project.png"	"images/d_editcut.png"	"images/d_pushbutton.png"	"images/d_editdelete.png"	"images/d_radiobutton.png"	"images/d_editgrid.png"	"images/d_redo.png"	"images/d_edithlayout.png"	"images/d_richtextedit.png"	"images/d_edithlayoutsplit.png"	"images/d_right.png"	"images/d_editlower.png"	"images/d_scrollbar.png"	"images/d_editpaste.png"\
	"images/d_searchfind.png"	"images/d_editraise.png"	"images/d_slider.png"	"images/d_editslots.png"	"images/d_spacer.png"	"images/d_editvlayout.png"	"images/d_spinbox.png"	"images/d_editvlayoutsplit.png"	"images/d_table.png"	"images/d_filenew.png"	"images/d_folder.png"	"images/d_tabwidget.png"	"images/d_fileopen.png"	"images/d_textbrowser.png"	"images/d_filesave.png"	"images/d_textedit.png"	"images/d_form.png"	"images/d_textview.png"	"images/d_frame.png"	"images/d_timeedit.png"	"images/d_groupbox.png"	"images/d_toolbutton.png"	"images/d_help.png"	"images/d_undo.png"	"images/d_home.png"	"images/d_up.png"	"images/d_iconview.png"	"images/d_widgetstack.png"	"images/d_setbuddy.png"	"images/d_textbold.png"	"images/d_texth1.png"	"images/d_texth2.png"	"images/d_texth3.png"	"images/d_textitalic.png"	"images/d_textjustify.png"	"images/d_textlarger.png"	"images/d_textleft.png"	"images/d_textlinebreak.png"	"images/d_textparagraph.png"	"images/d_textright.png"	"images/d_textsmaller.png"	"images/d_textteletext.png"	"images/d_textunderline.png"	"images/d_textcenter.png"	"images/d_wizarddata.png"	"images/d_wizarddialog.png"	"images/s_editcut.png"	"images/s_up.png"	"images/s_down.png"\
	"images/s_left.png"	"images/s_right.png"	"images/d_image.png"	"images/d_textfont.png"	"images/textfont.png"	"images/object.png"	"images/appicon.png"	
# Begin Custom Build - Creating image collection...
InputPath=images/adjustsize.png

"qmake_image_collection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\uic -embed designerlib -f images.tmp -o qmake_image_collection.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__ADJUS="images/adjustsize.png"	"images/edithlayoutsplit.png"	"images/left.png"	"images/sizeall.png"	"images/arrow.png"	"images/editlower.png"	"images/line.png"	"images/sizeb.png"	"images/background.png"	"images/editpaste.png"	"images/lineedit.png"	"images/sizef.png"	"images/book.png"	"images/editraise.png"	"images/listbox.png"	"images/sizeh.png"	"images/buttongroup.png"	"images/editslots.png"	"images/listview.png"	"images/sizev.png"	"images/checkbox.png"	"images/editvlayout.png"	"images/multilineedit.png"	"images/slider.png"	"images/combobox.png"	"images/editvlayoutsplit.png"	"images/newform.png"	"images/spacer.png"	"images/connecttool.png"	"images/filenew.png"	"images/no.png"	"images/spinbox.png"	"images/cross.png"	"images/fileopen.png"	"images/ordertool.png"	"images/splash.png"	"images/customwidget.png"	"images/filesave.png"	"images/pixlabel.png"	"images/table.png"	"images/databrowser.png"	"images/form.png"	"images/pointer.png"	"images/tabwidget.png"	"images/datatable.png"	"images/frame.png"	"images/print.png"	"images/textbrowser.png"	"images/dataview.png"	"images/groupbox.png"	"images/progress.png"	"images/textedit.png"	"images/dateedit.png"	"images/hand.png"\
	"images/project.png"	"images/textview.png"	"images/datetimeedit.png"	"images/help.png"	"images/pushbutton.png"	"images/timeedit.png"	"images/dial.png"	"images/home.png"	"images/qtlogo.png"	"images/toolbutton.png"	"images/down.png"	"images/hsplit.png"	"images/radiobutton.png"	"images/undo.png"	"images/editbreaklayout.png"	"images/ibeam.png"	"images/redo.png"	"images/up.png"	"images/resetproperty.png"	"images/editcopy.png"	"images/iconview.png"	"images/resetproperty.png"	"images/uparrow.png"	"images/editcut.png"	"images/image.png"	"images/richtextedit.png"	"images/vsplit.png"	"images/editdelete.png"	"images/label.png"	"images/right.png"	"images/wait.png"	"images/editgrid.png"	"images/layout.png"	"images/scrollbar.png"	"images/widgetstack.png"	"images/edithlayout.png"	"images/lcdnumber.png"	"images/searchfind.png"	"images/folder.png"	"images/setbuddy.png"	"images/textbold.png"	"images/textcenter.png"	"images/texth1.png"	"images/texth2.png"	"images/texth3.png"	"images/textitalic.png"	"images/textjustify.png"	"images/textlarger.png"	"images/textleft.png"	"images/textlinebreak.png"	"images/textparagraph.png"	"images/textright.png"	"images/textsmaller.png"	"images/textteletext.png"\
	"images/textunderline.png"	"images/wizarddata.png"	"images/wizarddialog.png"	"images/d_adjustsize.png"	"images/d_label.png"	"images/d_book.png"	"images/d_layout.png"	"images/d_buttongroup.png"	"images/d_lcdnumber.png"	"images/d_checkbox.png"	"images/d_left.png"	"images/d_combobox.png"	"images/d_line.png"	"images/d_connecttool.png"	"images/d_lineedit.png"	"images/d_customwidget.png"	"images/d_listbox.png"	"images/d_databrowser.png"	"images/d_listview.png"	"images/d_datatable.png"	"images/d_multilineedit.png"	"images/d_dataview.png"	"images/d_newform.png"	"images/d_dateedit.png"	"images/d_ordertool.png"	"images/d_datetimeedit.png"	"images/d_pixlabel.png"	"images/d_dial.png"	"images/d_pointer.png"	"images/d_down.png"	"images/d_print.png"	"images/d_editbreaklayout.png"	"images/d_progress.png"	"images/d_editcopy.png"	"images/d_project.png"	"images/d_editcut.png"	"images/d_pushbutton.png"	"images/d_editdelete.png"	"images/d_radiobutton.png"	"images/d_editgrid.png"	"images/d_redo.png"	"images/d_edithlayout.png"	"images/d_richtextedit.png"	"images/d_edithlayoutsplit.png"	"images/d_right.png"	"images/d_editlower.png"	"images/d_scrollbar.png"	"images/d_editpaste.png"\
	"images/d_searchfind.png"	"images/d_editraise.png"	"images/d_slider.png"	"images/d_editslots.png"	"images/d_spacer.png"	"images/d_editvlayout.png"	"images/d_spinbox.png"	"images/d_editvlayoutsplit.png"	"images/d_table.png"	"images/d_filenew.png"	"images/d_folder.png"	"images/d_tabwidget.png"	"images/d_fileopen.png"	"images/d_textbrowser.png"	"images/d_filesave.png"	"images/d_textedit.png"	"images/d_form.png"	"images/d_textview.png"	"images/d_frame.png"	"images/d_timeedit.png"	"images/d_groupbox.png"	"images/d_toolbutton.png"	"images/d_help.png"	"images/d_undo.png"	"images/d_home.png"	"images/d_up.png"	"images/d_iconview.png"	"images/d_widgetstack.png"	"images/d_setbuddy.png"	"images/d_textbold.png"	"images/d_texth1.png"	"images/d_texth2.png"	"images/d_texth3.png"	"images/d_textitalic.png"	"images/d_textjustify.png"	"images/d_textlarger.png"	"images/d_textleft.png"	"images/d_textlinebreak.png"	"images/d_textparagraph.png"	"images/d_textright.png"	"images/d_textsmaller.png"	"images/d_textteletext.png"	"images/d_textunderline.png"	"images/d_textcenter.png"	"images/d_wizarddata.png"	"images/d_wizarddialog.png"	"images/s_editcut.png"	"images/s_up.png"	"images/s_down.png"\
	"images/s_left.png"	"images/s_right.png"	"images/d_image.png"	"images/d_textfont.png"	"images/textfont.png"	"images/object.png"	"images/appicon.png"	
# Begin Custom Build - Creating image collection...
InputPath=images/adjustsize.png

"qmake_image_collection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\uic -embed designerlib -f images.tmp -o qmake_image_collection.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=images/appicon.png
# End Source File
# Begin Source File

SOURCE=images/arrow.png
# End Source File
# Begin Source File

SOURCE=images/background.png
# End Source File
# Begin Source File

SOURCE=images/book.png
# End Source File
# Begin Source File

SOURCE=images/buttongroup.png
# End Source File
# Begin Source File

SOURCE=images/checkbox.png
# End Source File
# Begin Source File

SOURCE=images/combobox.png
# End Source File
# Begin Source File

SOURCE=images/connecttool.png
# End Source File
# Begin Source File

SOURCE=images/cross.png
# End Source File
# Begin Source File

SOURCE=images/customwidget.png
# End Source File
# Begin Source File

SOURCE=images/d_adjustsize.png
# End Source File
# Begin Source File

SOURCE=images/d_book.png
# End Source File
# Begin Source File

SOURCE=images/d_buttongroup.png
# End Source File
# Begin Source File

SOURCE=images/d_checkbox.png
# End Source File
# Begin Source File

SOURCE=images/d_combobox.png
# End Source File
# Begin Source File

SOURCE=images/d_connecttool.png
# End Source File
# Begin Source File

SOURCE=images/d_customwidget.png
# End Source File
# Begin Source File

SOURCE=images/d_databrowser.png
# End Source File
# Begin Source File

SOURCE=images/d_datatable.png
# End Source File
# Begin Source File

SOURCE=images/d_dataview.png
# End Source File
# Begin Source File

SOURCE=images/d_dateedit.png
# End Source File
# Begin Source File

SOURCE=images/d_datetimeedit.png
# End Source File
# Begin Source File

SOURCE=images/d_dial.png
# End Source File
# Begin Source File

SOURCE=images/d_down.png
# End Source File
# Begin Source File

SOURCE=images/d_editbreaklayout.png
# End Source File
# Begin Source File

SOURCE=images/d_editcopy.png
# End Source File
# Begin Source File

SOURCE=images/d_editcut.png
# End Source File
# Begin Source File

SOURCE=images/d_editdelete.png
# End Source File
# Begin Source File

SOURCE=images/d_editgrid.png
# End Source File
# Begin Source File

SOURCE=images/d_edithlayout.png
# End Source File
# Begin Source File

SOURCE=images/d_edithlayoutsplit.png
# End Source File
# Begin Source File

SOURCE=images/d_editlower.png
# End Source File
# Begin Source File

SOURCE=images/d_editpaste.png
# End Source File
# Begin Source File

SOURCE=images/d_editraise.png
# End Source File
# Begin Source File

SOURCE=images/d_editslots.png
# End Source File
# Begin Source File

SOURCE=images/d_editvlayout.png
# End Source File
# Begin Source File

SOURCE=images/d_editvlayoutsplit.png
# End Source File
# Begin Source File

SOURCE=images/d_filenew.png
# End Source File
# Begin Source File

SOURCE=images/d_fileopen.png
# End Source File
# Begin Source File

SOURCE=images/d_filesave.png
# End Source File
# Begin Source File

SOURCE=images/d_folder.png
# End Source File
# Begin Source File

SOURCE=images/d_form.png
# End Source File
# Begin Source File

SOURCE=images/d_frame.png
# End Source File
# Begin Source File

SOURCE=images/d_groupbox.png
# End Source File
# Begin Source File

SOURCE=images/d_help.png
# End Source File
# Begin Source File

SOURCE=images/d_home.png
# End Source File
# Begin Source File

SOURCE=images/d_iconview.png
# End Source File
# Begin Source File

SOURCE=images/d_image.png
# End Source File
# Begin Source File

SOURCE=images/d_label.png
# End Source File
# Begin Source File

SOURCE=images/d_layout.png
# End Source File
# Begin Source File

SOURCE=images/d_lcdnumber.png
# End Source File
# Begin Source File

SOURCE=images/d_left.png
# End Source File
# Begin Source File

SOURCE=images/d_line.png
# End Source File
# Begin Source File

SOURCE=images/d_lineedit.png
# End Source File
# Begin Source File

SOURCE=images/d_listbox.png
# End Source File
# Begin Source File

SOURCE=images/d_listview.png
# End Source File
# Begin Source File

SOURCE=images/d_multilineedit.png
# End Source File
# Begin Source File

SOURCE=images/d_newform.png
# End Source File
# Begin Source File

SOURCE=images/d_ordertool.png
# End Source File
# Begin Source File

SOURCE=images/d_pixlabel.png
# End Source File
# Begin Source File

SOURCE=images/d_pointer.png
# End Source File
# Begin Source File

SOURCE=images/d_print.png
# End Source File
# Begin Source File

SOURCE=images/d_progress.png
# End Source File
# Begin Source File

SOURCE=images/d_project.png
# End Source File
# Begin Source File

SOURCE=images/d_pushbutton.png
# End Source File
# Begin Source File

SOURCE=images/d_radiobutton.png
# End Source File
# Begin Source File

SOURCE=images/d_redo.png
# End Source File
# Begin Source File

SOURCE=images/d_richtextedit.png
# End Source File
# Begin Source File

SOURCE=images/d_right.png
# End Source File
# Begin Source File

SOURCE=images/d_scrollbar.png
# End Source File
# Begin Source File

SOURCE=images/d_searchfind.png
# End Source File
# Begin Source File

SOURCE=images/d_setbuddy.png
# End Source File
# Begin Source File

SOURCE=images/d_slider.png
# End Source File
# Begin Source File

SOURCE=images/d_spacer.png
# End Source File
# Begin Source File

SOURCE=images/d_spinbox.png
# End Source File
# Begin Source File

SOURCE=images/d_table.png
# End Source File
# Begin Source File

SOURCE=images/d_tabwidget.png
# End Source File
# Begin Source File

SOURCE=images/d_textbold.png
# End Source File
# Begin Source File

SOURCE=images/d_textbrowser.png
# End Source File
# Begin Source File

SOURCE=images/d_textcenter.png
# End Source File
# Begin Source File

SOURCE=images/d_textedit.png
# End Source File
# Begin Source File

SOURCE=images/d_textfont.png
# End Source File
# Begin Source File

SOURCE=images/d_texth1.png
# End Source File
# Begin Source File

SOURCE=images/d_texth2.png
# End Source File
# Begin Source File

SOURCE=images/d_texth3.png
# End Source File
# Begin Source File

SOURCE=images/d_textitalic.png
# End Source File
# Begin Source File

SOURCE=images/d_textjustify.png
# End Source File
# Begin Source File

SOURCE=images/d_textlarger.png
# End Source File
# Begin Source File

SOURCE=images/d_textleft.png
# End Source File
# Begin Source File

SOURCE=images/d_textlinebreak.png
# End Source File
# Begin Source File

SOURCE=images/d_textparagraph.png
# End Source File
# Begin Source File

SOURCE=images/d_textright.png
# End Source File
# Begin Source File

SOURCE=images/d_textsmaller.png
# End Source File
# Begin Source File

SOURCE=images/d_textteletext.png
# End Source File
# Begin Source File

SOURCE=images/d_textunderline.png
# End Source File
# Begin Source File

SOURCE=images/d_textview.png
# End Source File
# Begin Source File

SOURCE=images/d_timeedit.png
# End Source File
# Begin Source File

SOURCE=images/d_toolbutton.png
# End Source File
# Begin Source File

SOURCE=images/d_undo.png
# End Source File
# Begin Source File

SOURCE=images/d_up.png
# End Source File
# Begin Source File

SOURCE=images/d_widgetstack.png
# End Source File
# Begin Source File

SOURCE=images/d_wizarddata.png
# End Source File
# Begin Source File

SOURCE=images/d_wizarddialog.png
# End Source File
# Begin Source File

SOURCE=images/databrowser.png
# End Source File
# Begin Source File

SOURCE=images/datatable.png
# End Source File
# Begin Source File

SOURCE=images/dataview.png
# End Source File
# Begin Source File

SOURCE=images/dateedit.png
# End Source File
# Begin Source File

SOURCE=images/datetimeedit.png
# End Source File
# Begin Source File

SOURCE=images/dial.png
# End Source File
# Begin Source File

SOURCE=images/down.png
# End Source File
# Begin Source File

SOURCE=images/editbreaklayout.png
# End Source File
# Begin Source File

SOURCE=images/editcopy.png
# End Source File
# Begin Source File

SOURCE=images/editcut.png
# End Source File
# Begin Source File

SOURCE=images/editdelete.png
# End Source File
# Begin Source File

SOURCE=images/editgrid.png
# End Source File
# Begin Source File

SOURCE=images/edithlayout.png
# End Source File
# Begin Source File

SOURCE=images/edithlayoutsplit.png
# End Source File
# Begin Source File

SOURCE=images/editlower.png
# End Source File
# Begin Source File

SOURCE=images/editpaste.png
# End Source File
# Begin Source File

SOURCE=images/editraise.png
# End Source File
# Begin Source File

SOURCE=images/editslots.png
# End Source File
# Begin Source File

SOURCE=images/editvlayout.png
# End Source File
# Begin Source File

SOURCE=images/editvlayoutsplit.png
# End Source File
# Begin Source File

SOURCE=images/filenew.png
# End Source File
# Begin Source File

SOURCE=images/fileopen.png
# End Source File
# Begin Source File

SOURCE=images/filesave.png
# End Source File
# Begin Source File

SOURCE=images/folder.png
# End Source File
# Begin Source File

SOURCE=images/form.png
# End Source File
# Begin Source File

SOURCE=images/frame.png
# End Source File
# Begin Source File

SOURCE=images/groupbox.png
# End Source File
# Begin Source File

SOURCE=images/hand.png
# End Source File
# Begin Source File

SOURCE=images/help.png
# End Source File
# Begin Source File

SOURCE=images/home.png
# End Source File
# Begin Source File

SOURCE=images/hsplit.png
# End Source File
# Begin Source File

SOURCE=images/ibeam.png
# End Source File
# Begin Source File

SOURCE=images/iconview.png
# End Source File
# Begin Source File

SOURCE=images/image.png
# End Source File
# Begin Source File

SOURCE=images/label.png
# End Source File
# Begin Source File

SOURCE=images/layout.png
# End Source File
# Begin Source File

SOURCE=images/lcdnumber.png
# End Source File
# Begin Source File

SOURCE=images/left.png
# End Source File
# Begin Source File

SOURCE=images/line.png
# End Source File
# Begin Source File

SOURCE=images/lineedit.png
# End Source File
# Begin Source File

SOURCE=images/listbox.png
# End Source File
# Begin Source File

SOURCE=images/listview.png
# End Source File
# Begin Source File

SOURCE=images/multilineedit.png
# End Source File
# Begin Source File

SOURCE=images/newform.png
# End Source File
# Begin Source File

SOURCE=images/no.png
# End Source File
# Begin Source File

SOURCE=images/object.png
# End Source File
# Begin Source File

SOURCE=images/ordertool.png
# End Source File
# Begin Source File

SOURCE=images/pixlabel.png
# End Source File
# Begin Source File

SOURCE=images/pointer.png
# End Source File
# Begin Source File

SOURCE=images/print.png
# End Source File
# Begin Source File

SOURCE=images/progress.png
# End Source File
# Begin Source File

SOURCE=images/project.png
# End Source File
# Begin Source File

SOURCE=images/pushbutton.png
# End Source File
# Begin Source File

SOURCE=images/qtlogo.png
# End Source File
# Begin Source File

SOURCE=images/radiobutton.png
# End Source File
# Begin Source File

SOURCE=images/redo.png
# End Source File
# Begin Source File

SOURCE=images/resetproperty.png
# End Source File
# Begin Source File

SOURCE=images/resetproperty.png
# End Source File
# Begin Source File

SOURCE=images/richtextedit.png
# End Source File
# Begin Source File

SOURCE=images/right.png
# End Source File
# Begin Source File

SOURCE=images/s_down.png
# End Source File
# Begin Source File

SOURCE=images/s_editcut.png
# End Source File
# Begin Source File

SOURCE=images/s_left.png
# End Source File
# Begin Source File

SOURCE=images/s_right.png
# End Source File
# Begin Source File

SOURCE=images/s_up.png
# End Source File
# Begin Source File

SOURCE=images/scrollbar.png
# End Source File
# Begin Source File

SOURCE=images/searchfind.png
# End Source File
# Begin Source File

SOURCE=images/setbuddy.png
# End Source File
# Begin Source File

SOURCE=images/sizeall.png
# End Source File
# Begin Source File

SOURCE=images/sizeb.png
# End Source File
# Begin Source File

SOURCE=images/sizef.png
# End Source File
# Begin Source File

SOURCE=images/sizeh.png
# End Source File
# Begin Source File

SOURCE=images/sizev.png
# End Source File
# Begin Source File

SOURCE=images/slider.png
# End Source File
# Begin Source File

SOURCE=images/spacer.png
# End Source File
# Begin Source File

SOURCE=images/spinbox.png
# End Source File
# Begin Source File

SOURCE=images/splash.png
# End Source File
# Begin Source File

SOURCE=images/table.png
# End Source File
# Begin Source File

SOURCE=images/tabwidget.png
# End Source File
# Begin Source File

SOURCE=images/textbold.png
# End Source File
# Begin Source File

SOURCE=images/textbrowser.png
# End Source File
# Begin Source File

SOURCE=images/textcenter.png
# End Source File
# Begin Source File

SOURCE=images/textedit.png
# End Source File
# Begin Source File

SOURCE=images/textfont.png
# End Source File
# Begin Source File

SOURCE=images/texth1.png
# End Source File
# Begin Source File

SOURCE=images/texth2.png
# End Source File
# Begin Source File

SOURCE=images/texth3.png
# End Source File
# Begin Source File

SOURCE=images/textitalic.png
# End Source File
# Begin Source File

SOURCE=images/textjustify.png
# End Source File
# Begin Source File

SOURCE=images/textlarger.png
# End Source File
# Begin Source File

SOURCE=images/textleft.png
# End Source File
# Begin Source File

SOURCE=images/textlinebreak.png
# End Source File
# Begin Source File

SOURCE=images/textparagraph.png
# End Source File
# Begin Source File

SOURCE=images/textright.png
# End Source File
# Begin Source File

SOURCE=images/textsmaller.png
# End Source File
# Begin Source File

SOURCE=images/textteletext.png
# End Source File
# Begin Source File

SOURCE=images/textunderline.png
# End Source File
# Begin Source File

SOURCE=images/textview.png
# End Source File
# Begin Source File

SOURCE=images/timeedit.png
# End Source File
# Begin Source File

SOURCE=images/toolbutton.png
# End Source File
# Begin Source File

SOURCE=images/undo.png
# End Source File
# Begin Source File

SOURCE=images/up.png
# End Source File
# Begin Source File

SOURCE=images/uparrow.png
# End Source File
# Begin Source File

SOURCE=images/vsplit.png
# End Source File
# Begin Source File

SOURCE=images/wait.png
# End Source File
# Begin Source File

SOURCE=images/widgetstack.png
# End Source File
# Begin Source File

SOURCE=images/wizarddata.png
# End Source File
# Begin Source File

SOURCE=images/wizarddialog.png
# End Source File
# End Group
# Begin Group "Generated"

# PROP Default_Filter ""
# Begin Source File

SOURCE=about.cpp
# End Source File
# Begin Source File

SOURCE=about.h
# End Source File
# Begin Source File

SOURCE=actiondnd.moc

!IF  "$(CFG)" == "designerlib - Win32 Release"

USERDEP__ACTIOND=".\actiondnd.cpp"	"$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing actiondnd.cpp...
InputPath=actiondnd.moc

"actiondnd.moc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc actiondnd.cpp -o actiondnd.moc

# End Custom Build

!ELSEIF  "$(CFG)" == "designerlib - Win32 Debug"

USERDEP__ACTIOND=".\actiondnd.cpp"	"$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing actiondnd.cpp...
InputPath=actiondnd.moc

"actiondnd.moc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc actiondnd.cpp -o actiondnd.moc

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=actioneditor.cpp
# End Source File
# Begin Source File

SOURCE=actioneditor.h
# End Source File
# Begin Source File

SOURCE=configtoolboxdialog.cpp
# End Source File
# Begin Source File

SOURCE=configtoolboxdialog.h
# End Source File
# Begin Source File

SOURCE=connectiondialog.cpp
# End Source File
# Begin Source File

SOURCE=connectiondialog.h
# End Source File
# Begin Source File

SOURCE=createtemplate.cpp
# End Source File
# Begin Source File

SOURCE=createtemplate.h
# End Source File
# Begin Source File

SOURCE=customwidgeteditor.cpp
# End Source File
# Begin Source File

SOURCE=customwidgeteditor.h
# End Source File
# Begin Source File

SOURCE=editfunctions.cpp
# End Source File
# Begin Source File

SOURCE=editfunctions.h
# End Source File
# Begin Source File

SOURCE=finddialog.cpp
# End Source File
# Begin Source File

SOURCE=finddialog.h
# End Source File
# Begin Source File

SOURCE=formsettings.cpp
# End Source File
# Begin Source File

SOURCE=formsettings.h
# End Source File
# Begin Source File

SOURCE=gotolinedialog.cpp
# End Source File
# Begin Source File

SOURCE=gotolinedialog.h
# End Source File
# Begin Source File

SOURCE=iconvieweditor.cpp
# End Source File
# Begin Source File

SOURCE=iconvieweditor.h
# End Source File
# Begin Source File

SOURCE=listboxeditor.cpp
# End Source File
# Begin Source File

SOURCE=listboxeditor.h
# End Source File
# Begin Source File

SOURCE=listeditor.cpp
# End Source File
# Begin Source File

SOURCE=listeditor.h
# End Source File
# Begin Source File

SOURCE=listvieweditor.cpp
# End Source File
# Begin Source File

SOURCE=listvieweditor.h
# End Source File
# Begin Source File

SOURCE=moc_about.cpp
# End Source File
# Begin Source File

SOURCE=moc_actiondnd.cpp
# End Source File
# Begin Source File

SOURCE=moc_actioneditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_actioneditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_actionlistview.cpp
# End Source File
# Begin Source File

SOURCE=moc_asciivalidator.cpp
# End Source File
# Begin Source File

SOURCE=moc_command.cpp
# End Source File
# Begin Source File

SOURCE=moc_configtoolboxdialog.cpp
# End Source File
# Begin Source File

SOURCE=moc_connectiondialog.cpp
# End Source File
# Begin Source File

SOURCE=moc_connectionitems.cpp
# End Source File
# Begin Source File

SOURCE=moc_connectiontable.cpp
# End Source File
# Begin Source File

SOURCE=moc_createtemplate.cpp
# End Source File
# Begin Source File

SOURCE=moc_customwidgeteditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_customwidgeteditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_editfunctions.cpp
# End Source File
# Begin Source File

SOURCE=moc_editfunctionsimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_filechooser.cpp
# End Source File
# Begin Source File

SOURCE=moc_finddialog.cpp
# End Source File
# Begin Source File

SOURCE=moc_formfile.cpp
# End Source File
# Begin Source File

SOURCE=moc_formsettings.cpp
# End Source File
# Begin Source File

SOURCE=moc_formsettingsimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_formwindow.cpp
# End Source File
# Begin Source File

SOURCE=moc_gotolinedialog.cpp
# End Source File
# Begin Source File

SOURCE=moc_hierarchyview.cpp
# End Source File
# Begin Source File

SOURCE=moc_iconvieweditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_iconvieweditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_layout.cpp
# End Source File
# Begin Source File

SOURCE=moc_listboxdnd.cpp
# End Source File
# Begin Source File

SOURCE=moc_listboxeditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_listboxeditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_listboxrename.cpp
# End Source File
# Begin Source File

SOURCE=moc_listdnd.cpp
# End Source File
# Begin Source File

SOURCE=moc_listeditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_listviewdnd.cpp
# End Source File
# Begin Source File

SOURCE=moc_listvieweditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_listvieweditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_mainwindow.cpp
# End Source File
# Begin Source File

SOURCE=moc_multilineeditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_multilineeditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_newform.cpp
# End Source File
# Begin Source File

SOURCE=moc_newformimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_orderindicator.cpp
# End Source File
# Begin Source File

SOURCE=moc_outputwindow.cpp
# End Source File
# Begin Source File

SOURCE=moc_paletteeditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_paletteeditoradvanced.cpp
# End Source File
# Begin Source File

SOURCE=moc_paletteeditoradvancedimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_paletteeditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_pixmapchooser.cpp
# End Source File
# Begin Source File

SOURCE=moc_pixmapcollectioneditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_pixmapfunction.cpp
# End Source File
# Begin Source File

SOURCE=moc_preferences.cpp
# End Source File
# Begin Source File

SOURCE=moc_previewframe.cpp
# End Source File
# Begin Source File

SOURCE=moc_previewwidget.cpp
# End Source File
# Begin Source File

SOURCE=moc_previewwidgetimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_project.cpp
# End Source File
# Begin Source File

SOURCE=moc_projectsettings.cpp
# End Source File
# Begin Source File

SOURCE=moc_projectsettingsimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_propertyeditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_qcategorywidget.cpp
# End Source File
# Begin Source File

SOURCE=moc_qcompletionedit.cpp
# End Source File
# Begin Source File

SOURCE=moc_replacedialog.cpp
# End Source File
# Begin Source File

SOURCE=moc_richtextfontdialog.cpp
# End Source File
# Begin Source File

SOURCE=moc_sizehandle.cpp
# End Source File
# Begin Source File

SOURCE=moc_sourceeditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_sourcefile.cpp
# End Source File
# Begin Source File

SOURCE=moc_startdialog.cpp
# End Source File
# Begin Source File

SOURCE=moc_startdialogimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_styledbutton.cpp
# End Source File
# Begin Source File

SOURCE=moc_timestamp.cpp
# End Source File
# Begin Source File

SOURCE=moc_variabledialog.cpp
# End Source File
# Begin Source File

SOURCE=moc_variabledialogimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_widgetaction.cpp
# End Source File
# Begin Source File

SOURCE=moc_widgetfactory.cpp
# End Source File
# Begin Source File

SOURCE=moc_wizardeditor.cpp
# End Source File
# Begin Source File

SOURCE=moc_wizardeditorimpl.cpp
# End Source File
# Begin Source File

SOURCE=moc_workspace.cpp
# End Source File
# Begin Source File

SOURCE=multilineeditor.cpp
# End Source File
# Begin Source File

SOURCE=multilineeditor.h
# End Source File
# Begin Source File

SOURCE=newform.cpp
# End Source File
# Begin Source File

SOURCE=newform.h
# End Source File
# Begin Source File

SOURCE=paletteeditor.cpp
# End Source File
# Begin Source File

SOURCE=paletteeditor.h
# End Source File
# Begin Source File

SOURCE=paletteeditoradvanced.cpp
# End Source File
# Begin Source File

SOURCE=paletteeditoradvanced.h
# End Source File
# Begin Source File

SOURCE=pixmapcollectioneditor.cpp
# End Source File
# Begin Source File

SOURCE=pixmapcollectioneditor.h
# End Source File
# Begin Source File

SOURCE=pixmapfunction.cpp
# End Source File
# Begin Source File

SOURCE=pixmapfunction.h
# End Source File
# Begin Source File

SOURCE=preferences.cpp
# End Source File
# Begin Source File

SOURCE=preferences.h
# End Source File
# Begin Source File

SOURCE=previewwidget.cpp
# End Source File
# Begin Source File

SOURCE=previewwidget.h
# End Source File
# Begin Source File

SOURCE=projectsettings.cpp
# End Source File
# Begin Source File

SOURCE=projectsettings.h
# End Source File
# Begin Source File

SOURCE=qmake_image_collection.cpp
# End Source File
# Begin Source File

SOURCE=replacedialog.cpp
# End Source File
# Begin Source File

SOURCE=replacedialog.h
# End Source File
# Begin Source File

SOURCE=richtextfontdialog.cpp
# End Source File
# Begin Source File

SOURCE=richtextfontdialog.h
# End Source File
# Begin Source File

SOURCE=startdialog.cpp
# End Source File
# Begin Source File

SOURCE=startdialog.h
# End Source File
# Begin Source File

SOURCE=variabledialog.cpp
# End Source File
# Begin Source File

SOURCE=variabledialog.h
# End Source File
# Begin Source File

SOURCE=wizardeditor.cpp
# End Source File
# Begin Source File

SOURCE=wizardeditor.h
# End Source File
# End Group
# End Target
# End Project
