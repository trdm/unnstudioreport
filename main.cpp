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
//    for (int r=0; r<10000; r++)    testStru();
//	uoReport::uoRunTest();
//	return -1;

    QGridLayout* gridLayout = new QGridLayout();

//    QDialog m_Dlg;
    QFrame m_Dlg;
    m_Dlg.setLayout(gridLayout);
    gridLayout->setSpacing(0);
    gridLayout->setHorizontalSpacing(0);
     gridLayout->setContentsMargins(0, 0, 0, 0);

    m_Dlg.resize(800,700);
    m_Dlg.show();

	// немножко протестируем....
    uoReport::uoReportView m_GR(&m_Dlg);
    gridLayout->addWidget(&m_GR);
    m_GR.show();

    uoReport::uoReportDoc* doc  = m_GR.getControl()->getDoc();
    if (true)
    {
		doc->addGroup(1,2, uoReport::rhtHorizontal);
		doc->addGroup(3,12, uoReport::rhtHorizontal);
		doc->addGroup(9,12, uoReport::rhtHorizontal);
		doc->addGroup(1,3, uoReport::rhtVertical);
		doc->addGroup(1,2, uoReport::rhtVertical);
		doc->addGroup(8,80, uoReport::rhtVertical);
		doc->addGroup(9,20, uoReport::rhtVertical);
		doc->addSection(1,4, uoReport::rhtHorizontal,"first");
		doc->addSection(2,3, uoReport::rhtHorizontal,"second");
		doc->addSection(12,12, uoReport::rhtHorizontal,"third");

		doc->addSection(1,1, uoReport::rhtVertical,"first");
		doc->addSection(2,10, uoReport::rhtVertical,"second");
		doc->addSection(2,4, uoReport::rhtVertical,"third");
		doc->addSection(2,2, uoReport::rhtVertical,"third78");
		doc->addSection(12,12, uoReport::rhtVertical,"third2");
		doc->addSection(12,12, uoReport::rhtVertical,"third3");
		doc->addSection(12,12, uoReport::rhtVertical,"third4");

		doc->setScaleSize(uoReport::rhtHorizontal, 2, 120.5);
		doc->setScaleSize(uoReport::rhtHorizontal, 9, 0.0);
		doc->setScaleSize(uoReport::rhtVertical, 9, 3.0);
		doc->setScaleSize(uoReport::rhtVertical, 1, 45.3);
		doc->setScaleSize(uoReport::rhtVertical, 35, 45.3);
	//    doc->setCellText(2,2,"Behind the scenes, QString uses implicit sharing (copy-on-write) to reduce memory usage and to avoid the needless copying of data. This also helps reduce the inherent overhead of storing 16-bit characters instead of 8-bit characters. ");
		doc->setCellText(2,3,"НеобычайнодлинноеСлово \n НеобычайнодлинноеСлово ");
		doc->setCellTextAlignment(2,3,uoReport::uoVA_Center, uoReport::uoHA_Center, uoReport::uoCTB_Transfer);
		doc->setCellTextAlignment(3,3,uoReport::uoVA_Center, uoReport::uoHA_Center, uoReport::uoCTB_Auto);
    }


    m_GR.getControl()->optionShow(true, true, true, true);
    m_GR.getControl()->setFocus();

    return app.exec();
}
