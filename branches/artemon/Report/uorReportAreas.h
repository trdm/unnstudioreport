/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UORREPORTAREAS_H
#define UORREPORTAREAS_H

#include "uoReport.h"
#include "uoReportCtrlItems.h"


namespace uoReport {
/**
	\struct uorReportAreaBase - область отчета, совокупность строк/столбцов.

	На основании uorReportAreaBase можно постороить массив областей,
	которые пойдут на печать. Почему именно на его основании?
	Теоретически допустимы настройки, ширина столбцов, высота строк,
	при которых при определенных масштабах невозможно напечатать целые
	строки/столбцы. Поэтому нам нужен не QRect как таковой (целые значения
	строк/столбцов, а именно значения со смещениями.)
*/


struct uorReportAreaBase
{
	uorReportAreaBase();
	~uorReportAreaBase();
	void clear();
	void 	setShift_RowTop(const uorNumber& val);
	void 	setShift_ColLeft(const uorNumber& val);
	uorNumber 	shift_RowTop() const {return m_shift_RowTop;}
	uorNumber 	shift_ColLeft() const {return m_shift_ColLeft;}

	int firstVisible_RowTop() const {return m_firstVisible_RowTop;}
	void setFirstVisible_RowTop(const int& val );

	int firstVisible_ColLeft() const {return m_firstVisible_ColLeft;}
	void setFirstVisible_ColLeft(const int& val );

	int lastVisibleRow() const {return m_lastVisibleRow;}
	void setLastVisibleRow(const int& val );

	int lastVisibleCol() const {return m_lastVisibleCol;}
	void setLastVisibleCol(const int& val );

	long changesVer() const {return m_changes;};
	void copyTo(uorReportAreaBase& target);

protected:
	inline void change() { m_changes += 1; }
	uorNumber m_shift_RowTop;		///< Смещение первой видимой строки вверх (грубо - размер невидимой/скрытой их части)
	uorNumber m_shift_ColLeft;		///< Смещение первой видимой колонки влево (грубо - размер невидимой/скрытой их части)
	int m_firstVisible_RowTop; 	///< Первая верхняя видимая строка
	int m_firstVisible_ColLeft; ///< Первая левая видимая колонка
	int m_lastVisibleRow; 		///< Последняя верхняя видимая строка
	int m_lastVisibleCol; 		///< Последняя левая видимая колонка
	long m_changes;

public:
	int m_areaType;				///< Тип зоны, нужен для тонкого тюнинга. в это версии 1

	uorRect m_area;
};

struct uorPageColSegment;

struct uorReportPrintArea : public uorReportAreaBase
{
	uorReportPrintArea();
	~uorReportPrintArea();

	/*	Фишка в том, что строки в ячейках вырываются за пределы ячеек
		и что-бы правильно напечатать необходимо либо перебрать либо все
		столбцы, либо только те, которые попадают в область печати.
	*/
	int m_colOverStart;
	int m_colOverEnd;
	int m_pageNumber;
	/*	Если ширина документа превышает одну страницу, получается матрица,
		которая описывает регионы документа. m_pageNumber и m_pageColumn
		определяют место страницы в этой матрице..
	*/
	int m_pageColumn;
	uorPageColSegment* m_segment;
};

/**
	\struct uorPageColSegment - сегменты страниц подготовленных для печати.

	Пояснение: велика вероятность, что колонки таблицы слишком велики для
	того, что-бы поместиться в ширину страницы. необходимо порубить
	на приемлемые участки полотно. эта структура поможет.
*/

struct uorPageColSegment
{
	uorPageColSegment();
	uorPageColSegment(int segmNo, int colS, int colN);
	~uorPageColSegment();
	int m_segmentNom; 	///< номер сегмента,
	int m_colStart;		///< начальная колонка
	int m_colEnd;		///< конечная колонка
	uorNumber m_offsetStart;	///< офсет от начала первой колонки
	uorNumber m_segmWidth;		///< офсет от начала последней колонки
};

typedef QList<uorPageColSegment*> uorPageColSegmentList;
typedef QList<uorPageColSegment*>::iterator uorPageColSegmentListIter;



typedef QLinkedList<uorReportPrintArea*> uorAresList;
typedef QLinkedList<uorReportPrintArea*>::const_iterator uorAresListCnstIter;
typedef QLinkedList<uorReportPrintArea*>::iterator uorAresListIter;
/**
	\struct uorReportViewArea - область отчета с закрепленными строками или столбцами.
	\brief используется для повышения удобства при просмотре отчета: например, фиксируется шапка а тело скролится...

	Что поместить в эту структуру, а что оставить во вьюве?

*/

struct uorReportViewArea : public uorReportAreaBase
{
	uorReportViewArea();
	uorReportViewArea(const uorReportViewArea& re);
	~uorReportViewArea();
	uorReportViewArea& operator=(uorReportViewArea& re);

	void clear();
	void copyTo(uorReportViewArea& target);

	uoRptGroupItemList* m_groupList;		///< список ректов группировок столбцов
	uoRptSectionItemList* m_sectItemList;		///< список итемов секций столбцов
	QString toDebug();

};

} //namespace uoReport

#endif // UORREPORTAREAS_H
