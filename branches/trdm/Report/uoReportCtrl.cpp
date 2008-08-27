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

	_rptDoc = NULL;
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
	_rectDataRegionFrame = new QRectF(0,0,0,0);


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

	_curentCell.setX(1);
	_curentCell.setY(1);

	_pageWidth 	= 0;
	_pageHeight = 0;
	_sizeVDoc 	= _sizeHDoc = 0;
	_sizeVvirt 	= _sizeHvirt = 0;

	initControls(parent);

	_iteractView = new uoReportViewIteract();
	_iteractView->createActions();
	_iteractView->connectActions(this);

	connect(_iteractView, SIGNAL(onScaleChange(qreal)), this, SLOT(onSetScaleFactor(qreal)));

	_selections = new uoReportSelection(this);
	setDoc(new uoReportDoc());
}

/// Инициализация контролов поля отчета.
void uoReportCtrl::initControls(QWidget *parent){

	_cornerWidget = new QWidget(parent); //, _cornerWidget(parent)
	QGridLayout* layout = new  QGridLayout(parent);
	layout->setMargin(0);	/// нулевой отступ...
	layout->setSpacing(0);
	_hScrollCtrl = new QScrollBar(Qt::Horizontal, parent);
	_vScrollCtrl = new QScrollBar(Qt::Vertical, parent);
	_vScrollCtrl->setLayoutDirection(Qt::RightToLeft);
	_vScrollCtrl->setFocusPolicy(Qt::StrongFocus);

	_hScrollCtrl->setRange(1,3);
	_hScrollCtrl->setValue(1);
	_vScrollCtrl->setRange(1,3);
	_vScrollCtrl->setValue(1);

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
	delete _rectDataRegionFrame;
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
/// Установить документа для вьюва.
void uoReportCtrl::setDoc(uoReportDoc* rptDoc)
{
	if (_rptDoc){
		_rptDoc->detachedView(this);
		if (!_rptDoc->isObjectAttached()) {
			delete _rptDoc;
			_rptDoc = NULL;
		}
	}
	_rptDoc = rptDoc;
	_rptDoc->attachView(this);
	connect(_rptDoc, SIGNAL(onSizeChange(qreal,qreal)), this, SLOT(changeDocSize(qreal, qreal)));


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


	rptSize xPos = -10,yPos = -10, yPos0 = -10, xSize = 0, xSize0 = 0, ySize = 0, ySize0 = 0;
	if (rht == rhtHorizontal) {

		if (_scaleStartPositionMapH.contains(grItem->_start))
			xPos = _scaleStartPositionMapH[grItem->_start];

		xSize0 = xSize = doc->getScaleSize(rht, grItem->_start);
		yPos = _rectGroupH->top() + ( UORPT_OFFSET_LINE * 2 + _charHeightPlus) * grItem->_level - UORPT_OFFSET_LINE;
		grItem->_rectIteract.setLeft(xPos);
		grItem->_rectIteract.setRight(xPos);

		grItem->_rectIteract.setBottom(yPos);
		yPos = yPos - _charHeightPlus;
		grItem->_rectIteract.setTop(yPos);

		grItem->_rectMidlePos = yPos + _charHeightPlus / 2;

		grItem->_sizeTail = xPos + xSize; // Координаты конца ячейки.

		if (xSize0 > (_charWidthPlus + UORPT_OFFSET_LINE*2)){


			xSize = (xSize - (UORPT_OFFSET_LINE + _charWidthPlus)) / 2;
			xPos = xPos + xSize;
			grItem->_rectIteract.setLeft(xPos);
			xPos = xPos + UORPT_OFFSET_LINE * 2 + _charWidthPlus;
			grItem->_rectIteract.setRight(xPos);
		} else if (xSize0>0){
			grItem->_rectIteract.setLeft(xPos);
			xPos = xPos + xSize;
			grItem->_rectIteract.setRight(xPos);
		}
		grItem->_rectEndPos = xPos;
		grItem->_sizeTail = grItem->_sizeTail - xPos; // Координаты конца ячейки.


	} else if (rht == rhtVertical) {
		if (_scaleStartPositionMapV.contains(grItem->_start))
			yPos0 = yPos = _scaleStartPositionMapV[grItem->_start];

		grItem->_sizeTail = yPos;

		ySize0 = ySize = doc->getScaleSize(rht, grItem->_start);
		ySize = (ySize  - _charHeightPlus) / 2;
		grItem->_sizeTail = yPos + ySize0; // Координаты конца ячейки.

		grItem->_rectIteract.setTop(yPos + ySize);
		ySize = ySize + _charHeightPlus; // + UORPT_OFFSET_LINE*2;
		grItem->_rectIteract.setBottom(yPos + ySize);
		if (grItem->_rectIteract.height()> ySize0){
			grItem->_rectIteract.setTop(yPos0);
			grItem->_rectIteract.setBottom(yPos0 + ySize0);
		}

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
		if (!doc->getScaleHide(rht, i))
			retVal = retVal + doc->getScaleSize(rht, i);
	}

	return retVal;
}


///Перекалькуляция размеров линеек, регионов секций и т.п.
///\todo 2 Перекалькуляция размеров РЕГИОНОВ СЕКЦИЙ
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
	zeroQRect(_rectDataRegion);	zeroQRect(_rectDataRegionFrame);

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
	_rectDataRegionFrame->setTop(_rectDataRegion->top());
	_rectDataRegionFrame->setBottom(_rectDataRegion->bottom());
	_rectDataRegionFrame->setLeft(_rectDataRegion->left());
	_rectDataRegionFrame->setRight(_rectDataRegion->right());
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

/// Тестовая отрисовка контура контрола, для визуального контроля и отладки.
void uoReportCtrl::drawHeaderControlContour(QPainter& painter)
{
	rptSize noLen = 2;
	bool drawSelfRects = false;
	painter.save();
	_penText.setStyle(Qt::DotLine);
	painter.setPen(_penText);
	// Рисуем контуры пространств, чисто для визуального контроля...
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

	painter.restore();
}


/// Отрисовка групп в HeaderControl.
void uoReportCtrl::drawHeaderControlGroup(QPainter& painter)
{
	int cntr = 0;
	uoRptGroupItem* rgItem;
	rptSize noLen = 2;
	qreal minDrawSize = 2.5;
	QString paintStr = "";
	QPointF pointStart, pointEnd;


	_penText.setStyle(Qt::SolidLine);
	painter.setPen(_penText);

	// Рисуем шапку, как она сама есть...
	if (_showGroup) {
		if (_rectGroupV->width()>noLen)	{
			if (!_groupListV->isEmpty()) {
				for (cntr = 0; cntr<_groupListV->size(); cntr++) {
					rgItem = _groupListV->at(cntr);

					if (rgItem->_rectIteract.height() > minDrawSize) {
						painter.drawRect(rgItem->_rectIteract);
						paintStr = "-";
						if (rgItem->_folded)
							paintStr = "+";
						painter.drawText(rgItem->_rectIteract,Qt::AlignCenter,  paintStr);
					}
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
					if (rgItem->_rectIteract.width() > minDrawSize) {
						painter.drawRect(rgItem->_rectIteract);
						paintStr = "-";
						if (rgItem->_folded)
							paintStr = "+";
						painter.drawText(rgItem->_rectIteract,Qt::AlignCenter,  paintStr);
					}

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
}


/// Отрисовка нерабочей области отчета: Группировки, секции, линейки, общий фрайм.
void uoReportCtrl::drawHeaderControl(QPainter& painter){


	if (_showFrame) {
		painter.drawRect(*_rectAll);
	}

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	QString paintStr = "";

	_penText.setStyle(Qt::SolidLine);
	painter.setPen(_penText);

	bool isSell = false;
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
	QRectF curRct, curRctCpy; // копия, для извращений...
	QPointF posStart, posEnd;
	zeroQRect(&curRct);

	painter.drawRect(*_rectDataRegion);

	QPen oldPen;
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

				isSell = _selections->isRowSelect(nmLine);

				paintCntTagr = paintCntTagr + curSize;
				curRct.setBottom(paintCntTagr);
				painter.drawRect(curRct);

				if (isSell) {
					painter.save();
					painter.setPen(_penWhiteText);
					painter.setBrush(_brushBlack);
					curRctCpy = curRct;
					curRctCpy.adjust(1,1,-1,-1);
					painter.drawRect(curRctCpy);
				}

				paintStr.setNum(nmLine);
				painter.drawText(curRct, Qt::AlignCenter,  paintStr);
				curRct.setTop(paintCntTagr);
				if (isSell)
					painter.restore();
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

				isSell = _selections->isColSelect(nmLine);


				curRct.setRight(paintCntTagr);
				painter.drawRect(curRct);
				paintStr.setNum(nmLine);

				if (isSell) {
					painter.save();
					painter.setPen(_penWhiteText);
					painter.setBrush(_brushBlack);
					curRctCpy = curRct;
					curRctCpy.adjust(1,1,-1,-1);
					painter.drawRect(curRctCpy);
				}

				painter.drawText(curRct, Qt::AlignCenter,  paintStr);
				curRct.setLeft(paintCntTagr);

				if (isSell)
					painter.restore();

			} while(paintCntTagr < paintEndTagr);
		}
		painter.setClipRect(*_rectAll); // Устанавливаем область прорисовки. Будем рисовать только в ней.
	}

}

/// Отрисовка поля данных.
void uoReportCtrl::drawDataArea(QPainter& painter)
{
	/*
		попробуем порисовать основное поле с данными...
		вот тут нужен такой фокус, необходимо научиться рисовать
		отсеченные части ректов и прочих фигур.
	*/
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;

	_penText.setStyle(Qt::SolidLine);
	painter.setPen(_penText);

	QRectF rectCell;
	QRectF rectCellCur; // ячейка на которой курсор стоит...
	zeroQRect(&rectCell);

	bool isSell = false;
	bool isHide = false;
	bool isCurent = false;
	// нарисуем рамку области данных, т.к. потом будем рисовать линии на ней в этой процедурине.
	painter.drawRect(*_rectDataRegionFrame);
	painter.fillRect(*_rectDataRegion, _brushBase);
	painter.setClipRect(*_rectDataRegion); // Устанавливаем область прорисовки. Будем рисовать только в ней.
	QPen oldPen = painter.pen();
	painter.setPen(_penGrey);

	int rowCur = _firstVisible_RowTop;
	int colCur = _firstVisible_ColLeft;
	qreal rowsLenCur = _rectDataRegion->top() - _shift_RowTop; // + 1 * _scaleFactorO;
	qreal rowsLensPage = _rectDataRegion->bottom();

	qreal colsLenCur = _rectDataRegion->left() - _shift_ColLeft;
	qreal colsLensPage = _rectDataRegion->right();
	rectCell.setTop(rowsLenCur);
	rectCell.setLeft(_rectDataRegion->left() - _shift_ColLeft);
	qreal sz = 0.0;
	do {	// строки, строки, строки и строки =============
		while((isHide = doc->getScaleHide(rhtVertical, rowCur))){
			++rowCur;
		}
		sz = doc->getScaleSize(rhtVertical, rowCur);
		if (sz == 0.0) {
			++rowCur;
			continue;
		}
		rowsLenCur = rowsLenCur + sz;
		rectCell.setBottom(rowsLenCur);

		colCur = _firstVisible_ColLeft;
		do {	// столбцы ||||||||||||||||||||||||||||||||||||
			if (colCur == _firstVisible_ColLeft) {
				colsLenCur = _rectDataRegion->left() - _shift_ColLeft;
				rectCell.setLeft(colsLenCur);
			}

			while((isHide = doc->getScaleHide(rhtHorizontal, colCur))){
				++colCur;
			}
			sz = doc->getScaleSize(rhtHorizontal, colCur);
			if (sz == 0.0){
				++colCur;
				continue;
			}
			colsLenCur += sz;
			rectCell.setRight(colsLenCur);
			/// draw,  draw,  draw,  draw,  aw, aw, aw, aw, aw, wu-u-u-u-u-u
			if (_showGrid){
				/// а вот если ячейка - текущая?
				if (_curentCell.x() == colCur && _curentCell.y() == rowCur){
					rectCellCur = rectCell;				/// у бля....
				}
				painter.drawRect(rectCell);

			}

			rectCell.setLeft(rectCell.right());
			colCur = colCur + 1;
		} while(colsLenCur < colsLensPage);
		rectCell.setTop(rectCell.bottom());
		rowCur = rowCur + 1;
	} while(rowsLenCur < rowsLensPage);

	if (!rectCellCur.isEmpty()){
		int wp = _penText.width();
		_penText.setWidth(2);
		painter.setPen(_penText);
		painter.drawRect(rectCellCur);
		_penText.setWidth(wp);
	}

	painter.setPen(oldPen);
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
   	_brushBlack = palette_c.brush(QPalette::WindowText /*Qt::white*/);

	_penText.setColor(palette_c.color(curColGrp, QPalette::WindowText));
	_penNoPen.setColor(palette_c.color(curColGrp, QPalette::Window));
	_penWhiteText.setColor(palette_c.color(curColGrp, QPalette::Base));
	_penGrey.setColor(palette_c.color(curColGrp, QPalette::Window));

	drawHeaderControlContour(painter);
	drawHeaderControlGroup(painter);
	drawHeaderControl(painter);
	drawDataArea(painter);


}
///\todo 1 Распланировать отрисовку так, что-бы был не единый модуль, а легко управляемые логические участки и рисовать не только на паинтере,но и на пиксмепе или принтере.
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
bool uoReportCtrl::mousePressEventForGroup(QMouseEvent *event){
	bool retVal = false;
	if (event->button() != Qt::LeftButton)
		return retVal;
	qreal posX = event->x();
	qreal posY = event->y();

	if (_scaleFactor != 1.0){
		posX = posX * _scaleFactorO;
		posY = posY * _scaleFactorO;
	}

	if (_rectGroupV->contains(posX, posY) || _rectGroupH->contains(posX, posY))
	{
		retVal = true;
		uoReportDoc* doc = getDoc();
		if (doc) {
			event->accept();
			rptGroupItemList* groupItList = _groupListH;

			uoRptHeaderType rht = rhtHorizontal;
			if (_rectGroupV->contains(posX, posY)){
				rht = rhtVertical;
				groupItList = _groupListV;

			}
			if (!groupItList->isEmpty()) {
				uoRptGroupItem* rptGrItem = NULL;
				bool found = false;
				for (int i = 0; i< groupItList->size(); i++){
					rptGrItem = groupItList->at(i);

					if (rptGrItem->_rectIteract.contains(posX, posY)){
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
		}
	}
	return retVal;
}

/// Поиск региона линейки по координатам с учетом масштаба.
bool uoReportCtrl::findScaleLocation(qreal posX, qreal posY, int &scaleNo, uoRptHeaderType rht)
{
	bool retVal = false;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return retVal;

	int cntScale = 0;
	qreal endPos = 0.0;
	qreal stratPos = 0.0;
	qreal scSize = 0.0;
	qreal nmShift = 0.0;

	if (rht == rhtVertical){
		nmShift = _rectRulerV->top();
		cntScale = _firstVisible_ColLeft;
		endPos = posY;
		stratPos = _shift_ColLeft;
		_curMouseSparesRect.setLeft(_rectRulerV->left());
		_curMouseSparesRect.setRight(_rectRulerV->right());
	} else if (rht == rhtHorizontal) {
		nmShift = _rectRulerH->left();
		cntScale = _firstVisible_RowTop;
		endPos = posX;
		stratPos = _shift_RowTop;
		_curMouseSparesRect.setTop(_rectRulerH->top());
		_curMouseSparesRect.setBottom(_rectRulerH->bottom());
	}

	stratPos = stratPos + nmShift;
	scaleNo = cntScale;
	while (stratPos < endPos){
		scSize = 0.0;
		if (!doc->getScaleHide(rht, cntScale)) {
			scSize = doc->getScaleSize(rht, cntScale);
		}
		if ((stratPos+scSize) >= endPos){
			endPos = stratPos+scSize;
			retVal = true;
			break;
		}
		stratPos += scSize;
		cntScale = cntScale + 1;
	}

	scaleNo = cntScale;

	if (rht == rhtVertical){
		_curMouseSparesRect.setTop(stratPos);
		_curMouseSparesRect.setBottom(endPos);
		mouseSparesAcceleratorSave(uoVst_ScaleV, scaleNo, rht);
	} else {
		_curMouseSparesRect.setLeft(stratPos);
		_curMouseSparesRect.setRight(endPos);
		mouseSparesAcceleratorSave(uoVst_ScaleH, scaleNo, rht);
	}

	return	retVal;
}

/// сбросить значения акселератора поиска по пространственным координатам.
void uoReportCtrl::mouseSparesAcceleratorDrop()
{
	_curMouseSparesType = uoVst_Unknown;
	_curMouseSparesNo	= -1;
	_curMouseSparesRht  = rhtUnknown;
	zeroQRect(&_curMouseSparesRect);
}

/// Запомнить значения акселератора поиска по пространственным координатам.
void uoReportCtrl::mouseSparesAcceleratorSave(uoRptSparesType spar, int nom, uoRptHeaderType rht)
{
	_curMouseSparesType = spar;
	_curMouseSparesNo	= nom;
	_curMouseSparesRht  = rht;
	// А рект вроде остается....
}

/// Оценка точки pos в rect, определение того, что точка находится в зоне изменения размеров.
uoBorderLocType uoReportCtrl::scaleLocationInBorder(qreal pos, QRectF rect, uoRptHeaderType rht)
{
	uoBorderLocType locType = uoBlt_Unknown;
	// UORPT_DRAG_AREA_SIZE
	// надо еще величину границы захвата определить.
	qreal posStart, posEnd;
	if (rht == rhtVertical || rht == rhtUnknown) {

		posStart 	= rect.top() - UORPT_DRAG_AREA_SIZE;
		posEnd 		= rect.top() + UORPT_DRAG_AREA_SIZE;
		if (posStart<=pos && pos<= posEnd)
			return uoBlt_Top;

		posStart 	= rect.bottom() - UORPT_DRAG_AREA_SIZE;
		posEnd 		= rect.bottom() + UORPT_DRAG_AREA_SIZE;
		if (posStart<=pos && pos<= posEnd)
			return uoBlt_Bottom;

	} else if (rht == rhtHorizontal || rht == rhtUnknown) {

		posStart 	= rect.left() - UORPT_DRAG_AREA_SIZE;
		posEnd 		= rect.left() + UORPT_DRAG_AREA_SIZE;
		if (posStart<=pos && pos<= posEnd)
			return uoBlt_Left;

		posStart 	= rect.right() - UORPT_DRAG_AREA_SIZE;
		posEnd 		= rect.right() + UORPT_DRAG_AREA_SIZE;
		if (posStart<=pos && pos<= posEnd)
			return uoBlt_Right;
	}
	return locType;
}


/// отработаем реакцию на нажатие мышки на линейке.
///\todo 1 реализовать логику выбора строк/столбцов.
bool uoReportCtrl::mousePressEventForRuler(QMouseEvent *event)
{
	bool retVal = false;
	if (event->button() != Qt::LeftButton)
		return retVal;
	qreal posX = event->x(), posY = event->y();

	if (_scaleFactor != 1.0){
		posX = posX * _scaleFactorO;
		posY = posY * _scaleFactorO;
	}
	if (_rectRuleCorner->contains(posX, posY)){
		_selections->selectDocument();
		emit update();
		return true;
	}
	if (!(_rectRulerH->contains(posX, posY) || _rectRulerV->contains(posX, posY))) {
		return retVal;
	}
	uoRptHeaderType rhtCur = rhtVertical;
	if (_rectRulerH->contains(posX, posY))
		rhtCur = rhtHorizontal;

	int scaleNo = 0;
	if (findScaleLocation(posX, posY, scaleNo, rhtCur)){
		// Я должен убедиться, что позиция мыши не у края ячейки, что-бы начать изменения размера
		uoBorderLocType locType = uoBlt_Unknown;
		if (rhtCur == rhtHorizontal){
			locType = scaleLocationInBorder(posX, _curMouseSparesRect, rhtCur);
			if (locType == uoBlt_Unknown) {
				_selections->selectCol(scaleNo);
			} else {
			}
		} else {
			locType = scaleLocationInBorder(posY, _curMouseSparesRect, rhtCur);
			if (locType == uoBlt_Unknown) {
				_selections->selectRow(scaleNo);
			} else {
			}
		}
		emit update();
	}

	return retVal;
}

/// Реакция на нажатие мышки-норушки...
void uoReportCtrl::mousePressEvent(QMouseEvent *event)
{
	if (_showGroup && mousePressEventForGroup(event)) {
		return;
	}
	if (_showRuler && mousePressEventForRuler(event)){
		return;
	}
	///\todo 1 добавить вычисление/отображение типа курсора в нужной позиции...
}

void uoReportCtrl::mouseReleaseEvent(QMouseEvent *event)
{}
void uoReportCtrl::mouseMoveEvent(QMouseEvent *event)
{}

void uoReportCtrl::showEvent( QShowEvent* event){
	recalcHeadersRects();
}

/// Обработка клавиатурных клавишь перемещения курсора....
void uoReportCtrl::keyPressEventMoveCursor ( QKeyEvent * event )
{
	int key = event->key();
	event->accept();

	bool needUpdate = true;
	switch (key)	{
		case Qt::Key_Down:	{
			_curentCell.setY(_curentCell.y()+1);
			break;
		}
		case Qt::Key_Up: {
			if (_curentCell.y() > 1) {
				_curentCell.setY(_curentCell.y()-1);
			} else {
				needUpdate = false;
			}
			break;
		}
		case Qt::Key_Left:	{
			if (_curentCell.x() > 1) {
				_curentCell.setX(_curentCell.x()-1);
			} else {
				needUpdate = false;
			}
			break;
		}
		case Qt::Key_Right: {
			_curentCell.setX(_curentCell.x()+1);
			break;
		}
		default: {
			event->ignore();
			break;
		}
	}
	if (needUpdate)
		emit update();
}

/// Обработка реакции клавиатуры..
void uoReportCtrl::keyPressEvent( QKeyEvent * event ){
	int key = event->key();
	event->accept();
	bool needUpdate = false;
	switch (key)
	{
		case Qt::Key_Down:
		case Qt::Key_Up:
		case Qt::Key_Left:
		case Qt::Key_Right:
		{
			keyPressEventMoveCursor ( event );
			break;
		}
//		case Qt::Key_PageUp:
//		{
//			m_StartScaleV -= m_pageSizeV;
//			break;
//		}
//		case Qt::Key_PageDown:
//		{
//			m_StartScaleV += m_pageSizeV;
//			break;
//		}
//		case Qt::Key_Home:
//		{
//			m_StartScaleV = 1;
//			break;
//		}
//		case Qt::Key_End:
//		{
//			m_StartScaleV = m_VLengthDoc_virt;
//			break;
//		}
		default:
		{
			/// остальные пусть папочка отрабатывает... QPrintTable
			event->ignore();
			break;
		}
	}
	if (needUpdate)
		emit update();

}

void uoReportCtrl::resizeEvent( QResizeEvent * event ){

	QWidget::resizeEvent(event);

	const QSize oldSizeW =  event->oldSize();
	const QSize sizeW =  event->size();
	_sizeVvirt = _sizeVvirt - oldSizeW.height() + sizeW.height();
	_sizeHvirt = _sizeHvirt - oldSizeW.width() + sizeW.width();

	recalcHeadersRects();
	recalcScrollBars();
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
		recalcScrollBars();
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

/// Выбор имени файла для сохранения файла..
bool uoReportCtrl::saveDocAs(){
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return false;
	QString 		 docFilePath = doc->getStorePathFile();
	uoRptStoreFormat storeFormat = doc->getStoreFormat();

	if (_iteractView->chooseSaveFilePathAndFormat(docFilePath, storeFormat, this)){
		doc->setStoreOptions(docFilePath, storeFormat);
		return doc->save();
	} else {
		return false;
	}
}

/// сигнал на запись документа
void uoReportCtrl::onSave(){
	saveDoc();
}
/// сигнал на запись документа с выбором файла..
void uoReportCtrl::onSaveAs(){
	saveDocAs();
}

/// Установить текушую ячейку с/без гарантии видимости
void uoReportCtrl::setCurentCell(int x, int y, bool ensureVisible)
{
	if (x<=0 || y <=0)
		return;
	if (ensureVisible){
		///\todo 2 гарантировать видимость ячейки. у-у-у-у!!!
	}
}


/// Перекалькуляция размеров и положения ползунка скролов
void uoReportCtrl::recalcScrollBars()
{
	// Чё есть?
	//	int _sizeVvirt;	///< Виртуальный размер документа по вертикали. Виртуальный потому что может увеличиваться скролом.
	//	int _sizeHvirt;	///< Виртуальный Размер документа по горизонтали
	//	int _sizeVDoc;	///< Реальный размер документа.
	//	int _sizeHDoc;	///< Реальный размер документа.
	//	int _pageWidth;		///< Ширина страницы в столбцах стандартного размера
	//	int _pageHeight;	///< Высота страницы в строках стандартного размера
	//  как считать?

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	int scalesCnt = 0;
	int pages  = 1;
	{
		// Вертикальный скролл.
		// вроде пролучается, но какой-то деревянный скролл...
		// Пока отработаем с горизонтальным....

		int heightV = getHeightWidget();
		qreal scaleSizeV = doc->getDefScaleSize(rhtVertical) * _scaleFactorO;
		int scalesCnt = (int)(heightV / scaleSizeV);


		pages  = 1;
		if (_sizeVvirt < heightV) {
			_sizeVvirt = heightV;
		}
		if (_sizeVvirt < _sizeVDoc){
			_sizeVvirt = _sizeVDoc;
		}
		if (heightV>0)
			pages  = _sizeVvirt / heightV;
		_vScrollCtrl->setMaximum(pages*scalesCnt);
		_vScrollCtrl->setPageStep(scalesCnt);


		qDebug()
		<< "_sizeVvirt: " 	<< _sizeVvirt
		<< "_sizeVDoc: "	<< _sizeVDoc
		<< "heightV: "		<< heightV
		<< "VScroll: min: " << _vScrollCtrl->minimum()
		<< "max: " 			<< _vScrollCtrl->maximum()
		<< "val: "			<< _vScrollCtrl->value();
	}
	{
		int widthW 	= getWidhtWidget(); // / doc->getDefScaleSize(rhtHorizontal);
		qreal scaleSizeH = doc->getDefScaleSize(rhtHorizontal) * _scaleFactorO;
		int scalesCnt = (int)(widthW / scaleSizeH);


		pages = 1;
		if (_sizeHvirt < widthW) {
			_sizeHvirt = widthW;
		}
		if (_sizeHvirt < _sizeVDoc){
			_sizeHvirt = _sizeVDoc;
		}
		if (widthW>0)
			pages  = _sizeHvirt / widthW;
		_hScrollCtrl->setMaximum(pages  * scalesCnt);
		_hScrollCtrl->setPageStep(scalesCnt);

	}

}

/// Сигнал установки новых размеров документа.
void uoReportCtrl::changeDocSize(qreal sizeV, qreal sizeH)
{
	uoReportDoc* doc = getDoc();
	if (!doc)
		return;

	int newSizeV = (int)sizeV;
	int newSizeH = (int)sizeH;
	if (newSizeV != _sizeVDoc || _sizeHDoc != newSizeH){
		_sizeVvirt = _sizeVvirt + newSizeV - _sizeVDoc;
		_sizeHvirt = _sizeHvirt + newSizeH - _sizeHDoc;
		_sizeVDoc = newSizeV;
		_sizeHDoc = newSizeH;
		recalcScrollBars();
	}
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
