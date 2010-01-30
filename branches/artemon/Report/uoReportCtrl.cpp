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
#include <QProgressDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include "uoReportDocBody.h"
#include "uoReportUndo.h"
#include "uoReportPreviewDlg.h"
#include "uorOptionsDlg.h"
#include "uoReportDrawHelper.h"
#include "uorPagePrintSetings.h"
#include "uorPageSetup.h"
#include "uorMimeData.h"
#include "uoReportManager.h"
#include "uoPainter.h"


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
	//setAttribute(Qt::WA_PaintOnScreen);
	m_dirtyDrawErea = uorDDE_All;
	m_cashDrawSellType = uoRst_Unknown; //= m_selections->selectionType();
	m_cashDrawMaVer = 0; 	 //= m_areaMain.changesVer();
	m_cashDrawDocVer = 0; 	 //= m_doc.changesVer();
	m_sPanel		= 0;

	m_rptDoc 		= NULL;
	m_useMode 		= rmDevelMode;
	m_resizeLine		= 0;
	m_freezUpdate 	= 0;
	m_rowCountDoc = m_colCountDoc = 0;
	m_fixationType = uorAF_None;
	m_fixationPoint = uorPoint(uorNumberNull, uorNumberNull);

	m_charWidthPlus 	= 3;

	m_maxVisibleLineNumberCnt = 3;

	m_areaMain.setFirstVisible_RowTop(1); 	///< Первая верхняя видимая строка
	m_areaMain.setFirstVisible_ColLeft(1); 	///< Первая левая видимая колонка
	m_areaMain.setLastVisibleRow(-1);
	m_areaMain.setLastVisibleCol(-1);

	m_scaleFactor 			= m_scaleFactorO = 1;

	m_areaMain.setShift_RowTop(0); ///< Смещение первой видимой строки вверх (грубо - размер невидимой их части)
	m_areaMain.setShift_ColLeft(0);

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
	connect(m_iteractView, SIGNAL(onScaleChange(uorNumber)), this, SLOT(onSetScaleFactor(uorNumber)));
	connect(m_selections, SIGNAL(onSelectonChange(uorSelectionType,uorSelectionType)), this, SLOT(onSelectonChange(uorSelectionType,uorSelectionType)));
	setDoc(new uoReportDoc());
	m_selections->setDoc(getDoc());

	m_drawHelper = new uoReportDrawHelper(getDoc());
	m_drawHelper->m_showInvisiblChar = false;

	m_charWidthPlus 	= 3;
 	m_showGroup		= false;
	m_showSection	= false;
	m_showRuler		= true;
	m_showGrid		= true;
	m_showFrame		= true;
	m_saveWithSelection = false;
	m_directDraw 	= false;



	m_imageView = QImage(size(), QImage::Format_ARGB32_Premultiplied);
	m_drawToImage = 1;
	m_fixationPoint = QPoint(0,0);
	m_fixationType = uorAF_None;
	m_paintLocker = 0;
}

/// Инициализация контролов поля отчета.
void uoReportCtrl::initControls(QWidget *parent){

	m_textEdit = NULL; // создадим когда понадобится...

	m_cornerWidget = new QWidget(parent); //, m_cornerWidget(parent)
	QGridLayout* layout = new  QGridLayout(parent);
	QGridLayout *gridLayout1 = new QGridLayout();
	gridLayout1->setHorizontalSpacing(0);
    gridLayout1->setVerticalSpacing(0);
	layout->setMargin(0);	/// нулевой отступ...
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	m_sPanel = new uoSheetPanel(parent);
	m_sPanel->setAdress(QString("R1:C1"));
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

	gridLayout1->addWidget(m_sPanel, 0, 0, 1, 2);//	layout->addWidget( m_sPanel, 0, 0 );


	gridLayout1->addWidget(this, 1, 0, 1, 1); //layout->addWidget( this, 1, 0 );

	gridLayout1->addWidget(m_hScrollCtrl, 2, 0, 1, 1);	//layout->addWidget(m_hScrollCtrl,1,0);
	gridLayout1->addWidget(m_vScrollCtrl, 1, 1, 1, 1);	//layout->addWidget(m_vScrollCtrl,1,1 );


    gridLayout1->addWidget(m_cornerWidget, 2, 1, 1, 1);	//layout->addWidget(m_cornerWidget,2,1);
    layout->addLayout(gridLayout1, 0, 0, 1, 1);

    m_messageFilter = new uoReportCtrlMesFilter(this);
    m_propEditor = NULL;

}


uoReportCtrl::~uoReportCtrl()
{
	//delete _repoDoc; //НЕТ!! нужно смотреть не подсоединен ли этот док к еще одному вьюву или к переменной!!!
	m_sPanel = 0;
	delete m_iteractView;
	clear();
}

///Очистка данных класса.
void uoReportCtrl::clear(){
	dropGropItemToCache();
	while (!m_groupListCache->isEmpty())    delete m_groupListCache->takeFirst();
	m_scaleStartPositionMapH.clear();
	m_scaleStartPositionMapV.clear();
	while (!m_shortcutList.isEmpty())    delete m_shortcutList.takeFirst();

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
	connect(m_rptDoc, SIGNAL(onSizeChange(uorNumber,uorNumber)), this, SLOT(changeDocSize(uorNumber, uorNumber)));


}


/// Перекалькулируем последние видимые ячейки и сопутствующие данные.
int uoReportCtrl::recalcVisibleScales(const uoRptHeaderType& rht){

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return -1;
	// так бля. начинаю рефакторинг. буду ломать.. (((((
	int numScale = 1;
	uorNumber curetnSize = 0;

	uorNumber sizeScale = 0;
	bool isHiden = false;
	uorNumber targetSize = (rht==uorRhtColumnHeader) ? m_rectDataRegion.right() : m_rectDataRegion.bottom();
	uorNumber shiftSize = uorNumberNull;
	if (rht == uorRhtColumnHeader) {
		m_scaleStartPositionMapH.clear();
		numScale 		= m_areaMain.firstVisible_ColLeft();
		shiftSize 	= m_rectDataRegion.left();
		curetnSize =  shiftSize - m_areaMain.shift_ColLeft();
		m_scaleStartPositionMapH[numScale] = curetnSize;
	} else if (rht == uorRhtRowsHeader){
		m_scaleStartPositionMapV.clear();
		numScale 		= m_areaMain.firstVisible_RowTop();
		curetnSize = m_rectDataRegion.top() - m_areaMain.shift_RowTop();
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
	grItem->m_rectMidlePos = uorNumberNull;
	grItem->m_rectEndPos = uorNumberNull;
	grItem->m_sizeTail = uorNumberNull;
	grItem->m_tailPosIsAbs = false;
	bool itemVisible = false;


	uorNumber xPos = -10,yPos = -10, yPos0 = -10, xSize = 0, xSize0 = 0, ySize = 0, ySize0 = 0;
	if (rht == uorRhtColumnHeader) {

		if (m_scaleStartPositionMapH.contains(grItem->m_start))
			xPos = m_scaleStartPositionMapH[grItem->m_start];

		itemVisible = colVisible(grItem->m_start);
		if (itemVisible) {

			xSize0 = xSize = doc->getScaleSize(rht, grItem->m_start);
			grItem->m_rectIteract.setLeft(xPos);
			grItem->m_rectIteract.setRight(xPos);


			grItem->m_sizeTail = xPos + xSize; // Координаты конца ячейки.

			if (xSize0 > (UORPT_SCALE_FOLD_ITEM_SIZE + UORPT_OFFSET_LINE*2)){


				xSize = (xSize - (UORPT_OFFSET_LINE + UORPT_SCALE_FOLD_ITEM_SIZE)) / 2;
				xPos = xPos + xSize;
				grItem->m_rectIteract.setLeft(xPos);
				xPos = xPos + UORPT_OFFSET_LINE * 2 + UORPT_SCALE_FOLD_ITEM_SIZE;
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

		yPos = m_rectGroupCol.top() + ( UORPT_OFFSET_LINE /* * 2*/ + UORPT_SCALE_FOLD_ITEM_SIZE) * grItem->m_level - UORPT_OFFSET_LINE;
		grItem->m_rectIteract.setBottom(yPos);
		yPos = yPos - UORPT_SCALE_FOLD_ITEM_SIZE;		//yPos = yPos - m_charHeightPlus;
		grItem->m_rectIteract.setTop(yPos);

		grItem->m_rectMidlePos = yPos + UORPT_SCALE_FOLD_ITEM_SIZE / 2; //		grItem->m_rectMidlePos = yPos + m_charHeightPlus / 2;

		grItem->m_rectIteract.adjust((uorNumber)0.0, (uorNumber)1.0, (uorNumber)0.0, (uorNumber)-1.0);
		grItem->m_rectEndPos = xPos;
		grItem->m_sizeTail = grItem->m_sizeTail - xPos; // Координаты конца ячейки.


	} else if (rht == uorRhtRowsHeader) {
		if (m_scaleStartPositionMapV.contains(grItem->m_start))
			yPos0 = yPos = m_scaleStartPositionMapV[grItem->m_start];

		itemVisible = rowVisible(grItem->m_start);

		if (itemVisible) {
			grItem->m_sizeTail = yPos;

			ySize0 = ySize = doc->getScaleSize(rht, grItem->m_start);
			ySize = (ySize  - UORPT_SCALE_FOLD_ITEM_SIZE) / 2; // Заменил m_charHeightPlus, думаю более актуально.
			grItem->m_sizeTail = yPos + ySize0; // Координаты конца ячейки.

			grItem->m_rectIteract.setTop(yPos + ySize);
			ySize = ySize + UORPT_SCALE_FOLD_ITEM_SIZE; // Заменил m_charHeightPlus, думаю более актуально.
			grItem->m_rectIteract.setBottom(yPos + ySize);
			if (grItem->m_rectIteract.height()> ySize0){
				grItem->m_rectIteract.setTop(yPos0 + UORPT_OFFSET_LINE);
				grItem->m_rectIteract.setBottom(yPos0 + ySize0 - UORPT_OFFSET_LINE);
			}


		} else {
			// ну вот так топорно решим глюку..
			grItem->m_rectIteract.setTop(-10);
			grItem->m_rectIteract.setBottom(-20);
		}


		xPos = m_rectGroupRow.left() + ( UORPT_OFFSET_LINE * 2 + UORPT_SCALE_FOLD_ITEM_SIZE) * grItem->m_level; //		xPos = m_rectGroupRow.left() + ( UORPT_OFFSET_LINE * 2 + m_charWidthPlus) * grItem->m_level;
		grItem->m_rectIteract.setRight(xPos);
		xPos = xPos - (UORPT_SCALE_FOLD_ITEM_SIZE + UORPT_OFFSET_LINE * 2);	//	xPos = xPos - (m_charWidthPlus + UORPT_OFFSET_LINE * 2);
		grItem->m_rectIteract.setLeft(xPos);

		grItem->m_rectMidlePos = xPos + UORPT_SCALE_FOLD_ITEM_SIZE / 2 + UORPT_OFFSET_LINE;//		grItem->m_rectMidlePos = xPos + m_charWidthPlus / 2 + UORPT_OFFSET_LINE;

		grItem->m_rectIteract.adjust((uorNumber)1.0, uorNumberNull, (uorNumber)-1.0,uorNumberNull);

		grItem->m_rectEndPos = grItem->m_rectIteract.bottom();
		grItem->m_sizeTail = grItem->m_sizeTail - grItem->m_rectEndPos; // Координаты конца ячейки.


	}
}

/// Вычислить длину диапазона ячеек.
uorNumber uoReportCtrl::getLengthOfScale(const uoRptHeaderType& rht, const int& start, const int& stop)
{
	uorNumber retVal = uorNumberNull;

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
	uorNumber rSize = uorNumberNull;

	// Маленькая оптимизация. Если у нас поменялась только высота строки, не нужно пересчитывать вертикальный хейдер.
	if (rht == uorRhtUnknown || rht == uorRhtColumnHeader)
		m_areaMain.setLastVisibleCol(recalcVisibleScales(uorRhtColumnHeader));

	if (rht == uorRhtUnknown || rht == uorRhtRowsHeader) {
		m_areaMain.setLastVisibleRow(recalcVisibleScales(uorRhtRowsHeader));
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
				const spanList* spanListH = doc->getGroupList(rhtCur, m_areaMain.firstVisible_ColLeft(), m_areaMain.lastVisibleCol());
				for (i=0; i<spanListH->size(); i++){
					spn = spanListH->at(i);
					grItem = getGropItemFromCache();
					if (grItem)	{
						grItem->copyFrom(spn);
						grItem->m_sizeTail = uorNumberNull;
						uorZeroRectF(grItem->m_rectIteract);
						grItem->m_tailPosIsAbs = false;

						rSize = uorNumberNull;
						calcGroupItemPosition(grItem, rhtCur);
						// посчитаем длину линии группировки.
						// хвост есть, нужно вычислить только толщину последующих ячеек и добавить его к хвосту.
						if (grItem->m_start >= m_areaMain.firstVisible_ColLeft()){
							// это если рект группировки нормально отображается, а если нет?
							grItem->m_sizeTail = grItem->m_sizeTail + getLengthOfScale(rhtCur, grItem->m_start + 1, grItem->m_end);
						} else {
							grItem->m_sizeTail = uorNumberNull;
							if (m_areaMain.firstVisible_ColLeft() <= grItem->m_end) {
								grItem->m_tailPosIsAbs = true;
								grItem->m_sizeTail = getLengthOfScale(rhtCur, m_areaMain.firstVisible_ColLeft(), grItem->m_end) - m_areaMain.shift_ColLeft();
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
				const spanList* spanListV = doc->getGroupList(rhtCur, m_areaMain.firstVisible_RowTop(), m_areaMain.lastVisibleRow());
				for (i=0; i<spanListV->size(); i++){
					spn = spanListV->at(i);
					grItem = getGropItemFromCache();
					if (grItem)	{
						grItem->copyFrom(spn);

						uorZeroRectF(grItem->m_rectIteract);
						grItem->m_tailPosIsAbs = false;
						calcGroupItemPosition(grItem, rhtCur);


						if (grItem->m_start >= m_areaMain.firstVisible_RowTop()){
							// это если рект группировки нормально отображается, а если нет?
							grItem->m_sizeTail = grItem->m_sizeTail + getLengthOfScale(rhtCur, grItem->m_start + 1, grItem->m_end);
						} else {
							grItem->m_sizeTail = uorNumberNull;
							if (m_areaMain.firstVisible_RowTop() <= grItem->m_end) {
								grItem->m_tailPosIsAbs = true;
								grItem->m_sizeTail = getLengthOfScale(rhtCur, m_areaMain.firstVisible_RowTop(), grItem->m_end) - m_areaMain.shift_RowTop();
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
		uorNumber coord = uorNumberNull;
		uorNumber offSet = uorNumberNull;

		if (rht == uorRhtUnknown || rht == uorRhtColumnHeader)
		{
			rhtCur = uorRhtColumnHeader;
			sectLevel = doc->getSectionLevel(rhtCur);
			if (sectLevel>0){

				const spanList* spanSectListH = doc->getSectionList(rhtCur, m_areaMain.firstVisible_ColLeft(), m_areaMain.lastVisibleCol());
				for (i=0; i<spanSectListH->size(); i++){
					spn = spanSectListH->at(i);
					selectItem = getSectionItemFromCache();
					if (selectItem)	{
						selectItem->copyFrom(spn);
						uorZeroRectF(selectItem->m_rectView);
						selectItem->m_selected = false;

						// Посчитаем рект данной секции....
						uorRect& rect = selectItem->m_rectView;
						offSet = uorNumberNull;
						coord = m_rectSectionCol.top();
						coord = coord - ( UORPT_OFFSET_LINE  *2  + UORPT_SCALE_FOLD_ITEM_SIZE) * (1 - selectItem->m_level);
						// С каждым уровнем секция все "ниже". Вот только не помню, левел нуль-ориентрованный или как?
						rect.setTop(coord);
						rect.setBottom(m_rectSectionCol.bottom());

						cellVisStart = qMax(selectItem->m_start, m_areaMain.firstVisible_ColLeft());
						cellVisEnd = qMin(selectItem->m_end, m_areaMain.lastVisibleCol());

						coord = m_rectSectionCol.left()-1; // пусть спрячется, если что..
						if (m_scaleStartPositionMapH.contains(cellVisStart)){
							coord = m_scaleStartPositionMapH[cellVisStart];
							if (cellVisStart == m_areaMain.firstVisible_ColLeft())
								offSet = m_areaMain.shift_ColLeft();
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

				const spanList* spanSectListV = doc->getSectionList(rhtCur, m_areaMain.firstVisible_RowTop(), m_areaMain.lastVisibleRow());
				for (i=0; i<spanSectListV->size(); i++){
					spn = spanSectListV->at(i);
					selectItem = getSectionItemFromCache();
					if (selectItem)	{
						selectItem->copyFrom(spn);
						uorZeroRectF(selectItem->m_rectView);
						selectItem->m_selected = false;

						cellVisStart = qMax(selectItem->m_start, m_areaMain.firstVisible_RowTop());
						cellVisEnd = qMin(selectItem->m_end, m_areaMain.lastVisibleRow());

						// Посчитаем рект данной секции....
						uorRect& rect = selectItem->m_rectView;
						offSet = uorNumberNull;
						coord = uorNumberNull;
						coord = m_rectSectionRow.left();
						offSet = m_rectSectionRow.right() - m_rectSectionRow.left();

						offSet = offSet * (selectItem->m_level-1) / sectLevel;
						coord = coord + offSet;
						offSet = uorNumberNull;

						// С каждым уровнем секция все "ниже". Вот только не помню, левел нуль-ориентрованный или как?
						rect.setLeft(coord);
						rect.setRight(m_rectSectionRow.right());

						coord = m_rectSectionRow.top()-2; // пусть спрячется, если что..
						if (m_scaleStartPositionMapV.contains(cellVisStart)){
							coord = m_scaleStartPositionMapV[cellVisStart];
							if (cellVisStart == m_areaMain.firstVisible_RowTop())
								offSet = m_areaMain.shift_RowTop();
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
	uorNumber sz = uorNumberNull;
	if (!doc)
		return;
	uoRptHeaderType rht = uorRhtRowsHeader;
	if (m_fixationType == uorAF_Rows || m_fixationType == uorAF_RowsAndCols) {
		area = &m_areas[0];
		sz = area->shift_RowTop();
		for (int i = area->firstVisible_RowTop();		i<=area->lastVisibleRow();		i++)
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
		sz = area->shift_ColLeft();
		for (int i = area->firstVisible_ColLeft();		i<=area->lastVisibleCol();		i++)
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
    uorNumber wWidth = getWidhtWidget();
    uorNumber wHeight = getHeightWidget();


	uorZeroRectF(m_rectGroupRow);  uorZeroRectF(m_rectSectionRow);	uorZeroRectF(m_rectRulerRow);
	uorZeroRectF(m_rectGroupCol);  uorZeroRectF(m_rectSectionCol);	uorZeroRectF(m_rectRulerCol);
	uorZeroRectF(m_rectRuleCorner);
	uorZeroRectF(m_rectAll);
	uorZeroRectF(m_rectDataRegion);	uorZeroRectF(m_rectDataRegionFrame);

	m_rectAll.setTopLeft(QPoint(1,1));
	m_rectAll.setBottom(wHeight);			m_rectAll.setRight(wWidth);
	m_rectDataRegion.setBottom(wHeight);	m_rectDataRegion.setRight(wWidth);
	m_rectGroupCol.setRight(wWidth);		m_rectSectionCol.setRight(wWidth); 	m_rectRulerCol.setRight(wWidth);
	m_rectGroupRow.setBottom(wHeight);	m_rectSectionRow.setBottom(wHeight); 	m_rectRulerRow.setBottom(wHeight);

	uorNumber curOffset = 0;

	// Расчитаем сначала высотные размеры горизонтальной секции
	int spnCntCol = doc->getGroupLevel(uorRhtColumnHeader);
	if (spnCntCol>0 && m_showGroup) {
		m_rectGroupCol.setTop(curOffset);
		curOffset += ( UORPT_OFFSET_LINE /* * 2*/ + UORPT_SCALE_FOLD_ITEM_SIZE) * spnCntCol;//		curOffset += ( UORPT_OFFSET_LINE * 2 + m_charHeightPlus) * spnCntCol;
		m_rectGroupCol.setBottom(curOffset);
	}
	spnCntCol = doc->getSectionLevel(uorRhtColumnHeader);
	if (spnCntCol>0 && m_showSection) {
		curOffset += UORPT_OFFSET_LINE; // Оффсет-отступ сверху.
		m_rectSectionCol.setTop(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + UORPT_SCALE_FOLD_ITEM_SIZE) * spnCntCol;		//curOffset += ( UORPT_OFFSET_LINE * 2 + m_charHeightPlus) * spnCntCol;
		m_rectSectionCol.setBottom(curOffset);
	}

	if (m_showRuler) {
		m_rectRulerCol.setTop(curOffset);
		curOffset += m_charHeightPlus;
		curOffset += UORPT_OFFSET_LINE;
		m_rectRulerCol.setBottom(curOffset);
	}
	m_rectDataRegion.setTop(curOffset);
	m_rectGroupRow.setTop(curOffset);	m_rectSectionRow.setTop(curOffset);	m_rectRulerRow.setTop(curOffset);

	curOffset = 0;
	int spnCntV = doc->getGroupLevel(uorRhtRowsHeader);
	if (spnCntV>0 && m_showGroup) {
		curOffset += UORPT_OFFSET_LINE;
		m_rectGroupRow.setLeft(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + UORPT_SCALE_FOLD_ITEM_SIZE) * spnCntV; //	curOffset += ( UORPT_OFFSET_LINE * 2 + m_charWidthPlus) * spnCntV;
		m_rectGroupRow.setRight(curOffset);
	}
	spnCntV = doc->getSectionLevel(uorRhtRowsHeader);
	if (spnCntV>0 && m_showSection) {
		curOffset += UORPT_OFFSET_LINE; // Оффсет-отступ сверху.
		m_rectSectionRow.setLeft(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + UORPT_SCALE_FOLD_ITEM_SIZE * UORPT_LENGTH_TEXT_H_SECTION) * spnCntV;//		curOffset += ( UORPT_OFFSET_LINE * 2 + m_charWidthPlus*UORPT_LENGTH_TEXT_H_SECTION) * spnCntV;
		m_rectSectionRow.setRight(curOffset);
	}

	if (m_showRuler) {
		m_rectRulerRow.setLeft(curOffset);
		curOffset += UORPT_SCALE_FOLD_ITEM_SIZE * m_maxVisibleLineNumberCnt+5;//		curOffset += m_charWidthPlus * m_maxVisibleLineNumberCnt+5;
		curOffset += UORPT_OFFSET_LINE;
		m_rectRulerRow.setRight(curOffset);

		m_rectRuleCorner.setTop(m_rectRulerCol.top());
		m_rectRuleCorner.setLeft(m_rectRulerRow.left());
		m_rectRuleCorner.setBottom(m_rectRulerCol.bottom());
		m_rectRuleCorner.setRight(m_rectRulerRow.right());
	}
	m_rectDataRegion.setLeft(curOffset);
	m_rectDataRegionFrame.setTop(m_rectDataRegion.top());
	m_rectDataRegionFrame.setBottom(m_rectDataRegion.bottom());
	m_rectDataRegionFrame.setLeft(m_rectDataRegion.left());
	m_rectDataRegionFrame.setRight(m_rectDataRegion.right());
	m_rectDataRegion.adjust(1, 1, -1, -1);

	m_rectGroupCol.setLeft(curOffset);		m_rectSectionCol.setLeft(curOffset); 	m_rectRulerCol.setLeft(curOffset);

	// обнулим ненужные...
	if (!m_showGroup)	{uorZeroRectF(m_rectGroupRow);  	uorZeroRectF(m_rectGroupCol);		}
	if (!m_showSection) 	{uorZeroRectF(m_rectSectionRow); 	uorZeroRectF(m_rectSectionCol);	}
	if (!m_showRuler) 	{uorZeroRectF(m_rectRulerRow); 	uorZeroRectF(m_rectRulerCol);	uorZeroRectF(m_rectRuleCorner);}

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
//	qDebug() << "m_rectGroupRow" << 		m_rectGroupRow;
//	qDebug() << "m_rectSectionRow" << 		m_rectSectionRow;
//	qDebug() << "m_rectRulerRow" << 		m_rectRulerRow;
//	qDebug() << "m_rectGroupCol" << 		m_rectGroupCol;
//	qDebug() << "m_rectSectionCol" << 		m_rectSectionCol;
//	qDebug() << "m_rectRulerCol" << 		m_rectRulerCol;
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
void uoReportCtrl::drawHeaderControlContour(uoPainter& painter)
{
	uorNumber noLen = 2;
	bool drawSelfRects = false;
	painter.save();
	m_drawHelper->m_penText.setStyle(Qt::DotLine);
	painter.setPen(m_drawHelper->m_penText);
	// Рисуем контуры пространств, чисто для визуального контроля...
	if (m_showGroup) {
		if (m_rectGroupRow.width()>noLen)	{
			if (drawSelfRects)
				painter.drawRect(m_rectGroupRow);
		}
		if (m_rectGroupCol.height()>noLen)		{
			if (drawSelfRects)
				painter.drawRect(m_rectGroupCol);
		}
	}
	if (m_showSection) {
		if (m_rectSectionRow.width()>noLen) {
			if (drawSelfRects)
				painter.drawRect(m_rectSectionRow);
		}
		if (m_rectSectionCol.height()>noLen) {
			if (drawSelfRects)
					painter.drawRect(m_rectSectionCol);
		}
	}
	if (m_showRuler) {
		if (m_rectRulerRow.width()>noLen) {
			if (drawSelfRects)
				painter.drawRect(m_rectRulerRow);
		}
		if (m_rectRulerCol.height()>noLen) {
			if (drawSelfRects)
				painter.drawRect(m_rectRulerCol);
		}
	}

	painter.restore();
}


/// Отрисовка групп в HeaderControl.
void uoReportCtrl::drawHeaderControlGroup(uoPainter& painter)
{
	int cntr = 0;
	uoRptGroupItem* grItem;
	uorNumber noLen = (uorNumber)2;
	uorNumber minDrawSize = (uorNumber)2.5;
	QString paintStr = "";
	uorPoint pointStart, pointEnd;
	uorNumber pos = uorNumberNull;

	painter.save();
	m_drawHelper->m_penText.setStyle(Qt::SolidLine);
	painter.setPen(m_drawHelper->m_penText);

	// Рисуем шапку, как она сама есть...
	if (m_showGroup) {
		if (m_rectGroupRow.width()>noLen)	{
			if (!m_groupListV->isEmpty()) {
				painter.setClipRect(m_rectGroupRow); // Устанавливаем область прорисовки. Будем рисовать только в ней.
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
							pos = m_rectGroupRow.top(); //grItem->m_rectEndPos;
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
		if (m_rectGroupCol.height()>noLen)		{
			if (!m_groupListH->isEmpty()) {
				painter.setClipRect(m_rectGroupCol); // Устанавливаем область прорисовки. Будем рисовать только в ней.
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
							pos = m_rectGroupCol.left();
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
			painter.setClipRect(m_rectSectionCol); // Устанавливаем область прорисовки. Будем рисовать только в ней.
			for (cntr = 0; cntr<m_sectItemListH->size(); cntr++) {
				sectItem = m_sectItemListH->at(cntr);
				uorRect rect = sectItem->m_rectView;
				painter.setPen(m_drawHelper->m_penText);
				if (sectItem->m_selected){
					painter.fillRect(rect, m_drawHelper->m_brushBlack);
					painter.setPen(m_drawHelper->m_penWhiteText);
				} else {
					painter.fillRect(rect, m_drawHelper->m_brushWindow);
				}
				painter.drawRect(rect);
				rect.adjust((uorNumber)1,(uorNumber)0,(uorNumber)-1,(uorNumber)0);
				painter.drawText(rect,Qt::AlignLeft,  sectItem->m_nameSections);

			}
		}
		if (!m_sectItemListV->isEmpty()){
			painter.setClipRect(m_rectSectionRow); // Устанавливаем область прорисовки. Будем рисовать только в ней.
			for (cntr = 0; cntr<m_sectItemListV->size(); cntr++) {
				sectItem = m_sectItemListV->at(cntr);
				uorRect rect = sectItem->m_rectView;
				painter.setPen(m_drawHelper->m_penText);
				if (sectItem->m_selected){
					painter.fillRect(rect, m_drawHelper->m_brushBlack);
					painter.setPen(m_drawHelper->m_penWhiteText);
				} else {
					painter.fillRect(rect, m_drawHelper->m_brushWindow);
				}
				painter.drawRect(rect);
				rect.adjust((uorNumber)2,(uorNumber)0,(uorNumber)-1,(uorNumber)0);
				painter.drawText(rect,Qt::AlignLeft,  sectItem->m_nameSections);

			}
		}
	}
	painter.setClipRect(m_rectAll); // Устанавливаем область прорисовки. Будем рисовать только в ней.
	painter.restore();
}


/// Отрисовка нерабочей области отчета: Группировки, секции, линейки, общий фрайм.
void uoReportCtrl::drawHeaderControl(uoPainter& painter){


	if (m_showFrame) {
//		painter.drawRect(m_rectAll);
	}

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	QString paintStr = "";

	m_drawHelper->m_penText.setStyle(Qt::SolidLine);
	painter.setPen(m_drawHelper->m_penText);

	bool isSell = false;
	//bool isSellPart = false;
	/*
		попробуем порисовать линейку...
		вот тут нужен такой фокус, необходимо научиться рисовать
		отсеченные части ректов и прочих фигур.
	*/


	uorNumber paintEndTagr = uorNumberNull; // Конечная граница отрисовки.
	uorNumber paintCntTagr = uorNumberNull; // Текущая величина отрисовки.
	uorNumber curSize = uorNumberNull; // Текущая величина отрисовки.

	int nmLine = 0;
	uoRptHeaderType hdrType;
	uorRect curRct, curRctCpy; // копия, для извращений...
	uorPoint posStart, posEnd;

	uorZeroRectF(curRct);

	//painter.drawRect(m_rectDataRegion); /// когда отрисовывалась только линейка, эта конструкция отрисовывала лишнюю рамку.

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
		painter.fillRect(curRctCpy, m_drawHelper->m_brushWindow);
		painter.drawRect(curRctCpy);
		curRctCpy.adjust(1,1,-1,-1);
		if (m_selections->isDocumSelect()){
			painter.fillRect(curRctCpy, m_drawHelper->m_brushDarkPhone);
		} else {
			painter.fillRect(curRctCpy, m_drawHelper->m_brushWindow);
		}
		painter.setPen(m_drawHelper->m_penText);

		hdrType = uorRhtRowsHeader;
		if (m_rectRulerRow.width() > 0) {

			painter.setClipRect(m_rectRulerRow); // Устанавливаем область прорисовки. Будем рисовать только в ней.
			painter.fillRect(m_rectRulerRow, m_drawHelper->m_brushWindow);

			curRct.setLeft(m_rectRulerRow.left()+m_scaleFactorO);
			curRct.setRight(m_rectRulerRow.right()-m_scaleFactorO);

			paintEndTagr = m_rectRulerRow.bottom();
			paintCntTagr = m_rectRulerRow.top() - m_areaMain.shift_RowTop() + 1 * m_scaleFactorO;
			curRct.setTop(paintCntTagr);
			nmLine = m_areaMain.firstVisible_RowTop()-1;
			do {
				nmLine = nmLine + 1;

				if (doc->getScaleHide(hdrType, nmLine))
					continue;
				if ((curSize = doc->getScaleSize(hdrType, nmLine)) == uorNumberNull)
					continue;

				isSell = m_selections->isRowSelect(nmLine) || m_selections->isRowPartlySelect(nmLine);
				if (!isSell){
					if (m_curentCell.y() == nmLine)
						isSell = true;
				}

				paintCntTagr = paintCntTagr + curSize;
				curRct.setBottom(paintCntTagr);
				painter.drawRect(curRct);

				if (isSell) {
					painter.save();
					painter.setPen(m_drawHelper->m_penWhiteText);
					painter.setBrush(m_drawHelper->m_brushDarkPhone);
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
		if (m_rectRulerCol.width() > 0) {

			painter.setClipRect(m_rectRulerCol); // Устанавливаем область прорисовки. Будем рисовать только в ней.
			painter.fillRect(m_rectRulerCol, m_drawHelper->m_brushWindow);
			curRct.setTop(m_rectRulerCol.top()+m_scaleFactorO);
			curRct.setBottom(m_rectRulerCol.bottom()-m_scaleFactorO);

			paintEndTagr = m_rectRulerCol.right();
			paintCntTagr = m_rectRulerCol.left() - m_areaMain.shift_ColLeft() + 1 * m_scaleFactorO;

			curRct.setLeft(paintCntTagr);
			nmLine = m_areaMain.firstVisible_ColLeft() - 1;
			do {
				nmLine = nmLine + 1;

				if (doc->getScaleHide(hdrType, nmLine))
					continue;
				if ((curSize = doc->getScaleSize(hdrType, nmLine)) == uorNumberNull)
					continue;

				paintCntTagr = paintCntTagr + curSize;

				isSell = m_selections->isColSelect(nmLine)  || m_selections->isColPartlySelect(nmLine);
				if (!isSell){
					if (m_curentCell.x() == nmLine)
						isSell = true;
				}


				curRct.setRight(paintCntTagr);
				painter.drawRect(curRct);
				paintStr.setNum(nmLine);

				if (isSell) {
					painter.save();
					painter.setPen(m_drawHelper->m_penWhiteText);
					painter.setBrush(m_drawHelper->m_brushDarkPhone);
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


void uoReportCtrl::drawDataAreaResizeRuler(uoPainter& painter)
{

	// Надо прорисовать линию будующей границы ячейки
	if (m_stateMode == rmsResizeRule_Top || m_stateMode == rmsResizeRule_Left){
		QPen oldPen = painter.pen();
		uorPoint pnt1, pnt2;
		uorNumber xxx;
		if (m_stateMode == rmsResizeRule_Top) {
			xxx = qMax((uorNumber)m_curMouseCurPos.x(), m_curMouseSparesRect.left());

			pnt1.setY(m_rectDataRegion.top());
			pnt2.setY(m_rectDataRegion.bottom());
			pnt1.setX(xxx);
			pnt2.setX(xxx);

		} else {
			xxx = qMax((uorNumber)m_curMouseCurPos.y(), m_curMouseSparesRect.top());

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


void uoReportCtrl::drawSpliter(uoPainter& painter)
{
	if (m_fixationType == uorAF_None)
		return;
	uorPoint pt1 = m_fixationPoint, pt2 = m_fixationPoint;


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

void uoReportCtrl::drawCurentCell(uoPainter& painter)
{
	QRect rect = getCellRect(m_curentCell.y(), m_curentCell.x(), true);
	if (!rect.isEmpty()){
		int wp = m_drawHelper->m_penText.width();
		m_drawHelper->m_penText.setWidth(2);
		painter.setPen(m_drawHelper->m_penText);
		painter.drawRect(rect);
		m_drawHelper->m_penText.setWidth(wp);

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
	m_drawHelper->setDirectDraw(m_directDraw);
}
/// Отрисовка виджета.
void uoReportCtrl::drawWidget(uoPainter& painter)
{
	initDrawInstruments();
	drawHeaderControlContour(painter);
	if (m_dirtyDrawErea & uorDDE_Group)		drawHeaderControlGroup(painter);
	if (m_dirtyDrawErea & uorDDE_Ruler)		drawHeaderControl(painter);

	if (m_dirtyDrawErea & uorDDE_DA_Data)	{
		m_drawHelper->drawDataArea(painter, m_areaMain);
	}
	drawSpliter(painter);
}


/**
	Надо вычислить смещение между m_areaMainCopy и m_areaMain.
	если это смещение есть, можно просто скопировать отрисованную область
	в новую со смещением и дорисовать отсутствующие строики или столбцы...
*/
bool uoReportCtrl::getOffsetFromLastArea(uoRptHeaderType& rht, uorNumber& offset)
{
	bool retVal = false;
	rht = uorRhtUnknown;
	offset = uorNumberNull;
	return retVal;
}
/// Вычислим и сохраним состояние документа для оптимизации перерисовки.
void uoReportCtrl::checkRedrawOption()
{
	if ((
	m_cashDrawSellType 	== m_selections->selectionType() &&
	m_cashDrawSellType 	== uoRst_Unknown &&
	m_cashDrawMaVer 	== m_areaMain.changesVer() &&
	m_cashDrawDocVer 	== m_rptDoc->getChangeCount()
	&& m_drawToImage == 0)
	) {
		m_dirtyDrawErea = uorDDE_Ruler;
	} else {
		if (m_cashDrawMaVer == m_areaMain.changesVer() && m_cashDrawDocVer == m_rptDoc->getChangeCount() && m_drawToImage == 0)	{
			m_dirtyDrawErea = uorDDE_DA_Selection;
			m_dirtyDrawErea |= uorDDE_Ruler;
		} else {
			m_dirtyDrawErea = uorDDE_All;
		}

	}
	m_cashDrawSellType = m_selections->selectionType();
	m_cashDrawMaVer = m_areaMain.changesVer();
	m_cashDrawDocVer = m_rptDoc->getChangeCount();
	m_areaMainCopy = m_areaMain;
}

void uoReportCtrl::drawAfterDataErea(uoPainter& painter)
{
	drawDataAreaResizeRuler(painter);
	if (m_dirtyDrawErea & uorDDE_DA_Selection && m_selections->selectionType() != uoRst_Unknown) {
		m_drawHelper->drawDataAreaSelection(painter, m_areaMain);
	}

	drawCurentCell(painter);
}

/// Типа рисуем QTextEdit
void uoReportCtrl::paintEvent(QPaintEvent *event)
{
	m_paintLocker = 1;
	//int ttt = int(uorDDE_All);
	checkRedrawOption();
	if (m_lastSize != size() || m_lastSize.isEmpty()){
		m_dirtyDrawErea = uorDDE_All;
		m_lastSize = size();
		m_imageView 	= QImage(size(), QImage::Format_ARGB32_Premultiplied);
		m_imageViewCopy	= QImage(size(), QImage::Format_ARGB32_Premultiplied);
	} else {
	}
	bool isWindowsOS = false;
	#ifdef Q_OS_WIN
	isWindowsOS = true;
	#endif

	if (isWindowsOS && m_directDraw){
		m_dirtyDrawErea = uorDDE_All;
		uoPainter painter(this);
		painter.scale(m_scaleFactor, m_scaleFactor);
		painter.save();
		drawWidget(painter);
		painter.restore();
		drawAfterDataErea(painter);
	} else {
		if (m_dirtyDrawErea == uorDDE_All){
			m_imageView.fill(0);
			uoPainter painter(&m_imageView);
			painter.setRenderHint(QPainter::Antialiasing, false);
			painter.setRenderHint(QPainter::TextAntialiasing, false);
			painter.scale(m_scaleFactor, m_scaleFactor);
			painter.save();
			drawWidget(painter);
			painter.restore();
		} else if (m_dirtyDrawErea != uorDDE_Unknown){
			uoPainter painter(&m_imageView);
			painter.scale(m_scaleFactor, m_scaleFactor);
			painter.save();
			drawWidget(painter);
			painter.restore();
		}
		m_drawToImage = 0;
	//	}
		uoPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, false);
		painter.drawImage(0,0,m_imageView);
		if (m_dirtyDrawErea != uorDDE_All){
			if (m_dirtyDrawErea & uorDDE_Ruler)
				drawHeaderControl(painter);
			if (m_dirtyDrawErea & uorDDE_DA_Selection) {
				m_drawHelper->drawDataAreaSelection(painter, m_areaMain);
				m_dirtyDrawErea = m_dirtyDrawErea & ~uorDDE_DA_Selection; // Исключим, что-бы не рисовать заново.
			}
		}
		drawAfterDataErea(painter);

	}
	m_dirtyDrawErea = uorDDE_Unknown;
	m_paintLocker = 0;

}

/// Установить m_drawToImage в плюcовое значение и обновить виджет.
void uoReportCtrl::updateImage()
{
	if(m_drawToImage<0)
		m_drawToImage = 0;
	++m_drawToImage;
	if (m_paintLocker == 0)
		emit update();
}

/// Реакция на нажатие мышки-норушки на группах.
bool uoReportCtrl::mousePressEventForGroup(QMouseEvent *event, bool isDoubleClick){
	bool retVal = false;
	if (event->button() != Qt::LeftButton)
		return retVal;
	uorNumber posX = event->x();
	uorNumber posY = event->y();

	if (m_scaleFactor != 1.0){
		posX = posX * m_scaleFactorO;
		posY = posY * m_scaleFactorO;
	}

	if (m_rectGroupRow.contains(posX, posY) || m_rectGroupCol.contains(posX, posY))
	{
		retVal = true;
		uoReportDoc* doc = getDoc();
		if (doc) {
			event->accept();
			uoRptGroupItemList* groupItList = m_groupListH;

			uoRptHeaderType rht = uorRhtColumnHeader;
			if (m_rectGroupRow.contains(posX, posY)){
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

	uorNumber posX = event->x();
	uorNumber posY = event->y();

	if (m_scaleFactor != 1.0){
		posX = posX * m_scaleFactorO;
		posY = posY * m_scaleFactorO;
	}


	if (m_rectSectionCol.contains(posX, posY) || m_rectSectionRow.contains(posX, posY))
	{
		uoRptHeaderType rHt = uorRhtRowsHeader;
		uoRptSectionItem* sItem = NULL;
		uoRptSectionItem* sItemCur = NULL;
		uoRptSectionItemList* sList = m_sectItemListV;
		uoRptSectionItemList* sListOther = m_sectItemListH;
		if (m_rectSectionCol.contains(posX, posY)) {
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
bool uoReportCtrl::findScaleLocation(uorNumber posX, uorNumber posY, int &scaleNo, uoRptHeaderType rht)
{
	bool retVal = false;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return retVal;

	int cntScale = 0;
	uorNumber endPos = uorNumberNull;
	uorNumber stratPos = uorNumberNull;
	uorNumber scSize = uorNumberNull;

	if (rht == uorRhtRowsHeader){
		cntScale = m_areaMain.firstVisible_RowTop();
		endPos = posY;
		stratPos = m_rectDataRegion.top() - m_areaMain.shift_RowTop();
		m_curMouseSparesRect.setLeft(m_rectRulerRow.left());
		m_curMouseSparesRect.setRight(m_rectRulerRow.right());
	} else if (rht == uorRhtColumnHeader) {
		cntScale = m_areaMain.firstVisible_ColLeft();
		endPos = posX;
		stratPos = m_rectDataRegion.left() - m_areaMain.shift_ColLeft();
		m_curMouseSparesRect.setTop(m_rectRulerCol.top());
		m_curMouseSparesRect.setBottom(m_rectRulerCol.bottom());
	}

	scaleNo = cntScale;
	stratPos = stratPos; // * m_scaleFactorO;
	while (stratPos < endPos){
		scSize = uorNumberNull;
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
uorSparesType uoReportCtrl::findPointLocation(uorNumber posX, uorNumber posY)
{
	uorSparesType rst = uoVst_Unknown;
	if (m_showGroup){
		if (m_rectGroupRow.contains(posX, posY)){
			rst = uoVst_GroupV;
		} else if (m_rectGroupCol.contains(posX, posY)) {
			rst = uoVst_GroupH;
		}
	}
	if (rst == uoVst_Unknown && m_showRuler) {
		if (m_rectRuleCorner.contains(posX, posY)){
			rst = uoVst_ScaleVH;
		} else if (m_rectRulerCol.contains(posX, posY)) {
			rst = uoVst_ScaleH;
		} else if (m_rectRulerRow.contains(posX, posY)) {
			rst = uoVst_ScaleV;
		}
	}

	if (rst == uoVst_Unknown && m_showSection) {
		if (m_rectSectionCol.contains(posX, posY)){
			rst = uoVst_SectionH;
		} else if (m_rectSectionRow.contains(posX, posY)){
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
	uorZeroRectF(m_curMouseSparesRect);
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
uorBorderLocType uoReportCtrl::scaleLocationInBorder(uorNumber pos, uorRect rect, uoRptHeaderType rht)
{
	uorBorderLocType locType = uoBlt_Unknown;
	uorNumber dragSize = (uorNumber)UORPT_DRAG_AREA_SIZE;
	// надо еще величину границы захвата определить.
	uorNumber posStart, posEnd, interVal = dragSize * (uorNumber)2;

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

		posStart 	= rect.left() - (uorNumber)UORPT_DRAG_AREA_SIZE;
		posEnd 		= rect.left() + (uorNumber)UORPT_DRAG_AREA_SIZE;
		if (posStart<=pos && pos<= posEnd)
			return uoBlt_Left;

		posStart 	= rect.right() - (uorNumber)UORPT_DRAG_AREA_SIZE;
		posEnd 		= rect.right() + (uorNumber)UORPT_DRAG_AREA_SIZE;
		if (posStart<=pos && pos<= posEnd)
			return uoBlt_Right;
	}
	return locType;
}

/// Клик мышки-норушки по области с данными.
bool uoReportCtrl::mousePressEventForDataArea(QMouseEvent *event, bool isDoubleClick)
{
	bool retVal = false;


	uorNumber posX = event->x(), posY = event->y();
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
	int curRow = pntCell.y();
	int curCol = pntCell.x();
	if (pntCell.isNull())
		return retVal;
	bool isSell = m_selections->isCellSelect(pntCell.y(), pntCell.x());

	if ((event->button() == Qt::LeftButton) && m_selections->isShiftPress())
	{
		if (m_selections->cellStartIsNull())
			m_selections->cellSelectedStart(m_curentCell.y(), m_curentCell.x());
		m_selections->cellSelectedMidle(curCol, curRow);
		updateThis();

	} else if (event->button() == Qt::LeftButton || !isSell){

//		if (m_selections->sta)
		if (!isSell){
			m_selections->clearSelections(uoRst_Cells);
			clearSelectionsSection();
		}

		setCurentCell(curCol, curRow, true);
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
	uorNumber posX = event->x(), posY = event->y();
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
	if (!(m_rectRulerCol.contains(posX, posY) || m_rectRulerRow.contains(posX, posY))) {
		return retVal;
	}
	uoRptHeaderType rhtCur = uorRhtRowsHeader;
	if (m_rectRulerCol.contains(posX, posY))
		rhtCur = uorRhtColumnHeader;

	int scaleNo = 0;
	bool scHide = true;
	uorNumber scSize = uorNumberNull;

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
				setCurentCell(scaleNo, 0);
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
					uorNumber topRct = uorNumberNull;

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
				setCurentCell(0, scaleNo);
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
					uorNumber topRct = uorNumberNull;

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
	if (m_freezUpdate==0 && m_paintLocker == 0){
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
				uorSelectionType  selType = m_selections->selectionType();
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
	uorSelectionType  selType = m_selections->selectionType();
	if (selType == uoRst_Unknown){
		curCell = doc->getCell(m_curentCell.y(), m_curentCell.x(), processor->m_needCreate);
		if (curCell){
			curCell = doc->getFirstUnionCell(curCell, m_curentCell.y());
			if (!curCell->skipVisitor())
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
						///\todo - нужно скипануть облать объединения, если она есть, кроме первой ячейки...
						if (processor->m_needCreate && selType != uoRst_Document){
							curCell->provideAllProps(doc, true);
						}

						if (!curCell->skipVisitor())
						{
//							qDebug() << "processor->visit: col "<< col << " row "<< row;
							curCell = doc->getFirstUnionCell(curCell, row);
							processor->visit(curCell, firstOne);
							if (firstOne)
								firstOne = !firstOne;
						}
					}
				}
			}
		}
		if (processor->m_type == uorSVT_Setter){
			doc->doFormatDocWithSelection(m_selections);
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
		propEditor->m_cellText = cell->text();
		propEditor->m_cellDecode = cell->m_textDecode;
	} else {
		propEditor->m_cellText = "";
		propEditor->m_cellDecode = "";
	}


	recalcSelectionProperty();
	propEditor->m_textProp->copyFrom(&m_textDec_Selection);
	propEditor->m_borderProp->copyFrom(&m_borderProp_Selection);
	propEditor->m_sellectonType = m_selections->selectionType();
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
	uorRect rct;
	int line = 0;
	bool needUpdate = false;

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;

	bool visProp = propertyEditorVisible();

	uorNumber posX = 0,posY = 0;
	posX = event->x();
	posY = event->y();

	if (m_scaleFactor != uorNumberNull) {
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
					setCurentCell(0,line);
					needUpdate = true;
				}
			}
			if (rmsSelectionRule_Top == m_stateMode) {
				if (findScaleLocation(posX, posY, line,uorRhtColumnHeader)) {
					m_selections->colSelectedEnd(line);
					setCurentCell(line, 0);
					needUpdate = true;
				}
			}
		} else if (m_stateMode == rmsResizeRule_Left || m_stateMode == rmsResizeRule_Top) {
			needUpdate = true;
			int distance = (m_curMouseLastPos - event->pos()).manhattanLength();
			if (distance > QApplication::startDragDistance()) {

				uorNumber delta = 0;
				uoRptHeaderType rht = (m_stateMode == rmsResizeRule_Left) ? uorRhtRowsHeader : uorRhtColumnHeader;
				if (m_stateMode == rmsResizeRule_Left) {
					delta = posY - m_curMouseSparesRect.bottom();
				} else {
					delta = posX - m_curMouseSparesRect.right();
				}
				uorNumber newSize = doc->getScaleSize(rht, m_resizeLine);
				newSize = newSize + delta;
				newSize = qMax(uorNumberNull, newSize);
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

			pntAdd.setY(qMax(pntAdd.y(), int(m_rectDataRegion.top()-m_areaMain.shift_RowTop())));
			pntAdd.setY(qMin(pntAdd.y(), int(m_rectDataRegion.bottom())));

			pntAdd.setX(qMax(pntAdd.x(), int(m_rectDataRegion.left()-m_areaMain.shift_ColLeft())));
			pntAdd.setX(qMin(pntAdd.x(), int(m_rectDataRegion.right())));

			QPoint pntCell = getCellFromPosition(pntAdd.y(), pntAdd.x());
			int curRow = pntCell.y();
			int curCol = pntCell.x();

			if (!pntCell.isNull()){
				m_selections->cellSelectedEnd(curCol, curRow);
				needUpdate = true;
				setCurentCell(curCol, curRow);
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

	if (m_scaleFactor != uorNumberNull) {
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
		uorPoint pos;
		bool vertLoc = false;
		uoRptHeaderType rhdType = uorRhtUnknown;
		uorNumber posX = event->x(), posY = event->y();

		if (m_scaleFactor != uorNumberNull) {
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
				if (m_rectRulerCol.contains(pos) || (vertLoc = m_rectRulerRow.contains(pos))){

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

/// Рукопашная обработка шорткатов, ибо QShortcut - срабатываю в ненужных местах.....
void uoReportCtrl::keyPressEventShortcut ( QKeyEvent * event )
{
	if (m_iteractView->shortkatUse())
		return;
	int key = event->key();
	Qt::KeyboardModifiers  kbrdMod = qApp->keyboardModifiers();
	bool ctrlPresed = (kbrdMod & Qt::ControlModifier) ? true : false;
	bool shiftPresed = (kbrdMod & Qt::ShiftModifier) ? true : false;
	switch (key) {
		case Qt::Key_Insert: {
			if (ctrlPresed && !shiftPresed){
				onCopy();		event->accept();
			} else 	if (shiftPresed){
				onPaste();		event->accept();
			}
			break;
		}
		case Qt::Key_V:{
			if (ctrlPresed){
				onPaste();		event->accept();
			}
			break;
		}
		case Qt::Key_C:{
			if (ctrlPresed){
				onCopy();		event->accept();
			}
			break;
		}

		default:
		{
			break;
		}
	}
	if (!event->isAccepted()){
		if 	(event->matches(QKeySequence::Save)){
			onSave();	event->accept();
		}
		else if (event->matches(QKeySequence::Copy)){
			onCopy();	event->accept();
		}
		else if (event->matches(QKeySequence::Paste)){
			onPaste();	event->accept();
		}
	}
	// !!!! Не ловит Qt Ctrl+v, Ctrl+с

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
	bool itemJoined = false;
	bool curJoinRectOk = false;
	QRect curJoinRect = doc->getCellJoinRect(m_curentCell);
	if (curJoinRect.top() != 0){
		curJoinRectOk = true;
	}

	//	научим курсор прыгать через скрытые ячейки.... но ни через границу поля...
	switch (key)	{
		case Qt::Key_Down:	{
			do	{
				posY += 1;
				itemHiden = doc->getScaleHide(uorRhtRowsHeader, posY);
				itemJoined = false;
				if (curJoinRectOk && curJoinRect.contains(posX, posY))
					itemJoined = true;

			}while(itemHiden || itemJoined);
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

				itemJoined = false;
				if (curJoinRectOk && curJoinRect.contains(posX, posY))
					itemJoined = true;

			} while(itemHiden || itemJoined);

			if (!(posY >= 1))
				posY = m_curentCell.y();
			break;
		}
		case Qt::Key_Right: {
			//while(itemHiden = doc->getScaleHide(uorRhtColumnHeader, ++posX)){}
			//break;
			do	{
				posX += 1;
				itemHiden = doc->getScaleHide(uorRhtColumnHeader, posX);
				itemJoined = false;
				if (curJoinRectOk && curJoinRect.contains(posX, posY))
					itemJoined = true;

			}while(itemHiden || itemJoined);
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
				itemJoined = false;
				if (curJoinRectOk && curJoinRect.contains(posX, posY))
					itemJoined = true;


			} while(itemHiden || itemJoined);

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
			if(key == Qt::Key_PageUp && m_areaMain.firstVisible_RowTop() == 1) {
				posY = 1;
			} else if (key == Qt::Key_PageDown && posY < m_areaMain.lastVisibleRow()) {
				posY = m_areaMain.lastVisibleRow();
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

				uorNumber pageHeight = m_rowsInPage * doc->getDefScaleSize(uorRhtRowsHeader);
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
					if (pageHeight < uorNumberNull)
						break;
				} while(true);
				if (curRow <= 0)
					curRow = 1;
				posY = curRow;

			}
			break;
		}
		default: {
			break;
		}
	}
	if (posX != m_curentCell.x() || posY != m_curentCell.y())	{
		// если мы попали на объединенные ячейки, надо спозиционироваться на левой верхней.
		// пока отложу, ибо с нафигацией лажа...
		uoCell* curCell = doc->getCell(posY, posX);
		if (curCell){
			if (curCell->isUnionHas()){
				QPoint pt = curCell->getFirstUnionCellPoint(posY);
				posX = pt.x();
				posY = pt.y();
			}
		}

	}

	uorSelectionType selType = m_selections->selectionType();
	uorSelectionType selStartType = m_selections->getStartSelectionType();
	bool procSelCell = true;
	if (selType == uoRst_Column || selType == uoRst_Columns || selType == uoRst_Row || selType == uoRst_Rows){
//		int lastCR = m_selections->
		if (shiftPresed) {
			procSelCell = false;
			if (selStartType == uoRst_Column || selStartType == uoRst_Columns) {
				m_selections->colSelectedMidle(posX);
			} else if (selStartType == uoRst_Row || selStartType == uoRst_Rows) {
				m_selections->rowSelectedMidle(posY);
			}
		}
	}

	if (procSelCell) {
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
		}
	}
	if (posX != m_curentCell.x() || posY != m_curentCell.y()){
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

	event->setAccepted(false);
	bool visProp = propertyEditorVisible();
	if (visProp){
		propertyEditorApply();
	}

	keyPressEventShortcut ( event );
	if (event->isAccepted())
		return;


	int key = event->key();
	QString str;

	--m_freezUpdate;
	//event->accept();
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
			event->accept();
			keyPressEventMoveCursor ( event );
			break;
		}
		case Qt::Key_Return:
		case Qt::Key_Enter:
		case Qt::Key_F2:
		{
			event->accept();
			if (doCellEditTextStart(str))
				setStateMode(rmsEditCell);
			break;
		}
		case Qt::Key_Escape:
		{
			event->accept();
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
			event->accept();
		}
	}


	++m_freezUpdate;
	if (event->isAccepted())
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
//	m_imageView = QImage(size(), QImage::Format_ARGB32_Premultiplied);
	const QSize oldSizeW =  event->oldSize();
	const QSize sizeW =  event->size();
	m_drawToImage = 1;
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

	QMenu *menuEdit 	= targMenu->addMenu(QString::fromUtf8("Редактирование"));
	menuEdit->addAction(m_iteractView->m_actCopy);
	menuEdit->addAction(m_iteractView->m_actCut);
	menuEdit->addAction(m_iteractView->m_actPaste);

	bool actEnable = false;
	uoCell* curCell = doc->getCell(m_curentCell.y(), m_curentCell.x());
	if (curCell && curCell->isUnionHas()) {
		menuEdit->addAction(m_iteractView->m_actUnJoin);
		m_iteractView->m_actUnJoin->setEnabled(true);
	} else {
		actEnable = m_selections->isTrueForJoin();
		menuEdit->addAction(m_iteractView->m_actJoin);
		m_iteractView->m_actJoin->setEnabled(true);
	}


	bool actPasteEnable = false;

	QClipboard* clipBrd = qApp->clipboard();
	if (clipBrd){
		const QMimeData* mime = clipBrd->mimeData();
		if (mime){
			if (mime->hasFormat(UOR_MIME_XML_DATA))	{
				actPasteEnable = true;
			}
			if (mime->hasFormat("text/csv") || mime->hasFormat("text/plain"))	{
				actPasteEnable = true;
			}
		}
	}
	m_iteractView->m_actPaste->setEnabled(actPasteEnable);

	QMenu *menuShow 	= targMenu->addMenu(QString::fromUtf8("Скрыть / показать"));


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

	if (m_sPanel->isVisible()) 	menuShow->addAction(m_iteractView->m_actSheetPanHide);
	else 					menuShow->addAction(m_iteractView->m_actSheetPanShow);


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

	menuSpecial->addAction(m_iteractView->m_actCreateMatrix);
	menuSpecial->addAction(m_iteractView->m_actLoad_TXT);
	menuSpecial->addSeparator();
	menuSpecial->addAction(m_iteractView->m_actOptions);

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
	uorNumber inputSize = uorNumberNull;
	uorNumber rezInput = uorNumberNull;
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
					inputSize = rezInput = uorNumberNull;
					break;
				}
			}
			++cntr;
			it++;
		}

		rezInput = (uorNumber) QInputDialog::getDouble(this, QString::fromUtf8("Введите размер")
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
		doc->doFormatDocWithSelection(m_selections);
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

void uoReportCtrl::onLoadTXT()
{
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;

	QProgressDialog* progresDlg = uoReport::uoReportManager::instance()->progressDlg();
	if (!progresDlg){
		qWarning() << "Error!!!";
		return;
	}


	QString docFilePath;
	if (!m_iteractView->chooseLoadTxtFilePath(docFilePath, this)){
		return;
	}
	QFile file(docFilePath);
	long sz = file.size();
	long sz_add = 0;

	docFilePath = file.fileName();


	progresDlg->setLabelText(QObject::tr("Load %1").arg(docFilePath));
	progresDlg->setRange(0,sz);
	progresDlg->setModal(true);

	bool oldCC = doc->enableCollectChanges(false);
	bool oldDF = doc->enableFormating(false);
	doc->clear();

	if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
		QTextStream qts_in(&file);
		QChar ch = '\t';
		QStringList list;
		QString line, linePart;
		int row = 1;
		while (!qts_in.atEnd()) {
			QString line = qts_in.readLine();
			sz_add += line.length();
			list = line.split(ch);
			for (int y=1; y<=list.size(); y++){
				linePart = list.at(y-1);
				doc->setCellText(row,y,linePart);
				doc->setCellTextAlignment(row,y,uoReport::uoVA_Center, uoReport::uoHA_Left, uoReport::uoCTB_Transfer);
			}

			++row;
			progresDlg->setValue(sz_add);
		}
	}
	qDebug() << "lod file: " << docFilePath << " size: " << sz << " size for row: " << sz_add;

	doc->enableFormating(oldDF);
	progresDlg->setLabelText(QObject::tr("Formated %1").arg(docFilePath));

	doc->doFormatDoc();
	progresDlg->hide();
	recalcHeadersRects();
	setStateMode(rmsNone);
	doc->enableCollectChanges(oldCC);
	updateImage();
	setFocus();

}

/**
	Вывод диалога с дополнительными опциями...
*/
void uoReportCtrl::onOptionsShow()
{
	uorOptionsDlg* dlg = new uorOptionsDlg(this);

	uoReportDoc* doc = getDoc();

	bool saveWithUndoStack = false;
	if (doc)
		saveWithUndoStack = doc->isSaveUndoStack();

	dlg->setSaveWithSelection(m_saveWithSelection);
	QString toDebug = m_areaMain.toDebug();
	toDebug = toDebug + QString("\n curr cell col = %1 row = %2").arg(m_curentCell.x()).arg(m_curentCell.y());

	QClipboard* clipBrd = qApp->clipboard();
	if (clipBrd){
		const QMimeData* mime = clipBrd->mimeData();
		if (mime){
			toDebug =toDebug + QString("\n QClipboard formats: ") + mime->formats().join(" / ");
		}
	}


	dlg->setDebugString(toDebug);
	dlg->setDirectDrawFlag(m_directDraw);
	dlg->setSaveWithUndoStack(saveWithUndoStack);
	bool oldDirectDraw = m_directDraw;

	int dret = dlg->exec();
	if (dret==1){
		m_saveWithSelection = dlg->saveWithSelection();
		if (doc) {
			doc->setSaveUndoStack(dlg->isSaveWithUndoStack());
		}
		m_directDraw = dlg->directDraw();
	}
	delete dlg;
	if (oldDirectDraw != m_directDraw)
		emit update();
}

void uoReportCtrl::onCellJoin()
{
	if (!m_selections->isTrueForJoin())
		return;
	QRect rect = m_selections->getSelectionBound();
	uoReportDoc* doc = getDoc();
	if (doc) {
		if (doc->joinCells(rect)){
			setCurentCell(m_curentCell.x(),m_curentCell.y()); // поправит текущую ячейку, если надо..
			doc->doFormatDoc(m_curentCell.y(), m_curentCell.x());
			updateImage();
		}
	}
}

void uoReportCtrl::onCellUnJoin()
{
	uoReportDoc* doc = getDoc();
	if (doc) {
		uoCell* curCell = doc->getCell(m_curentCell.y(), m_curentCell.x());
		if (curCell && curCell->isUnionHas()) {
			QRect rect = curCell->getCellJoinRect();
			if (doc->joinCells(rect, false)){
				setCurentCell(m_curentCell.x(),m_curentCell.y()); // поправит текущую ячейку, если надо..
				doc->doFormatDoc(m_curentCell.y(), m_curentCell.x());
				updateImage();
			}
		}
	}
}


void uoReportCtrl::onCreateMatrix()
{
	uoReportDoc* doc = getDoc();
	if (!doc)
		return;
	int all_val = 0, row_val = 0, col_val = 0;
	int lenghtAll = 0;
	bool ok;

	QProgressDialog* progresDlg = uoReport::uoReportManager::instance()->progressDlg();
	if (!progresDlg){
		qWarning() << "Error!!!";
		return;
	}

	row_val = (int) QInputDialog::getInteger( this,
	QString::fromUtf8("Сколько строк"),
	QString::fromUtf8("строк:"), 500, 1, 1000000, 50, &ok);

	if (!ok)
		return;
	col_val = (int) QInputDialog::getInteger( this,
	QString::fromUtf8("Сколько колонок"),
	QString::fromUtf8("колонок:"), 100, 1, 1000, 50, &ok);

	if (!ok)
		return;
	all_val = row_val * col_val;
	bool oldCC = doc->enableCollectChanges(false);
	bool oldDF = doc->enableFormating(false);
	doc->clear();

	QString addTextToCell = QInputDialog::getText(this, tr("Input additional text"),tr("text:"), QLineEdit::Normal,QString("text-text-text"), &ok);
	if (!ok)
		return;



	bool wasCanceled = false;
	progresDlg->setLabelText(QObject::tr("Load matrix %1x%2").arg(row_val).arg(col_val));
	progresDlg->setRange(0,row_val);
	progresDlg->setModal(true);

	QString cellSrt;

	for(int nnn = 1; nnn<=row_val; nnn++){
		for(int mm = 1; mm<=col_val; mm++) {
			all_val = nnn*mm;
			if (addTextToCell.isEmpty()) {
				cellSrt = QString("%1").arg(all_val);
			} else {
				cellSrt = QString("%1 + %2").arg(all_val).arg(addTextToCell);

			}
			lenghtAll += cellSrt.length() * 2;
			doc->setCellText(nnn,mm,cellSrt);
		}
		if (nnn%50==0)
			progresDlg->setLabelText(QObject::tr("Load matrix %1x%2 ->%3").arg(row_val).arg(col_val).arg(nnn));
		progresDlg->setValue(nnn);
		wasCanceled = progresDlg->wasCanceled();
		if (wasCanceled)
			break;
	}
	cellSrt = QString("lenghtAll = %1").arg(lenghtAll);
	doc->setCellText(row_val+1,1,cellSrt);

	doc->enableCollectChanges(oldCC);
	doc->enableFormating(oldDF);

	progresDlg->hide();
	updateImage();
	setFocus();

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
void uoReportCtrl::onSetScaleFactor(const uorNumber sFactor){
	if (sFactor != m_scaleFactor) {
		m_scaleFactor = sFactor;
	    m_scaleFactorO = 1 / m_scaleFactor;

		recalcHeadersRects();
		recalcScrollBars();
		updateImage();
	}
}

/// Видима ли строка
bool uoReportCtrl::rowVisible(const int& nmRow) const
{
	if (m_areaMain.firstVisible_RowTop()<=nmRow && m_areaMain.lastVisibleRow() >= nmRow) {
		return true;
	}
	return false;
}

/// Видим ли солбец
bool uoReportCtrl::colVisible(const int& nmCol) const
{
	if (m_areaMain.firstVisible_ColLeft() <= nmCol && m_areaMain.lastVisibleCol() >= nmCol){
		return true;
	}
	return false;
}


/// Ширина с учетом масштаба, и толщины скрола
uorNumber uoReportCtrl::getWidhtWidget() {
	return (width() /* - m_vScrollCtrl->width()*/)* m_scaleFactorO;
}
/// Высота  с учетом масштаба, и толщины скрола
uorNumber uoReportCtrl::getHeightWidget(){
	return (height()/* - m_hScrollCtrl->height() */) * m_scaleFactorO;
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
				uorNumber maxLength = uorNumberNull, heightText = uorNumberNull;
				for (int i = 0; i < list.size(); ++i){
					textCell = list.at(i);
					maxLength = qMax(maxLength, (uorNumber)fm.width(textCell));
				}
				heightText = (uorNumber)(fm.height() * (list.size()+1));
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
		QRect rct = getCellRect(m_curentCell.y(), m_curentCell.x(), true);
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
		updateImage();
	}

}

///Запись документа.
bool uoReportCtrl::saveDoc(){
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return false;

	if (doc->saveOptionsIsValid()){
		if (m_saveWithSelection) {
			return doc->save(m_selections);
		} else {
			return doc->save();
		}

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
		if (m_saveWithSelection) {
			return doc->save(m_selections);
		} else {
			return doc->save();
		}
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
		bool oldCC = doc->enableCollectChanges(false);
		bool oldDF = doc->enableFormating(false);

		doc->clear();
		doc->setStoreOptions(docFilePath, storeFormat);
		doc->load();
		doc->enableFormating(oldDF);
		doc->doFormatDoc();
		recalcHeadersRects();
		setStateMode(rmsNone);
		updateImage();
		setFocus();
		doc->enableCollectChanges(oldCC);
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
	if (m_areaMain.firstVisible_RowTop() == yC || yC<0) {
		return;
	}
	m_areaMain.setFirstVisible_RowTop(yC);
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
	m_areaMain.setFirstVisible_ColLeft(xC);
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
				m_areaMain.setFirstVisible_RowTop(m_vScrollCtrl->sliderPosition());
				m_areaMain.setShift_RowTop(uorNumberNull);
			} else {
				m_areaMain.setFirstVisible_ColLeft(m_hScrollCtrl->sliderPosition());
				m_areaMain.setShift_ColLeft(uorNumberNull);
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
			pos = m_areaMain.firstVisible_ColLeft() + dx;
			while(itemHiden = doc->getScaleHide(uorRhtColumnHeader, pos)){
				++pos;
			}
			m_areaMain.setFirstVisible_ColLeft(pos);
			m_areaMain.setShift_ColLeft(uorNumberNull);
		} else if (dx < 0){
			pos = m_areaMain.firstVisible_ColLeft() + dx;
			if (pos > 0) {
				while(itemHiden = doc->getScaleHide(uorRhtColumnHeader, pos) && (pos >= 1)){
					--pos;
				}
			} else {
				pos = 1;
			}
			pos = qMax(1, pos);

			m_areaMain.setFirstVisible_ColLeft(pos);
			m_areaMain.setShift_ColLeft(uorNumberNull);
		}
		onAccessRowOrCol(m_areaMain.firstVisible_ColLeft()/* + m_rowsInPage*/, uorRhtColumnHeader);
	}

	if (dy != 0) {
		if (dy > 0) {
			pos = m_areaMain.firstVisible_RowTop() + dy;
			while(itemHiden = doc->getScaleHide(uorRhtRowsHeader, pos)){
				++pos;
			}
			m_areaMain.setFirstVisible_RowTop(pos);
			m_areaMain.setShift_RowTop(uorNumberNull);
		} else if (dy < 0){
			pos = m_areaMain.firstVisible_RowTop() + dy;
			if (pos > 0) {
				while(itemHiden = doc->getScaleHide(uorRhtRowsHeader, pos) && (pos > 1)){
					--pos;
				}
			} else {
				pos = 1;
			}

			m_areaMain.setFirstVisible_RowTop(pos);
			m_areaMain.setShift_RowTop(uorNumberNull);
		}
		onAccessRowOrCol(m_areaMain.firstVisible_RowTop()/* + m_rowsInPage*/, uorRhtRowsHeader);

	}
	recalcHeadersRects();
	updateThis();
}

// тут получаем комманды от меню
void uoReportCtrl::onCopy()
{
	uoReportDoc* doc = getDoc();
	if (!doc)
		return;
	doc->onCopy(m_selections);

}
void uoReportCtrl::onPaste()
{
	uoReportDoc* doc = getDoc();
	if (!doc)
		return;
	doc->onPaste(m_selections);
	recalcHeadersRects();
	updateThis();

}
void uoReportCtrl::onCut()
{
	uoReportDoc* doc = getDoc();
	if (!doc)
		return;
	doc->onCut(m_selections);
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
	m_fixationPoint.setX(uorNumberNull);
	m_fixationPoint.setY(uorNumberNull);

	return;

	uoReportDoc* doc = getDoc();
	if (!doc)
		return;

	// надо рассчитать точку фиксации вьюва
	int i = 0;
	uorNumber tmpSize = uorNumberNull;
	if (m_fixationType == uorAF_Rows || m_fixationType == uorAF_RowsAndCols)
	{
		tmpSize = -m_areaMain.shift_RowTop();
		i = m_areaMain.firstVisible_RowTop();
		for (; i<=m_fixedRow; i++) {
			if (!doc->getScaleHide(uorRhtRowsHeader, i)){
				tmpSize += doc->getScaleSize(uorRhtRowsHeader, i);
			}
		}
		m_fixationPoint.setY(tmpSize);
	}

	if (m_fixationType == uorAF_Cols || m_fixationType == uorAF_RowsAndCols)
	{
		tmpSize = -m_areaMain.shift_ColLeft();
		i = m_areaMain.firstVisible_ColLeft();
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

		area->setFirstVisible_ColLeft(m_areaMain.firstVisible_ColLeft());
		area->setFirstVisible_RowTop(m_areaMain.firstVisible_RowTop());
		area->setShift_RowTop(m_areaMain.shift_RowTop());
		area->setShift_ColLeft(m_areaMain.shift_ColLeft());
		area->setLastVisibleRow(m_fixedRow);
		area->setLastVisibleCol(m_fixedCol);
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
	m_fixationPoint.setX(uorNumberNull);
	m_fixationPoint.setY(uorNumberNull);

	m_fixedCol = cols;
	m_fixedRow = rows;
	if (m_fixationType == uorAF_None)
		return retVal;

	recalcFixationPointStart();

	return retVal;
}

/// получить рекст ячейки по строке/ячейке, а возможно и объединенный рект.
QRect uoReportCtrl::getCellRect(const int& rowNo, const int& colNo, bool ifJoinGetFull/* = false */)
{
	QRect rect;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return rect;
	bool hasJoin = false;
	uoCell* cell = 0;
	QRect joinRect;
	if (ifJoinGetFull){
		hasJoin = false;
		cell = doc->getCell(rowNo, colNo, false);
		if (cell) {
			if (cell->isUnionHas()){
				joinRect = cell->getCellJoinRect();
				hasJoin = true;
			}
		}
	}

	if (m_scaleStartPositionMapV.contains(rowNo) && m_scaleStartPositionMapH.contains(colNo)) {
		rect.setTop((int)(m_scaleStartPositionMapV[rowNo] * m_scaleFactor));
		rect.setLeft((int)(m_scaleStartPositionMapH[colNo] * m_scaleFactor));
		if (ifJoinGetFull && cell && hasJoin){
			rect.setHeight((int)(doc->getScalesSize(uorRhtRowsHeader,rowNo, rowNo + joinRect.height() - 1) * m_scaleFactor));
			rect.setWidth((int)(doc->getScalesSize(uorRhtColumnHeader,colNo,colNo + joinRect.width() - 1) * m_scaleFactor));
		} else {
			rect.setHeight((int)(doc->getScaleSize(uorRhtRowsHeader,rowNo) * m_scaleFactor));
			rect.setWidth((int)(doc->getScaleSize(uorRhtColumnHeader,colNo) * m_scaleFactor));
		}
	}
	return rect;
}

/// Получить ячейку по локальным экранным координатам.
QPoint uoReportCtrl::getCellFromPosition(const uorNumber& posY, const uorNumber& posX)
{
	QPoint cell;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return cell;

	bool isHide = false;

	int rowCur = m_areaMain.firstVisible_RowTop();
	int colCur = m_areaMain.firstVisible_ColLeft();

	uorNumber rowsLenCur = m_rectDataRegion.top() - m_areaMain.shift_RowTop();
	uorNumber colsLenCur = m_rectDataRegion.left() - m_areaMain.shift_ColLeft();

	uorNumber sz = uorNumberNull;
	do {	// строки, строки, строки и строки =============
		while((isHide = doc->getScaleHide(uorRhtRowsHeader, rowCur))){
			++rowCur;
		}
		sz = doc->getScaleSize(uorRhtRowsHeader, rowCur);
		if (sz == uorNumberNull) {
			++rowCur;
			continue;
		}
		if (posY>= rowsLenCur && posY <= (rowsLenCur + sz)) {
			cell.setY(rowCur);
			// тут поищем колонку.
			do {

				sz = doc->getScaleSize(uorRhtColumnHeader, colCur);
				if (sz == uorNumberNull) {
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
	m_curentCell.x() < m_areaMain.firstVisible_ColLeft() ||
	m_curentCell.x() > m_areaMain.lastVisibleCol() ||
	m_curentCell.y() < m_areaMain.firstVisible_RowTop() ||
	m_curentCell.y() > m_areaMain.lastVisibleRow()
	) {
		return false;
	} else {
		///\todo надо еще проверить то, что строка или колонка не являются скрытими...
		if (doc->getScaleHide(uorRhtRowsHeader,m_curentCell.y()) || doc->getScaleHide(uorRhtColumnHeader,m_curentCell.x()))
			return false;
		return true;
	}
}

/**
	проверим текущую ячейку, если она в объединении и не первая -
	прийдется откорректировать и установить её первую в объединении.
*/
void uoReportCtrl::checkCurentCell(int& col, int& row, int& colsJn, int& rowsJn)
{
	colsJn = rowsJn = 0;
	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;
	uoCell* curCell = doc->getCell(row, col);
	if (curCell){
		if (curCell->isUnionHas()){
			QPoint pt = curCell->getFirstUnionCellPoint(row);
			col = pt.x();
			row = pt.y();
			curCell = doc->getCell(row, col);
			if (curCell && curCell->m_ceelJoin){
				QRect rctVal = curCell->m_ceelJoin->m_cellRect;
				colsJn = rctVal.width();
				rowsJn = rctVal.height();
			}
		}
	}

}

/// Установить текушую ячейку с/без гарантии видимости
/// Перемещаем курсор на ячейку, если ячейка не видима, делаем её видимой.
void uoReportCtrl::setCurentCell(const int& colTmp, const int& rowTmp, bool ensureVisible)
{
	const QPoint oldPoint = m_curentCell;
	int col = colTmp;
	int row = rowTmp;
	int colsJn = 0, rowsJn = 0;
	checkCurentCell(col, row, colsJn, rowsJn);
	if (m_selections)
		m_selections->setCurrentCell(row,col, rowsJn, colsJn);

	uoReportDoc* doc =  getDoc();
	if (!doc)
		return;

	if (m_sPanel){
		if (rowsJn>1 || colsJn>1){
			m_sPanel->setAdress(QString("R%1:C%2 + R%3xC%4").arg(row).arg(col).arg(rowsJn).arg(colsJn));
		} else {
			m_sPanel->setAdress(QString("R%1:C%2").arg(row).arg(col));
		}
		m_sPanel->setText(doc->getCellText(row,col, uorCTT_Text));
	}

	if (col<=0 || row <=0) {
		if (col>0) {
			m_curentCell.setX(col);
		}
		if (row>0) {
			m_curentCell.setY(row);
		}
		return;
	}

	const QPoint& newPoint = QPoint(col,row);


	// Что есть по позиционированию? Какие данные?
	bool itemHide = false;
	uoSideType moveTo = uost_Unknown;
	int oldX = m_curentCell.x(),  oldY = m_curentCell.y();
	if (m_curentCell.y() != row) {
		m_curentCell.setY(row);
		int ereaVer = m_areaMain.changesVer();
		if (ensureVisible){
			// гарантировать видимость ячейки. у-у-у-у!!!
			moveTo = uost_Bottom;
			if (oldY>row)
				moveTo = uost_Top;
			if (moveTo == uost_Top && m_areaMain.firstVisible_RowTop() >= row){
				m_areaMain.setFirstVisible_RowTop(row);
				m_areaMain.setShift_RowTop(0);
				m_areaMain.setLastVisibleRow(recalcVisibleScales(uorRhtRowsHeader));

			}
			else if (moveTo == uost_Bottom && ( m_areaMain.lastVisibleRow()) <= row)	{
				// Надо высчитать m_firstVisible_RowTop и m_shift_RowTop
				uorNumber sizeVAll = m_rectDataRegionFrame.height();
				uorNumber sizeItem = uorNumberNull;
				m_areaMain.setShift_RowTop(uorNumberNull);
				int scaleNo = row;
				do {
					m_areaMain.setFirstVisible_RowTop(scaleNo);
					itemHide = doc->getScaleHide(uorRhtRowsHeader,scaleNo);
					if(!itemHide){
						sizeItem = doc->getScaleSize(uorRhtRowsHeader,scaleNo);
						if (sizeItem > uorNumberNull){
							if (sizeItem < sizeVAll){
								sizeVAll = sizeVAll - sizeItem;
							} else {
								m_areaMain.setShift_RowTop(sizeItem-sizeVAll);
								break;
							}
						}
					}
					--scaleNo;
					if (scaleNo<=0)
						break;
				} while(true);
				m_areaMain.setLastVisibleRow(row);
			}
			else if (m_areaMain.firstVisible_RowTop() > row || m_areaMain.lastVisibleRow() < row) {
				if (m_areaMain.firstVisible_RowTop() > row) {
					// Текшая ячейка находится вверху относительно видимой области
					m_areaMain.setFirstVisible_RowTop(row);
					m_areaMain.setShift_RowTop(uorNumberNull);
				} else if (m_areaMain.lastVisibleRow() < row) {
					m_areaMain.setLastVisibleRow(row);
					m_areaMain.setFirstVisible_RowTop(recalcVisibleScales(uorRhtRowsHeader));
				}
			}
		}
		if (ereaVer != m_areaMain.changesVer())
			recalcHeadersRects();
		onAccessRowOrCol(m_curentCell.y(), uorRhtRowsHeader);

	}
	if (m_curentCell.x() != col){
		m_curentCell.setX(col);
		if (ensureVisible){
			// гарантировать видимость ячейки. у-у-у-у!!!
			moveTo = uost_Right;
			if (oldX>col)
				moveTo = uost_Left;
			if (moveTo == uost_Left && m_areaMain.firstVisible_ColLeft() >= col){
				m_areaMain.setFirstVisible_ColLeft(col);
				m_areaMain.setShift_ColLeft(uorNumberNull);
			}
			else if (moveTo == uost_Right && ( m_areaMain.lastVisibleCol()-1) <= col)	{
				// Надо высчитать m_firstVisible_ColLeft и m_shift_ColLeft
				uorNumber sizeVAll = m_rectDataRegionFrame.width();
				uorNumber sizeItem = uorNumberNull;
				m_areaMain.setShift_ColLeft(uorNumberNull);
				int scaleNo = col;
				do {
					m_areaMain.setFirstVisible_ColLeft(scaleNo);
					itemHide = doc->getScaleHide(uorRhtColumnHeader, scaleNo);
					if(!itemHide){
						sizeItem = doc->getScaleSize(uorRhtColumnHeader,scaleNo);
						if (sizeItem > uorNumberNull){
							if (sizeItem < sizeVAll){
								sizeVAll = sizeVAll - sizeItem;
							} else {
								m_areaMain.setShift_ColLeft(sizeItem-sizeVAll);
								break;
							}
						}
					}
					--scaleNo;
					if (scaleNo<=0)
						break;
				} while(true);
			}
			else if (m_areaMain.firstVisible_ColLeft() > col || m_areaMain.lastVisibleCol() < col) {
				if (m_areaMain.firstVisible_ColLeft() > col) {
					// Текшая ячейка находится вверху относительно видимой области
					m_areaMain.setFirstVisible_ColLeft(col);
					m_areaMain.setShift_ColLeft(uorNumberNull);
				} else if (m_areaMain.lastVisibleCol() < col) {
					m_areaMain.setLastVisibleCol(col);
					m_areaMain.setFirstVisible_ColLeft(recalcVisibleScales(uorRhtColumnHeader));
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
		vsMaxVal = qMax(vsMaxVal,m_areaMain.lastVisibleRow());
		vsMaxVal = qMax(vsMaxVal,m_curentCell.y());
		vsMaxVal = int(vsMaxVal / UORPT_VIEW_SCROLL_KOEFF);
		curVal = int(m_areaMain.firstVisible_RowTop() / UORPT_VIEW_SCROLL_KOEFF);

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
		vsMaxVal = qMax(vsMaxVal,m_areaMain.lastVisibleCol());
		vsMaxVal = qMax(vsMaxVal,m_curentCell.x());
		vsMaxVal = int(vsMaxVal / UORPT_VIEW_SCROLL_KOEFF);
		curVal = int(m_areaMain.firstVisible_ColLeft() / UORPT_VIEW_SCROLL_KOEFF);

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
void uoReportCtrl::onAccessRowOrCol(const int& nom, const uoRptHeaderType& rht)
{
	int cnt = 0;
	if (rht == uorRhtColumnHeader) // Колонка
	{
		if (m_colCountVirt < nom) {
			cnt = nom - m_colCountVirt;
			m_colCountVirt = qMax(nom,m_colCountDoc);
			doChangeVirtualSize(rht, cnt);
		} else if (m_areaMain.lastVisibleCol() < m_colCountVirt){
			cnt = m_colCountVirt - m_areaMain.lastVisibleCol();
			m_colCountVirt = qMax(m_areaMain.lastVisibleCol() ,m_colCountDoc);
			doChangeVirtualSize(rht, cnt);
		}
	} else if (rht == uorRhtRowsHeader) {		// строка
		if (m_rowCountVirt < nom) {
			cnt = nom - m_rowCountVirt;
			m_rowCountVirt = nom;
			doChangeVirtualSize(rht, cnt);
		} else if (m_areaMain.lastVisibleRow()<m_rowCountVirt){
			if (m_curentCell.y()<m_rowCountVirt && m_rowCountVirt > m_rowCountDoc) {
				cnt = m_rowCountVirt - m_areaMain.lastVisibleRow();
				m_rowCountVirt = m_areaMain.lastVisibleRow();
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
void uoReportCtrl::changeDocSize(uorNumber sizeV, uorNumber sizeH)
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

void uoReportCtrl::onSheetPanShow(){	if (!m_sPanel->isVisible()) 	{m_sPanel->show();	recalcHeadersRects();	updateImage();}}
void uoReportCtrl::onSheetPanHide(){	if (m_sPanel->isVisible()) 	{m_sPanel->hide();		recalcHeadersRects();	updateImage();}}


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
