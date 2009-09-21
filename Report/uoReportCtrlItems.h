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
	QRectF 	m_rectIteract; 	///< область "кнопки" свертки/развертки структуры.
	bool m_folded; 			///< уровень группировки.
	int m_level; 			///< уровень группировки.
	int m_start; 			///< Начало диапазона.
	int m_end;				///< Конец диапазона.
	int m_id;				///< Идентификатор гроуп итема.
	qreal	m_rectEndPos; 	///< Координаты правой|нижней стороны ректа. для расчета длины линии группировки.
	qreal	m_rectMidlePos; 	///< Координаты середины правой|нижней стороны ректа для вычерчивания горизонтальной линии группировки.
	qreal m_sizeTail;		///< Размер "хвоста" группировки.
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
		m_rectEndPos = 0.0;
		m_rectMidlePos = 0.0;
		m_sizeTail = 0.0;
		m_rectIteract.setTop(0.0);
		m_rectIteract.setRight(0.0);
		m_rectIteract.setBottom(0.0);
		m_rectIteract.setLeft(0.0);
	}
};

typedef QList<uoRptGroupItem*> uoRptGroupItemList;
typedef QMap<int, qreal> rptScalePositionMap; ///< словарь смещений ячеек линеек.

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
		,m_nameSections(0)
		{}
	QRectF 	m_rectView; 		///< область "кнопки" свертки/развертки структуры.
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
		m_nameSections	= spn->_name;

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
		m_rectView.setTop(0.0);
		m_rectView.setRight(0.0);
		m_rectView.setBottom(0.0);
		m_rectView.setLeft(0.0);
	}
};

typedef QList<uoRptSectionItem*> uoRptSectionItemList;

} //namespace uoReport

#endif // UOREPORTCTRLITEMS_H
