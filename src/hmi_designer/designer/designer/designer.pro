TEMPLATE	= lib
CONFIG		+= qt warn_on staticlib qmake_cache
CONFIG 		-= dll
!force_static:!win32:contains(QT_PRODUCT,qt-internal) {
   CONFIG          -= staticlib
   CONFIG          += dll 
}

TARGET	= designer
win32:TARGET = designerlib

DEFINES	+= DESIGNER
DEFINES += QT_INTERNAL_XML
DEFINES += QT_INTERNAL_WORKSPACE
DEFINES += QT_INTERNAL_ICONVIEW
DEFINES += QT_INTERNAL_TABLE
table:win32-msvc:DEFINES+=QM_TEMPLATE_EXTERN_TABLE=extern

unix {
	QMAKE_CFLAGS += $$QMAKE_CFLAGS_SHLIB
	QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_SHLIB
}

include( ../../../src/qt_professional.pri )

SOURCES	+= command.cpp \
		formwindow.cpp \
		defs.cpp \
		layout.cpp \
		mainwindow.cpp \
		mainwindowactions.cpp \
		metadatabase.cpp \
		pixmapchooser.cpp \
		propertyeditor.cpp \
		resource.cpp \
		sizehandle.cpp \
		orderindicator.cpp \
		widgetfactory.cpp \
		hierarchyview.cpp \
		listboxeditorimpl.cpp \
		newformimpl.cpp \
		workspace.cpp \
		listvieweditorimpl.cpp \
		customwidgeteditorimpl.cpp \
		paletteeditorimpl.cpp \
		styledbutton.cpp \
		iconvieweditorimpl.cpp \
		multilineeditorimpl.cpp \
		formsettingsimpl.cpp \
		asciivalidator.cpp \
		designerapp.cpp \
		designerappiface.cpp \
		actioneditorimpl.cpp \
		actionlistview.cpp \
		actiondnd.cpp \
		project.cpp \
		projectsettingsimpl.cpp \
		sourceeditor.cpp \
		outputwindow.cpp \
		../shared/widgetdatabase.cpp \
		../shared/parser.cpp \
		config.cpp \
		pixmapcollection.cpp \
		previewframe.cpp \
		previewwidgetimpl.cpp \
		paletteeditoradvancedimpl.cpp \
		sourcefile.cpp \
		filechooser.cpp \
		wizardeditorimpl.cpp \
		qcompletionedit.cpp \
		timestamp.cpp \
		formfile.cpp \
		qcategorywidget.cpp \
		widgetaction.cpp \
		propertyobject.cpp \
		startdialogimpl.cpp \
		syntaxhighlighter_html.cpp \
		connectionitems.cpp \
		editfunctionsimpl.cpp \
		variabledialogimpl.cpp \
		listviewdnd.cpp \
		listboxdnd.cpp \
		listdnd.cpp \
		listboxrename.cpp \
		connectiontable.cpp

HEADERS	+= command.h \
		defs.h \
		formwindow.h \
		layout.h \
		mainwindow.h \
		metadatabase.h \
		pixmapchooser.h \
		propertyeditor.h \
		resource.h \
		sizehandle.h \
		orderindicator.h \
		widgetfactory.h \
		hierarchyview.h \
		listboxeditorimpl.h \
		newformimpl.h \
		workspace.h \
		listvieweditorimpl.h \
		customwidgeteditorimpl.h \
		paletteeditorimpl.h \
		styledbutton.h \
		iconvieweditorimpl.h \
		multilineeditorimpl.h \
		formsettingsimpl.h \
		asciivalidator.h \
		../interfaces/widgetinterface.h \
		../interfaces/actioninterface.h \
		../interfaces/filterinterface.h \
		../interfaces/designerinterface.h \
		designerapp.h \
		designerappiface.h \
		actioneditorimpl.h \
		actionlistview.h \
		actiondnd.h \
		project.h \
		projectsettingsimpl.h \
		sourceeditor.h \
		outputwindow.h \
		../shared/widgetdatabase.h \
		../shared/parser.h \
		config.h \
		previewframe.h \
		previewwidgetimpl.h \
		paletteeditoradvancedimpl.h \
		pixmapcollection.h \
		sourcefile.h \
		filechooser.h \
		wizardeditorimpl.h \
		qcompletionedit.h \
		timestamp.h \
		formfile.h \
		qcategorywidget.h \
		widgetaction.h \
		propertyobject.h \
		startdialogimpl.h \
		syntaxhighlighter_html.h \
		connectionitems.h \
		editfunctionsimpl.h \
		variabledialogimpl.h \
		listviewdnd.h \
		listboxdnd.h \
		listdnd.h \
		listboxrename.h \
		connectiontable.h

FORMS		+= listboxeditor.ui \
		editfunctions.ui \
		newform.ui \
		listvieweditor.ui \
		customwidgeteditor.ui \
		paletteeditor.ui \
		iconvieweditor.ui \
		preferences.ui \
		multilineeditor.ui \
		formsettings.ui \
		about.ui \
		pixmapfunction.ui \
		createtemplate.ui \
		actioneditor.ui \
		projectsettings.ui \
		finddialog.ui \
		replacedialog.ui \
		gotolinedialog.ui \
		pixmapcollectioneditor.ui \
		previewwidget.ui \
		paletteeditoradvanced.ui \
		wizardeditor.ui \
		listeditor.ui \
		startdialog.ui \
		richtextfontdialog.ui \
		connectiondialog.ui \
		variabledialog.ui \
		configtoolboxdialog.ui

IMAGES		+= images/adjustsize.png \
		images/edithlayoutsplit.png \
		images/left.png \
		images/sizeall.png \
		images/arrow.png \
		images/editlower.png \
		images/line.png \
		images/sizeb.png \
		images/background.png \
		images/editpaste.png \
		images/lineedit.png \
		images/sizef.png \
		images/book.png \
		images/editraise.png \
		images/listbox.png \
		images/sizeh.png \
		images/buttongroup.png \
		images/editslots.png \
		images/listview.png \
		images/sizev.png \
		images/checkbox.png \
		images/editvlayout.png \
		images/multilineedit.png \
		images/slider.png \
		images/combobox.png \
		images/editvlayoutsplit.png \
		images/newform.png \
		images/spacer.png \
		images/connecttool.png \
		images/filenew.png \
		images/no.png \
		images/spinbox.png \
		images/cross.png \
		images/fileopen.png \
		images/ordertool.png \
		images/splash.png \
		images/customwidget.png \
		images/filesave.png \
		images/pixlabel.png \
		images/table.png \
		images/databrowser.png \
		images/form.png \
		images/pointer.png \
		images/tabwidget.png \
		images/datatable.png \
		images/frame.png \
		images/print.png \
		images/textbrowser.png \
		images/dataview.png \
		images/groupbox.png \
		images/progress.png \
		images/textedit.png \
		images/dateedit.png \
		images/hand.png \
		images/project.png \
		images/textview.png \
		images/datetimeedit.png \
		images/help.png \
		images/pushbutton.png \
		images/timeedit.png \
		images/dial.png \
		images/home.png \
		images/qtlogo.png \
		images/toolbutton.png \
		images/down.png \
		images/hsplit.png \
		images/radiobutton.png \
		images/undo.png \
		images/editbreaklayout.png \
		images/ibeam.png \
		images/redo.png \
		images/up.png \
		images/resetproperty.png \
		images/editcopy.png \
		images/iconview.png \
		images/resetproperty.png \
		images/uparrow.png \
		images/editcut.png \
		images/image.png \
		images/richtextedit.png \
		images/vsplit.png \
		images/editdelete.png \
		images/label.png \
		images/right.png \
		images/wait.png \
		images/editgrid.png \
		images/layout.png \
		images/scrollbar.png \
		images/widgetstack.png \
		images/edithlayout.png \
		images/lcdnumber.png \
		images/searchfind.png \
		images/folder.png \
		images/setbuddy.png \
		images/textbold.png \
		images/textcenter.png \
		images/texth1.png \
		images/texth2.png \
		images/texth3.png \
		images/textitalic.png \
		images/textjustify.png \
		images/textlarger.png \
		images/textleft.png \
		images/textlinebreak.png \
		images/textparagraph.png \
		images/textright.png \
		images/textsmaller.png \
		images/textteletext.png \
		images/textunderline.png \
		images/wizarddata.png \
		images/wizarddialog.png \
		images/d_adjustsize.png \
		images/d_label.png \
		images/d_book.png \
		images/d_layout.png \
		images/d_buttongroup.png \
		images/d_lcdnumber.png \
		images/d_checkbox.png \
		images/d_left.png \
		images/d_combobox.png \
		images/d_line.png \
		images/d_connecttool.png \
		images/d_lineedit.png \
		images/d_customwidget.png \
		images/d_listbox.png \
		images/d_databrowser.png \
		images/d_listview.png \
		images/d_datatable.png \
		images/d_multilineedit.png \
		images/d_dataview.png \
		images/d_newform.png \
		images/d_dateedit.png \
		images/d_ordertool.png \
		images/d_datetimeedit.png \
		images/d_pixlabel.png \
		images/d_dial.png \
		images/d_pointer.png \
		images/d_down.png \
		images/d_print.png \
		images/d_editbreaklayout.png \
		images/d_progress.png \
		images/d_editcopy.png \
		images/d_project.png \
		images/d_editcut.png \
		images/d_pushbutton.png \
		images/d_editdelete.png \
		images/d_radiobutton.png \
		images/d_editgrid.png \
		images/d_redo.png \
		images/d_edithlayout.png \
		images/d_richtextedit.png \
		images/d_edithlayoutsplit.png \
		images/d_right.png \
		images/d_editlower.png \
		images/d_scrollbar.png \
		images/d_editpaste.png \
		images/d_searchfind.png \
		images/d_editraise.png \
		images/d_slider.png \
		images/d_editslots.png \
		images/d_spacer.png \
		images/d_editvlayout.png \
		images/d_spinbox.png \
		images/d_editvlayoutsplit.png \
		images/d_table.png \
		images/d_filenew.png \
		images/d_folder.png \
		images/d_tabwidget.png \
		images/d_fileopen.png \
		images/d_textbrowser.png \
		images/d_filesave.png \
		images/d_textedit.png \
		images/d_form.png \
		images/d_textview.png \
		images/d_frame.png \
		images/d_timeedit.png \
		images/d_groupbox.png \
		images/d_toolbutton.png \
		images/d_help.png \
		images/d_undo.png \
		images/d_home.png \
		images/d_up.png \
		images/d_iconview.png \
		images/d_widgetstack.png \
		images/d_setbuddy.png \
		images/d_textbold.png \
		images/d_texth1.png \
		images/d_texth2.png \
		images/d_texth3.png \
		images/d_textitalic.png \
		images/d_textjustify.png \
		images/d_textlarger.png \
		images/d_textleft.png \
		images/d_textlinebreak.png \
		images/d_textparagraph.png \
		images/d_textright.png \
		images/d_textsmaller.png \
		images/d_textteletext.png \
		images/d_textunderline.png \
		images/d_textcenter.png \
		images/d_wizarddata.png \
		images/d_wizarddialog.png \
		images/s_editcut.png \
		images/s_up.png \
		images/s_down.png \
		images/s_left.png \
		images/s_right.png \
		images/d_image.png \
		images/d_textfont.png \
		images/textfont.png \
		images/object.png \
		images/appicon.png


OBJECTS_DIR	= .

DEPENDPATH	+= $$QT_SOURCE_TREE/include
VERSION  	= 1.0.0
DESTDIR		= $$QT_BUILD_TREE/lib

aix-g++ {
	QMAKE_CFLAGS += -mminimal-toc
	QMAKE_CXXFLAGS += -mminimal-toc
}

sql {
	SOURCES  += database.cpp dbconnectionimpl.cpp dbconnectionsimpl.cpp
	HEADERS += database.h dbconnectionimpl.h dbconnectionsimpl.h
	FORMS += dbconnections.ui dbconnection.ui dbconnectioneditor.ui
}

table {
	HEADERS += tableeditorimpl.h
	SOURCES += tableeditorimpl.cpp
	FORMS += tableeditor.ui
}

INCLUDEPATH	+= ../shared ../uilib
win32:LIBS	+= $$QT_BUILD_TREE/lib/qui.lib $$QT_BUILD_TREE/lib/qassistantclient.lib
unix:LIBS		+= -L$$QT_BUILD_TREE/lib -lqui $$QT_BUILD_TREE/lib/libqassistantclient.a

TRANSLATIONS	= designer_de.ts designer_fr.ts

target.path=$$libs.path
INSTALLS += target
templates.path=$$data.path/templates
templates.files = ../templates/*
INSTALLS += templates
