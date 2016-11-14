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
#include "uoNumVector2.h"
#include "uoNumVector.h"

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


struct uorTestEnumStru
{
	uorTestEnumStru(int nom)
		:m_nomber(nom)
		{};

	int m_nomber;
};



class uorTectRow : public uoNumVector2<uorTestEnumStru>
{
	public:
	uorTectRow(){}
	virtual~uorTectRow(){}
	virtual void onDeleteItem(uorTestEnumStru* delItem);
	virtual void onCreateItem(uorTestEnumStru* crItem);

};


void uorTectRow::onDeleteItem(uorTestEnumStru* delItem) { Q_UNUSED(delItem); }
void uorTectRow::onCreateItem(uorTestEnumStru* crItem)  { Q_UNUSED(crItem); }

void uoReportTest::testuoNumVector2()
{
	uorTectRow stru_map;
}

struct uoTestStru {
	uoTestStru(int no = 0):m_nom(no) {}
	int m_nom;
	QString m_srt;
	int number() {return m_nom;}
    void setNumber(int nom) {m_nom = nom;}
};


bool uoReportTest::testuoNumVector3()
{
	uoTestStru* item = 0;
	uoNumVector<uoTestStru>* massive = new uoNumVector<uoTestStru>;
	int rowCount = massive->getCountItem();
	item = massive->getItem(1, true);
	item = massive->getItem(10, true);
	rowCount = massive->getCountItem();
	delete massive;



	return true;
}


bool uorRangesExtract(QList<int>& listFrom, QList<QPoint*>& listTo )
{
	bool retVal = true;

	if (listFrom.isEmpty())
		return retVal;
	QPoint* point = new QPoint;

	int posStart = -1, posEnd = -1, posCur = -1, cntItem = listFrom.size();
	if (cntItem == 1) {
		posStart = posEnd = listFrom.at(0);
		point->setX(posStart);
		point->setY(posStart);
		listTo.append(point);
		return true;
	}

	int cnter = 0;
	int lastProcPosStart = -1; // Последняя обработанная позиция

	QList<int>::const_iterator it = listFrom.constBegin();
	while(it != listFrom.constEnd()){
		++cnter;
		posCur = *it;
		if (posStart == -1){
			posStart = posEnd = posCur;
		} else {
			if (posCur == (posEnd + 1)){
				posEnd = posCur;
			} else {
				point = new QPoint(posStart, posEnd);
				listTo.append(point);
				lastProcPosStart = posStart;
				posStart = posEnd = posCur;
			}
		}
		it++;
	}

	if (lastProcPosStart != posStart){
		point = new QPoint(posStart, posEnd);
		listTo.append(point);
	}
	return retVal;
}

void uorRangesClear(QList<QPoint*>& listTo)
{
	if (listTo.isEmpty())
		return;
	while(!listTo.isEmpty())
		delete listTo.takeFirst();
}
void uorZeroPointI(QPoint& point){
	point.setX(0); 	point.setY(0);
}

void uorZeroRectI(QRect& rct){
	rct.setLeft(0);	rct.setTop(0);	rct.setRight(0);	rct.setBottom(0);
}
void uorZeroRectF(uorRect& rct){
	rct.setLeft(0);	rct.setTop(0);	rct.setRight(0);	rct.setBottom(0);
}

void uorScaleRectI(QRect& rct, qreal scale)
{
	if (scale != 1.0) {
		qreal scale2 = 1 / scale;
		rct.setTop(int(scale2 * rct.top()));
		rct.setLeft(int(scale2 * rct.left()));
		rct.setBottom(int(scale2 * rct.bottom()));
		rct.setRight(int(scale2 * rct.right()));
	}

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
