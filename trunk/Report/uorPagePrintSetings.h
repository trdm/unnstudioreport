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

	int m_fieldLeft;
	int m_fieldTop;
	int m_fieldRight;
	int m_fieldBottom;

	int m_scopeType;
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
		if (m_scopeType == 1){
			return qreal(m_scopeVal)/100;
		}
		return m_scaleFactor;
	}
	void transformPageRect(QRect& rect);

	int m_titleTopSize;
	int m_titleBotSize;
	bool m_saveSettings;
};


} //namespace uoReport

#endif // UORPAGESSETTTINGS_H
