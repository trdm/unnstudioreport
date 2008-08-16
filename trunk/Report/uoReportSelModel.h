/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTSELMODEL_H
#define UOREPORTSELMODEL_H

#include <QObject>
#include <QtCore>
#include "uoReport.h"

namespace uoReport {

class uoReportSelModel : public QObject
{
	Q_OBJECT
	public:
		uoReportSelModel(QObject* parent = 0);
		virtual ~uoReportSelModel();

	public:
		bool isColSelect(int nmCol);
		bool isRowSelect(int nmRow);
		bool isDocumSelect();
		bool isCellSelect(int nmRow, int nmCol);

		void startRowSelected(int nmRow);
		void startColSelected(int nmCol);
		void startCellSelected(int nmCol, int nmRow);
		void selectDocument();

		uoRptSelectionType getSelectionType();

	private:
		QList<int>* 	_selRows; 		///< Список выделенных строк
		QList<int>* 	_selCols;		///< Список выделенных колонок
		QList<QRect*>* _selSpans;		///< Список выделенных областей/спанов
		QList<QRect*>* _selSpansCache;	///< Кешь выделенных областей/спанов

		uoRptSelectionType _startSelMode; ///< Режим начала выделения диапазона.
		uoRptSelectionType _selMode; ///< Режим начала выделения диапазона.

		/*
			Стартовые объекты для режима выделения.
			Для выделения строк/колонок _strartColRow
			Для выделения ячеек _strartSellX и _strartSellY
		*/

		int _strartColRow;	///< Стартовое значение строки/столбца..

		// для режима начала выделения диапазона ячеек
		int _strartSellX;	///< Стартовое значение столбца..
		int _strartSellY;	///< Стартовое значение строки..
};

} //namespace uoReport
#endif // UOREPORTSELMODEL_H
