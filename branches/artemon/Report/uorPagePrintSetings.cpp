/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "uorPagePrintSetings.h"
#include <QDebug>


namespace uoReport {

uorPagePrintSetings::uorPagePrintSetings()
{
	clear();
}

uorPagePrintSetings::~uorPagePrintSetings()
{
	clear();
}

/// Нормализует поля структуры (нулевые поля не приветствуются)
void uorPagePrintSetings::normalize()
{
	m_fieldLeft = qMax(1,m_fieldLeft);
	m_fieldTop  = qMax(1,m_fieldTop);
	m_fieldRight  = qMax(1,m_fieldRight);
	m_fieldBottom  = qMax(1,m_fieldBottom);
	m_titleTopSize  = qMax(1,m_titleTopSize);
	m_titleBotSize  = qMax(1,m_titleBotSize);
}


void uorPagePrintSetings::clear()
{
	m_paperSize 	= QPrinter::A4;
	m_orientation 	= QPrinter::Portrait;
	m_colorMode		= QPrinter::Color;
	m_saveSettings	= false;

	m_fieldLeft = m_fieldTop = m_fieldRight = m_fieldBottom = 10;

	m_scopeType = 1;
	m_scopeVal = 100;

	m_titleTopSize = m_titleBotSize = 10;
	m_scaleFactor = (uorNumber)1.0;
}

void uorPagePrintSetings::setScale(uorNumber scale)
{
	m_scaleFactor = qMax((uorNumber)0.1, scale);
 	m_scopeVal = int(100.0 * m_scaleFactor);
}

void uorPagePrintSetings::setScope(int sType, int sVal)
{
//	qDebug() << QString("void uorPagePrintSetings::setScope(int sType = %1, int sVal %2)").arg(sType).arg(sVal);
	m_scopeType =sType;
	switch (sType)
	{
		case 1:
		{
			m_scopeVal = qMax(10,qMin(sVal,500));
			break;
		}
		case 2:
		{
			m_scopeVal = 100;
			break;
		}
		default:
		{
			m_scopeType =1;
			m_scopeVal = 100;
			break;
		}
	}
	m_scaleFactor = uorNumber(m_scopeVal) / 100;
//	qDebug() << QString("m_scopeType = %1, m_scopeVal = %2, m_scaleFactor = %3").arg(m_scopeType).arg(m_scopeVal).arg(m_scaleFactor);
}

void uorPagePrintSetings::transformPageRect(QRect& rect)
{
	if (m_scaleFactor != 1.0) {
		int pos = 0;
		uorNumber scaleFactorO = 1 / m_scaleFactor;
		pos = rect.top();	pos = int(uorNumber(pos)*scaleFactorO);		rect.setTop(pos);
		pos = rect.left();	pos = int(uorNumber(pos)*scaleFactorO);		rect.setLeft(pos);
		pos = rect.bottom();	pos = int(uorNumber(pos)*scaleFactorO);	rect.setBottom(pos);
		pos = rect.right();	pos = int(uorNumber(pos)*scaleFactorO);		rect.setRight(pos);
	}
}

} //namespace uoReport
