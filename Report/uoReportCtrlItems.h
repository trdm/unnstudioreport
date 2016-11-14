/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTCTRLITEMS_H
#define UOREPORTCTRLITEMS_H

#include <QtCore>
#include "uoSpanTree.h"

namespace uoReport {

///\struct uoRptGroupItem
///\brief Координатное описание группировки строк/столбцов, подготовленное для рендринга.
struct uoRptGroupItem {
	uoRptGroupItem()
		: m_folded(false)
		,m_level(-1)
		,m_start(-1)
		,m_end(-1)
		,m_id(-1)
		,m_rectEndPos(0)
		,m_rectMidlePos(0)
		,m_sizeTail(0)
		,m_tailPosIsAbs(false)
		{}
	uorRect 	m_rectIteract; 	///< область "кнопки" свертки/развертки структуры.
	bool m_folded; 			///< уровень группировки.
	int m_level; 			///< уровень группировки.
	int m_start; 			///< Начало диапазона.
	int m_end;				///< Конец диапазона.
	int m_id;				///< Идентификатор гроуп итема.
	uorNumber	m_rectEndPos; 	///< Координаты правой|нижней стороны ректа. для расчета длины линии группировки.
	uorNumber	m_rectMidlePos; 	///< Координаты середины правой|нижней стороны ректа для вычерчивания горизонтальной линии группировки.
	uorNumber 	m_sizeTail;		///< Размер "хвоста" группировки.
	bool 	m_tailPosIsAbs;	///< Размер "хвоста" указан относительно левой или верхней сторони ректа группировок

	/// копируем данные из uoLineSpan
	void copyFrom(uoLineSpan* spn){
		m_start 	= spn->getStart();
		m_end 	= spn->getEnd();

		m_folded = spn->getFolded();
		m_level	= spn->getLevel();

		m_id 	= spn->getId();
	}
	/// чистка итема....
	void clear(){
		m_folded = false;
		m_level = -1;
		m_start = -1;
		m_end = -1;
		m_id = -1;
		m_rectEndPos = uorNumberNull;
		m_rectMidlePos = uorNumberNull;
		m_sizeTail = uorNumberNull;
		m_rectIteract.setTop(uorNumberNull);
		m_rectIteract.setRight(uorNumberNull);
		m_rectIteract.setBottom(uorNumberNull);
		m_rectIteract.setLeft(uorNumberNull);
	}
};

typedef QList<uoRptGroupItem*> uoRptGroupItemList;
typedef QMap<int, uorNumber> rptScalePositionMap; ///< словарь смещений ячеек линеек.

/**
	\struct uoRptSectionItem - структура для сохранения гуи-координат и атрибутов секции отчета.
	\brief Используется в uoReportCtr для отрисовки секций.
*/
struct uoRptSectionItem
{
		uoRptSectionItem()
		:m_level(-1)
		,m_start(-1)
		,m_end(-1)
		,m_id(-1)
		,m_nameSections("")
		{}
	uorRect 	m_rectView; ///< область отрисовки текста.
	int 	m_level; 		///< уровень группировки.
	int 	m_start; 		///< Начало диапазона.
	int 	m_end;			///< Конец диапазона.
	int 	m_id;			///< Идентификатор гроуп итема.
	bool 	m_selected;		///< Секция выделенна.
	QString m_nameSections;	///< Имя секции.

		/// копируем данные из uoLineSpan
	void copyFrom(uoLineSpan* spn){
		m_start 	= spn->getStart();
		m_end 	= spn->getEnd();

		m_level	= spn->getLevel();
		m_nameSections	= spn->m_name;

		m_id 	= spn->getId();
	}

	/// чистка итема....
	void clear(){
		m_level = -1;
		m_start = -1;
		m_end = -1;
		m_id = -1;
		m_nameSections = "";
		m_selected = false;
		m_rectView.setTop(uorNumberNull);
		m_rectView.setRight(uorNumberNull);
		m_rectView.setBottom(uorNumberNull);
		m_rectView.setLeft(uorNumberNull);
	}
};

typedef QList<uoRptSectionItem*> uoRptSectionItemList;

} //namespace uoReport

#endif // UOREPORTCTRLITEMS_H
