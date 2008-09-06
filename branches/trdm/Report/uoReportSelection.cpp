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
	_strartSellX	= -1;
	_strartSellY	= -1;
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

/// Очистка всех даных по выделениям
void uoReportSelection::clearSelections(uoRptSelectionType exclude)
{
	if (exclude != uoRst_Rows) {
		_selRows->clear();
		_selRowsColsTmp->clear();
	}
	if (exclude != uoRst_Columns)	_selCols->clear();
	_selMode		= uoRst_Unknown;
	_startSelMode	= uoRst_Unknown;
	_strartColRow	= -1;
	_strartSellX	= -1;
	_strartSellY	= -1;
	if (exclude != uoRst_Cells) {
		while (!_selSpans->isEmpty())
			_selSpansCache->append(_selSpans->takeFirst());
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

/// сигнал о начале выделения строк.
void uoReportSelection::startRowSelected(int nmRow){
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
void uoReportSelection::midleRowSelected(int nmRow)
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
void uoReportSelection::endRowSelected(int nmRow)
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
void uoReportSelection::startColSelected(int nmCol){
	if (!isCtrlPress()){
		clearSelections();
		selectCol(nmCol);
	}
	_strartColRow = nmCol;
	_selMode = uoRst_Column;
}

void uoReportSelection::endColSelected(int nmCol)
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
void uoReportSelection::startCellSelected(int nmCol, int nmRow){
}

/// сигнал о выделениИ документа
void uoReportSelection::selectDocument(){
	clearSelections();
	_selMode = uoRst_Document;
}



} //namespace uoReport
