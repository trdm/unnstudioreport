/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "QDebug"
#include "uoReportDoc.h"
#include "uoReportLoader.h"

namespace uoReport {


uoReportDoc::uoReportDoc()
//	:uoReportDocBody()
	: _spanTreeGrH(new uoSpanTree)
	, _spanTreeGrV(new uoSpanTree)
	, _spanTreeSctH(new uoSpanTree)
	, _spanTreeSctV(new uoSpanTree)
	, _headerV(new uoHeaderScale)
	, _headerH(new uoHeaderScale)
	, _rows(new uoRowsDoc)
{
	_spanTreeSctH->setCanOnlyOne(true);
	_spanTreeSctV->setCanOnlyOne(true);
	_storeFormat = uoRsf_Unknown;

	_headerV->setDefSize(UORPT_SCALE_SIZE_DEF_VERTICAL);
	_headerH->setDefSize(UORPT_SCALE_SIZE_DEF_HORIZONTAL);

	_rowCount_visible = _rowCount 	= 0;
	_colCount_visible = _colCount 	= 0;

	_sizeV_visible = _sizeV = 0.0;	///< Размер документа по вертикали
	_sizeH_visible = _sizeH = 0.0;	///< Размер документа по горизонтали
	_freezEvent = 0;
	_refCounter = 0;
}

uoReportDoc::~uoReportDoc()
{
	clear();
	delete _spanTreeGrH;
	delete _spanTreeGrV;
	delete _spanTreeSctH;
	delete _spanTreeSctV;
	delete _headerV;
	delete _headerH;

}

/// Очистка секций.
void uoReportDoc::clear()
{
	_spanTreeGrH->clear();
	_spanTreeGrV->clear();
	_spanTreeSctH->clear();
	_spanTreeSctV->clear();

	_rowCount_visible = _rowCount 	= 0;
	_colCount_visible = _colCount 	= 0;

	_sizeV_visible = _sizeV = 0.0;	///< Размер документа по вертикали
	_sizeH_visible = _sizeH = 0.0;	///< Размер документа по горизонтали

}

/// проверка возможности сформировать/вставить группу в документ.
/// может потребоваться для того, что-бы задизаблить пункты динамического меню
bool uoReportDoc::possiblyAddGroup(int start, int end, uoRptHeaderType ht)
{
	if(ht == rhtVertical)	return _spanTreeGrV->possiblyAddSpan(start, end);
	else					return _spanTreeGrH->possiblyAddSpan(start, end);
}

/// добавление группировки в отчет.
bool uoReportDoc::addGroup(int start, int end, uoRptHeaderType ht)
{
	bool retVal = false;
	uoSpanTree* treeGrp = NULL;

	if(ht == rhtVertical) {
		treeGrp = _spanTreeGrV;
		beforeAddRowOrCol(end - _rowCount, ht);
	} else if (ht == rhtHorizontal) {
		treeGrp = _spanTreeGrH;
		beforeAddRowOrCol(end - _colCount, ht);
	}
	retVal = treeGrp->addSpan(start, end);

	return retVal;
}

/// Максимальный уровень вложения групп.
int uoReportDoc::getGroupLevel(uoRptHeaderType ht){
	uoSpanTree* treeGrp = NULL;
	if(ht == rhtVertical)
		treeGrp = _spanTreeGrV;
	else
		treeGrp = _spanTreeGrH;
	return treeGrp->getLevel();
}

/// Максимальный уровень вложения секций.
int uoReportDoc::getSectionLevel(uoRptHeaderType ht){
	uoSpanTree* treeGrp = NULL;
	if(ht == rhtVertical)	treeGrp = _spanTreeSctV;
	else					treeGrp = _spanTreeSctH;
	return treeGrp->getLevel();
}

/// Свертка/развертка группы.
void uoReportDoc::doGroupFold(int idGrop, uoRptHeaderType rht, bool fold){
	uoSpanTree* treeGrp = NULL;
	if(rht == rhtVertical)	treeGrp = _spanTreeGrV;
	else					treeGrp = _spanTreeGrH;
	++_freezEvent;
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
	--_freezEvent;
	delete lineList;
	if (_freezEvent == 0)
		emit onSizeChange(_sizeV_visible, _sizeH_visible);
}

/// Возвращает установленный формат сохранения
uoRptStoreFormat uoReportDoc::getStoreFormat()	{
	return _storeFormat;
}

/// Возвращает установленное имя файла.
QString uoReportDoc::getStorePathFile()	{
	return _docFilePath;
}

/// установим опции сохранения
void uoReportDoc::setStoreOptions(QString  filePath, uoRptStoreFormat stFormat)	{
	_docFilePath = filePath;
	_storeFormat = stFormat;
}

/// Получить список спанов группировок по диапазону строк/столбцов. Исключая зафолденные...
const spanList* uoReportDoc::getGroupList(uoRptHeaderType rht, int start, int end)
{
	if (rht == rhtHorizontal)
		return _spanTreeGrH->getSpanList(start, end, true);
	else
		return _spanTreeGrV->getSpanList(start, end, true);
}


/// проверка возможности сформировать/вставить СЕКЦИЮ в документ.
/// может потребоваться для того, что-бы задизаблить пункты динамического меню
bool uoReportDoc::possiblyAddSection(int start, int end, uoRptHeaderType ht)
{
	if(ht == rhtHorizontal)
		return _spanTreeSctH->possiblyAddSpan(start, end);
	else
		return _spanTreeSctV->possiblyAddSpan(start, end);
}

/// вставка секции в отчет
bool uoReportDoc::addSection(int start, int end, uoRptHeaderType ht)
{
	if(ht == rhtHorizontal)	return _spanTreeSctH->addSpan(start, end);
	else					return _spanTreeSctV->addSpan(start, end);
}

void uoReportDoc::onDeleteLine(int lineStart, int count)
{
	_spanTreeSctH->onLinesDelete(lineStart, count);
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

	uoRptHeaderType curHeaderType = rhtHorizontal;
	// Тут просто перебор документа с обращением к лоадеру..
	retVal = loader->saveDocStart(this);
	if (!retVal) return false;

	// ---------------выгрузка группировок --------------
	for (typeHeader = 1; typeHeader<=2; typeHeader++) {
		if(typeHeader == 1){
			curTree	= _spanTreeGrH;
			curHeaderType = rhtHorizontal;
		} else {
			curTree	= _spanTreeGrV;
			curHeaderType = rhtVertical;
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
			curTree	= _spanTreeSctH;
			curHeaderType = rhtHorizontal;
		} else {
			curTree	= _spanTreeSctV;
			curHeaderType = rhtVertical;
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
			headerScale	= _headerH;
			curHeaderType = rhtHorizontal;
		} else {
			headerScale	= _headerV;
			curHeaderType = rhtVertical;
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
	int rowCount = _rows->getCountItem();
	if (rowCount>0) {
		loader->saveRowsStart(rowCount);
		_rows->saveItems(loader);
		loader->saveRowsEnd();
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
	if (_docFilePath.isEmpty() || _storeFormat == uoRsf_Unknown) {
		return false;
	}
	return true;

}

/// Сохранение отчета.
bool uoReportDoc::save(){
	if (saveOptionsIsValid()) {
		return saveToFile(_docFilePath, _storeFormat);
	}
	return false;
}

/// Возвращаем дефолтный размер строки или колонки
qreal  	uoReportDoc::getDefScaleSize(uoRptHeaderType rht)
{
	if (rht == rhtVertical){
		return _headerV->getDefSizeItem();
	} else if (rht == rhtHorizontal) {
		return _headerH->getDefSizeItem();
	}
	return 0.0;
}

/// Возвращаем высоту документа полную (по умолчанию) или видимую.
qreal  	uoReportDoc::getVSize(bool visible){
	if (visible)
		return _sizeV_visible;
	else
		return _sizeV;
}

/// Возвращаем длину документа полную (по умолчанию) или видимую.
qreal  	uoReportDoc::getHSize(bool visible){
	if (visible)
		return _sizeH_visible;
	else
		return _sizeH;
}

/// Возвращаем количество строк
int 	uoReportDoc::getRowCount(){	return _rowCount;}

/// Возвращаем количество столбцов
int 	uoReportDoc::getColCount(){	return _colCount;}


/// перед добавлением строк или столбцов. Необходимо что-бы посчитать длину/ширину дока.
void uoReportDoc::beforeAddRowOrCol(int count, uoRptHeaderType rht, int noLn)
{
	if (count <= 0)
		return;
	int oldCnt = 0;

	uoHeaderScale* header = NULL;
	if (rht == rhtVertical) {
		header = _headerV;
		oldCnt = _rowCount;
	} else {
		header = _headerH;
		oldCnt = _colCount;
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


	if (rht == rhtVertical) {
		_rowCount 		= _rowCount + count;
		_sizeV 			= _sizeV + addSize;
		_sizeV_visible 	= _sizeV_visible + addSizeVis;
	} else {
		_colCount 		= _colCount + count;
		_sizeH 			= _sizeH + addSize;
		_sizeH_visible 	= _sizeH_visible + addSizeVis;
	}
	///\todo 1 А вот тут нужен сигнал на изменение размеров документа....
	if (_freezEvent == 0)
		emit onSizeChange(_sizeV_visible, _sizeH_visible);
}

/// Изменить количество строк в документе
void uoReportDoc::doRowCountChange(int count, int pos)
{
	if (count == 0)
		return;
	qreal oldSize = _sizeV;
	qreal oldSizeVis = _sizeV_visible;

	_rowCount = _rowCount + count;
	_sizeV = oldSize + count * getDefScaleSize(rhtVertical);
	_sizeV_visible = oldSizeVis + count * getDefScaleSize(rhtVertical);
	if (_freezEvent == 0)
		emit onSizeChange(_sizeV_visible, _sizeH_visible);
}

/// Изменить количество столбцов в документе
void uoReportDoc::doColCountChange(int count, int pos )
{
	if (count == 0)
		return;
	qreal oldSize = _sizeH;
	qreal oldSizeVis = _sizeH_visible;

	_colCount = _colCount + count;
	_sizeH = oldSize + count * getDefScaleSize(rhtHorizontal);
	_sizeH_visible = oldSizeVis + count * getDefScaleSize(rhtHorizontal);
	if (_freezEvent == 0)
		emit onSizeChange(_sizeV_visible, _sizeH_visible);

}


/// При доступе к строке или ячейке на запись. Служит для определения длины/ширины документа.
void uoReportDoc::onAccessRowOrCol(int nom, uoRptHeaderType rht)
{
	int cnt = 0;
	if (rht == rhtHorizontal) // Колонка
	{
		if (_colCount < nom) {
			cnt = nom - _colCount;
			doColCountChange(cnt, _colCount+1);
		}
	} else if (rht == rhtVertical) {		// строка
		if (_rowCount < nom) {
			cnt = nom - _rowCount;
			doRowCountChange(cnt, _rowCount+1);
		}
	}
}

qreal uoReportDoc::getScaleSize(uoRptHeaderType hType, int nom, bool isDef)
{
	if (hType == rhtVertical)
		return _headerV->getSize(nom, isDef);
	else
		return _headerH->getSize(nom, isDef);
}

void uoReportDoc::setScaleSize(uoRptHeaderType hType, int nom, qreal size, bool isDef){
	bool scVisible = true;
	qreal oldSizeItem = 0.0, oldSize = 0.0;
	if (hType == rhtVertical) {
		oldSizeItem = _headerV->getSize(nom, isDef);
		scVisible = _headerV->getHide(nom);
		_headerV->setSize(nom, size, isDef);
		_sizeV = _sizeV - oldSizeItem + size;
		if (!scVisible) {
			oldSize = _sizeV_visible;
			_sizeV_visible = _sizeV_visible - oldSizeItem + size;
			if (nom <= _rowCount)
				emit onSizeVisibleChangeV(_sizeV_visible, _rowCount, oldSize, _rowCount, nom);
		}
	}
	else {
		oldSizeItem= _headerH->getSize(nom, isDef);
		_headerH->setSize(nom, size, isDef);
		_sizeH = _sizeH - oldSizeItem + size;
		scVisible = _headerH->getHide(nom);
		if (!scVisible) {
			oldSize = _sizeH_visible;
			_sizeH_visible = _sizeH_visible - oldSizeItem + size;
			if (nom <= _colCount)
				emit onSizeVisibleChangeH(_sizeH_visible, _colCount, oldSize, _colCount, nom);
		}
	}
	onAccessRowOrCol(nom-1, hType); // последнюю ячейку не трогаем, т.к. её размер устанавливается...
}

/// Прячем/Показываем диапазон ячеек...
void uoReportDoc::setScalesHide(uoRptHeaderType hType, int nmStart, int cnt,  bool hide){
	uoHeaderScale* header = NULL;
	if (hType == rhtVertical) {
		header = _headerV;
	} else {
		header = _headerH;
	}
	qreal szAdd = 0.0;
	for (int i = 0; i<cnt; i++)	{
		szAdd = szAdd + header->getSize(nmStart + i);
		header->setHide(nmStart + i, hide);
	}
	qreal oldSize = 0.0;
	szAdd = szAdd * ( hide ? -1 : 1);
	if (hType == rhtVertical){
		oldSize = _sizeV_visible;
		_sizeV_visible = _sizeV_visible + szAdd;
		if (_freezEvent == 0)
			emit onSizeChange(_sizeV_visible, _sizeH_visible);
	} else {
		oldSize = _sizeH_visible;
		_sizeH_visible = _sizeH_visible + szAdd;
		if (_freezEvent == 0)
			emit onSizeChange(_sizeV_visible, _sizeH_visible);
	}

	onAccessRowOrCol(nmStart + cnt - 1, hType);
}

void uoReportDoc::setCellText(const int posY, const int posX, const QString text)
{
	_rows->setText(posY, posX, text);
}
QString uoReportDoc::getCellText(const int posY, const int posX){
	return 	_rows->getText(posY, posX);
}

uoCell* uoReportDoc::getCell(const int posY, const int posX, bool needCreate){
	return _rows->getCell(posY, posX, needCreate);
}


bool uoReportDoc::getScaleHide(uoRptHeaderType hType, int nom){
	if (hType == rhtVertical)
		return _headerV->getHide(nom);
	else
		return _headerH->getHide(nom);
}

/// присоединим вьюв..
void uoReportDoc::attachView(uoReportCtrl* rCtrl, bool autoConnect){
	bool found = false;
	if (!_atachedView.isEmpty()) {
		 found = _atachedView.contains(rCtrl);
	}
	if (!found) {
		++_refCounter;
		_atachedView.append(rCtrl);
		/// надо его законнектить к сигналам. Тока сигналы до ума довести.....
		if (autoConnect) {
	//		connect(
		}

	}
}

/// Отсоединяемся от вьюва.
void uoReportDoc::detachedView(uoReportCtrl* rCtrl)
{
	if (!_atachedView.isEmpty()) {
		if (_atachedView.contains(rCtrl)) {
			int pos = 0; _atachedView.indexOf(rCtrl);
			while((pos = _atachedView.indexOf(rCtrl))>0){
				_atachedView.removeAt(pos);
			}
			--_refCounter;
		}
	}
}

/// Объект используется/не используется. Если используется, удалять нельзя...
bool uoReportDoc::isObjectAttached()
{
	if (_refCounter > 0 || !_atachedView.isEmpty() || !_atachedObj.isEmpty())
		return true;
	return false;
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
		setScaleSize(rhtVertical, 2, 15);
		setScaleSize(rhtVertical, 4, 18); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 3, 19); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 5, 25); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 2, sz); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 1, sz); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 7, 11); if (printCurent) _headerV->printToDebug();

		toDebugTest(getScaleSize(rhtVertical, 1) == sz, &nTestOk, &nTestAll, "getScaleSize(rhtVertical, 1) == sz");
		toDebugTest(getScaleSize(rhtVertical, 2) == sz, &nTestOk, &nTestAll, "getScaleSize(rhtVertical, 2) == sz");
		qDebug() << " size " << _headerV->getCountItem();
		qDebug() << " _headerV->deleteItem(2,2); ";
		_headerV->deleteItem(2,2);
		qDebug() << " size " << _headerV->getCountItem();
		_headerV->printToDebug();
	}


    qDebug()<<"Test all: "<<nTestAll<<" test OK: "<< nTestOk<<" test is: "<< (nTestOk==nTestAll);
    qDebug()<<"End test class \"uoReportDocBody\"";
    qDebug()<<"}";

}

} // namespace uoReport
