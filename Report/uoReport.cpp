/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "uoReport.h"
#include "uoReportDoc.h"
#include "uoSpanTree.h"
#include "uoReportDocBody.h"

#include "QDebug"
#include "QPrinter"
#include "QPainter"


namespace uoReport {

/**
	\class uoReportTest - класс обобщенного тестирования
	\brief класс для тестирования всех классов в пространстве uoReport
*/
uoReportTest::uoReportTest()
{
}
uoReportTest::~uoReportTest(){
}



void uoReportTest::exploreQPrinter()
{
    QPrinter* pPrinter = new QPrinter;
    qDebug() << "pPrinter->printerState()" << pPrinter->printerState();
    qDebug() << "pPrinter->printerName()" << pPrinter->printerName();
    qDebug() << "pPrinter->pageRect()" << pPrinter->pageRect();
    qDebug() << "pPrinter->paperRect()" << pPrinter->paperRect();
    qDebug() << "pPrinter->resolution()" << pPrinter->resolution();
    qDebug() << "pPrinter->pageSize()" << pPrinter->pageSize();
    qDebug() << "pPrinter->fullPage()" << pPrinter->fullPage();

	if (false) {
		QRect rct = pPrinter->pageRect();
		QPen pen;
		pen.setWidth(1);
		pen.setStyle(Qt::SolidLine);
		QPainter painter;
		painter.begin(pPrinter);
		painter.setPen(pen);
		QString str;
		int y = qMax(5, rct.y());
		int x = qMax(5, rct.x());
		qDebug() << "x" << x << "y" << y;
		QPoint pt1, pt2;

		for (; x<2000; x= x + 10, y= y + 10) {
			str = QString("%1").arg(x);
			pt1.setX(x);
			pt1.setY(y);
			pt2.setX(x+100);
			pt2.setY(y);
			painter.drawText(pt1, str);
			painter.drawLine(pt1, pt2);
			qDebug() << "x" << x << "y" << y;
			qDebug() << "pt1" << pt1 << "pt2" << pt2;
		}

		rct = pPrinter->paperRect();
		painter.drawLine(rct.topLeft(), rct.bottomRight());
		painter.drawLine(rct.topRight(), rct.bottomLeft());

		painter.drawLine(rct.topLeft(), rct.topRight());
		painter.drawLine(rct.bottomRight(), rct.bottomLeft());
		painter.drawLine(rct.topLeft(), rct.bottomLeft());
		painter.drawLine(rct.topRight(), rct.bottomRight());
		painter.drawRect(20,20,50,50);
		painter.end();
	}
    delete pPrinter;

}


/// Общая процедура запуска тестирования классов наймспейса "uoReport"
void uoRunTest(){
    uoSpanTree pST;
    pST.testClass();

    uoHeaderScale header;
    header.test();

    uoReportDoc uorptDoc;
    uorptDoc.test();

}

} // namespace uoReport
