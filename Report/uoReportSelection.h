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
#include "uoSpanTree.h"
#include "uoReportLoader.h"
#include "uoReportDoc.h"

namespace uoReport {

class uoReportSelection : public QObject
{
	Q_OBJECT
	public:
		uoReportSelection(QObject* parent = 0);
		virtual ~uoReportSelection();
		void 	setDoc(uoReportDoc*	doc) {m_doc = doc;}

	public:
		void clearSelections(uorSelectionType exclude = uoRst_Unknown);

		bool isColSelect(const int& nmCol) const;
		bool isRowSelect(const int& nmRow) const;

		bool isColPartlySelect(const int& nmCol) const;
		bool isRowPartlySelect(const int& nmRow) const;

inline	bool isDocumSelect()  const {	if (m_selMode == uoRst_Document){		return true;	} else {		return false;	}}

		bool isCellSelect(const int& nmRow, const int& nmCol)  const;
		bool isCurrentCell(int nmRow, int nmCol)  const;


		bool isTrueForSections()  const;
		bool isTrueForCopy()  const;
		bool isTrueForJoin()  const;

		bool getTrueSectionsCR(uoRptHeaderType& rht, int& start, int& end) const;
		bool getSelectedColRow(uoRptHeaderType& rht, QList<int>& list) const;

		bool isSpanFullSeleced(uoLineSpan* spn, const uoRptHeaderType& curHeaderType) const;

		void saveMidleRowCol();

		bool isCtrlPress();
		bool isShiftPress();

		int  rowCount() const;
		int  colCount() const;

		void rowSelectedStart(int nmRow);
		void rowSelectedMidle(int nmRow);
		void rowSelectedEnd(int nmRow);

		void colSelectedStart(int nmCol);
		void colSelectedMidle(int nmCol);
		void colSelectedEnd(int nmCol);

		void selectDocument();
		void selectRow(int nmRow, bool clearSel = true);
		void selectCol(int nmCol, bool clearSel = true);

		void cellSelectedStart(int nmCol, int nmRow);
		void cellSelectedMidle(int& nmCol, int& nmRow);
		void cellSelectedEnd(int& nmCol, int& nmRow);
		void accumulateCellSelection();

		void refreshPartlySelRC();
		void onRectSelected(QRect* selRect);

		void selectCell(int nmCol, int nmRow, bool clearSel = true);
		bool cellStartIsNull() { return m_cellStart.isNull();}

		QPoint	getMinMaxRC(const uoRptHeaderType curHeaderType) const;
		QPoint getInsertPoint() const;


		QPoint currentCell() const;
		QRect currentRect() const;
		void currentCell(int& row, int& col) const;
		void setCurrentCell(QPoint pnt, int rowsJn = 0, int colsJn = 0);
		void setCurrentCell(int row, int col, int rowsJn = 0, int colsJn = 0);

		uorSelectionType selectionType();
		uorSelectionType getStartSelectionType();
		uorSelectionType convertIntToSelMode(const int val) const;

		bool calcRectFromPoints(QRect& rct, const QPoint& posStart, const QPoint& posEnd) const;
		void refreshSelectionType();
		void setSelectionMode(uorSelectionType sMode);
		QRect getSelectionBound() const;

		void saveSelection(uoReportLoader* loader);
		void copyFrom(uoReportSelection* selOther, int rowOffset, int colOffset);

	private:
		QRect* 	getCellSpanNew();
		QPoint* getCellPoint();


	signals:
		void onSelectonChange(const uorSelectionType& sModeOld, const uorSelectionType& sModeNew);

	private:
		bool getMinMax(int& minVal, int& maxVal, QList<int>* list) const;
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
		int 			m_selSpansCount;	///< Кличество ректов в спане(оптимизадница: устраняем вызов isEmpty() > кушает зазря тики процессора)
		QList<QRect*>*  m_selSpansCache;	///< Кешь выделенных областей/спанов

		QList<QPoint*>* m_selPoints;		///< Список выделенных ячеек
		int 			m_selPointsCount;	///< Кличество поинтов в спане(оптимизадница: устраняем вызов isEmpty() > кушает зазря тики процессора)
		QList<QPoint*>* m_selPointsCache;	///< Кешь выделенных ячеек

		uorSelectionType m_startSelMode; 	///< Режим начала выделения диапазона.
		uorSelectionType m_selMode; 		///< Режим начала выделения диапазона.

		// Вспомогательные инструменты. (m_selRowsCell/m_selColsCell)
		QList<int>* 	m_selRowsCell;		///< Список строк c выделенными ячейками
		QList<int>* 	m_selColsCell;		///< Список колонок c выделенными ячейками

		uoReportDoc*	m_doc;

		/*
			Стартовые объекты для режима выделения.
			Для выделения строк/колонок m_strartColRow
			Для выделения ячеек _strartSellX и _strartSellY
		*/

		int m_strartColRow;	///< Стартовое значение строки/столбца..

		// для режима начала выделения диапазона ячеек
		QPoint m_currentCell; 	///< Текущее значение ячейки вьюва: Х - столбец, Y - строка
		QRect m_currentRect;	///< Если текущая ячейка - чать объединения, тут лежит рект.

		QPoint m_cellStart; 	///< Стартовое значение при начале выделения ячеек. x-колонка/y-строка.
		QPoint m_cellMidle; 	///< Временное конечное значение ячейки при выделения ячеек с пом. КПК.
		QRect m_rectCellsMidle; ///< Временное конечное значение ячейки при выделения ячеек с пом. КПК.
};

} //namespace uoReport
#endif // UOREPORTSELMODEL_H
