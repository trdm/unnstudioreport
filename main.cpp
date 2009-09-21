#include <qapplication.h>
#include "common/codecs.h"
#include <QDialog>
#include <QGridLayout>
#include <QSplitter>
#include "common/debug.h"
#include "Report/uoReport.h"
#include "Report/uoReportCtrl.h"
#include "Report/uoReportDoc.h"
#include "Report/uoSpanTree.h"
#include "Report/uoReportDocBody.h"
#include "Report/uoNumVector.h"
#include "Report/uoReportView.h"



int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "rus");
    InitCodecs();
    using namespace uoReport;

    QApplication app(argc, argv);
    InstallMsgHandler("debug_log.txt");    /// теперь можно тестировать...
    if (false) {
		uoReport::uoRunTest();
		return -1;
	}

	uoReportTest rTest;
	rTest.exploreQPrinter();



    QFrame m_Dlg;
    QSplitter* spliter;
    QGridLayout* gridLayout;
 	uoReport::uoReportView* m_GR;
	uoReport::uoReportView* m_GR_2;
    uoReport::uoReportDoc* doc;

	gridLayout = new QGridLayout();
	gridLayout->setSpacing(0);
	gridLayout->setHorizontalSpacing(0);
	gridLayout->setContentsMargins(0, 0, 0, 0);

	m_Dlg.setLayout(gridLayout);
	if (true) {
		gridLayout->setSpacing(0);
		gridLayout->setHorizontalSpacing(0);
		gridLayout->setContentsMargins(0, 0, 0, 0);
		// немножко протестируем....
		m_GR = new uoReport::uoReportView(&m_Dlg);
		doc = m_GR->getControl()->getDoc();
		gridLayout->addWidget(m_GR);
		m_GR->show();
	} else {

		spliter = new QSplitter(&m_Dlg);
		gridLayout->addWidget(spliter);
		m_GR = new uoReport::uoReportView(spliter);
		doc = m_GR->getControl()->getDoc();
		doc->m_ident = 150;
		m_GR_2 = new uoReport::uoReportView(spliter);
		m_GR_2->getControl()->setDoc(doc);

	}


    m_Dlg.resize(800,700);
    m_Dlg.show();


    if (true)
    {
		if (false) {
			doc->addGroup(1,2, uoReport::uorRhtColumnHeader);
			doc->addGroup(3,12, uoReport::uorRhtColumnHeader);
			doc->addGroup(9,12, uoReport::uorRhtColumnHeader);
			doc->addGroup(1,3, uoReport::uorRhtRowsHeader);
			doc->addGroup(1,2, uoReport::uorRhtRowsHeader);
			doc->addGroup(8,80, uoReport::uorRhtRowsHeader);
			doc->addGroup(9,20, uoReport::uorRhtRowsHeader);
			doc->addSection(1,4, uoReport::uorRhtColumnHeader,"first");
			doc->addSection(2,3, uoReport::uorRhtColumnHeader,"second");
			doc->addSection(12,12, uoReport::uorRhtColumnHeader,"third");

			doc->addSection(1,1, uoReport::uorRhtRowsHeader,"first");
			doc->addSection(2,10, uoReport::uorRhtRowsHeader,"second");
			doc->addSection(2,4, uoReport::uorRhtRowsHeader,"third");
			doc->addSection(2,2, uoReport::uorRhtRowsHeader,"third78");
			doc->addSection(12,12, uoReport::uorRhtRowsHeader,"third2");
			doc->addSection(12,12, uoReport::uorRhtRowsHeader,"third3");
			doc->addSection(12,12, uoReport::uorRhtRowsHeader,"third4");

			doc->setScaleSize(uoReport::uorRhtColumnHeader, 2, 120.5);
			doc->setScaleSize(uoReport::uorRhtColumnHeader, 9, 0.0);
			doc->setScaleSize(uoReport::uorRhtRowsHeader, 9, 3.0);
			doc->setScaleSize(uoReport::uorRhtRowsHeader, 1, 45.3);
			doc->setScaleSize(uoReport::uorRhtRowsHeader, 35, 45.3);
		}
		if (false)
		    doc->setCellText(2,2,"Behind the scenes, QString uses implicit sharing (copy-on-write) to reduce memory usage and to avoid the needless copying of data. This also helps reduce the inherent overhead of storing 16-bit characters instead of 8-bit characters. ");
		if (false) {
			QFile file("reference.txt");
			if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
				QTextStream in(&file);
				QChar ch = '\t';
				QStringList list;
				QString line, linePart;
				int row = 1;
				while (!in.atEnd()) {
					QString line = in.readLine();
					list = line.split(ch);
					for (int y=1; y<=list.size(); y++){
						linePart = list.at(y-1);
						doc->setCellText(row,y,linePart);
						doc->setCellTextAlignment(row,y,uoReport::uoVA_Center, uoReport::uoHA_Left, uoReport::uoCTB_Transfer);
					}

					++row;
				}
			}
		}
		if (false) {
			doc->setCellText(2,3,"НеобычайнодлинноеСлово \n НеобычайнодлинноеСлово ");
			doc->setCellTextAlignment(2,3,uoReport::uoVA_Center, uoReport::uoHA_Center, uoReport::uoCTB_Transfer);
			doc->setCellTextAlignment(3,3,uoReport::uoVA_Center, uoReport::uoHA_Center, uoReport::uoCTB_Auto);

			uoReport::uoCell* cell = NULL;
			cell = doc->getCell(2,2, true, true);		cell->m_borderProp->setBorderTypeAll(uoCBT_SolidLine);
			cell = doc->getCell(2,3, true, true);		cell->m_borderProp->setBorderTypeAll(uoCBT_SolidLine);
			cell = doc->getCell(3,2, true, true);		cell->m_borderProp->setBorderTypeAll(uoCBT_DashDotLine);
			cell = doc->getCell(3,3, true, true);		cell->m_borderProp->setBorderTypeAll(uoCBT_DashLine);
		}
    }


    m_GR->getControl()->optionShow(true, true, true, true);
    m_GR->getControl()->doFixedView(2,2);
    m_GR->getControl()->setFocus();
    doc->enableCollectChanges(true);

    return app.exec();
}
