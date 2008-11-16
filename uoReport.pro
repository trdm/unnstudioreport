######################################################################
# Automatically generated by qmake (2.01a) ?? 16. ??? 17:47:14 2008
######################################################################

TEMPLATE = app
TARGET =
DEPENDPATH += . common Report
INCLUDEPATH += .

QT += xml

include( shared.pri )
!win32 {
	debug {
		DESTDIR = Debug
		MOC_DIR = Debug
		OBJECTS_DIR = Debug
	}
	release {
		DESTDIR = Release
		MOC_DIR = Release
		OBJECTS_DIR = Release
	}
}

# Input
HEADERS += common/codecs.h \
           common/debug.h \
           common/export.h \
           common/params.h \
           common/sys_conv.h \
           common/types.h \
           Report/uoNumVector.h \
           Report/uoCacheItemizer.h \
           Report/uoReport.h \
           Report/uoReportCtrl.h \
           Report/uoReportDescr.h \
           Report/uoReportDoc.h \
           Report/uoReportDocBody.h \
           Report/uoReportLoader.h \
           Report/uoReportSelection.h \
           Report/uoReportView.h \
           Report/uoReportViewIteract.h \
           Report/uoSpanTree.h
SOURCES += main.cpp \
           common/codecs.cpp \
           common/debug.cpp \
           common/params.cpp \
           common/sys_conv.cpp \
           Report/uoNumVector.cpp \
           Report/uoCacheItemizer.cpp \
           Report/uoReport.cpp \
           Report/uoReportCtrl.cpp \
           Report/uoReportDoc.cpp \
           Report/uoReportDocBody.cpp \
           Report/uoReportLoader.cpp \
           Report/uoReportSelection.cpp \
           Report/uoReportView.cpp \
           Report/uoReportViewIteract.cpp \
           Report/uoSpanTree.cpp
