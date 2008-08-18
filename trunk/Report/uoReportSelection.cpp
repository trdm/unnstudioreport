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
	_selRows = new QList<int>;
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
	delete _selCols;
	while (!_selSpans->isEmpty())		delete _selSpans->takeFirst();
	while (!_selSpansCache->isEmpty())	delete _selSpansCache->takeFirst();
	delete _selSpans;
	delete _selSpansCache;
}

/// Очистка всех даных по выделениям
void uoReportSelection::clearSelections(uoRptSelectionType exclude)
{
	if (exclude != uoRst_Rows) 		_selRows->clear();
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
	} else if (!_selRows->isEmpty()) {
		retVal = _selRows->contains(nmRow);
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

}

/// сигнал о начале выделения колонок
void uoReportSelection::startColSelected(int nmCol){
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
