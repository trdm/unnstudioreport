######################################################################
# Automatically generated by qmake (2.01a) ?? 5. ??? 18:10:25 2016
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += . Report Report/common
INCLUDEPATH += .

# Это отладка включается.
#QMAKE_CXXFLAGS_DEBUG  *= -pg
#QMAKE_LFLAGS_DEBUG *= -pg

CONFIG += warn_on

#UI_DIR  = Report
#UI_HEADERS_DIR  = Report
UI_SOURCES_DIR  = Report

#!win32 {
	debug {
		DESTDIR = Debug
		MOC_DIR = Debug
		UI_HEADERS_DIR = Debug
		UI_DIR = Debug
		OBJECTS_DIR = Debug
	}
	release {
		DESTDIR = Release
		MOC_DIR = Release
		UI_HEADERS_DIR = Release
		UI_DIR = Release
		OBJECTS_DIR = Release
	}
#}
win32 {
     LIBS += -lgdi32
}

QT += xml
QT -= sql

# Input
HEADERS += Report/main.h \
           Report/uoCacheItemizer.h \
           Report/uoCellMatrix.h \
           Report/uoColorCContainer.h \
           Report/uoColorChooser.h \
           Report/uoFrameChooser.h \
           Report/uoNumVector.h \
           Report/uoNumVector2.h \
           Report/uoPainter.h \
           Report/uoReport.h \
           Report/uoReportCtrl.h \
           Report/uoReportCtrlItems.h \
           Report/uoReportDescr.h \
           Report/uoReportDoc.h \
           Report/uoReportDocBody.h \
           Report/uoReportDrawHelper.h \
           Report/uoReportLoader.h \
           Report/uoReportManager.h \
           Report/uoReportPreviewDlg.h \
           Report/uoReportPropEditor.h \
           Report/uoReportSelection.h \
           Report/uoReportUndo.h \
           Report/uoReportView.h \
           Report/uoReportViewIteract.h \
           Report/uorMimeData.h \
           Report/uorOptionsDlg.h \
           Report/uorPagePrintSetings.h \
           Report/uorPageSetup.h \
           Report/uorReportAreas.h \
           Report/uoSheetPanel.h \
           Report/uoSpanTree.h \
           Report/common/codecs.h \
           Report/common/debug.h \
           Report/common/export.h \
           Report/common/params.h \
           Report/common/sys_conv.h \
           Report/common/types.h
FORMS += Report/uoInputId.ui \
		 Report/uoReportPreviewDlg.ui \
         Report/uorBorderProp.ui \
         Report/uorOptions.ui \
         Report/uorPageSetupDlg.ui \
         Report/uorPropDlg.ui \
         Report/uorSheetPanel.ui \
         Report/uorSubstrateDlgDats.ui \
         Report/uorTextFontProp.ui \
         Report/uorTextLayotProp.ui \
         Report/uorTextProp.ui
SOURCES += Report/main.cpp \
           Report/uoCacheItemizer.cpp \
           Report/uoCellMatrix.cpp \
           Report/uoColorCContainer.cpp \
           Report/uoColorChooser.cpp \
           Report/uoFrameChooser.cpp \
           Report/uoNumVector.cpp \
           Report/uoPainter.cpp \
           Report/uoReport.cpp \
           Report/uoReportCtrl.cpp \
           Report/uoReportCtrlItems.cpp \
           Report/uoReportDoc.cpp \
           Report/uoReportDocBody.cpp \
           Report/uoReportDrawHelper.cpp \
           Report/uoReportLoader.cpp \
           Report/uoReportManager.cpp \
           Report/uoReportPreviewDlg.cpp \
           Report/uoReportPropEditor.cpp \
           Report/uoReportSelection.cpp \
           Report/uoReportUndo.cpp \
           Report/uoReportView.cpp \
           Report/uoReportViewIteract.cpp \
           Report/uorMimeData.cpp \
           Report/uorOptionsDlg.cpp \
           Report/uorPagePrintSetings.cpp \
           Report/uorPageSetup.cpp \
           Report/uorReportAreas.cpp \
           Report/uoSheetPanel.cpp \
           Report/uoSpanTree.cpp \
           Report/common/codecs.cpp \
           Report/common/debug.cpp \
           Report/common/params.cpp \
           Report/common/sys_conv.cpp
RESOURCES += Report/uoReport.qrc \
    uoReport.qrc \
    uoReport.qrc \
    uoReport.qrc
