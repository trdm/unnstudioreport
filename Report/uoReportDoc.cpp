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

namespace uoReport {


uoReportDoc::uoReportDoc()
//	:uoReportDocBody()
	: m_spanTreeGrH(new uoSpanTree)
	, m_spanTreeGrV(new uoSpanTree)
	, m_spanTreeSctH(new uoSpanTree)
	, m_spanTreeSctV(new uoSpanTree)
	, m_headerV(new uoHeaderScale)
	, m_headerH(new uoHeaderScale)
	, m_rows(new uoRowsDoc)
	, m_TextDecorDoc(0)
{

	m_spanTreeSctH->setCanOnlyOne(true);
	m_spanTreeSctV->setCanOnlyOne(true);
	m_storeFormat = uoRsf_Unknown;
	m_pagesSetings = new uorPagePrintSetings;
	m_fontColl = new uoReportDocFontColl;
	m_pageList = new uorAresList;

	m_rows->setDoc(this);
	initTextDecorDoc();
	setDefaultFont(QApplication::font());



	m_headerV->setDefSize(UORPT_SCALE_SIZE_DEF_VERTICAL);
	m_headerH->setDefSize(UORPT_SCALE_SIZE_DEF_HORIZONTAL);

	m_rowCount 	= 0;
	m_colCount 	= 0;

	m_sizeV_visible = m_sizeV = 0.0;	///< Размер документа по вертикали
	m_sizeH_visible = m_sizeH = 0.0;	///< Размер документа по горизонтали
	m_freezEvent = 0;
	m_refCounter = 0;
	m_defaultFontId = 0;
	m_ident = 0;
	m_pointBlock = new uoTextTrPointCash;
	m_undoManager = new uoReportUndo;
	if (m_undoManager)
		m_undoManager->setDoc(this);
	m_cellDefault = new uoCell(-1);
	if (m_cellDefault){
		m_cellDefault->provideAllProps(this, true);
	}
	m_changes = 0;
	m_formatStoped = false;
}

uoReportDoc::~uoReportDoc()
{
	clear();
	delete m_spanTreeGrH;
	delete m_spanTreeGrV;
	delete m_spanTreeSctH;
	delete m_spanTreeSctV;
	delete m_headerV;
	delete m_headerH;
	delete m_fontColl;
	delete m_TextDecorDoc;
	delete m_cellDefault;
	delete m_pagesSetings;

}

/// Очистка секций.
void uoReportDoc::clear()
{
	m_spanTreeGrH->clear();
	m_spanTreeGrV->clear();
	m_spanTreeSctH->clear();
	m_spanTreeSctV->clear();

	m_rowCount 	= 0;
	m_colCount 	= 0;
	m_pagesSetings->clear();

	m_sizeV_visible = m_sizeV = 0.0;
	m_sizeH_visible = m_sizeH = 0.0;
	m_fontColl->clear();
	m_headerV->clear();
	m_headerH->clear();
	m_rows->clear();
	m_pointBlock->clear();
	m_undoManager->clear();
	setDefaultFont(QApplication::font());
	++m_changes;
	while (!m_segmentList.isEmpty())     delete m_segmentList.takeFirst();
	while (!m_pageList->isEmpty())     delete m_pageList->takeFirst();

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
	if(ht == uorRhtRowsHeader)	return m_spanTreeGrV->possiblyAddSpan(start, end);
	else					return m_spanTreeGrH->possiblyAddSpan(start, end);
}

/// добавление группировки в отчет.
bool uoReportDoc::addGroup(int start, int end, uoRptHeaderType ht, bool folded)
{
	bool retVal = false;
	uoSpanTree* treeGrp = NULL;

	if(ht == uorRhtRowsHeader) {
		treeGrp = m_spanTreeGrV;
		beforeAddRowOrCol(end - m_rowCount, ht);
	} else if (ht == uorRhtColumnHeader) {
		treeGrp = m_spanTreeGrH;
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
		treeGrp = m_spanTreeGrV;
	else
		treeGrp = m_spanTreeGrH;
	return treeGrp->getLevel();
}

/// Максимальный уровень вложения секций.
int uoReportDoc::getSectionLevel(uoRptHeaderType ht){
	uoSpanTree* treeGrp = NULL;
	if(ht == uorRhtRowsHeader)	treeGrp = m_spanTreeSctV;
	else					treeGrp = m_spanTreeSctH;
	return treeGrp->getLevel();
}

/// Свертка/развертка группы.
void uoReportDoc::doGroupFold(int idGrop, uoRptHeaderType rht, bool fold){
	uoSpanTree* treeGrp = NULL;
	if(rht == uorRhtRowsHeader)	treeGrp = m_spanTreeGrV;
	else					treeGrp = m_spanTreeGrH;
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
		return m_spanTreeGrH->getSpanList(start, end, true);
	else
		return m_spanTreeGrV->getSpanList(start, end, true);
}

/// Получить список спанов СЕКЦИЙ по диапазону строк/столбцов
const spanList* uoReportDoc::getSectionList(uoRptHeaderType rht, int start, int end)
{
	if (rht == uorRhtColumnHeader)
		return m_spanTreeSctH->getSpanList(start, end, false);
	else
		return m_spanTreeSctV->getSpanList(start, end, false);
}



/// проверка возможности сформировать/вставить СЕКЦИЮ в документ.
/// может потребоваться для того, что-бы задизаблить пункты динамического меню
bool uoReportDoc::possiblyAddSection(int start, int end, uoRptHeaderType ht)
{
	if(ht == uorRhtColumnHeader)
		return m_spanTreeSctH->possiblyAddSpan(start, end);
	else
		return m_spanTreeSctV->possiblyAddSpan(start, end);
}

/// вставка секции в отчет
bool uoReportDoc::addSection(int start, int end, uoRptHeaderType ht, QString name)
{
	if(ht == uorRhtColumnHeader)	return m_spanTreeSctH->addSpan(start, end, name);
	else					return m_spanTreeSctV->addSpan(start, end, name);
	++m_changes;
}

/// Отдать менеджер секций
uoSpanTree* uoReportDoc::getSectionManager(uoRptHeaderType ht)
{
	if(ht == uorRhtColumnHeader)		return m_spanTreeSctH;
	else if(ht == uorRhtRowsHeader)	return m_spanTreeSctV;
	return NULL;
}
/// Отдать менеджер групп
uoSpanTree* uoReportDoc::getGroupManager(uoRptHeaderType ht)
{
	if(ht == uorRhtColumnHeader)		return m_spanTreeGrH;
	else if(ht == uorRhtRowsHeader)	return m_spanTreeGrV;
	return NULL;
}


void uoReportDoc::doDeleteColumns(int itemStart, int count)
{
	m_spanTreeSctH->onLinesDelete(itemStart, count);
	m_spanTreeGrH->onLinesDelete(itemStart, count);
	m_headerH->deleteItem(itemStart, count);
	uoRow* row = m_rows->getFirst();
	while(row)
	{
		row->deleteItem(itemStart, count);
		row = m_rows->getNext();
	}
	doFormatDoc();
	emit onDataChange();
	++m_changes;
}


void uoReportDoc::doDeleteRows(int itemStart, int count)
{
	m_spanTreeSctV->onLinesDelete(itemStart, count);
	m_spanTreeGrV->onLinesDelete(itemStart, count);
	m_headerV->deleteItem(itemStart, count);
	m_rows->deleteItem(itemStart, count);
	doFormatDoc();
	emit onDataChange();
	++m_changes;
}


void uoReportDoc::doAddColumns(int itemStart, int count)
{
	m_spanTreeSctH->onLinesAdd(itemStart, count);
	m_spanTreeGrH->onLinesAdd(itemStart, count);
	m_headerH->addEmptyItems(itemStart, count);
	uoRow* row = m_rows->getFirst();
	while(row)
	{
		row->addEmptyItems(itemStart, count);
		row = m_rows->getNext();
	}
	doFormatDoc();
	emit onDataChange();
	++m_changes;
}
void uoReportDoc::doAddRows(int itemStart, int count)
{
	m_spanTreeSctV->onLinesAdd(itemStart, count);
	m_spanTreeGrV->onLinesAdd(itemStart, count);
	m_headerV->addEmptyItems(itemStart, count);
	m_rows->addEmptyItems(itemStart, count);
	doFormatDoc();
	emit onDataChange();
}


/** Сброс документа на диск после инициализации лоадера.
	использую для обхода необходимости писать каждый раз
	деинициализацию лоадера при неудаче в выгрузке...
*/
bool uoReportDoc::flush(uoReportLoader* loader){

	bool retVal = false;

	int cntItem 	= 0;
	int i 			= 0;
	int typeHeader 	= 0;

	uoLineSpan* spn 	= NULL;
	uoSpanTree* curTree = NULL;

	uoRptHeaderType curHeaderType = uorRhtColumnHeader;
	// Тут просто перебор документа с обращением к лоадеру..
	retVal = loader->saveDocStart(this);
	if (!retVal) return false;

	// ---------------выгрузка группировок --------------
	for (typeHeader = 1; typeHeader<=2; typeHeader++) {
		if(typeHeader == 1){
			curTree	= m_spanTreeGrH;
			curHeaderType = uorRhtColumnHeader;
		} else {
			curTree	= m_spanTreeGrV;
			curHeaderType = uorRhtRowsHeader;
		}


		cntItem = curTree->getSize();
		if (cntItem > 0) {
			retVal = loader->saveGroupsHeaderStart(cntItem, curHeaderType);
			if (!retVal) break;

			const spanList* spnList = curTree->getSpanList();
			for (i = 0; i<cntItem; i++){
				spn = spnList->at(i);
				retVal = loader->saveGroupsItem(spn);
				if (!retVal) break;
			}

			delete spnList;
			spnList = NULL;

			retVal = loader->saveGroupsHeaderEnd(curHeaderType);
			if (!retVal) break;
		}
	}
	if (!retVal) return false;

	// ---------------выгрузка секций --------------
	for (typeHeader = 1; typeHeader<=2; typeHeader++) {
		if(typeHeader == 1){
			curTree	= m_spanTreeSctH;
			curHeaderType = uorRhtColumnHeader;
		} else {
			curTree	= m_spanTreeSctV;
			curHeaderType = uorRhtRowsHeader;
		}


		cntItem = curTree->getSize();
		if (cntItem > 0) {
			retVal = loader->saveSectionHeaderStart(cntItem, curHeaderType);
			if (!retVal) break;

			const spanList* spnList = curTree->getSpanList();
			for (i = 0; i<cntItem; i++){
				spn = spnList->at(i);
				retVal = loader->saveSectionItem(spn);
				if (!retVal) break;
			}

			delete spnList;
			spnList = NULL;

			retVal = loader->saveSectionHeaderEnd(curHeaderType);
			if (!retVal) break;
		}
	}
	if (!retVal) return false;

	uoHeaderScale* headerScale = NULL;

	// ---------------выгрузка размеров строк/столбцов --------------
	// Тут же надо делать выгрузку соодержимого ячеек.
	for (typeHeader = 1; typeHeader<=2; typeHeader++) {
		if(typeHeader == 1){
			headerScale	= m_headerH;
			curHeaderType = uorRhtColumnHeader;
		} else {
			headerScale	= m_headerV;
			curHeaderType = uorRhtRowsHeader;
		}

		cntItem = headerScale->getCountItem();
		if (cntItem > 0) {
			retVal = loader->saveScaleHeaderStart(cntItem, curHeaderType);
			if (!retVal) break;

			headerScale->onStoreItems(loader);

			retVal = loader->saveScaleHeaderEnd(curHeaderType);
			if (!retVal) break;
		}
	}

	//--------- выгрузка содержания строк -------------------
	int rowCount = m_rows->getCountItem();
	if (rowCount>0) {
		loader->saveRowsStart(rowCount);
		m_rows->saveItems(loader);
		loader->saveRowsEnd();
	}
	if (m_fontColl->countFonts()>0){
		QFont* font = NULL;
		loader->saveFontStart(m_fontColl->countFonts());
		for (i = 0; i< m_fontColl->countFonts(); i++){
			font = m_fontColl->getFont(i);
			loader->saveFont(font, i);
		}
		loader->saveFontEnd();


	}

	if (!retVal) return false;
	retVal = loader->saveDocEnd(this);
	return retVal;
}

/// Сохранение отчета. Инициализируем лоадер, вызываем сохранение...
bool uoReportDoc::saveToFile(QString path, uoRptStoreFormat stFormat)
{
	bool retVal = false;
	if (stFormat == uoRsf_Unknown) {
		qWarning() << tr("no define store format");
		return false;
	}
	uoReportLoader* loader = uoReportLoader::getLoader(stFormat);
	if (!loader) {
		qWarning() << tr("Can not create loader");		return false;
	}

	loader->setFileName(path);
	if (!loader->init(false)){
		qWarning() << loader->getLastError();
	} else {
		flush(loader);
	}
	loader->finalize();
	delete loader;

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
bool uoReportDoc::save(){
	if (saveOptionsIsValid()) {
		return saveToFile(m_docFilePath, m_storeFormat);
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
	bool retVal = false;
	if (stFormat == uoRsf_Unknown) {
		qWarning() << tr("no define store format");
		return false;
	}
	uoReportLoader* loader = uoReportLoader::getLoader(stFormat);
	if (!loader) {
		qWarning() << tr("Can not create loader");		return false;
	}

	m_formatStoped = true;

	loader->setFileName(path);
	if (!loader->init(true)){
		qWarning() << loader->getLastError();
	} else {
		loader->load(this);
	}
	loader->finalize();
	delete loader;
	m_formatStoped = false;
	doFormatDoc();

	return retVal;

}



/// Возвращаем дефолтный размер строки или колонки
qreal  	uoReportDoc::getDefScaleSize(uoRptHeaderType rht)
{
	if (rht == uorRhtRowsHeader){
		return m_headerV->getDefSizeItem();
	} else if (rht == uorRhtColumnHeader) {
		return m_headerH->getDefSizeItem();
	}
	return 0.0;
}

/// Возвращаем высоту документа полную (по умолчанию) или видимую.
qreal  	uoReportDoc::getVSize(bool visible){
	if (visible)
		return m_sizeV_visible;
	else
		return m_sizeV;
}

/// Возвращаем длину документа полную (по умолчанию) или видимую.
qreal  	uoReportDoc::getHSize(bool visible){
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

/// перед добавлением строк или столбцов. Необходимо что-бы посчитать длину/ширину дока.
void uoReportDoc::beforeAddRowOrCol(int count, uoRptHeaderType rht, int noLn)
{
	if (count <= 0)
		return;
	int oldCnt = 0;

	uoHeaderScale* header = NULL;
	if (rht == uorRhtRowsHeader) {
		header = m_headerV;
		oldCnt = m_rowCount;
	} else {
		header = m_headerH;
		oldCnt = m_colCount;
	}
	qreal itemSize 	 = 0.0;
	qreal addSize 	 = 0.0;
	qreal addSizeVis = 0.0;
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
	if (count == 0)
		return;
	qreal oldSize = m_sizeV;
	qreal oldSizeVis = m_sizeV_visible;

	m_rowCount = m_rowCount + count;
	m_sizeV = oldSize + count * getDefScaleSize(uorRhtRowsHeader);
	m_sizeV_visible = oldSizeVis + count * getDefScaleSize(uorRhtRowsHeader);
	if (m_freezEvent == 0)
		emit onSizeChange(m_sizeV_visible, m_sizeH_visible);
}

/// Изменить количество столбцов в документе
void uoReportDoc::doColCountChange(int count, int pos )
{
	if (count == 0)
		return;
	qreal oldSize = m_sizeH;
	qreal oldSizeVis = m_sizeH_visible;

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
qreal uoReportDoc::getScaleSize(uoRptHeaderType hType, int nom, bool isDef)
{
	if (hType == uorRhtRowsHeader)
		return m_headerV->getSize(nom, isDef);
	else
		return m_headerH->getSize(nom, isDef);
}

/// Вернуть размер диапазона ячек
qreal uoReportDoc::getScalesSize(const uoRptHeaderType& hType, const int& nomStart, const int& nomEnd, const bool& ignoreHiden, const bool& isDef) const
{
	uoHeaderScale* pHeader = NULL;
	if (hType == uorRhtRowsHeader) {
		pHeader = m_headerV;
	} else {
		pHeader = m_headerH;
	}
	qreal retVal = 0.0;
	int nmStart = qMin(nomStart,nomEnd), nmEnd = qMax(nomStart,nomEnd), i = 0;
	for (i = nmStart; i<=nmEnd; i++){
		if (pHeader->getHide(i) && ignoreHiden)
			continue;
		retVal = retVal + pHeader->getSize(i, isDef);
	}
	return retVal;
}

/// Установить размер ячейки
void uoReportDoc::setScaleSize(uoRptHeaderType hType, int nom, qreal size, bool isDef){
	bool scVisible = true;
	qreal oldSizeItem = 0.0, oldSize = 0.0;
	if (hType == uorRhtRowsHeader) {
		oldSizeItem = m_headerV->getSize(nom, isDef);
		scVisible = m_headerV->getHide(nom);

		if (m_undoManager){		m_undoManager->doScaleResize(hType,nom,oldSizeItem);		}

		m_headerV->setSize(nom, size, isDef);
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
		oldSizeItem= m_headerH->getSize(nom, isDef);

		if (m_undoManager){		m_undoManager->doScaleResize(hType,nom,oldSizeItem);		}

		m_headerH->setSize(nom, size, isDef);
		++m_changes;
		m_sizeH = m_sizeH - oldSizeItem + size;
		scVisible = m_headerH->getHide(nom);
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
void uoReportDoc::setScalesSize(const uoRptHeaderType& hType, const QList<int>& list, const qreal& size, const bool& isDef)
{
	if (list.size() == 0 || size < 0.0)
		return;
	bool scVisible = true;

	uoHeaderScale* heider = m_headerV;
	if (hType == uorRhtColumnHeader)
		heider = m_headerH;
	int itemNo = 0;

	qreal oldSizeItem = 0.0, oldSize = 0.0;
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

}
/**
	Установка свойства, что строка иммет фиксированную высоту,
	т.е. при изменении текста её размер не меняется
	Примечание. бесполезно для столбцов, они не будут подгоняться
	"под размер" текста. Ну по крайней мере пока...
*/
void uoReportDoc::setScaleFixedProp(uoRptHeaderType hType, int nom, bool isFixed)
{
	if (hType == uorRhtRowsHeader) {
		m_headerV->setFixed(nom, isFixed);
		++m_changes;
	}
	/// для колонок собственно безсмысленно...
}

bool uoReportDoc::getScaleFixedProp(uoRptHeaderType hType, int nom)
{
	if (hType == uorRhtRowsHeader) {
		return m_headerV->getFixed(nom);
	}
	return false;
}


/// Прячем/Показываем диапазон ячеек...
void uoReportDoc::setScalesHide(uoRptHeaderType hType, int nmStart, int cnt,  bool hide){
	uoHeaderScale* header = NULL;
	if (hType == uorRhtRowsHeader) {
		header = m_headerV;
	} else {
		header = m_headerH;
	}
	qreal szAdd = 0.0;
	for (int i = 0; i<cnt; i++)	{
		szAdd = szAdd + header->getSize(nmStart + i);
		header->setHide(nmStart + i, hide);
		++m_changes;
	}
	qreal oldSize = 0.0;
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
	int retVal = -1;

	uoRow* row = m_rows->getRow(rowCur);
	if (row) {
		row->getNextItemNom(colCur);
	}

	return retVal;
}

/// пошинковать строку на слова не превышающие длинны collWidth
QStringList uoReportDoc::splitQStringToWord(const QFontMetricsF& fm, const QString& str, const qreal& collWidth)
{
	QStringList list;
	QString curStr, curWord;
	QChar qchar;
	qreal lenStr = 0.0, lenCurW = 0.0;

	QStringList listW = str.split(QRegExp("\\b"));
	for (int i = 0; i<listW.count(); i++)
	{
		curWord = listW.at(i);
		if (curWord.isEmpty())
			continue;

		lenCurW = fm.width(curWord);
		if ((lenStr + lenCurW)>collWidth){
			if (!curStr.isEmpty()) {
				list.append(curStr);
				curStr = "";
				lenStr = 0.0;
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
		lenStr = fm.width(curStr);
	}
	if (!curStr.isEmpty())
		list.append(curStr);
	return list;
}


/// Форматирование текста ячейки, одновременно расчет её высоты..
qreal uoReportDoc::doFormatCellText(uoCell* cell, QFont* font, QFontMetricsF& fm, const qreal& collWidth)
{
	qreal cellHeight = 0.0;
	qreal collWidthThis = collWidth - UORPT_STANDART_OFFSET_TEXT*2; // 2 офсета, справ и слева.


	// задачка не из простых...
	QString cellStr = cell->getText();
	if (cell->m_textProp){
		m_pointBlock->savePoint(cell->m_textBoundary);
		cell->m_textBoundary = NULL;
		cell->m_maxRowLen = 0.0;
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
	int listSz = 0;

//	qreal fullLength = fm.width(cellStr); // нужно проверить, влезает ли текст в сейку..
	QString nextStr, curStr;
	if (pos != -1 || tb == uoCTB_Transfer){
		// Значит надо рубить
		QStringList strList = cellStr.split(chrLS);
		listSz = strList.count();

		for (int i = 0; i<strList.count(); i++) {
			nextStr = strList.at(i);

			if ((i+1)<listSz){
				nextStr = nextStr + "\n";
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

	qreal maxRowLength = 0.0;
	for (int i = 0; i < strListReal.size(); ++i){
		nextStr = strListReal.at(i);
		maxRowLength = qMax(maxRowLength, fm.width(nextStr));
	}

	cell->setMaxRowLength(maxRowLength, this);
	cell->applyTrPoint(m_pointBlock, strListReal, this);
	cellHeight = fm.height() * strListReal.size();
	cell->m_height = cellHeight;
	return cellHeight;
}
/**
	Просчитаем для строки максимальную длину, которую хочет её текст.
*/
void uoReportDoc::doFormatRowLRMaxLng(uoRow* row)
{
	if (!row)
		return;
	// Посчитаем для строки лефты, райты.
	row->m_lengthMaxToLeft = 0.0;
	row->m_lengthMaxToRight = 0.0;
	row->m_lengthFromCell = 0.0;

	QMap<int, qreal> mapColSize;
	QMap<int, qreal> mapColToLeng;
	mapColSize[0] = 0;
	mapColToLeng[0] = 0;

	int cellNo = 0, cellNoLast = 0;
	qreal collSize = 0.0, collSizeAll = 0.0, collSizeAllTmp = 0.0, textCellMaxLeng = 0.0;

	uoHorAlignment textHorAl = uoHA_Unknown;

	uoCell* cell = row->getFirst();
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
				collSizeAllTmp = qMax(0.0, collSizeAllTmp);
				row->m_lengthMaxToRight = qMax(row->m_lengthMaxToRight, collSizeAllTmp);
			} else if (textHorAl == uoHA_Right){
				collSizeAllTmp = textCellMaxLeng - mapColToLeng[cellNo];
				collSizeAllTmp = qMax(0.0, collSizeAllTmp);
				row->m_lengthMaxToLeft = qMax(row->m_lengthMaxToLeft, collSizeAllTmp);
			}
		}
		cell = row->getNext();
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
	QList<int> cellsNumbers = row->getItemNumList();
	qreal rowMinSize = m_headerV->getDefSizeItem();
	qreal rowRealSize = rowMinSize;
	int nmRow = row->number();
	if (cellsNumbers.isEmpty())	{
		// если мы удалили все содержимое, строку надо форматнуть, если её размер отличается от дефолтного
		if(!m_headerV->getFixed(nmRow)){
			if (m_headerV->getSize(nmRow) != rowMinSize) {
				m_headerV->setSize(nmRow,rowMinSize);
			}
		}
		return;
	}
	uoCell* cell = NULL;
	QString cellText;
	QFont* font;
	int cellPrevNo = -1;
	int cellCurNo = -1;
	int cellNextNo = -1;

	int cellCounts = cellsNumbers.size();
	qreal collSize = 0.0; //getScaleSize(uorRhtRowsHeader

	qreal cellHeight = 0;
	uoCellTextBehavior textBehav = uoCTB_Auto;
	uoHorAlignment textHorAl = uoHA_Left;

	bool collBeFormated = false;

	if(nmForCol<=0)
		collBeFormated = true;

	for (int i = 0; i<cellCounts; i++){
		cellCurNo = cellsNumbers.at(i);
		cell = row->getItem(cellCurNo, false);

		collSize = getScaleSize(uorRhtColumnHeader,cellCurNo);

		if (nmForCol != -1 && nmForCol != cellCurNo) {
			// нужно еще проверить ячейки на объединение...
			rowRealSize = qMax(cell->m_height,rowRealSize);
			rowRealSize = qMax(rowRealSize, rowMinSize);
			continue;
		}
		cell->m_height = 0.0;

		collBeFormated = true;
		cellNextNo = cellCurNo;
		if (cellCounts>i+1){
			cellNextNo = cellsNumbers.at(i+1);
		}

		if (cellPrevNo == -1 && cellCurNo>1)
			cellPrevNo = 1;


		textBehav = cell->getTextBehavior();
		textHorAl = cell->getAlignmentHor();

		font = cell->getFont(this);
		cellText = cell->getText();

		if (font && !cellText.isEmpty()) {

			// тут уже можно хотя бы посчитать минимальную высоту строки. что и делаем...
			QFontMetricsF fm(*font);
			rowMinSize = qMax(rowMinSize, (qreal)fm.height());

			// а тут можно переформатировать текст
			if (cellNextNo != -1 && textBehav == uoCTB_Auto) {
				if (textHorAl == uoHA_Center){
					if ((cellPrevNo != -1) && (cellNextNo != -1)) {
						collSize = getScalesSize(uorRhtColumnHeader, qMin(cellPrevNo+1, cellCurNo), qMax(cellCurNo,cellNextNo-1), true);
					} else if (cellNextNo != -1){
						collSize = getScalesSize(uorRhtColumnHeader, cellCurNo, qMax(cellCurNo,cellNextNo-1), true);
					} else if (cellPrevNo != -1){
						collSize = getScalesSize(uorRhtColumnHeader, qMin(cellPrevNo+1, cellCurNo), cellCurNo, true);
					}
				} else if (textHorAl == uoHA_Left) {
					if (cellNextNo != -1) {
						collSize = getScalesSize(uorRhtColumnHeader, cellCurNo, qMax(cellCurNo,cellNextNo-1), true);
					} else {
						collSize = getScalesSize(uorRhtColumnHeader, cellCurNo, cellCurNo, true);
					}
				} else if (textHorAl == uoHA_Right) {
					if (cellPrevNo != -1) {
						collSize = getScalesSize(uorRhtColumnHeader, qMin(cellPrevNo+1, cellCurNo), cellCurNo, true);
					} else {
						collSize = getScalesSize(uorRhtColumnHeader, cellCurNo, cellCurNo, true);
					}
				}
			}
			cellHeight = doFormatCellText(cell, font, fm, collSize);

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
	if (collBeFormated && !m_headerV->getFixed(nmRow)) {
		m_headerV->setSize(nmRow,rowRealSize);
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
void uoReportDoc::doFormatDoc(int nmRow /*= -1*/, int nmForCol /*= -1*/)
{
	if (m_rows->getCountItem() <=0 && m_headerH->getCountItem() <=0 && 	m_headerV->getCountItem() <= 0)
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
		row = m_rows->getItem(curRowNum, false);
		while(row){
			doFormatRow(row, nmForCol);
			row = m_rows->getNextItem(curRowNum);
			if (row){
				curRowNum = row->number();
				if (recalcCntRowCol) {
					m_rowCount = curRowNum;
					m_colCount = qMax(row->getMaxNo(), m_colCount);
				}
			}
		}
	}
}

/// Установить текст в ячейку
void uoReportDoc::setCellText(const int row, const int col, const QString text)
{
	if (m_undoManager->isCollectChanges()){
		QString oldText = m_rows->getText(row, col);
		m_undoManager->doTextChange(oldText, row, col);
	}
	m_rows->setText(row, col, text);	// форматирование? угу.
	++m_changes;
	onAccessRowOrCol(row, uorRhtRowsHeader, true);
	onAccessRowOrCol(col, uorRhtColumnHeader, true);
	if (!m_formatStoped)
		doFormatRow(row, col);
}

/// Установить выравнивание текста в ячейке
void uoReportDoc::setCellTextAlignment(const int posY, const int posX, uoVertAlignment va,  uoHorAlignment ha, uoCellTextBehavior tb)
{
	uoCell* cell = m_rows->getCell(posY, posX, true);
	if (cell){
		cell->setAlignment(va,  ha, tb, this);
		++m_changes;
		doFormatRow(posY, posX);
	}
}

QString uoReportDoc::getCellText(const int row, const int col){
	return 	m_rows->getText(row, col);
}

uoCell* uoReportDoc::getCell(const int posY, const int posX, bool needCreate, bool provideProp){
	uoCell* cell = m_rows->getCell(posY, posX, needCreate);
	if (cell && provideProp)
		cell->provideAllProps(this, provideProp);
	if (needCreate){
		onAccessRowOrCol(posY, uorRhtRowsHeader, true);
		onAccessRowOrCol(posX, uorRhtColumnHeader, true);
	}
	return cell;
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
	qreal scaleFactorO = 1 / scaleFactor;
	/*
		тут одна тонкость: если в параметрах страницы стоит "по ширине листа",
		тогда scaleFactor надо пересчитать в зависимости от ширины листа вправо.
	*/
	const bool toWidthOfSheet = m_pagesSetings->widthOfSheet();


	int pogreshn = 20;

	pPrinter->setPageSize(m_pagesSetings->m_paperSize);
	pPrinter->setOrientation(m_pagesSetings->m_orientation);

	QRect pageRect = pPrinter->pageRect();
	m_paperRectPrint = pPrinter->paperRect();
	int pDpi = pPrinter->resolution();
	qreal kDpi = pDpi / 25.4; /// точек в милиметре.

	int fldSz = m_pagesSetings->m_fieldLeft + m_pagesSetings->m_fieldRight;
	fldSz = int(fldSz * kDpi);
	if (fldSz + pogreshn >pageRect.width()){
		qWarning() << QString::fromUtf8("Левое и правое поля черезчур велики!");
		return retVal;
	}
	fldSz = m_pagesSetings->m_fieldBottom + m_pagesSetings->m_fieldTop;
	fldSz = int(fldSz * kDpi);

	if (fldSz + pogreshn >pageRect.height()){
		qWarning() << QString::fromUtf8("Верхние и нижние поля поля черезчур велики!");
		return retVal;
	}

	fldSz =
	m_pagesSetings->m_fieldBottom +
	m_pagesSetings->m_fieldTop +
	m_pagesSetings->m_titleBotSize +
	m_pagesSetings->m_titleTopSize;

	fldSz = int(fldSz * kDpi);

	if (fldSz + pogreshn > pageRect.height()){
		qWarning() << QString::fromUtf8("Верхние и нижние c колонтитулами поля поля черезчур велики!");
		return retVal;
	}

	m_pageRectPrint = QRect(0,0,0,0);
	bool badField = false;

	fldSz = int(m_pagesSetings->m_fieldTop * kDpi);
	if (pageRect.y() != fldSz && pageRect.y() <= fldSz) {
		m_pageRectPrint.setY(fldSz);
	} else {
		qWarning() << QString::fromUtf8("Скорректировали верхнее поле: слишком маленькое!");
		m_pageRectPrint.setY(pageRect.y());
	}

	fldSz = int(m_pagesSetings->m_fieldLeft * kDpi);
	if (pageRect.x() != fldSz && pageRect.x() <= fldSz) {
		m_pageRectPrint.setX(fldSz);
	} else {
		qWarning() << QString::fromUtf8("Скорректировали левое поле: слишком маленькое!");
		m_pageRectPrint.setX(pageRect.x());
	}

	fldSz = int(m_paperRectPrint.width() - (m_pagesSetings->m_fieldRight * kDpi) - m_pageRectPrint.x());
	if (fldSz+pogreshn > 0) {
		m_pageRectPrint.setWidth(fldSz);
	} else {
		qWarning() << QString::fromUtf8("Ширина слишком маленькая!");
		badField = true;
	}

	fldSz = int(m_paperRectPrint.height() - (m_pagesSetings->m_fieldTop * kDpi) - m_pageRectPrint.y());
	if (fldSz+pogreshn > 0) {
		m_pageRectPrint.setHeight(fldSz);
	} else {
		qWarning() << QString::fromUtf8("Высота слишком маленькая!");
		badField = true;
	}

	if (debug_this) {
		qDebug() << QString::fromUtf8("Расчет ректов страницы{");
		qDebug() << "pageRect" << pageRect;
		qDebug() << "m_paperRectPrint" << m_paperRectPrint;
		qDebug() << "m_pageRectPrint" << m_pageRectPrint;
		qDebug() << QString::fromUtf8("}Расчет ректов страницы");
	}
	m_pageRectPrint.adjust(-1,-1,1,1);

	m_pageRectPrint.setBottom(m_pageRectPrint.bottom()*scaleFactorO);
	m_pageRectPrint.setRight(m_pageRectPrint.right()*scaleFactorO);
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
	qreal scaleFactorO = 1 / scaleFactor;
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

	qreal rigthLengMax = 0.0;
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
	qreal pageWidth = pageRectPrintCpy.width();
	qreal nmColWidth = 0.0, nmColsWidth = 0.0, nmColsWidthAll = 0.0;

	if (toWidthOfSheet){
		for (nmColCntr=1; (nmColsWidthAll < rigthLengMax || nmColCntr<=m_colCount); nmColCntr++)
		{
			nmColWidth = getScaleSize(uorRhtColumnHeader, nmColCntr);
			nmColsWidthAll += nmColWidth;
		}
		scaleFactor = pageRectPrintCpy.width() / nmColsWidthAll;
		scaleFactor = qMax(0.1, scaleFactor);
		m_pagesSetings->setScale(scaleFactor);
		if (scaleFactor != 1.0){
			m_pagesSetings->transformPageRect(m_pageRectPrint);
		}
		segment->m_segmWidth = nmColsWidthAll;
		segment->m_colEnd = nmColCntr;
		return;
	}
	if (scaleFactor != 1.0){
		m_pagesSetings->transformPageRect(m_pageRectPrint);
		pageRectPrintCpy = m_pageRectPrint;
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
		nmColWidth = nmColWidth * scaleFactorO;

		nmColsWidthAll += nmColWidth ;
		if (debug_this) {			qDebug() << " col  # " << nmColCntr << " ColWidth " << nmColWidth << "nmColsWidthAll"<<nmColsWidthAll;		}
		if ((nmColWidth + nmColsWidth)<=pageWidth){
			segment->m_colEnd = nmColCntr;
			nmColsWidth += nmColWidth;
			segment->m_segmWidth = nmColsWidth;
		} else {
			nmColsWidth = nmColWidth;
			if (nmColsWidth>=pageWidth){
				offset = 0.0;
				do {
					if (segment->m_segmWidth > 0.0){
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
	для того, что-бы updatePageRectPrint увидел масштаб страницы...
*/
void uoReportDoc::updateScaleFactor()
{}

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

	const bool toWidthOfSheet = m_pagesSetings->widthOfSheet();
	if (toWidthOfSheet){
		// для того, что-бы updatePageRectPrint увидел масштаб страницы...
		updateScaleFactor();
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
	pageRectPrintCpy.adjust(2,2,-2,-2);


	int pageCnt = 0;

	uorPageColSegment* segment = NULL;

	uorReportPrintArea* areaPrint = NULL;
	uorReportPrintArea* areaBaseLast = NULL;

	int pagesColumnCurent = 1;
	qreal pageHeightCntr = 0.0;
	qreal rowsHeight = 0.0;
	qreal offset = 0.0;


	QString cellText;

	while (pagesColumnCurent <= m_pagesColumnTotal) {
		areaPrint = new uorReportPrintArea;
		segment = m_segmentList.at(pagesColumnCurent-1);
		m_pageList->append(areaPrint);
		areaPrint->m_firstVisible_ColLeft 	= segment->m_colStart;
		areaPrint->m_lastVisibleCol 		= segment->m_colEnd;
		areaPrint->m_firstVisible_RowTop 	= 1;
		areaPrint->m_lastVisibleRow 		= 1;
		areaPrint->m_pageColumn 			= segment->m_segmentNom;
		areaPrint->m_shift_ColLeft 			= segment->m_offsetStart;
		areaPrint->m_pageNumber 			= m_pageList->count();
		areaPrint->m_segment 				= segment;
		areaPrint->m_area.setTopLeft(pageRectPrintCpy.topLeft());
		areaPrint->m_area.setWidth(qMin(pageRectPrintCpy.width(),int(segment->m_segmWidth)));
		areaPrint->m_area.setHeight(pageRectPrintCpy.height());
		pageHeightCntr = 0.0;

		for (int nRow = 1; nRow<= m_rowCount; nRow++)
		{
			rowsHeight = getScaleSize(uorRhtRowsHeader, nRow);
//			rowsHeight = rowsHeight * scaleFactor;

			if ((pageHeightCntr + rowsHeight) <= pageRectPrintCpy.height()){
				// высота строки нормальная для страницы..
				areaPrint->m_lastVisibleRow = nRow;
				pageHeightCntr += rowsHeight;
				areaPrint->m_area.setHeight(pageHeightCntr);
			} else {

				if (rowsHeight>pageRectPrintCpy.height()){
					// Упс, высота строки превышает высоту страницы О_о. Бум резать... папазжа...
					offset = 0.0;
					do {
						offset += pageRectPrintCpy.height();
						areaPrint = new uorReportPrintArea;
						m_pageList->append(areaPrint);
						areaPrint->m_pageNumber 			= m_pageList->count();
						areaPrint->m_firstVisible_RowTop 	= nRow;
						areaPrint->m_lastVisibleRow 		= nRow;
						areaPrint->m_firstVisible_ColLeft 	= segment->m_colStart;
						areaPrint->m_lastVisibleCol 		= segment->m_colEnd;
						areaPrint->m_pageColumn 			= segment->m_segmentNom;
						areaPrint->m_shift_ColLeft 			= segment->m_offsetStart;
						areaPrint->m_shift_RowTop 			= offset;
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
					areaPrint->m_firstVisible_RowTop 	= nRow;
					areaPrint->m_lastVisibleRow 		= nRow;
					areaPrint->m_firstVisible_ColLeft 	= segment->m_colStart;
					areaPrint->m_lastVisibleCol 		= segment->m_colEnd;
					areaPrint->m_pageColumn 			= segment->m_segmentNom;
					areaPrint->m_shift_ColLeft 			= segment->m_offsetStart;
					areaPrint->m_area.setTopLeft(pageRectPrintCpy.topLeft());
					areaPrint->m_area.setWidth(qMin(pageRectPrintCpy.width(),int(segment->m_segmWidth)));

					areaPrint->m_area.setHeight(pageHeightCntr);
					areaPrint->m_segment 				= segment;
				}
			}
		}
		++pagesColumnCurent;
	}

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
					<< "rows" << areaPrint->m_firstVisible_RowTop << " - " << areaPrint->m_lastVisibleRow << " - " <<areaPrint->m_shift_RowTop
					<< "cols" << areaPrint->m_firstVisible_ColLeft << " - " << areaPrint->m_lastVisibleCol<< " - " <<areaPrint->m_shift_ColLeft
					<< "m_area" << areaPrint->m_area
					<< "m_pageColumn" << areaPrint->m_pageColumn;
				} else {
					qDebug() << "\t" << areaPrint->m_pageNumber << "\t"
					<< areaPrint->m_firstVisible_RowTop << "\t"
					<< areaPrint->m_lastVisibleRow << "\t"
					<< areaPrint->m_shift_RowTop << "\t"
					<< areaPrint->m_firstVisible_ColLeft << "\t"
					<< areaPrint->m_lastVisibleCol << "\t"
					<< areaPrint->m_shift_ColLeft << "\t"
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
	if (!list)
		return retVal;

	if (!setupPrinter(*pPrinter, qwidg))
		return retVal;

	int pageNomFrom = 0, pageNomTo = 0;
	QPrinter::PrintRange prnRange = pPrinter->printRange();
	if (prnRange == QPrinter::PageRange) {
		pageNomFrom = pPrinter->fromPage();
		pageNomTo = pPrinter->toPage();
	}
	qreal scaleFactor = m_pagesSetings->scale();
	qDebug() << QString("bool uoReportDoc::printDoc scaleFactor = %1").arg(scaleFactor);

    QPainter painter;
    painter.begin(pPrinter);
    painter.scale(scaleFactor,scaleFactor);
    bool firstPage = true;
	uoReportDrawHelper* drawHelper = new uoReportDrawHelper(this);
	drawHelper->initDrawInstruments();


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

	return true;
}

bool uoReportDoc::getScaleHide(uoRptHeaderType hType, int nom){
	if (hType == uorRhtRowsHeader)
		return m_headerV->getHide(nom);
	else
		return m_headerH->getHide(nom);
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
	QColor* col = NULL;
	return col;
}


/// создаем новую структуру uorTextDecor для хранения атрибутов текста.
/// теперь есть возможность вставить акселератор для считывания больших документов.
uorTextDecor* uoReportDoc::getNewTextProp()
{
	///\todo сбацать акселераторы под считывание документов
	uorTextDecor* prop = m_cellTextDecorList.getItem();
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
	uorBorderPropBase* prop = m_cellBordPropList.getItem(); //new uorBorderPropBase();
	return prop;
}


/// Создаем новую uoCellJoin
uoCellJoin* uoReportDoc::getCellJoin()
{
	return m_cellJoinCash.getItem();
}

/// Утилизируем uoCellJoin
void uoReportDoc::saveCellJoin(uoCellJoin* cellJItem)
{
	m_cellJoinCash.saveItem(cellJItem);
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

	qreal sz = 17;

    qDebug()<<"Start test class \"uoReportDocBody\"";
    qDebug()<<"{";
	if (false || printAll) {
		setScaleSize(uorRhtRowsHeader, 2, 15);
		setScaleSize(uorRhtRowsHeader, 4, 18); if (printCurent) m_headerV->printToDebug();
		setScaleSize(uorRhtRowsHeader, 3, 19); if (printCurent) m_headerV->printToDebug();
		setScaleSize(uorRhtRowsHeader, 5, 25); if (printCurent) m_headerV->printToDebug();
		setScaleSize(uorRhtRowsHeader, 2, sz); if (printCurent) m_headerV->printToDebug();
		setScaleSize(uorRhtRowsHeader, 1, sz); if (printCurent) m_headerV->printToDebug();
		setScaleSize(uorRhtRowsHeader, 7, 11); if (printCurent) m_headerV->printToDebug();

		toDebugTest(getScaleSize(uorRhtRowsHeader, 1) == sz, &nTestOk, &nTestAll, "getScaleSize(uorRhtRowsHeader, 1) == sz");
		toDebugTest(getScaleSize(uorRhtRowsHeader, 2) == sz, &nTestOk, &nTestAll, "getScaleSize(uorRhtRowsHeader, 2) == sz");
		qDebug() << " size " << m_headerV->getCountItem();
		qDebug() << " m_headerV->deleteItem(2,2); ";
		m_headerV->deleteItem(2,2);
		qDebug() << " size " << m_headerV->getCountItem();
		m_headerV->printToDebug();
	}


    qDebug()<<"Test all: "<<nTestAll<<" test OK: "<< nTestOk<<" test is: "<< (nTestOk==nTestAll);
    qDebug()<<"End test class \"uoReportDocBody\"";
    qDebug()<<"}";

}

} // namespace uoReport
