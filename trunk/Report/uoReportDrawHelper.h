/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTDRAWHELPER_H
#define UOREPORTDRAWHELPER_H

#include <QtGlobal>
#include <QtGui>
#include "uoReport.h"

class uoReportDoc;
class QApplication;
struct uorReportAreaBase;
struct 	uoCell;
class 	QRectF;


namespace uoReport {

/**
	\class uoReportDrawHelper - вспомогательный класс для рисования очета на разных устройствах

	Необходимо рисовать не только на виджете, но и на пиксмапе и на принтере.
	Скину сюда общие для рисования на всех этих устройствах данные и процедуры.
*/

class uoReportDrawHelper
{
	private:
		uoReportDrawHelper();
	public:
		uoReportDrawHelper(uoReportDoc* doc);
		virtual ~uoReportDrawHelper();
		void clear();
	protected:

	private:
		uoReportDoc* m_doc;
		qreal m_scaleFactor;	///< Положительный соэффициент масштаба виджета, если он > 0, тогда виджет крупнее, если меньше, виджет мельче.

	public:
		inline
		uoReportDoc* getDoc() {return m_doc;}
		void setDoc(uoReportDoc* rptDoc);

		inline
		void setScaleFactor(qreal sFactor){ m_scaleFactor = sFactor;}
		void initDrawInstruments(QWidget* wi = NULL, QPalette* pal = NULL);

		void drawCell(QPainter& painter, uoCell* cell, QRectF& rectCell, uoReportDoc* doc, bool isSell, uorReportAreaBase& drArea);
		void drawDataArea(QPainter& painter, uorReportAreaBase& drArea);
		void drawFields(QPainter& painter);
		inline void setCurCell(QPoint cureCell) {m_curentCell = cureCell;}

		void drawTest(QPainter& painter, qreal offset = 0.0);


	public:
		QBrush m_brushWindow;
		QBrush m_brushBase;
		QBrush m_brushBlack;
		QBrush m_brushSelection;
		QPen m_penText;
		QPen m_penNoPen;
		QPen m_penWhiteText;
		QPen m_penGrey;
		QRectF m_rectCurCell;

		bool m_showInvisiblChar;
		bool m_showGrid;
		bool m_showCurCell;

		bool m_fullPage;		///< Нужно отрисовать полную страницу, а уже на ней данные.
		bool m_printHidenRow;	///< Нужно отрисовывать и скрытые группировками строки.

		QPoint 	m_curentCell; 	///< Текущая ячейка вьюва. есть всегда. Даже когда работаем с картинками.
		uoReportSelection* 	 m_selections;

};
} //namespace uoReport {

#endif // UOREPORTDRAWHELPER_H
