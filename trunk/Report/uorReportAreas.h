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

	qreal m_shift_RowTop;		///< Смещение первой видимой строки вверх (грубо - размер невидимой/скрытой их части)
	qreal m_shift_ColLeft;		///< Смещение первой видимой колонки влево (грубо - размер невидимой/скрытой их части)

	int m_firstVisible_RowTop; 	///< Первая верхняя видимая строка
	int m_firstVisible_ColLeft; ///< Первая левая видимая колонка

	int m_lastVisibleRow; 		///< Последняя верхняя видимая строка
	int m_lastVisibleCol; 		///< Последняя левая видимая колонка
	int m_areaType;				///< Тип зоны, нужен для тонкого тюнинга. в это версии 1

	QRectF m_area;
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
	unsigned int m_colOverStart;
	unsigned int m_colOverEnd;
	unsigned int m_pageNumber;
	/*	Если ширина документа превышает одну страницу, получается матрица,
		которая описывает регионы документа. m_pageNumber и m_pageColumn
		определяют место страницы в этой матрице..
	*/
	unsigned int m_pageColumn;
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
	qreal m_offsetStart;	///< офсет от начала первой колонки
	qreal m_segmWidth;		///< офсет от начала последней колонки
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

	uoRptGroupItemList* m_groupList;		///< список ректов группировок столбцов
	uoRptSectionItemList* m_sectItemList;		///< список итемов секций столбцов

};

} //namespace uoReport

#endif // UORREPORTAREAS_H
