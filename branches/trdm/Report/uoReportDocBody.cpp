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


///=====================================================
///==============uoReportDocFontColl====================
uoReportDocFontColl::uoReportDocFontColl()
{
}

uoReportDocFontColl::~uoReportDocFontColl()
{
	QFont* font = NULL;
	while (!_fontList.isEmpty()) {
		font = _fontList.takeFirst();
		delete font;
	}

}

/// Получить фонт для отрисовки
QFont* uoReportDocFontColl::getFont(int nmFont)
{
	if (_fontList.isEmpty())
		return NULL;
	if (_fontList.size() < (nmFont-1))
		return NULL;
	return _fontList.at(nmFont);
}

/// чиста поиск.
int uoReportDocFontColl::findFont(QString fontName)
{
	int retVal = -1;
	if (!_fontList.isEmpty()){
		QFont* font = NULL;
		for (int i = 0; i<_fontList.size(); i++){
			font = _fontList.at(i);
			if (font->family() == fontName){
				return i;
			}
		}
	}
	return retVal;
}

/// Добавим фонт.
int uoReportDocFontColl::addFont(QString fontName)
{
	int retVal = findFont(fontName);
	if (retVal == -1){
		// есть возможность поработать с фонтом на свое усмотрение.
		QFont* font = new QFont(fontName);
		_fontList.append(font);
		retVal = _fontList.size()-1;
	}


	return retVal;
}

/// Получить ID фонта. Именно тут "Заводятся" новые фонты
int uoReportDocFontColl::getFontId(QString fontName)
{
	int retVal = findFont(fontName);
	if (retVal == -1){
		retVal = addFont(fontName);
	}

	return retVal;
}

/// количество зарегистрированный шрифтов.
int uoReportDocFontColl::countFonts()
{
	if (_fontList.isEmpty())
		return 0;
	return _fontList.count();
}




///==============uoReportDocFontColl====================
///=====================================================

/// Взять текст.
QString uoCell::getText()
{
	if (_textProp){
		return _textProp->_text;
	}
	return QString("");
}

/// Установить текст. Надо гарантировать наличие структуры _textProp;
void uoCell::setText(QString text, uoReportDoc* doc)
{
	if (!doc)
		return;
	if (!_textProp)
		_textProp = doc->getNewTextProp();
	if (_textProp)
		_textProp->_text = text;


//	uoCellTextProps* _textProp
}

/// Отдаем фонт сразус размером
QFont* uoCell::getFont(uoReportDoc* doc)
{
	if (!doc || !_textProp)
		return NULL;
	QFont* font = doc->getFontByID(_textProp->_fontID);
	if (font){
		font->setPointSize(_textProp->_fontSize);
	}
	return font;
}

const QColor*  uoCell::getFontColor(uoReportDoc* doc)
{
	if (!doc || !_textProp)
		return NULL;
	return doc->getColorByID(_textProp->_fontColID);
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

/// Получить перечень номеров ячеек строки.
QList<int> uoRow::getItemNumList()
{
	QList<int> listNo;
	if (_list->isEmpty())
		return listNo;
	uoCell* item = NULL;
	QLinkedList<uoCell*>::iterator itLst = _list->begin();
	while(itLst != _list->end()) {
		item = *itLst;
		listNo.append(item->number());
		itLst++;
	}
	return listNo;
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
		retVal = cell->getText();
	}
	return retVal;
}

/// Установить текст в ячейку.
bool uoRowsDoc::setText(const int posY, const int posX, QString text){
	bool isFind = false;

	uoCell* cell = getCell(posY, posX, true);
	if (cell){
		cell->setText(text, _doc);
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

/// Получить строку по номеру.
uoRow* uoRowsDoc::getRow(int nmRow, bool needCreate)
{
	return getItem(nmRow, needCreate);
}






} //namespace uoReport
