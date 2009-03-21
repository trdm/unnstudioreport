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
#include "uoReportDocBody.h"
#include "uoReportUndo.h"




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
bool uorSelVisitorSaveProps::visit(uoCell* cell, bool& first)
{
	bool retVal = true;
	if (cell){
		if (cell->m_textProp){
			if (first) {
				m_textDec_Selection.copyFrom(cell->m_textProp);
			} else {
				m_textDec_Selection.mergeItem(*cell->m_textProp);
			}
		}
	}
	return retVal;
}
/// Собрать все проперти в выделени и замержить(слить) их.
bool uorSelVisitorSetProps::visit(uoCell* cell, bool& first)
{
	bool retVal = true;
	if (cell){
		if (cell->m_textProp){
			cell->m_textProp->assignItem(m_textDec_Selection);
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


	m_charWidthPlus 	= 3;
 	m_showGroup		= false;
	m_showSection	= false;
	m_showRuler		= true;
	m_showGrid		= true;
	m_showFrame		= true;
	m_showInvisiblChar= false;

	m_maxVisibleLineNumberCnt = 3;

	m_firstVisible_RowTop 	= 1; 	///< Первая верхняя видимая строка
	m_firstVisible_ColLeft 	= 1; 	///< Первая левая видимая колонка
	m_lastVisibleRow 		= -1;
	m_lastVisibleCol 		= -1;

	m_scaleFactor 			= m_scaleFactorO = 1;

	m_shift_RowTop 	= 0; ///< Смещение первой видимой строки вверх (грубо - размер невидимой их части)
	m_shift_ColLeft 	= 0;

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
	m_imageView = QImage(size(), QImage::Format_ARGB32_Premultiplied);
	m_drawToImage = 1;
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

	//m_hScrollCtrl->setRange(1,3);
	m_hScrollCtrl->setValue(1);
	m_hScrollCtrl->setMinimum(1);

	m_vScrollCtrl->setMinimum(1);
	m_vScrollCtrl->setValue(1);
	connect(m_vScrollCtrl, SIGNAL(valueChanged(int)), this, SLOT(onSetVScrolPos(int)));
	connect(m_vScrollCtrl, SIGNAL(actionTriggered(int)), this, SLOT(onScrollActionV(int)));

//	connect(this, SIGNAL(wheelEvent(QWheelEvent*)), m_vScrollCtrl, SLOT(wheelEvent(QWheelEvent*))); /// не получается...

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
	qreal targetSize = (rht==rhtHorizontal) ? m_rectDataRegion.right() : m_rectDataRegion.bottom();
	qreal shiftSize = 0.0;
	if (rht == rhtHorizontal) {
		m_scaleStartPositionMapH.clear();
		numScale 		= m_firstVisible_ColLeft;
		shiftSize 	= m_rectDataRegion.left();
		curetnSize =  shiftSize - m_shift_ColLeft;
		m_scaleStartPositionMapH[numScale] = curetnSize;
	} else if (rht == rhtVertical){
		m_scaleStartPositionMapV.clear();
		numScale 		= m_firstVisible_RowTop;
		curetnSize = m_rectDataRegion.top() - m_shift_RowTop;
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
		if (rht == rhtHorizontal){
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
	if (rht == rhtHorizontal) {

		if (m_scaleStartPositionMapH.contains(grItem->m_start))
			xPos = m_scaleStartPositionMapH[grItem->m_start];

		itemVisible = colVisible(grItem->m_start);
		if (itemVisible) {

			xSize0 = xSize = doc->getScaleSize(rht, grItem->m_start);
			grItem->_rectIteract.setLeft(xPos);
			grItem->_rectIteract.setRight(xPos);


			grItem->m_sizeTail = xPos + xSize; // Координаты конца ячейки.

			if (xSize0 > (m_charWidthPlus + UORPT_OFFSET_LINE*2)){


				xSize = (xSize - (UORPT_OFFSET_LINE + m_charWidthPlus)) / 2;
				xPos = xPos + xSize;
				grItem->_rectIteract.setLeft(xPos);
				xPos = xPos + UORPT_OFFSET_LINE * 2 + m_charWidthPlus;
				grItem->_rectIteract.setRight(xPos);
			} else if (xSize0>0){
				grItem->_rectIteract.setLeft(xPos);
				xPos = xPos + xSize;
				grItem->_rectIteract.setRight(xPos);
			}
		} else {
			grItem->_rectIteract.setLeft(-10);
			grItem->_rectIteract.setRight(-20);
		}

		yPos = m_rectGroupH.top() + ( UORPT_OFFSET_LINE * 2 + m_charHeightPlus) * grItem->m_level - UORPT_OFFSET_LINE;
		grItem->_rectIteract.setBottom(yPos);
		yPos = yPos - m_charHeightPlus;
		grItem->_rectIteract.setTop(yPos);

		grItem->m_rectMidlePos = yPos + m_charHeightPlus / 2;

		grItem->_rectIteract.adjust(0.0, 1.0, 0.0, -1.0);
		grItem->m_rectEndPos = xPos;
		grItem->m_sizeTail = grItem->m_sizeTail - xPos; // Координаты конца ячейки.


	} else if (rht == rhtVertical) {
		if (m_scaleStartPositionMapV.contains(grItem->m_start))
			yPos0 = yPos = m_scaleStartPositionMapV[grItem->m_start];

		itemVisible = rowVisible(grItem->m_start);

		if (itemVisible) {
			grItem->m_sizeTail = yPos;

			ySize0 = ySize = doc->getScaleSize(rht, grItem->m_start);
			ySize = (ySize  - m_charHeightPlus) / 2;
			grItem->m_sizeTail = yPos + ySize0; // Координаты конца ячейки.

			grItem->_rectIteract.setTop(yPos + ySize);
			ySize = ySize + m_charHeightPlus; // + UORPT_OFFSET_LINE*2;
			grItem->_rectIteract.setBottom(yPos + ySize);
			if (grItem->_rectIteract.height()> ySize0){
				grItem->_rectIteract.setTop(yPos0);
				grItem->_rectIteract.setBottom(yPos0 + ySize0);
			}


			if (ySize0 < grItem->_rectIteract.height() && (yPos0 - 1) > 3) {
				grItem->_rectIteract.setTop(yPos0 + 1);
				grItem->_rectIteract.setBottom(yPos0 + ySize0 - 2);
			}
		} else {
			// ну вот так топорно решим глюку..
			grItem->_rectIteract.setTop(-10);
			grItem->_rectIteract.setBottom(-20);
		}


		xPos = m_rectGroupV.left() + ( UORPT_OFFSET_LINE * 2 + m_charWidthPlus) * grItem->m_level;
		grItem->_rectIteract.setRight(xPos);
		xPos = xPos - (m_charWidthPlus + UORPT_OFFSET_LINE * 2);
		grItem->_rectIteract.setLeft(xPos);

		grItem->m_rectMidlePos = xPos + m_charWidthPlus / 2 + UORPT_OFFSET_LINE;

		grItem->_rectIteract.adjust(1.0, 0.0, -1.0,0.0);

		grItem->m_rectEndPos = grItem->_rectIteract.bottom();
		grItem->m_sizeTail = grItem->m_sizeTail - grItem->m_rectEndPos; // Координаты конца ячейки.


	}
}

/// Вычислить длину диапазона ячеек.
qreal uoReportCtrl::getLengthOfScale(uoRptHeaderType rht, int start, int stop)
{
	qreal retVal = 0.0;

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
	if (rht == rhtUnknown || rht == rhtHorizontal)
		m_lastVisibleCol = recalcVisibleScales(rhtHorizontal);

	if (rht == rhtUnknown || rht == rhtVertical) {
		m_lastVisibleRow = recalcVisibleScales(rhtVertical);
	}

	uoLineSpan* spn;
	uoRptHeaderType rhtCur = rht;

	if (m_showGroup) {
		uoRptGroupItem* grItem;

		int spnCnt = 0;

		if (rht == rhtUnknown || rht == rhtHorizontal) {
			rhtCur = rhtHorizontal;
			spnCnt = doc->getGroupLevel(rhtCur);
			if (spnCnt>0) {
				const spanList* spanListH = doc->getGroupList(rhtCur, m_firstVisible_ColLeft, m_lastVisibleCol);
				for (i=0; i<spanListH->size(); i++){
					spn = spanListH->at(i);
					grItem = getGropItemFromCache();
					if (grItem)	{
						grItem->copyFrom(spn);
						grItem->m_sizeTail = rptSizeNull;
						zeroQRectF(grItem->_rectIteract);
						grItem->m_tailPosIsAbs = false;

						rSize = rptSizeNull;
						calcGroupItemPosition(grItem, rhtCur);
						// посчитаем длину линии группировки.
						// хвост есть, нужно вычислить только толщину последующих ячеек и добавить его к хвосту.
						if (grItem->m_start >= m_firstVisible_ColLeft){
							// это если рект группировки нормально отображается, а если нет?
							grItem->m_sizeTail = grItem->m_sizeTail + getLengthOfScale(rhtCur, grItem->m_start + 1, grItem->m_end);
						} else {
							grItem->m_sizeTail = 0.0;
							if (m_firstVisible_ColLeft <= grItem->m_end) {
								grItem->m_tailPosIsAbs = true;
								grItem->m_sizeTail = getLengthOfScale(rhtCur, m_firstVisible_ColLeft, grItem->m_end) - m_shift_ColLeft;
							}
						}
						m_groupListH->append(grItem);
					}
				}
				delete spanListH;
			}
		}

//		rht = rhtVertical;
		if (rht == rhtUnknown || rht == rhtVertical) {
			rhtCur = rhtVertical;
			spnCnt = doc->getGroupLevel(rhtCur);
			if (spnCnt>0) {
				const spanList* spanListV = doc->getGroupList(rhtCur, m_firstVisible_RowTop, m_lastVisibleRow);
				for (i=0; i<spanListV->size(); i++){
					spn = spanListV->at(i);
					grItem = getGropItemFromCache();
					if (grItem)	{
						grItem->copyFrom(spn);

						zeroQRectF(grItem->_rectIteract);
						grItem->m_tailPosIsAbs = false;
						calcGroupItemPosition(grItem, rhtCur);


						if (grItem->m_start >= m_firstVisible_RowTop){
							// это если рект группировки нормально отображается, а если нет?
							grItem->m_sizeTail = grItem->m_sizeTail + getLengthOfScale(rhtCur, grItem->m_start + 1, grItem->m_end);
						} else {
							grItem->m_sizeTail = 0.0;
							if (m_firstVisible_RowTop <= grItem->m_end) {
								grItem->m_tailPosIsAbs = true;
								grItem->m_sizeTail = getLengthOfScale(rhtCur, m_firstVisible_RowTop, grItem->m_end) - m_shift_RowTop;
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

		if (rht == rhtUnknown || rht == rhtHorizontal)
		{
			rhtCur = rhtHorizontal;
			sectLevel = doc->getSectionLevel(rhtCur);
			if (sectLevel>0){

				const spanList* spanSectListH = doc->getSectionList(rhtCur, m_firstVisible_ColLeft, m_lastVisibleCol);
				for (i=0; i<spanSectListH->size(); i++){
					spn = spanSectListH->at(i);
					selectItem = getSectionItemFromCache();
					if (selectItem)	{
						selectItem->copyFrom(spn);
						zeroQRectF(selectItem->_rectView);
						selectItem->m_selected = false;

						// Посчитаем рект данной секции....
						QRectF& rect = selectItem->_rectView;
						offSet = 0.0;
						coord = m_rectSectionH.top();
						coord = coord - ( UORPT_OFFSET_LINE * 2 + m_charHeightPlus) * (1 - selectItem->m_level);
						// С каждым уровнем секция все "ниже". Вот только не помню, левел нуль-ориентрованный или как?
						rect.setTop(coord);
						rect.setBottom(m_rectSectionH.bottom());

						cellVisStart = qMax(selectItem->m_start, m_firstVisible_ColLeft);
						cellVisEnd = qMin(selectItem->m_end, m_lastVisibleCol);

						coord = m_rectSectionH.left()-1; // пусть спрячется, если что..
						if (m_scaleStartPositionMapH.contains(cellVisStart)){
							coord = m_scaleStartPositionMapH[cellVisStart];
							if (cellVisStart == m_firstVisible_ColLeft)
								offSet = m_shift_ColLeft;
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
		if (rht == rhtUnknown || rht == rhtVertical)
		{
			rhtCur = rhtVertical;
			sectLevel = doc->getSectionLevel(rhtCur);
			if (sectLevel>0){

				const spanList* spanSectListV = doc->getSectionList(rhtCur, m_firstVisible_RowTop, m_lastVisibleRow);
				for (i=0; i<spanSectListV->size(); i++){
					spn = spanSectListV->at(i);
					selectItem = getSectionItemFromCache();
					if (selectItem)	{
						selectItem->copyFrom(spn);
						zeroQRectF(selectItem->_rectView);
						selectItem->m_selected = false;

						cellVisStart = qMax(selectItem->m_start, m_firstVisible_RowTop);
						cellVisEnd = qMin(selectItem->m_end, m_lastVisibleRow);

						// Посчитаем рект данной секции....
						QRectF& rect = selectItem->_rectView;
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
							if (cellVisStart == m_firstVisible_RowTop)
								offSet = m_shift_RowTop;
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
    qreal wWidth = getWidhtWidget(); //-2 -m_vScrollCtrl->width();
    qreal wHeight = getHeightWidget(); //-2-m_hScrollCtrl->height();


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
	int spnCntH = doc->getGroupLevel(rhtHorizontal);
	if (spnCntH>0 && m_showGroup) {
		curOffset += UORPT_OFFSET_LINE; // Оффсет-отступ сверху.
		m_rectGroupH.setTop(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + m_charHeightPlus) * spnCntH;
		m_rectGroupH.setBottom(curOffset);
	}
	spnCntH = doc->getSectionLevel(rhtHorizontal);
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
	int spnCntV = doc->getGroupLevel(rhtVertical);
	if (spnCntV>0 && m_showGroup) {
		curOffset += UORPT_OFFSET_LINE;
		m_rectGroupV.setLeft(curOffset);
		curOffset += ( UORPT_OFFSET_LINE * 2 + m_charWidthPlus) * spnCntV;
		m_rectGroupV.setRight(curOffset);
	}
	spnCntV = doc->getSectionLevel(rhtVertical);
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

	m_rowsInPage = (int) (m_rectDataRegion.height() / doc->getDefScaleSize(rhtVertical)); 		///< строк на страницу
	m_colsInPage = (int) (m_rectDataRegion.width() / doc->getDefScaleSize(rhtHorizontal)); 		///< столбцов на страницу


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
//			qDebug() << rgItem->_rectIteract << rgItem->m_start << rgItem->m_end;
//		}
//	}
//
//	if (!m_groupListV->isEmpty()) {
//		qDebug() << "m_groupListV";
//		for (cntr = 0; cntr<m_groupListV->size(); cntr++) {
//			rgItem = m_groupListV->at(cntr);
//			qDebug() << rgItem->_rectIteract << rgItem->m_start << rgItem->m_end;
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
	m_penText.setStyle(Qt::DotLine);
	painter.setPen(m_penText);
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
	m_penText.setStyle(Qt::SolidLine);
	painter.setPen(m_penText);

	// Рисуем шапку, как она сама есть...
	if (m_showGroup) {
		if (m_rectGroupV.width()>noLen)	{
			if (!m_groupListV->isEmpty()) {
				painter.setClipRect(m_rectGroupV); // Устанавливаем область прорисовки. Будем рисовать только в ней.
				for (cntr = 0; cntr<m_groupListV->size(); cntr++) {
					grItem = m_groupListV->at(cntr);

					if (grItem->_rectIteract.height() > minDrawSize) {
						painter.drawRect(grItem->_rectIteract);
						paintStr = "-";
						if (grItem->m_folded)
							paintStr = "+";
						painter.drawText(grItem->_rectIteract,Qt::AlignCenter,  paintStr);
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
					if (grItem->_rectIteract.width() > minDrawSize) {
						painter.drawRect(grItem->_rectIteract);
						paintStr = "-";
						if (grItem->m_folded)
							paintStr = "+";
						painter.drawText(grItem->_rectIteract,Qt::AlignCenter,  paintStr);
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
				QRectF rect = sectItem->_rectView;
				painter.setPen(m_penText);
				if (sectItem->m_selected){
					painter.fillRect(rect, m_brushBlack);
					painter.setPen(m_penWhiteText);
				} else {
					painter.fillRect(rect, m_brushWindow);
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
				QRectF rect = sectItem->_rectView;
				painter.setPen(m_penText);
				if (sectItem->m_selected){
					painter.fillRect(rect, m_brushBlack);
					painter.setPen(m_penWhiteText);
				} else {
					painter.fillRect(rect, m_brushWindow);
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

	m_penText.setStyle(Qt::SolidLine);
	painter.setPen(m_penText);

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
			painter.setPen(m_penWhiteText);
			painter.drawRect(curRctCpy);
			curRctCpy.adjust(1,1,-1,-1);
			painter.fillRect(curRctCpy, m_brushBlack);
			painter.setPen(m_penText);
		}

		hdrType = rhtVertical;
		if (m_rectRulerV.width() > 0) {

			painter.setClipRect(m_rectRulerV); // Устанавливаем область прорисовки. Будем рисовать только в ней.

			curRct.setLeft(m_rectRulerV.left()+m_scaleFactorO);
			curRct.setRight(m_rectRulerV.right()-m_scaleFactorO);

			paintEndTagr = m_rectRulerV.bottom();
			paintCntTagr = m_rectRulerV.top() - m_shift_RowTop + 1 * m_scaleFactorO;
			curRct.setTop(paintCntTagr);
			nmLine = m_firstVisible_RowTop-1;
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
					painter.setPen(m_penWhiteText);
					painter.setBrush(m_brushBlack);
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

		hdrType = rhtHorizontal;
		if (m_rectRulerH.width() > 0) {

			painter.setClipRect(m_rectRulerH); // Устанавливаем область прорисовки. Будем рисовать только в ней.

			curRct.setTop(m_rectRulerH.top()+m_scaleFactorO);
			curRct.setBottom(m_rectRulerH.bottom()-m_scaleFactorO);

			paintEndTagr = m_rectRulerH.right();
			paintCntTagr = m_rectRulerH.left() - m_shift_ColLeft + 1 * m_scaleFactorO;

			curRct.setLeft(paintCntTagr);
			nmLine = m_firstVisible_ColLeft - 1;
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
					painter.setPen(m_penWhiteText);
					painter.setBrush(m_brushBlack);
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

/// Отрисовка ячейки таблицы
void uoReportCtrl::drawCell(QPainter& painter, uoCell* cell, QRectF& rectCell, uoReportDoc* doc, bool isSell)
{
	if (!cell)
		return;

	QString text = cell->getTextWithLineBreak(m_showInvisiblChar);
	if (!text.isEmpty()) {
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
		qreal diffSize = cell->getMaxRowLength() - rectCpyCell.width();
		if (diffSize > 1.0){
			uoHorAlignment ha = cell->getAlignmentHor();
			qreal pointPos = 0.0, diffSize = cell->getMaxRowLength() - rectCpyCell.width();
			switch (ha){
				case uoHA_Left:
				{
					// значит надо двигать правый угол. вроде...
					pointPos = rectCpyCell.right() + diffSize;
					pointPos = qMin(pointPos, m_rectDataRegion.right());
					rectCpyCell.setRight(pointPos);
					break;
				}
				case uoHA_Right:
				{
					// значит надо двигать левый угол. вроде...
					pointPos = rectCpyCell.left() - diffSize;
					pointPos = qMin(pointPos, m_rectDataRegion.left());
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


		}
		painter.drawText(rectCpyCell,flags,text);
		painter.setPen(oldPen);
		painter.restore();
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

	m_penText.setStyle(Qt::SolidLine);
	painter.setPen(m_penText);

	QRectF rectCell;
	QRectF rectCellCur; // ячейка на которой курсор стоит...
	zeroQRectF(rectCell);

	bool isSell = false;
	bool isHide = false;
	{
		// нарисуем рамку области данных, т.к. потом будем рисовать линии на ней в этой процедурине.
		// painter.drawRect(m_rectDataRegionFrame);
	}
	painter.fillRect(m_rectDataRegion, m_brushBase);
	painter.setClipRect(m_rectDataRegion); // Устанавливаем область прорисовки. Будем рисовать только в ней.
	QPen oldPen = painter.pen();
	painter.setPen(m_penGrey);

	int rowCur = m_firstVisible_RowTop;
	int colCur = m_firstVisible_ColLeft;
	int colPrev = -1, colNext = -1;


	qreal rowsLenCur = m_rectDataRegion.top() - m_shift_RowTop; // + 1 * m_scaleFactorO;
	qreal rowsLensPage = m_rectDataRegion.bottom();

	qreal colsLenCur = m_rectDataRegion.left() - m_shift_ColLeft;
	qreal colsLensPage = m_rectDataRegion.right();
	rectCell.setTop(rowsLenCur);
	rectCell.setLeft(m_rectDataRegion.left() - m_shift_ColLeft);
	QString cellTest;
	uoCell* curCell = NULL;


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

		colCur = m_firstVisible_ColLeft;
		for ( int i = 0; i < 2; i++) {
			colCur = m_firstVisible_ColLeft;
			do {	// столбцы ||||||||||||||||||||||||||||||||||||
				if (colCur == m_firstVisible_ColLeft) {
					colsLenCur = m_rectDataRegion.left() - m_shift_ColLeft;
					rectCell.setLeft(colsLenCur);
				}

				while((isHide = doc->getScaleHide(rhtHorizontal, colCur))){
					++colCur;
				}
				sz = doc->getScaleSize(rhtHorizontal, colCur);
				colNext = doc->getNextCellNumber(rowCur, colCur, true);

				colsLenCur += sz;
				rectCell.setRight(colsLenCur);
				// а вот если ячейка - текущая?
				if (m_curentCell.x() == colCur && m_curentCell.y() == rowCur){
					rectCellCur = rectCell;				/// у бля....
				}
				isSell = m_selections->isCellSelect(rowCur,colCur);
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
						painter.drawRect(rectCell);
					}
				} else {

					curCell = doc->getCell(rowCur,colCur,false);
					if (curCell){
						drawCell(painter, curCell, rectCell, doc, isSell);
					}
				}

				rectCell.setLeft(rectCell.right());
				colPrev = colCur;
				colCur = colCur + 1;
			} while(colsLenCur < colsLensPage);
		}
		rectCell.setTop(rectCell.bottom());
		rowCur = rowCur + 1;
	} while(rowsLenCur < rowsLensPage);

	if (!rectCellCur.isNull()){
		int wp = m_penText.width();
		m_penText.setWidth(2);
		painter.setPen(m_penText);
		painter.drawRect(rectCellCur);
		m_penText.setWidth(wp);
	}
	painter.setPen(oldPen);
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
		QLineF line(pnt1, pnt2);
		QPen redPen;
		redPen.setColor(Qt::red);
		redPen.setStyle(Qt::DashDotLine);
		painter.setPen(redPen);
		painter.drawLine(line);

		painter.setPen(oldPen);
	}
}


/// Отрисовка виджета.
void uoReportCtrl::drawWidget(QPainter& painter)
{

	const QPalette palette_c = palette();
	QPalette::ColorGroup curColGrp = QPalette::Active;
	if (!isEnabled()) curColGrp = QPalette::Disabled;

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

	drawHeaderControlContour(painter);
	drawHeaderControlGroup(painter);
	drawHeaderControl(painter);
	drawDataArea(painter);
}
/// Типа рисуем
void uoReportCtrl::paintEvent(QPaintEvent *event)
{
	if (m_drawToImage>0){
		m_imageView = QImage(size(), QImage::Format_ARGB32_Premultiplied);
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
		painter.setPen(m_penNoPen);
		painter.scale(1/m_scaleFactor,1/m_scaleFactor);
		painter.fillRect(m_cornerWidget->frameGeometry(), m_brushWindow);
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

			uoRptHeaderType rht = rhtHorizontal;
			if (m_rectGroupV.contains(posX, posY)){
				rht = rhtVertical;
				groupItList = m_groupListV;

			}
			if (!groupItList->isEmpty()) {
				uoRptGroupItem* rptGrItem = NULL;
				bool found = false;
				for (int i = 0; i< groupItList->size(); i++){
					rptGrItem = groupItList->at(i);

					if (rptGrItem->_rectIteract.contains(posX, posY)){
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
		uoRptHeaderType rHt = rhtVertical;
		uoRptSectionItem* sItem = NULL;
		uoRptSectionItem* sItemCur = NULL;
		uoRptSectionItemList* sList = m_sectItemListV;
		uoRptSectionItemList* sListOther = m_sectItemListH;
		if (m_rectSectionH.contains(posX, posY)) {
			rHt = rhtHorizontal;
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
				if (sItemCur->_rectView.contains(posX, posY))
					sItem = sItemCur;
			}
		}
		if (sItem){
			m_selections->clearSelections();
			clearSelectionsSection();
			sItem->m_selected = true;
			if (rHt == rhtVertical){
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

	if (rht == rhtVertical){
		cntScale = m_firstVisible_RowTop;
		endPos = posY;
		stratPos = m_rectDataRegion.top() - m_shift_RowTop;
		m_curMouseSparesRect.setLeft(m_rectRulerV.left());
		m_curMouseSparesRect.setRight(m_rectRulerV.right());
	} else if (rht == rhtHorizontal) {
		cntScale = m_firstVisible_ColLeft;
		endPos = posX;
		stratPos = m_rectDataRegion.left() - m_shift_ColLeft;
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

	if (rht == rhtVertical){
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
	m_curMouseSparesRht  = rhtUnknown;
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

	if (rht == rhtVertical || rht == rhtUnknown) {

		posStart 	= rect.top() - dragSize;
		posEnd 		= posStart + interVal;
		if (posStart<=pos && pos<= posEnd)
			return uoBlt_Top;

		posStart 	= rect.bottom() - dragSize;
		posEnd 		= posStart + interVal;
		if (posStart<=pos && pos<= posEnd)
			return uoBlt_Bottom;

	}
	if (rht == rhtHorizontal || rht == rhtUnknown) {

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
	uoRptHeaderType rhtCur = rhtVertical;
	if (m_rectRulerH.contains(posX, posY))
		rhtCur = rhtHorizontal;

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
		if (rhtCur == rhtHorizontal){
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
		if (rhtCur == rhtVertical)
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
		uorSelVisitorSetProps processor;
		processor.m_needCreate = true;
		processor.m_textDec_Selection.copyFrom(m_propEditor->m_textPropRes);
		processSelection(&processor);
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
	qDebug() << "processSelection:";

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
				processor->visit(curCell, firstOne);
				if (curCell->m_textProp){
					processor->m_textDec_Selection.copyFrom(curCell->m_textProp);
				}
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
							qDebug() << "processor->visit: col "<< col << " row "<< row;
							processor->visit(curCell, firstOne);
							if (firstOne)
								firstOne = !firstOne;
						}
					}
				}
			}
		}
	}

}

/// При изменении выделения ИНОГДА неоходимо собрать сводные данные по свойствам выделения.
void uoReportCtrl::recalcSelectionProperty()
{
	m_textDec_Selection.resetItem();
	m_borderProp_Selection.resetItem();	///< закешированное значения свойств.
	uorSelVisitorSaveProps processor;
	processor.m_textDec_Selection.copyFrom(&m_textDec_Selection);
	processSelection(&processor);
	m_textDec_Selection.copyFrom(&processor.m_textDec_Selection);
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


//	uorTextDecor* 	textProp = propEditor->m_textProp;
	recalcSelectionProperty();
	propEditor->m_textProp->copyFrom(&m_textDec_Selection);
	propEditor->m_sellectonType = m_selections->getSelectionType();

//	uorBorderPropBase 	m_borderProp_Selection;	///< закешированное значения свойств.
//	uorBorderPropBase* 	m_borderProp;	///< закешированное значения свойств.


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
				if (findScaleLocation(posX, posY, line,rhtVertical)) {
					m_selections->rowSelectedEnd(line);
					needUpdate = true;
				}
			}
			if (rmsSelectionRule_Top == m_stateMode) {
				if (findScaleLocation(posX, posY, line,rhtHorizontal)) {
					m_selections->colSelectedEnd(line);
					needUpdate = true;
				}
			}
		} else if (m_stateMode == rmsResizeRule_Left || m_stateMode == rmsResizeRule_Top) {
			needUpdate = true;
			int distance = (m_curMouseLastPos - event->pos()).manhattanLength();
			if (distance > QApplication::startDragDistance()) {

				qreal delta = 0;
				uoRptHeaderType rht = (m_stateMode == rmsResizeRule_Left) ? rhtVertical : rhtHorizontal;
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

			pntAdd.setY(qMax(pntAdd.y(), int(m_rectDataRegion.top()-m_shift_RowTop)));
			pntAdd.setY(qMin(pntAdd.y(), int(m_rectDataRegion.bottom())));

			pntAdd.setX(qMax(pntAdd.x(), int(m_rectDataRegion.left()-m_shift_ColLeft)));
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
		uoRptHeaderType rhdType = rhtUnknown;
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

					rhdType = vertLoc ? rhtVertical : rhtHorizontal;
					uorBorderLocType locType = uoBlt_Unknown;

					need_shape = Qt::PointingHandCursor;
					findArea = true; //  типа нашли позицию и не хрю...
					// далее проанализируем точную позицию.
					findArea = findScaleLocation(posX, posY, lineNo, rhdType);
					if (!findArea)
						return;
					if (rhdType == rhtVertical) {
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
			while(itemHiden = doc->getScaleHide(rhtVertical, ++posY)){}
			break;
		}
		case Qt::Key_Up: {
			do {
				posY = posY - 1;
				if (posY == 0) {
					posY = m_curentCell.y();
					break;
				}
				itemHiden = doc->getScaleHide(rhtVertical, posY);

			} while(itemHiden);

			if (!(posY >= 1))
				posY = m_curentCell.y();
			break;
		}
		case Qt::Key_Right: {
			while(itemHiden = doc->getScaleHide(rhtHorizontal, ++posX)){}
			break;
		}
		case Qt::Key_Left:	{
			do {
				posX = posX - 1;
				if (posX == 0) {
					posX = m_curentCell.x();
					break;
				}
				itemHiden = doc->getScaleHide(rhtHorizontal, posX);

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
			if(key == Qt::Key_PageUp && m_firstVisible_RowTop == 1) {
				posY = 1;
			} else if (key == Qt::Key_PageDown /*&& m_firstVisible_RowTop == 1*/ && posY < m_lastVisibleRow) {
				posY = m_lastVisibleRow;
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

				qreal pageHeight = m_rowsInPage * doc->getDefScaleSize(rhtVertical);
				do {
					curRow = curRow + modif;
					if (curRow == 0) {
						curRow = 1;
						break;
					}
					rowHiden = doc->getScaleHide(rhtVertical, curRow);
					if (rowHiden)
						continue;
					pageHeight = pageHeight - doc->getScaleSize(rhtVertical, curRow);
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

	if (m_showInvisiblChar) 	menuShow->addAction(m_iteractView->m_actInvCharHide);
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

	targMenu->addSeparator();
	QMenu *menuSpecial 	= targMenu->addMenu(QString::fromUtf8("Специальное"));
	menuSpecial->addAction(m_iteractView->m_actClear);
	menuSpecial->addAction(m_iteractView->m_actOutToDebug);

	targMenu->addSeparator();
	targMenu->addAction(m_iteractView->m_actSave);
	targMenu->addAction(m_iteractView->m_actSaveAs);
	targMenu->addAction(m_iteractView->m_actLoad);
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



void uoReportCtrl::propertyEditorHide()
{
	if (m_propEditor){
		m_propEditor->hidePriperty();
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
	if (m_firstVisible_RowTop<=nmRow && m_lastVisibleRow >= nmRow) {
		return true;
	}
	return false;
}

/// Видим ли солбец
bool uoReportCtrl::colVisible(int nmCol) const
{
	if (m_firstVisible_ColLeft <= nmCol && m_lastVisibleCol >= nmCol){
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
				QFont* font = cell->getFont(doc);
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
		doc->setCellText(m_curentCell.y(), m_curentCell.x(), text);
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
	if (y == m_curentCell.y() || y<=0) {
		return;
	}
	m_firstVisible_RowTop = y;
	recalcHeadersRects();
	/// нафига они вообще нужны? onSetVScrolPos и onSetHScrolPos
}

/// Сигнал на изменение позиции горизонтального скрола
void uoReportCtrl::onSetHScrolPos(int x){
	if (x == m_curentCell.x() || x<=0) {
		return;
	}
	m_firstVisible_ColLeft = x;
	recalcHeadersRects();
}

/// попробуем поймать скрол вертикального слайдера..
void uoReportCtrl::onScrollActionV(int act){	doScrollAction(act, rhtVertical);}

/// попробуем поймать скрол горизонтального слайдера..
void uoReportCtrl::onScrollActionH(int act){	doScrollAction(act, rhtHorizontal);}

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
			if (rht == rhtVertical) {
				toY = 1;
			} else {
				toX = 1;
			}
			break;
		}
		case QAbstractSlider::SliderSingleStepSub:	{
			if (rht == rhtVertical) {
				toY = -1;
			} else {
				toX = -1;
			}
			break;
		}
		case QAbstractSlider::SliderPageStepAdd:	{
			if (rht == rhtVertical) {
				toY = 1 * m_rowsInPage;
			} else {
				toX = 1 * m_colsInPage;
			}
			break;
		}
		case QAbstractSlider::SliderPageStepSub:	{
			if (rht == rhtVertical) {
				toY = -1 * m_rowsInPage;
			} else {
				toX = -1 * m_colsInPage;
			}
			break;
		}
		case QAbstractSlider::SliderMove:	{
			if (rht == rhtVertical) {
				m_firstVisible_RowTop = m_vScrollCtrl->sliderPosition();
				m_shift_RowTop = 0.0;
			} else {
				m_firstVisible_ColLeft = m_hScrollCtrl->sliderPosition();
				m_shift_ColLeft = 0.0;
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
			pos = m_firstVisible_ColLeft + dx;
			while(itemHiden = doc->getScaleHide(rhtHorizontal, pos)){
				++pos;
			}
			m_firstVisible_ColLeft = pos;
			m_shift_ColLeft = 0.0;
		} else if (dx < 0){
			pos = m_firstVisible_ColLeft + dx;
			if (pos > 0) {
				while(itemHiden = doc->getScaleHide(rhtHorizontal, pos) && (pos >= 1)){
					--pos;
				}
			} else {
				pos = 1;
			}
			pos = qMax(1, pos);

			m_firstVisible_ColLeft = pos;
			m_shift_ColLeft = 0.0;
		}
		onAccessRowOrCol(m_firstVisible_ColLeft/* + m_rowsInPage*/, rhtHorizontal);
		pos = m_hScrollCtrl->value() + dx;
		if (pos>0)
			m_hScrollCtrl->setValue(pos);
	}

	if (dy != 0) {
		if (dy > 0) {
			pos = m_firstVisible_RowTop + dy;
			while(itemHiden = doc->getScaleHide(rhtVertical, pos)){
				++pos;
			}
			m_firstVisible_RowTop = pos;
			m_shift_RowTop = 0.0;
		} else if (dy < 0){
			pos = m_firstVisible_RowTop + dy;
			if (pos > 0) {
				while(itemHiden = doc->getScaleHide(rhtVertical, pos) && (pos > 1)){
					--pos;
				}
			} else {
				pos = 1;
			}

			m_firstVisible_RowTop = pos;
			m_shift_RowTop = 0.0;
		}
		onAccessRowOrCol(m_firstVisible_RowTop/* + m_rowsInPage*/, rhtVertical);
		pos = m_vScrollCtrl->value() + dy;
		if (pos>0)
			m_vScrollCtrl->setValue(pos);

	}
	recalcHeadersRects();
	updateThis();
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
		rect.setHeight((int)(doc->getScaleSize(rhtVertical,posY) * m_scaleFactor));
		rect.setWidth((int)(doc->getScaleSize(rhtHorizontal,posX) * m_scaleFactor));
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

	int rowCur = m_firstVisible_RowTop;
	int colCur = m_firstVisible_ColLeft;

	qreal rowsLenCur = m_rectDataRegion.top() - m_shift_RowTop;
	qreal colsLenCur = m_rectDataRegion.left() - m_shift_ColLeft;

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
		if (posY>= rowsLenCur && posY <= (rowsLenCur + sz)) {
			cell.setY(rowCur);
			// тут поищем колонку.
			do {

				sz = doc->getScaleSize(rhtHorizontal, colCur);
				if (sz == 0.0) {
					++colCur;
					continue;
				}
				if (doc->getScaleHide(rhtHorizontal, colCur)){
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
	m_curentCell.x() < m_firstVisible_ColLeft ||
	m_curentCell.x() > m_lastVisibleCol ||
	m_curentCell.y() < m_firstVisible_RowTop ||
	m_curentCell.y() > m_lastVisibleRow
	) {
		return false;
	} else {
		///\todo надо еще проверить то, что строка или колонка не являются скрытими...
		if (doc->getScaleHide(rhtVertical,m_curentCell.y()) || doc->getScaleHide(rhtHorizontal,m_curentCell.x()))
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
			if (moveTo == uost_Top && m_firstVisible_RowTop >= y){
				m_firstVisible_RowTop = y;
				m_shift_RowTop = 0;
				m_lastVisibleRow = recalcVisibleScales(rhtVertical);

			}
			else if (moveTo == uost_Bottom && ( m_lastVisibleRow) <= y)	{
				// Надо высчитать m_firstVisible_RowTop и m_shift_RowTop
				qreal sizeVAll = m_rectDataRegionFrame.height();
				qreal sizeItem = 0.0;
				m_shift_RowTop = 0.0;
				int scaleNo = y;
				do {
					m_firstVisible_RowTop = scaleNo;
					itemHide = doc->getScaleHide(rhtVertical,scaleNo);
					if(!itemHide){
						sizeItem = doc->getScaleSize(rhtVertical,scaleNo);
						if (sizeItem > 0.0){
							if (sizeItem < sizeVAll){
								sizeVAll = sizeVAll - sizeItem;
							} else {
								m_shift_RowTop = sizeItem-sizeVAll;
								break;
							}
						}
					}
					--scaleNo;
					if (scaleNo<=0)
						break;
				} while(true);
				m_lastVisibleRow = y;
			}
			else if (m_firstVisible_RowTop > y || m_lastVisibleRow < y) {
				if (m_firstVisible_RowTop > y) {
					// Текшая ячейка находится вверху относительно видимой области
					m_firstVisible_RowTop = y;
					m_shift_RowTop = 0.0;
				} else if (m_lastVisibleRow < y) {
					m_lastVisibleRow = y;
					m_firstVisible_RowTop = recalcVisibleScales(rhtVertical);
				}
			}
		}
		recalcHeadersRects();
		onAccessRowOrCol(m_curentCell.y(), rhtVertical);
		m_vScrollCtrl->setValue(m_curentCell.y());

	}
	if (m_curentCell.x() != x){
		m_curentCell.setX(x);
		if (ensureVisible){
			// гарантировать видимость ячейки. у-у-у-у!!!
			moveTo = uost_Right;
			if (oldX>x)
				moveTo = uost_Left;
			if (moveTo == uost_Left && m_firstVisible_ColLeft >= x){
				m_firstVisible_ColLeft = x;
				m_shift_ColLeft = 0;
			}
			else if (moveTo == uost_Right && ( m_lastVisibleCol-1) <= x)	{
				// Надо высчитать m_firstVisible_ColLeft и m_shift_ColLeft
				qreal sizeVAll = m_rectDataRegionFrame.width();
				qreal sizeItem = 0.0;
				m_shift_ColLeft = 0.0;
				int scaleNo = x;
				do {
					m_firstVisible_ColLeft = scaleNo;
					itemHide = doc->getScaleHide(rhtHorizontal, scaleNo);
					if(!itemHide){
						sizeItem = doc->getScaleSize(rhtHorizontal,scaleNo);
						if (sizeItem > 0.0){
							if (sizeItem < sizeVAll){
								sizeVAll = sizeVAll - sizeItem;
							} else {
								m_shift_ColLeft = sizeItem-sizeVAll;
								break;
							}
						}
					}
					--scaleNo;
					if (scaleNo<=0)
						break;
				} while(true);
			}
			else if (m_firstVisible_ColLeft > x || m_lastVisibleCol < x) {
				if (m_firstVisible_ColLeft > x) {
					// Текшая ячейка находится вверху относительно видимой области
					m_firstVisible_ColLeft = x;
					m_shift_ColLeft = 0.0;
				} else if (m_lastVisibleCol < x) {
					m_lastVisibleCol = x;
					m_firstVisible_ColLeft = recalcVisibleScales(rhtHorizontal);
				}
			}
		}
		recalcHeadersRects();
		onAccessRowOrCol(m_curentCell.x(), rhtHorizontal);
		m_hScrollCtrl->setValue(m_curentCell.x());

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
		m_vScrollCtrl->setMaximum(qMax(m_rowCountVirt, m_rowCountDoc));
		m_vScrollCtrl->blockSignals ( false );

	}
	{
		int widthW 	= (int)getWidhtWidget(); // / doc->getDefScaleSize(rhtHorizontal);

		if (m_sizeHvirt < (int)widthW) {
			m_sizeHvirt = widthW;
		}
		if (m_sizeHvirt < m_sizeHDoc){
			m_sizeHvirt = m_sizeHDoc;
		}
		m_hScrollCtrl->setMinimum(1);
		m_hScrollCtrl->setMaximum(qMax(m_colCountVirt, m_colCountDoc));
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
	if (rht == rhtVertical) {
		lineCount = doc->getRowCount();
		if (lineCount<m_rowCountVirt){
			m_sizeVvirt = (int) (doc->getVSize() + (m_rowCountVirt - lineCount) * doc->getDefScaleSize(rht));
		}
	} else if (rht == rhtHorizontal){
		lineCount = doc->getColCount();
		if (lineCount<m_colCountVirt){
			m_sizeHvirt = (int)(doc->getHSize() + (m_colCountVirt - lineCount) * doc->getDefScaleSize(rht));
		}
	}
	recalcScrollBars();
}

/// при доступе к строке или столбцу вьюва, если
void uoReportCtrl::onAccessRowOrCol(int nom, uoRptHeaderType rht)
{
	int cnt = 0;
	if (rht == rhtHorizontal) // Колонка
	{
		if (m_colCountVirt < nom) {
			cnt = nom - m_colCountVirt;
			m_colCountVirt = qMax(nom,m_colCountDoc);
			doChangeVirtualSize(rht, cnt);
		} else if (m_lastVisibleCol<m_colCountVirt){
			cnt = m_colCountVirt - m_lastVisibleCol;
			m_colCountVirt = qMax(m_lastVisibleCol,m_colCountDoc);
			doChangeVirtualSize(rht, cnt);
		}
	} else if (rht == rhtVertical) {		// строка
		if (m_rowCountVirt < nom) {
			cnt = nom - m_rowCountVirt;
			m_rowCountVirt = nom;
			doChangeVirtualSize(rht, cnt);
		} else if (m_lastVisibleRow<m_rowCountVirt){
			if (m_curentCell.y()<m_rowCountVirt && m_rowCountVirt > m_rowCountDoc) {
				cnt = m_rowCountVirt - m_lastVisibleRow;
				m_rowCountVirt = m_lastVisibleRow;
				doChangeVirtualSize(rht, cnt);
			}
		}
	}
}

///< при доступе к строке или столбцу документа...
void uoReportCtrl::onAccessRowCol(int nmRow, int nmCol)
{
	if (nmRow > 0)		onAccessRowOrCol(nmRow, rhtVertical);
	if (nmCol > 0)		onAccessRowOrCol(nmCol, rhtHorizontal);
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

void uoReportCtrl::onInvisibleCharShow(){	if (!m_showInvisiblChar) {		m_showInvisiblChar = true;		updateImage();	}}
void uoReportCtrl::onInvisibleCharHide(){	if (m_showInvisiblChar) {		m_showInvisiblChar = false;		updateImage();	}}


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
