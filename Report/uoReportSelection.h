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
		void clearSelections(uorSelectionType exclude = uoRst_Unknown);

		bool isColSelect(int nmCol);
		bool isRowSelect(int nmRow);
		bool isDocumSelect();
		bool isCellSelect(int nmRow, int nmCol);

		bool isTrueForSections();
		bool getTrueSectionsCR(uoRptHeaderType& rht, int& start, int& end) const;
		bool getSelectedColRow(uoRptHeaderType& rht, QList<int>& list) const;

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
		bool cellStartIsNull() { return m_cellStart.isNull();}

		QRect* getCellSpan();
		QPoint* getCellPoint();

		uorSelectionType getSelectionType();
		uorSelectionType getStartSelectionType();

		bool calcRectFromPoints(QRect& rct, const QPoint& posStart, const QPoint& posEnd) const;
		void refreshSelectionType();
		void setSelectionMode(uorSelectionType sMode);
		QRect getSelectionBound();

	signals:
		void onSelectonChange(const uorSelectionType& sModeOld, const uorSelectionType& sModeNew);

	private:
		/*
			зачем нужны временные выделенные строки?
			временные выделенные строки возникают когда пользователь откнопал
			с нажатой клавишей контрол несколько строк, а теперь хочет выделить
			интервал. и он нажимает на ячейку линейки и тащит мышку не отпуская.
			в это время он может перекрыть уже выделенные строки, но потом вернуться
			обратно, покинув их. выделение должно сохраниться.
			поэтому я работаю не с m_selRows а с m_selRowsColsTmp.
		*/
		QList<int>* 	m_selRows; 			///< Список выделенных строк
		QList<int>* 	m_selRowsColsTmp; 	///< Список временных выделенных строк
		QList<int>* 	m_selCols;			///< Список выделенных колонок
		QList<QRect*>*  m_selSpans;			///< Список выделенных областей/спанов
		QList<QRect*>*  m_selSpansCache;	///< Кешь выделенных областей/спанов

		QList<QPoint*>* m_selPoints;		///< Список выделенных ячеек
		QList<QPoint*>* m_selPointsCache;	///< Кешь выделенных ячеек

		uorSelectionType m_startSelMode; 	///< Режим начала выделения диапазона.
		uorSelectionType m_selMode; 		///< Режим начала выделения диапазона.

		/*
			Стартовые объекты для режима выделения.
			Для выделения строк/колонок m_strartColRow
			Для выделения ячеек _strartSellX и _strartSellY
		*/

		int m_strartColRow;	///< Стартовое значение строки/столбца..

		// для режима начала выделения диапазона ячеек
		QPoint m_cellStart; ///< Стартовое значение при начале выделения ячеек. x-колонка/y-строка.
		QPoint m_cellMidle; ///< Временное конечное значение ячейки при выделения ячеек с пом. КПК.
		QRect m_rectCellsMidle; ///< Временное конечное значение ячейки при выделения ячеек с пом. КПК.
};

} //namespace uoReport
#endif // UOREPORTSELMODEL_H
