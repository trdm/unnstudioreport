/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReportCtrl.h"


#include <QPainter>
#include <QPalette>
#include <QRect>
#include <QRectF>
#include <QMenu>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>



namespace uoReport {

#define UORPT_OFFSET_LINE 2

uoReportCtrl::uoReportCtrl(QWidget *parent)
	: QWidget(parent) //, _cornerWidget(parent)
{
	setMouseTracking(true); 	/// начинаем ловить перемещение мышки....
	setContextMenuPolicy(Qt::DefaultContextMenu);
	setFocusPolicy(Qt::StrongFocus);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);


	_repoDoc = new uoReportDoc();
	_useMode = rmDevelMode;
	_stateMode = rmsNone;

	_rectGroupV		= new QRectF(0,0,0,0);
	_rectGroupH		= new QRectF(0,0,0,0);
	_rectSectionV	= new QRectF(0,0,0,0);
	_rectSectionH	= new QRectF(0,0,0,0);
	_rectRulerV		= new QRectF(0,0,0,0);
	_rectRulerH		= new QRectF(0,0,0,0);
	_rectRuleCorner	= new QRectF(0,0,0,0);
	_rectAll		= new QRectF(0,0,0,0);
	_rectDataRegion	= new QRectF(0,0,0,0);


	_charWidthPlus = 3;
 	_showGroup	= false;
	_showSection= false;
	_showRuler	= true;
	_showGrid	= true;
	_showFrame	= true;

	_maxVisibleLineNumberCnt = 3;

	_firstVisible_RowTop = 1; 	///< Первая верхняя видимая строка
	_firstVisible_ColLeft = 1; 	///< Первая левая видимая колонка
	_lastVisibleRow = -1;
	_lastVisibleCol = -1;
	_scaleFactor = _scaleFactorO = 1;

	_shift_RowTop = 0; ///< Смещение первой видимой строки вверх (грубо - размер невидимой их части)
	_shift_ColLeft = 0;

	_groupListCache = new rptGroupItemList;	///< кешь для экземпляров uoRptGroupItem
	_groupListV = new rptGroupItemList;		///< список ректов группировок столбцов
	_groupListH = new rptGroupItemList;		///< список ректов группировок строк



	initControls(parent);


	_iteractView = new uoReportViewIteract();
	_iteractView->createActions();
	_iteractView->connectActions(this);

	connect(_iteractView, SIGNAL(onScaleChange(qreal)), this, SLOT(onSetScaleFactor(qreal)));

}

/// Инициализация контролов поля отчета.
void uoReportCtrl::initControls(QWidget *parent){

	_cornerWidget = new QWidget(parent); //, _cornerWidget(parent)
	QGridLayout* layout = new  QGridLayout(parent);
	layout->setMargin(0);	/// нулевой отступ...
	layout->setSpacing(0);
	QScrollBar* _hScrollCtrl = new QScrollBar(Qt::Horizontal, parent);

	_vScrollCtrl = new QScrollBar(Qt::Vertical, this);
	_vScrollCtrl->setLayoutDirection(Qt::RightToLeft);
	_vScrollCtrl->setFocusPolicy(Qt::StrongFocus);
	_vScrollCtrl->setPageStep(32);
	_vScrollCtrl->setMaximum(50);
	_vScrollCtrl->setMinimum(1);

    layout->addWidget(_cornerWidget,1,1);
	layout->addWidget( this, 0, 0 );
	layout->addWidget(_vScrollCtrl,0,1 );
    layout->addWidget(_hScrollCtrl,1,0);
    layout->setColumnStretch(0,1);
	layout->setColumnStretch(1,0);
    layout->setRowStretch(0,1);
    layout->setRowStretch(1,0);
}


uoReportCtrl::~uoReportCtrl()
{
	//delete _repoDoc; //НЕТ!! нужно смотреть не подсоединен ли этот док к еще одному вьюву или к переменной!!!
	delete _rectGroupV;
	delete _rectGroupH;
	delete _rectSectionV;
	delete _rectSectionH;
	delete _rectRulerV;
	delete _rectRulerH;
	delete _rectRuleCorner;
	delete _rectAll;
	delete _rectDataRegion;
	delete _iteractView;
	clear();
}

///Очистка данных класса.
void uoReportCtrl::clear(){
	dropGropItemToCache();
	while (!_groupListCache->isEmpty())    delete _groupListCache->takeFirst();
	_scaleStartPositionMapH.clear();
	_scaleStartPositionMapV.clear();

}

/// Обнуляем QRectF
void zeroQRect(QRectF *rct) { rct->setLeft(0);	rct->setTop(0);	rct->setRight(0);	rct->setBottom(0); }

/// Перекалькулируем последние видимые ячейки и сопутствующие данные.
int uoReportCtrl::recalcVisibleScales(uoRptHeaderType rht){

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return -1;
	int numScale = 1, lastVisScale = -1;
	rptSize sizeVisibleScl = 0;
	rptSize sizeScale = 0;
	bool isHiden = false;
	qreal widgetSize = (rht==rhtHorizontal) ? getWidhtWidget() : getHeightWidget();

	if (rht == rhtHorizontal) {
		_scaleStartPositionMapH.clear();
		numScale 		= _firstVisible_ColLeft;
		sizeVisibleScl 	= _rectGroupH->left() - _shift_ColLeft;
		_scaleStartPositionMapH[numScale] = sizeVisibleScl;
	} else {
		_scaleStartPositionMapV.clear();
		numScale 		= _firstVisible_RowTop;
		sizeVisibleScl 	= _rectGroupV->top() - _shift_RowTop;
		_scaleStartPositionMapV[numScale] = sizeVisibleScl;
	}

	sizeScale 	= 0;
	do
	{
		isHiden = doc->getScaleHide(rht, numScale);
		if (!isHiden) {
			sizeScale = doc->getScaleSize(rht, numScale);
			sizeVisibleScl = sizeVisibleScl + sizeScale;
		}
		++numScale;
		if (rht == rhtHorizontal)
			_scaleStartPositionMapH[numScale] = sizeVisibleScl;
		else
			_scaleStartPositionMapV[numScale] = sizeVisibleScl;

	}
	while(sizeVisibleScl < widgetSize);
	lastVisScale = numScale;

	return lastVisScale;
}

void uoReportCtrl::dropGropItemToCache()
{
	while (!_groupListV->isEmpty()) {
		 _groupListCache->append(_groupListV->takeFirst());
	}
	while (!_groupListH->isEmpty()) {
		 _groupListCache->append(_groupListH->takeFirst());
	}

}

/// Взять uoRptGroupItem из кеша, если кеш пуст, то создать.
uoRptGroupItem* uoReportCtrl::getGropItemFromCache()
{
	uoRptGroupItem* item = NULL;
	if (!_groupListCache->isEmpty())
		item = _groupListCache->takeFirst();
	else
		item = new uoRptGroupItem();
	return item;
}

/// Вычисление координат гроуп итема..
void uoReportCtrl::calcGroupItemPosition(uoRptGroupItem* grItem, uoRptHeaderType rht)
{
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	grItem->_rectMidlePos = 0.0;
	grItem->_rectEndPos = 0.0;
	grItem->_sizeTail = 0.0;


	rptSize xPos = -10,yPos = -10, yPos0 = -10, xSize = 0, ySize = 0, ySize0 = 0;
	if (rht == rhtHorizontal) {

		if (_scaleStartPositionMapH.contains(grItem->_start))
			xPos = _scaleStartPositionMapH[grItem->_start];

		grItem->_rectIteract.setLeft(xPos);

		yPos = _rectGroupH->top() + ( UORPT_OFFSET_LINE * 2 + _charHeightPlus) * grItem->_level - UORPT_OFFSET_LINE;
		grItem->_rectIteract.setBottom(yPos);
		yPos = yPos - _charHeightPlus;
		grItem->_rectIteract.setTop(yPos);

		grItem->_rectMidlePos = yPos + _charHeightPlus / 2;

		xSize = doc->getScaleSize(rht, grItem->_start);
		grItem->_sizeTail = xPos + xSize; // Координаты конца ячейки.


		xSize = (xSize - (UORPT_OFFSET_LINE + _charWidthPlus)) / 2;
		xPos = xPos + xSize;
		grItem->_rectIteract.setLeft(xPos);
		xPos = xPos + UORPT_OFFSET_LINE * 2 + _charWidthPlus;
		grItem->_rectIteract.setRight(xPos);
		grItem->_rectEndPos = xPos;

		grItem->_sizeTail = grItem->_sizeTail - xPos; // Координаты конца ячейки.


	} else if (rht == rhtVertical) {
		if (_scaleStartPositionMapV.contains(grItem->_start))
			yPos0 = yPos = _scaleStartPositionMapV[grItem->_start];

		grItem->_sizeTail = yPos;

		ySize0 = ySize = doc->getScaleSize(rht, grItem->_start);
		ySize = (ySize - _charHeightPlus ) / 2;
		grItem->_sizeTail = yPos + ySize0; // Координаты конца ячейки.


		grItem->_rectIteract.setTop(yPos + ySize);
		ySize = ySize + _charHeightPlus + UORPT_OFFSET_LINE*2;
		grItem->_rectIteract.setBottom(yPos + ySize);

		xPos = _rectGroupV->left() + ( UORPT_OFFSET_LINE * 2 + _charWidthPlus) * grItem->_level;
		grItem->_rectIteract.setRight(xPos);
		xPos = xPos - (_charWidthPlus + UORPT_OFFSET_LINE * 2);
		grItem->_rectIteract.setLeft(xPos);

		grItem->_rectMidlePos = xPos + _charWidthPlus / 2 + UORPT_OFFSET_LINE;

		if (ySize0 < grItem->_rectIteract.height() && (yPos0 - 1) > 3) {
			grItem->_rectIteract.setTop(yPos0 + 1);
			grItem->_rectIteract.setBottom(yPos0 + ySize0 - 2);
		}
		grItem->_rectEndPos = grItem->_rectIteract.bottom();
		grItem->_sizeTail = grItem->_sizeTail - grItem->_rectEndPos; // Координаты конца ячейки.


	}
}

/// Вычислить длину диапазона ячеек.
rptSize uoReportCtrl::getLengthOfScale(uoRptHeaderType rht, int start, int stop)
{
	rptSize retVal = 0.0;

	if (rht == rhtUnknown || start > stop || stop<=0 || start<=0)
		return retVal;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return retVal;


	for (int i = start; i<=stop; i++) {
		retVal = retVal + doc->getScaleSize(rht, i);
	}

	return retVal;
}


///Перекалькуляция размеров линеек, регионов секций и т.п.
///\todo Перекалькуляция размеров РЕГИОНОВ СЕКЦИЙ
void uoReportCtrl::recalcGroupSectionRects(uoRptHeaderType rht){

	// Нужно просчитать последнюю показываемую ячейку
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;

	// Сбрасываем гроуп итемы в кешь.
	dropGropItemToCache();

	int i = 0;
	rptSize rSize = rptSizeNull;

	// Маленькая оптимизация. Если у нас поменялась только высота строки, не нужно пересчитывать вертикальный хейдер.
	if (rht == rhtUnknown || rht == rhtHorizontal)
		_lastVisibleCol = recalcVisibleScales(rhtHorizontal);

	if (rht == rhtUnknown || rht == rhtVertical)
		_lastVisibleRow = recalcVisibleScales(rhtVertical);

	if (_showGroup) {
		uoLineSpan* spn;
		uoRptGroupItem* grItem;
		uoRptHeaderType rht = rhtHorizontal;

		int spnCnt = 0;

		if (rht == rhtUnknown || rht == rhtHorizontal) {
			spnCnt = doc->getGroupLevel(rht);
			if (spnCnt>0) {
				const spanList* spanListH = doc->getGroupList(rht, _firstVisible_ColLeft, _lastVisibleCol);
				for (i=0; i<spanListH->size(); i++){
					spn = spanListH->at(i);
					grItem = getGropItemFromCache();
					if (grItem)	{
						grItem->copyFrom(spn);
						grItem->_sizeTail = rptSizeNull;
						zeroQRect(&grItem->_rectIteract);

						rSize = rptSizeNull;
						calcGroupItemPosition(grItem, rht);
						// посчитаем длину линии группировки.
						// хвост есть, нужно вычислить только толщину последующих ячеек и добавить его к хвосту.
						grItem->_sizeTail = grItem->_sizeTail + getLengthOfScale(rht, grItem->_start + 1, grItem->_end);
						_groupListH->append(grItem);
					}
				}
				delete spanListH;
			}
		}

		rht = rhtVertical;
		if (rht == rhtUnknown || rht == rhtVertical) {

			spnCnt = doc->getGroupLevel(rht);
			if (spnCnt>0) {
				const spanList* spanListV = doc->getGroupList(rht, _firstVisible_RowTop, _lastVisibleRow);
				for (i=0; i<spanListV->size(); i++){
					spn = spanListV->at(i);
					grItem = getGropItemFromCache();
					if (grItem)	{
						grItem->copyFrom(spn);

						zeroQRect(&grItem->_rectIteract);
						calcGroupItemPosition(grItem, rht);
						grItem->_sizeTail = grItem->_sizeTail + getLengthOfScale(rht, grItem->_start + 1, grItem->_end);
						_groupListV->append(grItem);
					}
				}
				delete spanListV;
			}
		}
	}


}

/// Пересчитываем размеры прямоугольников для областей отчета в зависимости от количества \n
/// уровней групп, секций, максимального и минимального диапазона номеров вертикальной линейки.
void uoReportCtrl::recalcHeadersRects()
{
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;

	QFontMetrics fm = QFontMetrics(font());
    _charWidthPlus = fm.width("+");
    _charHeightPlus = fm.height();
    qreal wWidth = getWidhtWidget() -2;
    qreal wHeight = getHeightWidget() -2;

	zeroQRect(_rectGroupV);  zeroQRect(_rectSectionV);	zeroQRect(_rectRulerV);
	zeroQRect(_rectGroupH);  zeroQRect(_rectSectionH);	zeroQRect(_rectRulerH);
	zeroQRect(_rectRuleCorner);
	zeroQRect(_rectAll);
	zeroQRect(_rectDataRegion);

	_rectAll->setTopLeft(QPoint(1,1));
	_rectAll->setBottom(wHeight);			_rectAll->setRight(wWidth);
	_rectDataRegion->setBottom(wHeight);	_rectDataRegion->setRight(wWidth);
	_rectGroupH->setRight(wWidth);		_rectSectionH->setRight(wWidth); 	_rectRulerH->setRight(wWidth);
	_rectGroupV->setBottom(wHeight);	_rectSectionV->setBottom(wHeight); 	_rectRulerV->setBottom(wHeight);

	rptSize curOffset = 0;

	// Расчитаем сначала высотные размеры горизонтальной секции
	int spnCntH = doc->getGroupLevel(rhtHorizontal);
	if (spnCntH>0 && _showGroup) {
		curOffset += UORPT_OFFSET_LINE; // Оффсет-отступ сверху.
		_rectGroupH->setTop(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + _charHeightPlus) * spnCntH;
		_rectGroupH->setBottom(curOffset);
	}
	spnCntH = doc->getSectionLevel(rhtHorizontal);
	if (spnCntH>0 && _showSection) {
		curOffset += UORPT_OFFSET_LINE; // Оффсет-отступ сверху.
		_rectSectionH->setTop(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + _charHeightPlus) * spnCntH;
		_rectSectionH->setBottom(curOffset);
	}

	if (_showRuler) {
		curOffset += UORPT_OFFSET_LINE;
		_rectRulerH->setTop(curOffset);
		curOffset += _charHeightPlus;
		curOffset += UORPT_OFFSET_LINE;
		_rectRulerH->setBottom(curOffset);
		curOffset += UORPT_OFFSET_LINE;
	}
	_rectDataRegion->setTop(curOffset);
	_rectGroupV->setTop(curOffset);	_rectSectionV->setTop(curOffset);	_rectRulerV->setTop(curOffset);

	curOffset = 0;
	int spnCntV = doc->getGroupLevel(rhtVertical);
	if (spnCntV>0 && _showGroup) {
		curOffset += UORPT_OFFSET_LINE;
		_rectGroupV->setLeft(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + _charWidthPlus) * spnCntV;
		_rectGroupV->setRight(curOffset);
	}
	spnCntV = doc->getSectionLevel(rhtVertical);
	if (spnCntV>0 && _showSection) {
		curOffset += UORPT_OFFSET_LINE; // Оффсет-отступ сверху.
		_rectSectionV->setLeft(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + _charWidthPlus) * spnCntV;
		_rectSectionV->setRight(curOffset);
	}

	if (_showRuler) {
		curOffset += UORPT_OFFSET_LINE;
		_rectRulerV->setLeft(curOffset);
		curOffset += _charWidthPlus * _maxVisibleLineNumberCnt+3;
		curOffset += UORPT_OFFSET_LINE;
		_rectRulerV->setRight(curOffset);
		curOffset += UORPT_OFFSET_LINE;

		_rectRuleCorner->setTop(_rectRulerH->top());
		_rectRuleCorner->setLeft(_rectRulerV->left());
		_rectRuleCorner->setBottom(_rectRulerH->bottom());
		_rectRuleCorner->setRight(_rectRulerV->right());
	}
	_rectDataRegion->setLeft(curOffset);
	_rectDataRegion->adjust(1, 1, -1, -1);

	_rectGroupH->setLeft(curOffset);		_rectSectionH->setLeft(curOffset); 	_rectRulerH->setLeft(curOffset);

	// обнулим ненужные...
	if (!_showGroup)	{zeroQRect(_rectGroupV);  	zeroQRect(_rectGroupH);		}
	if (!_showSection) 	{zeroQRect(_rectSectionV); 	zeroQRect(_rectSectionH);	}
	if (!_showRuler) 	{zeroQRect(_rectRulerV); 	zeroQRect(_rectRulerH);	zeroQRect(_rectRuleCorner);}

	recalcGroupSectionRects();
}



/// Вывод отладочной информации по размерам ректов.
void uoReportCtrl::debugRects()
{
	qDebug() << "-----------------------------------";
	qDebug() << "uoReportCtrl::debugRects() {";
	qDebug() << "_rectAll" << 			*_rectAll;
	qDebug() << "_rectGroupV" << 		*_rectGroupV;
	qDebug() << "_rectSectionV" << 		*_rectSectionV;
	qDebug() << "_rectRulerV" << 		*_rectRulerV;
	qDebug() << "_rectGroupH" << 		*_rectGroupH;
	qDebug() << "_rectSectionH" << 		*_rectSectionH;
	qDebug() << "_rectRulerH" << 		*_rectRulerH;
	qDebug() << "_rectDataRegion" << 	*_rectDataRegion;
	if (_showFrame) qDebug() << "_showFrame";
	if (_showRuler) qDebug() << "_showRuler";
	if (_showSection) qDebug() << "_showSection";
	if (_showGroup) qDebug() << "_showGroup";
	if (_showGrid) qDebug() << "_showGrid";

	int cntr;
	uoRptGroupItem* rgItem;

	if (!_groupListH->isEmpty()) {
		qDebug() << "_groupListH";
		for (cntr = 0; cntr<_groupListH->size(); cntr++) {
			rgItem = _groupListH->at(cntr);
			qDebug() << rgItem->_rectIteract << rgItem->_start << rgItem->_end;
		}
	}

	if (!_groupListV->isEmpty()) {
		qDebug() << "_groupListV";
		for (cntr = 0; cntr<_groupListV->size(); cntr++) {
			rgItem = _groupListV->at(cntr);
			qDebug() << rgItem->_rectIteract << rgItem->_start << rgItem->_end;
		}
	}

	qDebug() << "uoReportCtrl::debugRects() }";
}


/// Отрисовка нерабочей области отчета: Группировки, секции, линейки, общий фрайм.
void uoReportCtrl::drawHeaderControl(QPainter& painter){


	if (_showFrame) {
		painter.drawRect(*_rectAll);
	}

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	rptSize noLen = 2;
	bool drawSelfRects = false;
	int cntr = 0;
	uoRptGroupItem* rgItem;
	painter.save();
	Qt::PenStyle oldStyle = _penText.style();
	_penText.setStyle(Qt::DotLine);
	painter.setPen(_penText);

	QString paintStr = "";

	// Рисуем контуры пространств, чисто для визуального контроля...
	{
		if (_showGroup) {
			if (_rectGroupV->width()>noLen)	{
				if (drawSelfRects)
					painter.drawRect(*_rectGroupV);
			}
			if (_rectGroupH->height()>noLen)		{
				if (drawSelfRects)
					painter.drawRect(*_rectGroupH);
			}
		}
		if (_showSection) {
			if (_rectSectionV->width()>noLen) {
				if (drawSelfRects)
					painter.drawRect(*_rectSectionV);
			}
			if (_rectSectionH->height()>noLen) {
				if (drawSelfRects)
						painter.drawRect(*_rectSectionH);
			}
		}
		if (_showRuler) {
			if (_rectRulerV->width()>noLen) {
				if (drawSelfRects)
					painter.drawRect(*_rectRulerV);
			}
			if (_rectRulerH->height()>noLen) {
				if (drawSelfRects)
					painter.drawRect(*_rectRulerH);
			}
		}
	}

	_penText.setStyle(Qt::SolidLine);
	painter.setPen(_penText);
	QPointF pointStart, pointEnd;

	// Рисуем шапку, как она сама есть...
	if (_showGroup) {
		if (_rectGroupV->width()>noLen)	{
			if (!_groupListV->isEmpty()) {
				for (cntr = 0; cntr<_groupListV->size(); cntr++) {
					rgItem = _groupListV->at(cntr);
					painter.drawRect(rgItem->_rectIteract);
					paintStr = "-";
					if (rgItem->_folded)
						paintStr = "+";
					painter.drawText(rgItem->_rectIteract,Qt::AlignCenter,  paintStr);
					if (rgItem->_sizeTail > 0) {
						pointStart.setY(rgItem->_rectEndPos);
						pointStart.setX(rgItem->_rectMidlePos);
						pointEnd.setY(rgItem->_rectEndPos + rgItem->_sizeTail);
						pointEnd.setX(rgItem->_rectMidlePos);
						painter.drawLine(pointStart, pointEnd );

						pointStart =  pointEnd;
						pointStart.setX(pointStart.x()+3);
						painter.drawLine(pointEnd, pointStart);
					}

				}
			}
		}
		if (_rectGroupH->height()>noLen)		{
			if (!_groupListH->isEmpty()) {
				for (cntr = 0; cntr<_groupListH->size(); cntr++) {
					rgItem = _groupListH->at(cntr);
					painter.drawRect(rgItem->_rectIteract);
					paintStr = "-";
					if (rgItem->_folded)
						paintStr = "+";
					painter.drawText(rgItem->_rectIteract,Qt::AlignCenter,  paintStr);

					if (rgItem->_sizeTail > 0) {
						pointStart.setX(rgItem->_rectEndPos);
						pointStart.setY(rgItem->_rectMidlePos);
						pointEnd.setX(rgItem->_rectEndPos + rgItem->_sizeTail);
						pointEnd.setY(rgItem->_rectMidlePos);
						painter.drawLine(pointStart, pointEnd );
						pointStart =  pointEnd;
						pointStart.setY(pointStart.y()+3);
						painter.drawLine(pointEnd, pointStart);
					}
				}
			}
		}
	}

	painter.drawRect(*_rectDataRegion);
	painter.fillRect(*_rectDataRegion, _brushBase);
	painter.restore();

	/*
		попробуем порисовать линейку...
		вот тут нужен такой фокус, необходимо научиться рисовать
		отсеченные части ректов и прочих фигур.
	*/


	rptSize paintEndTagr = rptSizeNull; // Конечная граница отрисовки.
	rptSize paintCntTagr = rptSizeNull; // Текущая величина отрисовки.
	rptSize curSize = rptSizeNull; // Текущая величина отрисовки.

	int nmLine = 0;
	uoRptHeaderType hdrType;
	QRectF curRct;
	zeroQRect(&curRct);
	if (_showRuler) {
		/* рисуем сначала вертикалку.
			| 10 |
			| 11 |
			| 12 |
		*/
		painter.drawRect(*_rectRuleCorner); /// Верхний корнер-виджет слева от горизонтальной и сверху от вертикальной линейки
		hdrType = rhtVertical;
		if (_rectRulerV->width() > 0) {

			painter.setClipRect(*_rectRulerV); // Устанавливаем область прорисовки. Будем рисовать только в ней.

			curRct.setLeft(_rectRulerV->left()+_scaleFactorO);
			curRct.setRight(_rectRulerV->right()-_scaleFactorO);

			paintEndTagr = _rectRulerV->bottom();
			paintCntTagr = _rectRulerV->top() - _shift_RowTop + 1 * _scaleFactorO;
			curRct.setTop(paintCntTagr);

			nmLine = _firstVisible_ColLeft-1;
			do {
				nmLine = nmLine + 1;

				if (doc->getScaleHide(hdrType, nmLine))
					continue;
				if ((curSize = doc->getScaleSize(hdrType, nmLine)) == 0.0)
					continue;

				paintCntTagr = paintCntTagr + curSize;
				curRct.setBottom(paintCntTagr);
				painter.drawRect(curRct);
				paintStr.setNum(nmLine);
				painter.drawText(curRct, Qt::AlignCenter,  paintStr);
				curRct.setTop(paintCntTagr);


			} while(paintCntTagr < paintEndTagr);
		}
		painter.setClipRect(*_rectAll); // Устанавливаем область прорисовки. Будем рисовать только в ней.

		hdrType = rhtHorizontal;
		if (_rectRulerH->width() > 0) {

			painter.setClipRect(*_rectRulerH); // Устанавливаем область прорисовки. Будем рисовать только в ней.

			curRct.setTop(_rectRulerH->top()+_scaleFactorO);
			curRct.setBottom(_rectRulerH->bottom()-_scaleFactorO);

			paintEndTagr = _rectRulerH->right();
			paintCntTagr = _rectRulerH->left() - _shift_ColLeft + 1 * _scaleFactorO;

			curRct.setLeft(paintCntTagr);

			nmLine = _firstVisible_RowTop - 1;
			do {
				nmLine = nmLine + 1;

				if (doc->getScaleHide(hdrType, nmLine))
					continue;
				if ((curSize = doc->getScaleSize(hdrType, nmLine)) == 0.0)
					continue;

				paintCntTagr = paintCntTagr + curSize;

				curRct.setRight(paintCntTagr);
				painter.drawRect(curRct);
				paintStr.setNum(nmLine);
				painter.drawText(curRct, Qt::AlignCenter,  paintStr);
				curRct.setLeft(paintCntTagr);


			} while(paintCntTagr < paintEndTagr);
		}
		painter.setClipRect(*_rectAll); // Устанавливаем область прорисовки. Будем рисовать только в ней.

	}

	/*
		попробуем порисовать основное поле с данными...
		вот тут нужен такой фокус, необходимо научиться рисовать
		отсеченные части ректов и прочих фигур.
	*/

	_penText.setStyle(oldStyle);

}

/// Отрисовка поля данных.
void uoReportCtrl::drawDataArea(QPainter& painter){
}


/// Отрисовка виджета.
void uoReportCtrl::drawWidget(QPainter& painter)
{

	const QPalette palette_c = palette();
	QPalette::ColorGroup curColGrp = QPalette::Active;
	if (!isEnabled()) curColGrp = QPalette::Disabled;

	// нормальный фон окна.
   	_brushWindow = palette_c.brush(QPalette::Window); //	brushWindow.setColor(palette_c.color(curColGrp, QPalette::Window));

   	_brushBase = palette_c.brush(QPalette::Base /*Qt::white*/); // QPalette::Base - типа белый для текста.

	_penText.setColor(palette_c.color(curColGrp, QPalette::WindowText));
	_penNoPen.setColor(palette_c.color(curColGrp, QPalette::Window));
	_penWhiteText.setColor(palette_c.color(curColGrp, QPalette::Base));

	drawHeaderControl(painter);
	drawDataArea(painter);


}
///\todo Распланировать отрисовку так, что-бы был не единый модуль, а легко управляемые логические участки.
void uoReportCtrl::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.scale(_scaleFactor, _scaleFactor);
    painter.save();
    drawWidget(painter);
	painter.restore();
	if (_cornerWidget){
		// Отрисовка квадратика закрывающего белый фон от данного контрола внизу вертикального скрола.
		painter.setPen(_penNoPen);
		painter.scale(1/_scaleFactor,1/_scaleFactor);
		painter.fillRect(_cornerWidget->frameGeometry(), _brushWindow);
	}

}

/// Реакция на нажатие мышки-норушки на группах.
///\todo !!! Автоматизировать данную реакию.
bool uoReportCtrl::mousePressEventForGroup(QMouseEvent *event){
	bool retVal = false;
	if (_rectGroupV->contains(event->x(), event->y()) || _rectGroupH->contains(event->x(), event->y()))
	{
		retVal = true;
		uoReportDoc* doc = getDoc();
		if (!doc)
			return retVal;

		event->accept();
		rptGroupItemList* groupItList = _groupListH;

		uoRptHeaderType rht = rhtHorizontal;
		if (_rectGroupV->contains(event->x(), event->y())){
			rht = rhtVertical;
			groupItList = _groupListV;

		}
		if (groupItList->isEmpty())
			return retVal;
		uoRptGroupItem* rptGrItem = NULL;
		bool found = false;
		for (int i = 0; i< groupItList->size(); i++){
			rptGrItem = groupItList->at(i);

			if (rptGrItem->_rectIteract.contains(event->x(), event->y())){
				// Нашли итем на котором сделан клик мышкой.
				doc->doGroupFold(rptGrItem->_id, rht, !rptGrItem->_folded);
				found = true;
				break;
			}
		}

		if (found) {
			recalcGroupSectionRects();
			emit update();
		}
	}

	return retVal;

}

/// Реакция на нажатие мышки-норушки...
void uoReportCtrl::mousePressEvent(QMouseEvent *event)
{
//	int i = 0;
	if (_showGroup && mousePressEventForGroup(event)) {	return;	}
}

void uoReportCtrl::mouseReleaseEvent(QMouseEvent *event)
{}
void uoReportCtrl::mouseMoveEvent(QMouseEvent *event)
{}

void uoReportCtrl::showEvent( QShowEvent* event){
	recalcHeadersRects();
}
void uoReportCtrl::resizeEvent( QResizeEvent * event ){
	QWidget::resizeEvent(event);
	recalcHeadersRects();
}


void uoReportCtrl::contextMenuEvent(QContextMenuEvent *event){
	QMenu *contextMenu = new QMenu(this);
	QMenu *menuShow = contextMenu->addMenu(QString::fromUtf8("Скрыть / показать"));

	if (_showGrid)	menuShow->addAction(_iteractView->m_actGridHide);
	else 			menuShow->addAction(_iteractView->m_actGridShow);

	if (_showGroup) menuShow->addAction(_iteractView->m_actGroupHide);
	else 			menuShow->addAction(_iteractView->m_actGroupShow);

	if (_showSection) 	menuShow->addAction(_iteractView->m_actSectionHide);
	else 				menuShow->addAction(_iteractView->m_actSectionShow);

	if (_showRuler) 	menuShow->addAction(_iteractView->m_actRulerHide);
	else 				menuShow->addAction(_iteractView->m_actRulerShow);

	QMenu *menuScope = contextMenu->addMenu(QString::fromUtf8("Масштаб"));
	_iteractView->setCheckedState(_scaleFactor);

	menuScope->addAction(_iteractView->m_actScope25);
	menuScope->addAction(_iteractView->m_actScope50);
	menuScope->addAction(_iteractView->m_actScope75);
	menuScope->addAction(_iteractView->m_actScope100);
	menuScope->addAction(_iteractView->m_actScope125);
	menuScope->addAction(_iteractView->m_actScope150);
	menuScope->addAction(_iteractView->m_actScope200);
	menuScope->addAction(_iteractView->m_actScope250);
	menuScope->addAction(_iteractView->m_actScope300);

	contextMenu->addSeparator();
	contextMenu->addAction(_iteractView->m_actOutToDebug);
	contextMenu->addSeparator();
	contextMenu->addAction(_iteractView->m_actSave);
	contextMenu->addAction(_iteractView->m_actSaveAs);
	contextMenu->exec(event->globalPos());
	delete contextMenu;
}

/// Сигнал изменения масштаба виджета
void uoReportCtrl::onSetScaleFactor(const qreal sFactor){
	if (sFactor != _scaleFactor) {
		_scaleFactor = sFactor;
	    _scaleFactorO = 1 / _scaleFactor;

		recalcHeadersRects();
		emit update();
	}
}

/// Регулирование показа сетки, групп, секций, линейки.
void uoReportCtrl::optionShow(bool shGrid, bool shGroup, bool shSection, bool shRuler)
{
	bool shangeAnything = false;
	if (shGrid != _showGrid) {
		shangeAnything = true;
		_showGrid = shGrid;
	}
	if (shGroup != _showGroup) {
		shangeAnything = true;
		_showGroup = shGroup;
	}
	if (shRuler != _showRuler) {
		shangeAnything = true;
		_showRuler = shRuler;
	}
	if (shSection != _showSection) {
		shangeAnything = true;
		_showSection = shSection;
	}

	if (shangeAnything) {
		recalcHeadersRects();
	}
	emit update();

}

///Запись документа.
bool uoReportCtrl::saveDoc(){
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return false;

	if (doc->saveOptionsIsValid()){
		return doc->save();
	} else {
		return saveDocAs();
	}

}

bool uoReportCtrl::saveDocAs(){
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return false;
	QString 		 docFilePath = doc->getStorePathFile();
	uoRptStoreFormat storeFormat = doc->getStoreFormat();

	if (docFilePath.isEmpty()){
		docFilePath = "report.xml";
	}

	QFileDialog::Options options;

	QString selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(this,
						 tr("Save report.."),
						 docFilePath,
						 tr("XML Files (*.xml);*.xml"),
						 &selectedFilter,
						 options);
	if (!fileName.isEmpty()){
		docFilePath = fileName;
		storeFormat = uoRsf_Unknown;
		if (fileName.endsWith(QString(".xml"), Qt::CaseInsensitive)){
			storeFormat = uoRsf_XML;
		} else {
			QMessageBox::information(this, tr("Attention"), tr("Not correct file name"));
			return false;
		}
		// Пока остальные оставим.
		doc->setStoreOptions(docFilePath, storeFormat);
		return doc->save();
	}
	return false;
}

void uoReportCtrl::onSave(){
	saveDoc();
}
void uoReportCtrl::onSaveAs(){
	saveDocAs();
}

/// Скрываем/показываем сетку.
void uoReportCtrl::onGridShow(){	if (!_showGrid) {_showGrid = true;		recalcHeadersRects();	emit update();}}
/// Скрываем/показываем сетку.
void uoReportCtrl::onGridHide(){	if (_showGrid)	{_showGrid = false;		recalcHeadersRects();	emit update(); }}

/// Скрываем/показываем рамку вокруг окна....
void uoReportCtrl::onFrameShow(){	if (!_showFrame) 	{_showFrame = true;		recalcHeadersRects();	emit update();}}
void uoReportCtrl::onFrameHide(){	if (_showFrame) 	{_showFrame = false;	recalcHeadersRects();	emit update();}}

/// Скрываем/показываем линейку.
void uoReportCtrl::onRulerShow(){	if (!_showRuler) 	{_showRuler = true;		recalcHeadersRects();	emit update();}}
void uoReportCtrl::onRulerHide(){	if (_showRuler) 	{_showRuler = false;		recalcHeadersRects();	emit update();}}

/// Скрываем/показываем секции.
void uoReportCtrl::onSectionShow(){	if (!_showSection) 	{_showSection = true;		recalcHeadersRects();	emit update();}}
void uoReportCtrl::onSectionHide(){	if (_showSection) 	{_showSection = false;		recalcHeadersRects();	emit update();}}

/// Скрываем/показываем группировки.
void uoReportCtrl::onGroupShow(){	if (!_showGroup) 	{_showGroup = true;			recalcHeadersRects();	emit update();}}
void uoReportCtrl::onGroupHide(){	if (_showGroup) 	{_showGroup = false;		recalcHeadersRects();	emit update();}}

/// Вывод отледачной информации по вычислению пространств управляющей области..
void uoReportCtrl::onOutToDebug() {debugRects();}


} //namespace uoReport
