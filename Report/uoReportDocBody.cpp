/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "QDebug"
#include "uoReportDocBody.h"

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
		point->m_textBoundary = NULL;
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
			next = next->m_textBoundary;
		}
	}
}


//=========uoTextTrPointCash==============
//=========================================

void uoCell::clear()
{

}

/// Взять текст.
QString uoCell::getText(){
	return m_text;
}

bool uoCell::provideTextProp(uoReportDoc* doc, bool needCreate /* = false*/)
{
	if (m_textProp)
		return true;
	if (doc && needCreate){
		m_textProp = doc->getNewTextProp();
		if (m_textProp)
			return true;
	}
	return false;
}

/// Вернуть текст, разделеленный согласно форматированию символом, специально для печати.
QString uoCell::getTextWithLineBreak(bool drawInv)
{
	QString str;
	if (m_textProp){
		QString cellTxt = m_text;
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


		uoTextBoundary* textTPoint = m_textBoundary;
		if (!textTPoint)
			str = cellTxt;
		while(textTPoint && fullLen>0){
			tmpStr = cellTxt.mid(alreadyLen, textTPoint->_charCount);
			str.append(tmpStr);
			fullLen = fullLen - textTPoint->_charCount;
			alreadyLen = alreadyLen + textTPoint->_charCount;
			if (textTPoint->m_textBoundary){
				str.append(QChar::LineSeparator);
				textTPoint = textTPoint->m_textBoundary;
			}
		}
	}
	return str;
}


/// Установить текст. Надо гарантировать наличие структуры m_textProp;
void uoCell::setText(QString text, uoReportDoc* doc)
{
	if (provideTextProp(doc, true))
		m_text = text;
}

void uoCell::setMaxRowLength(qreal len, uoReportDoc* doc)
{
	if (provideTextProp(doc, true))
		m_maxRowLen = len;
}

qreal uoCell::getMaxRowLength()
{
	return m_maxRowLen;
}


/// установить выравнивание текста в ячейке
void uoCell::setAlignment(const uoVertAlignment& va, const uoHorAlignment& ha, const uoCellTextBehavior& tb, uoReportDoc* doc)
{
	if (provideTextProp(doc, true)) {
		m_textProp->m_horTAlignment = ha;
		m_textProp->m_vertTAlignment = va;
		m_textProp->m_TextBehavior = tb;
	}
}

/// вернуть флаг выравнивания текста в ячейке для функции drawText, с флагами Qt
int uoCell::getAlignment()
{
	int flags = 0;
	if (m_textProp) {
		switch (m_textProp->m_vertTAlignment){
			case uoVA_Top:{	flags |= Qt::AlignTop; break; }
			case uoVA_Bottom:{	flags |= Qt::AlignBottom ; break; }
			case uoVA_Center:{	flags |= Qt::AlignVCenter ; break; }
			default:
			break;
		}
		switch (m_textProp->m_horTAlignment){
			case uoHA_Left:{	flags |= Qt::AlignLeft ; break; }
			case uoHA_Right:{	flags |= Qt::AlignRight  ; break; }
			case uoHA_Center:{	flags |= Qt::AlignHCenter ; break; }
			default:
			break;
		}
		switch (m_textProp->m_TextBehavior){
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
	if (m_textProp){
		return m_textProp->m_TextBehavior;
	}
	return uoCTB_Auto;
}
uoCellTextType	uoCell::getTextType()
{
	if (m_textProp){
		return m_textProp->_textType;
	}
	return uoCTT_Unknown;
}

uorTextDecor* uoCell::getTextProp(uoReportDoc* doc, bool needCreate)
{
	if (!m_textProp)
		return m_textProp;
	return doc->getDefaultTextProp();
}

uoHorAlignment 	uoCell::getAlignmentHor()
{
	if (m_textProp){
		return m_textProp->m_horTAlignment;
	}
	return uoHA_Left;
}
uoVertAlignment uoCell::getAlignmentVer()
{
	if (m_textProp){
		return m_textProp->m_vertTAlignment;
	}
	return uoVA_Top;
}



/// Отдаем фонт сразус размером
QFont* uoCell::getFont(uoReportDoc* doc, bool needCreate)
{
	if (!doc)
		return NULL;
	if (!m_textProp){
		if (!needCreate) {
			return NULL;
		}
		m_textProp = doc->getNewTextProp();
		if (!m_textProp)
			return NULL;

	}
	QFont* font = doc->getFontByID(m_textProp->m_fontId);
	if (font){
		if (m_textProp->m_fontSz>0)
			font->setPointSize(m_textProp->m_fontSz);
		if (m_textProp->m_fontB>=0)
			font->setBold(m_textProp->m_fontB);
		if (m_textProp->m_fontI>=0)
			font->setItalic(m_textProp->m_fontI);
	}
	return font;
}

/// Получить размер шрифта..
int	uoCell::getFontSize()
{
	if (!m_textProp)
		return 0;
	return m_textProp->m_fontSz;
}
/// Получить ID шрифта
int	uoCell::getFontId(){
	if (!m_textProp)
		return 0;
	return m_textProp->m_fontId;
}


/// Извлекаем шрифт из документа по ID
const QColor*  uoCell::getFontColor(uoReportDoc* doc)
{
	if (!doc || !m_textProp)
		return NULL;
	return doc->getColorByID(m_textProp->m_fontCol);
}

int uoCell::getFontColorId()
{
	if (!m_textProp)
		return 0;
	return m_textProp->m_fontCol;
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
	if (m_textProp){
		if (m_textBoundary){
			cash->savePoint(m_textBoundary);
			m_textBoundary = NULL;
		}
	}
}

/// Применим порезку на строки, подготовленные с учетом размера шрифта.
void uoCell::applyTrPoint(uoTextTrPointCash* cash, const QStringList& listStr, uoReportDoc* doc)
{
	uoTextBoundary* textTPoint = NULL;
	int cntStr = listStr.size();
	if (cntStr>1){
		if (!m_textProp) {
			m_textProp = doc->getNewTextProp();
		}
		if (m_textProp){
			textTPoint = cash->getTextTrPoint();

			int i = 0, strSize = 0;

			if (textTPoint){
				m_textBoundary = textTPoint;
				QString nextStr;
				while(textTPoint && i<cntStr){
					nextStr = listStr.at(i);
					strSize = nextStr.length();
					textTPoint->_charCount = strSize;
					textTPoint->m_textBoundary = NULL;
					if ((i+1)<cntStr){
						textTPoint->m_textBoundary = cash->getTextTrPoint();
						textTPoint = textTPoint->m_textBoundary;
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

} //namespace uoReport
