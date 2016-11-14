#include <qapplication.h>
#include "common/codecs.h"
#include <QDialog>
#include <QProgressDialog>
#include <QGridLayout>
#include <QSplitter>
#include "common/debug.h"
#include "uoReport.h"
#include "uoReportCtrl.h"
#include "uoReportDoc.h"
#include "uoSpanTree.h"
#include "uoReportDocBody.h"
//#include "Report/uoNumVector.h"
#include "uoNumVector2.h"
#include "uoReportView.h"
#include "uoReportManager.h"

using namespace uoReport;

void memoryTest()
{
	uoReport::uoReportDoc* doc = new uoReport::uoReportDoc;
	int all_val = 0, row_val = 0, col_val = 0;
	int lenghtAll = 0;
	for (int i=0; i<10; i++)
	{

		row_val = 500;
		col_val = 500;
		all_val = row_val * col_val;
		bool oldCC = doc->enableCollectChanges(false);
		bool oldDF = doc->enableFormating(false);
        doc->clear();

		QString addTextToCell = QString("text-text-text");

		QString cellSrt;

		for(int nnn = 1; nnn<=row_val; nnn++){
			for(int mm = 1; mm<=col_val; mm++) {
				all_val = nnn*mm;
				if (addTextToCell.isEmpty()) {
					cellSrt = QString("%1").arg(all_val);
				} else {
					cellSrt = QString("%1 + %2").arg(all_val).arg(addTextToCell);

				}
				lenghtAll += cellSrt.length() * 2;
				doc->setCellText(nnn,mm,cellSrt);
			}
		}
		cellSrt = QString("lenghtAll = %1").arg(lenghtAll);
		doc->setCellText(row_val+1,1,cellSrt);

		doc->enableCollectChanges(oldCC);
		doc->enableFormating(oldDF);
	}
	delete doc;
}

void test_report(){
	if (false) {
		uoReport::uoRunTest();
		uoReport::uoReportTest rTest;
		rTest.testuoNumVector2();
		rTest.testuoNumVector3();
		return;
	}
	if (false) {

		QList<int> listInt;
		listInt << 1 << 2 << 10 << 11 << 12 << 14;
		QList<QPoint*> listP;
		if (uorRangesExtract(listInt, listP )){
			QPoint* point;
			foreach ( point, listP )
				qDebug() << *point;
		}

		uorRangesClear(listP);
	}
	if (false){
		memoryTest();
    }	
}

uoReport::uoReportDoc* doc;

void test_temp1() {
    bool testTempl_0 = false;
    if (testTempl_0)    {
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

			doc->setScaleSize(uoReport::uorRhtColumnHeader, 2, (uorNumber)120.5);
			doc->setScaleSize(uoReport::uorRhtColumnHeader, 9, (uorNumber)0.0);
			doc->setScaleSize(uoReport::uorRhtRowsHeader, 9, (uorNumber)3.0);
			doc->setScaleSize(uoReport::uorRhtRowsHeader, 1, (uorNumber)45.3);
			doc->setScaleSize(uoReport::uorRhtRowsHeader, 35, (uorNumber)45.3);
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
				bool oldCC = doc->enableCollectChanges(false);
				bool oldDF = doc->enableFormating(false);
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
				doc->enableCollectChanges(oldCC);
				doc->enableFormating(oldDF);
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
		if (false) {
			uoReport::uoCell* cell = doc->getCell(1,1, true, true);
			cell->setText("НеобычайнодлинноеСлово \n НеобычайнодлинноеСлово ");
			doc->setCellTextAlignment(1,1,uoReport::uoVA_Center, uoReport::uoHA_Center, uoReport::uoCTB_Transfer);

		}

    }
    
}

void test_output1SLike() {
    bool testTempl = false;
    if (testTempl) {
        doc->enableCollectChanges(true);
//        doc->setSourceDocFile(QString("D:/Progekts/_Linux/unNStudio/Report/Src/_test/vedTemplate2.xml"));
        QString fName = qApp->applicationDirPath() + "/_test/vedTemplate2.xml";
        if (QFile::exists(fName)){
            qDebug() << "fName" << fName;
            doc->setSourceDocFile(fName);
            doc->setVariable("ПечЗаголовокОтчета","Тестовый отчет");
            doc->setVariable("ПечПериодОтчета","Период с '01.01.2011' по '01.01.2012' гг.");
            doc->setVariable("ПечФильтрыОтчета","Фильтры отчета: 'Отсутствуют'");
            doc->setVariable("ПечДатаВремяФормирования","Время не установлено!");
            doc->setVariable("ПечГрупировки","Пока не заданы, тренируюсь :) !");
            doc->outputSection("cap|verMain");
            doc->setVariable("ИмяДетализации","Color");
            doc->joinSection("cap|verDop");
            doc->setVariable("ИмяДетализации","Size");
            doc->joinSection("cap|verDop");
            for (int row = 1; row <= 3; row ++){
                doc->setVariable("ПечГруппировка",QString("Товар %1").arg(row));
                doc->setVariable("ПечНачОст","20");
                doc->setVariable("ПечПрих","20");
                doc->setVariable("ПечРасх","20");
                doc->setVariable("ПечКонОСт","20");
                doc->setVariable("ПечДетализация",QString("Color-Товар %1").arg(row+10));
                doc->outputSection("Гр1Г|verMain");
                doc->joinSection("Гр1Г|verDop");
                doc->setVariable("ПечДетализация",QString("Size-Товар %1").arg(row+100));
                doc->joinSection("Гр1Г|verDop");
                doc->outputSection("Гр1|verMain");
                doc->setVariable("ПечДетализация",QString("Color-Товар+++ %1").arg(row+10));
                doc->joinSection("Гр1|verDop");
                doc->setVariable("ПечДетализация",QString("Size-Товар %1").arg(row+100));
                doc->joinSection("Гр1|verDop");

            }
            doc->updateViews();
            /*
                Выводит, разыменовывает, но надо скопировать:
                - при выводе секции: высоту строк, флаг фиксации, флаг видимости бордюры и текст.
                - колекцию шрифтов.
                При присоединении секции надо установить флаг фиксации,
            */
        }
    }
	bool testTempl_1 = true;
	
	if (testTempl_1){
		doc->setCellText(1,1,"1");	doc->setCellText(2,1,"One");
		doc->setCellText(1,2,"2");	doc->setCellText(2,2,"Two");
		doc->setCellText(1,3,"3");	doc->setCellText(2,3,"Three");doc->setCellText(3,3,"Finish");
		uoReport::uoCell* cell = NULL;
		for (int i_ttc = 1; i_ttc<=3;i_ttc++){
			for (int i_ttr = 1; i_ttr<=2;i_ttr++){
				cell = doc->getCell(i_ttr,i_ttc, true, true);
				cell->m_borderProp->setBorderTypeAll(uoCBT_SolidLine);
			}
		}
		cell = doc->getCell(3,3, true, true);
		cell->m_borderProp->setBorderTypeAll(uoCBT_SolidLine);
	}
	doc->clear();
	bool testTempl_2 = true;
	if (testTempl_2){
		doc->enableCollectChanges(true);
		QString fName = QString("D:/Progekts/_Linux/unNStudio/Report/Src/Temlp/Temlp.xml");
        if (QFile::exists(fName)){
			qDebug() << fName;
			doc->setSourceDocFile(fName);
            doc->setVariable("Templ1","1");
            doc->setVariable("Templ2","2");
			doc->setVariable("Templ3","3");
			doc->outputSection("Section_1");
			doc->setVariable("Templ1","One");
            doc->setVariable("Templ2","Two");
			doc->setVariable("Templ3","Three");
			doc->outputSection("Section_1");
			doc->setVariable("Templ3","Finish");
			doc->outputSection("Section_2");		
			doc->updateViews();
			qDebug() << doc->getRowCount();
		}
	}
    
}

void runTestAll(){
	test_report();
	test_temp1();
	test_output1SLike();
}


int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "rus");
    InitCodecs();

    QApplication app(argc, argv);
    InstallMsgHandler("debug_log.txt");    /// теперь можно тестировать...

//    runTestAll();

    QFrame m_Dlg;
    QSplitter* spliter;
    QGridLayout* gridLayout;
 	uoReport::uoReportView* m_GR;
	uoReport::uoReportView* m_GR_2;

	gridLayout = new QGridLayout();
	gridLayout->setSpacing(0);
#if (QT_VERSION > 0x040201)
		gridLayout->setHorizontalSpacing(0);
		gridLayout->setContentsMargins(0, 0, 0, 0);
#endif

	m_Dlg.setLayout(gridLayout);
	if (true) {
		gridLayout->setSpacing(0);
//		gridLayout->setHorizontalSpacing(0);
//		gridLayout->setContentsMargins(0, 0, 0, 0);
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
		m_GR_2 = new uoReport::uoReportView(spliter);
		m_GR_2->getControl()->setDoc(doc);

	}

    m_Dlg.resize(800,700);
    m_Dlg.show();
	// D:\Progekts\_Linux\unNStudio\Report\Src\Temlp

    m_GR->getControl()->optionShow(true, true, true, true);
    m_GR->getControl()->doFixedView(2,2);
    m_GR->getControl()->setFocus();

    return app.exec();
}
