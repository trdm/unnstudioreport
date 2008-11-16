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

/// Получение свойства фиксированности размера строки
bool uoHeaderScale::getFixed(int nom )
{
	if (!findItem(nom)){
		return false;
	} else {
		uoRptNumLine* item = *_itSave;
		if (item) {
			return item->fixed();
		}
	}
	return false;

}

/// Установка/снятие свойства фиксированности
void uoHeaderScale::setFixed(int nom, bool fix){
	if (!findItem(nom, true)){
		return;
	} else {
		uoRptNumLine* item = *_itSave;
		if (item) {
			item->setFixed(fix);
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
//	qDebug() << "############ size = " << _list->size();
	uoRptNumLine* item = NULL;
	QLinkedList<uoRptNumLine*>::iterator itLst = _list->begin();
	while(itLst != _list->end()) {
		item = *itLst;
//		qDebug() << "№ "<< item->number() << " size " << item->size();
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
	clear();
}

/// чистка колекции шрифтов
void uoReportDocFontColl::clear()
{
	while (!_fontList.isEmpty()) {
		delete _fontList.takeFirst();
	}
	_fontList.clear();
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
		if (font){
			_fontList.append(font);
			retVal = _fontList.size()-1;
		}
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


//=========================================
//=========uoTextTrPointCash==============
uoTextTrPointCash::uoTextTrPointCash()
{
}

uoTextTrPointCash::~uoTextTrPointCash()
{
	clear();
}

/// Очистка
void uoTextTrPointCash::clear()
{
	while (!m_blockCash.isEmpty()) {
		delete m_blockCash.takeFirst();
	}
}
/// Возвращает свободную структуру uoTextBoundary
uoTextBoundary* uoTextTrPointCash::getTextTrPoint()
{
	uoTextBoundary* point = NULL;
	if (!m_blockCash.isEmpty()) {
		point = m_blockCash.takeFirst();
	} else {
		point = new uoTextBoundary;
	}
	if (point){
		point->_textBoundary = NULL;
		point->_charCount = 0;
	}

	return point;
}
/// Сдаем на склад ненужные итемы..
void uoTextTrPointCash::savePoint(uoTextBoundary* point)
{
	if (point){
		uoTextBoundary* next = point;
		while(next){
			next->_charCount = 0;
			m_blockCash.append(next);
			next = next->_textBoundary;
		}
	}
}


//=========uoTextTrPointCash==============
//=========================================

void uoCell::clear()
{

}

/// Взять текст.
QString uoCell::getText()
{
	if (_textProp){
		return _textProp->_text;
	}
	return QString("");
}

bool uoCell::provideTextProp(uoReportDoc* doc, bool needCreate /* = false*/)
{
	if (_textProp)
		return true;
	if (doc && needCreate){
		_textProp = doc->getNewTextProp();
		if (_textProp)
			return true;
	}
	return false;
}

/// Вернуть текст, разделеленный согласно форматированию символом, специально для печати.
QString uoCell::getTextWithLineBreak(bool drawInv)
{
	QString str;
	if (_textProp){
		QString cellTxt = _textProp->_text;
		QString tmpStr;
		QString ch = " ";

		if (drawInv) {
			ch = QString::fromUtf8("¶");
			cellTxt = cellTxt.replace('\n' ,ch);

			ch = QString::fromUtf8("•");
			cellTxt = cellTxt.replace(' ' ,ch);

			ch = QChar(0x2192);	//--> такая длинная стрелка...
			cellTxt = cellTxt.replace('\t' ,ch);

		} else {
			cellTxt = cellTxt.replace('\n' ,ch);
			/// Виснет. разобраться...
//			ch = QString::fromUtf8("    ");			cellTxt = cellTxt.replace('\t' ,ch);
		}


		int fullLen = cellTxt.length(), alreadyLen = 0;


		uoTextBoundary* textTPoint = _textProp->_textBoundary;
		if (!textTPoint)
			str = cellTxt;
		while(textTPoint && fullLen>0){
			tmpStr = cellTxt.mid(alreadyLen, textTPoint->_charCount);
			str.append(tmpStr);
			fullLen = fullLen - textTPoint->_charCount;
			alreadyLen = alreadyLen + textTPoint->_charCount;
			if (textTPoint->_textBoundary){
				str.append(QChar::LineSeparator);
				textTPoint = textTPoint->_textBoundary;
			}
		}
	}
	return str;
}


/// Установить текст. Надо гарантировать наличие структуры _textProp;
void uoCell::setText(QString text, uoReportDoc* doc)
{
	if (provideTextProp(doc, true))
		_textProp->_text = text;
}

void uoCell::setMaxRowLength(qreal len, uoReportDoc* doc)
{
	if (provideTextProp(doc, true))
		_textProp->m_maxRowLen = len;
}

qreal uoCell::getMaxRowLength()
{
	if (_textProp)
		return _textProp->m_maxRowLen;
	return 0.0;
}


/// установить выравнивание текста в ячейке
void uoCell::setAlignment(const uoVertAlignment& va, const uoHorAlignment& ha, const uoCellTextBehavior& tb, uoReportDoc* doc)
{
	if (provideTextProp(doc, true)) {
		_textProp->_horAlignment = ha;
		_textProp->_vertAlignment = va;
		_textProp->_behavior = tb;
	}
}

/// вернуть флаг выравнивания текста в ячейке для функции drawText, с флагами Qt
int uoCell::getAlignment()
{
	int flags = 0;
	if (_textProp) {
		switch (_textProp->_vertAlignment){
			case uoVA_Top:{	flags |= Qt::AlignTop; break; }
			case uoVA_Bottom:{	flags |= Qt::AlignBottom ; break; }
			case uoVA_Center:{	flags |= Qt::AlignVCenter ; break; }
			default:
			break;
		}
		switch (_textProp->_horAlignment){
			case uoHA_Left:{	flags |= Qt::AlignLeft ; break; }
			case uoHA_Right:{	flags |= Qt::AlignRight  ; break; }
			case uoHA_Center:{	flags |= Qt::AlignHCenter ; break; }
			default:
			break;
		}
		switch (_textProp->_behavior){
			case uoCTB_Transfer:{
				flags |= Qt::TextWordWrap;
				flags |= Qt::TextExpandTabs ;
				break;
			}
			default:	{
				break;
			}
		}
	}
	return flags;
}

/// Вернуть поведение текста.
uoCellTextBehavior 	uoCell::getTextBehavior()
{
	if (_textProp){
		return _textProp->_behavior;
	}
	return uoCTB_Auto;
}

uoHorAlignment 	uoCell::getAlignmentHor()
{
	if (_textProp){
		return _textProp->_horAlignment;
	}
	return uoHA_Left;
}
uoVertAlignment uoCell::getAlignmentVer()
{
	if (_textProp){
		return _textProp->_vertAlignment;
	}
	return uoVA_Top;
}



/// Отдаем фонт сразус размером
QFont* uoCell::getFont(uoReportDoc* doc, bool needCreate)
{
	if (!doc)
		return NULL;
	if (!_textProp){
		if (!needCreate) {
			return NULL;
		}
		_textProp = doc->getNewTextProp();
		if (!_textProp)
			return NULL;

	}
	QFont* font = doc->getFontByID(_textProp->_fontID);
	if (font){
		font->setPointSize(_textProp->_fontSize);
		font->setBold(_textProp->_fontBold);
		font->setItalic(_textProp->_fontItalic);
	}
	return font;
}

/// Получить размер шрифта..
int	uoCell::getFontSize()
{
	if (!_textProp)
		return 0;
	return _textProp->_fontSize;
}
/// Получить ID шрифта
int	uoCell::getFontId(){
	if (!_textProp)
		return 0;
	return _textProp->_fontID;
}


/// Извлекаем шрифт из документа по ID
const QColor*  uoCell::getFontColor(uoReportDoc* doc)
{
	if (!doc || !_textProp)
		return NULL;
	return doc->getColorByID(_textProp->_fontColID);
}

int uoCell::getFontColorId()
{
	if (!_textProp)
		return 0;
	return _textProp->_fontColID;
}

const
QColor*  uoCell::getBGColor(uoReportDoc* doc)
{
	if (!doc )
		return NULL;
	return doc->getColorByID(_bgColorID);
}

int uoCell::getBGColorId(){
	return _bgColorID;
}

/// Утилизируем точки выравнивания.
void uoCell::saveTrPoint(uoTextTrPointCash* cash)
{
	if (_textProp){
		if (_textProp->_textBoundary){
			cash->savePoint(_textProp->_textBoundary);
			_textProp->_textBoundary = NULL;
		}
	}
}

/// Применим порезку на строки, подготовленные с учетом размера шрифта.
void uoCell::applyTrPoint(uoTextTrPointCash* cash, const QStringList& listStr, uoReportDoc* doc)
{
	uoTextBoundary* textTPoint = NULL;
	int cntStr = listStr.size();
	if (cntStr>1){
		if (!_textProp) {
			_textProp = doc->getNewTextProp();
		}
		if (_textProp){
			textTPoint = cash->getTextTrPoint();

			int i = 0, strSize = 0;

			if (textTPoint){
				_textProp->_textBoundary = textTPoint;
				QString nextStr;
				while(textTPoint && i<cntStr){
					nextStr = listStr.at(i);
					strSize = nextStr.length();
					textTPoint->_charCount = strSize;
					textTPoint->_textBoundary = NULL;
					if ((i+1)<cntStr){
						textTPoint->_textBoundary = cash->getTextTrPoint();
						textTPoint = textTPoint->_textBoundary;
					}
					i = i + 1;
				}
			}
		}
	}
}


///============================================
/// uoRow uoRow uoRow uoRow uoRow uoRow

uoRow::uoRow(int nom)
	:_number(nom)
{
	_cellLast =_cellFirst = 0;
}
uoRow::~uoRow(){
}

/// Получить ячейку.
uoCell* uoRow::getCell(int posX, bool needCreate){
	return getItem(posX, needCreate);
}

/// Функция вызывается после создания нового итема. Возможно пригодится для ундо/редо.
void uoRow::onDeleteItem(uoCell* delItem)
{
	if (delItem)
		delItem->clear();
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

void uoRowsDoc::onDeleteItem(uoRow* delItem)
{
	if(delItem)
		delItem->clear();
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

///// Получить ячейку по строке/колонке.
//uoCell* uoRowsDoc::getCell(int nmRow, int nmCol,bool needCreate)
//{
//	uoRow* row = getRow(nmRow, needCreate);
//	if (!row)
//		return NILL;
//	return row->getCell(nmCol, needCreate);
//
//}







} //namespace uoReport
