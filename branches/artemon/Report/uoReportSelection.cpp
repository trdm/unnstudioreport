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
	m_selRows 		= new QList<int>;
	m_selRowsColsTmp = new QList<int>;
	m_selCols 		= new QList<int>;
	m_selSpans 		= new QList<QRect*>;
	m_selSpansCount = 0;
	m_selSpansCache = new QList<QRect*>;

	m_selPoints 	 = new QList<QPoint*>;
	m_selPointsCount = 0;
	m_selPointsCache = new QList<QPoint*>;
	m_selRowsCell	= new QList<int>;		///< Список строк c выделенными ячейками
	m_selColsCell	= new QList<int>;		///< Список колонок c выделенными ячейками



	m_startSelMode	= uoRst_Unknown;
	setSelectionMode(uoRst_Unknown);
	m_strartColRow	= -1;
	m_cellStart.setX(0);
	m_cellStart.setY(0);
	m_cellMidle.setX(0);
	m_cellMidle.setY(0);
	m_currentCell.setX(0);
	m_currentCell.setY(0);
}

uoReportSelection::~uoReportSelection()
{
	delete m_selRows;
	delete m_selRowsColsTmp;
	delete m_selCols;
	while (!m_selSpans->isEmpty())		delete m_selSpans->takeFirst();
	while (!m_selSpansCache->isEmpty())	delete m_selSpansCache->takeFirst();

	while (!m_selPoints->isEmpty())			delete m_selPoints->takeFirst();
	while (!m_selPointsCache->isEmpty())	delete m_selPointsCache->takeFirst();

	delete m_selSpans;
	delete m_selSpansCache;
	delete m_selRowsCell;
	delete m_selColsCell;

	while (!m_celectedCellRectList.isEmpty())	delete m_celectedCellRectList.takeFirst();
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

	if (exclude == uoRst_Rows || exclude == uoRst_Columns) {
		saveMidleRowCol();
	} else {
		m_selRowsColsTmp->clear();
	}
	// это должно быть в таком порядке..
	m_startSelMode	= uoRst_Unknown;
	m_strartColRow	= -1;


	if (exclude != uoRst_Rows) {
		m_selRows->clear();
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
		m_selSpansCount = 0;

		while (!m_selPoints->isEmpty())
			m_selPointsCache->append(m_selPoints->takeFirst());
		m_selPointsCount = 0;

		while (!m_celectedCellRectList.isEmpty())	delete m_celectedCellRectList.takeFirst();

		m_selRowsCell->clear();
		m_selColsCell->clear();
	} else {
		setSelectionMode(uoRst_Cells);
	}
}

/// Проверка на выделенность столбца
bool uoReportSelection::isColSelect(const int& nmCol)  const
{
	bool retVal = false;
	if (retVal = isDocumSelect()){
		return retVal;
	} else if (m_selMode == uoRst_Column || m_selMode == uoRst_Columns){

		if (!m_selCols->isEmpty()) {
			retVal = m_selCols->contains(nmCol);
			if (retVal)
				return true;
		}
		if (!m_selRowsColsTmp->isEmpty()){
			if (m_selMode == uoRst_Column || m_selMode == uoRst_Columns) {
				retVal = m_selRowsColsTmp->contains(nmCol);
			}
		}
	}

	return retVal;
}

/// Проверка на выделенность строки
bool uoReportSelection::isRowSelect(const int& nmRow)  const
{
	bool retVal = false;
	if (retVal = isDocumSelect()){
		return retVal;
	}
	if (m_selMode == uoRst_Row || m_selMode == uoRst_Rows) {
		if (!m_selRows->isEmpty()) {
			retVal = m_selRows->contains(nmRow);
			if (retVal)
				return true;
		}
		if (!m_selRowsColsTmp->isEmpty()){
			if (m_selMode == uoRst_Row || m_selMode == uoRst_Rows) {
				retVal = m_selRowsColsTmp->contains(nmRow);
			}
		}
	}
	return retVal;
}


/// Проверка КОЛОНКА выделена частично?
bool uoReportSelection::isColPartlySelect(const int& nmCol) const{
	return m_selColsCell->contains(nmCol);
}
/// Проверка СТРОКА выделена частично?
bool uoReportSelection::isRowPartlySelect(const int& nmRow) const{
	return m_selRowsCell->contains(nmRow);
}


/// Проверка на выделенность ячейки.
bool uoReportSelection::isCellSelect(const int& nmRow, const int& nmCol)  const
{
	bool retVal = false;
	if (isDocumSelect()) {
		return true;
	}

	if (m_selMode == uoRst_Cell || m_selMode == uoRst_Cells || m_selMode == uoRst_Mixed) {
		if(!m_rectCellsMidle.isEmpty()) {
			if (m_rectCellsMidle.contains(QPoint(nmCol,nmRow))){
				return true;
			}
		}


		if (m_selSpansCount>0) {
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
		//if (!m_selPoints->isEmpty()) {
		if (m_selPointsCount>0) {
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
	}
	if (isColSelect(nmCol) ||  isRowSelect(nmRow)) {
		return true;
	}

	return retVal;
}
///\todo заинлайнить...
bool uoReportSelection::isCurrentCell(int nmRow, int nmCol)  const
{
	if (m_currentCell.x() == nmCol && m_currentCell.y() == nmRow)
		return true;
	return false;
}
/**
	Получить правильный диапазон выделенных строк или столбцов и статус возврата.
*/
bool uoReportSelection::getTrueSectionsCR(uoRptHeaderType& rht, int& start, int& end) const
{
	rht = uorRhtUnknown;
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
			rht = uorRhtRowsHeader;

			if (m_selMode == uoRst_Column || m_selMode == uoRst_Columns)
			{
				list = m_selCols;
				rht = uorRhtColumnHeader;
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
		rht = uorRhtUnknown;
		start = end = -1;
	}
	return retVal;

}

/**
	Возвращает труе, если выделения имеют тип строка/строки или столбцы/столбец и
	их тип и список выделений.
*/
bool uoReportSelection::getSelectedColRow(uoRptHeaderType& rht, QList<int>& list) const
{
	rht = uorRhtUnknown;
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
			int rowCur;
			QList<int>* p_list = m_selRows;
			rht = uorRhtRowsHeader;

			if (m_selMode == uoRst_Column || m_selMode == uoRst_Columns)
			{
				p_list = m_selCols;
				rht = uorRhtColumnHeader;
			}

			if (p_list->isEmpty()) {
				retVal = false;

			} else if (p_list->size() == 1){
				retVal = true;
				list.append(p_list->at(0));

			} else {
				retVal = true;
				qSort(*p_list);
				QList<int>::const_iterator iter = p_list->constBegin();
				while(iter != p_list->constEnd()){
					rowCur = *iter;
					list.append(rowCur);
					iter++;
				}
				qSort(list);
			}
			break;
		}
		default:{	break;	}
	}
	if (!retVal)	{
		rht = uorRhtUnknown;
	}

	return retVal;
}


///Определим, выделены ли uoLineSpan полностью...
bool uoReportSelection::isSpanFullSeleced(uoLineSpan* spn, const uoRptHeaderType& curHeaderType) const
{
	bool retVal = false;
	int i = 0, spn_Start = 0, spn_End = 0;

	spn_Start = spn->getStart();
	spn_End = spn->getEnd();
	switch(m_selMode)
	{
		case uoRst_Unknown:
		case uoRst_Cell:
		case uoRst_Cells:
		case uoRst_Mixed:
		{
			return retVal;
		}
		case uoRst_Document:
		{
			return true;
		}
		case uoRst_Row:
		case uoRst_Rows:
		{
			if (curHeaderType == uorRhtColumnHeader) {
				return retVal;
			} else {
				for (i = spn_Start; i<= spn_End; i++)
				{
					if (!isRowSelect(i))
						return false;
				}
				return true;
			}
		}

		case uoRst_Column:
		case uoRst_Columns:
		{
			if (curHeaderType == uorRhtRowsHeader){
				return retVal;
			} else {
				for (i = spn_Start; i<= spn_End; i++)
				{
					if (!isColSelect(i))
						return false;
				}
				return true;
			}

			break;
		}
		default:{
			break;
		}
	}
	return retVal;

}

/**
	Определим, подходит ли выделение для операций с секциями.
	правильное выделение должно быть строка|Строки_подряд|Столбец|Столбцы_подряд
*/
bool uoReportSelection::isTrueForSections()  const
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

/**
	Определим пригодно ли выделение для копирования.

*/
bool uoReportSelection::isTrueForCopy()  const
{
	return true; // пока так, не вижу сложностей, это для драг-дропа будет сложно...

	bool retVal = false;

	switch(m_selMode)
	{
		case uoRst_Unknown:
		case uoRst_Mixed:
		{
			return retVal;
		}
		case uoRst_Document:
		case uoRst_Cell:
		case uoRst_Cells:
		case uoRst_Row:
		case uoRst_Rows:
		case uoRst_Column:
		case uoRst_Columns:
		{
			return true;
			/// пока отключим, будем выгружать все.
			/*
			uoRptHeaderType rht;
			int start, end;
			retVal = getTrueSectionsCR(rht, start, end);
			*/

			break;
		}
		default:{
			break;
		}
	}
	return retVal;

}

/**
	Выделение нормально для объединения ячеек...
*/
bool uoReportSelection::isTrueForJoin()  const
{
	bool retVal = false;
	if (m_selMode == uoRst_Cells){
		QRect* rct = 0;
		if (m_selSpansCount == 1){
			rct = m_selSpans->at(0);
			if (rct->width()>1 || rct->height()>1)
				retVal = true;
		}
		if (!retVal){
			if (m_rectCellsMidle.width()>1 || m_rectCellsMidle.height()>1)
				retVal = true;
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
void uoReportSelection::selectRow(int nmRow, bool clearSel)
{
	bool ctrl = isCtrlPress();
	if (!ctrl && clearSel) {
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
void uoReportSelection::selectCol(int nmCol, bool clearSel)
{
	bool ctrl = isCtrlPress();
	if (!ctrl && clearSel)
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

/**
	Возвращает количество полностю выделенных строк
*/
int  uoReportSelection::rowCount() const
{
	int retVal = -1;
	if (m_selMode == uoRst_Row || m_selMode == uoRst_Rows)
	{
		QList<int>* list;
		retVal = 0;

		QList<int>::const_iterator itRow;
		for (int i = 0; i<2; i++) {
			if (i == 0){
				list = m_selRows;
			} else {
				list = m_selRowsColsTmp;
			}
			if (list->isEmpty())
				continue;

			itRow = list->constBegin();
			while(itRow != list->constEnd())
			{
				retVal += 1;
				itRow++;
			}
		}
	}
	return retVal;
}
/**
	Возвращает количество полностю выделенных КОЛОНОК
*/
int  uoReportSelection::colCount() const
{
	int retVal = -1;
	if (m_selMode == uoRst_Column || m_selMode == uoRst_Columns)
	{
		QList<int>* list;
		retVal = 0;

		QList<int>::const_iterator itRow;
		for (int i = 0; i<2; i++) {
			if (i == 0){
				list = m_selCols;
			} else {
				list = m_selRowsColsTmp;
			}
			if (list->isEmpty())
				continue;

			itRow = list->constBegin();
			while(itRow != list->constEnd())
			{
				retVal += 1;
				itRow++;
			}
		}
	}
	return retVal;
}


QPoint uoReportSelection::currentCell() const
{
	return m_currentCell;
}
void uoReportSelection::currentCell(int& row, int& col) const
{
	row = m_currentCell.y();
	col = m_currentCell.x();
}

void uoReportSelection::setCurrentCell(QPoint pnt, int rowsJn, int colsJn)
{
	m_currentCell = pnt;
	m_currentRect = QRect(pnt.x(), pnt.y(), qMax(1,colsJn), qMax(1,rowsJn));

}

void uoReportSelection::setCurrentCell(int row, int col, int rowsJn, int colsJn)
{
	if (col>0)
		m_currentCell.setX(col);
	if (row>0)
		m_currentCell.setY(row);
	m_currentRect = QRect(col, row, qMax(1,colsJn), qMax(1,rowsJn));

}



/// Возвращает текуший тип выделения.
uorSelectionType uoReportSelection::selectionType(){
	return m_selMode;
}
/// Получить текуший тип старт-выделения.
uorSelectionType uoReportSelection::getStartSelectionType(){
	return m_startSelMode;
}
uorSelectionType uoReportSelection::convertIntToSelMode(const int val) const
{
	uorSelectionType retVal = uoRst_Unknown;
	switch(val){
		case uoRst_Cell: 	retVal = uoRst_Cell; break;
		case uoRst_Cells: 	retVal = uoRst_Cells; break;
		case uoRst_Row: 	retVal = uoRst_Row; break;
		case uoRst_Rows: 	retVal = uoRst_Rows; break;
		case uoRst_Column: 	retVal = uoRst_Column; break;
		case uoRst_Columns: retVal = uoRst_Columns; break;
		case uoRst_Mixed: 	retVal = uoRst_Mixed; break;
		case uoRst_Document: retVal = uoRst_Document; break;
		default:
			retVal = uoRst_Unknown;
	}
	return retVal;
}

void uoReportSelection::saveMidleRowCol()
{
	if (m_startSelMode == uoRst_Columns || m_startSelMode == uoRst_Rows) {
		if (!m_selRowsColsTmp->isEmpty()){
			int colTmp = 0;
			QList<int>::const_iterator it = m_selRowsColsTmp->constBegin();
			while(it != m_selRowsColsTmp->constEnd()){
				colTmp = *it;
				if (m_startSelMode == uoRst_Columns) {
					if (!m_selCols->contains(colTmp) && colTmp>0)
						m_selCols->append(colTmp);
				} else if (m_startSelMode == uoRst_Rows) {
					if (!m_selRows->contains(colTmp) && colTmp>0)
						m_selRows->append(colTmp);
				}
				it++;
			}
		}
	}
}

/// сигнал о начале выделения строк.
void uoReportSelection::rowSelectedStart(int nmRow){
	if (!isCtrlPress()){
		clearSelections();
		selectRow(nmRow);
	}
	m_strartColRow = nmRow;
	setSelectionMode(uoRst_Row);
	m_startSelMode = uoRst_Rows;
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
	m_startSelMode = uoRst_Columns;
}

void uoReportSelection::colSelectedMidle(int nmCol)
{
	if (m_strartColRow<=0)
		return;
	m_selRowsColsTmp->clear();

	int yStart 	= qMin(m_strartColRow, nmCol);
	int yEnd 	= qMax(m_strartColRow, nmCol);

	if (yStart <= 0 || yEnd <= 0)
		return;
	for (int i = yStart; i<= yEnd; i++)	{
		m_selRowsColsTmp->append(i);
	}
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
				m_selSpansCount = m_selSpans->count();
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

bool uoReportSelection::getMinMax(int& minVal, int& maxVal, QList<int>* list)  const
{
	bool retVal = true;
	if (list->isEmpty()){
		return false;
	}
	int curVal = -1;
	QList<int>::const_iterator itList = list->constBegin();
	while(itList != list->constEnd()){
		curVal = *itList;
		if (minVal == -1){
			minVal = curVal;
		} else {
			minVal = qMin(curVal,minVal);
		}
		if (maxVal == -1){
			maxVal = curVal;
		} else {
			maxVal = qMax(curVal,maxVal);
		}
	}


	return retVal;
}

/// Вернуть минимальную и максимальную строку/столбец выделения
QPoint	uoReportSelection::getMinMaxRC(const uoRptHeaderType curHeaderType) const
{
	QPoint	selPoint(-1, -1);
	int minNo = -1, maxNo = -1;

	if (curHeaderType == uorRhtColumnHeader && (m_selMode == uoRst_Column || m_selMode == uoRst_Columns)){
		if (!m_selCols->isEmpty()) {
			getMinMax(minNo, maxNo, m_selCols);
		}
		if (!m_selRowsColsTmp->isEmpty()){
			getMinMax(minNo, maxNo, m_selRowsColsTmp);
		}
		selPoint = QPoint(minNo,maxNo);

	} else if (curHeaderType == uorRhtRowsHeader && (m_selMode == uoRst_Row || m_selMode == uoRst_Rows)){
		if (!m_selRows->isEmpty()) {
			getMinMax(minNo, maxNo, m_selRows);
		}
		if (!m_selRowsColsTmp->isEmpty()){
			getMinMax(minNo, maxNo, m_selRowsColsTmp);
		}
		selPoint = QPoint(minNo,maxNo);
	}

	return (selPoint);
}

/// Вернуть точку вставки по выделению.
QPoint uoReportSelection::getInsertPoint() const
{
	QRect  selBound(1,1,1,1);

	QPoint insPoint(1,1);
	if (m_selMode == uoRst_Cell || m_selMode == uoRst_Cells || m_selMode == uoRst_Mixed)	{
		selBound = getSelectionBound();
		insPoint = selBound.topLeft();
	} else if (m_selMode == uoRst_Row || m_selMode == uoRst_Rows) {
		insPoint = getMinMaxRC(uorRhtRowsHeader);
	} else if (m_selMode == uoRst_Column || m_selMode == uoRst_Columns) {
		insPoint = getMinMaxRC(uorRhtColumnHeader);
	} else if (m_selMode == uoRst_Unknown) {
		insPoint = currentCell();
	}
	return (insPoint);

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

void uoReportSelection::onRectSelected(QRect* selRect)
{
	int i = 0;
	for (i = selRect->top(); i <= selRect->bottom(); i++){
		if (!m_selRowsCell->contains(i))
			m_selRowsCell->append(i);
	}
	for (i = selRect->left(); i <= selRect->right(); i++){
		if (!m_selColsCell->contains(i))
			m_selColsCell->append(i);
	}
}


/// Окончание редактирования выделения ячеек.
void uoReportSelection::cellSelectedEnd(int& nmCol, int& nmRow)
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
		m_selSpansCount = m_selSpans->count();
		onRectSelected(rect);
	}
	refreshSelectionType();
}

/// обновить списки частично выделенных строк/столбцов
void uoReportSelection::refreshPartlySelRC(){
	/* есть проблемы с выделениями, когда умешьшается раннее выделенный
	диапазон ячеек.	решим этой функцией */
	m_selRowsCell->clear();
	m_selColsCell->clear();

	if (m_selSpansCount>0) {
		QRect* rct = NULL;

		QList<QRect*>::const_iterator iter = m_selSpans->constBegin();
		while(iter != m_selSpans->constEnd()) {
			rct = *iter;
			if (rct) {
				onRectSelected(rct);
			}
			iter++;
		}
	}
	if (m_selPointsCount>0) {
		QPoint* point = NULL;

		QList<QPoint*>::const_iterator iter = m_selPoints->constBegin();
		while(iter != m_selPoints->constEnd()) {
			point = *iter;
			if (point) {
				m_selRowsCell->append(point->y());
				m_selColsCell->append(point->x());
			}
			iter++;
		}
	}
}

/// промежуточное выделение между cellSelectedStart и cellSelectedEnd
void uoReportSelection::cellSelectedMidle(int& nmCol, int& nmRow)
{
	if (m_cellStart.isNull())
		return;
	m_cellMidle.setX(nmCol);
	m_cellMidle.setY(nmRow);
	calcRectFromPoints(m_rectCellsMidle, m_cellMidle, m_cellStart);
	refreshPartlySelRC();
	onRectSelected(&m_rectCellsMidle);
	refreshSelectionType();
	emit onSelectonChange(m_selMode, m_selMode);

}



/// сигнал о выделениИ документа
void uoReportSelection::selectDocument(){
	clearSelections();
	setSelectionMode(uoRst_Document);
}

void uoReportSelection::selectCell(int nmCol, int nmRow, bool clearSel)
{
	bool ctrl = isCtrlPress();
	if (!ctrl && clearSel)
		clearSelections();

	clearSelections(uoRst_Cells);
	setSelectionMode(uoRst_Cells);

	if (!isCellSelect(nmRow, nmCol)){
		QPoint* point = getCellPoint();
		if (point){
			point->setX(nmCol);
			point->setY(nmRow);
			m_selPoints->append(point); ++m_selPointsCount;
			m_selRowsCell->append(nmRow);
			m_selColsCell->append(nmCol);
		}
	}
}

/// Вернем максимальный квадрат, охватывающий выделения.
QRect uoReportSelection::getSelectionBound() const
{
	QRect retVal(0,0,0,0);
	int min_row = 0, max_row = 0, min_col = 0, max_col = 0;
	int genVal = 0;
	if(m_selMode == uoRst_Document || m_selMode == uoRst_Unknown){
		if (m_selMode == uoRst_Unknown)
			return QRect(m_currentCell,m_currentCell);
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
			int spnCnt = m_selSpansCount; //m_selSpans->count();


			int cellCnt = m_selPointsCount; //m_selPoints->count();

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
						min_col = max_col = genPoint->x();
						min_row = max_row = genPoint->y();
					} else {
						min_row = qMin(genPoint->y(), min_row);
						max_row = qMax(genPoint->y(), max_row);
						min_col = qMin(genPoint->x(), min_col);
						max_col = qMax(genPoint->x(), max_col);
					}
					itRct++;
				}
			}
			if (m_rectCellsMidle.y()>0){
				if (spnCnt == 0 && cellCnt == 0) {
					min_row = m_rectCellsMidle.top();
					max_row = m_rectCellsMidle.bottom();
					min_col = m_rectCellsMidle.left();
					max_col = m_rectCellsMidle.right();
				} else {
					min_row = qMin(m_rectCellsMidle.y(), min_row);
					max_row = qMax(m_rectCellsMidle.bottom(), max_row);
					min_col = qMin(m_rectCellsMidle.x(), min_col);
					max_col = qMax(m_rectCellsMidle.right(), max_col);
				}
			} else if (spnCnt == 0 && cellCnt == 0){
				/// значит это только одна ячейка
				min_row = max_row = m_currentCell.y();
				min_col = max_col = m_currentCell.x();
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

/// Сохраним выделение
void uoReportSelection::saveSelection(uoReportLoader* loader)
{
	if(!loader)
		return;
	loader->saveSelectionStart(m_selMode);
	switch(m_selMode)
	{
		case uoRst_Document:
		{
			break;
		}
		case uoRst_Row:
		case uoRst_Rows:
		case uoRst_Column:
		case uoRst_Columns:
		{
			QList<int> int_list;
			uoRptHeaderType rht = uorRhtRowsHeader;
			if (!getSelectedColRow(rht, int_list))
				return;
			if(!int_list.isEmpty()){
				loader->saveSelectionRCStart(m_selMode, int_list.size());
				loader->saveSelectionRCItems(int_list);
				loader->saveSelectionRCEnd();
			}

			break;
		}
		case uoRst_Cell:
		case uoRst_Cells:
		case uoRst_Mixed:
		case uoRst_Unknown:
		{
			QList<QPoint> pntList;
			QPoint pt;
			if (uoRst_Unknown == m_selMode){
				pntList.append(m_currentCell);
			} else {
				QRect boundRect = getSelectionBound();
				int r_cntr = 1, c_cntr = 1;
				for (int rrow = boundRect.top(); rrow<=boundRect.bottom(); rrow++)	{
					c_cntr = 1;
					for (int rcol = boundRect.left(); rcol<=boundRect.right(); rcol++)	{
						if (isCellSelect(rrow, rcol)){
							/*
							curCell = getCell(rrow, rcol, false);
							cellMatrix->setCell(r_cntr, c_cntr, curCell);
							cellMatrix->setCellSelected(r_cntr, c_cntr);
							*/
							pt = QPoint(c_cntr, r_cntr);
							if (!pntList.contains(pt))
								pntList.append(pt);
						}
						c_cntr += 1;
					}
					r_cntr += 1;
				}
			}
			if (!pntList.isEmpty()){
				int cntrPoint = 1, listSz = pntList.size();

				loader->saveSelectionCellsStart(m_selMode, listSz);
				QList<QPoint>::const_iterator it = pntList.constBegin();
				while(it != pntList.constEnd()){
					loader->saveSelectionCell(*it, cntrPoint,listSz);
					++it;
				}

				loader->saveSelectionCellsEnd();
			}

//			qDebug() << "QList<QPoint> pntList: "<<pntList;

			break;
		}

		default: {
			break;
		}


	}
	loader->saveSelectionEnd();
}
/// копирование выделения
void uoReportSelection::copyFrom(uoReportSelection* selOther, int rowOffset, int colOffset)
{
	clearSelections();
	m_selMode = selOther->m_selMode;
	int curNo = 0;
	QList<int>* list = 0;
	list = selOther->m_selRows;

	if (!list->isEmpty()){
		foreach(curNo, *list){
			curNo = curNo + rowOffset;
			m_selRows->append(curNo);
		}
	}
	list = selOther->m_selRowsColsTmp;
	if (!list->isEmpty()){
		foreach(curNo, *list){
			if (m_selMode == uoRst_Column || m_selMode == uoRst_Columns){
				curNo = curNo + colOffset;
			} else {
				curNo = curNo + rowOffset;
			}
			m_selRowsColsTmp->append(curNo);
		}
	}
	list = selOther->m_selCols;
	if (!list->isEmpty()){
		foreach(curNo, *list){
			curNo = curNo + colOffset;
			m_selCols->append(curNo);
		}
	}

	QList<QRect*>*  listSpans = 0;
	QRect* rect = 0;
	QRect* rectNew = 0;

	listSpans = selOther->m_selSpans;
	if (!listSpans->isEmpty()){
		QList<QRect*>::const_iterator iterSpn = listSpans->constBegin();
		while(iterSpn != listSpans->constEnd()) {
			rect = *iterSpn;
			rectNew = new QRect(rect->left() + colOffset, rect->top() + rowOffset,rect->width() + colOffset, rect->height() + rowOffset );
			m_selSpans->append(rectNew);
			m_selSpansCount = m_selSpans->count();
			onRectSelected(rectNew);
			iterSpn++;
		}
	}
	QRect rctTmp = selOther->m_rectCellsMidle;
	if (rctTmp.top()*rctTmp.bottom() > 0){

		rectNew = new QRect(rctTmp.left() + colOffset, rctTmp.top()+ rowOffset, rctTmp.width() + colOffset, rctTmp.height() + rowOffset);
		m_selSpans->append(rectNew);
		m_selSpansCount = m_selSpans->count();
		onRectSelected(rectNew);
	}

	QList<QPoint*>*  listPoint = 0;
	QPoint* point = 0;
	QPoint* pointNew = 0;

	listPoint = selOther->m_selPoints;
	if (!listPoint->isEmpty()){
		QList<QPoint*>::const_iterator iterPnt = listPoint->constBegin();
		while(iterPnt != listPoint->constEnd()) {
			point = *iterPnt;
			pointNew = new QPoint(point->x() + colOffset, point->y() + rowOffset);
			m_selPoints->append(pointNew); ++m_selPointsCount;

			m_selRowsCell->append(pointNew->y());
			m_selColsCell->append(pointNew->x());

			iterPnt++;
		}
	}
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
		int spnCnt = m_selSpansCount; //m_selSpans->count();
		int cellCnt = m_selPointsCount; //m_selPoints->count();
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
