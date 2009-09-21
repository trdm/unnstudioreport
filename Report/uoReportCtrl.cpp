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
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include "uoReportDocBody.h"
#include "uoReportUndo.h"
#include "uoReportPreviewDlg.h"
#include "uoReportDrawHelper.h"
#include "uorPagePrintSetings.h"
#include "uorPageSetup.h"

namespace uoReport {

uoReportCtrlMesFilter::uoReportCtrlMesFilter(QObject* pObj /* = 0*/)
	:QObject(pObj)
{
	connect(this, SIGNAL(editComplete(bool)), pObj, SLOT(onCellEditTextEnd(bool)));
}

/*virtual*/
bool uoReportCtrlMesFilter::eventFilter(QObject* pObj, QEvent* pEvent)
{
	if (pEvent->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = (QKeyEvent*)pEvent;
		if (keyEvent){
			switch(keyEvent->key()){
				case Qt::Key_Escape:{
					emit editComplete(false);
					return true;
				}
				case Qt::Key_Enter: 	// цифровая клавиатура
				case Qt::Key_Return:	// основная клавиатура
				{
					Qt::KeyboardModifiers  kbrdMod = qApp->keyboardModifiers();
					if (kbrdMod & Qt::ControlModifier)
						return false;

					emit editComplete(true);
					return true;
				}
				case Qt::Key_Tab:
				{
					emit editComplete(true);
					return true;
				}
				default:{
					break;
				}
			}
		}
	}
	return false;
}


/// Собрать все проперти в выделени и замержить(слить) их.
bool uorSelVisitorGetProps::visit(uoCell* cell, bool first)
{
	bool retVal = true;
	if (cell){
		if (cell->m_textProp){
			if (first) {	m_textDec_Selection.copyFrom(cell->m_textProp);
			} else {		m_textDec_Selection.mergeItem(*cell->m_textProp);
			}
		}
		if (cell->m_borderProp){
			if (first) {	m_borderProp_Selection.copyFrom(cell->m_borderProp);
			} else {		m_borderProp_Selection.mergeItem(*cell->m_borderProp);
			}
		}
	}
	return retVal;
}
/// Собрать все проперти в выделени и замержить(слить) их.
bool uorSelVisitorSetProps::visit(uoCell* cell, bool first)
{
	bool retVal = true;
	if (cell){
		if (cell->m_textProp){
			cell->m_textProp->assignItem(m_textDec_Selection);
		}
		if (cell->m_borderProp){
			cell->m_borderProp->assignItem(m_borderProp_Selection);
		}
	}
	return retVal;
}



#define UORPT_OFFSET_LINE 2

uoReportCtrl::uoReportCtrl(QWidget *parent)
	: QWidget(parent) //, m_cornerWidget(parent)
{
	setMouseTracking(true); 	/// начинаем ловить перемещение мышки....
	setContextMenuPolicy(Qt::DefaultContextMenu);
	setFocusPolicy(Qt::StrongFocus);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	m_rptDoc 		= NULL;
	m_useMode 		= rmDevelMode;
	m_resizeLine		= 0;
	m_freezUpdate 	= 0;
	m_rowCountDoc = m_colCountDoc = 0;
	m_fixationType = uorAF_None;
	m_fixationPoint = QPointF(0.0, 0.0);

	m_charWidthPlus 	= 3;

	m_maxVisibleLineNumberCnt = 3;

	m_areaMain.m_firstVisible_RowTop 	= 1; 	///< Первая верхняя видимая строка
	m_areaMain.m_firstVisible_ColLeft 	= 1; 	///< Первая левая видимая колонка
	m_areaMain.m_lastVisibleRow 		= -1;
	m_areaMain.m_lastVisibleCol 		= -1;

	m_scaleFactor 			= m_scaleFactorO = 1;

	m_areaMain.m_shift_RowTop 	= 0; ///< Смещение первой видимой строки вверх (грубо - размер невидимой их части)
	m_areaMain.m_shift_ColLeft 	= 0;

	m_groupListCache = new uoRptGroupItemList;	///< кешь для экземпляров uoRptGroupItem
	m_groupListV = new uoRptGroupItemList;		///< список ректов группировок столбцов
	m_groupListH = new uoRptGroupItemList;		///< список ректов группировок строк

	m_sectItemListCache = new uoRptSectionItemList;	///< кешь для экземпляров uoRptSectionItem
	m_sectItemListV = new uoRptSectionItemList;		///< список итемов секций столбцов
	m_sectItemListH = new uoRptSectionItemList;		///< список итемов секций строк

	m_curentCell.setX(1);
	m_curentCell.setY(1);

	m_pageWidth 	= 0;
	m_pageHeight = 0;
	m_sizeVDoc 	= m_sizeHDoc = 0;
	m_sizeVvirt 	= m_sizeHvirt = 0;
	m_rowCountVirt = 0;
	m_colCountVirt = 0;
	m_rowsInPage = m_colsInPage = 1;

	setStateMode(rmsNone);

	initControls(parent);

	m_iteractView = new uoReportViewIteract();
	m_iteractView->createActions();
	m_iteractView->connectActions(this);


	m_selections = new uoReportSelection(this);
	connect(m_iteractView, SIGNAL(onScaleChange(qreal)), this, SLOT(onSetScaleFactor(qreal)));
	connect(m_selections, SIGNAL(onSelectonChange(uorSelectionType,uorSelectionType)), this, SLOT(onSelectonChange(uorSelectionType,uorSelectionType)));
	setDoc(new uoReportDoc());

	m_drawHelper = new uoReportDrawHelper(getDoc());
	m_drawHelper->m_showInvisiblChar = false;

	m_charWidthPlus 	= 3;
 	m_showGroup		= false;
	m_showSection	= false;
	m_showRuler		= true;
	m_showGrid		= true;
	m_showFrame		= true;



	m_imageView = QImage(size(), QImage::Format_ARGB32_Premultiplied);
	m_drawToImage = 1;
	m_fixationPoint = QPoint(0,0);
	m_fixationType = uorAF_None;
}

/// Инициализация контролов поля отчета.
void uoReportCtrl::initControls(QWidget *parent){

	m_textEdit = NULL; // создадим когда понадобится...

	m_cornerWidget = new QWidget(parent); //, m_cornerWidget(parent)
	QGridLayout* layout = new  QGridLayout(parent);
	layout->setMargin(0);	/// нулевой отступ...
	layout->setSpacing(0);
	m_hScrollCtrl = new QScrollBar(Qt::Horizontal, parent);
	m_vScrollCtrl = new QScrollBar(Qt::Vertical, parent);
	m_vScrollCtrl->setLayoutDirection(Qt::RightToLeft);
	m_vScrollCtrl->setFocusPolicy(Qt::StrongFocus);

	m_hScrollCtrl->setValue(1);
	m_hScrollCtrl->setMinimum(1);

	m_vScrollCtrl->setMinimum(1);
	m_vScrollCtrl->setValue(1);
	connect(m_vScrollCtrl, SIGNAL(valueChanged(int)), this, SLOT(onSetVScrolPos(int)));
	connect(m_vScrollCtrl, SIGNAL(actionTriggered(int)), this, SLOT(onScrollActionV(int)));

	connect(m_hScrollCtrl, SIGNAL(valueChanged(int)), this, SLOT(onSetHScrolPos(int)));
	connect(m_hScrollCtrl, SIGNAL(actionTriggered(int)), this, SLOT(onScrollActionH(int)));

    layout->addWidget(m_cornerWidget,1,1);
	layout->addWidget( this, 0, 0 );
	layout->addWidget(m_vScrollCtrl,0,1 );
    layout->addWidget(m_hScrollCtrl,1,0);
    layout->setColumnStretch(0,1);
	layout->setColumnStretch(1,0);
    layout->setRowStretch(0,1);
    layout->setRowStretch(1,0);

    m_messageFilter = new uoReportCtrlMesFilter(this);
    m_propEditor = NULL;

}


uoReportCtrl::~uoReportCtrl()
{
	//delete _repoDoc; //НЕТ!! нужно смотреть не подсоединен ли этот док к еще одному вьюву или к переменной!!!

	delete m_iteractView;
	clear();
}

///Очистка данных класса.
void uoReportCtrl::clear(){
	dropGropItemToCache();
	while (!m_groupListCache->isEmpty())    delete m_groupListCache->takeFirst();
	m_scaleStartPositionMapH.clear();
	m_scaleStartPositionMapV.clear();
}
/// Установить документа для вьюва.
void uoReportCtrl::setDoc(uoReportDoc* rptDoc)
{
	if (m_rptDoc){
		m_rptDoc->detachedView(this);
		if (!m_rptDoc->isObjectAttached()) {
			delete m_rptDoc;
			m_rptDoc = NULL;
		}
	}
	m_rptDoc = rptDoc;
	m_rptDoc->attachView(this);
	connect(m_rptDoc, SIGNAL(onSizeChange(qreal,qreal)), this, SLOT(changeDocSize(qreal, qreal)));


}

/// Обнуляем QRectF
inline void zeroQRectF(QRectF& rct) { rct.setLeft(0);	rct.setTop(0);	rct.setRight(0);	rct.setBottom(0); }
void zeroQRect(QRect& rct) { rct.setLeft(0);	rct.setTop(0);	rct.setRight(0);	rct.setBottom(0); }

/// Перекалькулируем последние видимые ячейки и сопутствующие данные.
int uoReportCtrl::recalcVisibleScales(uoRptHeaderType rht){

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return -1;
	// так бля. начинаю рефакторинг. буду ломать.. (((((
	int numScale = 1;
	qreal curetnSize = 0;

	qreal sizeScale = 0;
	bool isHiden = false;
	qreal targetSize = (rht==uorRhtColumnHeader) ? m_rectDataRegion.right() : m_rectDataRegion.bottom();
	qreal shiftSize = 0.0;
	if (rht == uorRhtColumnHeader) {
		m_scaleStartPositionMapH.clear();
		numScale 		= m_areaMain.m_firstVisible_ColLeft;
		shiftSize 	= m_rectDataRegion.left();
		curetnSize =  shiftSize - m_areaMain.m_shift_ColLeft;
		m_scaleStartPositionMapH[numScale] = curetnSize;
	} else if (rht == uorRhtRowsHeader){
		m_scaleStartPositionMapV.clear();
		numScale 		= m_areaMain.m_firstVisible_RowTop;
		curetnSize = m_rectDataRegion.top() - m_areaMain.m_shift_RowTop;
		m_scaleStartPositionMapV[numScale] = curetnSize;
	} else {
		return -1;
	}
	// fullVisScale = полностью видимая
	int fullVisScale = numScale;
	sizeScale 	= 0;
	do
	{
		isHiden = doc->getScaleHide(rht, numScale);
		if (!isHiden) {
			sizeScale = doc->getScaleSize(rht, numScale);
			curetnSize = curetnSize + sizeScale;
		}
		numScale = numScale + 1;
		if (rht == uorRhtColumnHeader){
			m_scaleStartPositionMapH[numScale] = curetnSize;
		}
		else {
			m_scaleStartPositionMapV[numScale] = curetnSize;
		}
		if (curetnSize < targetSize) {
			fullVisScale = numScale;
		}

	}
	while(curetnSize < targetSize);
	return fullVisScale;
}

void uoReportCtrl::dropGropItemToCache()
{
	uoRptGroupItem* item = NULL;
	while (!m_groupListV->isEmpty()) {
		item = m_groupListV->takeFirst();
		item->clear();
		m_groupListCache->append(item);
	}
	while (!m_groupListH->isEmpty()) {
		item = m_groupListH->takeFirst();
		item->clear();
		m_groupListCache->append(item);
	}

	uoRptSectionItem* rsItem = NULL;
	while (!m_sectItemListV->isEmpty()) {
		rsItem = m_sectItemListV->takeFirst();
		rsItem->clear();
		m_sectItemListCache->append(rsItem);
	}
	while (!m_sectItemListH->isEmpty()) {
		rsItem = m_sectItemListH->takeFirst();
		rsItem->clear();
		m_sectItemListCache->append(rsItem);
	}
}

/// Взять uoRptGroupItem из кеша, если кеш пуст, то создать.
uoRptGroupItem* uoReportCtrl::getGropItemFromCache()
{
	uoRptGroupItem* item = NULL;
	if (!m_groupListCache->isEmpty())
		item = m_groupListCache->takeFirst();
	else
		item = new uoRptGroupItem();
	return item;
}

/// Взять uoRptSectionItem из кеша, если кеш пуст, то создать.
uoRptSectionItem* uoReportCtrl::getSectionItemFromCache()
{
	uoRptSectionItem* item = NULL;
	if (!m_sectItemListCache->isEmpty())
		item = m_sectItemListCache->takeFirst();
	else
		item = new uoRptSectionItem();
	return item;
}


/// Вычисление координат гроуп итема..
void uoReportCtrl::calcGroupItemPosition(uoRptGroupItem* grItem, uoRptHeaderType rht)
{
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	grItem->m_rectMidlePos = 0.0;
	grItem->m_rectEndPos = 0.0;
	grItem->m_sizeTail = 0.0;
	grItem->m_tailPosIsAbs = false;
	bool itemVisible = false;


	qreal xPos = -10,yPos = -10, yPos0 = -10, xSize = 0, xSize0 = 0, ySize = 0, ySize0 = 0;
	if (rht == uorRhtColumnHeader) {

		if (m_scaleStartPositionMapH.contains(grItem->m_start))
			xPos = m_scaleStartPositionMapH[grItem->m_start];

		itemVisible = colVisible(grItem->m_start);
		if (itemVisible) {

			xSize0 = xSize = doc->getScaleSize(rht, grItem->m_start);
			grItem->m_rectIteract.setLeft(xPos);
			grItem->m_rectIteract.setRight(xPos);


			grItem->m_sizeTail = xPos + xSize; // Координаты конца ячейки.

			if (xSize0 > (m_charWidthPlus + UORPT_OFFSET_LINE*2)){


				xSize = (xSize - (UORPT_OFFSET_LINE + m_charWidthPlus)) / 2;
				xPos = xPos + xSize;
				grItem->m_rectIteract.setLeft(xPos);
				xPos = xPos + UORPT_OFFSET_LINE * 2 + m_charWidthPlus;
				grItem->m_rectIteract.setRight(xPos);
			} else if (xSize0>0){
				grItem->m_rectIteract.setLeft(xPos);
				xPos = xPos + xSize;
				grItem->m_rectIteract.setRight(xPos);
			}
		} else {
			grItem->m_rectIteract.setLeft(-10);
			grItem->m_rectIteract.setRight(-20);
		}

		yPos = m_rectGroupH.top() + ( UORPT_OFFSET_LINE * 2 + m_charHeightPlus) * grItem->m_level - UORPT_OFFSET_LINE;
		grItem->m_rectIteract.setBottom(yPos);
		yPos = yPos - m_charHeightPlus;
		grItem->m_rectIteract.setTop(yPos);

		grItem->m_rectMidlePos = yPos + m_charHeightPlus / 2;

		grItem->m_rectIteract.adjust(0.0, 1.0, 0.0, -1.0);
		grItem->m_rectEndPos = xPos;
		grItem->m_sizeTail = grItem->m_sizeTail - xPos; // Координаты конца ячейки.


	} else if (rht == uorRhtRowsHeader) {
		if (m_scaleStartPositionMapV.contains(grItem->m_start))
			yPos0 = yPos = m_scaleStartPositionMapV[grItem->m_start];

		itemVisible = rowVisible(grItem->m_start);

		if (itemVisible) {
			grItem->m_sizeTail = yPos;

			ySize0 = ySize = doc->getScaleSize(rht, grItem->m_start);
			ySize = (ySize  - m_charHeightPlus) / 2;
			grItem->m_sizeTail = yPos + ySize0; // Координаты конца ячейки.

			grItem->m_rectIteract.setTop(yPos + ySize);
			ySize = ySize + m_charHeightPlus; // + UORPT_OFFSET_LINE*2;
			grItem->m_rectIteract.setBottom(yPos + ySize);
			if (grItem->m_rectIteract.height()> ySize0){
				grItem->m_rectIteract.setTop(yPos0);
				grItem->m_rectIteract.setBottom(yPos0 + ySize0);
			}


			if (ySize0 < grItem->m_rectIteract.height() && (yPos0 - 1) > 3) {
				grItem->m_rectIteract.setTop(yPos0 + 1);
				grItem->m_rectIteract.setBottom(yPos0 + ySize0 - 2);
			}
		} else {
			// ну вот так топорно решим глюку..
			grItem->m_rectIteract.setTop(-10);
			grItem->m_rectIteract.setBottom(-20);
		}


		xPos = m_rectGroupV.left() + ( UORPT_OFFSET_LINE * 2 + m_charWidthPlus) * grItem->m_level;
		grItem->m_rectIteract.setRight(xPos);
		xPos = xPos - (m_charWidthPlus + UORPT_OFFSET_LINE * 2);
		grItem->m_rectIteract.setLeft(xPos);

		grItem->m_rectMidlePos = xPos + m_charWidthPlus / 2 + UORPT_OFFSET_LINE;

		grItem->m_rectIteract.adjust(1.0, 0.0, -1.0,0.0);

		grItem->m_rectEndPos = grItem->m_rectIteract.bottom();
		grItem->m_sizeTail = grItem->m_sizeTail - grItem->m_rectEndPos; // Координаты конца ячейки.


	}
}

/// Вычислить длину диапазона ячеек.
qreal uoReportCtrl::getLengthOfScale(uoRptHeaderType rht, int start, int stop)
{
	qreal retVal = 0.0;

	if (rht == uorRhtUnknown || start > stop || stop<=0 || start<=0)
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
void uoReportCtrl::recalcGroupSectionRects(uoRptHeaderType rht){
	// Нужно просчитать последнюю показываемую ячейку
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;

	// Сбрасываем гроуп итемы в кешь.
	dropGropItemToCache();

	int i = 0;
	qreal rSize = rptSizeNull;

	// Маленькая оптимизация. Если у нас поменялась только высота строки, не нужно пересчитывать вертикальный хейдер.
	if (rht == uorRhtUnknown || rht == uorRhtColumnHeader)
		m_areaMain.m_lastVisibleCol = recalcVisibleScales(uorRhtColumnHeader);

	if (rht == uorRhtUnknown || rht == uorRhtRowsHeader) {
		m_areaMain.m_lastVisibleRow = recalcVisibleScales(uorRhtRowsHeader);
	}

	uoLineSpan* spn;
	uoRptHeaderType rhtCur = rht;

	if (m_showGroup) {
		uoRptGroupItem* grItem;

		int spnCnt = 0;

		if (rht == uorRhtUnknown || rht == uorRhtColumnHeader) {
			rhtCur = uorRhtColumnHeader;
			spnCnt = doc->getGroupLevel(rhtCur);
			if (spnCnt>0) {
				const spanList* spanListH = doc->getGroupList(rhtCur, m_areaMain.m_firstVisible_ColLeft, m_areaMain.m_lastVisibleCol);
				for (i=0; i<spanListH->size(); i++){
					spn = spanListH->at(i);
					grItem = getGropItemFromCache();
					if (grItem)	{
						grItem->copyFrom(spn);
						grItem->m_sizeTail = rptSizeNull;
						zeroQRectF(grItem->m_rectIteract);
						grItem->m_tailPosIsAbs = false;

						rSize = rptSizeNull;
						calcGroupItemPosition(grItem, rhtCur);
						// посчитаем длину линии группировки.
						// хвост есть, нужно вычислить только толщину последующих ячеек и добавить его к хвосту.
						if (grItem->m_start >= m_areaMain.m_firstVisible_ColLeft){
							// это если рект группировки нормально отображается, а если нет?
							grItem->m_sizeTail = grItem->m_sizeTail + getLengthOfScale(rhtCur, grItem->m_start + 1, grItem->m_end);
						} else {
							grItem->m_sizeTail = 0.0;
							if (m_areaMain.m_firstVisible_ColLeft <= grItem->m_end) {
								grItem->m_tailPosIsAbs = true;
								grItem->m_sizeTail = getLengthOfScale(rhtCur, m_areaMain.m_firstVisible_ColLeft, grItem->m_end) - m_areaMain.m_shift_ColLeft;
							}
						}
						m_groupListH->append(grItem);
					}
				}
				delete spanListH;
			}
		}

//		rht = uorRhtRowsHeader;
		if (rht == uorRhtUnknown || rht == uorRhtRowsHeader) {
			rhtCur = uorRhtRowsHeader;
			spnCnt = doc->getGroupLevel(rhtCur);
			if (spnCnt>0) {
				const spanList* spanListV = doc->getGroupList(rhtCur, m_areaMain.m_firstVisible_RowTop, m_areaMain.m_lastVisibleRow);
				for (i=0; i<spanListV->size(); i++){
					spn = spanListV->at(i);
					grItem = getGropItemFromCache();
					if (grItem)	{
						grItem->copyFrom(spn);

						zeroQRectF(grItem->m_rectIteract);
						grItem->m_tailPosIsAbs = false;
						calcGroupItemPosition(grItem, rhtCur);


						if (grItem->m_start >= m_areaMain.m_firstVisible_RowTop){
							// это если рект группировки нормально отображается, а если нет?
							grItem->m_sizeTail = grItem->m_sizeTail + getLengthOfScale(rhtCur, grItem->m_start + 1, grItem->m_end);
						} else {
							grItem->m_sizeTail = 0.0;
							if (m_areaMain.m_firstVisible_RowTop <= grItem->m_end) {
								grItem->m_tailPosIsAbs = true;
								grItem->m_sizeTail = getLengthOfScale(rhtCur, m_areaMain.m_firstVisible_RowTop, grItem->m_end) - m_areaMain.m_shift_RowTop;
							}
						}

						m_groupListV->append(grItem);
					}
				}
				delete spanListV;
			}
		}
	} //if (m_showGroup)

	if (m_showSection)
	{
		uoRptSectionItem* selectItem = NULL;
		int sectLevel = 0;
		int cellVisStart = 0, cellVisEnd = 0; // Видимые ячейки
		qreal coord = 0.0;
		qreal offSet = 0.0;

		if (rht == uorRhtUnknown || rht == uorRhtColumnHeader)
		{
			rhtCur = uorRhtColumnHeader;
			sectLevel = doc->getSectionLevel(rhtCur);
			if (sectLevel>0){

				const spanList* spanSectListH = doc->getSectionList(rhtCur, m_areaMain.m_firstVisible_ColLeft, m_areaMain.m_lastVisibleCol);
				for (i=0; i<spanSectListH->size(); i++){
					spn = spanSectListH->at(i);
					selectItem = getSectionItemFromCache();
					if (selectItem)	{
						selectItem->copyFrom(spn);
						zeroQRectF(selectItem->m_rectView);
						selectItem->m_selected = false;

						// Посчитаем рект данной секции....
						QRectF& rect = selectItem->m_rectView;
						offSet = 0.0;
						coord = m_rectSectionH.top();
						coord = coord - ( UORPT_OFFSET_LINE * 2 + m_charHeightPlus) * (1 - selectItem->m_level);
						// С каждым уровнем секция все "ниже". Вот только не помню, левел нуль-ориентрованный или как?
						rect.setTop(coord);
						rect.setBottom(m_rectSectionH.bottom());

						cellVisStart = qMax(selectItem->m_start, m_areaMain.m_firstVisible_ColLeft);
						cellVisEnd = qMin(selectItem->m_end, m_areaMain.m_lastVisibleCol);

						coord = m_rectSectionH.left()-1; // пусть спрячется, если что..
						if (m_scaleStartPositionMapH.contains(cellVisStart)){
							coord = m_scaleStartPositionMapH[cellVisStart];
							if (cellVisStart == m_areaMain.m_firstVisible_ColLeft)
								offSet = m_areaMain.m_shift_ColLeft;
						}

						rect.setLeft(coord+offSet);
						coord = getLengthOfScale(rhtCur, cellVisStart, cellVisEnd);
						rect.setRight(rect.left() + coord-offSet);
						m_sectItemListH->append(selectItem);
					}
				}
				delete spanSectListH;
			}
		}
		if (rht == uorRhtUnknown || rht == uorRhtRowsHeader)
		{
			rhtCur = uorRhtRowsHeader;
			sectLevel = doc->getSectionLevel(rhtCur);
			if (sectLevel>0){

				const spanList* spanSectListV = doc->getSectionList(rhtCur, m_areaMain.m_firstVisible_RowTop, m_areaMain.m_lastVisibleRow);
				for (i=0; i<spanSectListV->size(); i++){
					spn = spanSectListV->at(i);
					selectItem = getSectionItemFromCache();
					if (selectItem)	{
						selectItem->copyFrom(spn);
						zeroQRectF(selectItem->m_rectView);
						selectItem->m_selected = false;

						cellVisStart = qMax(selectItem->m_start, m_areaMain.m_firstVisible_RowTop);
						cellVisEnd = qMin(selectItem->m_end, m_areaMain.m_lastVisibleRow);

						// Посчитаем рект данной секции....
						QRectF& rect = selectItem->m_rectView;
						offSet = 0.0;
						coord = 0.0;
						coord = m_rectSectionV.left();
						offSet = m_rectSectionV.right() - m_rectSectionV.left();

						offSet = offSet * (selectItem->m_level-1) / sectLevel;
						coord = coord + offSet;
						offSet = 0.0;

//						coord = coord - ( UORPT_OFFSET_LINE * 2 + m_charHeightPlus * UORPT_LENGTH_TEXT_H_SECTION) * (1 - selectItem->m_level);
						// С каждым уровнем секция все "ниже". Вот только не помню, левел нуль-ориентрованный или как?
						rect.setLeft(coord);
						rect.setRight(m_rectSectionV.right());

						coord = m_rectSectionV.top()-2; // пусть спрячется, если что..
						if (m_scaleStartPositionMapV.contains(cellVisStart)){
							coord = m_scaleStartPositionMapV[cellVisStart];
							if (cellVisStart == m_areaMain.m_firstVisible_RowTop)
								offSet = m_areaMain.m_shift_RowTop;
						}
						rect.setTop(coord+offSet);
						coord = getLengthOfScale(rhtCur, cellVisStart, cellVisEnd);
						rect.setBottom(rect.top() + coord-offSet);
						m_sectItemListV->append(selectItem);
					}
				}
				delete spanSectListV;
			}
		}
	} // if (m_showSection)
	updateImage();
}
/**
	Надо пересчитать размеры фиксированных областей вьюва.
*/
void uoReportCtrl::recalcFixedAreasRects()
{
	if(m_fixationType == uorAF_None)
		return;
	uorReportViewArea* area = NULL;
	uoReportDoc* doc = getDoc();
	qreal sz = 0.0;
	if (!doc)
		return;
	uoRptHeaderType rht = uorRhtRowsHeader;
	if (m_fixationType == uorAF_Rows || m_fixationType == uorAF_RowsAndCols) {
		area = &m_areas[0];
		sz = area->m_shift_RowTop;
		for (int i = area->m_firstVisible_RowTop;		i<=area->m_lastVisibleRow;		i++)
		{
			if (!doc->getScaleHide(rht, i)){
				sz += doc->getScaleSize(rht, i);
			}
		}
		m_fixationPoint.setY(m_rectDataRegion.top() + sz);
	}
	if (m_fixationType == uorAF_Cols || m_fixationType == uorAF_RowsAndCols) {
		rht	= uorRhtColumnHeader;
		if (m_fixationType == uorAF_Cols)
			area = &m_areas[1];
		else
			area = &m_areas[3];
		sz = area->m_shift_ColLeft;
		for (int i = area->m_firstVisible_ColLeft;		i<=area->m_lastVisibleCol;		i++)
		{
			if (!doc->getScaleHide(rht, i)){
				sz += doc->getScaleSize(rht, i);
			}
		}
		m_fixationPoint.setX(m_rectDataRegion.left() + sz);
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
    m_charWidthPlus = fm.width("+");
    m_charHeightPlus = fm.height();
    qreal wWidth = getWidhtWidget();
    qreal wHeight = getHeightWidget();


	zeroQRectF(m_rectGroupV);  zeroQRectF(m_rectSectionV);	zeroQRectF(m_rectRulerV);
	zeroQRectF(m_rectGroupH);  zeroQRectF(m_rectSectionH);	zeroQRectF(m_rectRulerH);
	zeroQRectF(m_rectRuleCorner);
	zeroQRectF(m_rectAll);
	zeroQRectF(m_rectDataRegion);	zeroQRectF(m_rectDataRegionFrame);

	m_rectAll.setTopLeft(QPoint(1,1));
	m_rectAll.setBottom(wHeight);			m_rectAll.setRight(wWidth);
	m_rectDataRegion.setBottom(wHeight);	m_rectDataRegion.setRight(wWidth);
	m_rectGroupH.setRight(wWidth);		m_rectSectionH.setRight(wWidth); 	m_rectRulerH.setRight(wWidth);
	m_rectGroupV.setBottom(wHeight);	m_rectSectionV.setBottom(wHeight); 	m_rectRulerV.setBottom(wHeight);

	qreal curOffset = 0;

	// Расчитаем сначала высотные размеры горизонтальной секции
	int spnCntH = doc->getGroupLevel(uorRhtColumnHeader);
	if (spnCntH>0 && m_showGroup) {
		curOffset += UORPT_OFFSET_LINE; // Оффсет-отступ сверху.
		m_rectGroupH.setTop(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + m_charHeightPlus) * spnCntH;
		m_rectGroupH.setBottom(curOffset);
	}
	spnCntH = doc->getSectionLevel(uorRhtColumnHeader);
	if (spnCntH>0 && m_showSection) {
		curOffset += UORPT_OFFSET_LINE; // Оффсет-отступ сверху.
		m_rectSectionH.setTop(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + m_charHeightPlus) * spnCntH;
		m_rectSectionH.setBottom(curOffset);
	}

	if (m_showRuler) {
//		curOffset += UORPT_OFFSET_LINE;
		m_rectRulerH.setTop(curOffset);
		curOffset += m_charHeightPlus;
		curOffset += UORPT_OFFSET_LINE;
		m_rectRulerH.setBottom(curOffset);
//		curOffset += UORPT_OFFSET_LINE;
	}
	m_rectDataRegion.setTop(curOffset);
	m_rectGroupV.setTop(curOffset);	m_rectSectionV.setTop(curOffset);	m_rectRulerV.setTop(curOffset);

	curOffset = 0;
	int spnCntV = doc->getGroupLevel(uorRhtRowsHeader);
	if (spnCntV>0 && m_showGroup) {
		curOffset += UORPT_OFFSET_LINE;
		m_rectGroupV.setLeft(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + m_charWidthPlus) * spnCntV;
		m_rectGroupV.setRight(curOffset);
	}
	spnCntV = doc->getSectionLevel(uorRhtRowsHeader);
	if (spnCntV>0 && m_showSection) {
		curOffset += UORPT_OFFSET_LINE; // Оффсет-отступ сверху.
		m_rectSectionV.setLeft(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + m_charWidthPlus*UORPT_LENGTH_TEXT_H_SECTION) * spnCntV;
		m_rectSectionV.setRight(curOffset);
	}

	if (m_showRuler) {
//		curOffset += UORPT_OFFSET_LINE;
		m_rectRulerV.setLeft(curOffset);
		curOffset += m_charWidthPlus * m_maxVisibleLineNumberCnt+5;
		curOffset += UORPT_OFFSET_LINE;
		m_rectRulerV.setRight(curOffset);
//		curOffset += UORPT_OFFSET_LINE;

		m_rectRuleCorner.setTop(m_rectRulerH.top());
		m_rectRuleCorner.setLeft(m_rectRulerV.left());
		m_rectRuleCorner.setBottom(m_rectRulerH.bottom());
		m_rectRuleCorner.setRight(m_rectRulerV.right());
	}
	m_rectDataRegion.setLeft(curOffset);
	m_rectDataRegionFrame.setTop(m_rectDataRegion.top());
	m_rectDataRegionFrame.setBottom(m_rectDataRegion.bottom());
	m_rectDataRegionFrame.setLeft(m_rectDataRegion.left());
	m_rectDataRegionFrame.setRight(m_rectDataRegion.right());
	m_rectDataRegion.adjust(1, 1, -1, -1);

	m_rectGroupH.setLeft(curOffset);		m_rectSectionH.setLeft(curOffset); 	m_rectRulerH.setLeft(curOffset);

	// обнулим ненужные...
	if (!m_showGroup)	{zeroQRectF(m_rectGroupV);  	zeroQRectF(m_rectGroupH);		}
	if (!m_showSection) 	{zeroQRectF(m_rectSectionV); 	zeroQRectF(m_rectSectionH);	}
	if (!m_showRuler) 	{zeroQRectF(m_rectRulerV); 	zeroQRectF(m_rectRulerH);	zeroQRectF(m_rectRuleCorner);}

	m_rowsInPage = (int) (m_rectDataRegion.height() / doc->getDefScaleSize(uorRhtRowsHeader)); 		///< строк на страницу
	m_colsInPage = (int) (m_rectDataRegion.width() / doc->getDefScaleSize(uorRhtColumnHeader)); 		///< столбцов на страницу

	m_areaMain.m_area = m_rectDataRegion;
	recalcFixedAreasRects();
	recalcGroupSectionRects();
}



/// Вывод отладочной информации по размерам ректов.
void uoReportCtrl::debugRects()
{
//	qDebug() << "-----------------------------------";
//	qDebug() << "uoReportCtrl::debugRects() {";
//	qDebug() << "m_rectAll" << 			m_rectAll;
//	qDebug() << "m_rectGroupV" << 		m_rectGroupV;
//	qDebug() << "m_rectSectionV" << 		m_rectSectionV;
//	qDebug() << "m_rectRulerV" << 		m_rectRulerV;
//	qDebug() << "m_rectGroupH" << 		m_rectGroupH;
//	qDebug() << "m_rectSectionH" << 		m_rectSectionH;
//	qDebug() << "m_rectRulerH" << 		m_rectRulerH;
//	qDebug() << "m_rectDataRegion" << 	m_rectDataRegion;

//	if (m_showFrame) qDebug() << "m_showFrame";
//	if (m_showRuler) qDebug() << "m_showRuler";
//	if (m_showSection) qDebug() << "m_showSection";
//	if (m_showGroup) qDebug() << "m_showGroup";
//	if (m_showGrid) qDebug() << "m_showGrid";

//	qDebug() << "m_shift_RowTop" << m_shift_RowTop;
//	qDebug() << "m_firstVisible_RowTop" << m_firstVisible_RowTop;
//	qDebug() << "m_lastVisibleRow" << m_lastVisibleRow;

//	qDebug() << "m_shift_ColLeft" << m_shift_ColLeft;
//	qDebug() << "m_firstVisible_ColLeft" << m_firstVisible_ColLeft;
//	qDebug() << "m_lastVisibleCol" << m_lastVisibleCol;


//	int cntr;
//	uoRptGroupItem* rgItem;
//
//	if (!m_groupListH->isEmpty()) {
//		qDebug() << "m_groupListH";
//		for (cntr = 0; cntr<m_groupListH->size(); cntr++) {
//			rgItem = m_groupListH->at(cntr);
//			qDebug() << rgItem->m_rectIteract << rgItem->m_start << rgItem->m_end;
//		}
//	}
//
//	if (!m_groupListV->isEmpty()) {
//		qDebug() << "m_groupListV";
//		for (cntr = 0; cntr<m_groupListV->size(); cntr++) {
//			rgItem = m_groupListV->at(cntr);
//			qDebug() << rgItem->m_rectIteract << rgItem->m_start << rgItem->m_end;
//		}
//	}

//	qDebug() << "uoReportCtrl::debugRects() }";
}

/// Тестовая отрисовка контура контрола, для визуального контроля и отладки.
void uoReportCtrl::drawHeaderControlContour(QPainter& painter)
{
	qreal noLen = 2;
	bool drawSelfRects = false;
	painter.save();
	m_drawHelper->m_penText.setStyle(Qt::DotLine);
	painter.setPen(m_drawHelper->m_penText);
	// Рисуем контуры пространств, чисто для визуального контроля...
	if (m_showGroup) {
		if (m_rectGroupV.width()>noLen)	{
			if (drawSelfRects)
				painter.drawRect(m_rectGroupV);
		}
		if (m_rectGroupH.height()>noLen)		{
			if (drawSelfRects)
				painter.drawRect(m_rectGroupH);
		}
	}
	if (m_showSection) {
		if (m_rectSectionV.width()>noLen) {
			if (drawSelfRects)
				painter.drawRect(m_rectSectionV);
		}
		if (m_rectSectionH.height()>noLen) {
			if (drawSelfRects)
					painter.drawRect(m_rectSectionH);
		}
	}
	if (m_showRuler) {
		if (m_rectRulerV.width()>noLen) {
			if (drawSelfRects)
				painter.drawRect(m_rectRulerV);
		}
		if (m_rectRulerH.height()>noLen) {
			if (drawSelfRects)
				painter.drawRect(m_rectRulerH);
		}
	}

	painter.restore();
}


/// Отрисовка групп в HeaderControl.
void uoReportCtrl::drawHeaderControlGroup(QPainter& painter)
{
	int cntr = 0;
	uoRptGroupItem* grItem;
	qreal noLen = 2;
	qreal minDrawSize = 2.5;
	QString paintStr = "";
	QPointF pointStart, pointEnd;
	qreal pos = 0.0;

	painter.save();
	m_drawHelper->m_penText.setStyle(Qt::SolidLine);
	painter.setPen(m_drawHelper->m_penText);

	// Рисуем шапку, как она сама есть...
	if (m_showGroup) {
		if (m_rectGroupV.width()>noLen)	{
			if (!m_groupListV->isEmpty()) {
				painter.setClipRect(m_rectGroupV); // Устанавливаем область прорисовки. Будем рисовать только в ней.
				for (cntr = 0; cntr<m_groupListV->size(); cntr++) {
					grItem = m_groupListV->at(cntr);

					if (grItem->m_rectIteract.height() > minDrawSize) {
						painter.drawRect(grItem->m_rectIteract);
						paintStr = "-";
						if (grItem->m_folded)
							paintStr = "+";
						painter.drawText(grItem->m_rectIteract,Qt::AlignCenter,  paintStr);
					}
					if (grItem->m_sizeTail > 0) {
						pointStart.setX(grItem->m_rectMidlePos);
						pointEnd.setX(grItem->m_rectMidlePos);

						pos = grItem->m_rectEndPos;
						if (grItem->m_tailPosIsAbs) {
							pos = m_rectGroupV.top(); //grItem->m_rectEndPos;
						}
						pointStart.setY(pos);
						pointEnd.setY(pos + grItem->m_sizeTail);

						painter.drawLine(pointStart, pointEnd );
						pointStart =  pointEnd;
						pointStart.setX(pointStart.x()+3);
						painter.drawLine(pointEnd, pointStart);
					}

				}
			}
		}
		if (m_rectGroupH.height()>noLen)		{
			if (!m_groupListH->isEmpty()) {
				painter.setClipRect(m_rectGroupH); // Устанавливаем область прорисовки. Будем рисовать только в ней.
				for (cntr = 0; cntr<m_groupListH->size(); cntr++) {
					grItem = m_groupListH->at(cntr);
					if (grItem->m_rectIteract.width() > minDrawSize) {
						painter.drawRect(grItem->m_rectIteract);
						paintStr = "-";
						if (grItem->m_folded)
							paintStr = "+";
						painter.drawText(grItem->m_rectIteract,Qt::AlignCenter,  paintStr);
					}
					if (grItem->m_sizeTail > 0) {
						pointStart.setY(grItem->m_rectMidlePos);
						pointEnd.setY(grItem->m_rectMidlePos);
						pos = grItem->m_rectEndPos;

						if (grItem->m_tailPosIsAbs) {
							pos = m_rectGroupH.left();
						}
						pointStart.setX(pos);
						pointEnd.setX(pos + grItem->m_sizeTail);

						painter.drawLine(pointStart, pointEnd );
						pointStart =  pointEnd;
						pointStart.setY(pointStart.y()+3);
						painter.drawLine(pointEnd, pointStart);
					}
				}
			}
		}
	}
	painter.setClipRect(m_rectAll); // Устанавливаем область прорисовки. Будем рисовать только в ней.
	if (m_showSection){
		uoRptSectionItem* sectItem = NULL;
		if (!m_sectItemListH->isEmpty()) {
			painter.setClipRect(m_rectSectionH); // Устанавливаем область прорисовки. Будем рисовать только в ней.
			for (cntr = 0; cntr<m_sectItemListH->size(); cntr++) {
				sectItem = m_sectItemListH->at(cntr);
				QRectF rect = sectItem->m_rectView;
				painter.setPen(m_drawHelper->m_penText);
				if (sectItem->m_selected){
					painter.fillRect(rect, m_drawHelper->m_brushBlack);
					painter.setPen(m_drawHelper->m_penWhiteText);
				} else {
					painter.fillRect(rect, m_drawHelper->m_brushWindow);
				}
				painter.drawRect(rect);
				rect.adjust(UORPT_OFFSET_LINE,UORPT_OFFSET_LINE,-UORPT_OFFSET_LINE,-UORPT_OFFSET_LINE);
				painter.drawText(rect,Qt::AlignLeft,  sectItem->m_nameSections);

			}
		}
		if (!m_sectItemListV->isEmpty()){
			painter.setClipRect(m_rectSectionV); // Устанавливаем область прорисовки. Будем рисовать только в ней.
			for (cntr = 0; cntr<m_sectItemListV->size(); cntr++) {
				sectItem = m_sectItemListV->at(cntr);
				QRectF rect = sectItem->m_rectView;
				painter.setPen(m_drawHelper->m_penText);
				if (sectItem->m_selected){
					painter.fillRect(rect, m_drawHelper->m_brushBlack);
					painter.setPen(m_drawHelper->m_penWhiteText);
				} else {
					painter.fillRect(rect, m_drawHelper->m_brushWindow);
				}
				painter.drawRect(rect);
				rect.adjust(UORPT_OFFSET_LINE,UORPT_OFFSET_LINE,-UORPT_OFFSET_LINE,-UORPT_OFFSET_LINE);
				painter.drawText(rect,Qt::AlignLeft,  sectItem->m_nameSections);

			}
		}
	}
	painter.setClipRect(m_rectAll); // Устанавливаем область прорисовки. Будем рисовать только в ней.
	painter.restore();
}


/// Отрисовка нерабочей области отчета: Группировки, секции, линейки, общий фрайм.
void uoReportCtrl::drawHeaderControl(QPainter& painter){


	if (m_showFrame) {
		painter.drawRect(m_rectAll);
	}

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	QString paintStr = "";

	m_drawHelper->m_penText.setStyle(Qt::SolidLine);
	painter.setPen(m_drawHelper->m_penText);

	bool isSell = false;
	/*
		попробуем порисовать линейку...
		вот тут нужен такой фокус, необходимо научиться рисовать
		отсеченные части ректов и прочих фигур.
	*/


	qreal paintEndTagr = rptSizeNull; // Конечная граница отрисовки.
	qreal paintCntTagr = rptSizeNull; // Текущая величина отрисовки.
	qreal curSize = rptSizeNull; // Текущая величина отрисовки.

	int nmLine = 0;
	uoRptHeaderType hdrType;
	QRectF curRct, curRctCpy; // копия, для извращений...
	QPointF posStart, posEnd;
	zeroQRectF(curRct);

	painter.drawRect(m_rectDataRegion);

	QPen oldPen;
	if (m_showRuler) {
		/* рисуем сначала вертикалку.
			| 10 |
			| 11 |
			| 12 |
		*/
		// Верхний корнер-виджет слева от горизонтальной и сверху от вертикальной линейки
		curRctCpy = m_rectRuleCorner;
		curRctCpy.adjust(1,1,-1,-1);
		painter.drawRect(curRctCpy);  //	painter.drawRect(m_rectRuleCorner);
		if (m_selections->isDocumSelect()){
			painter.setPen(m_drawHelper->m_penWhiteText);
			painter.drawRect(curRctCpy);
			curRctCpy.adjust(1,1,-1,-1);
			painter.fillRect(curRctCpy, m_drawHelper->m_brushBlack);
			painter.setPen(m_drawHelper->m_penText);
		}

		hdrType = uorRhtRowsHeader;
		if (m_rectRulerV.width() > 0) {

			painter.setClipRect(m_rectRulerV); // Устанавливаем область прорисовки. Будем рисовать только в ней.

			curRct.setLeft(m_rectRulerV.left()+m_scaleFactorO);
			curRct.setRight(m_rectRulerV.right()-m_scaleFactorO);

			paintEndTagr = m_rectRulerV.bottom();
			paintCntTagr = m_rectRulerV.top() - m_areaMain.m_shift_RowTop + 1 * m_scaleFactorO;
			curRct.setTop(paintCntTagr);
			nmLine = m_areaMain.m_firstVisible_RowTop-1;
			do {
				nmLine = nmLine + 1;

				if (doc->getScaleHide(hdrType, nmLine))
					continue;
				if ((curSize = doc->getScaleSize(hdrType, nmLine)) == 0.0)
					continue;

				isSell = m_selections->isRowSelect(nmLine);

				paintCntTagr = paintCntTagr + curSize;
				curRct.setBottom(paintCntTagr);
				painter.drawRect(curRct);

				if (isSell) {
					painter.save();
					painter.setPen(m_drawHelper->m_penWhiteText);
					painter.setBrush(m_drawHelper->m_brushBlack);
					curRctCpy = curRct;
					curRctCpy.adjust(1,1,-1,-1);
					painter.drawRect(curRctCpy);
				}

				paintStr.setNum(nmLine);
				m_maxVisibleLineNumberCnt = qMax(3, paintStr.length());
				painter.drawText(curRct, Qt::AlignCenter,  paintStr);
				curRct.setTop(paintCntTagr);
				if (isSell)
					painter.restore();
			} while(paintCntTagr < paintEndTagr);
			m_maxVisibleLineNumberCnt = qMax(3, paintStr.length());
		}
		painter.setClipRect(m_rectAll); // Устанавливаем область прорисовки. Будем рисовать только в ней.

		hdrType = uorRhtColumnHeader;
		if (m_rectRulerH.width() > 0) {

			painter.setClipRect(m_rectRulerH); // Устанавливаем область прорисовки. Будем рисовать только в ней.

			curRct.setTop(m_rectRulerH.top()+m_scaleFactorO);
			curRct.setBottom(m_rectRulerH.bottom()-m_scaleFactorO);

			paintEndTagr = m_rectRulerH.right();
			paintCntTagr = m_rectRulerH.left() - m_areaMain.m_shift_ColLeft + 1 * m_scaleFactorO;

			curRct.setLeft(paintCntTagr);
			nmLine = m_areaMain.m_firstVisible_ColLeft - 1;
			do {
				nmLine = nmLine + 1;

				if (doc->getScaleHide(hdrType, nmLine))
					continue;
				if ((curSize = doc->getScaleSize(hdrType, nmLine)) == 0.0)
					continue;

				paintCntTagr = paintCntTagr + curSize;

				isSell = m_selections->isColSelect(nmLine);


				curRct.setRight(paintCntTagr);
				painter.drawRect(curRct);
				paintStr.setNum(nmLine);

				if (isSell) {
					painter.save();
					painter.setPen(m_drawHelper->m_penWhiteText);
					painter.setBrush(m_drawHelper->m_brushBlack);
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
		painter.setClipRect(m_rectAll); // Устанавливаем область прорисовки. Будем рисовать только в ней.
	}
}


void uoReportCtrl::drawDataAreaResizeRuler(QPainter& painter)
{

	// Надо прорисовать линию будующей границы ячейки
	if (m_stateMode == rmsResizeRule_Top || m_stateMode == rmsResizeRule_Left){
		QPen oldPen = painter.pen();
		QPointF pnt1, pnt2;
		qreal xxx;
		if (m_stateMode == rmsResizeRule_Top) {
			xxx = qMax((qreal)m_curMouseCurPos.x(), m_curMouseSparesRect.left());

			pnt1.setY(m_rectDataRegion.top());
			pnt2.setY(m_rectDataRegion.bottom());
			pnt1.setX(xxx);
			pnt2.setX(xxx);

		} else {
			xxx = qMax((qreal)m_curMouseCurPos.y(), m_curMouseSparesRect.top());

			pnt1.setX(m_rectDataRegion.left());
			pnt2.setX(m_rectDataRegion.right());
			pnt1.setY(xxx);
			pnt2.setY(xxx);

		}
		if (m_scaleFactor != 1.0){
			pnt1 /= m_scaleFactorO;
			pnt2 /= m_scaleFactorO;
		}
		QLineF line(pnt1, pnt2);
		QPen redPen;
		redPen.setColor(Qt::red);
		redPen.setStyle(Qt::DashDotLine);
		painter.setPen(redPen);
		painter.drawLine(line);

		painter.setPen(oldPen);
	}
}


void uoReportCtrl::drawSpliter(QPainter& painter)
{
	if (m_fixationType == uorAF_None)
		return;
	QPointF pt1 = m_fixationPoint, pt2 = m_fixationPoint;


	if (m_fixationType == uorAF_Cols || m_fixationType == uorAF_RowsAndCols)
	{
		pt1.setY(m_rectAll.top());
		pt2.setY(m_rectAll.bottom());
		if (m_scaleFactor != 1.0){
			pt1 *= m_scaleFactorO;
			pt2 *= m_scaleFactorO;
		}
		painter.drawLine(pt1, pt2);
	}
	pt1 = m_fixationPoint, pt2 = m_fixationPoint;
	if (m_fixationType == uorAF_Cols || m_fixationType == uorAF_RowsAndCols)
	{
		pt1.setX(m_rectAll.left());
		pt2.setX(m_rectAll.right());
		if (m_scaleFactor != 1.0){
			pt1 /= m_scaleFactorO;
			pt2 /= m_scaleFactorO;
		}
		painter.drawLine(pt1, pt2);
	}

}

/**
	Инициализация кистей, карандашей и т.п
	Нужен потому, что раздельно используется процедуры
	рисования на виджете и при выводе на печать.
*/
void uoReportCtrl::initDrawInstruments()
{
	QPalette palette_c = palette();
	m_drawHelper->initDrawInstruments(this, &palette_c);
	m_drawHelper->setScaleFactor(m_scaleFactor);
	m_drawHelper->setCurCell(m_curentCell);
	m_drawHelper->m_showGrid = m_showGrid;
	m_drawHelper->m_selections = m_selections;
	m_drawHelper->m_showCurCell = true;
}
/// Отрисовка виджета.
void uoReportCtrl::drawWidget(QPainter& painter)
{
	initDrawInstruments();
	drawHeaderControlContour(painter);
	drawHeaderControlGroup(painter);
	drawHeaderControl(painter);
//	drawDataArea(painter, m_areaMain);
	m_drawHelper->drawDataArea(painter, m_areaMain);
	drawSpliter(painter);
}
/// Типа рисуем
void uoReportCtrl::paintEvent(QPaintEvent *event)
{
	if (m_drawToImage>0){
		m_imageView = QImage(size(), QImage::Format_ARGB32_Premultiplied);
		m_imageView.fill(0);
		QPainter painter(&m_imageView);
		painter.scale(m_scaleFactor, m_scaleFactor);
		painter.save();
		drawWidget(painter);
		painter.restore();
		m_drawToImage = 0;
	}

    QPainter painter(this);
    painter.drawImage(0,0,m_imageView);
	drawDataAreaResizeRuler(painter);

	if (m_cornerWidget && false){
		// Отрисовка квадратика закрывающего белый фон от данного контрола внизу вертикального скрола.
		painter.setPen(m_drawHelper->m_penNoPen);
		painter.scale(1/m_scaleFactor,1/m_scaleFactor);
		painter.fillRect(m_cornerWidget->frameGeometry(), m_drawHelper->m_brushWindow);
	}
}

/// Установить m_drawToImage в плюcовое значение и обновить виджет.
void uoReportCtrl::updateImage()
{
	if(m_drawToImage<0)
		m_drawToImage = 0;
	++m_drawToImage;
	emit update();
}

/// Реакция на нажатие мышки-норушки на группах.
bool uoReportCtrl::mousePressEventForGroup(QMouseEvent *event, bool isDoubleClick){
	bool retVal = false;
	if (event->button() != Qt::LeftButton)
		return retVal;
	qreal posX = event->x();
	qreal posY = event->y();

	if (m_scaleFactor != 1.0){
		posX = posX * m_scaleFactorO;
		posY = posY * m_scaleFactorO;
	}

	if (m_rectGroupV.contains(posX, posY) || m_rectGroupH.contains(posX, posY))
	{
		retVal = true;
		uoReportDoc* doc = getDoc();
		if (doc) {
			event->accept();
			uoRptGroupItemList* groupItList = m_groupListH;

			uoRptHeaderType rht = uorRhtColumnHeader;
			if (m_rectGroupV.contains(posX, posY)){
				rht = uorRhtRowsHeader;
				groupItList = m_groupListV;

			}
			if (!groupItList->isEmpty()) {
				uoRptGroupItem* rptGrItem = NULL;
				bool found = false;
				for (int i = 0; i< groupItList->size(); i++){
					rptGrItem = groupItList->at(i);

					if (rptGrItem->m_rectIteract.contains(posX, posY)){
						// Нашли итем на котором сделан клик мышкой.
						doc->doGroupFold(rptGrItem->m_id, rht, !rptGrItem->m_folded);
						found = true;
						break;
					}
				}
				if (found) {
					recalcGroupSectionRects();
					updateImage();
				}
			}
		}
	}
	return retVal;
}

/// Отработаем клик по области секций...
bool uoReportCtrl::mousePressEventForSection(QMouseEvent *event, bool isDoubleClick)
{
	bool retVal = false;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return retVal;

	qreal posX = event->x();
	qreal posY = event->y();

	if (m_scaleFactor != 1.0){
		posX = posX * m_scaleFactorO;
		posY = posY * m_scaleFactorO;
	}


	if (m_rectSectionH.contains(posX, posY) || m_rectSectionV.contains(posX, posY))
	{
		uoRptHeaderType rHt = uorRhtRowsHeader;
		uoRptSectionItem* sItem = NULL;
		uoRptSectionItem* sItemCur = NULL;
		uoRptSectionItemList* sList = m_sectItemListV;
		uoRptSectionItemList* sListOther = m_sectItemListH;
		if (m_rectSectionH.contains(posX, posY)) {
			rHt = uorRhtColumnHeader;
			sList = m_sectItemListH;
			sListOther = m_sectItemListV;
		}
		int i;
		for (i = 0; i<sListOther->size(); i++){
			sItemCur = sListOther->at(i);
			sItemCur->m_selected = false;
		}

		for (i = 0; i<sList->size(); i++){
			sItemCur = sList->at(i);
			sItemCur->m_selected = false;

			if (sItemCur){
				if (sItemCur->m_rectView.contains(posX, posY))
					sItem = sItemCur;
			}
		}
		if (sItem){
			m_selections->clearSelections();
			clearSelectionsSection();
			sItem->m_selected = true;
			if (rHt == uorRhtRowsHeader){
				m_selections->rowSelectedStart(sItem->m_start);
				m_selections->rowSelectedEnd(sItem->m_end);
			} else {
				m_selections->colSelectedStart(sItem->m_start);
				m_selections->colSelectedEnd(sItem->m_end);
			}
		}
		updateThis();
		if (sItem && isDoubleClick){
			// ну, тут нужно имя ввести...
			QString txt = sItem->m_nameSections;
			if (m_iteractView->inputSectionName(txt, this)){
				uoSpanTree* secMenager = doc->getSectionManager(rHt);
				if (secMenager){
					if (secMenager->isNameUnique(sItem->m_id, txt))	{
						secMenager->setSectionName(sItem->m_id, txt);
						recalcHeadersRects();
					}
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

	if (rht == uorRhtRowsHeader){
		cntScale = m_areaMain.m_firstVisible_RowTop;
		endPos = posY;
		stratPos = m_rectDataRegion.top() - m_areaMain.m_shift_RowTop;
		m_curMouseSparesRect.setLeft(m_rectRulerV.left());
		m_curMouseSparesRect.setRight(m_rectRulerV.right());
	} else if (rht == uorRhtColumnHeader) {
		cntScale = m_areaMain.m_firstVisible_ColLeft;
		endPos = posX;
		stratPos = m_rectDataRegion.left() - m_areaMain.m_shift_ColLeft;
		m_curMouseSparesRect.setTop(m_rectRulerH.top());
		m_curMouseSparesRect.setBottom(m_rectRulerH.bottom());
	}

	scaleNo = cntScale;
	stratPos = stratPos; // * m_scaleFactorO;
	while (stratPos < endPos){
		scSize = 0.0;
		if (!doc->getScaleHide(rht, cntScale)) {
			scSize = doc->getScaleSize(rht, cntScale); // * m_scaleFactorO;
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

	if (rht == uorRhtRowsHeader){
		m_curMouseSparesRect.setTop(stratPos);
		m_curMouseSparesRect.setBottom(endPos);
		mouseSparesAcceleratorSave(uoVst_ScaleV, scaleNo, rht);
	} else {
		m_curMouseSparesRect.setLeft(stratPos);
		m_curMouseSparesRect.setRight(endPos);
		mouseSparesAcceleratorSave(uoVst_ScaleH, scaleNo, rht);
	}
	return	retVal;
}

/// Определить запчать на которой находится точка QPoint для последующих уточняющих определений.
uorSparesType uoReportCtrl::findPointLocation(qreal posX, qreal posY)
{
	uorSparesType rst = uoVst_Unknown;
	if (m_showGroup){
		if (m_rectGroupV.contains(posX, posY)){
			rst = uoVst_GroupV;
		} else if (m_rectGroupH.contains(posX, posY)) {
			rst = uoVst_GroupH;
		}
	}
	if (rst == uoVst_Unknown && m_showRuler) {
		if (m_rectRuleCorner.contains(posX, posY)){
			rst = uoVst_ScaleVH;
		} else if (m_rectRulerH.contains(posX, posY)) {
			rst = uoVst_ScaleH;
		} else if (m_rectRulerV.contains(posX, posY)) {
			rst = uoVst_ScaleV;
		}
	}

	if (rst == uoVst_Unknown && m_showSection) {
		if (m_rectSectionH.contains(posX, posY)){
			rst = uoVst_SectionH;
		} else if (m_rectSectionV.contains(posX, posY)){
			rst = uoVst_SectionV;
		}
	}

	if (rst == uoVst_Unknown) {
		if (m_rectDataRegion.contains(posX, posY)){
			rst = uoVst_Cell;
		}
	}
	return rst;
}


/// сбросить значения акселератора поиска по пространственным координатам.
void uoReportCtrl::mouseSparesAcceleratorDrop()
{
	m_curMouseSparesType = uoVst_Unknown;
	m_curMouseSparesNo	= -1;
	m_curMouseSparesRht  = uorRhtUnknown;
	zeroQRectF(m_curMouseSparesRect);
}

/// Запомнить значения акселератора поиска по пространственным координатам.
void uoReportCtrl::mouseSparesAcceleratorSave(uorSparesType spar, int nom, uoRptHeaderType rht)
{
	m_curMouseSparesType = spar;
	m_curMouseSparesNo	= nom;
	m_curMouseSparesRht  = rht;
	// А рект вроде остается....
}

/// Оценка точки pos в rect, определение того, что точка находится в зоне изменения размеров.
uorBorderLocType uoReportCtrl::scaleLocationInBorder(qreal pos, QRectF rect, uoRptHeaderType rht)
{
	uorBorderLocType locType = uoBlt_Unknown;
	qreal dragSize = UORPT_DRAG_AREA_SIZE;
	// надо еще величину границы захвата определить.
	qreal posStart, posEnd, interVal = dragSize * 2;

	if (rht == uorRhtRowsHeader || rht == uorRhtUnknown) {

		posStart 	= rect.top() - dragSize;
		posEnd 		= posStart + interVal;
		if (posStart<=pos && pos<= posEnd)
			return uoBlt_Top;

		posStart 	= rect.bottom() - dragSize;
		posEnd 		= posStart + interVal;
		if (posStart<=pos && pos<= posEnd)
			return uoBlt_Bottom;

	}
	if (rht == uorRhtColumnHeader || rht == uorRhtUnknown) {

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

/// Клик мышки-норушки по области с данными.
bool uoReportCtrl::mousePressEventForDataArea(QMouseEvent *event, bool isDoubleClick)
{
	bool retVal = false;


	qreal posX = event->x(), posY = event->y();
	if (m_scaleFactor != 1.0){
		posX = posX * m_scaleFactorO;
		posY = posY * m_scaleFactorO;
	}
	if (!m_rectDataRegion.contains(posX, posY))
		return retVal;

//	uorSparesType rst = findPointLocation(posX, posY);

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return retVal;

	QPoint pntCell = getCellFromPosition(posY, posX);
	if (pntCell.isNull())
		return retVal;
	bool isSell = m_selections->isCellSelect(pntCell.y(), pntCell.x());

	if ((event->button() == Qt::LeftButton) && m_selections->isShiftPress())
	{
		if (m_selections->cellStartIsNull())
			m_selections->cellSelectedStart(m_curentCell.y(), m_curentCell.x());
		m_selections->cellSelectedMidle(pntCell.x(), pntCell.y());
		updateThis();

	} else if (event->button() == Qt::LeftButton || !isSell){

//		if (m_selections->sta)
		if (!isSell){
			m_selections->clearSelections(uoRst_Cells);
			clearSelectionsSection();
		}

		setCurentCell(pntCell.x(),pntCell.y(), true);
		updateThis();
		retVal = true;

		if (isDoubleClick) {
			if (doCellEditTextStart(""))
				setStateMode(rmsEditCell);
		} else {
			m_curMouseLastPos = event->pos();
			m_selections->cellSelectedStart(pntCell.x(),pntCell.y());
			setStateMode(rmsSelectionCell);
		}
	}

	// Надо определить
	return retVal;
}



/// отработаем реакцию на нажатие мышки на линейке.
bool uoReportCtrl::mousePressEventForRuler(QMouseEvent *event, bool isDoubleClick)
{
	bool retVal = false;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return retVal;

	if (event->button() != Qt::LeftButton)
		return retVal;
	qreal posX = event->x(), posY = event->y();
	m_curMouseLastPos.setX(event->x());
	m_curMouseLastPos.setY(event->y());


	if (m_scaleFactor != 1.0){
		posX = posX * m_scaleFactorO;
		posY = posY * m_scaleFactorO;
	}
	if (m_rectRuleCorner.contains(posX, posY)){
		m_selections->selectDocument();
		updateImage();
		return true;
	}
	if (!(m_rectRulerH.contains(posX, posY) || m_rectRulerV.contains(posX, posY))) {
		return retVal;
	}
	uoRptHeaderType rhtCur = uorRhtRowsHeader;
	if (m_rectRulerH.contains(posX, posY))
		rhtCur = uorRhtColumnHeader;

	int scaleNo = 0;
	bool scHide = true;
	qreal scSize = 0.0;

	if (findScaleLocation(posX, posY, scaleNo, rhtCur)){

		/*
			Я должен убедиться, что позиция мыши не у края ячейки,
			что-бы начать изменения размера или выделение ячейки.
		*/
		/*
			Интересная ситуевинка....
			Если мы хватаем за нижний край,
			и пытаемся изменить зазмер, тут все просто:
			- если это первая ячейка то просто выходим из процедуры.
			- если номер ячейки > 0, то размер можно менять.
			 А если это верхняя область?
			 Значит надо найти первую не скрытую верхнюю ячейку с номером > 0
			 и уже с ней работать...
			 Вроде все ясно?
		*/

		uorBorderLocType locType = uoBlt_Unknown;
		if (rhtCur == uorRhtColumnHeader){
			locType = scaleLocationInBorder(posX, m_curMouseSparesRect, rhtCur);
			if (locType == uoBlt_Unknown) {
				m_selections->selectCol(scaleNo);
				m_selections->colSelectedStart(scaleNo);
				setStateMode(rmsSelectionRule_Top);
			} else
			if (locType == uoBlt_Left || locType == uoBlt_Right)
			{
				if (locType == uoBlt_Left && scaleNo == 1) {
					return retVal;
				}
				if (locType == uoBlt_Left){
					// ишем первую видимую ячейку. пс. не забыть бы потом пересчитать m_curMouseSparesRect
					while(scaleNo>1 && scHide) {
						--scaleNo;
						scHide = doc->getScaleHide(rhtCur, scaleNo);
						if (!scHide)
							break;
					}
					if (scaleNo<=0)
						return retVal;
					// А вот интересно, если ячейка не скрыта, но размер нулевой? вроде пофиг....
					qreal topRct = 0.0;

					scSize = doc->getScaleSize(rhtCur, scaleNo);
					topRct = m_curMouseSparesRect.left();
					m_curMouseSparesRect.setLeft(topRct - scSize);
					m_curMouseSparesRect.setRight(topRct);
				}
				setStateMode(rmsResizeRule_Top);
				m_resizeLine = scaleNo;
			}

		} else
		if (rhtCur == uorRhtRowsHeader)
		{
			locType = scaleLocationInBorder(posY, m_curMouseSparesRect, rhtCur);
			if (locType == uoBlt_Unknown) {
				m_selections->selectRow(scaleNo);
				m_selections->rowSelectedStart(scaleNo);
				setStateMode(rmsSelectionRule_Left);
			} else
			if (locType == uoBlt_Bottom || locType == uoBlt_Top)
			{
				if (locType == uoBlt_Top && scaleNo == 1) {
					return retVal;
				}
				if (locType == uoBlt_Top){
					// ишем первую видимую ячейку. пс. не забыть бы потом пересчитать m_curMouseSparesRect
					while(scaleNo>1 && scHide) {
						--scaleNo;
						scHide = doc->getScaleHide(rhtCur, scaleNo);
						if (!scHide)
							break;
					}
					if (scaleNo<=0)
						return retVal;
					// А вот интересно, если ячейка не скрыта, но размер нулевой? вроде пофиг....
					qreal topRct = 0.0;

					scSize = doc->getScaleSize(rhtCur, scaleNo);
					topRct = m_curMouseSparesRect.top();
					m_curMouseSparesRect.setTop(topRct - scSize);
					m_curMouseSparesRect.setBottom(topRct);
				}
				setStateMode(rmsResizeRule_Left);
				m_resizeLine = scaleNo;

			}
		}
		updateImage();
	}

	return retVal;
}

/// Посылка сигнала на обновление в контролируемом порядке...
void uoReportCtrl::updateThis(){
	if (m_freezUpdate==0){
		updateImage();
	}
}

/// видим ли проперти эдитор...
bool uoReportCtrl::propertyEditorVisible()
{
	if (!m_propEditor)
		return false;
	return m_propEditor->editorIsVisible();
}


/// проверить изменения в палитре свойств и применить к документу.
bool uoReportCtrl::propertyEditorApply()
{
	if (m_propEditor){
		if (m_propEditor->m_canselApply)
			return true;
		uoReportDoc* doc =  getDoc();
		if (doc){
			if(m_propEditor->m_sellectonType == uoRst_Unknown)		{
				/// а текст и расшифровочку?
				uorSelectionType  selType = m_selections->getSelectionType();
				if (selType == uoRst_Unknown){
					doc->setCellText(m_curentCell.y(), m_curentCell.x(), m_propEditor->m_cellText);
					//m_propEditor->m_cellDecode
				}
			} else if (m_propEditor->m_sellectonType == uoRst_Document)
			{
				// карамба :) прийдется менять свойства в документе.
				uorTextDecor* td = doc->getDefaultTextProp();
				if (td)
					td->copyFrom(m_propEditor->m_textPropCopy);
			}
		}

		uorSelVisitorSetProps processor;
		processor.m_needCreate = true;
		processor.m_textDec_Selection.copyFrom(m_propEditor->m_textPropCopy);
		processor.m_borderProp_Selection.copyFrom(m_propEditor->m_borderPropCopy);
		processSelection(&processor);
		updateImage();//update();
	}
	return true;
}

/// Общая процедура для обработки выделения документов.
void uoReportCtrl::processSelection(uorSelVisitorBase* processor)
{
	if (!processor)
		return;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
//	qDebug() << "processSelection:";

	int cntRow = doc->getRowCount();
	int cntCol = doc->getColCount();
	int startRow = 1;
	int startCol = 1;

	bool firstOne = true;


	uoCell* curCell = NULL;
	uorSelectionType  selType = m_selections->getSelectionType();
	if (selType == uoRst_Unknown){
		curCell = doc->getCell(m_curentCell.y(), m_curentCell.x(), processor->m_needCreate);
		if (curCell){
			if (!curCell->isPartOfUnion(m_curentCell.y()))
			{
				if (processor->m_needCreate)
					curCell->provideAllProps(doc,true);
				processor->visit(curCell, firstOne);
				if (curCell->m_textProp){
					processor->m_textDec_Selection.copyFrom(curCell->m_textProp);
				}
				if (curCell->m_borderProp){
					processor->m_borderProp_Selection.copyFrom(curCell->m_borderProp);
				}
			}
			if (processor->m_type == uorSVT_Setter){
				doc->doFormatDoc(m_curentCell.y(), m_curentCell.x());
			}
		}
	} else {
		///\todo а вот тут фикус, т.к. если документ пустой, то cntRow и cntCol будут равны нулю....
		QRect selBound = m_selections->getSelectionBound();
		if (selType != uoRst_Document){
			int tmp = 0;
			tmp = selBound.x();			if (tmp>0)	startCol = qMax(startCol, tmp);
			tmp = selBound.y();			if (tmp>0)	startRow = qMax(startRow, tmp);
			tmp = selBound.right();		if (tmp>0)	cntCol = qMax(cntCol, tmp);
			tmp = selBound.bottom();	if (tmp>0)	cntRow = qMax(cntRow, tmp);
			if (processor->m_type == uorSVT_Setter){
				doc->clearFormatArea();
				doc->m_needFormatArea = selBound;
			}
		}

		for (int row = startRow; row<=cntRow; row++){
			for (int col = startCol; col<=cntCol; col++){
				if (m_selections->isCellSelect(row, col)) {
					curCell = doc->getCell(row, col, processor->m_needCreate);
					if (!curCell)
						curCell = doc->getCellDefault();
					if (curCell){
						/* фича. если обрабатываем ячейки не имеющие своего
						curCell, а документ имеет стандартное форматирование по умолчанию,
						то принимаются в расчет только заполненные, а пустые
						с отличаюшимся форматированием игнорятся. Смедовательно имеем неправильный результт мержинга...	*/
						if (processor->m_needCreate && selType != uoRst_Document){
							curCell->provideAllProps(doc, true);
						}

						if (!curCell->isPartOfUnion(row))
						{
//							qDebug() << "processor->visit: col "<< col << " row "<< row;
							processor->visit(curCell, firstOne);
							if (firstOne)
								firstOne = !firstOne;
						}
					}
				}
			}
		}
		if (processor->m_type == uorSVT_Setter){
			doc->doFormatDoc();
		}
	}
}

/// При изменении выделения ИНОГДА неоходимо собрать сводные данные по свойствам выделения.
void uoReportCtrl::recalcSelectionProperty()
{
	m_textDec_Selection.resetItem();
	m_borderProp_Selection.resetItem();	///< закешированное значения свойств.
	uorSelVisitorGetProps processor;
	// в процессе сбора информации надо учесть дефолтные значения шрифта документа.
	uoReportDoc* doc = getDoc();
	if (doc){
		uorTextDecor* td = doc->getDefaultTextProp();
		if (td)
			m_textDec_Selection.assignItem(*td);
//			m_textDec_Selection.mergeItem(td);
	}
	processor.m_textDec_Selection.copyFrom(&m_textDec_Selection);
	processor.m_borderProp_Selection.copyFrom(&m_borderProp_Selection);
	processSelection(&processor);
	m_textDec_Selection.copyFrom(&processor.m_textDec_Selection);
	m_borderProp_Selection.copyFrom(&processor.m_borderProp_Selection);
}


/// По выделению и по остальным свойствам мы должны заполнить свойствами редактор
bool uoReportCtrl::populatePropEditor(uoReportPropEditor* propEditor)
{
	bool retVal = true;
	Q_ASSERT(propEditor);
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return false;

	uoCell* cell = doc->getCell(m_curentCell.y(),m_curentCell.x(),false);
	if (cell){
		propEditor->m_cellText = cell->m_text;
		propEditor->m_cellDecode = cell->m_textDecode;
	} else {
		propEditor->m_cellText = "";
		propEditor->m_cellDecode = "";
	}


	recalcSelectionProperty();
	propEditor->m_textProp->copyFrom(&m_textDec_Selection);
	propEditor->m_borderProp->copyFrom(&m_borderProp_Selection);
	propEditor->m_sellectonType = m_selections->getSelectionType();
	return retVal;
}

/// Реакция на нажатие мышки-норушки...
void uoReportCtrl::mousePressEvent(QMouseEvent *event)
{
	bool visProp = propertyEditorVisible();
	if (visProp)
		propertyEditorApply();
	bool proc = true;

	if (modeTextEditing())
		onCellEditTextEnd(true);

	if (proc && m_showGroup && mousePressEventForGroup(event)) {
		proc = false;
	}
	if (proc && m_showRuler && mousePressEventForRuler(event)){
		proc = false;
	}
	if (proc && m_showSection && mousePressEventForSection(event)){
		proc = false;
	}

	if (proc && mousePressEventForDataArea(event)) {
		proc = false;
	}
	if (visProp) {
		propertyEditorShow();
	}
}

void uoReportCtrl::mouseDoubleClickEvent( QMouseEvent * event )
{
	//Note that the widgets gets a mousePressEvent() and a mouseReleaseEvent() before the mouseDoubleClickEvent().
	if (modeTextEditing())
		onCellEditTextEnd(true);

	if (m_showGroup && mousePressEventForGroup(event, true)) {
		return;
	}
	if (m_showRuler && mousePressEventForRuler(event, true)){
		return;
	}
	if (m_showSection && mousePressEventForSection(event, true)){
		return;
	}

	if (mousePressEventForDataArea(event, true)) {
		return;
	}

}
void uoReportCtrl::mouseReleaseEvent(QMouseEvent *event)
{
	QRectF rct;
	int line = 0;
	bool needUpdate = false;

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;

	bool visProp = propertyEditorVisible();

	qreal posX = 0,posY = 0;
	posX = event->x();
	posY = event->y();

	if (m_scaleFactor != 0.0) {
		posX = posX * m_scaleFactorO;
		posY = posY * m_scaleFactorO;
	}
	if (m_showRuler) {
		if (m_stateMode == rmsSelectionRule_Left || m_stateMode == rmsSelectionRule_Top){
			if (m_curMouseSparesRect.contains(event->pos())){
				/// мыша отрелейзилась там же где и была нажата...
				setStateMode(rmsNone);
				return;
			}
			if (rmsSelectionRule_Left == m_stateMode) {
				if (findScaleLocation(posX, posY, line,uorRhtRowsHeader)) {
					m_selections->rowSelectedEnd(line);
					needUpdate = true;
				}
			}
			if (rmsSelectionRule_Top == m_stateMode) {
				if (findScaleLocation(posX, posY, line,uorRhtColumnHeader)) {
					m_selections->colSelectedEnd(line);
					needUpdate = true;
				}
			}
		} else if (m_stateMode == rmsResizeRule_Left || m_stateMode == rmsResizeRule_Top) {
			needUpdate = true;
			int distance = (m_curMouseLastPos - event->pos()).manhattanLength();
			if (distance > QApplication::startDragDistance()) {

				qreal delta = 0;
				uoRptHeaderType rht = (m_stateMode == rmsResizeRule_Left) ? uorRhtRowsHeader : uorRhtColumnHeader;
				if (m_stateMode == rmsResizeRule_Left) {
					delta = posY - m_curMouseSparesRect.bottom();
				} else {
					delta = posX - m_curMouseSparesRect.right();
				}
				qreal newSize = doc->getScaleSize(rht, m_resizeLine);
				newSize = newSize + delta;
				newSize = qMax(0.0, newSize);
				doc->setScaleSize(rht, m_resizeLine,newSize);
				doc->setScaleFixedProp(rht, m_resizeLine, true);
				if (m_stateMode == rmsResizeRule_Top){
					doc->doFormatDoc(0,m_resizeLine);
					emit onColumnSizeChange(m_resizeLine, newSize);
				}
				recalcHeadersRects();
			}
		}
	}
	if (m_stateMode == rmsSelectionCell){
		// Надо сделать выбор единичных ячеек.
		int distance = (m_curMouseLastPos - event->pos()).manhattanLength();
		bool isCtrlPress =  m_selections->isCtrlPress();

		if (distance > QApplication::startDragDistance() || isCtrlPress) {
			// Значит процесс пошел...
			QPoint pntAdd; //= event->pos();
			pntAdd.setX((int)posX);
			pntAdd.setY((int)posY);

			pntAdd.setY(qMax(pntAdd.y(), int(m_rectDataRegion.top()-m_areaMain.m_shift_RowTop)));
			pntAdd.setY(qMin(pntAdd.y(), int(m_rectDataRegion.bottom())));

			pntAdd.setX(qMax(pntAdd.x(), int(m_rectDataRegion.left()-m_areaMain.m_shift_ColLeft)));
			pntAdd.setX(qMin(pntAdd.x(), int(m_rectDataRegion.right())));

			QPoint pntCell = getCellFromPosition(pntAdd.y(), pntAdd.x());
			if (!pntCell.isNull()){
				m_selections->cellSelectedEnd(pntCell.x(), pntCell.y());
				needUpdate = true;
				setCurentCell(pntCell.x(), pntCell.y());
			}
		}
	}

	if (m_stateMode != rmsEditCell){
		/*
			Тут особый случай, при rmsEditCell используется режим,
			который нужно отрабатывать отдельно.
			А одним из входов в rmsEditCell является мышко-дабл-клик,
			за которым естественно следует mouseReleaseEvent и
			скидывает этот режим, что не есть гуд...
		*/
		setStateMode(rmsNone);
	}

	if (needUpdate)
		updateThis();
	if (visProp)
		propertyEditorShow();

}
void uoReportCtrl::mouseMoveEvent(QMouseEvent *event)
{
	QCursor cur = cursor();
	bool needAnalysisPos = false, findArea = false;
	m_curMouseCurPos = event->pos();

	if (m_scaleFactor != 0.0) {
		// Надо учеть фактор масштаба..
		m_curMouseCurPos.setX((int)(m_scaleFactorO * event->pos().x()));
		m_curMouseCurPos.setY((int)(m_scaleFactorO * event->pos().y()));
	}

	Qt::CursorShape need_shape = Qt::ArrowCursor, cur_shape = cur.shape();
	switch(m_stateMode) {
		case rmsNone:{
			need_shape = Qt::ArrowCursor;
			needAnalysisPos = true;
			break;
		}
		case rmsSelectionRule_Top:
		case rmsSelectionRule_Left:	{
			need_shape = Qt::PointingHandCursor;
			break;
		}

		case rmsResizeRule_Top:	{
			need_shape = Qt::SizeHorCursor;
			break;
		}
		case rmsResizeRule_Left:		{
			need_shape = Qt::SizeVerCursor;
			break;
		}
		default:	{
			need_shape = Qt::ArrowCursor;
			needAnalysisPos = true;
			break;
		}
	}
	if (needAnalysisPos) {
		// необходим анализ позиции курсора....
		QPointF pos;
		bool vertLoc = false;
		uoRptHeaderType rhdType = uorRhtUnknown;
		qreal posX = event->x(), posY = event->y();

		if (m_scaleFactor != 0.0) {
			posX = posX * m_scaleFactorO;
			posY = posY * m_scaleFactorO;
		}
		pos.setX(posX);
		pos.setY(posY);

		int lineNo = 0;

		if (m_showRuler) {
			if (m_rectRuleCorner.contains(pos)) {
				need_shape = Qt::PointingHandCursor;
				findArea = true;
			}
			if (!findArea){
				if (m_rectRulerH.contains(pos) || (vertLoc = m_rectRulerV.contains(pos))){

					rhdType = vertLoc ? uorRhtRowsHeader : uorRhtColumnHeader;
					uorBorderLocType locType = uoBlt_Unknown;

					need_shape = Qt::PointingHandCursor;
					findArea = true; //  типа нашли позицию и не хрю...
					// далее проанализируем точную позицию.
					findArea = findScaleLocation(posX, posY, lineNo, rhdType);
					if (!findArea)
						return;
					if (rhdType == uorRhtRowsHeader) {
						locType = scaleLocationInBorder(posY, m_curMouseSparesRect, rhdType);
						if (locType == uoBlt_Bottom || locType == uoBlt_Top){
							if (locType == uoBlt_Top && lineNo == 1) {
								// какой смысл двигать верх 1-й строки или левый край 1-го столбца? О_о
							} else {
								need_shape = Qt::SizeVerCursor;
							}
						}
					} else {
						locType = scaleLocationInBorder(posX, m_curMouseSparesRect, rhdType);
						if (locType == uoBlt_Left || locType == uoBlt_Right) {
							if (locType == uoBlt_Left && lineNo == 1) {
							} else {
								need_shape = Qt::SizeHorCursor;
							}
						}
					}
				}
			}
		}
	}

	if (need_shape != cur_shape)
		setCursor(need_shape);
	if (m_stateMode == rmsResizeRule_Top || m_stateMode == rmsResizeRule_Left) {
		// Тут не нужно перерисовывать все.
		emit update(); //	updateThis();
	}


}

void uoReportCtrl::showEvent( QShowEvent* event){
	recalcHeadersRects();
}

/// Обработка клавиатурных клавишь перемещения курсора....
void uoReportCtrl::keyPressEventMoveCursor ( QKeyEvent * event )
{
	int key = event->key();
	event->accept();
	int posX = m_curentCell.x(),  posY = m_curentCell.y();

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;

	Qt::KeyboardModifiers  kbrdMod = qApp->keyboardModifiers();
	bool ctrlPresed = (kbrdMod & Qt::ControlModifier) ? true : false;
	bool shiftPresed = (kbrdMod & Qt::ShiftModifier) ? true : false;

	if (!shiftPresed){
		m_selections->clearSelections(); // под конкретным вопросом.
		clearSelectionsSection();
	}

	bool itemHiden = false;

	//	научим курсор прыгать через скрытые ячейки.... но ни через границу поля...
	switch (key)	{
		case Qt::Key_Down:	{
			while(itemHiden = doc->getScaleHide(uorRhtRowsHeader, ++posY)){}
			break;
		}
		case Qt::Key_Up: {
			do {
				posY = posY - 1;
				if (posY == 0) {
					posY = m_curentCell.y();
					break;
				}
				itemHiden = doc->getScaleHide(uorRhtRowsHeader, posY);

			} while(itemHiden);

			if (!(posY >= 1))
				posY = m_curentCell.y();
			break;
		}
		case Qt::Key_Right: {
			while(itemHiden = doc->getScaleHide(uorRhtColumnHeader, ++posX)){}
			break;
		}
		case Qt::Key_Left:	{
			do {
				posX = posX - 1;
				if (posX == 0) {
					posX = m_curentCell.x();
					break;
				}
				itemHiden = doc->getScaleHide(uorRhtColumnHeader, posX);

			} while(itemHiden);

			if (!(posX >= 1))
				posX = m_curentCell.x();
			break;
		}
		case Qt::Key_Home:{
			// В начало строки....
			if (!ctrlPresed) {
				posX = 1;
			} else {
				posY = 1;
			}
			break;
		}
		case Qt::Key_End:{
			if (!ctrlPresed) {
				posX = m_colCountDoc + 1;
			} else {
				posY = m_rowCountDoc + 1;
			}
			break;
		}
		case Qt::Key_PageUp:
		case Qt::Key_PageDown:{
			/*
				необходимо промотать страницу вниз или вверх...
				начнем с простейших случаев:
			*/
			if(key == Qt::Key_PageUp && m_areaMain.m_firstVisible_RowTop == 1) {
				posY = 1;
			} else if (key == Qt::Key_PageDown /*&& m_areaMain.m_firstVisible_RowTop == 1*/ && posY < m_areaMain.m_lastVisibleRow) {
				posY = m_areaMain.m_lastVisibleRow;
			} else {
				/*
					тут все смешнее. попробуем...
				*/
				if (!curentCellVisible())
					return;
				bool rowHiden = false;
				int modif = 1, curRow = m_curentCell.y();
				if (key == Qt::Key_PageUp) {
					modif = -1;
				}

				qreal pageHeight = m_rowsInPage * doc->getDefScaleSize(uorRhtRowsHeader);
				do {
					curRow = curRow + modif;
					if (curRow == 0) {
						curRow = 1;
						break;
					}
					rowHiden = doc->getScaleHide(uorRhtRowsHeader, curRow);
					if (rowHiden)
						continue;
					pageHeight = pageHeight - doc->getScaleSize(uorRhtRowsHeader, curRow);
					if (pageHeight < 0.0)
						break;
				} while(true);
				if (curRow <= 0)
					curRow = 1;
				posY = curRow;

			}
//			int rowCounter = m_rowsInPage;
			break;
		}
		default: {
			break;
		}
	}

	if (posX != m_curentCell.x() || posY != m_curentCell.y()){
		/*
			а вот тут можно начать/продолжить выделение, если у нас зажат шифт...
			только пока непонятно как его закончить?
		*/
		if (shiftPresed) {
			if (m_selections->getStartSelectionType() != uoRst_Cells){
				m_selections->clearSelections(uoRst_Cells);
				clearSelectionsSection();
				m_selections->cellSelectedStart(m_curentCell.x(), m_curentCell.y());
			}
		}
		m_selections->cellSelectedMidle(posX, posY);
		setCurentCell(posX, posY, true);
		updateThis();
	}
}

/// Поскролимся чуток...
void uoReportCtrl::wheelEvent ( QWheelEvent * event )
{
	if (modeTextEditing())		return;
	int numDegrees = event->delta() / 8;
	int numSteps = numDegrees / 5;

	scrollView(0, -numSteps);
	event->accept();
	recalcScrollBars();

}

void uoReportCtrl::focusInEvent ( QFocusEvent * event )
{
	bool visProp = propertyEditorVisible();
	if (visProp)
		propertyEditorApply();
}
void uoReportCtrl::focusOutEvent ( QFocusEvent * event )
{
}



/// Обработка реакции клавиатуры..
void uoReportCtrl::keyPressEvent( QKeyEvent * event ){
	if (modeTextEditing())
		return;
	bool visProp = propertyEditorVisible();
	if (visProp){
		propertyEditorApply();
	}

	int key = event->key();
	QString str;

	--m_freezUpdate;
	event->accept();
	switch (key)
	{
		case Qt::Key_Down:
		case Qt::Key_Up:
		case Qt::Key_Left:
		case Qt::Key_Right:
		case Qt::Key_Home:
		case Qt::Key_End:
		case Qt::Key_PageUp:
		case Qt::Key_PageDown:
		{
			keyPressEventMoveCursor ( event );
			break;
		}
		case Qt::Key_Return:
		case Qt::Key_Enter:
		case Qt::Key_F2:
		{
			if (doCellEditTextStart(str))
				setStateMode(rmsEditCell);
			break;
		}
		case Qt::Key_Escape:
		{
			if (m_stateMode == rmsResizeRule_Top || m_stateMode == rmsResizeRule_Left)
			{
				// Сбросим действие, вдруг не нужно..
				m_stateMode = rmsNone;
			}
			break;
		}
		default:
		{
			/// остальные пусть папочка отрабатывает... QPrintTable
			event->ignore();
			break;
		}
	}
	if (!event->isAccepted()){
		str = event->text();
		if (str.length() == 1) {
			QChar hr = str[0];
			if (hr.isDigit() || hr.isLetter()){
				event->accept();
				if (doCellEditTextStart(str))
					setStateMode(rmsEditCell);

			}
			// типа буква или цЫфра?
		}
	}

	++m_freezUpdate;
	updateThis();
	if (visProp){
		if (modeTextEditing()){
			propertyEditorHide();
		} else {
			propertyEditorShow();
		}
	}
}

void uoReportCtrl::resizeEvent( QResizeEvent * event ){

	QWidget::resizeEvent(event);
	m_imageView = QImage(size(), QImage::Format_ARGB32_Premultiplied);
	m_drawToImage = 1;
	const QSize oldSizeW =  event->oldSize();
	const QSize sizeW =  event->size();
	m_sizeVvirt = m_sizeVvirt - oldSizeW.height() + sizeW.height();
	m_sizeHvirt = m_sizeHvirt - oldSizeW.width() + sizeW.width();

	recalcHeadersRects();
	recalcScrollBars();
}

/**
	Заполнить меню для документа..
*/
bool uoReportCtrl::populateMenu(QMenu* targMenu)
{
	uoReportDoc* doc = getDoc();
	if (!doc)
		return false;

	targMenu->addAction(m_iteractView->m_actUndo); m_iteractView->m_actUndo->setEnabled(doc->m_undoManager->undoAvailability());
	targMenu->addAction(m_iteractView->m_actRedo); m_iteractView->m_actRedo->setEnabled(doc->m_undoManager->redoAvailability());


	QMenu *menuShow 	= targMenu->addMenu(QString::fromUtf8("Скрыть / показать"));

	bool actEnable = false;

	if (m_showGrid)	menuShow->addAction(m_iteractView->m_actGridHide);
	else 			menuShow->addAction(m_iteractView->m_actGridShow);

	if (m_showGroup) menuShow->addAction(m_iteractView->m_actGroupHide);
	else 			menuShow->addAction(m_iteractView->m_actGroupShow);

	if (m_showSection) 	menuShow->addAction(m_iteractView->m_actSectionHide);
	else 				menuShow->addAction(m_iteractView->m_actSectionShow);

	if (m_showRuler) 	menuShow->addAction(m_iteractView->m_actRulerHide);
	else 				menuShow->addAction(m_iteractView->m_actRulerShow);

	if (m_drawHelper->m_showInvisiblChar) 	menuShow->addAction(m_iteractView->m_actInvCharHide);
	else 					menuShow->addAction(m_iteractView->m_actInvCharShow);

	QMenu *menuScope = targMenu->addMenu(QString::fromUtf8("Масштаб"));
	m_iteractView->setCheckedState(m_scaleFactor);

	menuScope->addAction(m_iteractView->m_actScope25);
	menuScope->addAction(m_iteractView->m_actScope50);
	menuScope->addAction(m_iteractView->m_actScope75);
	menuScope->addAction(m_iteractView->m_actScope100);
	menuScope->addAction(m_iteractView->m_actScope125);
	menuScope->addAction(m_iteractView->m_actScope150);
	menuScope->addAction(m_iteractView->m_actScope200);
	menuScope->addAction(m_iteractView->m_actScope250);
	menuScope->addAction(m_iteractView->m_actScope300);

	actEnable = m_selections->isTrueForSections();

	QMenu *menuSections = targMenu->addMenu(QString::fromUtf8("Секции"));
	menuSections->addAction(m_iteractView->m_actSectionIn);
	menuSections->addAction(m_iteractView->m_actSectionOut);
	m_iteractView->m_actSectionIn->setEnabled(actEnable);
	m_iteractView->m_actSectionOut->setEnabled(actEnable);

	QMenu *menuGroup = targMenu->addMenu(QString::fromUtf8("Группы"));
	menuGroup->addAction(m_iteractView->m_actGroupIn);
	menuGroup->addAction(m_iteractView->m_actGroupOut);
	m_iteractView->m_actGroupIn->setEnabled(actEnable);
	m_iteractView->m_actGroupOut->setEnabled(actEnable);

	QMenu *rowColGroup = targMenu->addMenu(QString::fromUtf8("Строки/Столбцы"));
	rowColGroup->addAction(m_iteractView->m_actRowCol_Delete);
	rowColGroup->addAction(m_iteractView->m_actRowCol_Add);
	rowColGroup->addAction(m_iteractView->m_actRow_AutoSize);
	rowColGroup->addAction(m_iteractView->m_actRowCol_SetSize);

	targMenu->addSeparator();
	QMenu *menuSpecial 	= targMenu->addMenu(QString::fromUtf8("Специальное"));
	menuSpecial->addAction(m_iteractView->m_actClear);
	menuSpecial->addAction(m_iteractView->m_actOutToDebug);

	targMenu->addSeparator();
	targMenu->addAction(m_iteractView->m_actSave);
	targMenu->addAction(m_iteractView->m_actSaveAs);
	targMenu->addAction(m_iteractView->m_actLoad);
	targMenu->addSeparator();
	targMenu->addAction(m_iteractView->m_showPreview);
	targMenu->addAction(m_iteractView->m_showPageSettings);
	targMenu->addSeparator();

	targMenu->addAction(m_iteractView->m_showProp);
	return true;
}


void uoReportCtrl::contextMenuEvent(QContextMenuEvent *event){

	QMenu *contextMenu 	= new QMenu(this);
	populateMenu(contextMenu);
	contextMenu->exec(event->globalPos());
	delete contextMenu;
}


/// Сигнал на включение/исключение в секцию/в группу
void uoReportCtrl::onSectionInclude(){	onSpanInclude(uoSpanType_Sections);}
void uoReportCtrl::onSectionExclude(){	onSpanExclude(uoSpanType_Sections);}
void uoReportCtrl::onGroupInclude(){	onSpanInclude(uoSpanType_Group);}
void uoReportCtrl::onGroupExclude(){	onSpanExclude(uoSpanType_Group);}

void uoReportCtrl::onSpanInclude(uoRptSpanType spType)
{
	uoReportDoc* doc = getDoc();
	if (!doc)
		return;

	uoRptHeaderType rht;
	int start, end;

	bool retVal = m_selections->getTrueSectionsCR(rht, start, end);
	if (retVal){
		uoSpanTree* spanManager = NULL;
		retVal = false;

		if (spType == uoSpanType_Group) {
			spanManager = doc->getGroupManager(rht);
			retVal = doc->addGroup(start,end, rht,false);

		} else if (spType == uoSpanType_Sections){

			spanManager = doc->getSectionManager(rht);
			int secCount = spanManager->getSize();
			QString inputName = QString("Section_%1").arg(++secCount);
			while(!spanManager->isNameUnique(-1, inputName)){
				inputName = QString("Section_%1").arg(++secCount);
			}
			if (m_iteractView->inputSectionName(inputName, this)){
				if (spanManager->isNameUnique(-1, inputName)) {
					retVal = doc->addSection(start,end, rht,inputName);
				} else {
					qWarning() << "not unique section name " << inputName;
				}
			}
		} else {
			return;
		}
		if (retVal)
			recalcHeadersRects();

	}
}

void uoReportCtrl::onSpanExclude(uoRptSpanType spType)
{
	uoReportDoc* doc = getDoc();
	if (!doc)
		return;

	uoRptHeaderType rht;
	int start, end;

	bool retVal = m_selections->getTrueSectionsCR(rht, start, end);
	if (retVal){
		uoSpanTree* spanManager = NULL;
		retVal = false;

		if (spType == uoSpanType_Group) {
			spanManager = doc->getGroupManager(rht);

		} else if (spType == uoSpanType_Sections){
			spanManager = doc->getSectionManager(rht);

		}

		dropGropItemToCache();
		spanManager->onLineExclude(start, end-start+1);
		recalcHeadersRects();
	}

}

void uoReportCtrl::onUndo(){
	uoReportDoc* doc = getDoc();
	if (!doc)		return;
	doc->onUndo();
}
void uoReportCtrl::onRedo()
{
	uoReportDoc* doc = getDoc();
	if (!doc)		return;
	doc->onRedo();
}

/// Сигнал об изменении данных из вне, напирмер из механизма ундо/редо.
void uoReportCtrl::onDataChange()
{
	recalcHeadersRects();
	updateImage();
}

/// Обновить свойства, без форсированного показа редактора
void uoReportCtrl::propertyEditorShow(){
	if (!m_propEditor){
		m_propEditor = new uoReportPropEditor();
	}
	if (m_propEditor){
		// И че? Прийдется инициализировать?
		m_propEditor->showProperty(this);
	}
}

/// Обновить свойства, и активировать панель свойств.
void uoReportCtrl::propertyEditorShowActivate()
{
	if (!m_propEditor){
		m_propEditor = new uoReportPropEditor();
	}
	if (m_propEditor){
		// И че? Прийдется инициализировать?
		m_propEditor->showProperty(this, true);
	}

}

/// Сигнал на удаление строк/столбцов
void uoReportCtrl::onRowColDelete()
{
	onRowColGroupOperation(uorRCGroupOT_Delete);
}

/// Сигнал на добавление строк/столбцов
void uoReportCtrl::onRowColAdd()
{
	onRowColGroupOperation(uorRCGroupOT_Add);
}

/// Сигнал на установку для строк автоматического размера +сброс флага фиксированного размера.
void uoReportCtrl::onRowAutoSize() { onRowColGroupOperation2(uorRCGroupOT_SetAutoSize); }

/// Сигнал на установку размеров для строк/столбцов.
void uoReportCtrl::onRowColSetSize() { onRowColGroupOperation2(uorRCGroupOT_SetSize); }

/// Общая процедра для группы операций: uorRCGroupOT_SetSize и uorRCGroupOT_SetAutoSize
void uoReportCtrl::onRowColGroupOperation2(const uorRCGroupOperationType& opertn)
{
	QList<int> int_list;
	uoRptHeaderType rht = uorRhtRowsHeader;
	if (!m_selections->getSelectedColRow(rht, int_list))
		return;

	if (rht != uorRhtRowsHeader && opertn == uorRCGroupOT_SetAutoSize)
		return;

	if (int_list.isEmpty())
		return;
	int curPos = -1;

	uoReportDoc* doc = getDoc();
	if (!doc)
		return;

	QList<int>::const_iterator it = int_list.constBegin();
	qreal inputSize = 0.0;
	qreal rezInput = 0.0;
	if (opertn == uorRCGroupOT_SetSize) {
		bool ok;

		int cntr = 0;
		while(it != int_list.constEnd()){
			curPos = *it;
			rezInput = doc->getScaleSize(rht, curPos, rezInput);
			if(cntr == 0) {
				inputSize = rezInput;
			} else {
				if (inputSize != rezInput) {
					inputSize = rezInput = 0.0;
					break;
				}
			}
			++cntr;
			it++;
		}

		rezInput = (qreal) QInputDialog::getDouble(this, QString::fromUtf8("Введите размер")
			,QString::fromUtf8("Размер:"), inputSize, 0.0, 1200.0, 1, &ok);
		if (!ok)
			return; //integerLabel->setText(tr("%1%").arg(i));

	}


	it = int_list.constBegin();
	int cntChnges = 0;

	while(it != int_list.constEnd()){
		curPos = *it;
		switch(opertn)
		{
			case uorRCGroupOT_SetSize:
			{
				if (rht == uorRhtRowsHeader)
					doc->setScaleFixedProp(rht,curPos, true);
				doc->setScaleSize(rht, curPos, rezInput);
				++cntChnges;
				break;
			}
			case uorRCGroupOT_SetAutoSize:
			{
				doc->setScaleFixedProp(rht,curPos, false);
				++cntChnges;
				break;
			}
			default: { return;}
		}
		it++;
	}
	if (cntChnges>0) {
		doc->doFormatDoc();
		updateImage();
	}
}

/// Устроим документу предпросмотр
void uoReportCtrl::onShowPreview()
{
	uoReportPreviewDlg* pPreviewDlg = new uoReportPreviewDlg(this);
	pPreviewDlg->setDoc(getDoc());

	int dret = pPreviewDlg->exec();
	Q_UNUSED(dret);

	delete pPreviewDlg;

}

/// Вызываем диалог настройки страниц
void uoReportCtrl::onShowPagesSetings()
{
	uorPageSetup* pPageStDlg = new uorPageSetup(this);
	pPageStDlg->setSettings(getDoc()->pagesSetings());

//	pPageStDlg->m_pagesSetings = getDoc()->pagesSetings();
	int dret = pPageStDlg->exec();
	Q_UNUSED(dret);

	delete pPageStDlg;

}

/// Единая процедура для обработки некоторых групповых операций
void uoReportCtrl::onRowColGroupOperation(const uorRCGroupOperationType& opertn)
{
	uoRptHeaderType rht;
	int start,end, cnt = 0;

	if (!m_selections->getTrueSectionsCR(rht, start, end))
		return;

	cnt = end - start + 1;

	uoReportDoc* doc = getDoc();
	if (!doc)
		return;
	switch(opertn){
		case uorRCGroupOT_Delete:
		{
			switch(rht)
			{
				case uorRhtColumnHeader:	{	doc->doDeleteColumns(start, cnt);		break;	}
				case uorRhtRowsHeader:	{ doc->doDeleteRows(start, cnt);		break;	}
				default:		{			break;		}
			}
			break;
		}
		case uorRCGroupOT_Add:
		{
			switch(rht)
			{
				case uorRhtColumnHeader:	{	doc->doAddColumns(start, cnt);		break;	}
				case uorRhtRowsHeader:	{ 	doc->doAddRows(start, cnt);		break;	}
				default:		{			break;		}
			}
			break;
		}
		default:{
			break;
		}
	}
}




void uoReportCtrl::propertyEditorHide()
{
	if (m_propEditor){
		m_propEditor->hideProperty();
	}
}
void uoReportCtrl::onPropChange()
{

}




/// Сигнал изменения масштаба виджета
void uoReportCtrl::onSetScaleFactor(const qreal sFactor){
	if (sFactor != m_scaleFactor) {
		m_scaleFactor = sFactor;
	    m_scaleFactorO = 1 / m_scaleFactor;

		recalcHeadersRects();
		recalcScrollBars();
		updateImage();
	}
}

/// Видима ли строка
bool uoReportCtrl::rowVisible(int nmRow) const
{
	if (m_areaMain.m_firstVisible_RowTop<=nmRow && m_areaMain.m_lastVisibleRow >= nmRow) {
		return true;
	}
	return false;
}

/// Видим ли солбец
bool uoReportCtrl::colVisible(int nmCol) const
{
	if (m_areaMain.m_firstVisible_ColLeft <= nmCol && m_areaMain.m_lastVisibleCol >= nmCol){
		return true;
	}
	return false;
}


/// Ширина с учетом масштаба, и толщины скрола
qreal uoReportCtrl::getWidhtWidget() {
	return (width()-m_vScrollCtrl->width())* m_scaleFactorO;
}
/// Высота  с учетом масштаба, и толщины скрола
qreal uoReportCtrl::getHeightWidget(){
	return (height()-m_hScrollCtrl->height()) * m_scaleFactorO;
}

/// установка режима работы с отчетом.
void uoReportCtrl::setStateMode(uoReportStateMode stMode){
	m_resizeLine = 0;
	if (m_stateMode != stMode){
		const uoReportStateMode& oMode = m_stateMode;
		const uoReportStateMode& nMode = stMode;
		m_stateMode = stMode;
		emit onStateModeChange(oMode, nMode);
	}

}


///	Секции тоже выделяются, если они есть.	поэтому их надо и очищать...
void uoReportCtrl::clearSelectionsSection()
{
	uoRptSectionItem* sectItem = NULL;
	int cntr = 0;
	// список итемов секций столбцов
	if (m_sectItemListV){
		if (!m_sectItemListV->isEmpty()) {
			for (cntr = 0; cntr<m_sectItemListV->size(); cntr++) {
				sectItem = m_sectItemListV->at(cntr);
				if (sectItem)
					sectItem->m_selected = false;
			}
		}
	}
	// список итемов секций строк
	if (m_sectItemListH){
		if (!m_sectItemListH->isEmpty()) {
			for (cntr = 0; cntr<m_sectItemListH->size(); cntr++) {
				sectItem = m_sectItemListH->at(cntr);
				if (sectItem)
					sectItem->m_selected = false;
			}
		}
	}
}

/// типа текст пока редактируется...
bool uoReportCtrl::modeTextEditing() {
	return (rmsEditCell == m_stateMode) ? true : false;
}

/**
	Перекалькуляция размеров поля редактирования текста ячейки.
	Динамичски вызывается при редактировании текста ячейки
	По сигналу textChanged().
*/
void uoReportCtrl::recalcTextEditRect(QRect& rect)
{
	QString textCell;
	if (m_textEdit) {
		textCell = m_textEdit->toPlainText();
		if (textCell.isEmpty())
			return;
		int scrollWidht = m_textEdit->horizontalScrollBar()->width();
		scrollWidht = scrollWidht + scrollWidht / 2;

		uoReportDoc* doc = getDoc();
		if (doc) {
			uoCell* cell = doc->getCell(m_curentCell.y(), m_curentCell.x(), true);
			if (!cell)
				return;
			QFont* cellFont = cell->getFont(doc, true);
			if (cellFont)
			{
				QFontMetricsF fm(*cellFont);
				QStringList list = textCell.split('\n');
				qreal maxLength = 0.0, heightText = 0.0;
				for (int i = 0; i < list.size(); ++i){
					textCell = list.at(i);
					maxLength = qMax(maxLength, fm.width(textCell));
				}
				heightText = fm.height() * (list.size()+1);
				int col_Widht = (int)fm.height();
				int row_add = (int)heightText;
				int col_add = (int)maxLength + 20; // Прибавим от всяких колебаний...
				if ((rect.height()-20) < row_add)
				{
					rect.setHeight(row_add);
					if (!m_curentCellRect.isEmpty()){
						if (rect.height()<m_curentCellRect.height())
							rect.setHeight(m_curentCellRect.height());

					}
					if (rect.bottom()>=m_rectDataRegion.bottom())
					{
						rect.setBottom((int)m_rectDataRegion.bottom());
					}
				}
				if (((rect.width() - scrollWidht)+col_Widht) < col_add)
				{
					rect.setWidth(col_add+col_Widht);
					if (!m_curentCellRect.isEmpty()){
						if (rect.width()<m_curentCellRect.width())
							rect.setWidth(m_curentCellRect.width());

					}
					if (rect.right()>=m_rectDataRegion.right()){
						rect.setRight((int)m_rectDataRegion.right());
					}
				}
			}
		}
	}
}



/// Начинаем редактирование текста ячейки...
bool uoReportCtrl::doCellEditTextStart(const QString& str)
{
	if (rmsEditCell == m_stateMode){	/* О_о */}

	if (curentCellVisible()){
		if (!m_textEdit){
			m_textEdit = new QTextEdit(this);
			m_textEdit->hide(); // а помоему оно создается невидимым.
			m_textEdit->setAcceptRichText(false);
			m_textEdit->installEventFilter(m_messageFilter);
			m_textEdit->setTabStopWidth((int)(m_charWidthPlus*4));
			{
				m_textEdit->setFrameShape(QFrame::Box); // не очень красиво...
				//m_textEdit->setFrameShape(QFrame::NoFrame);
			}
		}
		QRect rct = getCellRect(m_curentCell.y(), m_curentCell.x());
		m_curentCellRect = rct;
		if (!rct.isEmpty()){
			rct.adjust(1,1,0,0);
			QString text;
			uoReportDoc* doc =  getDoc();
			if (!doc)
				return false;

			if (!str.isEmpty()) {
				text = str;
			} else {
				text = doc->getCellText(m_curentCell.y(), m_curentCell.x());
			}
			uoCell* cell = doc->getCell(m_curentCell.y(), m_curentCell.x(),true);
			if (cell)
			{
				QFont* font = cell->getFont(doc,true);
				if (font)
				{
					m_textEdit->setFontFamily(font->family());
					int fontSz = cell->getFontSize();
					if (fontSz>0)
						m_textEdit->setFontPointSize(fontSz);
				}
			}

			m_textEdit->setPlainText(text);
			if (!text.isEmpty()){
				recalcTextEditRect(rct);
			}
			m_textEdit->setGeometry(rct);
			m_textEdit->show();
			m_textEdit->activateWindow();
			m_textEdit->setFocus();
			if (!str.isEmpty()){
				// т.е. если вход в режим редактирования был иниццирован алфавитно-цыфирьнйо клавишей, курсорчик надо сдвинуть...
				m_textEdit->moveCursor(QTextCursor::End);
			}
			connect(m_textEdit, SIGNAL(textChanged()),this,SLOT(cellTextChangedFromEdit()));
			m_vScrollCtrl->setDisabled(true);
			m_hScrollCtrl->setDisabled(true);
			setStateMode(rmsEditCell);
			return true;
		}
		// так, вот тут надо получить координаты ячейки.
	}
	return false;
}

/**
	Слот, принимающий сигнал об окончании редактирования текста ячейки
	с приказом принять или отклонить результат редактирования..
*/
void uoReportCtrl::onCellEditTextEnd(bool accept)
{

	if (m_textEdit)	{
		m_textEdit->disconnect(SIGNAL(textChanged()));
	} else {
		return;
	}

	if (accept){
		QString text = m_textEdit->toPlainText();
		uoReportDoc* doc =  getDoc();
		if (!doc)
			return;
		QString text2 = doc->getCellText(m_curentCell.y(), m_curentCell.x());
		if (text2.compare(text) != 0){
			doc->setCellText(m_curentCell.y(), m_curentCell.x(), text);
		}
	}
	setFocus();
	activateWindow();

	m_textEdit->hide(); // а помоему оно создается невидимым.
	m_vScrollCtrl->setDisabled(false);
	m_hScrollCtrl->setDisabled(false);
	setStateMode(rmsNone);
	recalcHeadersRects();

}

/**
	Слот, принимающий сигнал об изменении текста в m_textEdit
	реагируем так: измиряем длинну и высоту текста, меняем
	размер, если требуется его увеличить...
*/
void uoReportCtrl::cellTextChangedFromEdit()
{
	QRect rect;
	if (m_textEdit){
		rect = m_textEdit->geometry();
		recalcTextEditRect(rect);
		m_textEdit->setGeometry(rect);
	}
}

/// меняется выделение в документе.
void uoReportCtrl::onSelectonChange(const uorSelectionType& sModeOld, const uorSelectionType& sModeNew)
{
	bool visProp = propertyEditorVisible();
	if (visProp){
		propertyEditorShow();
	}
	m_iteractView->refreshActions(this);
}


/// Регулирование показа сетки, групп, секций, линейки.
void uoReportCtrl::optionShow(bool shGrid, bool shGroup, bool shSection, bool shRuler)
{
	bool shangeAnything = false;
	if (shGrid != m_showGrid) {
		shangeAnything = true;
		m_showGrid = shGrid;
	}
	if (shGroup != m_showGroup) {
		shangeAnything = true;
		m_showGroup = shGroup;
	}
	if (shRuler != m_showRuler) {
		shangeAnything = true;
		m_showRuler = shRuler;
	}
	if (shSection != m_showSection) {
		shangeAnything = true;
		m_showSection = shSection;
	}

	if (shangeAnything) {
		recalcHeadersRects();
	}
	updateImage();

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

	if (m_iteractView->chooseSaveFilePathAndFormat(docFilePath, storeFormat, this)){
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

/// Сигнал на считывание документа.
void uoReportCtrl::onLoad()
{
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	QString 		 docFilePath = doc->getStorePathFile();
	uoRptStoreFormat storeFormat = doc->getStoreFormat();

	if (m_iteractView->chooseLoadFilePathAndFormat(docFilePath, storeFormat, this)){
		doc->clear();
		doc->setStoreOptions(docFilePath, storeFormat);
		doc->load();
		doc->doFormatDoc();
		recalcHeadersRects();
		setStateMode(rmsNone);
		updateImage();
		setFocus();
	}
}

/// Сигнал на очистку документа..
void uoReportCtrl::onClear()
{
	++m_freezUpdate;
	clear();
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	doc->clear();
	recalcHeadersRects();
	--m_freezUpdate;
	updateThis();
}



/// Сигнал на изменение позиции вертикального скрола
void uoReportCtrl::onSetVScrolPos(int y){
	int yC = y * UORPT_VIEW_SCROLL_KOEFF;
	if (y < UORPT_VIEW_SCROLL_KOEFF)		yC = y;
	if (m_areaMain.m_firstVisible_RowTop == yC || yC<0) {
		return;
	}
	m_areaMain.m_firstVisible_RowTop = yC;
	recalcHeadersRects();
	/// нафига они вообще нужны? onSetVScrolPos и onSetHScrolPos
}

/// Сигнал на изменение позиции горизонтального скрола
void uoReportCtrl::onSetHScrolPos(int x){
	int xC = x * UORPT_VIEW_SCROLL_KOEFF;
	if (x < UORPT_VIEW_SCROLL_KOEFF)		xC = x;
	if (xC == m_curentCell.x() || xC<0) {
		return;
	}
	m_areaMain.m_firstVisible_ColLeft = xC;
	recalcHeadersRects();
}

/// попробуем поймать скрол вертикального слайдера..
void uoReportCtrl::onScrollActionV(int act){	doScrollAction(act, uorRhtRowsHeader);}

/// попробуем поймать скрол горизонтального слайдера..
void uoReportCtrl::onScrollActionH(int act){	doScrollAction(act, uorRhtColumnHeader);}

void uoReportCtrl::doScrollAction(int act, uoRptHeaderType rht)
{

	/*
		Тут обрабатывается все другим путем: скрол без изменения
		позиции текущей ячейки. Меняем смещение самого вьюва, а не
		позиции курсора во вьюве..
		int m_rowsInPage; 		///< строк на страницу
		int m_colsInPage; 		///< столбцов на страницу

	*/
	int toX = 0, toY = 0;

	switch(act) {
		case QAbstractSlider::SliderSingleStepAdd:	{
			if (rht == uorRhtRowsHeader) {
				toY = 1;
			} else {
				toX = 1;
			}
			break;
		}
		case QAbstractSlider::SliderSingleStepSub:	{
			if (rht == uorRhtRowsHeader) {
				toY = -1;
			} else {
				toX = -1;
			}
			break;
		}
		case QAbstractSlider::SliderPageStepAdd:	{
			if (rht == uorRhtRowsHeader) {
				toY = 1 * m_rowsInPage;
			} else {
				toX = 1 * m_colsInPage;
			}
			break;
		}
		case QAbstractSlider::SliderPageStepSub:	{
			if (rht == uorRhtRowsHeader) {
				toY = -1 * m_rowsInPage;
			} else {
				toX = -1 * m_colsInPage;
			}
			break;
		}
		case QAbstractSlider::SliderMove:	{
			if (rht == uorRhtRowsHeader) {
				m_areaMain.m_firstVisible_RowTop = m_vScrollCtrl->sliderPosition();
				m_areaMain.m_shift_RowTop = 0.0;
			} else {
				m_areaMain.m_firstVisible_ColLeft = m_hScrollCtrl->sliderPosition();
				m_areaMain.m_shift_ColLeft = 0.0;
			}

			recalcHeadersRects();
			updateThis();
			break;
		}

		default:
		{
			break;
		}
	}
	if (toX != 0 || toY != 0)
		scrollView(toX, toY);


}


/**
	Скролим вьюв на dx - в горизонтальном или dy в вертикальном направлении.
	dx и dy могут быть: положительными, отрицательными или нулевыми.
	если они отрицательные, нужно проследить, что-бы мы не скроли вьюв так,
	что-бы он показывал отрицательные колонки или столбцы...
*/
void uoReportCtrl::scrollView(int dx, int dy)
{
	if (dx ==0 && dy == 0)
		return;
	int pos = 0;

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;

	bool itemHiden = false;
	if (dx != 0) {
		// горизонтальный.
		if (dx > 0) {
			pos = m_areaMain.m_firstVisible_ColLeft + dx;
			while(itemHiden = doc->getScaleHide(uorRhtColumnHeader, pos)){
				++pos;
			}
			m_areaMain.m_firstVisible_ColLeft = pos;
			m_areaMain.m_shift_ColLeft = 0.0;
		} else if (dx < 0){
			pos = m_areaMain.m_firstVisible_ColLeft + dx;
			if (pos > 0) {
				while(itemHiden = doc->getScaleHide(uorRhtColumnHeader, pos) && (pos >= 1)){
					--pos;
				}
			} else {
				pos = 1;
			}
			pos = qMax(1, pos);

			m_areaMain.m_firstVisible_ColLeft = pos;
			m_areaMain.m_shift_ColLeft = 0.0;
		}
		onAccessRowOrCol(m_areaMain.m_firstVisible_ColLeft/* + m_rowsInPage*/, uorRhtColumnHeader);
	}

	if (dy != 0) {
		if (dy > 0) {
			pos = m_areaMain.m_firstVisible_RowTop + dy;
			while(itemHiden = doc->getScaleHide(uorRhtRowsHeader, pos)){
				++pos;
			}
			m_areaMain.m_firstVisible_RowTop = pos;
			m_areaMain.m_shift_RowTop = 0.0;
		} else if (dy < 0){
			pos = m_areaMain.m_firstVisible_RowTop + dy;
			if (pos > 0) {
				while(itemHiden = doc->getScaleHide(uorRhtRowsHeader, pos) && (pos > 1)){
					--pos;
				}
			} else {
				pos = 1;
			}

			m_areaMain.m_firstVisible_RowTop = pos;
			m_areaMain.m_shift_RowTop = 0.0;
		}
		onAccessRowOrCol(m_areaMain.m_firstVisible_RowTop/* + m_rowsInPage*/, uorRhtRowsHeader);

	}
	recalcHeadersRects();
	updateThis();
}

/**
	пересчет точки фиксации строк/столбцов отчета
	стартовоя разбивка на области, потом разбивка будет осуществляться в других функциях
	использование областей описывается в uoReportDescr.h
*/

void uoReportCtrl::recalcFixationPointStart()
{
	m_fixationPoint.setX(0.0);
	m_fixationPoint.setY(0.0);

	return;

	uoReportDoc* doc = getDoc();
	if (!doc)
		return;

	// надо рассчитать точку фиксации вьюва
	int i = 0;
	qreal tmpSize = 0.0;
	if (m_fixationType == uorAF_Rows || m_fixationType == uorAF_RowsAndCols)
	{
		tmpSize = -m_areaMain.m_shift_RowTop;
		i = m_areaMain.m_firstVisible_RowTop;
		for (; i<=m_fixedRow; i++) {
			if (!doc->getScaleHide(uorRhtRowsHeader, i)){
				tmpSize += doc->getScaleSize(uorRhtRowsHeader, i);
			}
		}
		m_fixationPoint.setY(tmpSize);
	}

	if (m_fixationType == uorAF_Cols || m_fixationType == uorAF_RowsAndCols)
	{
		tmpSize = -m_areaMain.m_shift_ColLeft;
		i = m_areaMain.m_firstVisible_ColLeft;
		for (; i<=m_fixedCol; i++) {
			if (!doc->getScaleHide(uorRhtColumnHeader, i)){
				tmpSize += doc->getScaleSize(uorRhtColumnHeader, i);
			}
		}
		m_fixationPoint.setX(tmpSize);
	}
	m_fixationPoint += m_rectDataRegion.topLeft();

	/* тут нам надо пересчитать области фиксации,
	т.е. взять m_fixationPoint поделить область данных на
	квадратные области.	*/

	uorReportViewArea* area = NULL; // m_areas[4];
	// строки просчитаем
	if (m_fixationType == uorAF_RowsAndCols)
	{
		area = &m_areas[0];
		area->m_area.setTopLeft(m_rectDataRegion.topLeft());
		area->m_area.setBottomRight(m_fixationPoint);

		area->m_firstVisible_ColLeft = m_areaMain.m_firstVisible_ColLeft;
		area->m_firstVisible_RowTop = m_areaMain.m_firstVisible_RowTop;
		area->m_shift_RowTop = m_areaMain.m_shift_RowTop;
		area->m_shift_ColLeft = m_areaMain.m_shift_ColLeft;
		area->m_lastVisibleRow = m_fixedRow;
		area->m_lastVisibleCol = m_fixedCol;
	}


}

/// Фиксируем обрасти просмотра отчета для удобства нафигации...
bool	uoReportCtrl::doFixedView(int rows /* = -1*/, int cols /* = -1*/)
{
	// пока замораживаем. мне непонятна механника.
	// а може просто лень.
	bool retVal = true;
		return retVal;

	if (rows < 0)	rows = 0;
	if (cols < 0)	cols = 0;
	if (m_fixedCol == cols && rows == m_fixedRow)
		return retVal;

	m_fixationType = uorAF_None;

	if (cols >0 && rows > 0) {
		m_fixationType = uorAF_RowsAndCols;
	} else if (cols >0) {
		m_fixationType = uorAF_Cols;
	} else if (rows >0) {
		m_fixationType = uorAF_Rows;
	}
	int ctn = 0;
	while(ctn<4){
		m_areas[ctn].clear();
		++ctn;
	}
	m_fixationPoint.setX(0.0);
	m_fixationPoint.setY(0.0);

	m_fixedCol = cols;
	m_fixedRow = rows;
	if (m_fixationType == uorAF_None)
		return retVal;

	recalcFixationPointStart();

	return retVal;
}

/// получить рекст ячейки по строке/ячейке
QRect uoReportCtrl::getCellRect(const int& posY, const int& posX)
{
	QRect rect;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return rect;

	if (m_scaleStartPositionMapV.contains(posY) && m_scaleStartPositionMapH.contains(posX)) {
		rect.setTop((int)(m_scaleStartPositionMapV[posY] * m_scaleFactor));
		rect.setLeft((int)(m_scaleStartPositionMapH[posX] * m_scaleFactor));
		rect.setHeight((int)(doc->getScaleSize(uorRhtRowsHeader,posY) * m_scaleFactor));
		rect.setWidth((int)(doc->getScaleSize(uorRhtColumnHeader,posX) * m_scaleFactor));
	}
	return rect;
}

/// Получить ячейку по локальным экранным координатам.
QPoint uoReportCtrl::getCellFromPosition(const qreal& posY, const qreal& posX)
{
	QPoint cell;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return cell;

	bool isHide = false;

	int rowCur = m_areaMain.m_firstVisible_RowTop;
	int colCur = m_areaMain.m_firstVisible_ColLeft;

	qreal rowsLenCur = m_rectDataRegion.top() - m_areaMain.m_shift_RowTop;
	qreal colsLenCur = m_rectDataRegion.left() - m_areaMain.m_shift_ColLeft;

	qreal sz = 0.0;
	do {	// строки, строки, строки и строки =============
		while((isHide = doc->getScaleHide(uorRhtRowsHeader, rowCur))){
			++rowCur;
		}
		sz = doc->getScaleSize(uorRhtRowsHeader, rowCur);
		if (sz == 0.0) {
			++rowCur;
			continue;
		}
		if (posY>= rowsLenCur && posY <= (rowsLenCur + sz)) {
			cell.setY(rowCur);
			// тут поищем колонку.
			do {

				sz = doc->getScaleSize(uorRhtColumnHeader, colCur);
				if (sz == 0.0) {
					++colCur;
					continue;
				}
				if (doc->getScaleHide(uorRhtColumnHeader, colCur)){
					++colCur;
					continue;
				}
				if (posX>= colsLenCur && posX <= (colsLenCur + sz)) {
					cell.setX(colCur);
					break;
				}
				colsLenCur = colsLenCur + sz;
				++colCur;
			} while(cell.x() == 0);

			break;
		}
		rowsLenCur = rowsLenCur + sz;
		++rowCur;
	} while(cell.y() == 0);


	return cell;
}


/// проверка на видимость ячейки под курсором.
bool uoReportCtrl::curentCellVisible() {
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return false;

	if (
	m_curentCell.x() < m_areaMain.m_firstVisible_ColLeft ||
	m_curentCell.x() > m_areaMain.m_lastVisibleCol ||
	m_curentCell.y() < m_areaMain.m_firstVisible_RowTop ||
	m_curentCell.y() > m_areaMain.m_lastVisibleRow
	) {
		return false;
	} else {
		///\todo надо еще проверить то, что строка или колонка не являются скрытими...
		if (doc->getScaleHide(uorRhtRowsHeader,m_curentCell.y()) || doc->getScaleHide(uorRhtColumnHeader,m_curentCell.x()))
			return false;
		return true;
	}
}

/// Установить текушую ячейку с/без гарантии видимости
/// Перемещаем курсор на ячейку, если ячейка не видима, делаем её видимой.
void uoReportCtrl::setCurentCell(int x, int y, bool ensureVisible)
{
	if (x<=0 || y <=0)	return;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	const QPoint oldPoint = m_curentCell;
	const QPoint& newPoint = QPoint(x,y);
	// Что есть по позиционированию? Какие данные?
	bool itemHide = false;
	uoSideType moveTo = uost_Unknown;
	int oldX = m_curentCell.x(),  oldY = m_curentCell.y();
	if (m_curentCell.y() != y) {
		m_curentCell.setY(y);
		if (ensureVisible){
			// гарантировать видимость ячейки. у-у-у-у!!!
			moveTo = uost_Bottom;
			if (oldY>y)
				moveTo = uost_Top;
			if (moveTo == uost_Top && m_areaMain.m_firstVisible_RowTop >= y){
				m_areaMain.m_firstVisible_RowTop = y;
				m_areaMain.m_shift_RowTop = 0;
				m_areaMain.m_lastVisibleRow = recalcVisibleScales(uorRhtRowsHeader);

			}
			else if (moveTo == uost_Bottom && ( m_areaMain.m_lastVisibleRow) <= y)	{
				// Надо высчитать m_firstVisible_RowTop и m_shift_RowTop
				qreal sizeVAll = m_rectDataRegionFrame.height();
				qreal sizeItem = 0.0;
				m_areaMain.m_shift_RowTop = 0.0;
				int scaleNo = y;
				do {
					m_areaMain.m_firstVisible_RowTop = scaleNo;
					itemHide = doc->getScaleHide(uorRhtRowsHeader,scaleNo);
					if(!itemHide){
						sizeItem = doc->getScaleSize(uorRhtRowsHeader,scaleNo);
						if (sizeItem > 0.0){
							if (sizeItem < sizeVAll){
								sizeVAll = sizeVAll - sizeItem;
							} else {
								m_areaMain.m_shift_RowTop = sizeItem-sizeVAll;
								break;
							}
						}
					}
					--scaleNo;
					if (scaleNo<=0)
						break;
				} while(true);
				m_areaMain.m_lastVisibleRow = y;
			}
			else if (m_areaMain.m_firstVisible_RowTop > y || m_areaMain.m_lastVisibleRow < y) {
				if (m_areaMain.m_firstVisible_RowTop > y) {
					// Текшая ячейка находится вверху относительно видимой области
					m_areaMain.m_firstVisible_RowTop = y;
					m_areaMain.m_shift_RowTop = 0.0;
				} else if (m_areaMain.m_lastVisibleRow < y) {
					m_areaMain.m_lastVisibleRow = y;
					m_areaMain.m_firstVisible_RowTop = recalcVisibleScales(uorRhtRowsHeader);
				}
			}
		}
		recalcHeadersRects();
		onAccessRowOrCol(m_curentCell.y(), uorRhtRowsHeader);

	}
	if (m_curentCell.x() != x){
		m_curentCell.setX(x);
		if (ensureVisible){
			// гарантировать видимость ячейки. у-у-у-у!!!
			moveTo = uost_Right;
			if (oldX>x)
				moveTo = uost_Left;
			if (moveTo == uost_Left && m_areaMain.m_firstVisible_ColLeft >= x){
				m_areaMain.m_firstVisible_ColLeft = x;
				m_areaMain.m_shift_ColLeft = 0;
			}
			else if (moveTo == uost_Right && ( m_areaMain.m_lastVisibleCol-1) <= x)	{
				// Надо высчитать m_firstVisible_ColLeft и m_shift_ColLeft
				qreal sizeVAll = m_rectDataRegionFrame.width();
				qreal sizeItem = 0.0;
				m_areaMain.m_shift_ColLeft = 0.0;
				int scaleNo = x;
				do {
					m_areaMain.m_firstVisible_ColLeft = scaleNo;
					itemHide = doc->getScaleHide(uorRhtColumnHeader, scaleNo);
					if(!itemHide){
						sizeItem = doc->getScaleSize(uorRhtColumnHeader,scaleNo);
						if (sizeItem > 0.0){
							if (sizeItem < sizeVAll){
								sizeVAll = sizeVAll - sizeItem;
							} else {
								m_areaMain.m_shift_ColLeft = sizeItem-sizeVAll;
								break;
							}
						}
					}
					--scaleNo;
					if (scaleNo<=0)
						break;
				} while(true);
			}
			else if (m_areaMain.m_firstVisible_ColLeft > x || m_areaMain.m_lastVisibleCol < x) {
				if (m_areaMain.m_firstVisible_ColLeft > x) {
					// Текшая ячейка находится вверху относительно видимой области
					m_areaMain.m_firstVisible_ColLeft = x;
					m_areaMain.m_shift_ColLeft = 0.0;
				} else if (m_areaMain.m_lastVisibleCol < x) {
					m_areaMain.m_lastVisibleCol = x;
					m_areaMain.m_firstVisible_ColLeft = recalcVisibleScales(uorRhtColumnHeader);
				}
			}
		}
		recalcHeadersRects();
		onAccessRowOrCol(m_curentCell.x(), uorRhtColumnHeader);


	}
	recalcScrollBars();
	emit onCurentCellChange(oldPoint, newPoint);

}


/// Перекалькуляция размеров и положения ползунка скролов
void uoReportCtrl::recalcScrollBars()
{
	// Чё есть?
	//	int m_sizeVvirt;	///< Виртуальный размер документа по вертикали. Виртуальный потому что может увеличиваться скролом.
	//	int m_sizeHvirt;	///< Виртуальный Размер документа по горизонтали
	//	int m_sizeVDoc;	///< Реальный размер документа.
	//	int m_sizeHDoc;	///< Реальный размер документа.
	//	int m_pageWidth;		///< Ширина страницы в столбцах стандартного размера
	//	int m_pageHeight;	///< Высота страницы в строках стандартного размера
	//  как считать?

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	int vsMaxVal = 0, curVal = 0;
	{
		// Вертикальный скролл.
		// вроде пролучается, но какой-то деревянный скролл...
		// Пока отработаем с горизонтальным....

		int heightV = (int) m_rectDataRegion.height();
		if (m_sizeVvirt < heightV) {
			m_sizeVvirt = heightV;
		}
		if (m_sizeVvirt < m_sizeVDoc){
			m_sizeVvirt = m_sizeVDoc;
		}
		m_vScrollCtrl->setMinimum(1);

		m_vScrollCtrl->blockSignals ( true ); //<< лечение от сбоя при PageUp в зоне "непокрытой" длинной документа..

		vsMaxVal = 0, curVal = 0;

		vsMaxVal = qMax(m_rowCountVirt, m_rowCountDoc);
		vsMaxVal = qMax(vsMaxVal,m_areaMain.m_lastVisibleRow);
		vsMaxVal = qMax(vsMaxVal,m_curentCell.y());
		vsMaxVal = int(vsMaxVal / UORPT_VIEW_SCROLL_KOEFF);
		curVal = int(m_areaMain.m_firstVisible_RowTop / UORPT_VIEW_SCROLL_KOEFF);

		m_vScrollCtrl->setMaximum(vsMaxVal);
		m_vScrollCtrl->setValue(qMax(0, curVal-1));
		m_vScrollCtrl->blockSignals ( false );

	}
	{
		int widthW 	= (int)getWidhtWidget(); // / doc->getDefScaleSize(uorRhtColumnHeader);

		if (m_sizeHvirt < (int)widthW) {
			m_sizeHvirt = widthW;
		}
		if (m_sizeHvirt < m_sizeHDoc){
			m_sizeHvirt = m_sizeHDoc;
		}
		vsMaxVal = 0, curVal = 0;
		m_hScrollCtrl->blockSignals ( true );
		m_hScrollCtrl->setMinimum(1);

		vsMaxVal = qMax(m_colCountVirt, m_colCountDoc);
		vsMaxVal = qMax(vsMaxVal,m_areaMain.m_lastVisibleCol);
		vsMaxVal = qMax(vsMaxVal,m_curentCell.x());
		vsMaxVal = int(vsMaxVal / UORPT_VIEW_SCROLL_KOEFF);
		curVal = int(m_areaMain.m_firstVisible_ColLeft / UORPT_VIEW_SCROLL_KOEFF);

		m_hScrollCtrl->setMaximum(vsMaxVal);
		m_hScrollCtrl->setValue(qMax(0, curVal-1));
		m_hScrollCtrl->blockSignals ( false );
	}
}
///< обработать смену виртуального размера. Надо пересчитать вирт. длину и скролы
void uoReportCtrl::doChangeVirtualSize(uoRptHeaderType rht, int changeCnt)
{
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	/// а нафига оно нужно?
	int lineCount; // line тут абстракция - строка или толбец.
	if (rht == uorRhtRowsHeader) {
		lineCount = doc->getRowCount();
		if (lineCount<m_rowCountVirt){
			m_sizeVvirt = (int) (doc->getVSize() + (m_rowCountVirt - lineCount) * doc->getDefScaleSize(rht));
		}
	} else if (rht == uorRhtColumnHeader){
		lineCount = doc->getColCount();
		if (lineCount<m_colCountVirt){
			m_sizeHvirt = (int)(doc->getHSize() + (m_colCountVirt - lineCount) * doc->getDefScaleSize(rht));
		}
	}
}

/// при доступе к строке или столбцу вьюва, если
void uoReportCtrl::onAccessRowOrCol(int nom, uoRptHeaderType rht)
{
	int cnt = 0;
	if (rht == uorRhtColumnHeader) // Колонка
	{
		if (m_colCountVirt < nom) {
			cnt = nom - m_colCountVirt;
			m_colCountVirt = qMax(nom,m_colCountDoc);
			doChangeVirtualSize(rht, cnt);
		} else if (m_areaMain.m_lastVisibleCol<m_colCountVirt){
			cnt = m_colCountVirt - m_areaMain.m_lastVisibleCol;
			m_colCountVirt = qMax(m_areaMain.m_lastVisibleCol,m_colCountDoc);
			doChangeVirtualSize(rht, cnt);
		}
	} else if (rht == uorRhtRowsHeader) {		// строка
		if (m_rowCountVirt < nom) {
			cnt = nom - m_rowCountVirt;
			m_rowCountVirt = nom;
			doChangeVirtualSize(rht, cnt);
		} else if (m_areaMain.m_lastVisibleRow<m_rowCountVirt){
			if (m_curentCell.y()<m_rowCountVirt && m_rowCountVirt > m_rowCountDoc) {
				cnt = m_rowCountVirt - m_areaMain.m_lastVisibleRow;
				m_rowCountVirt = m_areaMain.m_lastVisibleRow;
				doChangeVirtualSize(rht, cnt);
			}
		}
	}
}

///< при доступе к строке или столбцу документа...
void uoReportCtrl::onAccessRowCol(int nmRow, int nmCol)
{
	if (nmRow > 0)		onAccessRowOrCol(nmRow, uorRhtRowsHeader);
	if (nmCol > 0)		onAccessRowOrCol(nmCol, uorRhtColumnHeader);
}

/// Сигнал установки новых размеров документа.
void uoReportCtrl::changeDocSize(qreal sizeV, qreal sizeH)
{
	uoReportDoc* doc = getDoc();
	if (!doc)
		return;
	if (m_rowCountVirt<doc->getRowCount())
		m_rowCountVirt = doc->getRowCount();
	if (m_colCountVirt<doc->getColCount())
		m_colCountVirt = doc->getColCount();


	int newSizeV = (int)sizeV;
	int newSizeH = (int)sizeH;
	if (newSizeV != m_sizeVDoc || m_sizeHDoc != newSizeH){
		m_sizeVvirt = m_sizeVvirt + newSizeV - m_sizeVDoc;
		m_sizeHvirt = m_sizeHvirt + newSizeH - m_sizeHDoc;
		m_sizeVDoc = newSizeV;
		m_sizeHDoc = newSizeH;
		m_rowCountDoc = doc->getRowCount();	///< виртуальные строки вьюва
		m_colCountDoc = doc->getColCount();	///< виртуальные колонки вьюва

		recalcScrollBars();
	}
}


/// Скрываем/показываем сетку.
void uoReportCtrl::onGridShow(){	if (!m_showGrid) {m_showGrid = true;		recalcHeadersRects();	updateImage();}}
/// Скрываем/показываем сетку.
void uoReportCtrl::onGridHide(){	if (m_showGrid)	{m_showGrid = false;		recalcHeadersRects();	updateImage(); }}

void uoReportCtrl::onInvisibleCharShow(){	if (!m_drawHelper->m_showInvisiblChar) {		m_drawHelper->m_showInvisiblChar = true;		updateImage();	}}
void uoReportCtrl::onInvisibleCharHide(){	if (m_drawHelper->m_showInvisiblChar) {		m_drawHelper->m_showInvisiblChar = false;		updateImage();	}}


/// Скрываем/показываем рамку вокруг окна....
void uoReportCtrl::onFrameShow(){	if (!m_showFrame) 	{m_showFrame = true;		recalcHeadersRects();	updateImage();}}
void uoReportCtrl::onFrameHide(){	if (m_showFrame) 	{m_showFrame = false;	recalcHeadersRects();	updateImage();}}

/// Скрываем/показываем линейку.
void uoReportCtrl::onRulerShow(){	if (!m_showRuler) 	{m_showRuler = true;		recalcHeadersRects();	updateImage();}}
void uoReportCtrl::onRulerHide(){	if (m_showRuler) 	{m_showRuler = false;		recalcHeadersRects();	updateImage();}}

/// Скрываем/показываем секции.
void uoReportCtrl::onSectionShow(){	if (!m_showSection) 	{m_showSection = true;		recalcHeadersRects();	updateImage();}}
void uoReportCtrl::onSectionHide(){	if (m_showSection) 	{m_showSection = false;		recalcHeadersRects();	updateImage();}}

/// Скрываем/показываем группировки.
void uoReportCtrl::onGroupShow(){	if (!m_showGroup) 	{m_showGroup = true;			recalcHeadersRects();	updateImage();}}
void uoReportCtrl::onGroupHide(){	if (m_showGroup) 	{m_showGroup = false;		recalcHeadersRects();	updateImage();}}

/// Вывод отледачной информации по вычислению пространств управляющей области..
void uoReportCtrl::onOutToDebug() {debugRects();}


} //namespace uoReport
