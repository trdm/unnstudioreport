/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UORPAGESSETTTINGS_H
#define UORPAGESSETTTINGS_H


#include <QPrinter>
#include <QRect>
#include "uoReport.h"

namespace uoReport {

/**
	\struct uorPagePrintSetings - Структура содержащая информацию о некоторых параметрах печати

	Дополнительно содержится расчитанные по областям в соответствии с установленными \n
	тут данными области для печати отчета.
*/

struct uorPagePrintSetings {

	uorPagePrintSetings();
	~uorPagePrintSetings();
	void clear();
	void normalize();

	enum QPrinter::PageSize 	m_paperSize;
	enum QPrinter::Orientation 	m_orientation;
	enum QPrinter::ColorMode 	m_colorMode;

	// Поля в милиметрах...
	int m_fieldLeft_mm; 	///< Поле в милиметрах левое
	int m_fieldTop_mm;		///< Поле в милиметрах верхнее
	int m_fieldRight_mm; 	///< Поле в милиметрах правое
	int m_fieldBottom_mm;	///< Поле в милиметрах нижнее

	// Минимальные значения полей.
	int m_fieldLeft_min; 	///< Поле в милиметрах левое
	int m_fieldTop_min;		///< Поле в милиметрах верхнее
	int m_fieldRight_min; 	///< Поле в милиметрах правое
	int m_fieldBottom_min;	///< Поле в милиметрах нижнее

	int m_scopeType;		///< тип масштабы 1 - точный, 2 - по ширине страницы
	int m_scopeVal;
	void setScope(int sType = 1 , int sVal = 100);
	void setScale(qreal scale = 1.0);
inline const
	bool widthOfSheet() const { return m_scopeType == 2 ? true : false; }

	/**
		Положительный соэффициент масштаба виджета, если он > 0, тогда виджет
		крупнее, если меньше, виджет мельче.
		Высчитываются, если установлен  m_scopeType
	*/
	qreal m_scaleFactor;
inline
	qreal scale() {
		qreal res = 0.0;
		if (m_scopeType == 1){
			res = qreal(m_scopeVal)/100;
			return res;
		}
		return m_scaleFactor;
	}
	void transformPageRect(QRect& rect);

	int m_titleTopSize_mm;
	int m_titleBotSize_mm;
	bool m_saveSettings;
};


} //namespace uoReport

#endif // UORPAGESSETTTINGS_H
