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

		bool isTrueForSections();
		bool getTrueSectionsCR(uoRptHeaderType& rht, int& start, int& end);

		bool isCtrlPress();
		bool isShiftPress();

		void rowSelectedStart(int nmRow);
		void rowSelectedMidle(int nmRow);
		void rowSelectedEnd(int nmRow);

		void colSelectedStart(int nmCol);
		void colSelectedEnd(int nmCol);

		void selectDocument();
		void selectRow(int nmRow);
		void selectCol(int nmCol);

		void cellSelectedStart(int nmCol, int nmRow);
		void cellSelectedEnd(int nmCol, int nmRow);
		void cellSelectedMidle(int nmCol, int nmRow);

		void selectCell(int nmCol, int nmRow);
		bool cellStartIsNull() { return _cellStart.isNull();}

		QRect* getCellSpan();
		QPoint* getCellPoint();

		uoRptSelectionType getSelectionType();
		uoRptSelectionType getStartSelectionType();

		bool calcRectFromPoints(QRect& rct, const QPoint& posStart, const QPoint& posEnd) const;
		void refreshSelectionType();
		void setSelectionMode(uoRptSelectionType sMode);

	signals:
		void onSelectonChange(const uoRptSelectionType& sModeOld, const uoRptSelectionType& sModeNew);

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
		QList<int>* 	_selRows; 			///< Список выделенных строк
		QList<int>* 	_selRowsColsTmp; 	///< Список временных выделенных строк
		QList<int>* 	_selCols;			///< Список выделенных колонок
		QList<QRect*>* _selSpans;			///< Список выделенных областей/спанов
		QList<QRect*>* _selSpansCache;		///< Кешь выделенных областей/спанов

		QList<QPoint*>* _selPoints;			///< Список выделенных ячеек
		QList<QPoint*>* _selPointsCache;	///< Кешь выделенных ячеек

		uoRptSelectionType _startSelMode; 	///< Режим начала выделения диапазона.
		uoRptSelectionType _selMode; 		///< Режим начала выделения диапазона.

		/*
			Стартовые объекты для режима выделения.
			Для выделения строк/колонок _strartColRow
			Для выделения ячеек _strartSellX и _strartSellY
		*/

		int _strartColRow;	///< Стартовое значение строки/столбца..

		// для режима начала выделения диапазона ячеек
		QPoint _cellStart; ///< Стартовое значение при начале выделения ячеек. x-колонка/y-строка.
		QPoint _cellMidle; ///< Временное конечное значение ячейки при выделения ячеек с пом. КПК.
		QRect _rectCellsMidle; ///< Временное конечное значение ячейки при выделения ячеек с пом. КПК.
};

} //namespace uoReport
#endif // UOREPORTSELMODEL_H
