/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include <QApplication>
#include "uoReportDrawHelper.h"
#include "uoReportDocBody.h"
#include "uoReportCtrl.h"

namespace uoReport {


inline void zeroQRectF(QRectF& rct) { rct.setLeft(0);	rct.setTop(0);	rct.setRight(0);	rct.setBottom(0); }

uoReportDrawHelper::uoReportDrawHelper()
{
	//ctor
	zeroQRectF(m_rectCurCell);
}

uoReportDrawHelper::uoReportDrawHelper(uoReportDoc* doc)
	: m_doc(doc)
{

	clear();
}


uoReportDrawHelper::~uoReportDrawHelper()
{
	//dtor
}

void uoReportDrawHelper::clear()
{
	m_showInvisiblChar = false;
	m_selections = NULL;
	m_showGrid = false;
	m_showCurCell = false;
	m_fullPage = false;
	m_printHidenRow = false;

}

void uoReportDrawHelper::initDrawInstruments(QWidget* wi, QPalette* pal)
{
	QPalette palette_c;
	if (!pal){
		palette_c = qApp->palette();
	}else{
		palette_c = *pal;
	}

	QPalette::ColorGroup curColGrp = QPalette::Active;
	if (wi) {
		if (!wi->isEnabled()) {
			curColGrp = QPalette::Disabled;
		}
	}

	// нормальный фон окна.
   	m_brushWindow = palette_c.brush(QPalette::Window); //	brushWindow.setColor(palette_c.color(curColGrp, QPalette::Window));

   	m_brushBase = palette_c.brush(QPalette::Base /*Qt::white*/); // QPalette::Base - типа белый для текста.
   	m_brushBlack = palette_c.brush(QPalette::WindowText /*Qt::white*/);
   	m_brushSelection = palette_c.brush(QPalette::Highlight /*Button*/ /*Qt::white*/);
	// Чуток осветлим выделение..
   	QColor brCol = m_brushSelection.color();
   	brCol.setAlpha(124);
   	m_brushSelection.setColor(brCol);

	m_penText.setColor(palette_c.color(curColGrp, QPalette::WindowText));
	m_penNoPen.setColor(palette_c.color(curColGrp, QPalette::Window));
	m_penWhiteText.setColor(palette_c.color(curColGrp, QPalette::Base));
	m_penGrey.setColor(palette_c.color(curColGrp, QPalette::Window));
	m_fullPage = false;
	m_printHidenRow = false;

}

void uoReportDrawHelper::drawTest(QPainter& painter, qreal offset)
{
	QLineF line(200.0-offset, 200.0+offset, 300.0-offset, 300.0+offset);
	painter.drawLine(line);
}

/// Отрисовка ячейки таблицы
void uoReportDrawHelper::drawCell(QPainter& painter, uoCell* cell,QRectF& rectCell, uoReportDoc* doc, bool isSell, uorReportAreaBase& drArea)
{
	if (!cell)
		return;

	QRectF mainArea = drArea.m_area;

	QString text = cell->getTextWithLineBreak(m_showInvisiblChar);
	if (!text.isEmpty()) {


		uoCellTextBehavior textBhv = cell->getTextBehavior();
		uoCellTextType textTp = cell->getTextType();


		text = text.replace('\t', " ");
		painter.save();
		QRectF rectCpyCell = rectCell;

		qreal adj = UORPT_STANDART_OFFSET_TEXT;
		rectCpyCell.adjust(adj,0,-adj,0); /// иначе текст перекрывается..

		int flags = cell->getAlignment();
		QPen oldPen = painter.pen();
		QFont* font = cell->getFont(doc);
		if (font)
			painter.setFont(*font);
		painter.setPen(m_penText);
		/*
			тут интересная байда. если текст длинен, тогда он может печататься и
			поверх остальных ячеек, но нам передается область его отсечения.
			т.е. рект ячейки... как его правильно распечатать в нужном месте/нужным образом?
			Очевидным ответом будет расширение ректа в стророну выравнивания на длинну строки...
		*/

		qreal maxRowLendht = cell->getMaxRowLength();
		QString addChrs;

		if (uoCTT_Templ == textTp )	{
			text = "{" + text + "}";
			addChrs = addChrs + "{}";
		} else if (uoCTT_Expr  == textTp) {
			text = "<" + text + ">";
			addChrs = addChrs + "<>";
		}
		if (!addChrs.isEmpty() && font)
		{
			QFontMetricsF fm(*font);
			maxRowLendht += fm.width(addChrs);
		}

		qreal diffSize = maxRowLendht - rectCpyCell.width();
		if (diffSize > 1.0 /*&& textTp == uoCTT_Text*/){

			if (textBhv == uoCTB_Auto){
				uoHorAlignment ha = cell->getAlignmentHor();
				qreal pointPos = 0.0, diffSize = maxRowLendht - rectCpyCell.width();
				switch (ha){
					case uoHA_Left:
					{
						// значит надо двигать правый угол. вроде...
						pointPos = rectCpyCell.right() + diffSize;
						pointPos = qMin(pointPos, mainArea.right());
						rectCpyCell.setRight(pointPos);
						break;
					}
					case uoHA_Right:
					{
						// значит надо двигать левый угол. вроде...
						pointPos = rectCpyCell.left() - diffSize;
						pointPos = qMin(pointPos, mainArea.left());
						rectCpyCell.setLeft(pointPos);
						break;
					}
					case uoHA_Center:
					{
						// значит надо двигать левый и правый угол. вроде...
						// значит надо двигать правый угол. вроде...
						diffSize = diffSize / 2;
						rectCpyCell.adjust(-diffSize, 0, diffSize, 0);

						break;
					}
					case uoHA_Unknown:
						break;
				}
			} else
			if (textBhv == uoCTB_Obstruct)
			{
				QChar ch;
				/// Забивать.... и забиваем с учетом переноса строк.
				for (int u = 0; u<text.size(); u++){
					ch = text.at(u);
					if (!ch.isSpace())
						ch = '#';
					text[u] = ch;
				}
			}
		}


		painter.drawText(rectCpyCell,flags,text);
		painter.setPen(oldPen);
		painter.restore();
	}
}



/// Отрисовка поля данных.
void uoReportDrawHelper::drawDataArea(QPainter& painter, uorReportAreaBase& drArea)
{
	/*	попробуем порисовать основное поле с данными...
		вот тут нужен такой фокус, необходимо научиться рисовать
		отсеченные части ректов и прочих фигур.	*/

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;



	m_penText.setStyle(Qt::SolidLine);
	painter.setPen(m_penText);

	//drawTest(painter, 60);

	QRectF rectCell;
	QRectF rectCellCur; // ячейка на которой курсор стоит...
	zeroQRectF(rectCell);

	bool isSell = false;
	bool isHide = false;

	// нарисуем рамку области данных, т.к. потом будем рисовать линии на ней в этой процедурине.
	// painter.drawRect(m_rectDataRegionFrame);

	QRectF drawingRect = drArea.m_area;

	painter.fillRect(drawingRect, m_brushBase);
	QPen oldPen = painter.pen();
	painter.setPen(m_penGrey);
	/*
		при рисовании на пиксмепе почему-то если используется
		painter.setClipRect(drawingRect);
		тогда не рисуется вообще О_о
	*/
	int rowCur = drArea.m_firstVisible_RowTop;
	int colCur = 1; //drArea.m_firstVisible_ColLeft;

	int colPrev = -1;
	if (drArea.m_areaType == 2)		drawingRect.adjust(-2,-2,2,2);

	painter.setClipRect(drawingRect); // Устанавливаем область прорисовки. Будем рисовать только в ней.

	if (drArea.m_areaType == 2)	{
		drawingRect.adjust(2,2,-2,-2);
		colCur = drArea.m_firstVisible_ColLeft;
	}

	qreal rowsLenCur = drawingRect.top() - drArea.m_shift_RowTop; // + 1 * m_scaleFactorO;
	qreal rowsLensPage = drawingRect.bottom();

	qreal colsLenCur = drawingRect.left() - drArea.m_shift_ColLeft;
	qreal colsLensPage = drawingRect.right();
	rectCell.setTop(rowsLenCur);
	rectCell.setLeft(drawingRect.left() - drArea.m_shift_ColLeft);
	QString cellTest;
	uoCell* curCell = NULL;

	/* надо изменить логику перерисовки по строкам.
	причина: можем не прорисовать длинные строки, которые начинаются за
	видимыми областями и прорисовываются на видимой */

	qreal leftInvOffset = 0.0; // скока у нас слева скрыто.
	for(int t = 1; t<drArea.m_firstVisible_ColLeft; t++)
	{
		if (!doc->getScaleHide(uorRhtColumnHeader, t))
			leftInvOffset += doc->getScaleSize(uorRhtColumnHeader, t);
	}

	qreal sz = 0.0;

	uoHorAlignment hCellAllign = uoHA_Unknown;

	do {	// строки, строки, строки и строки =============
		while((isHide = doc->getScaleHide(uorRhtRowsHeader, rowCur))){
			++rowCur;
		}
		sz = doc->getScaleSize(uorRhtRowsHeader, rowCur);
		if (sz == 0.0) {
			++rowCur;
			continue;
		}
		if (drArea.m_areaType == 2) {
			// эта область типа uorReportPrintArea, тут вывод ограничен строкой и столбцом.
			if (drArea.m_lastVisibleRow < rowCur)
				break;
		}
		rowsLenCur = rowsLenCur + sz;
		rectCell.setBottom(rowsLenCur);

		colCur = drArea.m_firstVisible_ColLeft;
		for ( int i = 0; i < 2; i++) {
			// 2 цикла нужны для того, что-бы первый раз отрисовать рамки, а на второй текст.
			if (i == 0) {
				colCur = drArea.m_firstVisible_ColLeft;
				colsLenCur = drawingRect.left() - drArea.m_shift_ColLeft;
			} else if (i == 1) {
				colCur = 1;
				colsLenCur = -leftInvOffset + drawingRect.left() - drArea.m_shift_ColLeft;
			}
			rectCell.setLeft(colsLenCur);

			do {// столбцы

				curCell = doc->getCell(rowCur,colCur,false);
				while((isHide = doc->getScaleHide(uorRhtColumnHeader, colCur))){
					if (i == 1) {
						/* даже если ячейка скрытая, то она может содержать текст, хвост которого
						необходимо будет отобразить.*/
						curCell = doc->getCell(rowCur,colCur,false);
						if (curCell){

						}
					}
					++colCur;
				}
				if (drArea.m_areaType == 2 && drArea.m_lastVisibleCol < colCur) {
					if (i == 0) {
						// эта область типа uorReportPrintArea, тут вывод ограничен строкой и столбцом.
						break;
					} else if (i == 1){
						// тут не печатаем текст, который имеет выравнивание по левому краю и по центру.
						if (curCell){
							hCellAllign = curCell->getAlignmentHor();
							if (uoHA_Left == hCellAllign || hCellAllign == uoHA_Center)
								break;
						}
					}
				}

				sz = doc->getScaleSize(uorRhtColumnHeader, colCur);

				colsLenCur += sz;
				rectCell.setRight(colsLenCur);
				// а вот если ячейка - текущая?
				if (m_curentCell.x() == colCur && m_curentCell.y() == rowCur){
					rectCellCur = rectCell;				/// у бля....
				}
				isSell = false;
				if (m_selections)	isSell = m_selections->isCellSelect(rowCur,colCur);

				if (i == 0) {
					if (isSell) {
						rectCell.adjust(-1,-1,-1,-1);
						painter.fillRect(rectCell, m_brushSelection);
						rectCell.adjust(1,1,1,1);
					}

					if (m_showGrid){
						// draw,  draw,  draw,  draw,  aw, aw, aw, aw, aw, wu-u-u-u-u-u
						if (isSell) {
							painter.setPen(m_penWhiteText);
						} else {
							painter.setPen(m_penGrey);
						}
						/* проблема, если рисовать ячейку цельняком, то закрашивается рамка верхней.
						лучше рисанем без верхней рамки... уберем левую вертикалку, мешает.*/
						//painter.drawLine(rectCell.topLeft(), rectCell.bottomLeft());
						painter.drawLine(rectCell.bottomLeft(), rectCell.bottomRight() );
						painter.drawLine(rectCell.bottomRight(), rectCell.topRight() );

					}
					if (curCell){
						curCell->drawBorder(painter, rectCell);
					}

				} else {

					if (curCell){
						drawCell(painter, curCell, rectCell, doc, isSell, drArea);
					}
				}

				rectCell.setLeft(rectCell.right());
				colPrev = colCur;
				colCur = colCur + 1;
				if (i==0){
					if (colsLenCur >= colsLensPage)
						break;
				} else if (i==1){
					if (colCur > doc->getColCount())
						break;
				}

			} while(true /*colsLenCur < colsLensPage*/);
		}
		rectCell.setTop(rectCell.bottom());
		rowCur = rowCur + 1;
	} while(rowsLenCur < rowsLensPage);

	if (!rectCellCur.isNull() && m_showCurCell){
		int wp = m_penText.width();
		m_penText.setWidth(2);
		painter.setPen(m_penText);
		painter.drawRect(rectCellCur);
		m_penText.setWidth(wp);
	}
	painter.setPen(oldPen);
}
/**
	Специально для предварительного просмотра, прорисуем поля отчета.
*/
void uoReportDrawHelper::drawFields(QPainter& painter)
{
	if (!m_doc)
		return;
	QRect pageRectPrint = m_doc->getPageRectPrint();
	if (pageRectPrint.isEmpty() || !pageRectPrint.isValid())
		return;

	QRect paperRectPrint = m_doc->getPaperRectPrint();
	QPalette palette_ = qApp->palette();


	QBrush brushFill = palette_.brush(QPalette::Window /*Button*/ /*Qt::white*/);
	// Чуток осветлим выделение..
   	QColor brCol = brushFill.color();
   	brCol.setAlpha(124);
   	brushFill.setColor(brCol);
   	painter.fillRect(paperRectPrint,brushFill);
}



} //namespace uoReport {
