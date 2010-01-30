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

QString g_textTextWithLineBreak;

uoReportDrawHelper::uoReportDrawHelper()
{
	m_directDraw = false;
	uorZeroRectF(m_rectCurCell);
}

uoReportDrawHelper::uoReportDrawHelper(uoReportDoc* doc)
	: m_doc(doc)
{
	m_directDraw = false;
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

	m_brushDarkPhone = palette_c.brush(QPalette::WindowText /*Qt::white*/);
   	brCol = m_brushDarkPhone.color();
   	brCol.setAlpha(124);
   	m_brushDarkPhone.setColor(brCol);

	m_penText.setColor(palette_c.color(curColGrp, QPalette::WindowText));
	m_penNoPen.setColor(palette_c.color(curColGrp, QPalette::Window));
	m_penWhiteText.setColor(palette_c.color(curColGrp, QPalette::Base));
	m_penGrey.setColor(palette_c.color(curColGrp, QPalette::Window));
	m_fullPage = false;
	m_printHidenRow = false;

}


void uoReportDrawHelper::clipRect(uoPainter& painter, uorRect& rect)
{
	#ifdef Q_OS_WIN
	if (m_directDraw) {
		//m_rgnCreated
	} else {

	}
	#else
//		painter.drawText(rectCpyCell,flags,text);
	#endif
}


void uoReportDrawHelper::drawText(uoPainter& painter, uorRect& rectCpyCell, QString& text, uoCell* cell)
{
	int flags = cell->getAlignment();
	QFont* font = 0;
	QPen oldPen = painter.pen();

	font = cell->getFont(m_doc);
	uorTextDecor* textProp = cell->getTextProp(m_doc);

	#ifdef Q_OS_WIN
		if (m_directDraw) {
			rectCpyCell.adjust(0,1,0,0);
			HFONT oldFont;
			HFONT curFont;

			if (textProp) {
				int fSize = (int)textProp->m_fontSz;
				fSize = -MulDiv(fSize, GetDeviceCaps(m_hdc, LOGPIXELSY), 72);
				curFont = CreateFont(fSize, 0, 0, 0, FW_NORMAL, 0, 0, 0,0, 0, 0, 0,VARIABLE_PITCH | FF_SWISS, TEXT("Arial"));
				oldFont = (HFONT)SelectObject(m_hdc, curFont);
			}
			RECT r;
			SetRect(&r, (int)rectCpyCell.left(), (int)rectCpyCell.top(), (int)rectCpyCell.right(), (int)rectCpyCell.bottom());

			wchar_t *we = new wchar_t[text.size()+1];
			text.toWCharArray(we);
			we[text.size()] = 0;
			DrawText(m_hdc, we, -1, &r,0);
			delete[] we;
			if (textProp) {
				SelectObject(m_hdc, oldFont);
			}
			DeleteObject(curFont);
		} else {
			if (true) {
				painter.drawText(rectCpyCell,flags,text);

			} else {
				// Текст под углом от Виктора ICQ: 177287
				// Пример: "@-90@текст"

				QRectF rRect = rectCpyCell;
				QRegExp reRegExp("^@(.*)@");
				reRegExp.indexIn(text);
				double dAngle = reRegExp.cap(1).toDouble();
				qreal realRectHeight = rRect.height();
				qreal realRectWidth = rRect.width();
				if(dAngle != .0){
					text.replace(reRegExp.cap(0), "");
					if(((dAngle > 0) && (dAngle < 180)) || ((dAngle > -360) && (dAngle < -180))){
							rRect.setX(rRect.x() + realRectHeight);
						}else{
							rRect.setY(rRect.y() + realRectHeight);
					}
					rRect.setHeight(realRectHeight);
					rRect.setWidth(realRectWidth);
					painter.save();
					painter.translate(rRect.x(), rRect.y());
					painter.rotate(dAngle);
					painter.drawText(0, 0, rRect.width(), rRect.height(), flags, text);
					painter.restore();
				} else {
					painter.drawText(rectCpyCell,flags,text);
				}

			}

		}
	#else
		painter.drawText(rectCpyCell,flags,text);
	#endif

}

/// Отрисовка ячейки таблицы
void uoReportDrawHelper::drawCell(uoPainter& painter, uoCell* cell,uorRect& rectCell, uoReportDoc* doc, const bool& isSell, uorReportAreaBase& drArea)
{
	if (!cell)
		return;

	uorRect mainArea = drArea.m_area;

	static QString text = "";
	text = cell->getTextWithLineBreak(m_showInvisiblChar);
	if (!text.isEmpty()) {


		uoCellTextBehavior textBhv = cell->getTextBehavior();
		uoCellTextType textTp = cell->getTextType();

		uorRect rectCpyCell = rectCell;

		uorNumber adj = (uorNumber)UORPT_STANDART_OFFSET_TEXT;
		rectCpyCell.adjust(adj,uorNumberNull,-adj,uorNumberNull); /// иначе текст перекрывается..

		int flags = cell->getAlignment();
		QFont* font = 0;
		QPen oldPen = painter.pen();

		font = cell->getFont(doc);
		if (font)
			painter.setFont(*font);

		painter.setPen(m_penText);
		/*
			тут интересная байда. если текст длинен, тогда он может печататься и
			поверх остальных ячеек, но нам передается область его отсечения.
			т.е. рект ячейки... как его правильно распечатать в нужном месте/нужным образом?
			Очевидным ответом будет расширение ректа в стророну выравнивания на длинну строки...
		*/

		uorNumber maxRowLendht = cell->getMaxRowLength();
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
			maxRowLendht += (uorNumber)fm.width(addChrs);
		}

		uorNumber diffSize = maxRowLendht - rectCpyCell.width();
		if (diffSize > 1.0 /*&& textTp == uoCTT_Text*/){

			if (textBhv == uoCTB_Auto){
				uoHorAlignment ha = cell->getAlignmentHor();
				uorNumber pointPos = uorNumberNull, diffSize = maxRowLendht - rectCpyCell.width();
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
// На пиксмапе не пашет :((((
		drawText(painter, rectCpyCell, text, cell);

		painter.setPen(oldPen);
	}
}

/// Отрисовка выделения на поле данных
void uoReportDrawHelper::drawDataAreaSelection(uoPainter& painter, uorReportAreaBase& drArea)
{
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;

	uorRect rectCell;
	uorZeroRectF(rectCell);

	bool isSell = false;
	bool isHide = false;

	uorRect drawingRect = drArea.m_area;

	int rowCur = drArea.firstVisible_RowTop();
	int colCur = 1;

	if (drArea.m_areaType == 2)		drawingRect.adjust(-2,-2,2,2);

	painter.setClipRect(drawingRect); // Устанавливаем область прорисовки. Будем рисовать только в ней.

	if (drArea.m_areaType == 2)	{
		drawingRect.adjust(2,2,-2,-2);
		colCur = drArea.firstVisible_ColLeft();
	}

	uorNumber rowsLenCur = drawingRect.top() - drArea.shift_RowTop(); // + 1 * m_scaleFactorO;
	uorNumber rowsLensPage = drawingRect.bottom();

	uorNumber colsLenCur = drawingRect.left() - drArea.shift_ColLeft();
	uorNumber colsLensPage = drawingRect.right();
	rectCell.setTop(rowsLenCur);
	rectCell.setLeft(drawingRect.left() - drArea.shift_ColLeft());

	/* надо изменить логику перерисовки по строкам.
	причина: можем не прорисовать длинные строки, которые начинаются за
	видимыми областями и прорисовываются на видимой */

	uorNumber sz = uorNumberNull;

	do {
		// строки, строки, строки и строки =============
		while((isHide = doc->getScaleHide(uorRhtRowsHeader, rowCur))){
			++rowCur;
		}
		sz = doc->getScaleSize(uorRhtRowsHeader, rowCur);
		if (sz == uorNumberNull) {
			++rowCur;
			continue;
		}
		if (drArea.m_areaType == 2) {
			// эта область типа uorReportPrintArea, тут вывод ограничен строкой и столбцом.
			if (drArea.lastVisibleRow() < rowCur)
				break;
		}
		rowsLenCur = rowsLenCur + sz;
		rectCell.setBottom(rowsLenCur);
		colCur = drArea.firstVisible_ColLeft();

		colCur = drArea.firstVisible_ColLeft();
		colsLenCur = drawingRect.left() - drArea.shift_ColLeft();
		rectCell.setLeft(colsLenCur);

		do {// столбцы


			while((isHide = doc->getScaleHide(uorRhtColumnHeader, colCur))){
				++colCur;
			}
			if (drArea.m_areaType == 2 && drArea.lastVisibleCol() < colCur) {
				break; // эта область типа uorReportPrintArea, тут вывод ограничен строкой и столбцом.
			}

			sz = doc->getScaleSize(uorRhtColumnHeader, colCur);

			colsLenCur += sz;
			rectCell.setRight(colsLenCur);

			// а вот если ячейка - текущая?

			isSell = false;
			if (m_selections)	isSell = m_selections->isCellSelect(rowCur,colCur);

			if (isSell) {
				rectCell.adjust(-1,-1,-1,-1);
				painter.fillRect(rectCell, m_brushSelection);
				rectCell.adjust(1,1,1,1);
			}

			// тут уже идет вычисление нового ректа для следующей  ячейки.

			rectCell.setLeft(rectCell.right());
			colCur = colCur + 1;
			if (colsLenCur >= colsLensPage)
				break;

		} while(true /*colsLenCur < colsLensPage*/);

		rectCell.setTop(rectCell.bottom());
		rowCur = rowCur + 1;
	} while(rowsLenCur < rowsLensPage);

}

/// Отрисовка поля данных, без выделения.
void uoReportDrawHelper::drawDataArea(uoPainter& painter, uorReportAreaBase& drArea)
{
	/*	попробуем порисовать основное поле с данными...
		вот тут нужен такой фокус, необходимо научиться рисовать
		отсеченные части ректов и прочих фигур.	*/

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
#ifdef Q_OS_WIN
	if (m_directDraw) {		m_hdc = painter.paintEngine()->getDC();	}
#endif


	m_penText.setStyle(Qt::SolidLine);
	painter.setPen(m_penText);

	uorRect rectCell;
	uorRect rectCellCur; 	// экранная область текущей ясейки.
	uorRect rectCellToPaint; // отрисовываемая область.
	uorZeroRectF(rectCell);
	uorZeroRectF(rectCellToPaint);

	bool isSell = false;
	bool isHide = false;

	// нарисуем рамку области данных, т.к. потом будем рисовать линии на ней в этой процедурине.
	// painter.drawRect(m_rectDataRegionFrame);

	uorRect drawingRect = drArea.m_area;

	painter.fillRect(drawingRect, m_brushBase);
	QPen oldPen = painter.pen();
	painter.setPen(m_penGrey);

	int rowCur = drArea.firstVisible_RowTop();
	int colCur = 1; //drArea.firstVisible_ColLeft();
	int colCurFirst = 1; //drArea.firstVisible_ColLeft();

	int colPrev = -1;
	if (drArea.m_areaType == 2)		drawingRect.adjust(-2,-2,2,2);


	#ifdef Q_OS_WIN
		if (m_directDraw) {
		m_hdc = painter.paintEngine()->getDC();
		m_hrgn = CreateRectRgn((int)drawingRect.left(),
		(int)drawingRect.top(),
        (int)drawingRect.right(),
        (int)drawingRect.bottom());
		SelectClipRgn(m_hdc, m_hrgn);
		} else {
			painter.setClipRect(drawingRect); // Устанавливаем область прорисовки. Будем рисовать только в ней.
		}
	#else
		painter.setClipRect(drawingRect); // Устанавливаем область прорисовки. Будем рисовать только в ней.
	#endif


	if (drArea.m_areaType == 2)	{
		drawingRect.adjust(2,2,-2,-2);
		colCur = drArea.firstVisible_ColLeft();
	}

	uorNumber rowsLenCur = drawingRect.top() - drArea.shift_RowTop(); // + 1 * m_scaleFactorO;
	uorNumber rowsLensPage = drawingRect.bottom();

	uorNumber colsLenCur = drawingRect.left() - drArea.shift_ColLeft();
	uorNumber colsLensPage = drawingRect.right();
	rectCell.setTop(rowsLenCur);
	rectCell.setLeft(drawingRect.left() - drArea.shift_ColLeft());
	QString cellTest;
	uoCell* curCell = 0;
	uoCell* curCellPaint = 0; // отрисовываемая ячейка.
	uoRow* curRow = 0;

	/* надо изменить логику перерисовки по строкам.
	причина: можем не прорисовать длинные строки, которые начинаются за
	видимыми областями и прорисовываются на видимой */

	uorNumber leftInvOffset = uorNumberNull; // скока у нас слева скрыто.
	uorNumber sizeCol = uorNumberNull; // скока у нас слева скрыто, но содержит текст, который необходимо отпечатать
	for(int t = 1; t<drArea.firstVisible_ColLeft(); t++)
	{
		if (!doc->getScaleHide(uorRhtColumnHeader, t))
			leftInvOffset += doc->getScaleSize(uorRhtColumnHeader, t);
	}

	uorNumber sz = uorNumberNull;
	uorNumber lengthMaxOver = uorNumberNull;
	bool hasUnion = false;
	bool needDrawCell = true;
	uoCellUnionRect* cellURect = 0;
	uoCellJoin* cellJoinItem = 0;
	QList<uoCellUnionRect*> cellUnionList;
	QList<uoCellUnionRect*>::iterator cellUnionListIt;

	uoCell* vect[100];
	memset(vect,0,sizeof(uoCell*)*100);
	bool getFromCache = false;
	int cacheColStart = 0, cacheColEnd = 0;
	int cycleDrwText = 1, cycleDrwBorder = 0;

	bool debugThis = false;
	bool textDraw = true;


	uoHorAlignment hCellAllign = uoHA_Unknown;
	///\todo - отладить прорисовку >> D:\Мои документы\Win_Мои_документы2\Мои рисунки\unNStudio\report\ba\report_bag_001.bmp

	do {
		// строки, строки, строки и строки =============
		while((isHide = doc->getScaleHide(uorRhtRowsHeader, rowCur))){
			++rowCur;
		}
		sz = doc->getScaleSize(uorRhtRowsHeader, rowCur);
		if (sz == uorNumberNull) {
			++rowCur;
			continue;
		}
		if (drArea.m_areaType == 2) {
			// эта область типа uorReportPrintArea, тут вывод ограничен строкой и столбцом.
			if (drArea.lastVisibleRow() < rowCur)
				break;
		}
		rowsLenCur = rowsLenCur + sz;
		rectCell.setBottom(rowsLenCur);
		curRow = doc->getRow(rowCur);
		lengthMaxOver = uorNumberNull;
		if (curRow)
			lengthMaxOver = curRow->m_lengthMaxOver;

		colCur = drArea.firstVisible_ColLeft();
		// 2 цикла нужны для того, что-бы первый раз отрисовать рамки, а на второй текст.
		//cycleCounter =
		for ( int cycleCounter = cycleDrwBorder; cycleCounter < 2; cycleCounter++) {
			///\todo - надо свести к 1-му циклу.... ибо пипец.... и продумать нормальную прорисовку
			if (cycleCounter == cycleDrwBorder) {
				colCur = drArea.firstVisible_ColLeft();
				colsLenCur = drawingRect.left() - drArea.shift_ColLeft();
			} else if (cycleCounter == cycleDrwText) {
				colCur = 1; // Для длинных строк, которые пересекают несколько ячеек.
				colsLenCur = -leftInvOffset + drawingRect.left() - drArea.shift_ColLeft(); // leftInvOffsetRow
				if (lengthMaxOver <= uorNumberNull){
					colCur = drArea.firstVisible_ColLeft();
					colsLenCur = drawingRect.left() - drArea.shift_ColLeft();
				} else {
					colCur = drArea.firstVisible_ColLeft();
					colsLenCur = drawingRect.left() - drArea.shift_ColLeft();
					if (colCur>1) {
						while(lengthMaxOver>0 && colCur>1){
							colCur = colCur - 1;
							if (!doc->getScaleHide(uorRhtColumnHeader, colCur)){
								sizeCol = doc->getScaleSize(uorRhtColumnHeader, colCur);
								colsLenCur = colsLenCur-sizeCol;
								lengthMaxOver = lengthMaxOver - sizeCol;
							}
						}
						colCur = qMax(1, colCur);
					}

				}
			}
			if (debugThis && cycleCounter == cycleDrwText){
				colCurFirst = colCur;
			}
			rectCell.setLeft(colsLenCur);
			if (cycleCounter == cycleDrwBorder) {
				memset(vect,0,sizeof(uoCell*)*100);
				cacheColStart = colCur; // , casheColEnd = 0
			}
			textDraw = false;

			do {// столбцы

				hasUnion = false;
				needDrawCell = true;
				curCell = 0;

				while((isHide = doc->getScaleHide(uorRhtColumnHeader, colCur))){
					++colCur;
				}

				getFromCache = false;
				if (cycleCounter == cycleDrwText && colCur<100 && colCur>= cacheColStart && cacheColEnd >= colCur){
					getFromCache = true;
				}

				if (!getFromCache){
					if (curRow)		curCell = curRow->getCell(colCur,false);
					if (colCur<100)	vect[colCur] = curCell;
				} else {
					curCell = vect[colCur];
				}

				if (drArea.m_areaType == 2 && drArea.lastVisibleCol() < colCur) {
					if (cycleCounter == cycleDrwBorder) {
						break; // эта область типа uorReportPrintArea, тут вывод ограничен строкой и столбцом.
					} else if (cycleCounter == cycleDrwText){
						if (curCell){		// тут не печатаем текст, который имеет выравнивание по левому краю и по центру.
							hCellAllign = curCell->getAlignmentHor();
							if (uoHA_Left == hCellAllign || hCellAllign == uoHA_Center)
								break;
						}
					}
				}

				sz = doc->getScaleSize(uorRhtColumnHeader, colCur);

				colsLenCur += sz;
				rectCell.setRight(colsLenCur);


				cellURect = 0;
				hasUnion = false;
				if (curCell) {
					hasUnion = curCell->isUnionHas();
					if (!textDraw)
						textDraw = (curCell->m_maxRowLen > 1);
				}

				rectCellToPaint = rectCell;
				// если есть объединение, нужно вычислить рект, он отличается от ректа ячейки.
				if (hasUnion){
					needDrawCell = false;
					if (curCell){
						// проанализируем на участие в объединени
						// надо найти калькулируемый рект и увеличить его на высоту и ирину ячейки
						if (curCell->isFirstOfUnionCell()) {
							needDrawCell = true;
							cellJoinItem = curCell->cellJoin();
							if (cellJoinItem) {
								rectCellToPaint.setRight(rectCellToPaint.left() + cellJoinItem->m_width);
								rectCellToPaint.setBottom(rectCellToPaint.top() + cellJoinItem->m_height);
							}
						}
					}
				}
				// а вот если ячейка - текущая?
				if (cycleCounter==cycleDrwBorder){
					if (m_curentCell.x() == colCur && m_curentCell.y() == rowCur){
						rectCellCur = rectCellToPaint;				/// у бля....
					}
				}


				isSell = false;

				if (needDrawCell) {
					if (cycleCounter == cycleDrwBorder) {

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
							painter.drawLine(rectCellToPaint.bottomLeft(), rectCellToPaint.bottomRight() );

							if (cellURect || !hasUnion){
								// либо последняя правая при объединении, либо рисуем
								painter.drawLine(rectCellToPaint.bottomRight(), rectCellToPaint.topRight() );
							}

						}
						if (curCell){
							curCell->drawBorder(painter, rectCellToPaint);
						}

					} else {

						if (curCell){
							drawCell(painter, curCell, rectCellToPaint, doc, isSell, drArea);
						}
					}
				}

				// тут уже идет вычисление нового ректа для следующей  ячейки.

				rectCell.setLeft(rectCell.right());
				colPrev = colCur;
				colCur = colCur + 1;
				if (cycleCounter==cycleDrwBorder){
					cacheColEnd = colCur;
					if (colsLenCur >= colsLensPage)
						break;
				} else if (cycleCounter==cycleDrwText){
					if (colCur > doc->getColCount())
						break;
					if (lengthMaxOver <= uorNumberNull){
						if (colCur == drArea.lastVisibleCol() + 1)
							break;
					} else if (lengthMaxOver < colsLenCur - drawingRect.width()){
						break;
					}

				}

			} while(true /*colsLenCur < colsLensPage*/);
			if (debugThis && cycleCounter == cycleDrwText && textDraw){
				qDebug() << "Draw text- first col: " << colCurFirst << "last col: " << colCur << " colsLenCur: " << colsLenCur  << " colsLenCur: " << colsLenCur;

			}

		}
		rectCell.setTop(rectCell.bottom());
		rowCur = rowCur + 1;
	} while(rowsLenCur < rowsLensPage);

	painter.setPen(oldPen);
	#ifdef Q_OS_WIN
		if (m_directDraw) {
			SelectClipRgn(m_hdc, 0);
			painter.paintEngine()->releaseDC(m_hdc);
		}
	#endif


	while (!cellUnionList.isEmpty()){
		delete cellUnionList.takeFirst();
	}
}
/**
	Специально для предварительного просмотра, прорисуем поля отчета.
*/
void uoReportDrawHelper::drawFields(uoPainter& painter)
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
