######################################################################
# Automatically generated by qmake (2.01a) ?? 16. ??? 17:47:14 2008
######################################################################

TEMPLATE = app
TARGET =
DEPENDPATH += . common Report
INCLUDEPATH += .

QMAKE_CXXFLAGS_DEBUG  *= -pg
QMAKE_LFLAGS_DEBUG *= -pg

CONFIG += warn_on

UI_DIR  = Report
UI_HEADERS_DIR  = Report
UI_SOURCES_DIR  = Report

include( shared.pri )

QT += xml
QT -= sql

# Input
HEADERS += main.h \
           common/codecs.h \
           common/debug.h \
           common/export.h \
           common/params.h \
           common/sys_conv.h \
           common/types.h \
           Report/uoCacheItemizer.h \
           Report/uoColorCContainer.h \
           Report/uoColorChooser.h \
           Report/uoFrameChooser.h \
           Report/uoNumVector.h \
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
           Report/uorPagePrintSetings.h \
           Report/uorPageSetup.h \
           Report/uorReportAreas.h \
           Report/uoSpanTree.h
FORMS += Report/uoInputId.ui \
         Report/uoPreviewDlg.ui \
         Report/uorBorderProp.ui \
         Report/uorPageSetupDlg.ui \
         Report/uorPropDlg.ui \
         Report/uorSubstrateDlgDats.ui \
         Report/uorTextFontProp.ui \
         Report/uorTextLayotProp.ui \
         Report/uorTextProp.ui
SOURCES += main.cpp \
           common/codecs.cpp \
           common/debug.cpp \
           common/params.cpp \
           common/sys_conv.cpp \
           Report/uoCacheItemizer.cpp \
           Report/uoColorCContainer.cpp \
           Report/uoColorChooser.cpp \
           Report/uoFrameChooser.cpp \
           Report/uoNumVector.cpp \
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
           Report/uorPagePrintSetings.cpp \
           Report/uorPageSetup.cpp \
           Report/uorReportAreas.cpp \
           Report/uoSpanTree.cpp
RESOURCES += uoReport.qrc
