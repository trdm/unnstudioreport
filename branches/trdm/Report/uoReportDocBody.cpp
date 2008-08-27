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

void uoHeaderScale::test()
{
	qDebug() << "uoHeaderScale::test";
	setDefSize(200);
	setSize(1, 0.0);
	setSize(2, 0.0);
	setSize(2, 120.5);
	setSize(9, 120.5);
	printToDebug();
	getSize(4);
	setHide(4, true);
	printToDebug();
	getSize(5);
	setHide(5, true);
	printToDebug();
	clear();
	qDebug() << "uoHeaderScale::test-2";
	setSize(1, 0.0);
	setSize(9, 120.5);
	getSize(2);
	setHide(2, true);
	printToDebug();

	clear();
	qDebug() << "uoHeaderScale::test-3";
	setSize(9, 3.0);
	setSize(1, 45.3);
	printToDebug();
	int i = 0;
	for (i = 9; i<=20; i++){
		getSize(i);
		setHide(i, true);
	}
	printToDebug();
	setHide(2, true);
	printToDebug();

}






} //namespace uoReport
