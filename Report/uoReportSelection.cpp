/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "uoReportSelection.h"

namespace uoReport{

/**
	\class uoReportSelection - объект хранения и обработки выделения объектов в uoReportCtrl
	\brief uoReportSelection - объект хранения и обработки выделения объектов в uoReportCtrl

	см. описание в uoReportDescr.h
*/

uoReportSelection::uoReportSelection(QObject* parent)
	:QObject(parent)
{
	m_selRows 	= new QList<int>;
	m_selRowsColsTmp = new QList<int>;
	m_selCols = new QList<int>;
	m_selSpans 		= new QList<QRect*>;
	m_selSpansCache 	= new QList<QRect*>;

	m_selPoints 		= new QList<QPoint*>;
	m_selPointsCache = new QList<QPoint*>;


	m_startSelMode	= uoRst_Unknown;
	setSelectionMode(uoRst_Unknown);
	m_strartColRow	= -1;
	m_cellStart.setX(0);
	m_cellStart.setY(0);
	m_cellMidle.setX(0);
	m_cellMidle.setY(0);
}

uoReportSelection::~uoReportSelection()
{
	delete m_selRows;
	delete m_selRowsColsTmp;
	delete m_selCols;
	while (!m_selSpans->isEmpty())		delete m_selSpans->takeFirst();
	while (!m_selSpansCache->isEmpty())	delete m_selSpansCache->takeFirst();

	while (!m_selPoints->isEmpty())	delete m_selPoints->takeFirst();
	while (!m_selPointsCache->isEmpty())	delete m_selPointsCache->takeFirst();

	delete m_selSpans;
	delete m_selSpansCache;
}

/// Вычислить rct из 2-х точек.
bool uoReportSelection::calcRectFromPoints(QRect& rct, const QPoint& posStart, const QPoint& posEnd) const
{
	if ((posStart.x() <= 0) ||
		(posEnd.y() <= 0) ||
		(posStart.x() <= 0) ||
		(posEnd.y() <= 0) )
	{
		return false;
	}
	rct.setTop(qMin(posStart.y(),posEnd.y()));
	rct.setLeft(qMin(posStart.x(),posEnd.x()));

	rct.setBottom(qMax(posStart.y(),posEnd.y()));
	rct.setRight(qMax(posStart.x(),posEnd.x()));

	return true;
}

/// единая точка для установки сел-мода
void uoReportSelection::setSelectionMode(uorSelectionType sMode)
{
	if (m_selMode != sMode){
		const uorSelectionType selModeOld = m_selMode;
		const uorSelectionType selModeNew = sMode;
		m_selMode = sMode;
		emit onSelectonChange(selModeOld, selModeNew);
	}
}

/// Очистка всех даных по выделениям
void uoReportSelection::clearSelections(uorSelectionType exclude)
{
	setSelectionMode(uoRst_Unknown);
	m_startSelMode	= uoRst_Unknown;
	m_strartColRow	= -1;


	if (exclude != uoRst_Rows) {
		m_selRows->clear();
		m_selRowsColsTmp->clear();
	} else {
		setSelectionMode(uoRst_Rows);
	}
	if (exclude != uoRst_Columns) {
		m_selCols->clear();
	} else {
		setSelectionMode(uoRst_Columns);
	}

	if (exclude != uoRst_Cells) {
		m_cellStart.setX(0);
		m_cellStart.setY(0);
		m_cellMidle.setX(0);
		m_cellMidle.setY(0);
		m_rectCellsMidle.setTop(0);
		m_rectCellsMidle.setLeft(0);
		m_rectCellsMidle.setBottom(0);
		m_rectCellsMidle.setRight(0);

		while (!m_selSpans->isEmpty())
			m_selSpansCache->append(m_selSpans->takeFirst());

		while (!m_selPoints->isEmpty())
			m_selPointsCache->append(m_selPoints->takeFirst());

	} else {
		setSelectionMode(uoRst_Cells);
	}
}

/// Проверка на выделенность столбца
bool uoReportSelection::isColSelect(int nmCol)
{
	bool retVal = false;
	if (retVal = isDocumSelect()){
		return retVal;
	} else if (!m_selCols->isEmpty()) {
		retVal = m_selCols->contains(nmCol);
	}
	return retVal;
}

/// Проверка на выделенность строки
bool uoReportSelection::isRowSelect(int nmRow)
{
	bool retVal = false;
	if (retVal = isDocumSelect()){
		return retVal;
	}
	if (!m_selRows->isEmpty()) {
		retVal = m_selRows->contains(nmRow);
		if (retVal)
			return true;
	}
	if (!m_selRowsColsTmp->isEmpty()){
		retVal = m_selRowsColsTmp->contains(nmRow);
		if (retVal)
			return true;
	}
	return retVal;
}

/// Проверка на выделенность документа
bool uoReportSelection::isDocumSelect()
{
	if (m_selMode == uoRst_Document){
		return true;
	} else {
		return false;
	}
}

/// Проверка на выделенность ячейки.
bool uoReportSelection::isCellSelect(int nmRow, int nmCol)
{
	bool retVal = false;
	if (isDocumSelect() || isColSelect(nmCol) ||  isRowSelect(nmRow)) {
		return true;
	} else if(!m_rectCellsMidle.isEmpty()) {
		if (m_rectCellsMidle.contains(QPoint(nmCol,nmRow))){
			return true;
		}
	}


	if (!m_selSpans->isEmpty()) {
		QRect* rct = NULL;

		QList<QRect*>::const_iterator iter = m_selSpans->constBegin();
		while(iter != m_selSpans->constEnd()) {
			rct = *iter;
			if (rct) {
				if (rct->contains(nmCol, nmRow))
					return true;
			}
			iter++;
		}
	}
	if (!m_selPoints->isEmpty()) {
		QPoint* point = NULL;

		QList<QPoint*>::const_iterator iter = m_selPoints->constBegin();
		while(iter != m_selPoints->constEnd()) {
			point = *iter;
			if (point) {
				if (point->x() == nmCol &&  point->y() == nmRow)
					return true;
			}
			iter++;
		}
	}
	return retVal;
}

/**
	Получить правильный диапазон выделенных строк или столбцов и статус возврата.
*/
bool uoReportSelection::getTrueSectionsCR(uoRptHeaderType& rht, int& start, int& end)
{
	rht = rhtUnknown;
	start = -1;
	end = -1;

	bool retVal = false;
	switch(m_selMode)
	{
		case uoRst_Unknown:
		case uoRst_Document:
		case uoRst_Cell:
		case uoRst_Cells:
		case uoRst_Mixed:
		{
			return retVal;
		}
		case uoRst_Column:
		case uoRst_Columns:
		case uoRst_Row:
		case uoRst_Rows:
		{
			int rowLast = -1, rowCur;
			QList<int>* list = m_selRows;
			rht = rhtVertical;

			if (m_selMode == uoRst_Column || m_selMode == uoRst_Columns)
			{
				list = m_selCols;
				rht = rhtHorizontal;
			}

			if (list->isEmpty()) {
				retVal = false;

			} else if (list->size() == 1){
				retVal = true;
				end = list->at(0);
				start = end;

			} else {
				retVal = true;
				qSort(*list);
				QList<int>::const_iterator iter = list->constBegin();
				while(iter != list->constEnd()){
					rowCur = *iter;
					if (rowLast != -1){
						if (rowLast+1 != rowCur) {
							retVal = false;
							break;
						}
						end = rowCur;
					} else {
						start = rowCur;
					}
					rowLast = rowCur;
					iter++;
				}
			}
			break;
		}
		default:{
			break;
		}
	}
	if (!retVal)
	{
		rht = rhtUnknown;
		start = end = -1;
	}
	return retVal;

}

/**
	Определим, подходит ли выделение для операций с секциями.
	правильное выделение должно быть строка|Строки_подряд|Столбец|Столбцы_подряд
*/
bool uoReportSelection::isTrueForSections()
{
	bool retVal = false;
	switch(m_selMode)
	{
		case uoRst_Unknown:
		case uoRst_Document:
		case uoRst_Cell:
		case uoRst_Cells:
		case uoRst_Mixed:
		{
			return retVal;
		}
		case uoRst_Row:
		case uoRst_Rows:
		case uoRst_Column:
		case uoRst_Columns:
		{
			uoRptHeaderType rht;
			int start, end;
			retVal = getTrueSectionsCR(rht, start, end);

			break;
		}
		default:{
			break;
		}
	}
	return retVal;
}


/// Определим, нажат ли Ctrl
bool uoReportSelection::isCtrlPress()
{
	Qt::KeyboardModifiers  kbrdMod = qApp->keyboardModifiers();
	if (kbrdMod & Qt::ControlModifier)
		return true;
	return false;
}

///Определим, нажат ли Shift
bool uoReportSelection::isShiftPress()
{
	Qt::KeyboardModifiers  kbrdMod = qApp->keyboardModifiers();
	if (kbrdMod & Qt::ShiftModifier)
		return true;
	return false;
}

/// Добавим строку в список выделений.
void uoReportSelection::selectRow(int nmRow)
{
	bool ctrl = isCtrlPress();
	if (!ctrl) {
		clearSelections();
	}
	clearSelections(uoRst_Rows);
	setSelectionMode(uoRst_Rows);
	if (m_selRows->contains(nmRow)){
		if (ctrl){
			int pos = 0;
			while((pos = m_selRows->indexOf(nmRow)) != -1)
				m_selRows->removeAt(pos);
		}

	} else {
		m_selRows->append(nmRow);
	}
}
void uoReportSelection::selectCol(int nmCol)
{
	bool ctrl = isCtrlPress();
	if (!ctrl)
		clearSelections();
	clearSelections(uoRst_Columns);
	setSelectionMode(uoRst_Columns);
	if (m_selCols->contains(nmCol)){
		if (ctrl){
			int pos = 0;
			while((pos = m_selCols->indexOf(nmCol)) != -1)
				m_selCols->removeAt(pos);
		}

	} else {
		m_selCols->append(nmCol);
	}
}



/// Возвращает текуший тип выделения.
uorSelectionType uoReportSelection::getSelectionType(){
	return m_selMode;
}
/// Получить текуший тип старт-выделения.
uorSelectionType uoReportSelection::getStartSelectionType(){
	return m_startSelMode;
}


/// сигнал о начале выделения строк.
void uoReportSelection::rowSelectedStart(int nmRow){
	if (!isCtrlPress()){
		clearSelections();
		selectRow(nmRow);
	}
	m_strartColRow = nmRow;
	setSelectionMode(uoRst_Row);
}

/**
	Используется для фиксации выделения в том случае когда
	пользователь работает в режиме выделения но
	еще не отпустил клавишу мыши.
*/
void uoReportSelection::rowSelectedMidle(int nmRow)
{
	if (m_strartColRow<=0)
		return;
	m_selRowsColsTmp->clear();

	int yStart 	= qMin(m_strartColRow, nmRow);
	int yEnd 	= qMax(m_strartColRow, nmRow);

	if (yStart <= 0 || yEnd <= 0)
		return;
	for (int i = yStart; i<= yEnd; i++)	{
		m_selRowsColsTmp->append(i);
	}
}


/// окончание выделения строк. пользователь отпустил мышку..
void uoReportSelection::rowSelectedEnd(int nmRow)
{
	if (m_strartColRow<=0)
		return;
	m_selRowsColsTmp->clear();

	int yStart 	= qMin(m_strartColRow, nmRow);
	int yEnd 	= qMax(m_strartColRow, nmRow);

	if (yStart <= 0 || yEnd <= 0)
		return;
	for (int i = yStart; i<= yEnd; i++)	{
		if (!m_selRows->contains(i))
			m_selRows->append(i);
	}
	m_strartColRow = 0;
}


/// сигнал о начале выделения колонок
void uoReportSelection::colSelectedStart(int nmCol){
	if (!isCtrlPress()){
		clearSelections();
		selectCol(nmCol);
	}
	m_strartColRow = nmCol;
	setSelectionMode(uoRst_Column);
}

void uoReportSelection::colSelectedEnd(int nmCol)
{
	if (m_strartColRow<=0)
		return;
	m_selRowsColsTmp->clear();

	int yStart 	= qMin(m_strartColRow, nmCol);
	int yEnd 	= qMax(m_strartColRow, nmCol);

	if (yStart <= 0 || yEnd <= 0)
		return;
	for (int i = yStart; i<= yEnd; i++)	{
		if (!m_selCols->contains(i))
			m_selCols->append(i);
	}
	m_strartColRow = 0;

}


/// сигнал о начале выделения диапазона ячеек.
void uoReportSelection::cellSelectedStart(int nmCol, int nmRow)
{
	if (isCtrlPress()){
		clearSelections(uoRst_Cells);
		if (!m_cellMidle.isNull()){
			QRect* rect = getCellSpan();
			if (rect){
				calcRectFromPoints((*rect), m_cellMidle, m_cellStart);
				m_selSpans->append(rect);
			}
			m_cellMidle.setX(0);
			m_cellMidle.setY(0);
			m_rectCellsMidle.setTop(0);
			m_rectCellsMidle.setLeft(0);
			m_rectCellsMidle.setBottom(0);
			m_rectCellsMidle.setRight(0);
		}
	} else {
		clearSelections();
	}
	if (nmCol>0 && nmRow > 0) {
		m_cellStart.setX(nmCol);
		m_cellStart.setY(nmRow);
		m_startSelMode = uoRst_Cells;
	}
}

/// Выдать QRect* из резерва, или произвести на свет новый :)
QRect* uoReportSelection::getCellSpan()
{
	QRect* rect = NULL;
	if (!m_selSpansCache->isEmpty()){
		rect = m_selSpansCache->takeFirst();
		rect->setBottom(0);
		rect->setTop(0);
		rect->setLeft(0);
		rect->setRight(0);
	} else {
		rect = new QRect(0,0,0,0);
	}
	return rect;
}

QPoint* uoReportSelection::getCellPoint()
{
	QPoint* point = NULL;
	if (!m_selPointsCache->isEmpty()){
		point = m_selPointsCache->takeFirst();
		point->setX(0);
		point->setY(0);
	} else {
		point = new QPoint(0,0);
	}
	return point;
}



/// Окончание редактирования выделения ячеек.
void uoReportSelection::cellSelectedEnd(int nmCol, int nmRow)
{
	if (m_cellStart.isNull() || (m_startSelMode != uoRst_Cells))	{
		clearSelections();
		return;
	}
	if ((m_cellStart.x() == nmCol) && (m_cellStart.y() == nmRow)){
		if (isCtrlPress() && !isCellSelect(nmRow,nmCol))
			selectCell(nmCol, nmRow);
		return;
	}
	QRect* rect = getCellSpan();
	if (rect){
		calcRectFromPoints((*rect), QPoint(nmCol,nmRow), m_cellStart);
		m_selSpans->append(rect);
	}
	refreshSelectionType();
}

/// промежуточное выделение между cellSelectedStart и cellSelectedEnd
void uoReportSelection::cellSelectedMidle(int nmCol, int nmRow)
{
	if (m_cellStart.isNull())
		return;
	m_cellMidle.setX(nmCol);
	m_cellMidle.setY(nmRow);
	calcRectFromPoints(m_rectCellsMidle, m_cellMidle, m_cellStart);
	refreshSelectionType();
	emit onSelectonChange(m_selMode, m_selMode);

}



/// сигнал о выделениИ документа
void uoReportSelection::selectDocument(){
	clearSelections();
	setSelectionMode(uoRst_Document);
}

void uoReportSelection::selectCell(int nmCol, int nmRow)
{
	bool ctrl = isCtrlPress();
	if (!ctrl)
		clearSelections();

	clearSelections(uoRst_Cells);
	setSelectionMode(uoRst_Cells);

	if (!isCellSelect(nmRow, nmCol)){
		QPoint* point = getCellPoint();
		if (point){
			point->setX(nmCol);
			point->setY(nmRow);
			m_selPoints->append(point);
		}
	}
}

/// Вернем максимальный квадрат, охватывающий выделения.
QRect uoReportSelection::getSelectionBound()
{
	QRect retVal(0,0,0,0);
	int min_row = 0, max_row = 0;
	int min_col = 0, max_col = 0;
	int genVal = 0;
	if(m_selMode == uoRst_Document || m_selMode == uoRst_Unknown){
		return retVal;
	} else {
		if (!m_selRows->isEmpty()){
			QList<int>::const_iterator itRow = m_selRows->constBegin();
			while(itRow != m_selRows->constEnd())		{
				genVal = *itRow;
				if (min_row == 0){
					min_row = max_row = genVal;
				} else {
					min_row = qMin(genVal, min_row);
					max_row = qMax(genVal, max_row);
				}
				itRow++;
			}

		} else if (!m_selCols->isEmpty()){
			QList<int>::const_iterator itCol = m_selCols->constBegin();
			while(itCol != m_selCols->constEnd())		{
				genVal = *itCol;
				if (min_col == 0){
					min_col = max_col = genVal;
				} else {
					min_col = qMin(genVal, min_col);
					max_col = qMax(genVal, max_col);
				}
				itCol++;
			}
		} else {
			int spnCnt = m_selSpans->count();
			int cellCnt = m_selPoints->count();

			if (spnCnt>0){
				QRect* genRect = NULL;
				QList<QRect*>::const_iterator itRct = m_selSpans->constBegin();
				while(itRct != m_selSpans->constEnd())		{
					genRect = *itRct;
					if (min_col == 0){
						min_col = genRect->x();
						max_col = genRect->right();
						min_row = genRect->y();
						max_row = genRect->bottom();
					} else {
						min_row = qMin(genRect->y(), min_row);
						max_row = qMax(genRect->bottom(), max_row);
						min_col = qMin(genRect->x(), min_col);
						max_col = qMax(genRect->right(), max_col);
					}
					itRct++;
				}
			}
			if (cellCnt>0){
				QPoint* genPoint = NULL;
				QList<QPoint*>::const_iterator itRct = m_selPoints->constBegin();
				while(itRct != m_selPoints->constEnd())		{
					genPoint = *itRct;
					if (min_col == 0){
						min_col = max_col = genPoint->y();
						min_row = max_row = genPoint->x();
					} else {
						min_row = qMin(genPoint->y(), min_row);
						max_row = qMax(genPoint->y(), max_row);
						min_col = qMin(genPoint->x(), min_col);
						max_col = qMax(genPoint->x(), max_col);
					}
					itRct++;
				}
			}
			if (!m_rectCellsMidle.isEmpty()){
				min_row = qMin(m_rectCellsMidle.y(), min_row);
				max_row = qMax(m_rectCellsMidle.bottom(), max_row);
				min_col = qMin(m_rectCellsMidle.x(), min_col);
				max_col = qMax(m_rectCellsMidle.right(), max_col);
			}
		}
		retVal.setY(min_row);
		retVal.setBottom(max_row);
		retVal.setX(min_col);
		retVal.setRight(max_col);

//		qDebug()<<" min_row" <<min_row<<"max_row " <<max_row <<" min_col" <<min_col <<" max_col" <<max_col;

	}
	return retVal;
}
/// Анализируем структуры выделений, просто обновляем m_selMode
void uoReportSelection::refreshSelectionType()
{
	if (m_selMode == uoRst_Document)
		return;
	uorSelectionType selModeOld = m_selMode;
	uorSelectionType selModeNew = m_selMode;

	int countItem = 0;
	if (!m_selRows->isEmpty()){
		countItem = m_selRows->count();
		selModeNew = uoRst_Row;
		if (countItem>1){
			selModeNew = uoRst_Rows;
		}
	} else if (!m_selCols->isEmpty()){
		countItem = m_selCols->count();
		selModeNew = uoRst_Column;
		if (countItem>1){
			selModeNew = uoRst_Columns;
		}
	} else {
		int spnCnt = m_selSpans->count();
		int cellCnt = m_selPoints->count();
		if (spnCnt > 0 && cellCnt > 0){
			selModeNew = uoRst_Mixed;
		} else {
			selModeNew = uoRst_Cells;
			if (cellCnt == 1 && spnCnt == 0)
				selModeNew = uoRst_Cell;
		}
	}
	if (selModeOld != selModeNew)
		setSelectionMode(selModeNew);

}



} //namespace uoReport
