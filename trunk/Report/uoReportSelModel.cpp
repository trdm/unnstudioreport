/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReportSelModel.h"

namespace uoReport{

/**
	\class uoReportSelModel - объект хранения и обработки выделения объектов в uoReportCtrl
	\brief uoReportSelModel - объект хранения и обработки выделения объектов в uoReportCtrl

	см. описание в uoReportDescr.h
*/

uoReportSelModel::uoReportSelModel(QObject* parent)
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

uoReportSelModel::~uoReportSelModel()
{
	delete _selRows;
	delete _selCols;
	while (!_selSpans->isEmpty())		delete _selSpans->takeFirst();
	while (!_selSpansCache->isEmpty())	delete _selSpansCache->takeFirst();
	delete _selSpans;
	delete _selSpansCache;
}

/// Проверка на выделенность столбца
bool uoReportSelModel::isColSelect(int nmCol)
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
bool uoReportSelModel::isRowSelect(int nmRow)
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
bool uoReportSelModel::isDocumSelect()
{
	if (_selMode == uoRst_Document){
		return true;
	} else {
		return false;
	}
}

/// Проверка на выделенность ячейки.
bool uoReportSelModel::isCellSelect(int nmRow, int nmCol)
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

/// Возвращает текуший тип выделения.
uoRptSelectionType uoReportSelModel::getSelectionType(){
	return _selMode;
}

/// сигнал о начале выделения строк.
void startRowSelected(int nmRow){
}

/// сигнал о начале выделения колонок
void startColSelected(int nmCol){
}

/// сигнал о начале выделения диапазона ячеек.
void startCellSelected(int nmCol, int nmRow){
}

/// сигнал о выделениИ документа
void selectDocument(){
}



} //namespace uoReport
