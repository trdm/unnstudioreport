/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "QDebug"
#include <QApplication>
#include <QFontMetrics>
#include <QRegExp>
#include <QPainter>
#include "uoReportDocBody.h"
#include "uoReportDoc.h"
#include "uoReportLoader.h"
#include "uoReportUndo.h"
#include "uoCacheItemizer.h"
#include "uoReportCtrl.h"
#include "uorPagePrintSetings.h"
#include "uoReportManager.h"
#include "uoReportDrawHelper.h"
#include "uorMimeData.h"
#include "uoCellMatrix.h"
#include "uoPainter.h"

namespace uoReport {

uoReportDoc::uoReportDoc()
//	:uoReportDocBody()
	: m_headerRow(new uoHeaderScale)
	, m_headerCol(new uoHeaderScale)
	, m_rows(new uoRowsDoc)
	, m_TextDecorDoc(0)
	, m_sourceDoc()
{
	m_spanTreeGrCol = new uoSpanTree(this, uorStt_Group);
	m_spanTreeGrRow = new uoSpanTree(this, uorStt_Group);
	m_spanTreeSctCol = new uoSpanTree(this, uorStt_Section);
	m_spanTreeSctRow = new uoSpanTree(this, uorStt_Section);

	m_spanTreeSctCol->setCanOnlyOne(true);
	m_spanTreeSctRow->setCanOnlyOne(true);
	m_storeFormat = uoRsf_Unknown;
	m_pagesSetings = new uorPagePrintSetings;
	m_fontColl = new uoReportDocFontColl;
	m_pageList = new uorAresList;
	m_selection = new uoReportSelection;
	m_pictCasher = new uoCellPictCasher;

	m_matrixCell = 0;

	m_rows->setDoc(this);
	initTextDecorDoc();
	setDefaultFont(QApplication::font());

	m_headerRow->setDefSize((uorNumber)UORPT_SCALE_SIZE_DEF_VERTICAL);
	m_headerCol->setDefSize((uorNumber)UORPT_SCALE_SIZE_DEF_HORIZONTAL);

	m_rowCount 	= 0;
	m_colCount 	= 0;

	m_sizeV_visible = m_sizeV = uorNumberNull;	///< Размер документа по вертикали
	m_sizeH_visible = m_sizeH = uorNumberNull;	///< Размер документа по горизонтали
	m_freezEvent = 0;
	m_refCounter = 0;
	m_defaultFontId = 0;
	m_ident = 0;
	m_pointBlock = new uoTextTrPointCash;
	m_undoManager = new uoReportUndo;
	if (m_undoManager) {		
		m_undoManager->setDoc(this);
		m_undoManager->enableCollectChanges(true);
	}

	m_cellDefault = new uoCell(-1);
	if (m_cellDefault){
		m_cellDefault->provideAllProps(this, true);
	}
	m_changes = 0;
	m_formatEnable = true;
	m_saveWithUndoStack = false;
	m_pictCasherUse = true;
#ifdef Q_OS_WIN
	m_pictCasherUse = false;
#endif


}

uoReportDoc::~uoReportDoc()
{
	clear();
	delete m_spanTreeGrCol;
	delete m_spanTreeGrRow;
	delete m_spanTreeSctCol;
	delete m_spanTreeSctRow;
	delete m_headerRow;
	delete m_headerCol;
	delete m_fontColl;
	delete m_TextDecorDoc;
	delete m_cellDefault;
	delete m_pagesSetings;
	delete m_selection;
	if(m_matrixCell) 	delete m_matrixCell;


}

/// Очистка секций.
void uoReportDoc::clear()
{
	m_spanTreeGrCol->clear();
	m_spanTreeGrRow->clear();
	m_spanTreeSctCol->clear();
	m_spanTreeSctRow->clear();
	m_selection->clearSelections();

	m_rowCount 	= 0;
	m_colCount 	= 0;
	m_pagesSetings->clear();

	m_sizeV_visible = m_sizeV = uorNumberNull;
	m_sizeH_visible = m_sizeH = uorNumberNull;
	m_fontColl->clear();
	m_headerRow->clear();
	m_headerCol->clear();
	m_rows->clear();
	m_pointBlock->clear();
	m_undoManager->clear();
	setDefaultFont(QApplication::font());
	++m_changes;
	while (!m_segmentList.isEmpty())     delete m_segmentList.takeFirst();
	while (!m_pageList->isEmpty())     delete m_pageList->takeFirst();
	m_pictCasher->pixmapClearAll();

}

/// Очистака шрифтов. Обусловлено спецификой порядка выгрузки...
void uoReportDoc::clearFonts()
{
	m_fontColl->clear();
	++m_changes;
}



/// Установить дефолтный шрифт, размер и семейство.
void uoReportDoc::setDefaultFont(const QFont& defFont)
{
	QString fam = defFont.family();
	if (m_fontColl){
		int fId = m_fontColl->findFont(fam);
		if (fId == -1){
			fId = m_fontColl->addFont(fam);
		}
		if (m_TextDecorDoc){
			m_TextDecorDoc->m_fontSz = defFont.pointSize();
			m_TextDecorDoc->m_fontB 	= defFont.bold();
			m_TextDecorDoc->m_fontId 	= fId;
			m_TextDecorDoc->m_fontI 	= defFont.italic();
			m_TextDecorDoc->m_fontU 	= defFont.underline();
		} else {

		}

	} else {
		qWarning() << "Not init m_fontColl";
	}
	//fontApp = QApplication::font();
}

/// Возвращает настройки страниц документа для печати.
uorPagePrintSetings* uoReportDoc::pagesSetings() const
{
	return m_pagesSetings;
}

void uoReportDoc::initTextDecorDoc()
{
	m_TextDecorDoc = new uorTextDecor;
	m_TextDecorDoc->resetItem();
}

/// проверка возможности сформировать/вставить группу в документ.
/// может потребоваться для того, что-бы задизаблить пункты динамического меню
bool uoReportDoc::possiblyAddGroup(int start, int end, uoRptHeaderType ht)
{
	if(ht == uorRhtRowsHeader)	return m_spanTreeGrRow->possiblyAddSpan(start, end);
	else					return m_spanTreeGrCol->possiblyAddSpan(start, end);
}

/// добавление группировки в отчет.
bool uoReportDoc::addGroup(int start, int end, uoRptHeaderType ht, bool folded)
{
	bool retVal = false;
	uoSpanTree* treeGrp = NULL;

	if(ht == uorRhtRowsHeader) {
		treeGrp = m_spanTreeGrRow;
		beforeAddRowOrCol(end - m_rowCount, ht);
	} else if (ht == uorRhtColumnHeader) {
		treeGrp = m_spanTreeGrCol;
		beforeAddRowOrCol(end - m_colCount, ht);
	}
	retVal = treeGrp->addSpan(start, end, folded);
	++m_changes;
	return retVal;
}

/// Максимальный уровень вложения групп.
int uoReportDoc::getGroupLevel(uoRptHeaderType ht){
	uoSpanTree* treeGrp = NULL;
	if(ht == uorRhtRowsHeader)
		treeGrp = m_spanTreeGrRow;
	else
		treeGrp = m_spanTreeGrCol;
	return treeGrp->getLevel();
}

/// Максимальный уровень вложения секций.
int uoReportDoc::getSectionLevel(uoRptHeaderType ht){
	uoSpanTree* treeGrp = NULL;
	if(ht == uorRhtRowsHeader)	treeGrp = m_spanTreeSctRow;
	else					treeGrp = m_spanTreeSctCol;
	return treeGrp->getLevel();
}

/// Свертка/развертка группы.
void uoReportDoc::doGroupFold(int idGrop, uoRptHeaderType rht, bool fold){
	uoSpanTree* treeGrp = NULL;
	if(rht == uorRhtRowsHeader)	treeGrp = m_spanTreeGrRow;
	else					treeGrp = m_spanTreeGrCol;
	++m_freezEvent;
	QList<int>* lineList= treeGrp->onGroupFold(idGrop, fold);
	if (!lineList)
		return;
	if (!lineList->isEmpty()){
		int lineNo = 0;
		for (int i = 0; i<lineList->size(); i++){
			lineNo = lineList->at(i);
			setScalesHide(rht, lineNo, 1, fold);
		}
	}
	--m_freezEvent;
	delete lineList;
	if (m_freezEvent == 0)
		emit onSizeChange(m_sizeV_visible, m_sizeH_visible);
	++m_changes;
}


bool uoReportDoc::enableCollectChanges(const bool enable)
{
	bool old = false;
	if (m_undoManager){
		old = m_undoManager->isCollectChanges();
		m_undoManager->enableCollectChanges(enable);
	}
	return old;
}
bool uoReportDoc::isCollectChanges() const
{
	if (m_undoManager){
		return m_undoManager->isCollectChanges();
	}
	return false;
}

/// Возвращает установленный формат сохранения
uoRptStoreFormat uoReportDoc::getStoreFormat()	{
	return m_storeFormat;
}

/// Возвращает установленное имя файла.
QString uoReportDoc::getStorePathFile()	{
	return m_docFilePath;
}

/// установим опции сохранения
void uoReportDoc::setStoreOptions(QString  filePath, uoRptStoreFormat stFormat)	{
	m_docFilePath = filePath;
	m_storeFormat = stFormat;
}

/// Получить список спанов группировок по диапазону строк/столбцов. Исключая зафолденные...
const spanList* uoReportDoc::getGroupList(uoRptHeaderType rht, int start, int end)
{
	if (rht == uorRhtColumnHeader)
		return m_spanTreeGrCol->getSpanList(start, end, true);
	else
		return m_spanTreeGrRow->getSpanList(start, end, true);
}

/// Получить список спанов СЕКЦИЙ по диапазону строк/столбцов
const spanList* uoReportDoc::getSectionList(uoRptHeaderType rht, int start, int end)
{
	if (rht == uorRhtColumnHeader)
		return m_spanTreeSctCol->getSpanList(start, end, false);
	else
		return m_spanTreeSctRow->getSpanList(start, end, false);
}



/// проверка возможности сформировать/вставить СЕКЦИЮ в документ.
/// может потребоваться для того, что-бы задизаблить пункты динамического меню
bool uoReportDoc::possiblyAddSection(int start, int end, uoRptHeaderType ht)
{
	if(ht == uorRhtColumnHeader)
		return m_spanTreeSctCol->possiblyAddSpan(start, end);
	else
		return m_spanTreeSctRow->possiblyAddSpan(start, end);
}

/// вставка секции в отчет
bool uoReportDoc::addSection(int start, int end, uoRptHeaderType ht, QString name)
{
	if(ht == uorRhtColumnHeader)	return m_spanTreeSctCol->addSpan(start, end, name);
	else					return m_spanTreeSctRow->addSpan(start, end, name);
	++m_changes;
}

/// Отдать менеджер секций
uoSpanTree* uoReportDoc::getSectionManager(uoRptHeaderType ht)
{
	if(ht == uorRhtColumnHeader)		return m_spanTreeSctCol;
	else if(ht == uorRhtRowsHeader)	return m_spanTreeSctRow;
	return NULL;
}
/// Отдать менеджер групп
uoSpanTree* uoReportDoc::getGroupManager(uoRptHeaderType ht)
{
	if(ht == uorRhtColumnHeader)		return m_spanTreeGrCol;
	else if(ht == uorRhtRowsHeader)	return m_spanTreeGrRow;
	return NULL;
}


void uoReportDoc::doDeleteColumns(int itemStart, int count)
{
	m_spanTreeSctCol->onLinesDelete(itemStart, count);
	m_spanTreeGrCol->onLinesDelete(itemStart, count);
	m_headerCol->deleteItem(itemStart, count);
	uoRow* row = m_rows->getFirst();
	while(row)
	{
		row->deleteItem(itemStart, count);
		row = m_rows->getNext();
	}
	m_pictCasher->pixmapClearAll();
	doFormatDoc();
	emit onDataChange();
	++m_changes;
}


void uoReportDoc::doDeleteRows(int itemStart, int count)
{
	m_spanTreeSctRow->onLinesDelete(itemStart, count);
	m_spanTreeGrRow->onLinesDelete(itemStart, count);
	m_headerRow->deleteItem(itemStart, count);
	m_rows->deleteItem(itemStart, count);

	m_pictCasher->clearFromToEnd(uorRhtRowsHeader, itemStart);
	doFormatDoc(itemStart);
	emit onDataChange();
	++m_changes;
}


void uoReportDoc::doAddColumns(int itemStart, int count)
{
	m_spanTreeSctCol->onLinesAdd(itemStart, count);
	m_spanTreeGrCol->onLinesAdd(itemStart, count);
	m_headerCol->addEmptyItems(itemStart, count);
	uoRow* row = m_rows->getFirst();
	while(row)
	{
		row->addEmptyItems(itemStart, count);
		row = m_rows->getNext();
	}
	m_pictCasher->pixmapClearAll();
	doFormatDoc();
	emit onDataChange();
	++m_changes;
}
void uoReportDoc::doAddRows(int itemStart, int count)
{
	m_spanTreeSctRow->onLinesAdd(itemStart, count);
	m_spanTreeGrRow->onLinesAdd(itemStart, count);
	m_headerRow->addEmptyItems(itemStart, count);
	m_rows->addEmptyItems(itemStart, count);
	m_pictCasher->pixmapClearAll();
	doFormatDoc();
	emit onDataChange();
}

/**
	Обеспечение работы буфера обмена и перетаскивания.
*/

uorMimeData* uoReportDoc::createMimeData(uoReportSelection* selection)
{
	uorMimeData* data = new uorMimeData(this, selection);
	uoReportLoader* loader = uoReportLoader::getLoader(uoRsf_XML_Mime);
	QString allText;
	if (loader){
		flush(loader, selection);
		allText = loader->readAll();
	}

	delete loader;
	return data;
}



/** Сброс документа на диск после инициализации лоадера.
	использую для обхода необходимости писать каждый раз
	деинициализацию лоадера при неудаче в выгрузке...
*/
bool uoReportDoc::flush(uoReportLoader* loader, uoReportSelection* selection, bool erase){

	bool retVal = false;

	int cntItem 	= 0;
	int cntItem2 	= 0;
	int i 			= 0;
	int typeHeader 	= 0;
//	bool store 		= false;

	uoLineSpan* spn 	= NULL;
	uoSpanTree* curTree = NULL;

	uoRptHeaderType curHeaderType = uorRhtColumnHeader;
	// Тут просто перебор документа с обращением к лоадеру..
	if (loader){
		retVal = loader->saveDocStart(this);
		if (!retVal) return false;
	} else {
		retVal = true;
	}

	// ---------------выгрузка группировок --------------
	for (typeHeader = 1; typeHeader<=2; typeHeader++) {
		if(typeHeader == 1){
			curTree	= m_spanTreeGrCol;
			curHeaderType = uorRhtColumnHeader;
		} else {
			curTree	= m_spanTreeGrRow;
			curHeaderType = uorRhtRowsHeader;
		}


		cntItem = curTree->getSize();
		if (cntItem > 0) {
			const spanList* spnList = curTree->getSpanList();
			spanList spnList2;
			cntItem2 = 0;
			for (i = 0; i<cntItem; i++){
				spn = spnList->at(i);
				if (selection){
					if (selection->isSpanFullSeleced(spn, curHeaderType)){
						spnList2.append(spn);
						cntItem2 += 1;
					}
				} else {
					spnList2.append(spn);
					cntItem2 += 1;
				}
			}
			if (cntItem2 > 0 && loader){

				retVal = loader->saveGroupsHeaderStart(cntItem2, curHeaderType);
				if (!retVal) break;
				for (i = 0; i<cntItem2; i++){
					spn = spnList2.at(i);
					retVal = loader->saveGroupsItem(spn);
					if (!retVal) break;
				}
				retVal = loader->saveGroupsHeaderEnd(curHeaderType);
				if (!retVal) break;
			}

			delete spnList;
			spnList = NULL;
		}
	}
	if (!retVal) return false;

	// ---------------выгрузка секций --------------
	for (typeHeader = 1; typeHeader<=2; typeHeader++) {
		if(typeHeader == 1){
			curTree	= m_spanTreeSctCol;
			curHeaderType = uorRhtColumnHeader;
		} else {
			curTree	= m_spanTreeSctRow;
			curHeaderType = uorRhtRowsHeader;
		}


		cntItem = curTree->getSize();
		if (cntItem > 0) {

			const spanList* spnList = curTree->getSpanList();

			spanList spnList2;
			cntItem2 = 0;
			for (i = 0; i<cntItem; i++){
				spn = spnList->at(i);
				if (selection){
					if (selection->isSpanFullSeleced(spn, curHeaderType)){
						spnList2.append(spn);
						cntItem2 += 1;
					}
				} else {
					spnList2.append(spn);
					cntItem2 += 1;
				}
			}
			if (cntItem2 > 0 && loader){
				retVal = loader->saveSectionHeaderStart(cntItem2, curHeaderType);
				if (!retVal) break;

				for (i = 0; i<cntItem2; i++){
					spn = spnList2.at(i);
					retVal = loader->saveSectionItem(spn);
					if (!retVal) break;
				}

				retVal = loader->saveSectionHeaderEnd(curHeaderType);
				if (!retVal) break;
			}
			delete spnList;
			spnList = NULL;

		}
	}
	if (!retVal) return false;

	uoHeaderScale* headerScale = NULL;

	uorSelectionType selType = uoRst_Unknown;
	if (selection){
		selType = selection->selectionType();
	}


	QList<int> setOfScale;
	bool hasItem = false;
	bool useSelection = false;

	// ---------------выгрузка размеров и фиксации строк/ размеров столбцов --------------
	for (typeHeader = 1; typeHeader<=2; typeHeader++) {
		if(typeHeader == 1){
			headerScale	= m_headerCol;
			curHeaderType = uorRhtColumnHeader;
		} else {
			headerScale	= m_headerRow;
			curHeaderType = uorRhtRowsHeader;
		}

		cntItem = headerScale->getCountItem();
		hasItem = true;
		useSelection = false;
		if (selection){
			if (!selection->isTrueForCopy())
					continue;
			selType = selection->selectionType();

			if (curHeaderType == uorRhtColumnHeader){
				if (!(selType == uoRst_Document || selType == uoRst_Column || selType == uoRst_Column))
					continue;
			} else if (curHeaderType == uorRhtRowsHeader){
				if (!(selType == uoRst_Document || selType == uoRst_Row || selType == uoRst_Rows))
					continue;
			}
			useSelection = true;

			if (useSelection && selType == uoRst_Document)
				useSelection = false; // не надо филтровать

			if (useSelection){
				if (!selection->getSelectedColRow(curHeaderType, setOfScale))
					continue;
				cntItem = setOfScale.size();
			}
		}

		if (cntItem > 0) {
			if (loader){
				retVal = loader->saveScaleHeaderStart(cntItem, curHeaderType);
				if (!retVal) break;
			}
			if (useSelection) {
				headerScale->onStoreItems(loader, &setOfScale,erase);
			} else {
				headerScale->onStoreItems(loader,0,erase);
			}
			if (loader){
				retVal = loader->saveScaleHeaderEnd(curHeaderType);
				if (!retVal) break;
			}
		}

	}

	//--------- выгрузка содержания строк -------------------
	///\todo вот тут остановился... НУЖНА ОТЛАДКА.....

	QRect boundRect; // если выделение миксированное, то нет смысла сливать туда все троки.
	uoCellMatrix* cellMatrix = 0;
	bool useStandatrMethod = true;

	int rowCount = m_rows->getCountItem();
	if (selection) {
		if (selType == uoRst_Row || selType == uoRst_Rows) {
			rowCount = selection->rowCount();
		} else if (selType == uoRst_Cell || selType == uoRst_Cells || selType == uoRst_Mixed || selType == uoRst_Unknown){
			useStandatrMethod = false;
 			boundRect = selection->getSelectionBound();
			if (boundRect.height()>0)
				rowCount = boundRect.height();
		}
		if (loader)
			selection->saveSelection(loader);

	}
	if (rowCount>0) {
		if (useStandatrMethod) {
			if (loader)
				loader->saveRowsStart(rowCount);
			m_rows->saveItems(loader, selection,erase);
			if (loader)
				loader->saveRowsEnd();
		} else {
			if (loader)
				cellMatrix = new uoCellMatrix(boundRect.height(),boundRect.width());
			QPoint curCellView = selection->currentCell();
			QRect  curCellRect = selection->getSelectionBound(); // на случай если текщая ячейка предстваляет собой объединение.

			uoCell* curCell = 0;
			bool cellIsSelected = false;
			int r_cntr = 1, c_cntr = 1;
			for (int rrow = boundRect.top(); rrow<=boundRect.bottom(); rrow++)	{
				c_cntr = 1;
				for (int rcol = boundRect.left(); rcol<=boundRect.right(); rcol++)	{
					cellIsSelected = selection->isCellSelect(rrow, rcol);
					if (!cellIsSelected && selType == uoRst_Unknown){
						if (rcol == curCellView.x() && rrow == curCellView.y()){
							cellIsSelected = true;
						} else if (curCellRect.contains(rcol, rrow)){
							cellIsSelected = true;
						}
					}

					if (cellIsSelected){
						curCell = getCell(rrow, rcol, false);
						if (curCell){
							if (loader) {
								cellMatrix->setCell(r_cntr, c_cntr, curCell);
								cellMatrix->setCellSelected(r_cntr, c_cntr);
							}
							if (erase)
								curCell->clear();
						}
					}
					c_cntr += 1;
				}
				r_cntr += 1;
			}
			if (loader) {
				loader->saveMatrixStart(cellMatrix);

				r_cntr = 1, c_cntr = 1;
				int oldCellNom = 0;
				for (int rrow = 1; rrow<=cellMatrix->rows(); rrow++)	{
					c_cntr = 1;
					loader->saveMatrixRowStart(r_cntr);
					for (int rcol = 1; rcol<=cellMatrix->cols(); rcol++)	{
						curCell = cellMatrix->cell(r_cntr, c_cntr);
						if (curCell){
							// Необходимо установить номер.
							oldCellNom = curCell->number();
							curCell->setNumber(c_cntr);
							loader->saveCell(curCell);
							curCell->setNumber(oldCellNom);
						}
						c_cntr += 1;
					}
					r_cntr += 1;
					loader->saveMatrixRowEmd();
				}
				loader->saveMatrixEnd(cellMatrix);
			}
			delete cellMatrix;
		}
	}
	if (!loader)
		return true;

	if (m_fontColl->countFonts()>0){
		QFont* font = NULL;
		loader->saveFontStart(m_fontColl->countFonts());
		for (i = 0; i< m_fontColl->countFonts(); i++){
			font = m_fontColl->getFont(i);
			loader->saveFont(font, i);
		}
		loader->saveFontEnd();
	}

	if (m_saveWithUndoStack){
		loader->saveArbitraryBlockStart("Undo stack");
		m_undoManager->save(loader);
		loader->saveArbitraryBlockEnd("Undo stack");
	}

	if (!retVal) return false;
	retVal = loader->saveDocEnd(this);
	return retVal;
}

/// Сохранение отчета. Инициализируем лоадер, вызываем сохранение...
bool uoReportDoc::saveToFile(QString path, uoRptStoreFormat stFormat, uoReportSelection* sel)
{
	bool retVal = false;
	if (stFormat == uoRsf_Unknown) {
		qWarning() << tr("no define store format");
		return false;
	}
	uoReportLoader* loader = uoReportLoader::getLoader(stFormat);
	if (!loader) {
        if (stFormat == uoRsf_PDF) {
            ///\todo Печать в PDF сделать.
            printDoc();
        } else {
            qWarning() << tr("Can not create loader");		return false;
        }
	}

	loader->setFileName(path);
	if (!loader->initFO(false)){
		qWarning() << loader->getLastError();
	} else {
		flush(loader, sel);
	}
	loader->finalize();
	delete loader;

	return retVal;
}

bool uoReportDoc::saveToByteArray(QByteArray& byteArr, uoReportSelection* sel)
{
    Q_UNUSED(byteArr); Q_UNUSED(sel);
	return true;
}
bool uoReportDoc::loadFromByteArray(QByteArray& byteArr, uoReportSelection* sel)
{
    Q_UNUSED(sel);
	bool retVal = false;
	uoReportLoader* loader = uoReportLoader::getLoader(uoRsf_XML_Mime);
	loader->enableProcDialog(false);
	if (!loader) {
		qWarning() << tr("Can not create loader");
		return false;
	}
//	bool is_ch = enableCollectChanges(false);
//	bool is_fr = enableFormating(false);


	m_formatEnable = false;

//	qDebug() << "uoReportDoc::loadFromByteArray byteArr.size() "<< byteArr.size();
	if (!loader->initBA(uorSO_LoadFromBArray, byteArr)){
		qWarning() << loader->getLastError();
	} else {
		retVal = loader->load(this);
	}
	loader->finalize();
	delete loader;
	m_formatEnable = true;
	return retVal;
}


/// Проверка на валидность опций записи: имя файла и тип документа.
bool uoReportDoc::saveOptionsIsValid(){
	if (m_docFilePath.isEmpty() || m_storeFormat == uoRsf_Unknown) {
		return false;
	}
	return true;

}

/// Сохранение отчета.
bool uoReportDoc::save(uoReportSelection* sel){
	if (saveOptionsIsValid()) {
		return saveToFile(m_docFilePath, m_storeFormat, sel);
	}
	return false;
}

/// Считывание отчета.
bool uoReportDoc::load()
{
	if (saveOptionsIsValid()) {
		++m_changes;
		return loadFromFile(m_docFilePath, m_storeFormat);
	}
	return false;
}

bool uoReportDoc::loadFromFile(QString path, uoRptStoreFormat stFormat)
{
	bool retVal = true;
	if (stFormat == uoRsf_Unknown) {
		qWarning() << tr("no define store format");
		return false;
	}
	uoReportLoader* loader = uoReportLoader::getLoader(stFormat);
	if (!loader) {
		qWarning() << tr("Can not create loader");		return false;
	}
	bool is_ch = enableCollectChanges(false);
	bool is_fr = enableFormating(false);


	m_formatEnable = false;
	clear();
	loader->setFileName(path);
	if (!loader->initFO(true)){
		qWarning() << loader->getLastError();
	} else {
		retVal = loader->load(this);
	}
	loader->finalize();
	delete loader;
	m_formatEnable = true;
	enableCollectChanges(is_ch);
	enableFormating(is_fr);
	doFormatDoc();

	return retVal;

}



/// Возвращаем дефолтный размер строки или колонки
uorNumber  	uoReportDoc::getDefScaleSize(uoRptHeaderType rht)
{
	if (rht == uorRhtRowsHeader){
		return m_headerRow->getDefSizeItem();
	} else if (rht == uorRhtColumnHeader) {
		return m_headerCol->getDefSizeItem();
	}
	return uorNumberNull;
}

/// Возвращаем высоту документа полную (по умолчанию) или видимую.
uorNumber  	uoReportDoc::getVSize(bool visible){
	if (visible)
		return m_sizeV_visible;
	else
		return m_sizeV;
}

/// Возвращаем длину документа полную (по умолчанию) или видимую.
uorNumber  	uoReportDoc::getHSize(bool visible){
	if (visible)
		return m_sizeH_visible;
	else
		return m_sizeH;
}

/// Возвращаем количество строк
int 	uoReportDoc::getRowCount(){	return m_rowCount;}

/// Возвращаем количество столбцов
int 	uoReportDoc::getColCount(){	return m_colCount;}

/// Ундо/редо
void uoReportDoc::onUndo(){
	if (m_undoManager){
		if (m_undoManager->undoAvailability()){
			--m_changes;
			m_undoManager->undo(this);
		}
	}
}
/// Ундо/редо
void uoReportDoc::onRedo(){
	if (m_undoManager){
		if (m_undoManager->redoAvailability()){
			m_undoManager->redo(this);
			++m_changes;
		}
	}
}
/// Сигнал об изменении данных из вне, напирмер из механизма ундо/редо.
void uoReportDoc::onDataChange()
{
	/// надо переапдейтить все прикрепленные вьювы...
	uoReportCtrl* ctrl = NULL;
	for (int i = 0; i<m_atachedView.size(); i++){
		ctrl = m_atachedView.at(i);
		if (ctrl){
			ctrl->onDataChange();
		}
	}
}

/// Сигнал для вывода на печать.
void uoReportDoc::onPrint()
{
	printDoc(true);
}


void uoReportDoc::onPasteText(uoReportSelection* sel, const QMimeData* mime)
{
	QString plainText = mime->text();
	QString rowText, cellText;
	if (plainText.isEmpty() || !sel)
		return;

	QPoint insPoint = sel->getInsertPoint();


	bool isCsv = mime->hasFormat("text/csv");
	QStringList strListRow;
	QStringList strListWord;
	strListRow = plainText.split("\n");

	int n_row = 0, n_col = 0;
	int real_row = insPoint.y(), real_col = insPoint.x();

	m_undoManager->groupCommandStart();
	for(n_row = 0; n_row < strListRow.size(); n_row++) {
		rowText = strListRow.at(n_row);
		cellText = "";
		if (isCsv){
			strListWord = rowText.split(";");
		} else {
			strListWord = rowText.split("\t");
		}
		for(n_col = 0; n_col<strListWord.size(); n_col++){
			cellText = strListWord.at(n_col);
			setCellText(real_row+n_row, real_col+n_col , cellText);
		}
		doFormatRow(real_row+n_row, -1);
	}
	m_undoManager->groupCommandEnd();
}

void uoReportDoc::copyDocum(uoReportDoc* fromDoc)
{
	clear();
	// Группы
	m_spanTreeGrCol->copyFrom(fromDoc->m_spanTreeGrCol);
	m_spanTreeGrRow->copyFrom(fromDoc->m_spanTreeGrRow);

	// Секции.
	m_spanTreeSctCol->copyFrom(fromDoc->m_spanTreeSctCol);
	m_spanTreeSctRow->copyFrom(fromDoc->m_spanTreeSctRow);

	m_headerRow->copyFrom(fromDoc->m_headerRow);
	m_headerCol->copyFrom(fromDoc->m_headerCol);

	m_rows->copyFrom(fromDoc->m_rows);
	doFormatDoc(-1, -1);
}


void uoReportDoc::onPasteDocum(uoReportSelection* sel, const QMimeData* mime)
{
	QByteArray byteArr = mime->data(UOR_MIME_XML_DATA);
	if (byteArr.isEmpty())
		return;
	uoReportDoc* newDoc = new uoReportDoc();
	if (newDoc){
		if (newDoc->loadFromByteArray(byteArr)){
			//пока тут замержим, потом перенесем в отдельную процедуру.

			uoReportSelection* selOth_Doc = newDoc->selection();
			if (selOth_Doc) {
				m_undoManager->groupCommandStart();
				// а без матрицы че делать? Вот только не помню, если весь документ выделен, матрица там какая?
				uoCellMatrix* matrixO_Doc = newDoc->getCellMatrix();
				uorSelectionType selType = selOth_Doc->selectionType();

				if (selType == uoRst_Document ){
					copyDocum(newDoc);
					return;
				}
				uoRptHeaderType rht;
				int rowOffset = 0;
				int colOffset = 0;
				int startWith = 0;

				QPoint curCell = sel->currentCell();

				if (selType == uoRst_Row || selType == uoRst_Rows ||selType == uoRst_Column || selType == uoRst_Columns) {
					/**
						\todo продолжить тут 2009
						сделать функцию копирования документа.
						сделать вставку в нужное место, т.е. сдвинуть относительно текущего положения курсора в таблице.
						добавить вызов комманды форматирования после вставки.
					*/
					QList<int> listRC;
					if (selOth_Doc->getSelectedColRow(rht, listRC)) {

						startWith = listRC.at(0);

						if (rht == uorRhtColumnHeader){
							colOffset = curCell.x() - startWith;
							m_spanTreeGrCol->copyFrom(newDoc->m_spanTreeGrCol, listRC, colOffset);
							m_headerCol->copyFrom(newDoc->m_headerCol, listRC, colOffset);
							m_spanTreeSctCol->copyFrom(newDoc->m_spanTreeSctCol, listRC, colOffset);

						} else if (rht == uorRhtRowsHeader){
							rowOffset = curCell.y() - startWith;

							m_spanTreeGrRow->copyFrom(newDoc->m_spanTreeGrRow, listRC, rowOffset);
							m_spanTreeSctRow->copyFrom(newDoc->m_spanTreeSctRow, listRC, rowOffset);
							m_headerRow->copyFrom(newDoc->m_headerRow, listRC, rowOffset);
						}
						m_rows->copyFrom(newDoc->m_rows, rht, listRC, rowOffset, colOffset);
					}
					sel->copyFrom(selOth_Doc, rowOffset, colOffset);

				} else if (selType == uoRst_Cell || selType == uoRst_Cells || selType == uoRst_Unknown) {
					if (matrixO_Doc){
						rowOffset = curCell.y()-1;
						colOffset = curCell.x()-1;
						int row_cnt = 0;
						int col_cnt = 0;
						int cellCount = 0;
						sel->clearSelections();


						uoCell *cell = 0, *cellOther = 0;
						/*  В случае матрицы у нас запомнен селекшинз и сама матрица,
						нужно только пробежаться и вставить с учетом текущей ячейки	*/
						for (row_cnt = 1; row_cnt <= matrixO_Doc->rows();  row_cnt++) {
							for (col_cnt = 1; col_cnt <= matrixO_Doc->cols();  col_cnt++) {
								cellCount += 1;
								cellOther = matrixO_Doc->cell(row_cnt, col_cnt);
								if (cellOther){
									cell = getCell(row_cnt + rowOffset, col_cnt + colOffset, true, true);
									cell->copyFrom(cellOther, this, newDoc, row_cnt + rowOffset);
									sel->selectCell(col_cnt + colOffset, row_cnt + rowOffset, false);
								}
							}
						}
					}
				}

				/* вместо того, что-бы заниматься форматированием по меcту, лучше имплементировать
				копирование uoReportSelection и сдвиг его на rowOffset и colOffset	*/
				doFormatDocWithSelection(sel);

				m_undoManager->groupCommandEnd();
			}
		}
	}
	delete newDoc;
}


void uoReportDoc::onCopy(uoReportSelection* sel, bool withDelete)
{
	if (!sel)
		return;
	QClipboard* clipBrd = qApp->clipboard();
	if (clipBrd){
		QByteArray byteArr;
		QMimeData* mime = new QMimeData;

		uoReportLoader* loader2 = uoReportLoader::getLoader(uoRsf_TXL);
		if (loader2) {
			QByteArray byteArr2;
			if (loader2->initBA(uorSO_SaveToBArray,byteArr2)){
				flush(loader2, sel);
			}
			loader2->finalize();
			byteArr2 = loader2->getByteArray();
			QString str = QString(byteArr2);
//			qDebug() << " buffer: " << str;
			mime->setText(str);
			delete loader2;
		} else {
			qWarning() << tr("Insufficient memory to copy");
			return;
		}

		uoReportLoader* loader = uoReportLoader::getLoader(uoRsf_XML);
		if (loader && mime) {
			loader->enableProcDialog(false);
			if (loader->initBA(uorSO_SaveToBArray,byteArr)){
				flush(loader, sel,withDelete);
			}
			loader->finalize();
			byteArr = loader->getByteArray();
			mime->setData(UOR_MIME_XML_DATA, byteArr);
			delete loader;
		} else {
			qWarning() << tr("Insufficient memory to copy");
			return;
		}

		clipBrd->setMimeData(mime);

	}
}


void uoReportDoc::onPaste(uoReportSelection* sel)
{
	if (!sel) return;

	QClipboard* clipBrd = qApp->clipboard();
	if (clipBrd){
		const QMimeData* mime = clipBrd->mimeData();
		if (mime){
			if (mime->hasFormat(UOR_MIME_XML_DATA))	{
				onPasteDocum(sel, mime);
			}else if (mime->hasFormat("text/csv") || mime->hasFormat("text/plain"))	{
				onPasteText(sel, mime);
			}
			++m_changes;
		}
	}
}
void uoReportDoc::onCut(uoReportSelection* sel){
	onCopy(sel, true);	
}

void uoReportDoc::onDeleteCells(uoReportSelection *sel){
	flush(0, sel, true);
}



/// перед добавлением строк или столбцов. Необходимо что-бы посчитать длину/ширину дока.
void uoReportDoc::beforeAddRowOrCol(int count, uoRptHeaderType rht, int noLn)
{
    Q_UNUSED(noLn);
	if (count <= 0)
		return;
	int oldCnt = 0;

	uoHeaderScale* header = NULL;
	if (rht == uorRhtRowsHeader) {
		header = m_headerRow;
		oldCnt = m_rowCount;
	} else {
		header = m_headerCol;
		oldCnt = m_colCount;
	}
	uorNumber itemSize 	 = uorNumberNull;
	uorNumber addSize 	 = uorNumberNull;
	uorNumber addSizeVis = uorNumberNull;
	for (int i = 1; i<=count; i++){
		itemSize = header->getSize(oldCnt + i);
		addSize = addSize + itemSize;
		if (!header->getHide(oldCnt + i)){
			addSizeVis = addSizeVis + itemSize;
		}
	}


	if (rht == uorRhtRowsHeader) {
		m_rowCount 		= m_rowCount + count;
		m_sizeV 			= m_sizeV + addSize;
		m_sizeV_visible 	= m_sizeV_visible + addSizeVis;
	} else {
		m_colCount 		= m_colCount + count;
		m_sizeH 			= m_sizeH + addSize;
		m_sizeH_visible 	= m_sizeH_visible + addSizeVis;
	}
	///\todo 1 А вот тут нужен сигнал на изменение размеров документа....
	if (m_freezEvent == 0)
		emit onSizeChange(m_sizeV_visible, m_sizeH_visible);
}

/// Изменить количество строк в документе
void uoReportDoc::doRowCountChange(int count, int pos)
{
    Q_UNUSED(pos);
	if (count == 0)
		return;
	uorNumber oldSize = m_sizeV;
	uorNumber oldSizeVis = m_sizeV_visible;

	m_rowCount = m_rowCount + count;
	m_sizeV = oldSize + count * getDefScaleSize(uorRhtRowsHeader);
	m_sizeV_visible = oldSizeVis + count * getDefScaleSize(uorRhtRowsHeader);
	if (m_freezEvent == 0)
		emit onSizeChange(m_sizeV_visible, m_sizeH_visible);
}

/// Изменить количество столбцов в документе
void uoReportDoc::doColCountChange(int count, int pos )
{
    Q_UNUSED(pos);
	if (count == 0)
		return;
	uorNumber oldSize = m_sizeH;
	uorNumber oldSizeVis = m_sizeH_visible;

	m_colCount = m_colCount + count;
	m_sizeH = oldSize + count * getDefScaleSize(uorRhtColumnHeader);
	m_sizeH_visible = oldSizeVis + count * getDefScaleSize(uorRhtColumnHeader);
	if (m_freezEvent == 0)
		emit onSizeChange(m_sizeV_visible, m_sizeH_visible);

}


/// При доступе к строке или ячейке на запись. Служит для определения длины/ширины документа.
void uoReportDoc::onAccessRowOrCol(int nom, uoRptHeaderType rht, bool write)
{
	int cnt = 0;
	if (rht == uorRhtColumnHeader) // Колонка
	{
		if (m_colCount < nom) {
			cnt = nom - m_colCount;
			doColCountChange(cnt, m_colCount+1);
			if (write)
				m_colCount = nom;

		}
	} else if (rht == uorRhtRowsHeader) {		// строка
		if (m_rowCount < nom) {
			cnt = nom - m_rowCount;
			doRowCountChange(cnt, m_rowCount+1);
			if (write)
				m_rowCount = nom;
		}
	}
}

void uoReportDoc::onAccessRowCol(int nmRow, int nmCol)
{
	if (nmRow > 0)		onAccessRowOrCol(nmRow, uorRhtRowsHeader);
	if (nmCol > 0)		onAccessRowOrCol(nmCol, uorRhtColumnHeader);
}


/// Вернуть размер ячейки
uorNumber uoReportDoc::getScaleSize(const uoRptHeaderType& hType, const int& nom, bool isDef)
{
	if (hType == uorRhtRowsHeader)
		return m_headerRow->getSize(nom, isDef);
	else
		return m_headerCol->getSize(nom, isDef);
}

/// Вернуть размер диапазона ячек
uorNumber uoReportDoc::getScalesSize(const uoRptHeaderType& hType, const int& nomStart, const int& nomEnd, const bool& ignoreHiden, const bool& isDef) const
{
	uoHeaderScale* pHeader = NULL;
	if (hType == uorRhtRowsHeader) {
		pHeader = m_headerRow;
	} else {
		pHeader = m_headerCol;
	}
	uorNumber retVal = uorNumberNull;
	int nmStart = qMin(nomStart,nomEnd), nmEnd = qMax(nomStart,nomEnd), i = 0;
	for (i = nmStart; i<=nmEnd; i++){
		if (pHeader->getHide(i) && ignoreHiden)
			continue;
		retVal = retVal + pHeader->getSize(i, isDef);
	}
	return retVal;
}

/// Установить размер ячейки
void uoReportDoc::setScaleSize(const uoRptHeaderType& hType, const int& nom, uorNumber size, bool isDef){
	bool scVisible = true;
	uorNumber oldSizeItem = uorNumberNull, oldSize = uorNumberNull;
	if (hType == uorRhtRowsHeader) {
		oldSizeItem = m_headerRow->getSize(nom, isDef);
		scVisible = m_headerRow->getHide(nom);

		if (m_undoManager){		m_undoManager->doScaleResize(hType,nom,oldSizeItem);		}

		m_headerRow->setSize(nom, size, isDef);
		++m_changes;
		m_sizeV = m_sizeV - oldSizeItem + size;
		if (!scVisible) {
			oldSize = m_sizeV_visible;
			m_sizeV_visible = m_sizeV_visible - oldSizeItem + size;
			if (nom <= m_rowCount)
				emit onSizeVisibleChangeV(m_sizeV_visible, m_rowCount, oldSize, m_rowCount, nom);
		}
	}
	else {
		oldSizeItem= m_headerCol->getSize(nom, isDef);

		if (m_undoManager){		m_undoManager->doScaleResize(hType,nom,oldSizeItem);		}

		m_headerCol->setSize(nom, size, isDef);
		++m_changes;
		m_sizeH = m_sizeH - oldSizeItem + size;
		scVisible = m_headerCol->getHide(nom);
		if (!scVisible) {
			oldSize = m_sizeH_visible;
			m_sizeH_visible = m_sizeH_visible - oldSizeItem + size;
			if (nom <= m_colCount)
				emit onSizeVisibleChangeH(m_sizeH_visible, m_colCount, oldSize, m_colCount, nom);
		}
	}
	onAccessRowOrCol(nom-1, hType); // последнюю ячейку не трогаем, т.к. её размер устанавливается...
}

/// Установка размера для группы или одного столбца/строки.
void uoReportDoc::setScalesSize(const uoRptHeaderType& hType, const QList<int>& list, const uorNumber& size, const bool& isDef)
{
	if (list.size() == 0 || size < uorNumberNull)
		return;
	bool scVisible = true;

	uoHeaderScale* heider = m_headerRow;
	if (hType == uorRhtColumnHeader)
		heider = m_headerCol;
	int itemNo = 0;

	uorNumber oldSizeItem = uorNumberNull, oldSize = uorNumberNull;
	QList<int>::const_iterator it = list.constBegin();
	while(it != list.constEnd()){
		itemNo = *it;

		oldSizeItem = heider->getSize(itemNo, isDef);
		scVisible = heider->getHide(itemNo);

		if (m_undoManager && size != oldSizeItem){		m_undoManager->doScaleResize(hType,itemNo,oldSizeItem);		}

		heider->setSize(itemNo, size, isDef);
		++m_changes;
		if (hType == uorRhtRowsHeader)
			heider->setFixed(itemNo,true);
		m_sizeV = m_sizeV - oldSizeItem + size;
		if (!scVisible) {
			oldSize = m_sizeV_visible;
			m_sizeV_visible = m_sizeV_visible - oldSizeItem + size;
		}
		it++;
	}

	onAccessRowOrCol(itemNo-1, hType); // последнюю ячейку не трогаем, т.к. её размер устанавливается...
}

/// Сбрасываем фиксированные размеры у списка строк.
void uoReportDoc::setRowAutoSize(const QList<int>& list)
{
    Q_UNUSED(list);
}
/**
	Установка свойства, что строка иммет фиксированную высоту,
	т.е. при изменении текста её размер не меняется
	Примечание. бесполезно для столбцов, они не будут подгоняться
	"под размер" текста. Ну по крайней мере пока...
*/
void uoReportDoc::setScaleFixedProp(const uoRptHeaderType& hType, const int& nom, bool isFixed)
{
	if (hType == uorRhtRowsHeader) {
		m_headerRow->setFixed(nom, isFixed);
		++m_changes;
	}
	/// для колонок собственно безсмысленно...
}

bool uoReportDoc::getScaleFixedProp(const uoRptHeaderType& hType, const int& nom)
{
	if (hType == uorRhtRowsHeader) {
		return m_headerRow->getFixed(nom);
	}
	return false;
}


/// Прячем/Показываем диапазон ячеек...
void uoReportDoc::setScalesHide(uoRptHeaderType hType, int nmStart, int cnt,  bool hide){
	uoHeaderScale* header = NULL;
	if (hType == uorRhtRowsHeader) {
		header = m_headerRow;
	} else {
		header = m_headerCol;
	}
	uorNumber szAdd = uorNumberNull;
	for (int i = 0; i<cnt; i++)	{
		szAdd = szAdd + header->getSize(nmStart + i);
		header->setHide(nmStart + i, hide);
		++m_changes;
	}
	uorNumber oldSize = uorNumberNull;
	szAdd = szAdd * ( hide ? -1 : 1);
	if (hType == uorRhtRowsHeader){
		oldSize = m_sizeV_visible;
		m_sizeV_visible = m_sizeV_visible + szAdd;
		if (m_freezEvent == 0)
			emit onSizeChange(m_sizeV_visible, m_sizeH_visible);
	} else {
		oldSize = m_sizeH_visible;
		m_sizeH_visible = m_sizeH_visible + szAdd;
		if (m_freezEvent == 0)
			emit onSizeChange(m_sizeV_visible, m_sizeH_visible);
	}

	onAccessRowOrCol(nmStart + cnt - 1, hType);
}

/// Нужно получить следующую занятую ячейку в строке отноительно предыдущей...
int uoReportDoc::getNextCellNumber(const int& rowCur, const int& colCur, const bool& ignoreHiden)
{
    Q_UNUSED(ignoreHiden);
	int retVal = -1;

	uoRow* row = m_rows->getRow(rowCur);
	if (row) {
		row->getNextItemNom(colCur);
	}

	return retVal;
}

/// пошинковать строку на слова не превышающие длинны collWidth
QStringList uoReportDoc::splitQStringToWord(const QFontMetricsF& fm, const QString& str, const uorNumber& collWidth)
{
	QStringList list;
	QString curStr, curWord;
	QChar qchar;
	uorNumber lenStr = uorNumberNull, lenCurW = uorNumberNull;

	QStringList listW = str.split(QRegExp("\\b"));
	for (int i = 0; i<listW.count(); i++)
	{
		curWord = listW.at(i);
		if (curWord.isEmpty())
			continue;

		lenCurW = (uorNumber)fm.width(curWord);
		if ((lenStr + lenCurW)>collWidth){
			if (!curStr.isEmpty()) {
				list.append(curStr);
				curStr = "";
				lenStr = uorNumberNull;
			}
			if (lenCurW > collWidth) {
				// а вдруг длинна слова больше чем общая ширина?
				QString ww;
				QChar chr;
				int ii = 0; curWord.length();
				while(ii<curWord.length())
				{
					chr = curWord.at(ii);
					if ((fm.width(chr) + fm.width(ww))>collWidth){
						list.append(ww);
						ww = "";
					}
					ww.append(chr);
					ii = ii + 1;
				}
				curStr.append(ww);
			} else {
				curStr.append(curWord);
			}
		} else {
			curStr.append(curWord);
		}
		lenStr = (uorNumber)fm.width(curStr);
	}
	if (!curStr.isEmpty())
		list.append(curStr);
	return list;
}


/// Форматирование текста ячейки, одновременно расчет её высоты..
uorNumber uoReportDoc::doFormatCellText(uoCell* cell, QFont* font, QFontMetricsF& fm, const uorNumber& collWidth)
{
	uorNumber cellHeight = uorNumberNull;
	uorNumber collWidthThis = collWidth - (uorNumber)(UORPT_STANDART_OFFSET_TEXT*2); // 2 офсета, справ и слева.
    Q_UNUSED(font);

	// задачка не из простых...
	QString cellStr = cell->getText();
	if (cell->m_textProp){
		m_pointBlock->savePoint(cell->m_textBoundary);
		cell->m_textBoundary = NULL;
		cell->m_maxRowLen = uorNumberNull;
	}

	if (cellStr.isEmpty())
		return cellHeight;


	cellStr = cellStr.replace('\t', " ");
	uoCellTextBehavior 	tb = cell->getTextBehavior();
	/*
		походу очень плохая идея рубить текст и оставлять его порубленным в ячейке.
		лучше уж определить масив сплитеров и рубить его при выводе...
	*/

	QChar chrLS = '\n';
	int pos = cellStr.indexOf(chrLS, 0);
	QStringList strListReal;
	int listSz = 0, breakCount = 0;

//	uorNumber fullLength = fm.width(cellStr); // нужно проверить, влезает ли текст в сейку..
	QString nextStr, curStr;
	if (pos != -1 || tb == uoCTB_Transfer){
		// Значит надо рубить
		QStringList strList = cellStr.split(chrLS);
		listSz = strList.count();

		for (int i = 0; i<strList.count(); i++) {
			nextStr = strList.at(i);

			if ((i+1)<listSz){
				nextStr = nextStr + "\n";
				if (tb == uoCTB_Transfer)
					breakCount += 1;
			}
			if (tb == uoCTB_Transfer){
				if (fm.width(cellStr) > collWidthThis){
					// надо сплитануть по словам: по табу, пробелу, знакам препинания и т.п.
					// незабыть стандартный офсет, служащий для антиприлипания строки.
					QStringList listOfWord = splitQStringToWord(fm, nextStr, collWidthThis);
					strListReal << listOfWord;
				} else {
					strListReal << nextStr;
				}
			} else {
				strListReal << nextStr;
			}
		}
	} else {
		strListReal << cellStr;
	}
	uorNumber maxRowLength = uorNumberNull;
	for (int i = 0; i < strListReal.size(); ++i){
		nextStr = strListReal.at(i);
		maxRowLength = qMax(maxRowLength, (uorNumber)fm.width(nextStr));
	}


	cell->setMaxRowLength(maxRowLength, this);
	cell->applyTrPoint(m_pointBlock, strListReal, this);
	listSz = strListReal.size() + breakCount;
	cellHeight = (uorNumber)(fm.height() * listSz);
	bool hasJoin = cell->isUnionHas();
	cell->m_height = cellHeight;
	if (hasJoin){
		cellHeight = (uorNumber)fm.height();
		if (cell->unionRow() == 1){
			cellHeight = cell->m_height;// если объединение включает 1 строку, почему бы и не растянуть?
		}
	}

	return cellHeight;
}

bool uoReportDoc::enableFormating(const bool format)
{
	bool old = m_formatEnable;
	m_formatEnable = format;
	return old;
};

/// Нужно расчитать высоту и ширину ректа объединения ячейки...
void uoReportDoc::doCalculateCellUnionSizeHW(uoCell* psCell)
{
	if (!psCell)
		return;
	uoCell* cell = psCell;
	uoCell* cellBP = 0; // Бекпоинт селс. для правки координат.

	QRect cellRect;
	uoCellsJoinType unionType = cell->unionType();
	if (unionType == uoCJT_Unknown) {
		return;
	}
	if (unionType == uoCJT_BackPoint) {
		cellRect = cell->getCellJoinRect();
		cell = getCell(cellRect.top(), cellRect.left(), false);
		if (!cell)
			return;
	}
	uoCellJoin* src = 0;
//	uoCellJoin* firstJn = 0;
	cellRect = cell->getCellJoinRect();
	unionType = cell->unionType();
	if (unionType == uoCJT_Normal || unionType == uoCJT_TextToCol) {
		uoCellJoin* cJoin =  cell->cellJoin();
		if (!cJoin)
			return;
		int rowCntr = 0, colCntr = 0;
		cJoin->m_height = uorNumberNull;
		cJoin->m_width = uorNumberNull;

		for (rowCntr = cellRect.top(); rowCntr <= cellRect.bottom() ;rowCntr++){
			if (!getScaleHide(uorRhtRowsHeader, rowCntr))
				cJoin->m_height += getScaleSize(uorRhtRowsHeader, rowCntr);
		}
		for (colCntr = cellRect.left(); colCntr <= cellRect.right() ;colCntr++){
			if (!getScaleHide(uorRhtColumnHeader, colCntr))
			cJoin->m_width += getScaleSize(uorRhtColumnHeader, colCntr);
		}

		/* Ситуевинко. Когда выполняется копирование/вставка, то ячейки копируются
		так-же копируется и бекпоинт координаты. После вставки тут можно поправить дело.
		так тут и поправим.		*/
		for (rowCntr = cellRect.top(); rowCntr <= cellRect.bottom() ;rowCntr++){
			for (colCntr = cellRect.left(); colCntr <= cellRect.right() ;colCntr++){
				cellBP = getCell(rowCntr, colCntr);
				if (cellBP && cellBP->unionType() == uoCJT_BackPoint){
					src = cellBP->cellJoin();
					if (src){
						src->m_col = cellRect.left();
						src->m_row = cellRect.top();
						src->m_cellRect = QRect(cellRect.left(),cellRect.top(), cJoin->m_col, cJoin->m_row);
					}
				}
			}
		}
	}
}


/**
	Просчитаем для строки максимальную длину, которую хочет её текст.
*/
void uoReportDoc::doFormatRowLRMaxLng(uoRow* row)
{
	if (!row)
		return;
	// Посчитаем для строки лефты, райты.
	row->m_lengthMaxToLeft = uorNumberNull;
	row->m_lengthMaxToRight = uorNumberNull;
	row->m_lengthFromCell = uorNumberNull;
	row->m_unionCount = 0;

	QMap<int, uorNumber> mapColSize;
	QMap<int, uorNumber> mapColToLeng;
	QList<int> hashNomUnion;
	QRect curentUnion;
	mapColSize[0] = 0;
	mapColToLeng[0] = 0;

	int cellNo = 0, cellNoLast = 0;
	uorNumber collSize = uorNumberNull, collSizeAll = uorNumberNull, collSizeAllTmp = uorNumberNull, textCellMaxLeng = uorNumberNull;

	uoHorAlignment textHorAl = uoHA_Unknown;
	uoCellsJoinType unionType ;

	uoCell* cell = row->getFirst();
	QList<uoCell*> unionCells;
	while(cell){
		cellNo = cell->number();
		while(cellNoLast < cellNo){
			cellNoLast += 1;
			collSize = getScaleSize(uorRhtColumnHeader,cellNoLast);
			collSizeAll += collSize;
			mapColSize[cellNoLast] = collSize;
			mapColToLeng[cellNoLast] = collSizeAll;
			row->m_lengthFromCell = collSizeAll;
		}
		textHorAl = cell->getAlignmentHor();

		textCellMaxLeng = cell->getMaxRowLength();
		collSize = mapColSize[cellNo];
		if (textCellMaxLeng > collSize){
			textHorAl = cell->getAlignmentHor();
			if (textHorAl == uoHA_Left){
				collSizeAllTmp = textCellMaxLeng + mapColToLeng[cellNo-1];
				collSizeAllTmp = qMax(uorNumberNull, collSizeAllTmp);
				row->m_lengthMaxToRight = qMax(row->m_lengthMaxToRight, collSizeAllTmp);
			} else if (textHorAl == uoHA_Right){
				collSizeAllTmp = textCellMaxLeng - mapColToLeng[cellNo];
				collSizeAllTmp = qMax(uorNumberNull, collSizeAllTmp);
				row->m_lengthMaxToLeft = qMax(row->m_lengthMaxToLeft, collSizeAllTmp);
			}
		}
		if (cell->isUnionHas()) {
			// Нужно считать только уникальные объединения
			unionType = cell->unionType();
			if (unionType == uoCJT_Normal || unionType == uoCJT_TextToCol){
				row->m_unionCount += 1;
				unionCells.append(cell); // Что-бы не сбить выборку итератора, он самопальный.
			} else if (unionType == uoCJT_BackPoint){
				curentUnion = cell->getCellJoinRect();
				if (curentUnion.left() != 0){
					if (cellNo == curentUnion.left())
						row->m_unionCount += 1;
						unionCells.append(cell); // Что-бы не сбить выборку итератора, он самопальный.
				}
			}
		}
		cell = row->getNext();
	}
	while(!unionCells.isEmpty()){
		cell = unionCells.takeFirst();
		if (cell->isUnionHas()) {
			// Нужно считать только уникальные объединения
			unionType = cell->unionType();
			if (unionType == uoCJT_Normal || unionType == uoCJT_TextToCol){
				doCalculateCellUnionSizeHW(cell);
			} else if (unionType == uoCJT_BackPoint){
				curentUnion = cell->getCellJoinRect();
				if (curentUnion.left() != 0){
					if (cellNo == curentUnion.left())
						doCalculateCellUnionSizeHW(cell);
				}
			}
		}
	}
}


/**
	Форматирование строки: \n
		- вычисление максимальной ВЫСОТЫ строки.
		- вычисление переносов \n
		- вычисление ВЫСОТЫ, переносов и т.п. \n
*/
void uoReportDoc::doFormatRow(uoRow* row, int nmForCol /* = -1*/ )
{
	/*
		Задача по форматированию:
		- вычисление максимальной высоты строки	если её размер не зафиксирован.
		- вычисление переносов текста в ячейках.
	*/
	if (!row)
		return;
	if (!m_formatEnable)
		return;

	//QList<int> cellsNumbers = row->getItemNumList();
	uorNumber rowMinSize = m_headerRow->getDefSizeItem();
	uorNumber rowRealSize = rowMinSize;
	int nmRow = row->number();
	m_pictCasher->pixmapClear(nmRow);
	if (row->isEmpty())	{
		// если мы удалили все содержимое, строку надо форматнуть, если её размер отличается от дефолтного
		if(!m_headerRow->getFixed(nmRow)){
			if (m_headerRow->getSize(nmRow) != rowMinSize) {
				m_headerRow->setSize(nmRow,rowMinSize);
			}
		}
		return;
	}
	uoCell* cellPrev = 0;
	uoCell* cell 	= 0;
	uoCell* cellNext = 0;

	QString cellText;
	QFont* font;
	int cellPrevNo = -1;
	int cellCurNo = -1;
	int cellNextNo = -1;

	int cellCounts = row->getMaxNo();

	uorNumber collSize = uorNumberNull; //getScaleSize(uorRhtRowsHeader
	uorNumber collsSizeTS = uorNumberNull; //getScaleSize(uorRhtRowsHeader

	uorNumber cellHeight = 0;
	uoCellTextBehavior textBehav = uoCTB_Auto;
	uoHorAlignment textHorAl = uoHA_Left;
	bool hasUnion = false;
	uoCellsJoinType unionType = uoCJT_Unknown;
	int unionCols = 0;

	bool collBeFormated = false;

	if(nmForCol<=0) {
		collBeFormated = true;
		row->m_lengthMaxOver = uorNumberNull;
	}

	for (int i = 1; i<=cellCounts; i++){

		if (cell) {
			cellPrev = cell;
		}
		cell = row->getItem(i, false);
		cellNext = NULL;
		if (cell){
			cellNext = row->getNextItem(cell->number());
			if (cellNext)
				cellNextNo = cellNext->number();
		} else {
			continue;
		}
		cellCurNo = i;

		hasUnion = false;
		unionCols = 0;

		collSize = getScaleSize(uorRhtColumnHeader,cellCurNo);
		collsSizeTS = collSize;

		if ((nmForCol > 0 && nmForCol != cellCurNo) || (!cell)) {
			// нужно еще проверить ячейки на объединение...
			if (cell)
				rowRealSize = qMax(cell->m_height,rowRealSize);
			rowRealSize = qMax(rowRealSize, rowMinSize);
			continue;
		}
		unionType = cell->unionType();
		if (unionType == uoCJT_BackPoint){
			continue;
		}
		hasUnion = (unionType == uoCJT_Unknown); hasUnion = !hasUnion;
		if (hasUnion)
			unionCols = cell->unionCol();

		cell->m_height = uorNumberNull;

		collBeFormated = true;

		if (cellPrevNo == -1 && cellCurNo>1)
			cellPrevNo = 1;


		textBehav = cell->getTextBehavior();
		textHorAl = cell->getAlignmentHor();

		font = cell->getFont(this);
		cellText = cell->getText();

		if (font && !cellText.isEmpty()) {

			// тут уже можно хотя бы посчитать минимальную высоту строки. что и делаем...
			QFontMetricsF fm(*font);
			rowMinSize = qMax(rowMinSize, (uorNumber)fm.height());

			if (hasUnion){
				collsSizeTS = getScalesSize(uorRhtColumnHeader, cellCurNo, cellCurNo + unionCols - 1, true);
			} else {
				// а тут можно переформатировать текст
				if (cellNextNo != -1 && textBehav == uoCTB_Auto) {
					if (textHorAl == uoHA_Center){
						if ((cellPrevNo != -1) && (cellNextNo != -1)) {
							collsSizeTS = getScalesSize(uorRhtColumnHeader, qMin(cellPrevNo+1, cellCurNo), qMax(cellCurNo,cellNextNo-1), true);
						} else if (cellNextNo != -1){
							collsSizeTS = getScalesSize(uorRhtColumnHeader, cellCurNo, qMax(cellCurNo,cellNextNo-1), true);
						} else if (cellPrevNo != -1){
							collsSizeTS = getScalesSize(uorRhtColumnHeader, qMin(cellPrevNo+1, cellCurNo), cellCurNo, true);
						}
					} else if (textHorAl == uoHA_Left) {
						if (cellNextNo != -1) {
							collsSizeTS = getScalesSize(uorRhtColumnHeader, cellCurNo, qMax(cellCurNo,cellNextNo-1), true);
						} else {
							collsSizeTS = getScalesSize(uorRhtColumnHeader, cellCurNo, cellCurNo, true);
						}
					} else if (textHorAl == uoHA_Right) {
						if (cellPrevNo != -1) {
							collsSizeTS = getScalesSize(uorRhtColumnHeader, qMin(cellPrevNo+1, cellCurNo), cellCurNo, true);
						} else {
							collsSizeTS = getScalesSize(uorRhtColumnHeader, cellCurNo, cellCurNo, true);
						}
					}
				}
			}
			cellHeight = doFormatCellText(cell, font, fm, collsSizeTS);
			if (cell->getMaxRowLength()>collSize){
				row->m_lengthMaxOver = qMax(row->m_lengthMaxOver, cell->getMaxRowLength() - collSize);
			}



			rowRealSize = qMax(cellHeight,rowRealSize);
			rowRealSize = qMax(rowRealSize, rowMinSize);


		}
		/*
			есть несколько вариантов выравнивания текста по горизонтали:
			влево, [<=   ]
			вправо, [   =>]
			центр, [  <=>  ]
			а так же признак "по выделенным столбцам". влево вправо или по центру.
			[  <=  ][  =  ][  =  ][  =  ][  =>  ]
			нужно вычислить как именно выравнивается текст и в какие рамки он может уместиться..
			если он достаточно длинен, и имеет признак выравнивания авто,
			то его можно напечатать и на соседних ячейках, не прорисовывая их..
			ДЛЯ НАЧАЛА - попробуем пересчитать высоту строки при изменениии ширины столбца.
		*/
		cellPrevNo = cellCurNo; // должно быть вконце....
	}
	if (collBeFormated && !m_headerRow->getFixed(nmRow)) {
		m_headerRow->setSize(nmRow,rowRealSize);
	}
	doFormatRowLRMaxLng(row);

	///\todo а вот тут надо гавкнуть, что-бы перещитали ректы, если документ подсоединен ко вьюву...

}

/// Форматирование строки: вычисление ВЫСОТЫ, переносов и т.п.
void uoReportDoc::doFormatRow(int nmRow, int nmForCol)
{

	uoRow* row = m_rows->getRow(nmRow, false);
	if (!row)
		return;
	doFormatRow(row, nmForCol);
}

/// Форматирование документа..
///\todo 1 имплементировать форматирование по переданному m_selections после установки/изменения свойств.
void uoReportDoc::doFormatDoc(int nmRow /*= -1*/, int nmForCol /*= -1*/)
{
	if (m_rows->getCountItem() <=0 && m_headerCol->getCountItem() <=0 && 	m_headerRow->getCountItem() <= 0)
		return;
	bool recalcCntRowCol = false;

	if (nmRow == -1 && nmForCol == -1) {
		recalcCntRowCol = true;
		m_rowCount = m_colCount = 0;
	}

	uoRow* row = NULL;
	int curRowNum = m_rows->getMinNo();
	if (nmRow > 0){
		row = m_rows->getItem(nmRow, false);
		doFormatRow(row, nmForCol);
	} else {

		int rowMaxNo = m_rows->getMaxNo();
		m_rowCount = rowMaxNo;
		while(curRowNum <= rowMaxNo){
			row = m_rows->getItem(curRowNum, false);
			if (row){
				m_rowCount = qMax(m_rowCount, row->number());
				m_colCount = qMax(row->getMaxNo(), m_colCount);
				doFormatRow(row, nmForCol);
				qApp->processEvents(/*QEventLoop::ExcludeUserInputEvents*/);
			}
			++curRowNum;
		}
	}
}

/// Форматирование с передачей selection, полезно на случай установки свойств.
void uoReportDoc::doFormatDocWithSelection(uoReportSelection * selection)
{
	if (!selection)
		return;
	if (m_rows->getCountItem() <=0 && m_headerCol->getCountItem() <=0 && 	m_headerRow->getCountItem() <= 0)
		return;
	int nmRow = 0, nmCol = 0;

	/*
		это была неплохая оптимизация, есть маленькие баги,
		обструкт не принимался, видно неправильно с селекшионз работает,
		ну да пока отложу, как ника 2 ночи О_о...
	*/

	uorSelectionType selMode = selection->selectionType();
//	uoRow* row = NULL;
	if (selMode == uoRst_Unknown || selMode == uoRst_Cell) {
		selection->currentCell(nmRow, nmCol);
		doFormatDoc(nmRow, nmCol);
	} else if (selMode == uoRst_Column || selMode == uoRst_Columns) {
		for(int i = 1; i<=m_colCount; i++){
			if (selection->isColSelect(i))
				doFormatDoc(-1, i);
		}


	} else if (selMode == uoRst_Row || selMode == uoRst_Rows) {
		for(int i = 1; i<=m_rowCount; i++){
			if (selection->isRowSelect(i))
				doFormatDoc(i, -1);
		}

	} else if (selMode == uoRst_Cells) {
		QRect cb = selection->getSelectionBound();
		for(int i = cb.top(); i<=cb.bottom(); i++){
			if (selection->isRowSelect(i) || selection->isRowPartlySelect(i))
				doFormatDoc(i, -1);
		}
	} else {
		doFormatDoc(-1, -1);
	}
}


/// Установить текст в ячейку
void uoReportDoc::setCellText(const int& row, const int& col, const QString& text, uorCellTextType type)
{
    Q_UNUSED(type);
	if (m_undoManager->isCollectChanges()){
		QString oldText = m_rows->getText(row, col);
		m_undoManager->doTextChange(oldText, row, col);
	}
	m_rows->setText(row, col, text);	// форматирование? угу.
	++m_changes;
	onAccessRowOrCol(row, uorRhtRowsHeader, true);
	onAccessRowOrCol(col, uorRhtColumnHeader, true);
	if (m_formatEnable)
		doFormatRow(row, col);
}

/// Установить выравнивание текста в ячейке
void uoReportDoc::setCellTextAlignment(const int rowNo, const int colNo, uoVertAlignment va,  uoHorAlignment ha, uoCellTextBehavior tb)
{
	uoCell* cell = m_rows->getCell(rowNo, colNo, true);
	if (cell){
		cell->setAlignment(va,  ha, tb, this);
		++m_changes;
		doFormatRow(rowNo, colNo);
	}
}

QString uoReportDoc::getCellText(const int& row, const int& col, uorCellTextType type){
    Q_UNUSED(type);
	return 	m_rows->getText(row, col);
}

uoCell* uoReportDoc::getCell(const int& rowNo, const int& colNo, bool needCreate, bool provideProp){
	uoCell* cell = m_rows->getCell(rowNo, colNo, needCreate);
	if (cell && provideProp)
		cell->provideAllProps(this, provideProp);
	if (needCreate){
		onAccessRowOrCol(rowNo, uorRhtRowsHeader, true);
		onAccessRowOrCol(colNo, uorRhtColumnHeader, true);
	}
	return cell;
}

uoRow* uoReportDoc::getRow(const int& rowNum)
{
	Q_ASSERT(rowNum);
	return m_rows->getRow(rowNum, false);
}

/**
	Вычислим нормальный рект для страницы с учетом настроек страницы
	и принтера.
*/
bool uoReportDoc::updatePageRectPrint()
{
	bool retVal = false;
	QPrinter* pPrinter = uoReportManager::instance()->printer();
	if(!pPrinter){
		qWarning() << QString::fromUtf8("Принтер не создан!");
		return retVal;
	}

	bool debug_this = true;
	m_pagesSetings->normalize();

	qreal scaleFactor = m_pagesSetings->scale();
	if (scaleFactor == 0.0)
        updatePageSegmentList();
    if (scaleFactor == 0.0)
        return retVal;

	qreal scaleFactorO = 1 / scaleFactor;
	/*
		тут одна тонкость: если в параметрах страницы стоит "по ширине листа",
		тогда scaleFactor надо пересчитать в зависимости от ширины листа вправо.
	*/
	qreal pogreshn = 20.0;

	pPrinter->setPageSize(m_pagesSetings->m_paperSize);
	pPrinter->setOrientation(m_pagesSetings->m_orientation);

	QRectF pageRect = pPrinter->pageRect();
	m_paperRectPrint = pPrinter->paperRect();
	int pDpi = pPrinter->resolution();
	/* dpi количество точек на линейный дюйм
		дюйм - 2,5399931 см
	*/
	qreal kDpi = (uorNumber)(pDpi / 25.399); /// точек в милиметре.
	//qreal kDpi_fl = (pDpi / 25.399);
	QString message;
	qreal fldSz = m_pagesSetings->m_fieldLeft_mm + m_pagesSetings->m_fieldRight_mm;
	fldSz = int(fldSz * kDpi);
	if (fldSz + pogreshn >pageRect.width()){
		message = QString::fromUtf8("Левое и правое поля черезчур велики!");
		qWarning() << message;
		return retVal;
	}
	fldSz = m_pagesSetings->m_fieldBottom_mm + m_pagesSetings->m_fieldTop_mm;
	fldSz = fldSz * kDpi;

	if (fldSz + pogreshn >pageRect.height()){
		message = QString::fromUtf8("Верхние и нижние поля поля черезчур велики!");
		qWarning() << message;
		return retVal;
	}

	fldSz =
	m_pagesSetings->m_fieldBottom_mm +
	m_pagesSetings->m_fieldTop_mm +
	m_pagesSetings->m_titleBotSize_mm +
	m_pagesSetings->m_titleTopSize_mm;
	int fld_mm1 = 0, fld_mm2 = 0;

	fldSz = int(fldSz * kDpi);

	if (fldSz + pogreshn > pageRect.height()){
		message = QString::fromUtf8("Верхние и нижние c колонтитулами поля поля черезчур велики!");
		qWarning() << message;
		return retVal;
	}

	m_pageRectPrint = QRect(0,0,0,0);
	bool badField = false;


	fldSz = int(m_pagesSetings->m_fieldTop_mm * kDpi);
	if (pageRect.y() != fldSz && pageRect.y() < fldSz) {
		m_pageRectPrint.setY(fldSz);
	} else {
		fld_mm1 = fldSz / kDpi;
		fld_mm2 = pageRect.y() / kDpi;
		message = QString::fromUtf8("Скорректировали верхнее поле %1 >> %2: слишком маленькое!").arg(fld_mm1).arg(fld_mm2);
		m_pageRectPrint.setY(qMax(fldSz, pageRect.y()));
		m_pagesSetings->m_fieldTop_mm = fld_mm2;
		qWarning() << message;
		//m_pageRectPrint.setY(pageRect.y());
	}

	fldSz = int(m_pagesSetings->m_fieldLeft_mm * kDpi);
	if (pageRect.x() != fldSz && pageRect.x() < fldSz) {
		m_pageRectPrint.setX(fldSz);
	} else {
		fld_mm1 = fldSz / kDpi;
		fld_mm2 = pageRect.x() / kDpi;
		message = QString::fromUtf8("Скорректировали левое поле %1 >> %2: слишком маленькое!").arg(fld_mm1).arg(fld_mm2);
		qWarning() << message;
		m_pageRectPrint.setY(qMax(fldSz, pageRect.x()));
		m_pagesSetings->m_fieldLeft_mm = fld_mm2;
		//m_pageRectPrint.setX(pageRect.x());
	}

	fldSz = int(m_paperRectPrint.width() - (m_pagesSetings->m_fieldRight_mm * kDpi) - m_pageRectPrint.x());
	if (fldSz+pogreshn > 0) {
		m_pageRectPrint.setWidth(fldSz);
	} else {
		qWarning() << QString::fromUtf8("Ширина слишком маленькая!");
		badField = true;
	}

	fldSz = int(m_paperRectPrint.height() - (m_pagesSetings->m_fieldTop_mm * kDpi) - m_pageRectPrint.y());
	if (fldSz+pogreshn > 0) {
		m_pageRectPrint.setHeight(fldSz);
	} else {
		qWarning() << QString::fromUtf8("Высота слишком маленькая!");
		badField = true;
	}
	debug_this = false;
	if (debug_this) {
		qDebug() << QString::fromUtf8("Расчет ректов страницы{");
		qDebug() << "pageRect" << pageRect;
		qDebug() << "m_paperRectPrint" << m_paperRectPrint;
		qDebug() << "m_pageRectPrint" << m_pageRectPrint;
		qDebug() << QString::fromUtf8("}Расчет ректов страницы");
	}
	m_pageRectPrint.adjust(-1,-1,1,1);

	if (debug_this) {
		qDebug() << QString::fromUtf8("Расчет ректов страницы ++{");
		qDebug() << "scaleFactor" << scaleFactor;
		qDebug() << "scaleFactorO" << scaleFactorO;
		qDebug() << "m_pageRectPrint" << m_pageRectPrint;
		qDebug() << QString::fromUtf8("}Расчет ректов страницы++");
	}
	if (badField){
		return retVal;
	}

	return true;
}

void uoReportDoc::updatePageSegmentList()
{
	bool debug_this = false;
	while (!m_segmentList.isEmpty())     delete m_segmentList.takeFirst();

	qreal scaleFactor = m_pagesSetings->scale();
	/*
		тут одна тонкость: если в параметрах страницы стоит "по ширине листа",
		тогда scaleFactor надо пересчитать в зависимости от ширины листа вправо.
	*/
	const bool toWidthOfSheet = m_pagesSetings->widthOfSheet();

	uorPageColSegment* segment = new uorPageColSegment(1,1,1);
	m_segmentList.append(segment);
	m_pagesColumnTotal = 1;
	QRect pageRectPrintCpy = m_pageRectPrint;
	pageRectPrintCpy.adjust(2,2,-2,-2); // нужно правильно печатать толстые рамки, иначе края режутся.
	/*
		неправильно расчитываются сегменты.
		проблема вот в чем: если есть очнь длинная строка
		и она перелезает на следующий лист сегменты это неподхватывают.
		Нужно высчитать максимальную ширину страницы.
		ОК. решил проблему в функции форматирования строки.doFormatRowLRMaxLng(row);
	*/

	uorNumber rigthLengMax = uorNumberNull;
	uoRow* row = m_rows->getFirst();
	m_rowCount = m_colCount = 0; // подпересчитаем.
	while(row){
		rigthLengMax = qMax(rigthLengMax, row->m_lengthMaxToRight);
		m_rowCount = qMax(m_rowCount, row->number());
		m_colCount = qMax(m_colCount, row->getMaxNo());
		row = m_rows->getNext();
	}

	int nmColCntr = 0;
	qreal offset = 0.0;
	qreal pageWidth = 0.0;
	qreal nmColWidth = 0.0, nmColsWidth = 0.0, nmColsWidthAll = 0.0;

	if (toWidthOfSheet){
		for (nmColCntr=1; (nmColsWidthAll < rigthLengMax || nmColCntr<=m_colCount); nmColCntr++)
		{
			nmColWidth = (qreal)getScaleSize(uorRhtColumnHeader, nmColCntr);
			nmColsWidthAll += nmColWidth;
		}
		qreal prWidht = qreal(pageRectPrintCpy.width());
		scaleFactor = prWidht / nmColsWidthAll;
		scaleFactor = qMax(0.1, scaleFactor);
		m_pagesSetings->setScale(scaleFactor);
		segment->m_segmWidth = nmColsWidthAll;
		segment->m_colEnd = nmColCntr;
		return;
	}
	pageWidth = pageRectPrintCpy.width();
	if (scaleFactor != 1.0){
		pageRectPrintCpy = m_pageRectPrint;
		m_pagesSetings->transformPageRect(pageRectPrintCpy);
		pageRectPrintCpy.adjust(2,2,-2,-2); // нужно правильно печатать толстые рамки, иначе края режутся.
		pageWidth = pageRectPrintCpy.width();
	}

	if (debug_this) {
		qDebug() << " pageWidth " << pageWidth << " rigthLengMax " << rigthLengMax;
	}

	for (nmColCntr=1; (nmColsWidthAll < rigthLengMax || nmColCntr<=m_colCount); nmColCntr++)
	{
		// не учитывается если колонка больше ширины страницы :(
		nmColWidth = getScaleSize(uorRhtColumnHeader, nmColCntr);
		// Если мы увеличили размер страницы, согласно масштаба, то нафига уменьшать размеры
		nmColWidth = nmColWidth; // * scaleFactorO;

		nmColsWidthAll += nmColWidth ;
		if (debug_this) {			qDebug() << " col  # " << nmColCntr << " ColWidth " << nmColWidth << "nmColsWidthAll"<<nmColsWidthAll;		}

		if ((nmColWidth + nmColsWidth)<=pageWidth){
			segment->m_colEnd = nmColCntr;
			nmColsWidth += nmColWidth;
			segment->m_segmWidth = nmColsWidth;
		} else {
			nmColsWidth = nmColWidth;
			if (nmColsWidth>=pageWidth){
				offset = uorNumberNull;
				do {
					if (segment->m_segmWidth > uorNumberNull){
						segment = new uorPageColSegment(++m_pagesColumnTotal,nmColCntr,nmColCntr);
						m_segmentList.append(segment);
					}
					segment->m_offsetStart = offset;
					segment->m_segmWidth = pageWidth;
					offset += pageWidth;
					nmColsWidth -= offset;
				}while(nmColsWidth>=pageWidth);

				segment = new uorPageColSegment(++m_pagesColumnTotal,nmColCntr,nmColCntr);
				m_segmentList.append(segment);
				segment->m_segmWidth = nmColsWidth;
				segment->m_offsetStart = offset;

			} else {
				segment = new uorPageColSegment(++m_pagesColumnTotal,nmColCntr,nmColCntr);
				m_segmentList.append(segment);
				segment->m_segmWidth = nmColsWidth;
			}
		}
	}

	if (debug_this) {
		uorPageColSegmentListIter it = m_segmentList.begin();
		while (it != m_segmentList.end()) {
			segment = *it;
			if (segment) {
				qDebug() << " segment # " << segment->m_segmentNom
				<< " start " << segment->m_colStart
				<< " end  " << segment->m_colEnd
				<< " ofss start " << segment->m_offsetStart
				<< " sgm heght " << segment->m_segmWidth;
			}
			it++;
		}
		qDebug() << " height ALL: " << nmColsWidthAll;
	}
}
/**
	Устанавливаем матрицу ячеек.
*/
void 	uoReportDoc::setCellMatrix(uoCellMatrix* matrix)
{
	if (m_matrixCell){
		delete m_matrixCell;
		m_matrixCell = 0;
	}
	m_matrixCell = matrix;
}


/**
	Сформируем список пространств-страниц для печати.
*/
uorAresList* uoReportDoc::getPageList()
{
	while (!m_pageList->isEmpty())     delete m_pageList->takeFirst();

	QPrinter* pPrinter = uoReportManager::instance()->printer();
	if(!pPrinter){
		qWarning() << QString::fromUtf8("Принтер не создан!");
		return NULL;
	}

	if (!updatePageRectPrint()){
		// страницу расчитали
		return NULL;
	}
	// расчитаем вертикальное сегментирование листов.
	updatePageSegmentList();
	int segSize = m_segmentList.count();
	if (segSize <= 0)
		return NULL;


	bool debug_this = true;
	qreal scaleFactor = m_pagesSetings->scale();

	QRect pageRectPrintCpy = m_pageRectPrint;
	if (scaleFactor != 1.0){
		m_pagesSetings->transformPageRect(pageRectPrintCpy);
	}
	pageRectPrintCpy.adjust(2,2,-2,-2); // нужно правильно печатать толстые рамки, иначе края режутся.


	int pageCnt = 0;

	uorPageColSegment* segment = NULL;

	uorReportPrintArea* areaPrint = NULL;
	uorReportPrintArea* areaBaseLast = NULL;

	int pagesColumnCurent = 1;
	uorNumber pageHeightCntr = uorNumberNull;
	uorNumber rowsHeight = uorNumberNull;
	uorNumber offset = uorNumberNull;


	QString cellText;
	bool isHide = false;

	while (pagesColumnCurent <= m_pagesColumnTotal) {
		areaPrint = new uorReportPrintArea;
		segment = m_segmentList.at(pagesColumnCurent-1);
		m_pageList->append(areaPrint);
		areaPrint->setFirstVisible_ColLeft(segment->m_colStart);
		areaPrint->setLastVisibleCol(segment->m_colEnd);
		areaPrint->setFirstVisible_RowTop(1);
		areaPrint->setLastVisibleRow(1);
		areaPrint->m_pageColumn 			= segment->m_segmentNom;
		areaPrint->setShift_ColLeft(segment->m_offsetStart);
		areaPrint->m_pageNumber 			= m_pageList->count();
		areaPrint->m_segment 				= segment;
		areaPrint->m_area.setTopLeft(pageRectPrintCpy.topLeft());
		areaPrint->m_area.setWidth(qMin(pageRectPrintCpy.width(),int(segment->m_segmWidth)));
		areaPrint->m_area.setHeight(pageRectPrintCpy.height());
		pageHeightCntr = uorNumberNull;

		for (int nRow = 1; nRow<= m_rowCount; nRow++)
		{
			while((isHide = getScaleHide(uorRhtRowsHeader, nRow))){
				++nRow;
			}

			rowsHeight = getScaleSize(uorRhtRowsHeader, nRow);

			if ((pageHeightCntr + rowsHeight) <= pageRectPrintCpy.height()){
				// высота строки нормальная для страницы..
				areaPrint->setLastVisibleRow(nRow);
				pageHeightCntr += rowsHeight;
				areaPrint->m_area.setHeight(pageHeightCntr);
			} else {

				if (rowsHeight>pageRectPrintCpy.height()){
					// Упс, высота строки превышает высоту страницы О_о. Бум резать... папазжа...
					offset = uorNumberNull;
					do {
						offset += pageRectPrintCpy.height();
						areaPrint = new uorReportPrintArea;
						m_pageList->append(areaPrint);
						areaPrint->m_pageNumber 			= m_pageList->count();
						areaPrint->setFirstVisible_RowTop(nRow);
						areaPrint->setLastVisibleRow(nRow);
						areaPrint->setFirstVisible_ColLeft(segment->m_colStart);
						areaPrint->setLastVisibleCol(segment->m_colEnd);
						areaPrint->m_pageColumn 			= segment->m_segmentNom;
						areaPrint->setShift_ColLeft(segment->m_offsetStart);
						areaPrint->setShift_RowTop(offset);
						areaPrint->m_area.setTopLeft(pageRectPrintCpy.topLeft());
						areaPrint->m_area.setWidth(qMin(pageRectPrintCpy.width(),int(segment->m_segmWidth)));
						areaPrint->m_area.setHeight(pageHeightCntr);
						areaPrint->m_segment = segment;
						rowsHeight 			-= offset;

					}while(rowsHeight>pageRectPrintCpy.height());
				} else {
					++pageCnt;
					pageHeightCntr = rowsHeight;
					areaBaseLast = areaPrint;
					areaPrint = new uorReportPrintArea;
					m_pageList->append(areaPrint);
					areaPrint->m_pageNumber 			= m_pageList->count();
					areaPrint->setFirstVisible_RowTop(nRow);
					areaPrint->setLastVisibleRow(nRow);
					areaPrint->setFirstVisible_ColLeft(segment->m_colStart);
					areaPrint->setLastVisibleCol(segment->m_colEnd);
					areaPrint->m_pageColumn 			= segment->m_segmentNom;
					areaPrint->setShift_ColLeft(segment->m_offsetStart);
					areaPrint->m_area.setTopLeft(pageRectPrintCpy.topLeft());
					areaPrint->m_area.setWidth(qMin(pageRectPrintCpy.width(),int(segment->m_segmWidth)));

					areaPrint->m_area.setHeight(pageHeightCntr);
					areaPrint->m_segment 				= segment;
				}
			}
		}
		++pagesColumnCurent;
	}
	debug_this = false;
	if (debug_this) {
		qDebug() << QString::fromUtf8("Расчет страниц: ");

		pageCnt = 0;
		uorAresListCnstIter it = m_pageList->constBegin();

		qDebug() << "\n";
		qDebug() << "\t Page\t rowsFrom\t rowsTo\t rows_shift\t colsFr\t colsTo\t cols_shift\t m_pageColumn\t" << pageCnt;
		while(it != m_pageList->constEnd()){
			++pageCnt;
			areaPrint = *it;
			if (areaPrint){
				if (true) {
					qDebug() << "Page №: " << areaPrint->m_pageNumber
					<< "rows" << areaPrint->firstVisible_RowTop() << " - " << areaPrint->lastVisibleRow() << " - " <<areaPrint->shift_RowTop()
					<< "cols" << areaPrint->firstVisible_ColLeft() << " - " << areaPrint->lastVisibleCol()<< " - " <<areaPrint->shift_ColLeft()
					<< "m_area" << areaPrint->m_area
					<< "m_pageColumn" << areaPrint->m_pageColumn;
				} else {
					qDebug() << "\t" << areaPrint->m_pageNumber << "\t"
					<< areaPrint->firstVisible_RowTop() << "\t"
					<< areaPrint->lastVisibleRow() << "\t"
					<< areaPrint->shift_RowTop() << "\t"
					<< areaPrint->firstVisible_ColLeft() << "\t"
					<< areaPrint->lastVisibleCol() << "\t"
					<< areaPrint->shift_ColLeft() << "\t"
					<< areaPrint->m_pageColumn; // << "\t"
				}
			}
			it++;
		}
	}

	return m_pageList;
}

uorTextDecor* uoReportDoc::getDefaultTextProp()
{
	return m_TextDecorDoc;
}

bool uoReportDoc::setupPrinter(QPrinter &printer, QWidget* qwidg)
{
	QWidget* pWidg = qwidg;
	if (!pWidg)
		pWidg = qApp->activeWindow();

	printer.setPrintRange(QPrinter::AllPages);
	printer.setPageSize(m_pagesSetings->m_paperSize);
	printer.setOrientation(m_pagesSetings->m_orientation);

    QPrintDialog dialog(&printer, pWidg);
    dialog.setEnabledOptions(QAbstractPrintDialog::PrintPageRange);
    dialog.setMinMax(1,m_pageList->size());
    if (dialog.exec() == QDialog::Accepted)
        return true;
    else
        return false;
}

/// Вывод на печать....
bool uoReportDoc::printDoc(const bool updPrintSourse, QWidget* qwidg)
{
	bool retVal = false;

	QPrinter* pPrinter = uoReportManager::instance()->printer();
	if(!pPrinter){
		qWarning() << QString::fromUtf8("Принтер не создан!");
		return retVal;
	}

	uorAresList* list = m_pageList;
	if(updPrintSourse){
		list = getPageList();
	}
    if (!list || !list->count())    /// Nagumanov 14.05.2010 Добавлена проверка на размер листа
        return retVal;

	if (!setupPrinter(*pPrinter, qwidg))
		return true;   /// Nagumanov 14.05.2010 Возвращать true, даже если отказались от печати

	int pageNomFrom = 0, pageNomTo = 0;
	QPrinter::PrintRange prnRange = pPrinter->printRange();
	if (prnRange == QPrinter::PageRange) {
		pageNomFrom = pPrinter->fromPage();
		pageNomTo = pPrinter->toPage();
	}
	qreal scaleFactor = m_pagesSetings->scale();
	qDebug() << QString("bool uoReportDoc::printDoc scaleFactor = %1").arg(scaleFactor);

    uoPainter painter;
    painter.begin(pPrinter);
    painter.scale(scaleFactor,scaleFactor);
    bool firstPage = true;
	uoReportDrawHelper* drawHelper = new uoReportDrawHelper(this);
	drawHelper->initDrawInstruments();
	drawHelper->setDirectDraw(false); // Если используется пиксмап кешер, то текст почему-то печатается на черном фоне белым..
	bool usePC = usePictCasher();
	setUsePictCasher(false);

	uorReportAreaBase aBase;

	uorReportPrintArea* area = NULL;
	int pageGurNo = 0;
	uorAresListCnstIter iter = list->constBegin();
	while (iter != list->constEnd()){
		pageGurNo += 1;
		if (prnRange == QPrinter::PageRange) {
			if (!(pageNomFrom<=pageGurNo && pageNomTo>=pageGurNo)){
				iter++;
				continue;
			}
		}
		if (!firstPage)
			pPrinter->newPage();

		area = *iter;
		aBase = (*area);
		aBase.m_areaType = 2;
		drawHelper->drawDataArea(painter, aBase);

		iter++;
		firstPage = false;

	}
	setUsePictCasher(usePC);
	delete drawHelper;
	return true;
}

bool uoReportDoc::getScaleHide(const uoRptHeaderType& hType, const int& nom){
	if (hType == uorRhtRowsHeader)
		return m_headerRow->getHide(nom);
	else
		return m_headerCol->getHide(nom);
}
bool uoReportDoc::getRowHide(const int& rowNom){
	return m_headerRow->getHide(rowNom);
}
bool uoReportDoc::getColHide(const int& colNom){
	return m_headerCol->getHide(colNom);
}

bool uoReportDoc::joinCells(QRect& joinRect, bool join /* = true*/){
	bool retVal = false;
	if (joinRect.width()>1 || joinRect.height()>0)
		return joinCells(joinRect.left(), joinRect.top(), joinRect.right(), joinRect.bottom(), join);
	return retVal;
}


bool uoReportDoc::joinCells(int colSt, int rowSt, int colEnd, int rowEnd, bool join /* = true*/)
{
	Q_ASSERT(colSt);
	Q_ASSERT(colEnd);
	Q_ASSERT(colSt<=colEnd);
	Q_ASSERT(rowSt);
	Q_ASSERT(rowEnd);
	Q_ASSERT(rowSt<=rowEnd);

	QList<uoCell*> listSell;
	uoCell* curCell = 0;

//	bool mayBeJoin = true;
//	bool needUnJoin = false; // Если все выделенные ячейки объединены, тогда от нас хотят UnJoin

	int joinCount = 0;
	int countCells = (colEnd - colSt + 1) * (rowEnd - rowSt + 1);
	QRect sellRect = QRect(colSt, rowSt, colEnd - colSt+1, rowEnd - rowSt+1);

	bool brokenCell = false; // Ошибка

	/*	true - объединить, false - разьединить.
		фишка в том, что мы не можем объединить уже объединенные и
		разьединить не объединенные. По этому будем решать по месту,
		какая операция предстоит.
	*/

	bool curentOper = join;
	uoCellJoin* cellJoinItem = 0;

	bool allJoinInsideSel = true; // все объединения внутри выделения.
	/* Используется когда пошла команда на объединение, но в выделении салат: и свободные и объединенные
	в таких случаях переключаемся на удаление объединений*/

	for (int cellOper = 0; cellOper<2; cellOper++){
		/*	первая операция проверка возможности объединения
			вторая само объединение	*/

		///\todo - продолжать тут. кончил 03.12.2009 01:39
		for (int rowCntr = rowSt; rowCntr <= rowEnd; rowCntr++){
			for (int colCntr = colSt; colCntr <= colEnd; colCntr++){
				curCell = getCell(rowCntr, colCntr, true,true);

				if (cellOper == 0) {
					// цикл проверки.
					if (!curCell) {
						brokenCell = true;
						break;
					}

					if (curCell->isUnionHas()){
						joinCount += 1;
						QRect rJoin = curCell->getCellJoinRect();
						if (!sellRect.contains(rJoin))
							allJoinInsideSel = false;
					}
				} else if (cellOper == 1) {
					// Цикл объединения/разьединения...
					if (curentOper) {
						// Объединяем...
						if (curCell->isUnionHas()){
							++m_changes;
							cellJoinItem = curCell->deleteCellJoin();
						} else {
							cellJoinItem = getCellJoinStruct();
							++m_changes;
						}
						if (cellJoinItem){
							if (rowCntr == rowSt && colCntr == colSt) {
								// Первая ячейка объединения.
								cellJoinItem->m_JoinType = uoCJT_Normal;
								cellJoinItem->m_col = colEnd - colSt + 1;
								cellJoinItem->m_row = rowEnd - rowSt + 1;
								cellJoinItem->m_cellRect = QRect(colSt,rowSt, cellJoinItem->m_col, cellJoinItem->m_row);
							} else {
								clearCell(curCell);
								cellJoinItem->m_JoinType = uoCJT_BackPoint;
								cellJoinItem->m_col = colSt;
								cellJoinItem->m_row = rowSt;
								cellJoinItem->m_cellRect = QRect(colSt,rowSt, cellJoinItem->m_col, cellJoinItem->m_row);
							}

							curCell->setCellJoin(cellJoinItem, this);
						} else {
							brokenCell = true;
						}
					} else {
						// разьединяем
						if (curCell->isUnionHas()){
							saveCellJoin(curCell->deleteCellJoin());
						}
					}
				}
			}
			if (brokenCell)
				break;
		}
		if (cellOper == 0){
			if ((countCells == joinCount) && curentOper){
				// у нас комманда на объединение, а все ячейки объединены.
				curentOper = false;
			} else if ((countCells != joinCount && joinCount>0) && curentOper){
				// Если операция - объединить, но мы находим объединенные ячейки в выделении, то прерываем операцию.
				if (allJoinInsideSel && !brokenCell){
					curentOper = false;
				} else {
					brokenCell = true;
					return false;
				}
			}
		}
	}//	for (int cellOper = 0; cellOper<2; cellOper++){

	return !brokenCell;
}

/// Получить первую ячейку объединения, если оно есть, или вернуть текущую, переданную.
uoCell* uoReportDoc::getFirstUnionCell(uoCell* curCell, int rowNo)
{
	uoCell* retCell = curCell;
	if (curCell){
		if (curCell->joinType() == uoCJT_BackPoint){
			QPoint curCellPnt = curCell->getFirstUnionCellPoint(rowNo);
			retCell = getCell(curCellPnt.y(), curCellPnt.x(), true, true);
		}
	}
	return retCell;
}

/// Очистка ячейки
void uoReportDoc::clearCell(uoCell* cell) {
	if (cell->m_textProp){
		m_pointBlock->savePoint(cell->m_textBoundary);
		cell->m_textBoundary = NULL;
		cell->m_maxRowLen = uorNumberNull;
	}
	cell->setText("", this);
	delete cell->m_textProp;
	cell->m_textProp = 0;
	delete cell->m_borderProp;
	cell->m_borderProp = 0;
}


/// присоединим вьюв..
void uoReportDoc::attachView(uoReportCtrl* rCtrl, bool autoConnect){
	bool found = false;
	if (!m_atachedView.isEmpty()) {
		 found = m_atachedView.contains(rCtrl);
	}
	if (!found) {
		++m_refCounter;
		m_atachedView.append(rCtrl);
		/// надо его законнектить к сигналам. Тока сигналы до ума довести.....
		if (autoConnect) {
	//		connect(
		}

	}
}

/// Отсоединяемся от вьюва.
void uoReportDoc::detachedView(uoReportCtrl* rCtrl)
{
	if (!m_atachedView.isEmpty()) {
		if (m_atachedView.contains(rCtrl)) {
			int pos = 0; m_atachedView.indexOf(rCtrl);
			while((pos = m_atachedView.indexOf(rCtrl))>0){
				m_atachedView.removeAt(pos);
			}
			--m_refCounter;
		}
	}
}

/// Объект используется/не используется. Если используется, удалять нельзя...
bool uoReportDoc::isObjectAttached()
{
	if (m_refCounter > 0 || !m_atachedView.isEmpty() || !m_atachedObj.isEmpty())
		return true;
	return false;
}

/// Получить шрифт по ID
QFont*  uoReportDoc::getFontByID(const int idFont)
{
	if (idFont<0)
		return NULL;

	QFont* font = m_fontColl->getFont(idFont);
	if (!font) {
	}
	return font;

}

int  uoReportDoc::addFont(QString family)
{
	int retVal = -1;
	if (!family.isEmpty())
	{
		retVal =  m_fontColl->findFont(family);
		if (retVal == -1){
			retVal = m_fontColl->addFont(family);
		}
//		retVal = (fontId == -1) ? false : true;
	}
	return retVal;
}


/// Вернуть цвет из коллекции
QColor*  uoReportDoc::getColorByID(const int idColor)
{
    Q_UNUSED(idColor);
	QColor* col = NULL;
	return col;
}


/// создаем новую структуру uorTextDecor для хранения атрибутов текста.
/// теперь есть возможность вставить акселератор для считывания больших документов.
uorTextDecor* uoReportDoc::getNewTextProp()
{
	///\todo сбацать акселераторы под считывание документов
	uorTextDecor* prop = new uorTextDecor;
	uorTextDecor* propDef = getDefaultTextProp();
	if (prop && propDef){
		prop->copyFrom(propDef);
	}

	return prop;
}

/// теперь есть возможность вставить акселератор для считывания больших документов.
uorBorderPropBase* uoReportDoc::getNewBordProp()
{
	///\todo сбацать акселераторы под считывание документов
	uorBorderPropBase* prop = new uorBorderPropBase;
	return prop;
}

uoCellPictCasher*
uoReportDoc::pictCasher()
{
	return m_pictCasher;
}

uoReportDoc *uoReportDoc::sourceDoc(){
	return m_sourceDoc;
}

bool uoReportDoc::setSourceDocFile(QString fileName){
	uoReportDoc* sourceDoc = new uoReportDoc();
	uoRptStoreFormat storeFormat = uoReportLoader::getFormatByName(fileName);
	if (uoRsf_Unknown == storeFormat){
		qDebug() << "Attention! Not correct file name" << fileName;
		return false;
	}
	sourceDoc->setStoreOptions(fileName, storeFormat);
	bool retVal = sourceDoc->load();
	if (!retVal)
		return retVal;
	sourceDoc->enableFormating(true);
	sourceDoc->doFormatDoc();
	setSourceDoc(sourceDoc);
	return true;
}

bool uoReportDoc::setSourceDocXML(QString xml){
	bool retVal = false;
	uoReportDoc* sourceDoc = new uoReportDoc();
	uoReportLoaderXML* loader = new uoReportLoaderXML;
	QByteArray ba = xml.toUtf8();
	if (loader->initBA(uorSO_LoadFromBArray, ba)) {
		if (loader->load(sourceDoc))
			retVal = loader->finalize();
	}
	delete loader;
	setSourceDoc(sourceDoc);
	return retVal;
}

bool uoReportDoc::setSourceDoc(uoReportDoc *doc){
	bool retVal = false;
	if (m_sourceDoc)
		delete m_sourceDoc;
	m_sourceDoc = doc;
	m_fontColl->copyFrom(doc->getFontColl());
	return retVal;
}

bool uoReportDoc::setVariable(QString var, QString value){
	bool retVal = false;
	QString varL = var.toLower();
	if (!varL.isEmpty()){
		retVal = m_varaiableMap.contains(varL);
		m_varaiableMap.insert(varL,value);
	}
	return retVal;
}

QString uoReportDoc::variable(QString var){
	QString value = "";
	QString varL = var.toLower();
	if (!varL.isEmpty()){
		value = m_varaiableMap.value(varL);
	}
	return value;
}

bool uoReportDoc::useSectionAsHeader(const QString &name)
{
	bool retVal = false;
	m_headerSection = name;
	if (!m_headerSection.isEmpty()){
		// mogno proverit nalichie seccii
	}
	return retVal;
}

bool uoReportDoc::useSectionAsFooter(const QString &name)
{
	bool retVal = false;
	m_footerSection = name;
	if (!m_footerSection.isEmpty()){
		// mogno proverit nalichie seccii
	}
	return retVal;
}

bool uoReportDoc::getSectionRect(const QString &name, QRect &rect)
{
	bool retVal = false;
	uorZeroRectI(rect);
	if (name.isEmpty())
		return retVal;
	QString secVert = "";
	QString secGor = "";
	int posSep = name.indexOf("|");
	if (posSep != -1){
		secGor = name.mid(0,posSep);
		secVert = name.mid(posSep+1);
	}
	uoLineSpan *spanGor = m_spanTreeSctRow->getSpanByName(secGor);
	if (!spanGor)
		return retVal;
	rect.setY(spanGor->getStart());
	rect.setBottom(spanGor->getEnd());
	rect.setX(1);
	rect.setRight(m_rowCount);
	uoLineSpan *spanVer = 0;
	if (!secVert.isEmpty()) {
		spanVer = m_spanTreeSctCol->getSpanByName(secVert);
		if (spanVer){
			rect.setX(spanVer->getStart());
			rect.setRight(spanVer->getEnd());
			retVal = true;
		}
	}
	return retVal;
}
/*
	С "текстом" все просто, его надо просто вывести
	"Выражение", достаточно просто, например "PrnPrice", просто
	ищем в списке выражений в документе и заменяем на значения
	Шаблон, это просто текст, который надо распарсить. Образец:
	"Общая сумма: [ВсегоСумма] руб.".
	Извлекаются части:
	- "Общая сумма: "
	- "[ВсегоСумма]"
	- " руб."
	"[ВсегоСумма]" - заменяется по принцыпу как и с "Выражением."
	Потом склеивается.
*/


bool uoReportDoc::bindValuesToText(QString &name, uoCellTextType tt) const
{
	bool retVal = true;
	QString txt = name.trimmed();
	QString nameOld = name;
	txt = txt.toLower();
	if (tt == uoCTT_Expr) {
		if (m_varaiableMap.contains(txt)){
			// просмто заменить или с учетом пробелов? Ну пока так.
			name = m_varaiableMap.value(txt);
		}
	} else if (tt == uoCTT_Templ) {
		QString tarTxt = name;
		while(tarTxt.indexOf("[")){
			int posF = tarTxt.indexOf("[");
			int posL = tarTxt.indexOf("]");
			if (posF != -1 && posL != -1 && posL>posF){
				QString frstTxt = "";
				QString varTxt = "";
				QString nextTxt = "";
				if (posF > 0)
					frstTxt = tarTxt.mid(0,posF-1);
				if(posF+1 < posL)
					varTxt = tarTxt.mid(posF+1,posL-1);
				if (posL< (tarTxt.length()-1))
					nextTxt = tarTxt.mid(posL+1);
				tarTxt = frstTxt;
				varTxt = varTxt.toLower();
				if (m_varaiableMap.contains(varTxt)){
					// просмто заменить или с учетом пробелов? Ну пока так.
					varTxt = m_varaiableMap.value(varTxt);
				}
				tarTxt.append(varTxt);
				tarTxt.append(nextTxt);
			} else {
				name = nameOld;
				return false;
			}
		}

	}
	return retVal;
}

/// послать сигнал на перерисовку
void uoReportDoc::updateViews(){
	emit refresh();
}

bool uoReportDoc::outputSection(const QString &name){
	QRect sectRc;
	if(!m_sourceDoc)
		return false;
	bool retVal = m_sourceDoc->getSectionRect(name, sectRc);

	if (!retVal)
		return retVal;
	int curRowNo = m_rowCount+1;
	int curColNo = 1;
	QString cellText;
	for (int rowScrNo = sectRc.y(); rowScrNo <= sectRc.bottom(); rowScrNo++){
		uoRow* rowSrc = m_sourceDoc->getRow(rowScrNo);
		uoRow* rowTar = m_rows->getRow(curRowNo, true);
		if (!rowSrc){
			++curRowNo;
			continue; // пустая строка, че поделаешь..
		}
		curColNo = 1;
		rowTar->copyFromTempl(rowSrc,this,m_sourceDoc, sectRc.x(), sectRc.right());
		for (int colSrcNo = sectRc.x(); colSrcNo <= sectRc.right(); colSrcNo++){
			uorNumber sSize = m_sourceDoc->getScaleSize(uorRhtColumnHeader,colSrcNo);
			setScaleSize(uorRhtColumnHeader,curColNo,sSize);
			bool sHide = m_sourceDoc->getScaleHide(uorRhtColumnHeader,colSrcNo);
			setScalesHide(uorRhtColumnHeader,curColNo,1,sHide);
			bool nCreate = (colSrcNo == sectRc.right()) ? true : false;
			uoCell* cellTar = getCell(curRowNo, curColNo, nCreate); // Надо обеспечить цельность области
			if (cellTar && cellTar->isIndependentCell() && !cellTar->textIsEmpty()){
				uoCellTextType tt = cellTar->getTextType();
				cellText = cellTar->text();
				if (bindValuesToText(cellText, tt)){
					cellTar->setTextType(uoCTT_Text, this);
					cellTar->setText(cellText, this);
				}
			}
			curColNo += 1;
		}
		++curRowNo;
		doFormatRow(curRowNo);
	}
	return retVal;
}

bool uoReportDoc::joinSection(const QString &name){
	bool retVal = false;
	QRect sectRc;
	if(!m_sourceDoc)
		return false;
	retVal = m_sourceDoc->getSectionRect(name, sectRc);

	if (!retVal)
		return retVal;
	/**
		Надо найти рект, наиболее подходящий для джоина
		Нати максимальную длину строки с конца документа (в пределах диапазона заполненных строк),
		это будет координата 'X' с 'X'+1 можно джоинить.
		Найти строку с которой надо джоинить = мин(КолСтрок - высота ректа,1)
	*/
	int heitRc = sectRc.height();
	heitRc -= 1;
	int rowJoinStartNo = qMax(1, m_rowCount - heitRc);
	int colJoinStartNo = 0, rowForCS = m_rowCount, rosSecCount = sectRc.height();
	uoRow* row = 0;
	while(rowForCS >= 1 && rosSecCount>0){
		row = getRow(rowForCS);
		if (row) {
			colJoinStartNo = qMax(colJoinStartNo, row->getMaxNo());
		}
		--rosSecCount;
		--rowForCS;
	}
	++colJoinStartNo;
//	qDebug() << "colJoinStartNo" << colJoinStartNo << "rowJoinStartNo" << rowJoinStartNo << "m_rowCount" << m_rowCount << "sectRc" << sectRc << "name" << name;
	int curRowTar = rowJoinStartNo;
	int curColTar = colJoinStartNo;

	QString cellText;
	for (int rowSrcNo = sectRc.y(); rowSrcNo <= sectRc.bottom(); rowSrcNo++){
		uoRow* rowSrc = m_sourceDoc->getRow(rowSrcNo);
        //uoRow* rowTar = m_rows->getRow(curRowTar, true);
		if (!rowSrc){
			++curRowTar;
			continue; // пустая строка, че поделаешь..
		}
		curColTar = colJoinStartNo;
		for (int colSrcNo = sectRc.x(); colSrcNo <= sectRc.right(); colSrcNo++){
			uorNumber sSize = m_sourceDoc->getScaleSize(uorRhtColumnHeader,colSrcNo);
			setScaleSize(uorRhtColumnHeader,curColTar,sSize);
			bool sHide = m_sourceDoc->getScaleHide(uorRhtColumnHeader,colSrcNo);
			setScalesHide(uorRhtColumnHeader,curColTar,1,sHide);

			uoCell* cellTar = getCell(curRowTar, curColTar,true, true);
			uoCell* cellSrc = m_sourceDoc->getCell(rowSrcNo,colSrcNo);
			if (cellSrc && cellTar)
				cellTar->copyFrom(cellSrc,this,m_sourceDoc, curRowTar);

			if (cellTar && cellTar->isIndependentCell() && !cellTar->textIsEmpty()){
				uoCellTextType tt = cellTar->getTextType();
				cellText = cellTar->text();
				if (bindValuesToText(cellText, tt)){
					cellTar->setTextType(uoCTT_Text, this);
					cellTar->setText(cellText, this);
				}
			}
		}
		++curRowTar;
		doFormatRow(curRowTar);
	}
	return retVal;
}



/// Создаем новую uoCellJoin
uoCellJoin* uoReportDoc::getCellJoinStruct()
{
	return new uoCellJoin;
}

/// Если текущая ячейка участник объединения, возвращает рект объединения.
QRect uoReportDoc::getCellJoinRect(QPoint curCell)
{
	QRect rctVal = QRect(curCell.x(),curCell.y(), 1,1);
	uoCell* cell = getCell(curCell.y(), curCell.x());
	uoCell* cell2 = 0;
	if (cell){
		if (cell->isUnionHas()){
			rctVal = cell->m_ceelJoin->m_cellRect;
			if (cell->joinType() == uoCJT_BackPoint){
				cell2 = getCell(cell->m_ceelJoin->m_row, cell->m_ceelJoin->m_col);
				if (cell2){
					rctVal = cell2->m_ceelJoin->m_cellRect;
				}
			}
		}
	}
	return rctVal;
}
/// Утилизируем uoCellJoin
void uoReportDoc::saveCellJoin(uoCellJoin* cellJItem)
{
	delete cellJItem;
	cellJItem = NULL;
}





static void toDebugTest(bool resComp, int *testOkCnt, int *testAll, const char* str)
{
	if (resComp) {
		// если тест прошел, зачем печатать?
		++(*testOkCnt);  //qDebug()<<"success: "<<str;
	} else {
		qDebug()<<"failed: "<<str;
	}
	++(*testAll);
}

/// Тестирование класса.
void uoReportDoc::test(){

    int nTestOk = 0, nTestAll = 0;
    bool printAll = true;
	bool printCurent = true;

	uorNumber sz = 17;

    qDebug()<<"Start test class \"uoReportDocBody\"";
    qDebug()<<"{";
	if (false || printAll) {
		setScaleSize(uorRhtRowsHeader, 2, 15);
		setScaleSize(uorRhtRowsHeader, 4, 18); if (printCurent) m_headerRow->printToDebug();
		setScaleSize(uorRhtRowsHeader, 3, 19); if (printCurent) m_headerRow->printToDebug();
		setScaleSize(uorRhtRowsHeader, 5, 25); if (printCurent) m_headerRow->printToDebug();
		setScaleSize(uorRhtRowsHeader, 2, sz); if (printCurent) m_headerRow->printToDebug();
		setScaleSize(uorRhtRowsHeader, 1, sz); if (printCurent) m_headerRow->printToDebug();
		setScaleSize(uorRhtRowsHeader, 7, 11); if (printCurent) m_headerRow->printToDebug();

		toDebugTest(getScaleSize(uorRhtRowsHeader, 1) == sz, &nTestOk, &nTestAll, "getScaleSize(uorRhtRowsHeader, 1) == sz");
		toDebugTest(getScaleSize(uorRhtRowsHeader, 2) == sz, &nTestOk, &nTestAll, "getScaleSize(uorRhtRowsHeader, 2) == sz");
		qDebug() << " size " << m_headerRow->getCountItem();
		qDebug() << " m_headerRow->deleteItem(2,2); ";
		m_headerRow->deleteItem(2,2);
		qDebug() << " size " << m_headerRow->getCountItem();
		m_headerRow->printToDebug();
	}


    qDebug()<<"Test all: "<<nTestAll<<" test OK: "<< nTestOk<<" test is: "<< (nTestOk==nTestAll);
    qDebug()<<"End test class \"uoReportDocBody\"";
    qDebug()<<"}";

}

} // namespace uoReport
