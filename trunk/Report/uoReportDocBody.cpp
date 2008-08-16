/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "uoReportDocBody.h"
#include "QDebug"

namespace uoReport {





uoHeaderScale::uoHeaderScale()
	: uoNumVector<uoRptNumLine>()
{}

uoHeaderScale::~uoHeaderScale()
{}

/// Получение признака спрятанности итема
bool uoHeaderScale::getHide(int nom ){
	if (!findItem(nom)){
		return false;
	} else {
		uoRptNumLine* item = *_itSave;
		if (item) {
			return item->hiden();
		}
	}
	return false;
}

/// Установка признака спрятанности итема
void uoHeaderScale::setHide(int nom, bool hide){
	if (!findItem(nom, true)){
		return;
	} else {
		uoRptNumLine* item = *_itSave;
		if (item) {
			item->setHiden(hide);
			return;
		}
	}
	return;
}
/// сохраняем итемы...
bool uoHeaderScale::onStoreItems(uoReportLoader* loader){

	detachIter();
	if (_list->isEmpty())
		return true;
	uoRptNumLine* item = NULL;
	QLinkedList<uoRptNumLine*>::iterator itLst = _list->begin();
	while(itLst != _list->end()) {
		item = *itLst;
		loader->saveScaleItem(item);
		itLst++;
	}
	return true;
}


/// Вывод в текстовый файл отладочной информации.
void uoHeaderScale::printToDebug()
{
	detachIter();
	if (_list->isEmpty())
		return;
	qDebug() << "############ size = " << _list->size();
	uoRptNumLine* item = NULL;
	QLinkedList<uoRptNumLine*>::iterator itLst = _list->begin();
	while(itLst != _list->end()) {
		item = *itLst;
		qDebug() << "№ "<< item->number() << " size " << item->size();
		itLst++;
	}
}


/*====================================================================
						uoReportDocBody
тело документа  содержимое документа: строки, текст, картинки и т.п.

====================================================================*/


uoReportDocBody::uoReportDocBody()
	: _headerV(new uoHeaderScale)
	, _headerH(new uoHeaderScale)
{
	_headerV->setDefSize(UORPT_SCALE_SIZE_DEF_VERTICAL);
	_headerH->setDefSize(UORPT_SCALE_SIZE_DEF_HORIZONTAL);
	_rowCount = 0;
	_columnCount = 0;
	//ctor
}

uoReportDocBody::~uoReportDocBody()
{
	delete _headerV;
	delete _headerH;
}

rptSize uoReportDocBody::getScaleSize(uoRptHeaderType hType, int nom, bool isDef)
{
	if (hType == rhtVertical)
		return _headerV->getSize(nom, isDef);
	else
		return _headerH->getSize(nom, isDef);
}

void uoReportDocBody::setScaleSize(uoRptHeaderType hType, int nom, rptSize size, bool isDef){
	if (hType == rhtVertical)
		_headerV->setSize(nom, size, isDef);
	else
		_headerH->setSize(nom, size, isDef);
}

/// Прячем/Показываем диапазон ячеек...
void uoReportDocBody::setScalesHide(uoRptHeaderType hType, int nmStart, int cnt,  bool hide){
	uoHeaderScale* header = NULL;
	if (hType == rhtVertical)
		header = _headerV;
	else
		header = _headerH;
	for (int i = 0; i<cnt; i++)	{
		header->setHide(nmStart + i, hide);
	}
}

bool uoReportDocBody::getScaleHide(uoRptHeaderType hType, int nom){
	if (hType == rhtVertical)
		return _headerV->getHide(nom);
	else
		return _headerH->getHide(nom);
}


static void toDebugTest(bool resComp, int *testOkCnt, int *testAll, const char* str)
{
	if (resComp) {
		// если тест прошел, зачем печатать?
		++(*testOkCnt);  //qDebug()<<"success: "<<str;
	} else {
		qDebug()<<"failed: "<<str;
	}
	++(*testAll);
}

/// Тестирование класса.
void uoReportDocBody::test(){

    int nTestOk = 0, nTestAll = 0;
    bool printAll = true;
	bool printCurent = true;

	rptSize sz = 17;

    qDebug()<<"Start test class \"uoReportDocBody\"";
    qDebug()<<"{";
	if (false || printAll) {
		setScaleSize(rhtVertical, 2, 15);
		setScaleSize(rhtVertical, 4, 18); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 3, 19); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 5, 25); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 2, sz); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 1, sz); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 7, 11); if (printCurent) _headerV->printToDebug();

		toDebugTest(getScaleSize(rhtVertical, 1) == sz, &nTestOk, &nTestAll, "getScaleSize(rhtVertical, 1) == sz");
		toDebugTest(getScaleSize(rhtVertical, 2) == sz, &nTestOk, &nTestAll, "getScaleSize(rhtVertical, 2) == sz");
		qDebug() << " size " << _headerV->getCountItem();
		qDebug() << " _headerV->deleteItem(2,2); ";
		_headerV->deleteItem(2,2);
		qDebug() << " size " << _headerV->getCountItem();
		_headerV->printToDebug();
	}


    qDebug()<<"Test all: "<<nTestAll<<" test OK: "<< nTestOk<<" test is: "<< (nTestOk==nTestAll);
    qDebug()<<"End test class \"uoReportDocBody\"";
    qDebug()<<"}";

}


} //namespace uoReport
