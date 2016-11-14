/**************************************
*
*	© trdm, as Troshin D. V. 2010
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoPainter.h"
#include <QPaintEngine>

//namespace uoReport {

uoPainter::uoPainter()
{	//ctor
	m_useGDI = false;
	m_clippedRgn = false;
}
uoPainter::uoPainter(QPaintDevice * pd)
	:QPainter(pd)
{	//ctor
	m_useGDI = false;
	m_clippedRgn = false;
}


uoPainter::~uoPainter()
{
	if (m_useGDI) {
#ifdef Q_OS_WIN
		QPainter::paintEngine()->releaseDC(m_hdc);
#endif
	}
}

#ifdef Q_OS_WIN
void uoPainter::useGDI(bool use, QRect rectWin)
{
	m_useGDI = use;
	m_rectWin = rectWin;
	if (m_useGDI){
		m_hdc = QPainter::paintEngine()->getDC();
	}
}
#endif

void uoPainter::scale(qreal sx, qreal sy)
{
	QPainter::scale(sx, sy);
}

void uoPainter::setClipRect(const QRectF &rect, Qt::ClipOperation op)
{
	QPainter::setClipRect(rect, op);
}
void uoPainter::setClipRect(const QRect &rect, Qt::ClipOperation op){
	QPainter::setClipRect(rect, op);
}

//} //namespace uoReport {
