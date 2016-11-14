/**************************************
*
*	В© trdm, as Troshin D. V. 2008
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
	m_fieldLeft_mm = qMax(1,m_fieldLeft_mm);
	m_fieldTop_mm  = qMax(1,m_fieldTop_mm);
	m_fieldRight_mm  = qMax(1,m_fieldRight_mm);
	m_fieldBottom_mm  = qMax(1,m_fieldBottom_mm);
	m_titleTopSize_mm  = qMax(1,m_titleTopSize_mm);
	m_titleBotSize_mm  = qMax(1,m_titleBotSize_mm);
}


void uorPagePrintSetings::clear()
{
	m_paperSize 	= QPrinter::A4;
	m_orientation 	= QPrinter::Portrait;
	m_colorMode		= QPrinter::Color;
	m_saveSettings	= false;

	m_fieldLeft_mm = m_fieldTop_mm = m_fieldRight_mm = m_fieldBottom_mm = 10;

	m_scopeType = 1;
	m_scopeVal = 100;

	m_titleTopSize_mm = m_titleBotSize_mm = 10;
	m_scaleFactor = (uorNumber)1.0;
}

void uorPagePrintSetings::setScale(qreal scale)
{
    m_scaleFactor = qMax(0.1, scale);
    //int sf = int(m_scaleFactor *100 );
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
			m_scopeVal = qMax(15,qMin(sVal,500));
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
	m_scaleFactor = qreal(m_scopeVal) / 100;
//	qDebug() << QString("m_scopeType = %1, m_scopeVal = %2, m_scaleFactor = %3").arg(m_scopeType).arg(m_scopeVal).arg(m_scaleFactor);
}

void uorPagePrintSetings::transformPageRect(QRect& rect)
{
	if (m_scaleFactor != 1.0 && m_scaleFactor != 0.0) {
		int pos = 0;
		qreal scaleFactorO = 1 / m_scaleFactor;
		pos = rect.top();	pos = int(uorNumber(pos)*scaleFactorO);		rect.setTop(pos);
		pos = rect.left();	pos = int(uorNumber(pos)*scaleFactorO);		rect.setLeft(pos);
		pos = rect.bottom();	pos = int(uorNumber(pos)*scaleFactorO);	rect.setBottom(pos);
		pos = rect.right();	pos = int(uorNumber(pos)*scaleFactorO);		rect.setRight(pos);
	}
}

} //namespace uoReport
