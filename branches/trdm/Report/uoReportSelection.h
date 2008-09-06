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
#include <QApplication>
#include "uoReport.h"

namespace uoReport {

class uoReportSelection : public QObject
{
	Q_OBJECT
	public:
		uoReportSelection(QObject* parent = 0);
		virtual ~uoReportSelection();

	public:
		void clearSelections(uoRptSelectionType exclude = uoRst_Unknown);

		bool isColSelect(int nmCol);
		bool isRowSelect(int nmRow);
		bool isDocumSelect();
		bool isCellSelect(int nmRow, int nmCol);

		bool isCtrlPress();
		bool isShiftPress();

		void startRowSelected(int nmRow);
		void midleRowSelected(int nmRow);
		void endRowSelected(int nmRow);

		void startColSelected(int nmCol);
		void endColSelected(int nmCol);

		void startCellSelected(int nmCol, int nmRow);
		void selectDocument();
		void selectRow(int nmRow);
		void selectCol(int nmCol);

		uoRptSelectionType getSelectionType();

	private:
		/*
			зачем нужны временные выделенные строки?
			временные выделенные строки возникают когда пользователь откнопал
			с нажатой клавишей контрол несколько строк, а теперь хочет выделить
			интервал. и он нажимает на ячейку линейки и тащит мышку не отпуская.
			в это время он может перекрыть уже выделенные строки, но потом вернуться
			обратно, покинув их. выделение должно сохраниться.
			поэтому я работаю не с _selRows а с _selRowsColsTmp.
		*/
		QList<int>* 	_selRows; 		///< Список выделенных строк
		QList<int>* 	_selRowsColsTmp; 	///< Список временных выделенных строк
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
