/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "QDebug"
#include "QtGui"
#include "uoReportDocBody.h"
#include "uoReportUndo.h"


namespace uoReport {


//uoHeaderScale::uoHeaderScale()	: uoNumVector2<uoRptNumLine>()
uoHeaderScale::uoHeaderScale()	: uoNumVector<uoRptNumLine>()
{
	m_def_size = -1;
}

uoHeaderScale::~uoHeaderScale(){
	//clear(); // а надо тут вызывать??????
}

/// Получение признака спрятанности итема
bool uoHeaderScale::getHide(const int& nom ){
	if (getMaxNo() == 0)
		return false;
	if (m_map->isEmpty() || m_map->count() < nom)
		return false;
	if (!findItem(nom)){
		return false;
	} else {
		uoRptNumLine* item = m_map->at(nom-1);
		if (item) {
			return item->hiden();
		}
	}
	return false;
}

/// Установка признака спрятанности итема
void uoHeaderScale::setHide(const int& nom, const bool& hide){
	if (!findItem(nom, true)){
		return;
	} else {
		uoRptNumLine* item = m_map->at(nom-1);
		if (item) {
			item->setHiden(hide);
			return;
		}
	}
	return;
}

/// Получение свойства фиксированности размера строки
bool uoHeaderScale::getFixed(const int& nom )
{
	if (getMaxNo() == 0)
		return false;

	if (!findItem(nom)){
		return false;
	} else {
		uoRptNumLine* item = m_map->at(nom-1);
		if (item) {
			return item->fixed();
		}
	}
	return false;

}

/// Установка/снятие свойства фиксированности
void uoHeaderScale::setFixed(const int& nom, const bool& fix){
	if (!findItem(nom, true)){
		return;
	} else {
		uoRptNumLine* item = m_map->at(nom-1);
		if (item) {
			item->setFixed(fix);
			return;
		}
	}
	return;
}

/// Скопировать из другого.
void uoHeaderScale::copyFrom(uoHeaderScale* fromSrc, int startNo /*= -1*/, int endNo /*= -1*/, int colOffset )
{
	if (fromSrc->isEmpty())
		return;
	int startNoSrc = fromSrc->getMinNo(), endNoSrc = fromSrc->getMaxNo();
	uoRptNumLine* item = NULL;

	for (int i = startNoSrc; i <= endNoSrc; i++){
		if (startNo == -1 && endNo == -1) {
		} else if (i >= startNo && i <= endNo ){
		} else {
			continue;
		}

		item = fromSrc->getItem(i, false);
		if (item){
			setSize(i+colOffset, item->size());
			setHide(i+colOffset, item->hiden());
			setFixed(i+colOffset, item->fixed());
		}
	}
}

bool uoHeaderScale::copyFrom(uoHeaderScale* fromSrc, QList<int>& listRc, int colOffset)
{
	bool retVal = true;
	if (listRc.isEmpty())
		return true;
	QList<QPoint*> listTo;
	if (uorRangesExtract(listRc,listTo)){
		QPoint* point;
		foreach ( point, listTo )
		{
			copyFrom(fromSrc, point->x(), point->y(), colOffset);
		}
		uorRangesClear(listTo);
	}
	return retVal;
}

/// сохраняем итемы...
bool uoHeaderScale::onStoreItems(uoReportLoader* loader, QList<int>* list){

	detachIter();
	if (m_map->isEmpty())
		return true;
	uoRptNumLine* item = NULL;
	//QMap<int, uoRptNumLine*>::iterator itLst = m_map->begin();
	QVector<uoRptNumLine*>::iterator itLst = m_map->begin();
	while(itLst != m_map->end()) {
		item = *itLst;
		if (item) {
			if (list){
				if (list->contains(item->number()))
					loader->saveScaleItem(item);
			} else {
				loader->saveScaleItem(item);
			}
		}
		itLst++;
	}
	return true;
}


/// Вывод в текстовый файл отладочной информации.
void uoHeaderScale::printToDebug()
{
	detachIter();
	if (m_map->isEmpty())
		return;
	uoRptNumLine* item = NULL;
//	QLinkedList<uoRptNumLine*>::iterator itLst = m_map->begin();
	//QMap<int, uoRptNumLine*>::iterator itLst = m_map->begin();
	QVector<uoRptNumLine*>::iterator itLst = m_map->begin();
	while(itLst != m_map->end()) {
		item = *itLst;

		if (item);

		itLst++;
	}
}

void uoHeaderScale::test()
{
	qDebug() << "uoHeaderScale::test";
	setDefSize(200);
	setSize(1, (uorNumber)0.0);
	setSize(2, (uorNumber)0.0);
	setSize(2, (uorNumber)120.5);
	setSize(9, (uorNumber)120.5);
	printToDebug();
	getSize(4);
	setHide(4, true);
	printToDebug();
	getSize(5);
	setHide(5, true);
	printToDebug();
	clear();
	qDebug() << "uoHeaderScale::test-2";
	setSize(1, (uorNumber)0.0);
	setSize(9, (uorNumber)120.5);
	getSize(2);
	setHide(2, true);
	printToDebug();

	clear();
	qDebug() << "uoHeaderScale::test-3";
	setSize(9, (uorNumber)3.0);
	setSize(1, (uorNumber)45.3);
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
uorNumber uoHeaderScale::getSize(const int& nom, bool isDef){
	if (m_map->size() == 0)
		return m_def_size;
	if (findItem(nom)) {
		uoRptNumLine* item = m_map->at(nom-1);
		if (item)
			return item->size(isDef);
	}
	return m_def_size;
}

/// Установить размер итема, а если итем не существует, создать и установить
bool	uoHeaderScale::setSize(const int& nom, const uorNumber& size, bool isDef){
	if (!findItem(nom, true)){
		return false;
	} else {
		uoRptNumLine* item = m_map->at(nom-1);
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
		crItem->setSize(m_def_size);
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
	m_fCount = 0;
}

uoReportDocFontColl::~uoReportDocFontColl()
{
	clear();
}

/// чистка колекции шрифтов
void uoReportDocFontColl::clear()
{
	QFont* font = 0;
	for (int i = 0; i< _fontList.count(); i++)	{
		delete _fontList.at(i);
	}
	//while (!_fontList.isEmpty()) {		delete _fontList.takeFirst();	}
	_fontList.clear();
	m_fCount = 0;
}

/// Получить фонт для отрисовки
QFont* uoReportDocFontColl::getFont(const int& nmFont)
{
	if (m_fCount == 0)
		return NULL;
	if (nmFont < _fontList.size())
		return _fontList.at(nmFont);
	return NULL;
}

/// чиста поиск.
int uoReportDocFontColl::findFont(const QString& fontName)
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
int uoReportDocFontColl::addFont(const QString& fontName)
{
	int retVal = findFont(fontName);
	if (retVal == -1){
		// есть возможность поработать с фонтом на свое усмотрение.
		QFont* font = new QFont(fontName);
		if (font){
			_fontList.append(font);
			m_fCount = _fontList.size();
			retVal = _fontList.size()-1;
		}
	}
	return retVal;
}

/// Получить ID фонта. Именно тут "Заводятся" новые фонты
int uoReportDocFontColl::getFontId(const QString& fontName)
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
	return m_fCount;
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
/// Очистка форматов
void uoCell::clearFormat(uoReportDoc* doc)
{
	Q_ASSERT(false);
}

/// Взять текст.
QString& uoCell::getText(){
	return m_text;
}

/// Обеспеспечивает свойства ячейки.
///\todo Думаю надо сразу все свойства инициализировать из этого места и переименовать в provideAllProps, ы? угу...
bool uoCell::provideAllProps(uoReportDoc* doc, bool needCreate /* = false*/)
{
	if (m_textProp && m_borderProp)
		return true;
	if (doc && needCreate){
		m_textProp = doc->getNewTextProp();
		m_borderProp = doc->getNewBordProp();
		if (m_textProp && m_borderProp){
			return true;
		}
	}
	return false;
}
/// Обеспеспечивает структуру для объединения
bool uoCell::provideJoinProp(uoReportDoc* doc)
{
	if (m_ceelJoin)
		return true;
	if (doc){
		m_ceelJoin = doc->getCellJoinStruct();
		if (m_ceelJoin){
			return true;
		}
	}
	return false;

}

/// Вернуть текст, разделеленный согласно форматированию символом, специально для печати.
QString uoCell::getTextWithLineBreak(bool drawInv)
{
	QString str;
	static const QString str2192 = QChar(0x2192);	//--> такая длинная стрелка...
	static const QString str1 = QString::fromUtf8("¶");
	static const QString str2 = QString::fromUtf8("•");
	static const QString str3 = " ";
	if (m_textProp){
		QString cellTxt = m_text;
		QString tmpStr;

		if (drawInv) {
			cellTxt = cellTxt.replace('\n' ,str1);

			cellTxt = cellTxt.replace(' ' ,str2);

			if (m_textProp->m_alignFlags & uoCTF_ContainsTab)
				cellTxt = cellTxt.replace('\t' ,str2192);

		} else {
			if (m_textProp->m_alignFlags & uoCTF_ContainsLineBreak)
				cellTxt = cellTxt.replace('\n' ,str3);

			if (m_textProp->m_alignFlags & uoCTF_ContainsTab)
				cellTxt = cellTxt.replace('\t', str3);
		}


		int fullLen = 0, alreadyLen = 0;


		uoTextBoundary* textTPoint = m_textBoundary;
		if (!textTPoint) {
			str = cellTxt;
		} else {
			fullLen = cellTxt.length();
		}
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
void uoCell::setText(const QString& text, uoReportDoc* doc)
{
	if (provideAllProps(doc, true))
		m_text = text;
}

QRect uoCell::getCellJoinRect() const
{
	QRect retVarRC;
	if (isUnionHas()){
		retVarRC = m_ceelJoin->m_cellRect;
		if (retVarRC.isEmpty() || retVarRC.top() == 0){
			m_ceelJoin->m_cellRect = QRect(m_colNo,m_rowNo, m_ceelJoin->m_col, m_ceelJoin->m_row);
			retVarRC = m_ceelJoin->m_cellRect;
		}
	}
	return retVarRC;
}

void uoCell::setMaxRowLength(const uorNumber& len, uoReportDoc* doc)
{
	if (provideAllProps(doc, true))
		m_maxRowLen = len;
}

void uoCell::copyFrom(uoCell* fromSrc, uoReportDoc* doc, uoReportDoc* docSrc, int rowNo)
{
	uoReportUndo* undoManager = doc->getUndoManager();
	bool collectChanges = doc->isCollectChanges();
	if (!undoManager){
		collectChanges = false;
	}

	QString oldText;
	if (collectChanges){
		oldText = fromSrc->getText();
		undoManager->doTextChange(oldText, rowNo, fromSrc->number());
	}
	m_text 		= fromSrc->m_text;
	m_textDecode = fromSrc->m_textDecode;

	if (m_borderProp)
		m_borderProp->copyFrom(fromSrc->m_borderProp);
	if (m_textProp)
		m_textProp->copyFrom(fromSrc->m_textProp);
	/**
		\todo - надо убедиться, что шрифты (наименования есть)
		ну и цвета скопировать.
	*/
}



/// установить выравнивание текста в ячейке
void uoCell::setAlignment(const uoVertAlignment& va, const uoHorAlignment& ha, const uoCellTextBehavior& tb, uoReportDoc* doc)
{
	if (provideAllProps(doc, true)) {
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
				flags |= Qt::TextSingleLine;
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
		return m_textProp->m_textType;
	}
	return uoCTT_Unknown;
}

uorTextDecor* uoCell::getTextProp(uoReportDoc* doc, bool needCreate)
{
	if (m_textProp)
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



/// Отдаем фонт сразу с размером
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
		if (m_textProp->m_fontU>=0)
			font->setUnderline(m_textProp->m_fontU);
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
	return doc->getColorByID(getBGColorId());
}

int uoCell::getBGColorId(){
	if (m_textProp)
		return m_textProp->m_BgCol;
	return -1;
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
/**	Возвращает true если ячейка является частью объединения,
	в зависимости от параметра "basic" = true
	пс. Кчему тут строка не помню..
*/
bool uoCell::isPartOfUnion(const int& row, const bool& basic) const
{
	bool rewtVal = false;
	if (m_ceelJoin){

		if (m_ceelJoin->m_JoinType != uoCJT_Unknown)
		{
			rewtVal = true;
			if (basic && m_ceelJoin->m_JoinType == uoCJT_BackPoint)
				rewtVal = false;
		}

	}
	return rewtVal;
}

///	Ячейка задействована в объединении.
bool uoCell::isUnionHas() const
{
	bool rewtVal = false;
	if (m_ceelJoin){
		if (m_ceelJoin->m_JoinType != uoCJT_Unknown)
		{
			rewtVal = true;
		}
	}
	return rewtVal;
}

/// Устанавливаем объединение.
void uoCell::setCellJoin(uoCellJoin* cellJ, uoReportDoc* doc){
	Q_ASSERT(!m_ceelJoin); /// не должно быть объединено.
	Q_ASSERT(cellJ);
	m_ceelJoin = cellJ;
}
/// Разрываем объединение, солим старое :)
uoCellJoin* uoCell::deleteCellJoin()
{
	Q_ASSERT(m_ceelJoin); /// не должно быть объединено.
	uoCellJoin* retVal = m_ceelJoin;
	m_ceelJoin = 0;
	return retVal;
}

/// Какой тип объединения.
uoCellsJoinType uoCell::joinType() const
{
	if (m_ceelJoin)
		return m_ceelJoin->m_JoinType;
	return uoCJT_Unknown;
}

uoCellsJoinType uoCell::unionType() const
{
	return joinType();
}
/// Это первая ячейка в объединении? Для прорисовки остальные не нужны.
bool uoCell::isFirstOfUnionCell() const
{
	if (m_ceelJoin && (m_ceelJoin->m_JoinType == uoCJT_Normal || m_ceelJoin->m_JoinType == uoCJT_TextToCol))
		return true;
	return false;
}

int uoCell::unionRow() const
{
	if (m_ceelJoin && (m_ceelJoin->m_JoinType == uoCJT_Normal || m_ceelJoin->m_JoinType == uoCJT_TextToCol))
		return m_ceelJoin->m_row;
	return 0;
}
int uoCell::unionCol() const
{
	if (m_ceelJoin && (m_ceelJoin->m_JoinType == uoCJT_Normal || m_ceelJoin->m_JoinType == uoCJT_TextToCol))
		return m_ceelJoin->m_col;
	return 0;
}


/// Совершенно не нужно обрабатывать ячейки, задействованные как uoCJT_BackPoint в объединении
bool uoCell::skipVisitor()
{
	if (m_ceelJoin){
		if (m_ceelJoin->m_JoinType == uoCJT_BackPoint)
			return true;
		return false;
	}
	return false;
}

QPoint uoCell::getFirstUnionCellPoint(const int rowNo) const
{
	QPoint pt = QPoint(m_colNo,rowNo);
	if (joinType() == uoCJT_BackPoint){
		pt.setX(m_ceelJoin->m_col);
		pt.setY(m_ceelJoin->m_row);
	}

	return pt;
}

void uoCell::setText(const QString& text)
{
	m_text = text;
	if (m_textProp){
		m_textProp->m_alignFlags = 0;
		if (text.contains('\t'))
			m_textProp->m_alignFlags |= uoCTF_ContainsTab;
		if (text.contains('\n'))
			m_textProp->m_alignFlags |= uoCTF_ContainsLineBreak;
	}
}

QString& uoCell::text(){
	return m_text;
}


void uoCell::drawBorder(uoPainter& painter, uorPoint& pt1, uorPoint& pt2) const
{
	bool horiz = (pt1.y() == pt2.y()) ? true : false;

	if (horiz){
		pt1.setY(pt1.y()-(uorNumber)0.5);
		pt2.setY(pt2.y()-(uorNumber)0.5);
		painter.drawLine(pt1, pt2);
		pt1.setY(pt1.y()+(uorNumber)1.5);
		pt2.setY(pt2.y()+(uorNumber)1.5);
		painter.drawLine(pt1, pt2);
	} else {
		pt1.setX(pt1.x()-(uorNumber)0.5);
		pt2.setX(pt2.x()-(uorNumber)0.5);
		painter.drawLine(pt1, pt2);
		pt1.setX(pt1.x()+(uorNumber)1.5);
		pt2.setX(pt2.x()+(uorNumber)1.5);
		painter.drawLine(pt1, pt2);
	}
}


/// Прорисуем бордюр
void uoCell::drawBorder(uoPainter& painter, uorRect& rectCell) const
{
	if (!m_borderProp)
		return;
	QPen oldPen = painter.pen();
	QPen newPen;
	newPen.setWidthF(1);
	Qt::PenStyle pStyle;
	uoCellBorderType bt;
	uorPoint pt1, pt2;
	for (int i=0; i<4; i++)	{
		bt = m_borderProp->m_bordType[i];
		if (bt == uoCBT_Unknown)
			continue;

		pStyle = (Qt::PenStyle)bt;
		newPen.setStyle(pStyle);
		painter.setPen(newPen);
		switch (i){
			case 0: // левый
			{
				pt1 = rectCell.bottomLeft();
				pt2 = rectCell.topLeft();
				break;
			}
			case 1: // верхний
			{
				pt1 = rectCell.topLeft();
				pt2 = rectCell.topRight();
//				painter.drawLine(pt1, pt2);
				break;
			}
			case 2: // правый
			{
				pt1 = rectCell.topRight();
				pt2 = rectCell.bottomRight();
//				painter.drawLine(pt1, pt2);
				break;
			}
			case 3: // низ
			{
				pt1 = rectCell.bottomRight();
				pt2 = rectCell.bottomLeft();
//				painter.drawLine(pt1, pt2);
				break;
			}
			default:
			{
				continue;
				break;
			}
		}
		if (bt != uoCBT_CustomDashLine){
			painter.drawLine(pt1, pt2);
		} else {
			drawBorder(painter, pt1, pt2);
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
	:m_number(nom)
{
	m_cellLast =m_cellFirst = 0;
	m_lengthMaxOver = m_lengthMaxToRight = m_lengthMaxToLeft = m_lengthFromCell = uorNumberNull;
	m_unionCount = 0;

}
uoRow::~uoRow(){
}

/// Получить ячейку.
uoCell* uoRow::getCell(int colNo, bool needCreate){
	return getItem(colNo, needCreate);
}

/// Копирование строки
//void copyFrom(uoRow* fromSrc, uoReportDoc* docThere, uoReportDoc* docSrc, int startColNo = -1, int endColNo = -1,int rowOffset = 0, int colOffset = 0);
void uoRow::copyFrom(uoRow* fromSrc, uoReportDoc* docThere, uoReportDoc* docSrc, int startColNo, int endColNo, int rowOffset, int colOffset)
{
	if (fromSrc->isEmpty())
		return;
	int startNoSrc = fromSrc->getMinNo(), endNoSrc = fromSrc->getMaxNo();
	int rowNo = fromSrc->number();

	uoCell* cellFrom = NULL;
	uoCell* cellThere = NULL;

	for (int i = startNoSrc; i <= endNoSrc; i++){
		if (startColNo == -1 && endColNo == -1) {
		} else if (i >= startColNo && i <= endColNo ){
		} else {
			continue;
		}

		cellFrom = fromSrc->getItem(i, false);
		if (cellFrom){
			cellThere = docThere->getCell(rowNo+rowOffset, i+colOffset , true, true);
			cellThere->copyFrom(cellFrom, docThere, docSrc, rowNo+rowOffset);
		}
	}
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
void uoRow::saveItems(uoReportLoader* loader, uoReportSelection* selection){
	detachIter();
	if (m_map->isEmpty())
		return;

	uoCell* item = NULL;

	uorSelectionType selMode = uoRst_Unknown;

	bool saveThisCell = true;

	if (selection)
		selMode = selection->selectionType();

	//QMap<int, uoCell*>::iterator itLst = m_map->begin(); // uoNumVector
	QVector<uoCell*>::iterator itLst = m_map->begin(); // uoNumVector2
	while(itLst != m_map->end()) {
		item = *itLst;
		saveThisCell = true;
		if (selection) {
			saveThisCell = false;

			if (selMode == uoRst_Column || selMode == uoRst_Columns){
				saveThisCell = selection->isColSelect(item->number());
			} else if (selMode == uoRst_Document) {
				saveThisCell = true;
			} else if (selMode == uoRst_Unknown) {
				saveThisCell = selection->isCurrentCell(number(), item->number());;
			} else {
				saveThisCell = selection->isCellSelect(number(), item->number());
			}
		}
		if (saveThisCell && item)
			loader->saveCell(item);
		itLst++;
	}
}

/// Получить перечень номеров ячеек строки.
QList<int> uoRow::getItemNumList()
{
	QList<int> listNo;
	if (m_map->isEmpty())
		return listNo;
	uoCell* item = NULL;
	//QMap<int, uoCell*>::iterator itLst = m_map->begin(); // uoNumVector2
	QVector<uoCell*>::iterator itLst = m_map->begin(); // uoNumVector
	while(itLst != m_map->end()) {
		item = *itLst;
		if (item)
			listNo.append(item->number());
		itLst++;
	}
	return listNo;
}

/// uoRowsDoc==========================================
//uoRowsDoc::uoRowsDoc(): uoNumVector2<uoRow>(){
uoRowsDoc::uoRowsDoc(): uoNumVector<uoRow>(){
	clearLastRow();
}
uoRowsDoc::~uoRowsDoc(){
	clearLastRow();
}
void uoRowsDoc::clearLastRow()
{
	m_lastFRow = 0;
	m_lastFRowNo = 0;
}
void uoRowsDoc::onDeleteItem(uoRow* delItem)
{
	if(delItem)
		delItem->clear();
	clearLastRow();
}
void uoRowsDoc::onCreateItem(uoRow* crItem){
	clearLastRow();
}

/// Найти ячейку, если она существует, и оздать в случае необходимости (параметр needCreate).
uoCell* uoRowsDoc::getCell(const int& rowNo, const int& colNo, bool needCreate){
	uoRow* row = 0;
	if (m_lastFRowNo != 0 && m_lastFRowNo == rowNo){
		row = m_lastFRow;
		m_lastFRowNo = rowNo;
	} else {
		row = getItem(rowNo, needCreate);
		m_lastFRow = row;
	}

	if (row) {
		uoCell* cell = row->getItem(colNo, needCreate);
		if (cell)
			cell->m_rowNo = rowNo;
		return cell;
	}
	return NULL;
}


/// Получить текст ячейки
QString uoRowsDoc::getText(int rowNo, int colNo, uorCellTextType type /*= uorCTT_Text*/){
	QString retVal;

	uoCell* cell = getCell(rowNo, colNo, false);
	if (cell){
		if (type == uorCTT_Text) {
			retVal = cell->getText();
		} else if (type == uorCTT_Decode) {
			retVal = cell->m_textDecode;
		}
	}
	return retVal;
}

/// Установить текст в ячейку.
bool uoRowsDoc::setText(const int rowNo, const int colNo, const QString& text, uorCellTextType type /*= uorCTT_Text*/){
	bool isFind = false;

	uoCell* cell = getCell(rowNo, colNo, true);
	if (cell){
		if (type == uorCTT_Text){
			cell->setText(text, m_doc);
		} else if (type == uorCTT_Decode){
			cell->m_textDecode = text;
		}

		isFind = true;
	}
	return isFind;
}

/// Копирование строк из другого документа...
void uoRowsDoc::copyFrom(uoRowsDoc* fromSrc, int startRowNo, int endRowNo, int startColNo, int endColNo, int rowOffset, int colOffset){

	if (fromSrc->isEmpty())
		return;
	int startNoSrc = fromSrc->getMinNo(), endNoSrc = fromSrc->getMaxNo();
//	int startColNoSrc = -1, endColNoSrc = -1;

	uoRow* row = NULL;
	uoRow* rowThere = NULL;

	uoReportDoc* docThere = m_doc;
	uoReportDoc* docSrc = fromSrc->m_doc;

	for (int i = startNoSrc; i <= endNoSrc; i++){
		if (startRowNo == -1 && endRowNo == -1) {
		} else if (i >= startRowNo && i <= endRowNo ){
		} else {
			continue;
		}

		row = fromSrc->getItem(i, false);
		if (row){
			rowThere = getRow(i+rowOffset, true);
			if (rowThere){
				rowThere->copyFrom(row, docThere, docSrc, startColNo, endColNo,rowOffset ,colOffset);
			}
		}
	}
}

void uoRowsDoc::copyFrom(uoRowsDoc* fromSrc, uoRptHeaderType& rht, QList<int> listRc, int rowOffset, int colOffset)
{
	if (listRc.isEmpty())
		return;

	QList<QPoint*> listTo;
	if (uorRangesExtract(listRc,listTo)){
		QPoint* point;
		foreach ( point, listTo )
		{
			if (rht == uorRhtColumnHeader){
				copyFrom(fromSrc,-1,-1, point->x(), point->y(), rowOffset, colOffset);
			} else if (rht == uorRhtRowsHeader){
				copyFrom(fromSrc, point->x(), point->y(),-1,-1, rowOffset, colOffset);
			}
		}
		uorRangesClear(listTo);
	}
}

/// Сохранение строк документа.
void uoRowsDoc::saveItems(uoReportLoader* loader, uoReportSelection* selection)
{

	detachIter();
	if (m_map->isEmpty())
		return;
	bool saveThisRow = true;

	uorSelectionType selMode = uoRst_Unknown;

	QRect sbRect; // если выделение миксированное, то нет смысла сливать туда все троки.

	if (selection) {
		selMode = selection->selectionType();
		if (selMode == uoRst_Cell || selMode == uoRst_Cells || selMode == uoRst_Mixed){
 			sbRect = selection->getSelectionBound();
		} else if (selMode == uoRst_Unknown){
			sbRect = QRect(selection->currentCell(), selection->currentCell());
		}
	}

	int rowNum = -1;

	uoRow* item = NULL;
//	QMap<int,uoRow*>::iterator itLst = m_map->begin(); //uoNumVector2
	QVector<uoRow*>::iterator itLst = m_map->begin(); // uoNumVector
	while(itLst != m_map->end()) {
		item = *itLst;
		if (item) {
			// может быть пустым...
			rowNum = item->number();
			saveThisRow = true;
			if (selection){
				if (selMode == uoRst_Row || selMode == uoRst_Rows)
				{
					saveThisRow = selection->isRowSelect(rowNum);
				} else 	if (selMode == uoRst_Cell || selMode == uoRst_Cells || selMode == uoRst_Mixed || selMode == uoRst_Unknown){
					saveThisRow = false;
					if (sbRect.top()<= rowNum && rowNum <=sbRect.bottom())
						saveThisRow = true;
				}


			}
			if (saveThisRow) {
				loader->saveRowItemStart(item);
				item->saveItems(loader, selection);
				loader->saveRowItemEnd();
			}
		}
		itLst++;
	}
}

/// Получить строку по номеру.
uoRow* uoRowsDoc::getRow(int nmRow, bool needCreate)
{
	return getItem(nmRow, needCreate);
}

} //namespace uoReport
