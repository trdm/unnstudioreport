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
	_selRows 	= new QList<int>;
	_selRowsColsTmp = new QList<int>;
	_selCols = new QList<int>;
	_selSpans 		= new QList<QRect*>;
	_selSpansCache 	= new QList<QRect*>;

	_startSelMode	= uoRst_Unknown;
	_selMode		= uoRst_Unknown;
	_strartColRow	= -1;
	_cellStart.setX(0);
	_cellStart.setY(0);
	_cellMidle.setX(0);
	_cellMidle.setY(0);
}

uoReportSelection::~uoReportSelection()
{
	delete _selRows;
	delete _selRowsColsTmp;
	delete _selCols;
	while (!_selSpans->isEmpty())		delete _selSpans->takeFirst();
	while (!_selSpansCache->isEmpty())	delete _selSpansCache->takeFirst();
	delete _selSpans;
	delete _selSpansCache;
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

/// Очистка всех даных по выделениям
void uoReportSelection::clearSelections(uoRptSelectionType exclude)
{
	_selMode		= uoRst_Unknown;
	_startSelMode	= uoRst_Unknown;
	_strartColRow	= -1;


	if (exclude != uoRst_Rows) {
		_selRows->clear();
		_selRowsColsTmp->clear();
	} else {
		_selMode = uoRst_Rows;
	}
	if (exclude != uoRst_Columns) {
		_selCols->clear();
	} else {
		_selMode = uoRst_Columns;
	}

	if (exclude != uoRst_Cells) {
		_cellStart.setX(0);
		_cellStart.setY(0);
		_cellMidle.setX(0);
		_cellMidle.setY(0);
		_rectCellsMidle.setTop(0);
		_rectCellsMidle.setLeft(0);
		_rectCellsMidle.setBottom(0);
		_rectCellsMidle.setRight(0);

	while (!_selSpans->isEmpty())
		_selSpansCache->append(_selSpans->takeFirst());
	} else {
		_selMode = uoRst_Cells;
	}
}

/// Проверка на выделенность столбца
bool uoReportSelection::isColSelect(int nmCol)
{
	bool retVal = false;
	if (retVal = isDocumSelect()){
		return retVal;
	} else if (!_selCols->isEmpty()) {
		retVal = _selCols->contains(nmCol);
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
	if (!_selRows->isEmpty()) {
		retVal = _selRows->contains(nmRow);
		if (retVal)
			return true;
	}
	if (!_selRowsColsTmp->isEmpty()){
		retVal = _selRowsColsTmp->contains(nmRow);
		if (retVal)
			return true;
	}
	return retVal;
}

/// Проверка на выделенность документа
bool uoReportSelection::isDocumSelect()
{
	if (_selMode == uoRst_Document){
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
	} else if(!_rectCellsMidle.isEmpty()) {
		if (_rectCellsMidle.contains(QPoint(nmCol,nmRow))){
			return true;
		}
	}


	if (_selSpans->isEmpty())
		return false;

	QRect* rct = NULL;

	QList<QRect*>::const_iterator iter = _selSpans->constBegin();
	while(iter != _selSpans->constEnd()) {
		rct = *iter;
		if (rct) {
			if (rct->contains(nmCol, nmRow))
				return true;
		}
		iter++;
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
	if (!ctrl)
		clearSelections();
	clearSelections(uoRst_Rows);
	_selMode = uoRst_Rows;
	if (_selRows->contains(nmRow)){
		if (ctrl){
			int pos = 0;
			while((pos = _selRows->indexOf(nmRow)) != -1)
				_selRows->removeAt(pos);
		}

	} else {
		_selRows->append(nmRow);
	}
}
void uoReportSelection::selectCol(int nmCol)
{
	bool ctrl = isCtrlPress();
	if (!ctrl)
		clearSelections();
	clearSelections(uoRst_Columns);
	_selMode = uoRst_Columns;
	if (_selCols->contains(nmCol)){
		if (ctrl){
			int pos = 0;
			while((pos = _selCols->indexOf(nmCol)) != -1)
				_selCols->removeAt(pos);
		}

	} else {
		_selCols->append(nmCol);
	}
}



/// Возвращает текуший тип выделения.
uoRptSelectionType uoReportSelection::getSelectionType(){
	return _selMode;
}
/// Получить текуший тип старт-выделения.
uoRptSelectionType uoReportSelection::getStartSelectionType(){
	return _startSelMode;
}


/// сигнал о начале выделения строк.
void uoReportSelection::rowSelectedStart(int nmRow){
	if (!isCtrlPress()){
		clearSelections();
		selectRow(nmRow);
	}
	_strartColRow = nmRow;
	_selMode = uoRst_Row;
}

/**
	Используется для фиксации выделения в том случае когда
	пользователь работает в режиме выделения но
	еще не отпустил клавишу мыши.
*/
void uoReportSelection::rowSelectedMidle(int nmRow)
{
	if (_strartColRow<=0)
		return;
	_selRowsColsTmp->clear();

	int yStart 	= qMin(_strartColRow, nmRow);
	int yEnd 	= qMax(_strartColRow, nmRow);

	if (yStart <= 0 || yEnd <= 0)
		return;
	for (int i = yStart; i<= yEnd; i++)	{
		_selRowsColsTmp->append(i);
	}
}


/// окончание выделения строк. пользователь отпустил мышку..
void uoReportSelection::rowSelectedEnd(int nmRow)
{
	if (_strartColRow<=0)
		return;
	_selRowsColsTmp->clear();

	int yStart 	= qMin(_strartColRow, nmRow);
	int yEnd 	= qMax(_strartColRow, nmRow);

	if (yStart <= 0 || yEnd <= 0)
		return;
	for (int i = yStart; i<= yEnd; i++)	{
		if (!_selRows->contains(i))
			_selRows->append(i);
	}
	_strartColRow = 0;
}


/// сигнал о начале выделения колонок
void uoReportSelection::colSelectedStart(int nmCol){
	if (!isCtrlPress()){
		clearSelections();
		selectCol(nmCol);
	}
	_strartColRow = nmCol;
	_selMode = uoRst_Column;
}

void uoReportSelection::colSelectedEnd(int nmCol)
{
	if (_strartColRow<=0)
		return;
	_selRowsColsTmp->clear();

	int yStart 	= qMin(_strartColRow, nmCol);
	int yEnd 	= qMax(_strartColRow, nmCol);

	if (yStart <= 0 || yEnd <= 0)
		return;
	for (int i = yStart; i<= yEnd; i++)	{
		if (!_selCols->contains(i))
			_selCols->append(i);
	}
	_strartColRow = 0;

}


/// сигнал о начале выделения диапазона ячеек.
void uoReportSelection::cellSelectedStart(int nmCol, int nmRow)
{
	if (isCtrlPress()){
		clearSelections(uoRst_Cells);
		if (!_cellMidle.isNull()){
			QRect* rect = getCellSpan();
			if (rect){
				calcRectFromPoints((*rect), _cellMidle, _cellStart);
				_selSpans->append(rect);
			}
			_cellMidle.setX(0);
			_cellMidle.setY(0);
			_rectCellsMidle.setTop(0);
			_rectCellsMidle.setLeft(0);
			_rectCellsMidle.setBottom(0);
			_rectCellsMidle.setRight(0);
		}
	} else {
		clearSelections();
	}
	if (nmCol>0 && nmRow > 0) {
		_cellStart.setX(nmCol);
		_cellStart.setY(nmRow);
		_startSelMode = uoRst_Cells;
	}
}

/// Выдать QRect* из резерва, или произвести на свет новый :)
QRect* uoReportSelection::getCellSpan()
{
	QRect* rect = NULL;
	if (!_selSpansCache->isEmpty()){
		rect = _selSpansCache->takeFirst();
	} else {
		rect = new QRect(0,0,0,0);
	}
	return rect;
}


/// Окончание редактирования выделения ячеек.
void uoReportSelection::cellSelectedEnd(int nmCol, int nmRow)
{
	if (_cellStart.isNull() || (_startSelMode != uoRst_Cells))	{
		clearSelections();
		return;
	}
	if ((_cellStart.x() == nmCol) && (_cellStart.y() == nmRow)){
		return;
	}
	QRect* rect = getCellSpan();
	if (rect){
		calcRectFromPoints((*rect), QPoint(nmCol,nmRow), _cellStart);
		_selSpans->append(rect);
	}
}

/// промежуточное выделение между cellSelectedStart и cellSelectedEnd
void uoReportSelection::cellSelectedMidle(int nmCol, int nmRow)
{
	if (_cellStart.isNull())
		return;
	_cellMidle.setX(nmCol);
	_cellMidle.setY(nmRow);
	calcRectFromPoints(_rectCellsMidle, _cellMidle, _cellStart);

}



/// сигнал о выделениИ документа
void uoReportSelection::selectDocument(){
	clearSelections();
	_selMode = uoRst_Document;
}



} //namespace uoReport
