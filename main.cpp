#include <qapplication.h>
#include "common/codecs.h"
#include <QDialog>
#include <QGridLayout>
#include "common/debug.h"
#include "Report/uoReport.h"
#include "Report/uoReportCtrl.h"
#include "Report/uoReportDoc.h"
#include "Report/uoSpanTree.h"
#include "Report/uoReportDocBody.h"
#include "Report/uoNumVector.h"
#include "Report/uoReportView.h"



#include "QDebug"
#include "QPushButton"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "rus");
    InitCodecs();


    QApplication app(argc, argv);
    InstallMsgHandler("debug_log.txt");    /// теперь можно тестировать...
//	uoReport::uoRunTest();
//	return -1;

    QGridLayout* gridLayout = new QGridLayout();

    QDialog m_Dlg;
    m_Dlg.setLayout(gridLayout);
    gridLayout->setSpacing(0);
    gridLayout->setHorizontalSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    m_Dlg.resize(500,500);
    m_Dlg.show();

	// немножко протестируем....
    uoReport::uoReportView m_GR(&m_Dlg);
    gridLayout->addWidget(&m_GR);
    m_GR.show();

    uoReport::uoReportDoc* doc  = m_GR.getControl()->getDoc();
    doc->addGroup(1,2, uoReport::rhtHorizontal);
    doc->addGroup(3,12, uoReport::rhtHorizontal);
    doc->addGroup(9,12, uoReport::rhtHorizontal);
    doc->addGroup(1,3, uoReport::rhtVertical);
    doc->addGroup(1,2, uoReport::rhtVertical);
    doc->addGroup(8,22, uoReport::rhtVertical);
    doc->addGroup(9,20, uoReport::rhtVertical);
    doc->addSection(1,4, uoReport::rhtHorizontal);
    doc->addSection(2,3, uoReport::rhtHorizontal);
    doc->setScaleSize(uoReport::rhtHorizontal, 1, 0.0);
    doc->setScaleSize(uoReport::rhtHorizontal, 2, 120.5);
    doc->setScaleSize(uoReport::rhtHorizontal, 9, 0.0);
    doc->setScaleSize(uoReport::rhtVertical, 9, 3.0);
    doc->setScaleSize(uoReport::rhtVertical, 1, 45.3);

    m_GR.getControl()->optionShow(true, true, false, true);

    return app.exec();
}
