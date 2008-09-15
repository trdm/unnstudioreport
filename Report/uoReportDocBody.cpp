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
{
	_def_size = -1;
}

uoHeaderScale::~uoHeaderScale(){
	//clear(); // а надо тут вызывать??????
}

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


/// Получить размер итема, а если итем не существует, тогда его дефолтный размер..
qreal uoHeaderScale::getSize(int nom, bool isDef){
	if (findItem(nom)) {
		uoRptNumLine* item = *_itSave;
		if (item)
			return item->size(isDef);
	}
	return _def_size;
}

/// Установить размер итема, а если итем не существует, создать и установить
bool	uoHeaderScale::setSize(int nom, qreal size, bool isDef){
	if (!findItem(nom, true)){
		return false;
	} else {
		uoRptNumLine* item = *_itSave;
		if (item) {
			item->setSize(size, isDef);
			return true;
		}
	}
	return false;
}

/// После создания итема.
void uoHeaderScale::onCreateItem(uoRptNumLine* crItem)
{
	if (crItem){
		crItem->setSize(_def_size);
	}
}

/// Перед удалением итема.
void uoHeaderScale::onDeleteItem(uoRptNumLine* delItem)
{
}

///============================================
/// uoRow uoRow uoRow uoRow uoRow uoRow

uoRow::uoRow(int nom)
	:_number(nom){

}
uoRow::~uoRow(){
}

/// Получить ячейку.
uoCell* uoRow::getCell(int posX, bool needCreate){
	return getItem(posX, needCreate);
}

/// Функция вызывается после создания нового итема. Возможно пригодится для ундо/редо.
void uoRow::onDeleteItem(uoCell* delItem){
}

/// Функция вызывается перед удалением итема.
void uoRow::onCreateItem(uoCell* crItem){
}

/// Записываем в файл содержимое строки.
void uoRow::saveItems(uoReportLoader* loader){
	detachIter();
	if (_list->isEmpty())
		return;
	uoCell* item = NULL;
	QLinkedList<uoCell*>::iterator itLst = _list->begin();
	while(itLst != _list->end()) {
		item = *itLst;
		loader->saveCell(item);
		itLst++;
	}
}

/// uoRowsDoc==========================================
uoRowsDoc::uoRowsDoc(): uoNumVector<uoRow>()

{
}
uoRowsDoc::~uoRowsDoc(){
	//clear(); // а надо тут вызывать??????
}

void uoRowsDoc::onDeleteItem(uoRow* delItem){
}
void uoRowsDoc::onCreateItem(uoRow* crItem){
}

/// Найти ячейку, если она существует, и оздать в случае необходимости (параметр needCreate).
uoCell* uoRowsDoc::getCell(int posY, int posX, bool needCreate){
	uoRow* row = getItem(posY, needCreate);
	if (row) {
		return row->getItem(posX, needCreate);
	}
	return NULL;
}


/// Получить текст ячейки
QString uoRowsDoc::getText(int posY, int posX){
	QString retVal;

	uoCell* cell = getCell(posY, posX, false);
	if (cell){
		retVal = cell->_text;
	}
	return retVal;
}

/// Установить текст в ячейку.
bool uoRowsDoc::setText(const int posY, const int posX, QString text){
	bool isFind = false;

	uoCell* cell = getCell(posY, posX, true);
	if (cell){
		cell->_text = text;
		isFind = true;
	}
	return isFind;
}

/// Сохранение строк документа.
void uoRowsDoc::saveItems(uoReportLoader* loader)
{

	detachIter();
	if (_list->isEmpty())
		return;
	uoRow* item = NULL;
	QLinkedList<uoRow*>::iterator itLst = _list->begin();
	while(itLst != _list->end()) {
		item = *itLst;
		loader->saveRowItemStart(item->number(), item->getCountItem());
		item->saveItems(loader);
		loader->saveRowItemEnd();
		itLst++;
	}
}






} //namespace uoReport
