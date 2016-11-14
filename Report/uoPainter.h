/**************************************
*
*	© trdm, as Troshin D. V. 2010
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOPAINTER_H
#define UOPAINTER_H

#include <QPainter>

#ifdef Q_OS_WIN
	#include <windows.h>
#endif


//namespace uoReport {

class uoPainter : public QPainter
{
	public:
		uoPainter();
		uoPainter(QPaintDevice *);
		virtual ~uoPainter();
#ifdef Q_OS_WIN
	void useGDI(bool use, QRect rectWin);
	HDC m_hdc; // дескриптор контекста устройства
#endif
	void scale(qreal sx, qreal sy);
	void setClipRect(const QRectF &, Qt::ClipOperation op = Qt::ReplaceClip);
    void setClipRect(const QRect &, Qt::ClipOperation op = Qt::ReplaceClip);



	protected:
		bool m_useGDI;
		QRect m_rectWin;
		qreal m_scaleX;
		qreal m_scaleY;
	private:
		bool m_clippedRgn;
};
//} //namespace uoReport {

#endif // UOPAINTER_H
